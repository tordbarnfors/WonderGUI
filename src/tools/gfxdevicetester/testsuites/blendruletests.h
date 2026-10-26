#include "testsuite.h"

#include <cmath>

// Tests of the rules for blending and transparency, as defined in the guide under
// "Blending and transparency":
//
// - Effect modes (Add, Subtract, Multiply, Invert, Min, Max) work on the light of the canvas, its
//   premultiplied color, and use the alpha of the source as the strength of the effect. Subtract,
//   Multiply and Min keep the alpha of the canvas, Add, Invert and Max raise it to hold their light.
// - Morph cross-fades premultiplied colors, so a transparent side just fades out.
// - With a paint mode, a partially covered pixel is drawn as if the canvas had a much higher
//   resolution, the shape was drawn there without anti-aliasing and the subpixels were then
//   averaged. With an effect mode, the coverage scales the strength of the effect.
// - Outside the source of a ClipBlit is transparent, which has no effect in any effect mode.
// - An Alpha_8 canvas is a one-channel image, on which effect modes do arithmetic with the
//   alpha of the source as the value.
// - A canvas without alpha doesn't store the alpha of the result.

class BlendRuleTests : public TestSuite
{
public:
	BlendRuleTests()
	{
		name = "BlendRuleTests";

		addTest("EffectsOnGlass", &BlendRuleTests::effectsOnGlass);
		addTest("EffectFills", &BlendRuleTests::effectFills);
		addTest("MorphTransparency", &BlendRuleTests::morphTransparency);
		addTest("AlphaCanvasEffects", &BlendRuleTests::alphaCanvasEffects);
		addTest("ClipBlitOutside", &BlendRuleTests::clipBlitOutside);
		addTest("CanvasWithoutAlpha", &BlendRuleTests::canvasWithoutAlpha);
		addTest("PaintEdges", &BlendRuleTests::paintEdges);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		m_pLogo = pAppAPI->loadSurface("resources/logo-200.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Bilinear });
		if (!m_pLogo)
			return false;

		SizeI canvasSize = canvas.size() / 64;

		m_pLayer = pDevice->surfaceFactory()->createSurface(WGBP(Surface,
			_.size = canvasSize,
			_.format = PixelFormat::ARGB_8,
			_.canvas = true));

		m_pAlphaCanvas = pDevice->surfaceFactory()->createSurface(WGBP(Surface,
			_.size = canvasSize,
			_.format = PixelFormat::Alpha_8,
			_.canvas = true));

		m_pOpaqueCanvas = pDevice->surfaceFactory()->createSurface(WGBP(Surface,
			_.size = canvasSize,
			_.format = PixelFormat::XRGB_8,
			_.canvas = true));

		// Alpha-only surface with a soft round spot, like the light of a lamp.

		const int spotSize = 128;
		uint8_t spot[spotSize * spotSize];

		for (int y = 0; y < spotSize; y++)
			for (int x = 0; x < spotSize; x++)
			{
				double dx = (x + 0.5 - spotSize / 2) / (spotSize / 2);
				double dy = (y + 0.5 - spotSize / 2) / (spotSize / 2);
				double light = std::clamp(1.0 - std::sqrt(dx * dx + dy * dy), 0.0, 1.0);
				spot[y * spotSize + x] = uint8_t(std::lround(light * 255));
			}

		m_pSpot = pDevice->surfaceFactory()->createSurface(WGBP(Surface,
			_.size = { spotSize, spotSize },
			_.format = PixelFormat::Alpha_8,
			_.sampleMethod = SampleMethod::Bilinear),
			spot, PixelFormat::Alpha_8, spotSize);

		// Edgemap with a transparent, a semi-transparent and an opaque segment.

		const int nEdges = 2;
		spx samples[nEdges][c_edgemapWidth + 1];

		for (int edge = 0; edge < nEdges; edge++)
			for (int spl = 0; spl <= c_edgemapWidth; spl++)
				samples[edge][spl] = int((10 + edge * 22 + std::sin(spl / 9.0 + edge * 2) * 8) * 64);

		HiColor colors[3] = { HiColor::Transparent, HiColor(4096, 1024, 0, 2400), HiColor(0, 2048, 4096, 4096) };

		m_pEdgemap = pDevice->edgemapFactory()->createEdgemap(WGBP(Edgemap,
			_.size = { c_edgemapWidth, c_edgemapHeight },
			_.segments = 3,
			_.colors = colors),
			SampleOrigo::Top, samples[0], nEdges, c_edgemapWidth + 1);

		return m_pLayer && m_pAlphaCanvas && m_pOpaqueCanvas && m_pSpot && m_pEdgemap;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pLogo = nullptr;
		m_pLayer = nullptr;
		m_pAlphaCanvas = nullptr;
		m_pOpaqueCanvas = nullptr;
		m_pSpot = nullptr;
		m_pEdgemap = nullptr;
		return true;
	}

	// A transparent layer with paint of increasing alpha in each row, lit by a soft spot of
	// light in each of the effect modes. Effects change the light of the paint. Subtract,
	// Multiply and Min keep its alpha, so transparent parts stay transparent, while Add,
	// Invert and Max make the glass glow where its paint can't hold their light. The layer
	// is shown on a checkerboard.

	bool effectsOnGlass(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->beginCanvasUpdate(m_pLayer);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor::Transparent);

		spx rowHeight = canvas.h / c_nbEffectModes;
		spx columnWidth = canvas.w / 5;

		for (int row = 0; row < c_nbEffectModes; row++)
		{
			RectSPX rowRect(0, row * rowHeight, canvas.w, rowHeight);

			pDevice->setBlendMode(BlendMode::Blend);
			for (int column = 0; column < 5; column++)
				pDevice->fill(RectSPX(column * columnWidth, rowRect.y + 4 * 64, columnWidth, rowHeight - 8 * 64), HiColor(Color8(220, 120, 50, uint8_t(column * 255 / 4))));

			pDevice->setBlendMode(c_effectModes[row]);
			pDevice->setTintColor(effectColor(row));
			pDevice->setBlitSource(m_pSpot);
			pDevice->stretchBlit(rowRect);
			pDevice->setTintColor(HiColor::White);
		}

		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->endCanvasUpdate();

		drawCheckerboard(pDevice, canvas);
		pDevice->setBlitSource(m_pLayer);
		pDevice->blit({ 0,0 });
		return true;
	}

