#include "edgemapbox.h"

#include <wondergui.h>
#include <wax_device_gen2.h>
#include <wax_softbackend.h>
#include <wax_softsurface.h>
#include <wax_softedgemapfactory.h>
#include <wax_softkernels_default.h>
#include <wax_referencebackend.h>
#include <wax_edgemapbox.h>

#include <algorithm>
#include <cstring>
#include <functional>
#include <vector>

using namespace wg;
using namespace wax;

// EdgemapBox drawn by SoftBackend, compared to drawRoundedBox() drawn by ReferenceBackend. Canvases
// are ARGB_8 in linear color space, so differences are in coverage.

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
	BorderSPX border(double t, double r, double b, double l) { return { spx(t * 64), spx(r * 64), spx(b * 64), spx(l * 64) }; }
	CornersSPX corners(double tl, double tr, double br, double bl) { return { spx(tl * 64), spx(tr * 64), spx(br * 64), spx(bl * 64) }; }

	struct Case
	{
		const char*		pName;
		RoundedBoxSpec	spec;
	};

	std::vector<Case> cases()
	{
		std::vector<Case> out;
		RoundedBoxSpec s;

		s = {}; s.rect = pixels(10, 10, 140, 140); s.radius = corners(20, 20, 20, 20); s.outline = border(2, 2, 2, 2); s.fillColor = Color::Red; s.outlineColor = Color::White;
		out.push_back({ "Box on pixels", s });

		s = {}; s.rect = pixels(10.3, 20.6, 130.25, 110.5); s.radius = corners(8, 30, 0, 18); s.outline = border(1.5, 3, 0.5, 6); s.fillColor = Color::Red; s.outlineColor = Color::White;
		out.push_back({ "Box between pixels", s });

		s = {}; s.rect = pixels(10.4, 10.7, 120.25, 90.5); s.outline = border(2.25, 2.25, 2.25, 2.25); s.fillColor = Color::Blue; s.outlineColor = Color::Yellow;
		out.push_back({ "Square corners", s });

		s = {}; s.rect = pixels(10, 60, 140, 40); s.radius = corners(100, 100, 100, 100); s.outline = border(3, 3, 3, 3); s.fillColor = Color::Green; s.outlineColor = Color::White;
		out.push_back({ "Pill", s });

		s = {}; s.rect = pixels(10.5, 10.5, 139, 139); s.radius = corners(15, 35, 15, 35); s.outline = border(20, 20, 20, 20); s.fillColor = Color::Gray; s.outlineColor = Color::Cyan;
		out.push_back({ "Thick outline", s });

		return out;
	}
}

//____ constructor ____________________________________________________________

EdgemapBoxTest::EdgemapBoxTest()
{
	ADD_TEST(accuracyTest);
	ADD_TEST(updateTest);
	ADD_TEST(nothingDrawnTest);
}

EdgemapBoxTest::~EdgemapBoxTest()
{
}

//____ accuracyTest() _________________________________________________________
// Boxes look like drawRoundedBox() draws them. Edges are straight between samples, so pixels
// along the corners differ a little, more at their tops and bottoms. Horizontal edges between
// pixels are moved to pixel lines, so pixels along them can differ by half their coverage.

bool EdgemapBoxTest::accuracyTest(std::ostream& output)
{
	auto pSoft = createSoftDevice();
	auto pRef = GfxDeviceGen2::create(ReferenceBackend::create());
	auto pFactory = SoftEdgemapFactory::create();

	bool bOk = true;

	for (auto& c : cases())
	{
		auto pBox = EdgemapBox::create(WGBP(EdgemapBox, _.factory = pFactory), c.spec);
		TEST_ASSERT(pBox != nullptr);

		auto edge = render(pSoft, [&](GfxDevice* p) { pBox->draw(p); });
		auto ref = render(pRef, [&](GfxDevice* p) { p->drawRoundedBox(c.spec); });

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

		if (worst > 4096 * 9 / 16 || over16th > 600)
		{
			output << c.pName << ": largest difference " << worst << " of 4096, " << over16th << " pixels differ by more than 1/16." << std::endl;
			bOk = false;
		}
	}

	return bOk;
}

//____ updateTest() ___________________________________________________________
// An updated box is drawn as if created with the new spec, and keeps its edgemaps when only
// colors change.

bool EdgemapBoxTest::updateTest(std::ostream& output)
{
	auto pSoft = createSoftDevice();
	auto pFactory = SoftEdgemapFactory::create();

	RoundedBoxSpec spec = cases()[1].spec;

	auto pBox = EdgemapBox::create(WGBP(EdgemapBox, _.factory = pFactory), spec);
	TEST_ASSERT(pBox != nullptr);

	RoundedBoxSpec resized = spec;
	resized.rect.w -= 300;
	resized.radius[1] = 10 * 64;

	RoundedBoxSpec recolored = resized;
	recolored.fillColor = Color::Blue;
	recolored.outlineColor = Color::Yellow;

	for (auto* pSpec : { &resized, &recolored })
	{
		std::vector<Edgemap*> before;
		for (auto& piece : pBox->pieces())
			before.push_back(piece.pEdgemap);

		TEST_ASSERT(pBox->update(*pSpec));
		auto pFresh = EdgemapBox::create(WGBP(EdgemapBox, _.factory = pFactory), *pSpec);

		auto updated = render(pSoft, [&](GfxDevice* p) { pBox->draw(p); });
		auto fresh = render(pSoft, [&](GfxDevice* p) { pFresh->draw(p); });
		TEST_ASSERT(updated == fresh);
		TEST_ASSERT(pBox->spec().rect == pSpec->rect);

		if (pSpec == &recolored)
		{
			TEST_ASSERT(pBox->pieces().size() == before.size());
			for (size_t i = 0; i < before.size(); i++)
				TEST_ASSERT(pBox->pieces()[i].pEdgemap == before[i]);
		}
	}

	return true;
}

//____ nothingDrawnTest() _____________________________________________________
// A box with nothing to draw has no pieces.

bool EdgemapBoxTest::nothingDrawnTest(std::ostream& output)
{
	auto pFactory = SoftEdgemapFactory::create();

	RoundedBoxSpec spec;
	spec.rect = pixels(10, 10, 50, 30);
	spec.radius = corners(5, 5, 5, 5);
	spec.outline = border(2, 2, 2, 2);
	spec.outlineColor = HiColor::Undefined;

	auto pBox = EdgemapBox::create(WGBP(EdgemapBox, _.factory = pFactory), spec);
	TEST_ASSERT(pBox != nullptr && pBox->pieces().empty());

	spec.rect = {};
	spec.fillColor = Color::Red;
	TEST_ASSERT(pBox->update(spec));
	TEST_ASSERT(pBox->pieces().empty());

	return true;
}
