#include "rangeallocator.h"

#include <wax_rangeallocator.h>

#include <vector>

using namespace wax;

RangeAllocatorTest::RangeAllocatorTest()
{
	ADD_TEST(allocTest);
	ADD_TEST(alignmentTest);
	ADD_TEST(joinTest);
	ADD_TEST(badFreeTest);
	ADD_TEST(randomTest);
}

RangeAllocatorTest::~RangeAllocatorTest()
{
}

bool RangeAllocatorTest::init(std::ostream& output)
{
	return true;
}

//____ allocTest() ____________________________________________________________
//
// Ranges come from the start of the block, one after the other, until it is full.

bool RangeAllocatorTest::allocTest(std::ostream& output)
{
	RangeAllocator a(1024, 1);

	TEST_ASSERT(a.size() == 1024);
	TEST_ASSERT(a.isEmpty());
	TEST_ASSERT(a.largestFree() == 1024);

	TEST_ASSERT(a.alloc(100) == 0);
	TEST_ASSERT(a.alloc(200) == 100);
	TEST_ASSERT(a.alloc(724) == 300);

	TEST_ASSERT(a.allocated() == 1024);
	TEST_ASSERT(a.largestFree() == 0);
	TEST_ASSERT(a.alloc(1) == -1);

	// Nothing to give out, and nothing bigger than the block.

	TEST_ASSERT(a.alloc(0) == -1);
	TEST_ASSERT(a.alloc(-5) == -1);

	RangeAllocator b(1024, 1);
	TEST_ASSERT(b.alloc(1025) == -1);
	TEST_ASSERT(b.alloc(1024) == 0);

	// A range given back is the first to be taken again, if it is big enough.

	a.free(100, 200);
	TEST_ASSERT(a.allocated() == 824);
	TEST_ASSERT(a.alloc(201) == -1);
	TEST_ASSERT(a.alloc(50) == 100);
	TEST_ASSERT(a.alloc(150) == 150);
	TEST_ASSERT(a.allocated() == 1024);

	return true;
}

//____ alignmentTest() ________________________________________________________
//
// Offsets and sizes are whole numbers of the alignment, and what is left over at the end of
// the block isn't used.

bool RangeAllocatorTest::alignmentTest(std::ostream& output)
{
	RangeAllocator a(1000, 192);

	TEST_ASSERT(a.size() == 960);

	TEST_ASSERT(a.alloc(1) == 0);
	TEST_ASSERT(a.alloc(192) == 192);
	TEST_ASSERT(a.alloc(193) == 384);
	TEST_ASSERT(a.allocated() == 768);
	TEST_ASSERT(a.largestFree() == 192);

	TEST_ASSERT(a.alloc(193) == -1);
	TEST_ASSERT(a.alloc(100) == 768);
	TEST_ASSERT(a.allocated() == 960);

	// Freed with the size asked for, which is rounded the same way.

	a.free(384, 193);
	TEST_ASSERT(a.allocated() == 576);
	TEST_ASSERT(a.largestFree() == 384);

	a.free(0, 1);
	a.free(192, 192);
	a.free(768, 100);
	TEST_ASSERT(a.isEmpty());
	TEST_ASSERT(a.largestFree() == 960);

	return true;
}

//____ joinTest() _____________________________________________________________
//
// A range given back becomes one with the free ranges next to it, in whichever order they
// were given back.

bool RangeAllocatorTest::joinTest(std::ostream& output)
{
	const int orders[6][3] = { {0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0} };

	for( auto& order : orders )
	{
		RangeAllocator a(500, 1);

		int offsets[5];
		for( int i = 0 ; i < 5 ; i++ )
			offsets[i] = a.alloc(100);

		TEST_ASSERT(a.largestFree() == 0);

		// The three in the middle, with one taken on either side.

		for( int i = 0 ; i < 3 ; i++ )
			a.free(offsets[1 + order[i]], 100);

		TEST_ASSERT(a.allocated() == 200);
		TEST_ASSERT(a.largestFree() == 300);
		TEST_ASSERT(a.alloc(300) == 100);

		// And with the ends of the block.

		a.free(100, 300);
		a.free(0, 100);
		TEST_ASSERT(a.largestFree() == 400);
		a.free(400, 100);
		TEST_ASSERT(a.isEmpty());
		TEST_ASSERT(a.largestFree() == 500);
		TEST_ASSERT(a.alloc(500) == 0);
	}

	return true;
}

