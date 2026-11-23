#include "testsuite.h"

// Surfaces created without pixels but with a fill color, in different formats, as canvases
// or not and buffered or not.
//
// Each is drawn blown up 4x, which should show its fill color and nothing else. The square
// to its right is green if the pixels read back with pushPixels() are all the same and
// alpha() returns the alpha of the color, red if not.

class SurfaceFillTests : public TestSuite
{
public:
	SurfaceFillTests()
	{
		name = "SurfaceFillTests";

		addTest("FillColor", &SurfaceFillTests::fillColor);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		auto pFactory = pDevice->surfaceFactory();

		for (int i = 0; i < c_nbCases; i++)
		{
			const Case& c = c_cases[i];
			bool bIndexed = (c.format == PixelFormat::Index_8);

			m_pSurfaces[i] = pFactory->createSurface(WGBP(Surface, _.size = { c_width, c_height }, _.format = c.format, _.canvas = c.bCanvas, _.buffered = c.bBuffered,
														  _.fillColor = HiColor(c.color), _.palette = bIndexed ? c_palette : nullptr, _.paletteSize = bIndexed ? c_paletteSize : 0,
														  _.sampleMethod = SampleMethod::Nearest));
			if (!m_pSurfaces[i])
				return false;
		}
		return true;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < c_nbCases; i++)
			m_pSurfaces[i] = nullptr;

		return true;
	}

	bool fillColor(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(canvas, HiColor(Color8(60, 60, 90, 255)));
		pDevice->setBlendMode(BlendMode::Blend);

		SizeSPX size = SizeSPX(c_width * 4, c_height * 4) * 64;

		spx y = 64 * 8;
		for (int i = 0; i < c_nbCases; i++)
		{
			pDevice->setBlitSource(m_pSurfaces[i]);
			pDevice->stretchBlit({ 64 * 8, y, size });

			pDevice->fill({ 64 * 16 + size.w, y, size.h, size.h }, _readsAsFilled(i) ? HiColor(Color::Green) : HiColor(Color::Red));

			y += size.h + 64 * 8;
		}
		return true;
	}

private:

	bool _readsAsFilled(int i)
	{
		const Case& c = c_cases[i];
		Surface * pSurface = m_pSurfaces[i];

		// The format of the buffer can differ from the one we asked for, since a backend may
		// store a format it lacks in a wider one.

		auto buffer = pSurface->allocPixelBuffer({ 0, 0, c_width, c_height });
		pSurface->pushPixels(buffer, { 0, 0, c_width, c_height });

		int pixelBytes = Util::pixelFormatToDescription(buffer.format, buffer.bigEndian).bits / 8;

		bool bSame = (buffer.pixels != nullptr && pixelBytes > 0);

		for (int y = 0; y < c_height && bSame; y++)
		{
			for (int x = 0; x < c_width && bSame; x++)
				bSame = (memcmp(buffer.pixels + y * buffer.pitch + x * pixelBytes, buffer.pixels, pixelBytes) == 0);
		}

		pSurface->freePixelBuffer(buffer);

		int alpha = pSurface->alpha({ 64 * 3, 64 * 2 });
		int expected = Util::pixelFormatToDescription(c.format).A_mask == 0 && c.format != PixelFormat::Index_8 ? 4096 : c.color.a * 4096 / 255;

		return bSame && std::abs(alpha - expected) <= 32;
	}

	struct Case { PixelFormat format; bool bCanvas; bool bBuffered; Color8 color; };

	static const int c_width = 16;
	static const int c_height = 8;

	static const int c_paletteSize = 4;
	static constexpr Color8 c_palette[c_paletteSize] = { Color8(0, 0, 0, 255), Color8(255, 0, 0, 255), Color8(40, 160, 220, 255), Color8(255, 255, 255, 255) };

	static const int c_nbCases = 7;
	static constexpr Case c_cases[c_nbCases] = { { PixelFormat::ARGB_8, false, false, Color8(200, 100, 50, 128) },
												 { PixelFormat::ARGB_8, false, true, Color8(200, 100, 50, 128) },
												 { PixelFormat::ARGB_8, true, false, Color8(50, 200, 100, 128) },
												 { PixelFormat::ARGB_8, true, true, Color8(50, 200, 100, 255) },
												 { PixelFormat::XRGB_8, true, false, Color8(220, 180, 40, 255) },
												 { PixelFormat::Alpha_8, false, false, Color8(255, 255, 255, 160) },
												 { PixelFormat::Index_8, false, false, Color8(40, 160, 220, 255) } };

	Surface_p	m_pSurfaces[c_nbCases];
};
