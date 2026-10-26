#include "testsuite.h"

#include <wg_canvaslayers.h>

// Calls GfxDeviceGen2::flattenLayers() in the middle of a canvas update, without and with CanvasLayers.
//
// Each test draws two rounds of drawing, overlapping each other, twice: onto the left half in one
// canvas update, flattened between the rounds, and onto the right half with each round in a canvas
// update of its own. Flattening renders what has been drawn so far and lets the drawing continue on
// top of it, so the two halves are identical.

class FlattenLayersTests : public TestSuite
{
public:
	FlattenLayersTests()
	{
		name = "FlattenLayersTests";

		addTest("NoCanvasLayers", &FlattenLayersTests::noCanvasLayers);
		addTest("WithCanvasLayers", &FlattenLayersTests::withCanvasLayers);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		// flattenLayers() is specific to GfxDeviceGen2.

		if (!pDevice->isInstanceOf(GfxDeviceGen2::TYPEINFO))
			return false;

		m_pLogo = pAppAPI->loadSurface("resources/logo-200.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Nearest });
		if (!m_pLogo)
			return false;

		SizeI halfSize = { canvas.w / 64 / 2, canvas.h / 64 };

		for (auto& pHalf : m_pHalves)
		{
			pHalf = pDevice->surfaceFactory()->createSurface(WGBP(Surface,
				_.size = halfSize,
				_.format = PixelFormat::ARGB_8,
				_.canvas = true));
		}

		// Layer 1 is blended onto the base layer the default way, layer 2 by a blend function of its own.

		m_pCanvasLayers = CanvasLayers::create(WGBP(CanvasLayers,
			_.baseLayer = 0,
			_.layers = { { .format = PixelFormat::ARGB_8 },
						 { .format = PixelFormat::ARGB_8, .blendFunc = [](GfxDevice* pDevice) {
							 pDevice->setTintColor(HiColor(4096, 4096, 4096, 2867));
							 pDevice->blit({ 0,0 }); } } }));

		return m_pHalves[0] && m_pHalves[1] && m_pCanvasLayers;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pLogo = nullptr;
		m_pHalves[0] = nullptr;
		m_pHalves[1] = nullptr;
		m_pCanvasLayers = nullptr;
		return true;
	}

	bool noCanvasLayers(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pGen2 = static_cast<GfxDeviceGen2*>(pDevice);

		pDevice->beginCanvasUpdate(m_pHalves[0]);
		drawFirstRound(pDevice, false);
		pGen2->flattenLayers();
		drawSecondRound(pDevice, false);
		pDevice->endCanvasUpdate();

		pDevice->beginCanvasUpdate(m_pHalves[1]);
		drawFirstRound(pDevice, false);
		pDevice->endCanvasUpdate();

		pDevice->beginCanvasUpdate(m_pHalves[1]);
		drawSecondRound(pDevice, false);
		pDevice->endCanvasUpdate();

		blitHalves(pDevice, canvas);
		return true;
	}

	bool withCanvasLayers(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pGen2 = static_cast<GfxDeviceGen2*>(pDevice);

		// flattenLayers() leaves the content of the layer canvases, so they are
		// cleared as well, the way a new canvas update starts with them.

		pDevice->beginCanvasUpdate(m_pHalves[0], 0, nullptr, m_pCanvasLayers);
		drawFirstRound(pDevice, true);
		pGen2->flattenLayers();
		pGen2->clearLayers();
		drawSecondRound(pDevice, true);
		pDevice->endCanvasUpdate();

		pDevice->beginCanvasUpdate(m_pHalves[1], 0, nullptr, m_pCanvasLayers);
		drawFirstRound(pDevice, true);
		pDevice->endCanvasUpdate();

		pDevice->beginCanvasUpdate(m_pHalves[1], 0, nullptr, m_pCanvasLayers);
		drawSecondRound(pDevice, true);
		pDevice->endCanvasUpdate();

		blitHalves(pDevice, canvas);
		return true;
	}

private:

	// Opaque background with a semi-transparent fill, a blit and a rotated blit (which adds a transform).
	// With layers, there is also drawing to both layers and opaque drawing to the base layer after
	// them, which still ends up below the layers.
	//
	// Every round sets the states it uses, since flattenLayers() keeps the states while
	// a new canvas update starts with them reset.

	void drawFirstRound(GfxDevice * pDevice, bool bLayers)
	{
		pDevice->setRenderLayer(0);

		pDevice->clearTintColor();
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(Color8(40, 40, 60));

		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->fill(RectSPX(10, 10, 150, 120) * 64, HiColor(4096, 2048, 0, 2048));

		pDevice->setBlitSource(m_pLogo);
		pDevice->blit(CoordSPX(28, 150) * 64);
		pDevice->rotScaleBlit(RectSPX(20, 330, 140, 140) * 64, 25.f, 0.6f);

		if (bLayers)
		{
			pDevice->setRenderLayer(1);
			pDevice->fill(RectSPX(100, 60, 140, 200) * 64, HiColor(0, 4096, 1024, 2048));
			pDevice->setTintColor(HiColor(4096, 4096, 4096, 2048));
			pDevice->blit(CoordSPX(40, 200) * 64);
			pDevice->clearTintColor();

			pDevice->setRenderLayer(2);
			pDevice->fill(RectSPX(60, 250, 180, 60) * 64, HiColor(0, 0, 4096, 3072));

			pDevice->setRenderLayer(0);
			pDevice->fill(RectSPX(0, 100, 256, 30) * 64, Color8(200, 200, 200));
		}
	}

	// Semi-transparent fills and another rotated blit, overlapping the first round.

	void drawSecondRound(GfxDevice * pDevice, bool bLayers)
	{
		pDevice->setRenderLayer(0);

		pDevice->clearTintColor();
		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->fill(RectSPX(50, 40, 180, 100) * 64, HiColor(4096, 4096, 0, 1536));

		pDevice->setBlitSource(m_pLogo);
		pDevice->rotScaleBlit(RectSPX(90, 300, 150, 150) * 64, -40.f, 0.8f);

		if (bLayers)
		{
			pDevice->setRenderLayer(1);
			pDevice->fill(RectSPX(20, 180, 120, 120) * 64, HiColor(4096, 0, 4096, 2048));

			pDevice->setRenderLayer(2);
			pDevice->fill(RectSPX(140, 400, 100, 90) * 64, HiColor(0, 4096, 4096, 2560));

			pDevice->setRenderLayer(0);
			pDevice->blit(CoordSPX(10, 330) * 64);
		}
	}

	void blitHalves(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pHalves[0]);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pHalves[1]);
		pDevice->blit({ canvas.w / 2, 0 });
		pDevice->setBlendMode(BlendMode::Blend);
	}

	Surface_p		m_pLogo;
	Surface_p		m_pHalves[2];			// Flattened between the rounds, followed by a canvas update per round.
	CanvasLayers_p	m_pCanvasLayers;
};
