#include <string.h>


#include "gfxstream.h"

#include <wondergui.h>
#include <wondergfxstream.h>

#include <wg_string.h>

using namespace wg;

GfxStreamTest::GfxStreamTest()
{

	ADD_TEST(streamLoopWrapperTest);
	ADD_TEST(streamReaderPumpWithOptimizationTest);

}

GfxStreamTest::~GfxStreamTest()
{
}


bool GfxStreamTest::init(std::ostream& output)
{
	return true;
}

//____ _createTestStream() ____________________________________________________
//
// A stream of chunks in the current format, made here so it can't fall behind
// the format the way a recorded one does. The tests below only pass chunks on,
// so what they hold doesn't matter, only that they are well formed and come in
// every size: each size from an empty chunk up to GfxStream::c_maxBlockSize
// once, then a run of sizes in no particular order, so that chunks get split
// at all sorts of places where a loop buffer wraps.

Blob_p GfxStreamTest::_createTestStream()
{
	const int maxDataSize = GfxStream::c_maxBlockSize - GfxStream::HeaderSize;
	const int nSizes = maxDataSize / 2 + 1;			// Data sizes are even.
	const int nRandomChunks = 1000;

	std::vector<int> dataSizes;

	for( int i = 0 ; i < nSizes ; i++ )
		dataSizes.push_back(i * 2);

	uint32_t seed = 12345;
	auto random = [&seed]() { seed = seed * 1664525 + 1013904223; return seed >> 8; };

	for( int i = 0 ; i < nRandomChunks ; i++ )
		dataSizes.push_back( int(random() % nSizes) * 2 );

	int streamSize = 0;
	for( int size : dataSizes )
		streamSize += GfxStream::HeaderSize + size;

	Blob_p pBlob = Blob::create(streamSize);

	uint16_t data[GfxStream::c_maxBlockSize / 2];
	uint8_t * pWrite = (uint8_t *) pBlob->data();
	int chunkType = 1;

	for( int size : dataSizes )
	{
		for( int i = 0 ; i < size / 2 ; i++ )
			data[i] = uint16_t(random());

		GfxStream::createChunk( pWrite, GfxStream::ChunkId(chunkType), size, data );
		pWrite += GfxStream::HeaderSize + size;

		chunkType = chunkType % int(GfxStream::ChunkId_max) + 1;		// Any but OutOfData.
	}

	return pBlob;
}