	// Fills, lines and an edgemap with semi-transparent colors and subpixel edges, in each
	// of the effect modes, onto an opaque background of stripes.

	bool effectFills(GfxDevice * pDevice, const RectSPX& canvas)
	{
		drawStripes(pDevice, canvas);

		spx rowHeight = canvas.h / c_nbEffectModes;

		for (int row = 0; row < c_nbEffectModes; row++)
		{
			spx y = row * rowHeight;
			HiColor color = effectColor(row);

			pDevice->setBlendMode(c_effectModes[row]);

			for (int i = 0; i < 3; i++)
			{
				pDevice->fill(RectSPX(10 * 64 + 21 + i * 70 * 64, y + 6 * 64 + 40, 60 * 64 + 13, rowHeight - 14 * 64 - 15), color.withAlpha(4096 - i * 1400));
			}

			pDevice->drawLine({ 225 * 64, y + 8 * 64 }, { 330 * 64, y + rowHeight - 8 * 64 }, color.withAlpha(2600), 5 * 64 + 20);
			pDevice->drawLine({ 225 * 64, y + rowHeight - 8 * 64 }, { 330 * 64, y + 8 * 64 }, color, 2 * 64 + 40);

			pDevice->drawEdgemap({ 345 * 64, y + 4 * 64 }, m_pEdgemap);
		}

		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

	// Morph onto a transparent layer with paint: a transparent source makes the paint fade
	// out without darkening, opaque and semi-transparent sources fade in. The layer is shown
	// on a checkerboard.

	bool morphTransparency(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->beginCanvasUpdate(m_pLayer);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor::Transparent);

		spx columnWidth = canvas.w / 4;
		spx rowHeight = canvas.h / 4;

		// Paint in the left half of each column, opaque at top and semi-transparent below.

		for (int column = 0; column < 4; column++)
			for (int row = 0; row < 4; row++)
				pDevice->fill(RectSPX(column * columnWidth, row * rowHeight, columnWidth / 2, rowHeight), HiColor(Color8(40, 200, 90, uint8_t(255 - row * 60))));

		// Morph over the whole column with a different source in each row, column by column
		// with increasing morph factor.

		HiColor sources[4] = { HiColor::Transparent, HiColor(Color8(230, 40, 160, 255)), HiColor(Color8(230, 40, 160, 100)), HiColor(Color8(255, 255, 255, 30)) };

		pDevice->setBlendMode(BlendMode::Morph);

		for (int column = 0; column < 4; column++)
		{
			pDevice->setMorphFactor(0.2f + column * 0.2f);

			for (int row = 0; row < 4; row++)
				pDevice->fill(RectSPX(column * columnWidth + 10 * 64 + 17, row * rowHeight + 10 * 64 + 33, columnWidth - 20 * 64, rowHeight - 20 * 64), sources[row]);
		}

		pDevice->setMorphFactor(0.5f);
		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->endCanvasUpdate();

		drawCheckerboard(pDevice, canvas);
		pDevice->setBlitSource(m_pLayer);
		pDevice->blit({ 0,0 });
		return true;
	}

