#include "testsuite.h"
#include <wg_blurbrush.h>

// Palette based sources with palettes of different sizes and capacities. The left half is
// drawn from the palette source, the right half from a BGRA source with the same pixels,
// so the two halves should be identical. The A8 variants draw onto an Alpha_8 canvas,
// which is then blitted tinted onto the main canvas.

class PaletteCapacityTests : public TestSuite
{
public:
	PaletteCapacityTests()
	{
		name = "PaletteCapacityTests";

		addTest("Palette8", &PaletteCapacityTests::palette8);
		addTest("Palette8Capacity16", &PaletteCapacityTests::palette8Capacity16);
		addTest("Palette5sRGB", &PaletteCapacityTests::palette5sRGB);
		addTest("Palette256", &PaletteCapacityTests::palette256);
		addTest("Palette8OnA8Canvas", &PaletteCapacityTests::palette8A8);
		addTest("Palette8Capacity16OnA8Canvas", &PaletteCapacityTests::palette8Capacity16A8);
		addTest("Palette5sRGBOnA8Canvas", &PaletteCapacityTests::palette5sRGBA8);
		addTest("Palette256OnA8Canvas", &PaletteCapacityTests::palette256A8);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		struct Setup { int size; int capacity; ColorSpace colorSpace; };
		Setup setups[c_nbSetups] = { { 8, 8, ColorSpace::Linear }, { 8, 16, ColorSpace::Linear },
									 { 5, 5, ColorSpace::sRGB }, { 256, 256, ColorSpace::Linear } };

		auto pFactory = pDevice->surfaceFactory();

		for (int s = 0; s < c_nbSetups; s++)
		{
			const Setup& setup = setups[s];

			Color8 palette[256];
			for (int i = 0; i < setup.size; i++)
				palette[i] = Color8(uint8_t(i * 37 + 10), uint8_t(220 - i * 25), uint8_t((i * 70) & 0xFF), uint8_t(i % 2 ? 255 : 90 + (i * 15) % 160));

			uint8_t indices[c_srcSize * c_srcSize];
			Color8 pixels[c_srcSize * c_srcSize];

			for (int y = 0; y < c_srcSize; y++)
			{
				for (int x = 0; x < c_srcSize; x++)
				{
					uint8_t index = uint8_t(((x / 3) + (y / 2) * 3 + x * y) % setup.size);
					indices[y * c_srcSize + x] = index;
					pixels[y * c_srcSize + x] = palette[index];
				}
			}


			for (int m = 0; m < 2; m++)
			{
				SampleMethod method = m == 0 ? SampleMethod::Nearest : SampleMethod::Bilinear;

				m_pIndexed[s][m] = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = PixelFormat::Index_8, _.colorSpace = setup.colorSpace,
																_.sampleMethod = method, _.palette = palette, _.paletteSize = setup.size, _.paletteCapacity = setup.capacity),
														   indices, PixelFormat::Index_8, c_srcSize, palette, setup.size);

				m_pDirect[s][m] = pFactory->createSurface(WGBP(Surface, _.size = { c_srcSize, c_srcSize }, _.format = PixelFormat::ARGB_8, _.colorSpace = setup.colorSpace,
																_.sampleMethod = method),
														  (uint8_t*)pixels, PixelFormat::ARGB_8, c_srcSize * 4);
			}
		}

		m_pA8Canvas = pFactory->createSurface(WGBP(Surface, _.size = canvas.size() / 64, _.format = PixelFormat::Alpha_8, _.canvas = true, _.sampleMethod = SampleMethod::Nearest));

		m_pTint = Tint::create(HiColor(Color8(255, 255, 255, 255)), HiColor(Color8(255, 64, 0, 200)), { 0.f, 0.f }, { 1.f, 1.f });
		m_pBrush = Blurbrush::create();
		return true;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int s = 0; s < c_nbSetups; s++)
		{
			for (int m = 0; m < 2; m++)
			{
				m_pIndexed[s][m] = nullptr;
				m_pDirect[s][m] = nullptr;
			}
		}
		m_pA8Canvas = nullptr;
		m_pTint = nullptr;
		m_pBrush = nullptr;
		return true;
	}

	bool palette8(GfxDevice * pDevice, const RectSPX& canvas) { return _test(pDevice, canvas, 0, false); }
	bool palette8Capacity16(GfxDevice * pDevice, const RectSPX& canvas) { return _test(pDevice, canvas, 1, false); }
	bool palette5sRGB(GfxDevice * pDevice, const RectSPX& canvas) { return _test(pDevice, canvas, 2, false); }
	bool palette256(GfxDevice * pDevice, const RectSPX& canvas) { return _test(pDevice, canvas, 3, false); }
	bool palette8A8(GfxDevice * pDevice, const RectSPX& canvas) { return _test(pDevice, canvas, 0, true); }
	bool palette8Capacity16A8(GfxDevice * pDevice, const RectSPX& canvas) { return _test(pDevice, canvas, 1, true); }
	bool palette5sRGBA8(GfxDevice * pDevice, const RectSPX& canvas) { return _test(pDevice, canvas, 2, true); }
	bool palette256A8(GfxDevice * pDevice, const RectSPX& canvas) { return _test(pDevice, canvas, 3, true); }