//____ badFreeTest() __________________________________________________________
//
// Giving back what wasn't taken changes nothing.

bool RangeAllocatorTest::badFreeTest(std::ostream& output)
{
	RangeAllocator a(1024, 64);

	TEST_ASSERT(a.alloc(128) == 0);
	TEST_ASSERT(a.alloc(128) == 128);

	a.free(256, 64);			// Free already.
	a.free(192, 128);			// Half of it free already.
	a.free(-64, 64);
	a.free(1024, 64);			// Past the end.
	a.free(960, 128);
	a.free(32, 64);				// Not where a range can start.
	a.free(0, 0);

	TEST_ASSERT(a.allocated() == 256);
	TEST_ASSERT(a.largestFree() == 768);

	a.free(0, 128);
	a.free(0, 128);				// Twice.

	TEST_ASSERT(a.allocated() == 128);
	TEST_ASSERT(a.alloc(128) == 0);
	TEST_ASSERT(a.alloc(768) == 256);

	return true;
}

//____ randomTest() ___________________________________________________________
//
// Many ranges taken and given back in an order of no pattern, checked against a map of the
// block with an owner for each unit of it: no two ranges may share any, and what the allocator
// says is taken must be what the map has.

bool RangeAllocatorTest::randomTest(std::ostream& output)
{
	const int alignment = 16;
	const int units = 256;

	RangeAllocator a(units * alignment, alignment);

	struct Taken { int offset; int size; };

	std::vector<Taken>	taken;
	std::vector<int>	owner(units, -1);

	uint32_t seed = 12345;
	auto random = [&seed]() { seed = seed * 1664525u + 1013904223u; return int(seed >> 8); };

	for( int step = 0 ; step < 20000 ; step++ )
	{
		bool bAlloc = taken.empty() || (random() % 100) < 55;

		if( bAlloc )
		{
			int size = 1 + random() % (alignment * 12);
			int nUnits = (size + alignment - 1) / alignment;

			// The first run of free units that is long enough is where it should end up.

			int expected = -1;
			int run = 0;

			for( int i = 0 ; i < units && expected < 0 ; i++ )
			{
				run = (owner[i] < 0) ? run + 1 : 0;
				if( run == nUnits )
					expected = (i + 1 - nUnits) * alignment;
			}

			int offset = a.alloc(size);
			TEST_ASSERT(offset == expected);

			if( offset >= 0 )
			{
				for( int i = 0 ; i < nUnits ; i++ )
				{
					TEST_ASSERT(owner[offset / alignment + i] < 0);
					owner[offset / alignment + i] = step;
				}

				taken.push_back( { offset, size } );
			}
		}
		else
		{
			int index = random() % int(taken.size());
			Taken t = taken[index];
			taken.erase(taken.begin() + index);

			a.free(t.offset, t.size);

			int nUnits = (t.size + alignment - 1) / alignment;
			for( int i = 0 ; i < nUnits ; i++ )
				owner[t.offset / alignment + i] = -1;
		}

		int nTaken = 0;
		int largest = 0;
		int run = 0;

		for( int i = 0 ; i < units ; i++ )
		{
			if( owner[i] >= 0 )
			{
				nTaken++;
				run = 0;
			}
			else if( ++run > largest )
				largest = run;
		}

		TEST_ASSERT(a.allocated() == nTaken * alignment);
		TEST_ASSERT(a.largestFree() == largest * alignment);
	}

	for( auto& t : taken )
		a.free(t.offset, t.size);

	TEST_ASSERT(a.isEmpty());
	TEST_ASSERT(a.largestFree() == units * alignment);

	return true;
}
