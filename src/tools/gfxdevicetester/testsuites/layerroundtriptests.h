#include "testsuite.h"

#include <cmath>

#include <wg_canvaslayers.h>

// Draws the same scene twice: directly onto the canvas (left half) and onto a transparent
// layer that is then blended onto the canvas (right half). With correct alpha compositing
// (BlendMode::Blend being Porter-Duff 'over') the two halves are identical.
//
// The scene has semi-transparent content overlapping other semi-transparent content within
// the layer: fills with subpixel edges, anti-aliased lines, an edgemap, blits with alpha,
// a rotated bilinear blit with transparent surroundings and a Tint with transparency.

class LayerRoundTripTests : public TestSuite
{
public:
	LayerRoundTripTests()
	{
		name = "LayerRoundTripTests";

		addTest("ViaSurface", &LayerRoundTripTests::viaSurface);
		addTest("ViaCanvasLayer", &LayerRoundTripTests::viaCanvasLayer);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		m_pLogo = pAppAPI->loadSurface("resources/logo-200.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Bilinear });
		m_pShadow = pAppAPI->loadSurface("resources/shadow.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Bilinear });
		if (!m_pLogo || !m_pShadow)
			return false;

		SizeI canvasSize = canvas.size() / 64;
		SizeI halfSize = { canvasSize.w / 2, canvasSize.h };

		m_pLayerSurface = pDevice->surfaceFactory()->createSurface(WGBP(Surface,
			_.size = halfSize,
			_.format = PixelFormat::ARGB_8,
			_.canvas = true));

		m_pLayeredCanvas = pDevice->surfaceFactory()->createSurface(WGBP(Surface,
			_.size = canvasSize,
			_.format = PixelFormat::ARGB_8,
			_.canvas = true));

		// One ARGB_8 layer above the base layer, blended onto it the default way.

		m_pCanvasLayers = CanvasLayers::create(WGBP(CanvasLayers,
			_.baseLayer = 0,
			_.layers = { { .format = PixelFormat::ARGB_8 } }));

		// Edgemap with semi-transparent segments.

		const int nEdges = 3;
		spx samples[nEdges][c_edgemapWidth + 1];

		for (int edge = 0; edge < nEdges; edge++)
			for (int spl = 0; spl <= c_edgemapWidth; spl++)
				samples[edge][spl] = int((20 + edge * 30 + std::sin(spl / 15.0 + edge * 2) * 14) * 64);

		HiColor colors[4] = { HiColor::Transparent, HiColor(4096, 0, 0, 1536), HiColor(0, 4096, 0, 3072), HiColor(0, 0, 4096, 1024) };

		m_pEdgemap = pDevice->edgemapFactory()->createEdgemap(WGBP(Edgemap,
			_.size = { c_edgemapWidth, c_edgemapHeight },
			_.segments = 4,
			_.colors = colors),
			SampleOrigo::Top, samples[0], nEdges, c_edgemapWidth + 1);

		// Tint going from opaque to transparent.

		m_pTint = Tint::create(HiColor(4096, 4096, 0, 4096), HiColor(0, 4096, 4096, 0), { 0.f, 0.f }, { 1.f, 1.f });

		return m_pLayerSurface && m_pLayeredCanvas && m_pCanvasLayers && m_pEdgemap;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pLogo = nullptr;
		m_pShadow = nullptr;
		m_pLayerSurface = nullptr;
		m_pLayeredCanvas = nullptr;
		m_pCanvasLayers = nullptr;
		m_pEdgemap = nullptr;
		m_pTint = nullptr;
		return true;
	}

	// Scene drawn onto an offscreen surface with transparent background, which is then blended
	// onto the canvas.

	bool viaSurface(GfxDevice * pDevice, const RectSPX& canvas)
	{
		drawBackground(pDevice, canvas);
		drawScene(pDevice, { 0,0 });

		pDevice->beginCanvasUpdate(m_pLayerSurface);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor::Transparent);
		drawScene(pDevice, { 0,0 });
		pDevice->endCanvasUpdate();

		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->setBlitSource(m_pLayerSurface);
		pDevice->blit({ canvas.w / 2, 0 });
		return true;
	}

	// Scene drawn onto a layer of CanvasLayers, which the GfxDevice blends onto the base layer.

	bool viaCanvasLayer(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->beginCanvasUpdate(m_pLayeredCanvas, 0, nullptr, m_pCanvasLayers);

		pDevice->setRenderLayer(0);
		drawBackground(pDevice, canvas);
		drawScene(pDevice, { 0,0 });

		pDevice->setRenderLayer(1);
		drawScene(pDevice, { canvas.w / 2, 0 });

		pDevice->endCanvasUpdate();

		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pLayeredCanvas);
		pDevice->blit({ 0,0 });
		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

private:

	// Opaque background in stripes of different colors, over the whole canvas.

	void drawBackground(GfxDevice * pDevice, const RectSPX& canvas)
	{
		HiColor stripes[4] = { Color8(40, 40, 60), Color8(200, 200, 200), Color8(120, 30, 90), Color8(30, 120, 60) };

		pDevice->setBlendMode(BlendMode::Replace);

		int stripeHeight = canvas.h / 8;
		for (int i = 0; i < 8; i++)
			pDevice->fill(RectSPX(canvas.x, canvas.y + i * stripeHeight, canvas.w, stripeHeight), stripes[i % 4]);

		pDevice->setBlendMode(BlendMode::Blend);
	}

	// The scene, within 256x512 pixels from ofs.

	void drawScene(GfxDevice * pDevice, CoordSPX ofs)
	{
		pDevice->setBlendMode(BlendMode::Blend);

		// Overlapping semi-transparent fills with subpixel edges.

		pDevice->fill(RectSPX(ofs.x + 10 * 64 + 21, ofs.y + 10 * 64 + 40, 150 * 64 + 13, 90 * 64 + 7), HiColor(4096, 2048, 0, 2048));
		pDevice->fill(RectSPX(ofs.x + 80 * 64 + 7, ofs.y + 50 * 64 + 50, 160 * 64 + 30, 80 * 64 + 20), HiColor(0, 1024, 4096, 2867));

		// Anti-aliased lines, opaque and semi-transparent, crossing each other.

		for (int i = 0; i < 6; i++)
		{
			pDevice->drawLine(ofs + CoordSPX(10 + i * 40, 150) * 64, ofs + CoordSPX(240 - i * 30, 240) * 64, HiColor::White, 2 * 64 + i * 32);
			pDevice->drawLine(ofs + CoordSPX(10, 160 + i * 14) * 64, ofs + CoordSPX(245, 170 + i * 10) * 64, HiColor(4096, 0, 2048, 2048), 3 * 64);
		}

		// Edgemap with semi-transparent segments.

		pDevice->drawEdgemap(ofs + CoordSPX(10, 250) * 64, m_pEdgemap);

		// Blits with alpha: an image with anti-aliased edges, a soft shadow and a tinted copy.

		pDevice->setBlitSource(m_pShadow);
		pDevice->stretchBlit(RectSPX(ofs.x + 20 * 64, ofs.y + 340 * 64, 220 * 64, 160 * 64));

		pDevice->setBlitSource(m_pLogo);
		pDevice->blit(ofs + CoordSPX(28, 330) * 64);

		pDevice->setTintColor(HiColor(4096, 4096, 4096, 2048));
		pDevice->blit(ofs + CoordSPX(50, 360) * 64);
		pDevice->setTintColor(HiColor::White);

		// Rotated bilinear blit, with transparent surroundings.

		pDevice->rotScaleBlit(RectSPX(ofs.x + 130 * 64, ofs.y + 380 * 64, 120 * 64, 120 * 64), 25.f, 0.5f);

		// Fill with a Tint going from opaque to transparent.

		RectSPX tintRect(ofs.x + 140 * 64, ofs.y + 250 * 64, 100 * 64, 70 * 64);
		pDevice->setTint(tintRect, m_pTint);
		pDevice->fill(tintRect, HiColor::White);
		pDevice->clearTint();
	}

	static const int c_edgemapWidth = 120;
	static const int c_edgemapHeight = 90;

	Surface_p		m_pLogo;
	Surface_p		m_pShadow;
	Surface_p		m_pLayerSurface;
	Surface_p		m_pLayeredCanvas;
	CanvasLayers_p	m_pCanvasLayers;
	Edgemap_p		m_pEdgemap;
	Tint_p			m_pTint;
};