bool GfxStreamTest::streamLoopWrapperTest(std::ostream& output)
{
	Blob_p pBlob = _createTestStream();

	char * pOutputBuffer = new char[pBlob->size()+10000];			// Some bytes margin, just in case.
	char * pOutputWrite = pOutputBuffer;
	
	
	
	const int loopBufferSize = GfxStream::c_maxBlockSize+2;
	
	char loopBuffer[loopBufferSize];
	
	char * pBufferWrite = loopBuffer;
	char * pBufferRead = loopBuffer;
		
	auto pStreamLoopWrapper = StreamLoopWrapper::create(loopBuffer, loopBuffer+loopBufferSize,
														   [&pBufferWrite] () { return pBufferWrite; },
														   [&pBufferRead] (const void * pReadPos) { pBufferRead = (char *) pReadPos; } );

	auto pStreamWriter = StreamWriter::create([&pOutputWrite](int nBytes, const void * pBytes) {
		memcpy( pOutputWrite, pBytes, nBytes), pOutputWrite += nBytes;
		
	} );
	
	auto pStreamPump = StreamPump::create( {pStreamLoopWrapper, pStreamLoopWrapper->output}, {pStreamWriter, pStreamWriter->input} );
	
	
	char * pBlobRead = (char *) pBlob->begin();
	char * pBlobEnd = (char *) pBlob->end();
	
	while( pBlobRead < pBlobEnd )
	{
		// Neither writing nor reading getting anywhere would loop forever. That
		// is what a chunk too big for the buffer, or a stream in a format we no
		// longer read, comes down to.

		const char * pBlobReadBefore = pBlobRead;
		const char * pBufferReadBefore = pBufferRead;

		int leftToRead = pBlobEnd - pBlobRead;

		if( pBufferRead <= pBufferWrite )
		{
			int endSpace = loopBuffer + loopBufferSize - pBufferWrite;
			if( pBufferRead < loopBuffer + 2 )
				endSpace -= 2;
			
			if( endSpace > 0 )
			{
				int amount = std::min( endSpace, leftToRead );
				memcpy( pBufferWrite, pBlobRead, amount );
				pBufferWrite += amount;
				pBlobRead += amount;
				leftToRead -= amount;
				
				if( pBufferWrite == loopBuffer + loopBufferSize )
					pBufferWrite = loopBuffer;
			}
		}
		

		if( pBufferRead > pBufferWrite )
		{
			int amount = std::min(int(pBufferRead - pBufferWrite - 2), leftToRead);
			if( amount > 0 )
			{
				memcpy( pBufferWrite, pBlobRead, amount );
				pBufferWrite += amount;
				pBlobRead += amount;
				leftToRead -= amount;
			}
		}

		
/*
		* pBufferWrite++ = * pBlobRead++;
		if( pBufferWrite == loopBuffer + loopBufferSize )
			pBufferWrite = loopBuffer;
*/

		pStreamPump->pumpAll();

		TEST_ASSERT( pBlobRead != pBlobReadBefore || pBufferRead != pBufferReadBefore );
	}

	
	int newSize = pOutputWrite - pOutputBuffer;
	
	TEST_ASSERT( newSize == pBlob->size() );
	
	char * pOrg = (char *) pBlob->data();
	char * pCopy = (char *) pOutputBuffer;
	
	for( int i = 0 ; i < pBlob->size() ; i++ )
	{
		TEST_ASSERT( * pCopy++ == * pOrg++ );
	}

	return true;
}

//____ streamReaderPumpWithOptimizationTest() _________________________________

bool GfxStreamTest::streamReaderPumpWithOptimizationTest(std::ostream& output)
{
	Blob_p pBlob = _createTestStream();

//	Blob_p pBlob = loadBlob("softubehwstream-crash.dat");

	char* pOutputBuffer = new char[pBlob->size() + 10000];			// Some bytes margin, just in case.
	char* pOutputWrite = pOutputBuffer;

	char* pBlobRead = (char *) pBlob->begin();
	char* pBlobReadMax = (char*)pBlob->begin();
	char* pBlobEnd = (char *) pBlob->end();


	auto pStreamReader = StreamReader::create([&pBlobRead, &pBlobReadMax](int nBytes, void* pDest) {

		int bytes = std::min(nBytes, int(pBlobReadMax - pBlobRead));

		memcpy(pDest, pBlobRead, bytes);
		pBlobRead += bytes;
		return bytes;

	});
		

	auto pStreamWriter = StreamWriter::create([&pOutputWrite](int nBytes, const void* pBytes) {
		memcpy(pOutputWrite, pBytes, nBytes), pOutputWrite += nBytes;

		});

	auto pStreamPump = StreamPump::create({ pStreamReader, pStreamReader->output }, { pStreamWriter, pStreamWriter->input });


	while (pBlobRead < pBlobEnd)
	{

		pStreamPump->pumpAll();

		pBlobReadMax += std::min(1024, int(pBlobEnd - pBlobReadMax));

	}

	int newSize = pOutputWrite - pOutputBuffer;

	TEST_ASSERT(newSize == pBlob->size());

	char* pOrg = (char*)pBlob->data();
	char* pCopy = (char*)pOutputBuffer;

	for (int i = 0; i < pBlob->size(); i++)
	{
		TEST_ASSERT(*pCopy++ == *pOrg++);
	}


	return true;
}