private:

	bool _test(GfxDevice * pDevice, const RectSPX& canvas, int setup, bool bA8Canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(canvas, HiColor(Color8(40, 40, 40, 255)));
		pDevice->setBlendMode(BlendMode::Blend);

		if (bA8Canvas)
		{
			pDevice->beginCanvasUpdate(m_pA8Canvas);
			pDevice->setBlendMode(BlendMode::Replace);
			pDevice->fill(HiColor(Color8(0, 0, 0, 30)));
			pDevice->setBlendMode(BlendMode::Blend);
		}

		_drawOps(pDevice, canvas, m_pIndexed[setup][0], m_pIndexed[setup][1], 0);
		_drawOps(pDevice, canvas, m_pDirect[setup][0], m_pDirect[setup][1], 64 * 256);

		if (bA8Canvas)
		{
			pDevice->endCanvasUpdate();
			pDevice->setTintColor(HiColor(Color8(255, 220, 120, 255)));
			pDevice->setBlitSource(m_pA8Canvas);
			pDevice->blit({ 0, 0 });
			pDevice->clearTintColor();
		}
		return true;
	}

	void _drawOps(GfxDevice * pDevice, const RectSPX& canvas, Surface * pNearest, Surface * pBilinear, spx ofsX)
	{
		const spx col1 = ofsX + 64 * 4, col2 = ofsX + 64 * 130;

		// Row 1: stretch.

		pDevice->setBlitSource(pNearest);
		pDevice->stretchBlit({ col1, 64 * 4, 64 * 120, 64 * 100 });
		pDevice->setBlitSource(pBilinear);
		pDevice->stretchBlit({ col2, 64 * 4, 64 * 120, 64 * 100 });

		// Row 2: rotScale, which is a ClipBlit.

		pDevice->setBlitSource(pNearest);
		pDevice->rotScaleBlit({ col1, 64 * 112, 64 * 120, 64 * 120 }, 30.f, 5.f);
		pDevice->setBlitSource(pBilinear);
		pDevice->rotScaleBlit({ col2, 64 * 112, 64 * 120, 64 * 120 }, 30.f, 5.f);

		// Row 3: tinted stretch.

		pDevice->setTint(canvas, m_pTint);
		pDevice->setBlitSource(pNearest);
		pDevice->stretchBlit({ col1, 64 * 240, 64 * 120, 64 * 100 });
		pDevice->setBlitSource(pBilinear);
		pDevice->stretchBlit({ col2, 64 * 240, 64 * 120, 64 * 100 });
		pDevice->clearTint();

		// Row 4: blit, blur and tinted blur, all 1:1.

		pDevice->setBlitSource(pNearest);
		pDevice->blit({ col1, 64 * 360 });
		pDevice->setBlurbrush(m_pBrush);
		pDevice->blur({ col1 + 64 * 40, 64 * 360 });
		pDevice->setTint(canvas, m_pTint);
		pDevice->blur({ col1 + 64 * 80, 64 * 360 });
		pDevice->blit({ col1 + 64 * 120, 64 * 360 });
		pDevice->clearTint();
	}

	static const int c_srcSize = 16;
	static const int c_nbSetups = 4;

	Surface_p	m_pIndexed[c_nbSetups][2];		// Nearest, bilinear.
	Surface_p	m_pDirect[c_nbSetups][2];
	Surface_p	m_pA8Canvas;
	Tint_p		m_pTint;
	Blurbrush_p	m_pBrush;
};
