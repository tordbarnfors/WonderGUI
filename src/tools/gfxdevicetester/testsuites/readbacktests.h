#include "testsuite.h"

// Reading back canvases that were just rendered to: Alpha_8 canvases of different sizes
// and buffered canvases, which keep a copy of their pixels that must be refreshed.
//
// Each canvas is rendered twice, so stale pixels from the first round would show. Then it
// is read back with pushPixels() and the copy is drawn to the right of the canvas itself,
// both blown up 3x. The two should be identical. The square to the far right is green if
// alpha() returns the expected value for a pixel in the second round, red if not.

class ReadbackTests : public TestSuite
{
public:
	ReadbackTests()
	{
		name = "ReadbackTests";

		addTest("Readback", &ReadbackTests::readback);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		auto pFactory = pDevice->surfaceFactory();

		for (int i = 0; i < c_nbCases; i++)
		{
			const Case& c = c_cases[i];
			m_pCanvases[i] = pFactory->createSurface(WGBP(Surface, _.size = { c.w, c.h }, _.format = c.format, _.canvas = true,
														  _.buffered = c.bBuffered, _.sampleMethod = SampleMethod::Nearest));
		}
		return true;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < c_nbCases; i++)
		{
			m_pCanvases[i] = nullptr;
			m_pCopies[i] = nullptr;
		}
		return true;
	}

	bool readback(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pFactory = pDevice->surfaceFactory();

		bool bAlphaOk[c_nbCases];

		for (int i = 0; i < c_nbCases; i++)
		{
			const Case& c = c_cases[i];
			Surface * pCanvas = m_pCanvases[i];

			for (int round = 0; round < 2; round++)
			{
				pDevice->beginCanvasUpdate(pCanvas);
				pDevice->setBlendMode(BlendMode::Replace);
				pDevice->fill(HiColor(Color8(10, 20, 30, 40)));
				pDevice->fill(RectSPX(64 * (1 + round), 64, 64 * (c.w / 2), 64 * (c.h / 2)), HiColor(Color8(200, 150, 100, uint8_t(100 + round * 100))));
				pDevice->fill(RectSPX(64 * (c.w - 2), 64 * (c.h - 2), 64, 64), HiColor(Color8(50, 250, 50, 255)));
				pDevice->setBlendMode(BlendMode::Blend);
				pDevice->endCanvasUpdate();
			}

			// Copy what we read back into a new surface.
			// The buffer's format can differ from the one we asked for, since a backend
			// may store a format it lacks in a wider one, like BGR_8 as BGRX_8 on DX12.

			auto buffer = pCanvas->allocPixelBuffer({ 0, 0, c.w, c.h });
			pCanvas->pushPixels(buffer, { 0, 0, c.w, c.h });
			m_pCopies[i] = pFactory->createSurface(WGBP(Surface, _.size = { c.w, c.h }, _.format = c.format, _.sampleMethod = SampleMethod::Nearest),
												   buffer.pixels, buffer.format, buffer.pitch);
			pCanvas->freePixelBuffer(buffer);

			// Pixel (2,1) is inside the second fill of the second round.

			int alpha = pCanvas->alpha({ 64 * 2, 64 * 1 });
			int expected = Util::pixelFormatToDescription(c.format).A_mask == 0 ? 4096 : 200 * 4096 / 255;
			bAlphaOk[i] = std::abs(alpha - expected) <= 32;
		}

		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(canvas, HiColor(Color8(60, 60, 90, 255)));
		pDevice->setBlendMode(BlendMode::Blend);

		spx y = 64 * 8;
		for (int i = 0; i < c_nbCases; i++)
		{
			const Case& c = c_cases[i];
			SizeSPX size = SizeSPX(c.w * 3, c.h * 3) * 64;

			pDevice->setBlitSource(m_pCanvases[i]);
			pDevice->stretchBlit({ 64 * 8, y, size });
			pDevice->setBlitSource(m_pCopies[i]);
			pDevice->stretchBlit({ 64 * 216, y, size });

			spx marker = std::min(spx(64 * 24), size.h);
			pDevice->fill({ 64 * 440, y, marker, marker }, bAlphaOk[i] ? HiColor(Color::Green) : HiColor(Color::Red));

			y += size.h + 64 * 8;
		}
		return true;
	}

private:

	struct Case { PixelFormat format; int w, h; bool bBuffered; };

	static const int c_nbCases = 6;
	static constexpr Case c_cases[c_nbCases] = { { PixelFormat::Alpha_8, 64, 32, false }, { PixelFormat::Alpha_8, 61, 37, false },
												 { PixelFormat::Alpha_8, 3, 5, false }, { PixelFormat::BGRA_8_linear, 61, 15, true },
												 { PixelFormat::BGR_8_linear, 61, 15, true }, { PixelFormat::BGRA_8_linear, 61, 15, false } };

	Surface_p	m_pCanvases[c_nbCases];
	Surface_p	m_pCopies[c_nbCases];
};
