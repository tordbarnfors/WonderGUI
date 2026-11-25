#include "testsuite.h"

// Reading back canvases that were just rendered to: Alpha_8 canvases of different sizes
// and canvases with and without a restorer.
//
// Each canvas is rendered twice, so stale pixels from the first round would show. Then it
// is read back with pushPixels() and the copy is drawn to the right of the canvas itself,
// both blown up 3x. The two should be identical. The square to the far right is green if
// alpha() returns the expected value for a pixel in the second round, red if not.
//
// "Rectangles" writes part of each canvas with the CPU after it has been rendered to, with
// pixels that have alpha, through a part of a pixel buffer. Then the right half of the canvas
// is read back into a part of a pixel buffer and drawn to the right. The square is green if
// alpha() returns what the CPU wrote. Only what is pulled is written and only what is pushed
// is read, as a pixel buffer may be the pixels of the surface or memory of its own.
//
// "Restorer" does the same with surfaces that are no canvases and have a restorer, which a
// GPU backend keeps no pixels of: they are created with a fill color, a rectangle of each is
// written through a pixel buffer of just that, and the right half is read back. The square is
// green if alpha() returns the alpha of what was written where it was, and that of the fill
// color beside it. Their restorer fills them again, so the test gives the same after the
// device of a GPU backend has been replaced.
//
// "WriteAll" and "WriteAllRestorer" are for timing: a surface that has all its pixels written
// each time, as one showing a stream has, without and with restorer. Mind that the tester runs
// a test many times in one canvas update, where a backend that collects what was written sends
// it once for all of them, so the two can't be told apart by their times on such a backend.

