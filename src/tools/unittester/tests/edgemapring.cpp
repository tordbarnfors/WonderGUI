#include "edgemapring.h"

#include <wondergui.h>
#include <wg_gfxdevice_gen2.h>
#include <wg_softbackend.h>
#include <wg_softsurface.h>
#include <wg_softedgemapfactory.h>
#include <wg_softkernels_default.h>
#include <wg_referencebackend.h>
#include <wg_edgemapring.h>

#include <algorithm>
#include <cstring>
#include <functional>
#include <vector>

using namespace wg;

// EdgemapRing drawn by SoftBackend, compared to drawRing() drawn by ReferenceBackend. Canvases are
// ARGB_8 in linear color space, so differences are in coverage.

namespace
{
	const SizeI c_canvasSize = { 160, 160 };

	std::vector<uint8_t> render(GfxDevice* pDevice, const std::function<void(GfxDevice*)>& draw)
	{
		auto pCanvas = SoftSurface::create(WGBP(Surface, _.size = c_canvasSize, _.format = PixelFormat::ARGB_8, _.colorSpace = ColorSpace::Linear, _.canvas = true));

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(pCanvas);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor::Black);
		pDevice->setBlendMode(BlendMode::Blend);
		draw(pDevice);
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		std::vector<uint8_t> out(c_canvasSize.w * c_canvasSize.h * 4);
		auto buffer = pCanvas->allocPixelBuffer();
		pCanvas->pushPixels(buffer);
		for (int y = 0; y < c_canvasSize.h; y++)
			std::memcpy(out.data() + y * c_canvasSize.w * 4, buffer.pixels + y * buffer.pitch, c_canvasSize.w * 4);
		pCanvas->freePixelBuffer(buffer);
		return out;
	}

	GfxDeviceGen2_p createSoftDevice()
	{
		auto pBackend = SoftBackend::create();
		addDefaultSoftKernels(pBackend);
		return GfxDeviceGen2::create(pBackend);
	}

	RectSPX pixels(double x, double y, double w, double h) { return { spx(x * 64), spx(y * 64), spx(w * 64), spx(h * 64) }; }

	struct Case
	{
		const char*				pName;
		RingSpec				spec;
		std::vector<RingSlice>	slices;
	};

	std::vector<Case> cases()
	{
		std::vector<RingSlice> seven = { { 0.1f, Color::Red }, { 0.2f, Color::Orange }, { 0.05f, Color::Yellow }, { 0.15f, Color::Green },
										 { 0.25f, Color::Cyan }, { 0.13f, Color::Blue }, { 0.12f, Color::Magenta } };

		std::vector<Case> out;
		RingSpec s;

		s = {}; s.rect = pixels(8, 8, 144, 144); s.outline = 2 * 64; s.outlineColor = Color::White;
		out.push_back({ "Disc", s, { { 1.f, Color::Red } } });

		s = {}; s.rect = pixels(8, 8, 144, 144);
		out.push_back({ "Pie, center on pixel corner", s, seven });

		s = {}; s.rect = pixels(8.3, 8.6, 143.25, 143.25); s.start = 0.1f;
		out.push_back({ "Pie, center between pixels", s, seven });

		s = {}; s.rect = pixels(8.3, 8.6, 143.25, 143.25); s.thickness = 40 * 64; s.outline = 2 * 64; s.outlineColor = Color::White; s.hubColor = Color::Gray;
		out.push_back({ "Donut", s, seven });

		s = {}; s.rect = pixels(20, 20, 120, 120); s.thickness = 12 * 64; s.start = 0.625f; s.backColor = HiColor(4096, 4096, 4096, 1024);
		out.push_back({ "Gauge", s, { { 0.45f, Color::Cyan }, { 0.3f, HiColor::Undefined } } });

		s = {}; s.rect = pixels(10.4, 40.2, 140.5, 80.7); s.thickness = 20 * 64; s.backColor = Color::DarkBlue; s.hubColor = Color::Gray;
		out.push_back({ "Back in wide rect", s, { { 0.3f, Color::Red }, { 0.3f, Color::Orange }, { 0.2f, Color::Yellow } } });

		s = {}; s.rect = pixels(20.5, 20.5, 120, 120); s.bSquare = true; s.outline = 3 * 64; s.outlineColor = Color::White; s.thickness = 30 * 64; s.hubColor = Color::Gray;
		out.push_back({ "Square", s, { { 0.25f, Color::Red }, { 0.25f, Color::Orange }, { 0.25f, Color::Yellow } } });

		return out;
	}
}

//____ constructor ____________________________________________________________

EdgemapRingTest::EdgemapRingTest()
{
	ADD_TEST(accuracyTest);
	ADD_TEST(updateTest);
	ADD_TEST(nothingDrawnTest);
}

EdgemapRingTest::~EdgemapRingTest()
{
}