	// An Alpha_8 canvas, half at 50% and half at 100%, with fills, lines and an edgemap in all
	// blend modes. Then shown tinted onto an opaque background.

	bool alphaCanvasEffects(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->beginCanvasUpdate(m_pAlphaCanvas);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(RectSPX(0, 0, canvas.w / 2, canvas.h), HiColor(0, 0, 0, 2048));
		pDevice->fill(RectSPX(canvas.w / 2, 0, canvas.w - canvas.w / 2, canvas.h), HiColor(0, 0, 0, 4096));

		BlendMode modes[c_nbEffectModes + 3] = { BlendMode::Replace, BlendMode::Blend, BlendMode::Morph,
												 BlendMode::Add, BlendMode::Subtract, BlendMode::Multiply, BlendMode::Invert, BlendMode::Min, BlendMode::Max };

		int nRows = c_nbEffectModes + 3;
		spx rowHeight = canvas.h / nRows;

		for (int row = 0; row < nRows; row++)
		{
			spx y = row * rowHeight;

			pDevice->setBlendMode(modes[row]);
			pDevice->fill(RectSPX(10 * 64 + 21, y + 6 * 64 + 40, canvas.w - 20 * 64 - 50, rowHeight / 2 - 8 * 64 + 7), HiColor(0, 0, 0, 1600));
			pDevice->fill(RectSPX(40 * 64 + 50, y + rowHeight / 2 + 2 * 64 + 10, 150 * 64 + 13, rowHeight / 2 - 8 * 64 - 30), HiColor(0, 0, 0, 3500));
			pDevice->drawLine({ 200 * 64, y + rowHeight - 4 * 64 }, { 330 * 64, y + rowHeight / 2 + 4 * 64 }, HiColor(0, 0, 0, 3000), 3 * 64 + 20);
			pDevice->drawEdgemap({ 360 * 64, y + rowHeight / 2 - 10 * 64 }, m_pEdgemap);
		}

		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->endCanvasUpdate();

		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor(Color8(30, 40, 70)));
		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->setTintColor(HiColor(Color8(255, 230, 160)));
		pDevice->setBlitSource(m_pAlphaCanvas);
		pDevice->blit({ 0,0 });
		pDevice->setTintColor(HiColor::White);
		return true;
	}

	// Rotated and scaled-down blits in each effect mode. Outside the source is transparent,
	// so the background outside the image is unchanged in all modes, Min and Multiply included.

	bool clipBlitOutside(GfxDevice * pDevice, const RectSPX& canvas)
	{
		drawStripes(pDevice, canvas);

		spx cellWidth = canvas.w / 3;
		spx cellHeight = canvas.h / 2;

		pDevice->setBlitSource(m_pLogo);

		for (int i = 0; i < c_nbEffectModes; i++)
		{
			RectSPX cell((i % 3) * cellWidth, (i / 3) * cellHeight, cellWidth, cellHeight);

			pDevice->setBlendMode(c_effectModes[i]);
			pDevice->rotScaleBlit(cell, 20.f + i * 13, 0.6f);
		}

		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

	// Paint modes onto an XRGB_8 canvas, which doesn't store the alpha of the result. Edges
	// are still mixed with what was there.

	bool canvasWithoutAlpha(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->beginCanvasUpdate(m_pOpaqueCanvas);
		drawStripes(pDevice, canvas);

		spx rowHeight = canvas.h / 4;
		HiColor colors[4] = { HiColor(Color8(250, 200, 40, 255)), HiColor(Color8(250, 200, 40, 128)), HiColor(Color8(250, 200, 40, 40)), HiColor::Transparent };

		BlendMode modes[3] = { BlendMode::Replace, BlendMode::Morph, BlendMode::BlendFixedColor };

		pDevice->setFixedBlendColor(HiColor(Color8(20, 20, 200, 128)));

		for (int row = 0; row < 4; row++)
		{
			for (int i = 0; i < 3; i++)
			{
				pDevice->setBlendMode(modes[i]);
				pDevice->fill(RectSPX(10 * 64 + 21 + i * 120 * 64, row * rowHeight + 10 * 64 + 40, 100 * 64 + 13, rowHeight - 20 * 64 - 15), colors[row]);
			}
		}

		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->drawEdgemap({ 380 * 64, 30 * 64 }, m_pEdgemap);
		pDevice->drawLine({ 370 * 64, 120 * 64 }, { 500 * 64, 300 * 64 }, HiColor(Color8(255, 255, 255, 128)), 6 * 64 + 20);

		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->endCanvasUpdate();

		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pOpaqueCanvas);
		pDevice->blit({ 0,0 });
		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

	// Edges of paint modes that don't look at the canvas (Replace, BlendFixedColor and Morph),
	// on a layer that is half opaque red and half transparent. The covered part of an edge pixel
	// is replaced, the rest keeps what was there. The layer is shown on a checkerboard.

	bool paintEdges(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->beginCanvasUpdate(m_pLayer);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor::Transparent);
		pDevice->fill(RectSPX(0, 0, canvas.w, canvas.h / 2), HiColor(Color8(220, 30, 30)));

		pDevice->setFixedBlendColor(HiColor(Color8(20, 20, 200, 128)));

		BlendMode modes[3] = { BlendMode::Replace, BlendMode::BlendFixedColor, BlendMode::Morph };
		HiColor colors[3] = { HiColor(Color8(0, 0, 0, 128)), HiColor(Color8(250, 200, 40, 200)), HiColor::Transparent };

		spx columnWidth = canvas.w / 3;

		for (int i = 0; i < 3; i++)
		{
			spx x = i * columnWidth;

			pDevice->setBlendMode(modes[i]);

			// Fills covering a quarter, half and three quarters of the pixels at their edges.

			for (int j = 0; j < 3; j++)
			{
				spx y = canvas.h / 2 - 150 * 64 + j * 100 * 64;
				pDevice->fill(RectSPX(x + 20 * 64 + 16 * (j + 1), y + 16 * (j + 1), 60 * 64, 80 * 64 + 16), colors[i]);
			}

			pDevice->drawLine({ x + 100 * 64, canvas.h / 2 - 160 * 64 }, { x + columnWidth - 10 * 64, canvas.h / 2 + 160 * 64 }, colors[i], 4 * 64 + 20);
			pDevice->drawEdgemap({ x + 10 * 64, canvas.h / 2 - 22 * 64 }, m_pEdgemap);
		}

		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->endCanvasUpdate();

		drawCheckerboard(pDevice, canvas);
		pDevice->setBlitSource(m_pLayer);
		pDevice->blit({ 0,0 });
		return true;
	}

