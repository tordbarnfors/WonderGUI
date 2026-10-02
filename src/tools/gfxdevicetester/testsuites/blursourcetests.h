#include "testsuite.h"
#include <wg_blurbrush.h>

// Blur from every kind of source: BGRA (linear and sRGB), BGRX, Alpha_8 and palette based
// (linear and sRGB). Alpha_8 sources behave like white BGRA. Blur keeps the alpha of the
// center pixel.
//
// Top row shows the sources, the rows below are plain, gradient tinted and tint colored
// blurs, all blown up 4x. Each blur goes to a canvas of its own, since the main canvas
// isn't rendered until all of them are done. The A8 test blurs onto Alpha_8 canvases
// and draws them with an orange tint color.

class BlurSourceTests : public TestSuite
{
public:
	BlurSourceTests()
	{
		name = "BlurSourceTests";

		addTest("BlurSources", &BlurSourceTests::blurSources);
		addTest("BlurSourcesOnA8Canvas", &BlurSourceTests::blurSourcesA8);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		auto pFactory = pDevice->surfaceFactory();

		Color8 palette[8];
		for (int i = 0; i < 8; i++)
			palette[i] = Color8(uint8_t(i * 30 + 10), uint8_t(220 - i * 25), uint8_t((i * 70) & 0xFF), uint8_t(i % 2 ? 255 : 90 + i * 15));

		uint8_t indices[c_srcSize * c_srcSize];
		uint8_t alpha[c_srcSize * c_srcSize];
		Color8 pixels[c_srcSize * c_srcSize];

		for (int y = 0; y < c_srcSize; y++)
		{
			for (int x = 0; x < c_srcSize; x++)
			{
				int i = y * c_srcSize + x;
				indices[i] = uint8_t(((x / 3) + (y / 2) * 3) % 8);
				pixels[i] = palette[indices[i]];
				alpha[i] = pixels[i].a;
			}
		}

		PixelFormat formats[c_nbSources] = { PixelFormat::BGRA_8_linear, PixelFormat::BGRA_8_sRGB, PixelFormat::BGRX_8_linear,
											 PixelFormat::Alpha_8, PixelFormat::Index_8_linear, PixelFormat::Index_8_sRGB };

		for (int s = 0; s < c_nbSources; s++)
		{
			PixelFormat format = formats[s];

			if (format == PixelFormat::Index_8_linear || format == PixelFormat::Index_8_sRGB)
				m_pSources[s] = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = format, _.sampleMethod = SampleMethod::Nearest,
															 _.palette = palette, _.paletteSize = 8), indices, format, c_srcSize, palette, 8);
			else if (format == PixelFormat::Alpha_8)
				m_pSources[s] = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = format, _.sampleMethod = SampleMethod::Nearest),
														alpha, format, c_srcSize);
			else
				m_pSources[s] = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = format, _.sampleMethod = SampleMethod::Nearest),
														(uint8_t*)pixels, format == PixelFormat::BGRA_8_sRGB ? PixelFormat::BGRA_8_sRGB : PixelFormat::BGRA_8_linear, c_srcSize * 4);

			for (int r = 0; r < c_nbRows; r++)
			{
				m_pCanvases[r][s] = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = PixelFormat::BGRA_8, _.canvas = true, _.sampleMethod = SampleMethod::Nearest));
				m_pA8Canvases[r][s] = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = PixelFormat::Alpha_8, _.canvas = true, _.sampleMethod = SampleMethod::Nearest));
			}
		}

		// Separate matrices per channel, so mixed up channels show.

		float red[9] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
		float green[9] = { 0.f, 0.25f, 0.f, 0.25f, 0.f, 0.25f, 0.f, 0.25f, 0.f };
		float blue[9] = { 0.3f, 0.f, 0.f, 0.f, 0.4f, 0.f, 0.f, 0.f, 0.3f };

		m_pBrush = Blurbrush::create(WGBP(Blurbrush, _.red = red, _.green = green, _.blue = blue, _.size = 64 * 3));
		m_pTint = Tint::create(HiColor(Color8(255, 255, 255, 255)), HiColor(Color8(255, 64, 0, 160)), { 0.f, 0.f }, { 1.f, 1.f });
		return true;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int s = 0; s < c_nbSources; s++)
		{
			m_pSources[s] = nullptr;
			for (int r = 0; r < c_nbRows; r++)
			{
				m_pCanvases[r][s] = nullptr;
				m_pA8Canvases[r][s] = nullptr;
			}
		}
		m_pBrush = nullptr;
		m_pTint = nullptr;
		return true;
	}

	bool blurSources(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _test(pDevice, canvas, m_pCanvases);
	}

	bool blurSourcesA8(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _test(pDevice, canvas, m_pA8Canvases);
	}

private:

	bool _test(GfxDevice * pDevice, const RectSPX& canvas, Surface_p (&canvases)[3][6])
	{
		bool bA8 = canvases[0][0]->pixelFormat() == PixelFormat::Alpha_8;

		for (int r = 0; r < c_nbRows; r++)
		{
			for (int s = 0; s < c_nbSources; s++)
			{
				pDevice->beginCanvasUpdate(canvases[r][s]);
				pDevice->setBlendMode(BlendMode::Replace);
				pDevice->fill(HiColor(Color8(40, 40, 40, bA8 ? 30 : 255)));
				pDevice->setBlendMode(BlendMode::Blend);

				if (r == 1)
					pDevice->setTint({ 0, 0, 64 * c_srcSize, 64 * c_srcSize }, m_pTint);
				else if (r == 2)
					pDevice->setTintColor(HiColor(Color8(120, 200, 255, 255)));

				pDevice->setBlitSource(m_pSources[s]);
				pDevice->setBlurbrush(m_pBrush);
				pDevice->blur({ 0, 0 });

				if (r == 1)
					pDevice->clearTint();
				else if (r == 2)
					pDevice->clearTintColor();

				pDevice->endCanvasUpdate();
			}
		}

		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(canvas, HiColor(Color8(20, 20, 60, 255)));
		pDevice->setBlendMode(BlendMode::Blend);

		const spx cell = 64 * 80, size = 64 * c_srcSize * 4;

		for (int s = 0; s < c_nbSources; s++)
		{
			pDevice->setBlitSource(m_pSources[s]);
			pDevice->stretchBlit({ 64 * 8 + s * cell, 64 * 8, size, size });
		}

		if (bA8)
			pDevice->setTintColor(HiColor(Color8(255, 160, 40, 255)));

		for (int r = 0; r < c_nbRows; r++)
		{
			for (int s = 0; s < c_nbSources; s++)
			{
				pDevice->setBlitSource(canvases[r][s]);
				pDevice->stretchBlit({ 64 * 8 + s * cell, 64 * 8 + (r + 1) * cell, size, size });
			}
		}

		if (bA8)
			pDevice->clearTintColor();

		return true;
	}

	static const int c_srcSize = 16;
	static const int c_nbSources = 6;
	static const int c_nbRows = 3;

	Surface_p	m_pSources[c_nbSources];
	Surface_p	m_pCanvases[c_nbRows][c_nbSources];
	Surface_p	m_pA8Canvases[c_nbRows][c_nbSources];
	Blurbrush_p	m_pBrush;
	Tint_p		m_pTint;
};
