#include "testsuite.h"
#include <wg_blurbrush.h>

// Where pixels are sampled from: nearest and bilinear stretches at exact and odd scales,
// edges of rotated ClipBlits from sources with and without alpha, and blur tap positions.
// Small sources are blown up with nearest sampling so single pixel differences are visible.

class SamplingTests : public TestSuite
{
public:
	SamplingTests()
	{
		name = "SamplingTests";

		addTest("StretchNearestExactScales", &SamplingTests::stretchNearestExact);
		addTest("StretchNearestOddScales", &SamplingTests::stretchNearestOdd);
		addTest("StretchBilinear", &SamplingTests::stretchBilinear);
		addTest("RotScaleNearestAlpha", &SamplingTests::rotScaleNearestAlpha);
		addTest("RotScaleBilinearAlpha", &SamplingTests::rotScaleBilinearAlpha);
		addTest("RotScaleNearestRGBX", &SamplingTests::rotScaleNearestRGBX);
		addTest("RotScaleBilinearRGBX", &SamplingTests::rotScaleBilinearRGBX);
		addTest("RotScaleBilinearRGBXTinted", &SamplingTests::rotScaleBilinearRGBXTinted);
		addTest("BlurImpulse", &SamplingTests::blurImpulse);
		addTest("BlurPattern", &SamplingTests::blurPattern);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		auto pFactory = pDevice->surfaceFactory();

		// 16x16 pattern: red and green ramps, blue checker and every third pixel half transparent.

		Color8 pattern[c_srcSize * c_srcSize];
		Color8 opaque[c_srcSize * c_srcSize];
		Color8 impulse[c_srcSize * c_srcSize];

		for (int y = 0; y < c_srcSize; y++)
		{
			for (int x = 0; x < c_srcSize; x++)
			{
				int i = y * c_srcSize + x;
				pattern[i] = Color8(uint8_t(x * 16), uint8_t(255 - y * 16), uint8_t(((x ^ y) & 1) ? 230 : 30), uint8_t((x + y) % 3 == 0 ? 128 : 255));
				opaque[i] = pattern[i];
				opaque[i].a = 255;
				impulse[i] = (x == 8 && y == 8) ? Color8(255, 255, 255, 255) : Color8(0, 0, 0, 255);
			}
		}

		for (int i = 0; i < 2; i++)
		{
			SampleMethod method = i == 0 ? SampleMethod::Nearest : SampleMethod::Bilinear;

			m_pPattern[i] = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = PixelFormat::ARGB_8, _.colorSpace = ColorSpace::Linear, _.sampleMethod = method),
													(uint8_t*)pattern, PixelFormat::ARGB_8);
			m_pOpaque[i] = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = PixelFormat::XRGB_8, _.colorSpace = ColorSpace::Linear, _.sampleMethod = method),
												   (uint8_t*)opaque, PixelFormat::ARGB_8);
		}

		m_pImpulse = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = PixelFormat::ARGB_8, _.colorSpace = ColorSpace::Linear, _.sampleMethod = SampleMethod::Nearest),
											 (uint8_t*)impulse, PixelFormat::ARGB_8);

		m_pBlurCanvas = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = PixelFormat::ARGB_8, _.canvas = true, _.sampleMethod = SampleMethod::Nearest));

		m_pBrush = Blurbrush::create();
		return true;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < 2; i++)
		{
			m_pPattern[i] = nullptr;
			m_pOpaque[i] = nullptr;
		}
		m_pImpulse = nullptr;
		m_pBlurCanvas = nullptr;
		m_pBrush = nullptr;
		return true;
	}

	// Stretches by exact integer factors. Every source pixel must cover exactly as many
	// destination pixels as the factor.

	bool stretchNearestExact(GfxDevice * pDevice, const RectSPX& canvas)
	{
		_background(pDevice, canvas);
		pDevice->setBlitSource(m_pPattern[0]);
		pDevice->stretchBlit({ 64 * 8, 64 * 8, 64 * 16 * 2, 64 * 16 * 2 });
		pDevice->stretchBlit({ 64 * 48, 64 * 8, 64 * 16 * 3, 64 * 16 * 3 });
		pDevice->stretchBlit({ 64 * 104, 64 * 8, 64 * 16 * 5, 64 * 16 * 5 });
		pDevice->stretchBlit({ 64 * 192, 64 * 8, 64 * 16 * 7, 64 * 16 * 7 });
		pDevice->stretchBlit({ 64 * 8, 64 * 128, 64 * 16 * 8, 64 * 16 * 3 });
		pDevice->stretchBlit({ 64 * 144, 64 * 128, 64 * 16 * 3, 64 * 16 * 8 });
		pDevice->stretchBlit({ 64 * 200, 64 * 128, 64 * 8 * 6, 64 * 8 * 6 }, { 64 * 4, 64 * 4, 64 * 8, 64 * 8 });
		pDevice->stretchBlit({ 64 * 260, 64 * 128, 64 * 8, 64 * 8 });		// Down by 2.
		return true;
	}

	bool stretchNearestOdd(GfxDevice * pDevice, const RectSPX& canvas)
	{
		_background(pDevice, canvas);
		pDevice->setBlitSource(m_pPattern[0]);
		pDevice->stretchBlit({ 64 * 3, 64 * 5, 64 * 77, 64 * 61 }, { 64 * 1, 64 * 2, 64 * 13, 64 * 11 });
		pDevice->stretchBlit({ 64 * 100, 64 * 5, 64 * 150, 64 * 37 });
		pDevice->stretchBlit({ 64 * 260, 64 * 5, 64 * 23, 64 * 230 });
		pDevice->stretchBlit({ 64 * 3, 64 * 250, 64 * 11, 64 * 9 });
		pDevice->stretchBlit({ 64 * 30, 64 * 250, 64 * 203, 64 * 199 }, { 64 * 3, 64 * 1, 64 * 11, 64 * 14 });
		return true;
	}

	bool stretchBilinear(GfxDevice * pDevice, const RectSPX& canvas)
	{
		_background(pDevice, canvas);
		pDevice->setBlitSource(m_pPattern[1]);
		pDevice->stretchBlit({ 64 * 8, 64 * 8, 64 * 16 * 3, 64 * 16 * 3 });
		pDevice->stretchBlit({ 64 * 64, 64 * 8, 64 * 16 * 8, 64 * 16 * 8 });
		pDevice->stretchBlit({ 64 * 200, 64 * 8, 64 * 77, 64 * 61 }, { 64 * 1, 64 * 2, 64 * 13, 64 * 11 });
		pDevice->stretchBlit({ 64 * 300, 64 * 8, 64 * 11, 64 * 9 });
		return true;
	}

	bool rotScaleNearestAlpha(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _rotScale(pDevice, canvas, m_pPattern[0], false);
	}

	bool rotScaleBilinearAlpha(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _rotScale(pDevice, canvas, m_pPattern[1], false);
	}

	bool rotScaleNearestRGBX(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _rotScale(pDevice, canvas, m_pOpaque[0], false);
	}

	bool rotScaleBilinearRGBX(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _rotScale(pDevice, canvas, m_pOpaque[1], false);
	}

	bool rotScaleBilinearRGBXTinted(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _rotScale(pDevice, canvas, m_pOpaque[1], true);
	}

	// A single white pixel shows where each of the nine blur taps is taken from.
	// They should lie symmetrically around the center.

	bool blurImpulse(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _blur(pDevice, canvas, m_pImpulse);
	}

	bool blurPattern(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _blur(pDevice, canvas, m_pPattern[0]);
	}

