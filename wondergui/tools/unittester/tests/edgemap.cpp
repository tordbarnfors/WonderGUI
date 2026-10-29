#include "edgemap.h"

#include <wondergui.h>

#include <cstdlib>
#include <vector>

using namespace wg;

// Tests of the Edgemap base class, which don't depend on any backend.

namespace
{
	// Minimal Edgemap that gives access to the samples it stores.

	class TestEdgemap : public Edgemap
	{
	public:
		TestEdgemap(const Blueprint& bp) : Edgemap(bp) {}

		spx		sample(int edge, int sample) const { return m_pSamples[sample * (m_nbSegments - 1) + edge]; }

	protected:
		void	_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) override {}
		void	_colorsUpdated(int beginSegment, int endSegment) override {}
	};

	const int c_segments = 4;
	const SizeI c_size = { 6, 8 };						// 7 samples per edge, 512 spx high so float samples are exact.

	// Value given to sample of edge, in spx from the top.

	spx value(int edge, int sample) { return (edge * 10 + sample) * 8; }
}

//____ constructor ____________________________________________________________

EdgemapTest::EdgemapTest()
{
	ADD_TEST(importSamplesTest);
	ADD_TEST(importSamplesPitchTest);
}

EdgemapTest::~EdgemapTest()
{
}

//____ importSamplesTest() ____________________________________________________
// Importing part of the edges and samples, the source starts with sample sampleBegin of edge
// edgeBegin, whether samples are spx or float, and samples outside the range are left as they were.

bool EdgemapTest::importSamplesTest(std::ostream& output)
{
	const int edgeBegin = 1, edgeEnd = 3, sampleBegin = 2, sampleEnd = 5;
	const spx height = c_size.h * 64;

	std::vector<spx> spxSource;
	std::vector<float> floatSource;

	for (int edge = edgeBegin; edge < edgeEnd; edge++)
		for (int sample = sampleBegin; sample < sampleEnd; sample++)
		{
			spxSource.push_back(value(edge, sample));
			floatSource.push_back(value(edge, sample) / float(height));
		}

	TestEdgemap spxMap({ .segments = c_segments, .size = c_size });
	TestEdgemap floatMap({ .segments = c_segments, .size = c_size });

	TEST_ASSERT(spxMap.importSamples(SampleOrigo::Top, spxSource.data(), edgeBegin, edgeEnd, sampleBegin, sampleEnd));
	TEST_ASSERT(floatMap.importSamples(SampleOrigo::Top, floatSource.data(), edgeBegin, edgeEnd, sampleBegin, sampleEnd));

	for (int edge = 0; edge < c_segments - 1; edge++)
		for (int sample = 0; sample < spxMap.samples(); sample++)
		{
			bool bImported = edge >= edgeBegin && edge < edgeEnd && sample >= sampleBegin && sample < sampleEnd;
			spx expected = bImported ? value(edge, sample) : height;		// Edges start at the bottom.

			TEST_ASSERT(spxMap.sample(edge, sample) == expected);
			TEST_ASSERT(floatMap.sample(edge, sample) == expected);
		}

	// Same through Edgemap::convertSamples(), from the bottom.

	std::vector<float> fromBottom;
	for (float f : floatSource)
		fromBottom.push_back(1.f - f);

	TestEdgemap bottomMap({ .segments = c_segments, .size = c_size });
	TEST_ASSERT(bottomMap.importSamples(SampleOrigo::Bottom, fromBottom.data(), edgeBegin, edgeEnd, sampleBegin, sampleEnd));

	for (int edge = edgeBegin; edge < edgeEnd; edge++)
		for (int sample = sampleBegin; sample < sampleEnd; sample++)
			TEST_ASSERT(std::abs(bottomMap.sample(edge, sample) - value(edge, sample)) <= 1);

	TEST_ASSERT(Edgemap::convertSample(0.25f, SampleOrigo::Top, height) == height / 4);
	TEST_ASSERT(Edgemap::convertSample(0.25f, SampleOrigo::Bottom, height) == height * 3 / 4);

	return true;
}

//____ importSamplesPitchTest() _______________________________________________
// Edge and sample pitch are steps in the source, starting from sample sampleBegin of edge edgeBegin.

bool EdgemapTest::importSamplesPitchTest(std::ostream& output)
{
	const int edgeBegin = 1, edgeEnd = 3, sampleBegin = 3, sampleEnd = 7;
	const int nEdges = edgeEnd - edgeBegin;
	const spx height = c_size.h * 64;

	// Interleaved: all edges of a sample before the next sample, as the stream sends them.

	std::vector<spx> spxSource;
	std::vector<float> floatSource;

	for (int sample = sampleBegin; sample < sampleEnd; sample++)
		for (int edge = edgeBegin; edge < edgeEnd; edge++)
		{
			spxSource.push_back(value(edge, sample));
			floatSource.push_back(value(edge, sample) / float(height));
		}

	TestEdgemap spxMap({ .segments = c_segments, .size = c_size });
	TestEdgemap floatMap({ .segments = c_segments, .size = c_size });

	TEST_ASSERT(spxMap.importSamples(SampleOrigo::Top, spxSource.data(), edgeBegin, edgeEnd, sampleBegin, sampleEnd, 1, nEdges));
	TEST_ASSERT(floatMap.importSamples(SampleOrigo::Top, floatSource.data(), edgeBegin, edgeEnd, sampleBegin, sampleEnd, 1, nEdges));

	for (int edge = edgeBegin; edge < edgeEnd; edge++)
		for (int sample = sampleBegin; sample < sampleEnd; sample++)
		{
			TEST_ASSERT(spxMap.sample(edge, sample) == value(edge, sample));
			TEST_ASSERT(floatMap.sample(edge, sample) == value(edge, sample));
		}

	return true;
}