//____ accuracyTest() _________________________________________________________
// Rings look like drawRing() draws them. Edges are straight between samples, so pixels along
// curves differ a little and a pixel where a line between slices meets a circle can differ by
// up to almost half its coverage.

bool EdgemapRingTest::accuracyTest(std::ostream& output)
{
	auto pSoft = createSoftDevice();
	auto pRef = GfxDeviceGen2::create(ReferenceBackend::create());
	auto pFactory = SoftEdgemapFactory::create();

	bool bOk = true;

	for (auto& c : cases())
	{
		c.spec.nSlices = int(c.slices.size());
		c.spec.pSlices = c.slices.data();

		auto pRing = EdgemapRing::create(WGBP(EdgemapRing, _.factory = pFactory), c.spec);
		TEST_ASSERT(pRing != nullptr);

		auto edge = render(pSoft, [&](GfxDevice* p) { pRing->draw(p); });
		auto ref = render(pRef, [&](GfxDevice* p) { p->drawRing(c.spec); });

		int worst = 0, over16th = 0;
		for (size_t i = 0; i < edge.size(); i += 4)
		{
			int diff = 0;
			for (int ch = 0; ch < 3; ch++)
				diff = std::max(diff, std::abs(HiColor::unpackLinearTab[edge[i + ch]] - HiColor::unpackLinearTab[ref[i + ch]]));

			worst = std::max(worst, diff);
			if (diff > 4096 / 16)
				over16th++;
		}

		if (worst >= 4096 / 2 || over16th > 24)
		{
			output << c.pName << ": largest difference " << worst << " of 4096, " << over16th << " pixels differ by more than 1/16." << std::endl;
			bOk = false;
		}
	}

	return bOk;
}

//____ updateTest() ___________________________________________________________
// An updated ring is drawn as if created with the new spec, and keeps its edgemaps when only
// colors change.

bool EdgemapRingTest::updateTest(std::ostream& output)
{
	auto pSoft = createSoftDevice();
	auto pFactory = SoftEdgemapFactory::create();

	auto all = cases();
	Case& from = all[3];			// Donut.

	std::vector<RingSlice> moved = from.slices;
	moved[0].size = 0.3f;
	moved[2].size = 0.02f;
	moved[4].color = Color::White;

	std::vector<RingSlice> recolored = from.slices;
	for (auto& slice : recolored)
		slice.color = Color::Blue;

	RingSpec spec = from.spec;
	spec.nSlices = int(from.slices.size());
	spec.pSlices = from.slices.data();

	auto pRing = EdgemapRing::create(WGBP(EdgemapRing, _.factory = pFactory), spec);
	TEST_ASSERT(pRing != nullptr);

	for (auto* pSlices : { &moved, &recolored })
	{
		RingSpec newSpec = spec;
		newSpec.pSlices = pSlices->data();

		std::vector<Edgemap*> before;
		for (auto& piece : pRing->pieces())
			before.push_back(piece.pEdgemap);

		TEST_ASSERT(pRing->update(newSpec));
		auto pFresh = EdgemapRing::create(WGBP(EdgemapRing, _.factory = pFactory), newSpec);

		auto updated = render(pSoft, [&](GfxDevice* p) { pRing->draw(p); });
		auto fresh = render(pSoft, [&](GfxDevice* p) { pFresh->draw(p); });
		TEST_ASSERT(updated == fresh);

		TEST_ASSERT(pRing->spec().nSlices == newSpec.nSlices);
		TEST_ASSERT(pRing->spec().pSlices[0].color == (*pSlices)[0].color);

		if (pSlices == &recolored)
		{
			TEST_ASSERT(pRing->pieces().size() == before.size());
			for (size_t i = 0; i < before.size(); i++)
				TEST_ASSERT(pRing->pieces()[i].pEdgemap == before[i]);
		}
	}

	return true;
}

//____ nothingDrawnTest() _____________________________________________________
// A ring with nothing to draw has no pieces.

bool EdgemapRingTest::nothingDrawnTest(std::ostream& output)
{
	auto pFactory = SoftEdgemapFactory::create();

	RingSlice slice = { 1.f, HiColor::Undefined };

	RingSpec spec;
	spec.rect = pixels(10, 10, 50, 50);
	spec.thickness = 10 * 64;
	spec.nSlices = 1;
	spec.pSlices = &slice;

	auto pRing = EdgemapRing::create(WGBP(EdgemapRing, _.factory = pFactory), spec);
	TEST_ASSERT(pRing != nullptr && pRing->pieces().empty());

	spec.rect = {};
	slice.color = Color::Red;
	TEST_ASSERT(pRing->update(spec));
	TEST_ASSERT(pRing->pieces().empty());
	TEST_ASSERT(pRing->bounds().isEmpty());

	return true;
}