private:

	void _background(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(canvas, HiColor(Color8(110, 160, 90, 255)));
		pDevice->setBlendMode(BlendMode::Blend);
	}

	bool _rotScale(GfxDevice * pDevice, const RectSPX& canvas, Surface * pSource, bool bTint)
	{
		_background(pDevice, canvas);
		if (bTint)
			pDevice->setTintColor(HiColor(Color8(255, 255, 255, 128)));
		pDevice->setBlitSource(pSource);
		pDevice->rotScaleBlit(canvas, 32.f, 18.f);		// Not 30 degrees, where samples land exactly on pixel edges.
		if (bTint)
			pDevice->clearTintColor();
		return true;
	}

	bool _blur(GfxDevice * pDevice, const RectSPX& canvas, Surface * pSource)
	{
		pDevice->beginCanvasUpdate(m_pBlurCanvas);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor::Black);
		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->setBlitSource(pSource);
		pDevice->setBlurbrush(m_pBrush);
		pDevice->blur({ 0, 0 });
		pDevice->endCanvasUpdate();

		_background(pDevice, canvas);
		pDevice->setBlitSource(pSource);
		pDevice->stretchBlit({ 64 * 8, 64 * 8, 64 * 16 * 15, 64 * 16 * 15 });
		pDevice->setBlitSource(m_pBlurCanvas);
		pDevice->stretchBlit({ 64 * 256, 64 * 8, 64 * 16 * 15, 64 * 16 * 15 });
		return true;
	}

	static const int c_srcSize = 16;

	Surface_p	m_pPattern[2];		// Nearest, bilinear.
	Surface_p	m_pOpaque[2];		// BGRX, nearest and bilinear.
	Surface_p	m_pImpulse;
	Surface_p	m_pBlurCanvas;
	Blurbrush_p	m_pBrush;
};