class ReadbackTests : public TestSuite
{
public:
	ReadbackTests()
	{
		name = "ReadbackTests";

		addTest("Readback", &ReadbackTests::readback);
		addTest("Rectangles", &ReadbackTests::rectangles);
		addTest("Restorer", &ReadbackTests::restorer);
		addTest("WriteAll", &ReadbackTests::writeAll);
		addTest("WriteAllRestorer", &ReadbackTests::writeAllRestorer);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		auto pFactory = pDevice->surfaceFactory();

		for (int i = 0; i < c_nbCases; i++)
		{
			const Case& c = c_cases[i];
			m_pCanvases[i] = pFactory->createSurface(WGBP(Surface, _.size = { c.w, c.h }, _.format = c.format, _.colorSpace = c.colorSpace, _.canvas = true,
														  _.restorer = c.bRestorer ? Surface::declineRestore : nullptr, _.sampleMethod = SampleMethod::Nearest));
		}

		for (int i = 0; i < c_nbRestorerCases; i++)
		{
			const RestorerCase& c = c_restorerCases[i];
			m_pRestorerSurfaces[i] = pFactory->createSurface(WGBP(Surface, _.size = { c_restorerWidth, c_restorerHeight }, _.format = c.format, _.colorSpace = c.colorSpace,
																  _.fillColor = HiColor(c.fill), _.identity = i, _.restorer = _restoreFill, _.sampleMethod = SampleMethod::Nearest));
		}

		for (int i = 0; i < 2; i++)
		{
			m_pStreamSurfaces[i] = pFactory->createSurface(WGBP(Surface, _.size = { c_streamSize, c_streamSize }, _.format = PixelFormat::XRGB_8, _.colorSpace = ColorSpace::Linear,
																_.fillColor = HiColor::Black, _.restorer = i == 1 ? Surface::declineRestore : nullptr,
																_.sampleMethod = SampleMethod::Nearest));
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

		for (int i = 0; i < c_nbRestorerCases; i++)
			m_pRestorerSurfaces[i] = nullptr;

		for (int i = 0; i < 2; i++)
			m_pStreamSurfaces[i] = nullptr;

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
			// may store a format it lacks in a wider one, like RGB_565 as XRGB_8 on DX12 and Metal.

			auto buffer = pCanvas->allocPixelBuffer({ 0, 0, c.w, c.h });
			pCanvas->pushPixels(buffer, { 0, 0, c.w, c.h });
			m_pCopies[i] = pFactory->createSurface(WGBP(Surface, _.size = { c.w, c.h }, _.format = c.format, _.colorSpace = buffer.colorSpace, _.sampleMethod = SampleMethod::Nearest),
												   buffer.pixels, wax::Util::pixelFormatToDescription(buffer.format, buffer.bigEndian), buffer.pitch);
			pCanvas->freePixelBuffer(buffer);

			// Pixel (2,1) is inside the second fill of the second round.

			int alpha = pCanvas->alpha({ 64 * 2, 64 * 1 });
			int expected = wax::Util::pixelFormatToDescription(c.format).A_mask == 0 ? 4096 : 200 * 4096 / 255;
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

	bool rectangles(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pFactory = pDevice->surfaceFactory();

		bool bAlphaOk[c_nbCases];

		for (int i = 0; i < c_nbCases; i++)
		{
			const Case& c = c_cases[i];
			Surface * pCanvas = m_pCanvases[i];

			pDevice->beginCanvasUpdate(pCanvas);
			pDevice->setBlendMode(BlendMode::Replace);
			pDevice->fill(HiColor(Color8(10, 20, 30, 40)));
			pDevice->fill(RectSPX(64, 64, 64 * (c.w / 2), 64 * (c.h / 2)), HiColor(Color8(200, 150, 100, 200)));
			pDevice->setBlendMode(BlendMode::Blend);
			pDevice->endCanvasUpdate();

			// Written by the CPU: all but the edges, through a pixel buffer of everything.

			RectI written = { 1, 1, c.w - 2, c.h - 2 };

			auto buffer = pCanvas->allocPixelBuffer({ 0, 0, c.w, c.h });
			wax::PixelTools::fillBitmap(buffer.pixels, buffer.format, buffer.colorSpace, buffer.bigEndian, buffer.pitch, written,
										HiColor(Color8(250, 100, 50, 128)), buffer.palette);
			pCanvas->pullPixels(buffer, written);
			pCanvas->freePixelBuffer(buffer);

			// Read back: the right half, into that part of a pixel buffer of everything.

			RectI read = { c.w / 2, 0, c.w - c.w / 2, c.h };

			buffer = pCanvas->allocPixelBuffer({ 0, 0, c.w, c.h });
			pCanvas->pushPixels(buffer, read);
			m_pCopies[i] = pFactory->createSurface(WGBP(Surface, _.size = { read.w, read.h }, _.format = c.format, _.colorSpace = buffer.colorSpace, _.sampleMethod = SampleMethod::Nearest),
												   buffer.pixels + read.y * buffer.pitch + read.x * (wax::Util::pixelFormatToDescription(buffer.format).bits / 8),
												   wax::Util::pixelFormatToDescription(buffer.format, buffer.bigEndian), buffer.pitch);
			pCanvas->freePixelBuffer(buffer);

			// Pixel (1,1) is what the CPU wrote.

			int alpha = pCanvas->alpha({ 64 * 1, 64 * 1 });
			int expected = wax::Util::pixelFormatToDescription(c.format).A_mask == 0 ? 4096 : 128 * 4096 / 255;
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
			pDevice->stretchBlit({ 64 * 216, y, SizeSPX((c.w - c.w / 2) * 3, c.h * 3) * 64 });

			spx marker = std::min(spx(64 * 24), size.h);
			pDevice->fill({ 64 * 440, y, marker, marker }, bAlphaOk[i] ? HiColor(Color::Green) : HiColor(Color::Red));

			y += size.h + 64 * 8;
		}
		return true;
	}

	bool restorer(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pFactory = pDevice->surfaceFactory();

		const int w = c_restorerWidth;
		const int h = c_restorerHeight;

		bool bAlphaOk[c_nbRestorerCases];

		for (int i = 0; i < c_nbRestorerCases; i++)
		{
			const RestorerCase& c = c_restorerCases[i];
			Surface * pSurface = m_pRestorerSurfaces[i];

			// Written by the CPU: a rectangle in the right half, through a pixel buffer of just that.

			RectI written = { w / 2 - 6, 2, 24, h - 5 };

			auto buffer = pSurface->allocPixelBuffer(written);
			wax::PixelTools::fillBitmap(buffer.pixels, buffer.format, buffer.colorSpace, buffer.bigEndian, buffer.pitch, { 0, 0, written.w, written.h },
										HiColor(c.write), buffer.palette);
			pSurface->pullPixels(buffer);
			pSurface->freePixelBuffer(buffer);

			// Read back: the right half, into that part of a pixel buffer of everything.

			RectI read = { w / 2, 0, w - w / 2, h };

			buffer = pSurface->allocPixelBuffer({ 0, 0, w, h });
			pSurface->pushPixels(buffer, read);
			m_pRestorerCopies[i] = pFactory->createSurface(WGBP(Surface, _.size = { read.w, read.h }, _.format = c.format, _.colorSpace = buffer.colorSpace, _.sampleMethod = SampleMethod::Nearest),
														   buffer.pixels + read.y * buffer.pitch + read.x * (wax::Util::pixelFormatToDescription(buffer.format).bits / 8),
														   wax::Util::pixelFormatToDescription(buffer.format, buffer.bigEndian), buffer.pitch);
			pSurface->freePixelBuffer(buffer);

			bool bAlpha = wax::Util::pixelFormatToDescription(c.format).A_mask != 0;

			int alphaWritten = pSurface->alpha({ 64 * (written.x + 1), 64 * (written.y + 1) });
			int alphaBeside = pSurface->alpha({ 64 * 1, 64 * 1 });

			bAlphaOk[i] = std::abs(alphaWritten - (bAlpha ? c.write.a * 4096 / 255 : 4096)) <= 32 &&
						  std::abs(alphaBeside - (bAlpha ? c.fill.a * 4096 / 255 : 4096)) <= 32;
		}

		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(canvas, HiColor(Color8(60, 60, 90, 255)));
		pDevice->setBlendMode(BlendMode::Blend);

		SizeSPX size = SizeSPX(w * 3, h * 3) * 64;

		spx y = 64 * 8;
		for (int i = 0; i < c_nbRestorerCases; i++)
		{
			pDevice->setBlitSource(m_pRestorerSurfaces[i]);
			pDevice->stretchBlit({ 64 * 8, y, size });
			pDevice->setBlitSource(m_pRestorerCopies[i]);
			pDevice->stretchBlit({ 64 * 216, y, SizeSPX((w - w / 2) * 3, h * 3) * 64 });

			pDevice->fill({ 64 * 440, y, 64 * 24, 64 * 24 }, bAlphaOk[i] ? HiColor(Color::Green) : HiColor(Color::Red));

			y += size.h + 64 * 8;
		}
		return true;
	}

	bool writeAll(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _writeAll(pDevice, m_pStreamSurfaces[0]);
	}

	bool writeAllRestorer(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return _writeAll(pDevice, m_pStreamSurfaces[1]);
	}

private:

	// Restorer of the surfaces of "Restorer", which have their case as identity.

	static bool _restoreFill(Surface * pSurface)
	{
		pSurface->fill(HiColor(c_restorerCases[pSurface->identity()].fill));
		return true;
	}

	bool _writeAll(GfxDevice * pDevice, Surface * pSurface)
	{
		// Nothing is read, so pushPixels() is left out, as whoever shows a stream would.

		auto buffer = pSurface->allocPixelBuffer();

		int pixelBytes = wax::Util::pixelFormatToDescription(buffer.format).bits / 8;

		for (int y = 0; y < c_streamSize; y++)
			memset(buffer.pixels + y * buffer.pitch, 64 + (y & 127), c_streamSize * pixelBytes);

		pSurface->pullPixels(buffer);
		pSurface->freePixelBuffer(buffer);

		pDevice->setBlitSource(pSurface);
		pDevice->blit({ 64 * 8, 64 * 8 });
		return true;
	}

	struct Case { PixelFormat format; ColorSpace colorSpace; int w, h; bool bRestorer; };

	static const int c_nbCases = 6;
	static constexpr Case c_cases[c_nbCases] = { { PixelFormat::Alpha_8, ColorSpace::Linear, 64, 32, false }, { PixelFormat::Alpha_8, ColorSpace::Linear, 61, 37, false },
												 { PixelFormat::Alpha_8, ColorSpace::Linear, 3, 5, false }, { PixelFormat::ARGB_8, ColorSpace::Linear, 61, 15, true },
												 { PixelFormat::RGB_565, ColorSpace::Linear, 61, 15, true }, { PixelFormat::ARGB_8, ColorSpace::Linear, 61, 15, false } };

	// What the surface is filled with, and what is written to part of it: alpha into what was
	// opaque, opaque into what had alpha, nothing but opaque, and a format without alpha. The
	// last two are sRGB, which has to be read back as it is.

	struct RestorerCase { PixelFormat format; ColorSpace colorSpace; Color8 fill; Color8 write; };

	static const int c_restorerWidth = 61;
	static const int c_restorerHeight = 15;

	static const int c_nbRestorerCases = 4;
	static constexpr RestorerCase c_restorerCases[c_nbRestorerCases] = { { PixelFormat::ARGB_8, ColorSpace::Linear, Color8(40, 80, 160, 255), Color8(250, 100, 50, 128) },
																		 { PixelFormat::ARGB_8, ColorSpace::Linear, Color8(40, 80, 160, 100), Color8(250, 100, 50, 255) },
																		 { PixelFormat::ARGB_8, ColorSpace::sRGB, Color8(40, 80, 160, 255), Color8(250, 100, 50, 255) },
																		 { PixelFormat::XRGB_8, ColorSpace::sRGB, Color8(40, 80, 160, 255), Color8(250, 100, 50, 255) } };

	static const int c_streamSize = 256;

	Surface_p	m_pCanvases[c_nbCases];
	Surface_p	m_pCopies[c_nbCases];

	Surface_p	m_pRestorerSurfaces[c_nbRestorerCases];
	Surface_p	m_pRestorerCopies[c_nbRestorerCases];

	Surface_p	m_pStreamSurfaces[2];			// Without and with restorer.
};