private:

	// Opaque vertical stripes from black to white and in a few colors.

	void drawStripes(GfxDevice * pDevice, const RectSPX& canvas)
	{
		HiColor stripes[8] = { Color8(0, 0, 0), Color8(255, 255, 255), Color8(128, 128, 128), Color8(200, 60, 40),
							   Color8(40, 160, 60), Color8(40, 70, 200), Color8(230, 220, 60), Color8(60, 30, 80) };

		pDevice->setBlendMode(BlendMode::Replace);

		spx stripeWidth = canvas.w / 8;
		for (int i = 0; i < 8; i++)
			pDevice->fill(RectSPX(canvas.x + i * stripeWidth, canvas.y, (i == 7) ? canvas.w - 7 * stripeWidth : stripeWidth, canvas.h), stripes[i]);

		pDevice->setBlendMode(BlendMode::Blend);
	}

	// Gray checkerboard that shows what is transparent.

	void drawCheckerboard(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(canvas, HiColor(Color8(90, 90, 90)));

		spx square = 16 * 64;
		for (spx y = 0; y < canvas.h; y += square)
			for (spx x = (y / square) % 2 ? square : 0; x < canvas.w; x += square * 2)
				pDevice->fill(RectSPX(canvas.x + x, canvas.y + y, square, square), HiColor(Color8(160, 160, 160)));

		pDevice->setBlendMode(BlendMode::Blend);
	}

	static const int c_edgemapWidth = 140;
	static const int c_edgemapHeight = 44;

	static const int c_nbEffectModes = 6;

	inline static const BlendMode c_effectModes[c_nbEffectModes] = { BlendMode::Add, BlendMode::Subtract, BlendMode::Multiply, BlendMode::Invert, BlendMode::Min, BlendMode::Max };

	// Color for each effect mode. Not a static table, since converting Color8 to HiColor needs
	// tables that GfxBase sets up at runtime.

	static HiColor effectColor(int mode)
	{
		const Color8 colors[c_nbEffectModes] = { Color8(120, 180, 255), Color8(120, 180, 255), Color8(80, 200, 255),
												 Color8(255, 255, 255), Color8(30, 40, 120), Color8(255, 240, 120) };
		return HiColor(colors[mode]);
	}

	Surface_p		m_pLogo;
	Surface_p		m_pLayer;
	Surface_p		m_pAlphaCanvas;
	Surface_p		m_pOpaqueCanvas;
	Surface_p		m_pSpot;
	Edgemap_p		m_pEdgemap;
};
