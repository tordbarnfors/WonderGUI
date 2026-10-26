#define _USE_MATH_DEFINES		// For M_PI in MSVC.

#include "linerender.h"

#include <wondergui.h>
#include <wax_gfxdevice_gen2.h>
#include <wax_gfxbackend.h>
#include <wax_backendlogger.h>
#include <wax_softbackend.h>
#include <wax_linearbackend.h>
#include <wax_referencebackend.h>
#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_softedgemapfactory.h>
#include <wax_softkernels_default.h>
#include <wax_softshapes.h>
#include <wax_streambackend.h>
#include <wax_streamplayer.h>
#include <wax_streamencoder.h>
#include <wax_c.h>
#include <wax_c_object.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

using namespace wg;

/*
	Draws lines with caps through GfxDeviceGen2 and the software backends.

	- ReferenceBackend draws them as exact shapes. SoftBackend draws the parts around the ends
	  as exact shapes too and whole columns between them with its line kernels, which estimate
	  the coverage of edge pixels from the middle of each column. That is off where an edge
	  crosses into the next row within a column, by up to 1/8 of a pixel at 45 degrees, so
	  pixels of those columns are allowed to differ more. Lines are recorded as they are drawn
	  to find those columns.
	- LinearBackend draws exactly as SoftBackend.

	Canvases are ARGB_8 in linear color space. Lines are semi-transparent, so that pixels drawn
	twice where the parts of a line meet would show.
*/

namespace
{
	const SizeI c_canvasSize = { 256, 256 };

	const HiColor c_background = HiColor(800, 1500, 2600, 4096);
	const HiColor c_lineColor = HiColor(4096, 3000, 1200, 3000);

	enum class Backend
	{
		Soft,
		Linear,
		Reference
	};

	using Scene = std::function<void(GfxDevice* pDevice)>;
	using Pixels = std::vector<uint8_t>;

	//____ render() ___________________________________________________________
	// Renders a scene on a cleared canvas. LinearBackend renders in two update rectangles.

	Pixels render(Backend backend, const Scene& scene)
	{
		Pixels pixels(c_canvasSize.w * c_canvasSize.h * 4, 0);

		if (backend == Backend::Linear)
		{
			std::vector<uint8_t> buffer(c_canvasSize.w * c_canvasSize.h * 4 * 2);

			auto pBackend = LinearBackend::create(
				[&](CanvasRef ref, int bytes)
				{
					return (void*)buffer.data();
				},
				[&](CanvasRef ref, int nSegments, const LinearBackend::Segment* pSegments)
				{
					for (int i = 0; i < nSegments; i++)
					{
						auto& seg = pSegments[i];
						for (int y = 0; y < seg.rect.h; y++)
							std::memcpy(pixels.data() + ((seg.rect.y + y) * c_canvasSize.w + seg.rect.x) * 4, seg.pBuffer + y * seg.pitch, seg.rect.w * 4);
					}
				});

			addDefaultSoftKernels(pBackend);
			pBackend->defineCanvas(CanvasRef::Default, c_canvasSize * 64, PixelFormat::ARGB_8, 64, ColorSpace::Linear);

			auto pDevice = GfxDeviceGen2::create(pBackend);

			RectSPX updateRects[2] = { { 0, 0, c_canvasSize.w * 64, 64 * 101 }, { 0, 64 * 101, c_canvasSize.w * 64, (c_canvasSize.h - 101) * 64 } };

			pDevice->beginRender();
			pDevice->beginCanvasUpdate(CanvasRef::Default, 2, updateRects);
			pDevice->fill(c_background);
			scene(pDevice);
			pDevice->endCanvasUpdate();
			pDevice->endRender();
			return pixels;
		}

		GfxBackend_p pBackend;

		if (backend == Backend::Soft)
		{
			auto pSoft = SoftBackend::create();
			addDefaultSoftKernels(pSoft);
			pBackend = pSoft;
		}
		else
			pBackend = ReferenceBackend::create();

		auto pDevice = GfxDeviceGen2::create(pBackend);
		auto pCanvas = SoftSurface::create(WGBP(Surface, _.size = c_canvasSize, _.format = PixelFormat::ARGB_8, _.colorSpace = ColorSpace::Linear, _.canvas = true));

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(pCanvas);
		pDevice->fill(c_background);
		scene(pDevice);
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		auto buffer = pCanvas->allocPixelBuffer();
		pCanvas->pushPixels(buffer);

		for (int y = 0; y < c_canvasSize.h; y++)
			std::memcpy(pixels.data() + y * c_canvasSize.w * 4, buffer.pixels + y * buffer.pitch, c_canvasSize.w * 4);

		pCanvas->freePixelBuffer(buffer);
		return pixels;
	}

	//____ Recorded lines _____________________________________________________

	struct RecordedLine
	{
		CoordSPX	begin;
		CoordSPX	end;
		spx			thickness;
		LineCaps	caps;
	};

	std::vector<RecordedLine> g_lines;

	// Pixels drawn by the line kernels of SoftBackend, whole columns between the ends of lines.

	std::vector<bool> bodyMask()
	{
		std::vector<bool> mask(c_canvasSize.w * c_canvasSize.h, false);
		SoftShapes shapes;

		for (const RecordedLine& line : g_lines)
		{
			SoftShapes::LineParts parts;
			if (!shapes.prepareLine({ line.begin, line.end, line.thickness, line.caps }, parts))
				continue;

			for (int column = parts.bodyBeg; column < parts.bodyEnd; column++)
			{
				int64_t pos = parts.bodyPos + ((int64_t(column - parts.bodyBeg) * parts.bodySlope24 + 128) >> 8);
				int first = int(pos >> 16) - 1;							// The real edges can reach a bit further.
				int last = int((pos + parts.bodyWidth) >> 16) + 1;

				for (int minor = first; minor <= last; minor++)
				{
					int x = parts.bAlongX ? column : minor;
					int y = parts.bAlongX ? minor : column;

					if (x >= 0 && y >= 0 && x < c_canvasSize.w && y < c_canvasSize.h)
						mask[y * c_canvasSize.w + x] = true;
				}
			}
		}
		return mask;
	}

	//____ compare() __________________________________________________________
	// Compares two renderings. Channels may differ by tolerance, and by up to maxDiff on at most
	// maxOutliers pixels. Pixels in pBodyMask may differ by bodyTolerance.

	bool compare(std::ostream& output, const char* pWhat, const Pixels& a, const char* pNameA, const Pixels& b, const char* pNameB,
				 int tolerance, int maxOutliers = 0, int maxDiff = 0, const std::vector<bool>* pBodyMask = nullptr, int bodyTolerance = 0)
	{
		int outliers = 0;

		for (int y = 0; y < c_canvasSize.h; y++)
		{
			for (int x = 0; x < c_canvasSize.w; x++)
			{
				int diff = 0;
				int channel = 0;

				for (int ch = 0; ch < 4; ch++)
				{
					int i = (y * c_canvasSize.w + x) * 4 + ch;
					int d = std::abs(int(a[i]) - int(b[i]));
					if (d > diff)
					{
						diff = d;
						channel = ch;
					}
				}

				const bool bBody = pBodyMask && (*pBodyMask)[y * c_canvasSize.w + x];

				if (diff <= (bBody ? bodyTolerance : tolerance))
					continue;

				if (diff <= maxDiff && ++outliers <= maxOutliers)
					continue;

				int i = (y * c_canvasSize.w + x) * 4 + channel;

				output << pWhat << ": pixel " << x << "," << y << " channel " << channel << " (BGRA) is " << int(a[i]) << " with " << pNameA
					   << ", " << int(b[i]) << " with " << pNameB << (bBody ? " in body" : "") << " (" << outliers << " outliers allowed up to " << maxOutliers << ")." << std::endl;
				return false;
			}
		}
		return true;
	}

	// SoftBackend compared to ReferenceBackend.

	const int c_tolerance = 3;			// Ends are rounded to whole spx, see SoftShapes.
	const int c_bodyTolerance = 26;		// 1/8 of the difference between line color and background, see above.

	bool compareToReference(std::ostream& output, const char* pWhat, const std::function<void(GfxDevice* pDevice)>& scene,
							int tolerance = c_tolerance, int bodyTolerance = c_bodyTolerance)
	{
		Pixels reference = render(Backend::Reference, scene);

		g_lines.clear();
		Pixels soft = render(Backend::Soft, scene);
		std::vector<bool> mask = bodyMask();
		g_lines.clear();

		return compare(output, pWhat, soft, "SoftBackend", reference, "ReferenceBackend", tolerance, 0, 0, &mask, bodyTolerance);
	}

	//____ Scenes _____________________________________________________________

	const LineCap c_caps[] = { LineCap::None, LineCap::Flat, LineCap::Square, LineCap::Round, LineCap::Vertical,
							   LineCap::Horizontal, LineCap::Spear(50), LineCap::Spear(300) };
	const int c_nCaps = 8;

	// Line from center cx, cy (pixels) at angle (degrees, clockwise from along x) and length.

	void drawLineAt(GfxDevice* pDevice, double cx, double cy, double angle, double length, double thickness, LineCaps caps, HiColor color = c_lineColor)
	{
		double rad = angle * M_PI / 180;
		double dx = std::cos(rad) * length / 2;
		double dy = std::sin(rad) * length / 2;

		CoordSPX beg = { spx(std::lround((cx - dx) * 64)), spx(std::lround((cy - dy) * 64)) };
		CoordSPX end = { spx(std::lround((cx + dx) * 64)), spx(std::lround((cy + dy) * 64)) };

		spx t = spx(std::lround(thickness * 64));
		g_lines.push_back({ beg, end, t, caps });
		pDevice->drawLine(beg, end, color, t, caps);
	}

	// Every cap at both ends, in eight directions, in cells of 32 x 32 pixels.

	void capsScene(GfxDevice* pDevice, double thickness, double length)
	{
		const double angles[8] = { 0, 17, 45, 70, 90, 128, 200, 333 };

		for (int c = 0; c < c_nCaps; c++)
			for (int a = 0; a < 8; a++)
				drawLineAt(pDevice, c * 32 + 16.3, a * 32 + 16.6, angles[a], length, thickness, c_caps[c]);
	}
}

LineRenderTest::LineRenderTest()
{
	ADD_TEST(capsCodecTest);
	ADD_TEST(capsCommandTest);
	ADD_TEST(softVsReferenceTest);
	ADD_TEST(shortLinesTest);
	ADD_TEST(mixedCapsTest);
	ADD_TEST(extremesTest);
	ADD_TEST(linearBackendTest);
	ADD_TEST(clipTest);
	ADD_TEST(tintTest);
	ADD_TEST(blendModesTest);
	ADD_TEST(streamTest);
	ADD_TEST(cApiTest);
}

LineRenderTest::~LineRenderTest()
{
}

//____ capsCodecTest() ________________________________________________________
// Caps survive the caps word, with spear lengths rounded to multiples of 25 within 25 to 800.

bool LineRenderTest::capsCodecTest(std::ostream& output)
{
	TEST_ASSERT(GfxBackend::encodeLineCaps(LineCaps()) == 0);
	TEST_ASSERT(GfxBackend::encodeLineCaps(LineCaps(LineCap::None, LineCap::None)) == 0);

	// Every shape at both ends.

	for (int b = 0; b < LineCapShape_size; b++)
	{
		for (int e = 0; e < LineCapShape_size; e++)
		{
			LineCaps caps(LineCap(LineCapShape(b), 125), LineCap(LineCapShape(e), 650));
			LineCaps decoded = GfxBackend::decodeLineCaps(GfxBackend::encodeLineCaps(caps));

			TEST_ASSERT(decoded.begin.shape == caps.begin.shape);
			TEST_ASSERT(decoded.end.shape == caps.end.shape);

			if (caps.begin.shape == LineCapShape::Spear)
				TEST_ASSERT(decoded.begin.spearLength == 125);
			if (caps.end.shape == LineCapShape::Spear)
				TEST_ASSERT(decoded.end.spearLength == 650);
		}
	}

	// Spear lengths.

	struct { uint16_t in, out; } lengths[] = { {0, 25}, {1, 25}, {25, 25}, {37, 25}, {38, 50}, {200, 200}, {212, 200}, {213, 225},
											   {787, 775}, {788, 800}, {800, 800}, {801, 800}, {65535, 800} };

	for (auto& l : lengths)
	{
		LineCaps decoded = GfxBackend::decodeLineCaps(GfxBackend::encodeLineCaps(LineCaps(LineCap::Flat, LineCap::Spear(l.in))));

		if (decoded.end.spearLength != l.out)
		{
			output << "Spear length " << l.in << " decoded as " << decoded.end.spearLength << ", expected " << l.out << "." << std::endl;
			return false;
		}
		TEST_ASSERT(decoded.begin.shape == LineCapShape::Flat);
	}

	// Unused shape values decode as None.

	TEST_ASSERT(GfxBackend::decodeLineCaps(0x0707).isNone());
	TEST_ASSERT(GfxBackend::encodeLineCaps(LineCaps(LineCap(LineCapShape(7)))) == 0);

	return true;
}

//____ capsCommandTest() ______________________________________________________
// GfxDeviceGen2 encodes caps into its line commands, also from the direction overload.

bool LineRenderTest::capsCommandTest(std::ostream& output)
{
	auto pSoft = SoftBackend::create();
	addDefaultSoftKernels(pSoft);

	std::ostringstream log;
	auto pLogger = BackendLogger::create(&log, pSoft);
	auto pDevice = GfxDeviceGen2::create(pLogger);
	auto pCanvas = SoftSurface::create(WGBP(Surface, _.size = { 32, 32 }, _.format = PixelFormat::ARGB_8, _.canvas = true));

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(pCanvas);
	pDevice->drawLine({ 64, 64 }, { 64 * 20, 64 * 8 }, Color::Black, 64);
	pDevice->drawLine({ 64, 64 }, { 64 * 20, 64 * 8 }, Color::Black, 64, LineCap::Round);
	pDevice->drawLine({ 64, 64 }, { 64 * 20, 64 * 8 }, Color::Black, 64, { LineCap::Flat, LineCap::Spear(50) });
	pDevice->drawLine({ 64 * 4, 64 * 4 }, Direction::Down, 64 * 10, Color::Black, 128, LineCap::Square);
	pDevice->endCanvasUpdate();
	pDevice->endRender();

	std::string text = log.str();

	TEST_ASSERT(text.find("from (64, 64) to (1280, 512) with thickness 64 and color 0, 0, 0, 4096\n") != std::string::npos);
	TEST_ASSERT(text.find("caps Round and Round\n") != std::string::npos);
	TEST_ASSERT(text.find("caps Flat and Spear(50%)\n") != std::string::npos);
	TEST_ASSERT(text.find("from (256, 256) to (256, 896) with thickness 128 and color 0, 0, 0, 4096, caps Square and Square\n") != std::string::npos);

	return true;
}

//____ softVsReferenceTest() __________________________________________________
// Every cap in eight directions, thin and thick.

bool LineRenderTest::softVsReferenceTest(std::ostream& output)
{
	for (double thickness : { 1.5, 6.0 })
	{
		auto scene = [&](GfxDevice* pDevice) { capsScene(pDevice, thickness, 20.0); };

		std::string what = "Caps at thickness " + std::to_string(thickness);
		if (!compareToReference(output, what.c_str(), scene))
			return false;
	}
	return true;
}

//____ shortLinesTest() _______________________________________________________
// Lines too short for whole columns between their ends, drawn all as shapes, and lines of
// no length: dots and squares.

bool LineRenderTest::shortLinesTest(std::ostream& output)
{
	auto scene = [&](GfxDevice* pDevice)
	{
		for (int c = 0; c < c_nCaps; c++)
		{
			for (int i = 0; i < 8; i++)
			{
				double length = (i % 4) * 0.7;
				double thickness = i < 4 ? 3.0 : 9.5;
				drawLineAt(pDevice, c * 32 + 16.3, i * 32 + 16.6, 30.0 * i, length, thickness, c_caps[c]);
			}
		}
	};

	return compareToReference(output, "Short lines", scene);
}

//____ mixedCapsTest() ________________________________________________________
// Different caps at begin and end, drawn both ways.

bool LineRenderTest::mixedCapsTest(std::ostream& output)
{
	auto scene = [&](GfxDevice* pDevice)
	{
		for (int b = 0; b < c_nCaps; b++)
		{
			for (int e = 0; e < c_nCaps; e++)
			{
				double angle = (b * 8 + e) * 23.0;
				drawLineAt(pDevice, b * 32 + 16.3, e * 32 + 16.6, angle, 18.0, 4.5, { c_caps[b], c_caps[e] });
			}
		}
	};

	return compareToReference(output, "Mixed caps", scene);
}

//____ extremesTest() _________________________________________________________
// Ends far outside the canvas, caps reaching far, very thin and very thick lines.

bool LineRenderTest::extremesTest(std::ostream& output)
{
	auto scene = [&](GfxDevice* pDevice)
	{
		auto line = [&](CoordSPX beg, CoordSPX end, spx thickness, LineCaps caps)
		{
			g_lines.push_back({ beg, end, thickness, caps });
			pDevice->drawLine(beg, end, c_lineColor, thickness, caps);
		};

		line({ -64 * 100000, 64 * 10 + 17 }, { 64 * 100000, 64 * 200 + 45 }, 64 * 3, LineCap::Round);
		line({ 64 * 30 + 5, -64 * 200000 }, { 64 * 31 + 50, 64 * 250000 }, 64 * 2, LineCap::Spear(800));
		line({ 64 * 100, 64 * 20 }, { 64 * 100 + 1, 64 * 230 }, 64 * 4, LineCap::Vertical);
		line({ 64 * 20, 64 * 120 }, { 64 * 230, 64 * 120 + 3 }, 64 * 4, LineCap::Horizontal);
		line({ 64 * 20 + 7, 64 * 40 }, { 64 * 200, 64 * 90 }, 1, LineCap::Square);
		line({ 64 * 150, 64 * 170 }, { 64 * 170, 64 * 175 }, 64 * 120, { LineCap::Round, LineCap::Spear(25) });
		line({ 64 * 60, 64 * 240 }, { 64 * 60, 64 * 240 }, 64 * 40, LineCap::Horizontal);		// Nothing, no extent along y.
	};

	if (!compareToReference(output, "Extremes", scene))
		return false;

	g_lines.clear();
	return compare(output, "Extremes", render(Backend::Linear, scene), "LinearBackend", render(Backend::Soft, scene), "SoftBackend", 0);
}

//____ linearBackendTest() ____________________________________________________

bool LineRenderTest::linearBackendTest(std::ostream& output)
{
	auto scene = [&](GfxDevice* pDevice)
	{
		capsScene(pDevice, 5.0, 22.0);

		// Across the boundary between the update rectangles.

		drawLineAt(pDevice, 128.4, 101.2, 80.0, 60.0, 7.0, LineCap::Round);
		drawLineAt(pDevice, 60.4, 100.7, 3.0, 90.0, 3.0, { LineCap::Spear(150), LineCap::Square });
	};

	return compare(output, "LinearBackend", render(Backend::Linear, scene), "LinearBackend", render(Backend::Soft, scene), "SoftBackend", 0);
}

//____ clipTest() _____________________________________________________________
// Lines drawn through a clip list are the same within it and leave the rest untouched.

bool LineRenderTest::clipTest(std::ostream& output)
{
	const RectSPX clips[3] = { { 64 * 10, 64 * 10, 64 * 100, 64 * 37 }, { 64 * 30, 64 * 47, 64 * 150, 64 * 60 }, { 64 * 7, 64 * 150, 64 * 240, 64 * 80 } };

	auto scene = [&](GfxDevice* pDevice) { capsScene(pDevice, 6.0, 26.0); };

	auto clipped = [&](GfxDevice* pDevice)
	{
		pDevice->setClipList(3, clips);
		capsScene(pDevice, 6.0, 26.0);
		pDevice->resetClipList();
	};

	Pixels whole = render(Backend::Soft, scene);
	Pixels empty = render(Backend::Soft, [](GfxDevice*) {});
	Pixels expected(whole.size());

	for (int y = 0; y < c_canvasSize.h; y++)
	{
		for (int x = 0; x < c_canvasSize.w; x++)
		{
			bool bInside = false;
			for (auto& clip : clips)
				bInside = bInside || (clip / 64).contains(CoordI(x, y));

			const Pixels& source = bInside ? whole : empty;
			std::memcpy(expected.data() + (y * c_canvasSize.w + x) * 4, source.data() + (y * c_canvasSize.w + x) * 4, 4);
		}
	}

	return compare(output, "Clipped", render(Backend::Soft, clipped), "clip list", expected, "masked", 0);
}

//____ tintTest() _____________________________________________________________
// Lines are tinted like everything else, by the tint color and a Tint.

bool LineRenderTest::tintTest(std::ostream& output)
{
	auto scene = [&](GfxDevice* pDevice)
	{
		pDevice->setTintColor(HiColor(3000, 4096, 2000, 3500));
		capsScene(pDevice, 4.0, 20.0);

		auto pTint = Tint::create(HiColor(4096, 1000, 1000, 4096), HiColor(1000, 1000, 4096, 2000), { 0.f, 0.f }, { 1.f, 1.f });
		pDevice->setTint(RectSPX(64 * 20, 64 * 20, 64 * 200, 64 * 200), pTint);
		drawLineAt(pDevice, 128.3, 128.6, 33.0, 180.0, 9.0, LineCap::Round);
		drawLineAt(pDevice, 128.3, 128.6, 123.0, 180.0, 6.0, { LineCap::Spear(200), LineCap::Horizontal });
		pDevice->clearTint();
		pDevice->clearTintColor();
	};

	return compareToReference(output, "Tinted", scene);
}

//____ blendModesTest() _______________________________________________________

bool LineRenderTest::blendModesTest(std::ostream& output)
{
	for (BlendMode mode : { BlendMode::Replace, BlendMode::Add, BlendMode::Multiply, BlendMode::Max })
	{
		auto scene = [&](GfxDevice* pDevice)
		{
			pDevice->setBlendMode(mode);
			capsScene(pDevice, 5.0, 20.0);
			pDevice->setBlendMode(BlendMode::Blend);
		};

		// Replace mixes partly covered pixels weighted by alpha, which magnifies the error of the
		// coverage. Ends are placed within 1/128 pixel, as SoftShapes works in spx.

		std::string what = std::string("Blend mode ") + toString(mode);
		if (!compareToReference(output, what.c_str(), scene, c_tolerance + 1, c_bodyTolerance + 10))
			return false;
	}
	return true;
}

//____ streamTest() ___________________________________________________________
// Caps make it through a stream.

bool LineRenderTest::streamTest(std::ostream& output)
{
	auto scene = [&](GfxDevice* pDevice)
	{
		capsScene(pDevice, 5.0, 20.0);
		pDevice->drawLine({ 64 * 3, 64 * 250 }, { 64 * 250, 64 * 251 }, c_lineColor, 64 * 2);		// Without caps.
	};

	auto direct = render(Backend::Soft, scene);

	Pixels streamed(c_canvasSize.w * c_canvasSize.h * 4, 0);

	auto pCanvas = SoftSurface::create(WGBP(Surface, _.size = c_canvasSize, _.format = PixelFormat::ARGB_8, _.colorSpace = ColorSpace::Linear, _.canvas = true));

	auto pBackend = SoftBackend::create();
	addDefaultSoftKernels(pBackend);
	pBackend->defineCanvas(CanvasRef::Default, pCanvas);

	auto pStreamPlayer = StreamPlayer::create(pBackend, SoftSurfaceFactory::create(), SoftEdgemapFactory::create());
	auto pStreamEncoder = StreamEncoder::create({ pStreamPlayer, pStreamPlayer->input });
	auto pStreamBackend = StreamBackend::create(pStreamEncoder);
	pStreamBackend->defineCanvas(CanvasRef::Default, c_canvasSize, PixelFormat::ARGB_8, 64, ColorSpace::Linear);

	auto pStreamDevice = GfxDeviceGen2::create(pStreamBackend);

	pStreamDevice->beginRender();
	pStreamDevice->beginCanvasUpdate(CanvasRef::Default);
	pStreamDevice->fill(c_background);
	scene(pStreamDevice);
	pStreamDevice->endCanvasUpdate();
	pStreamDevice->endRender();

	auto buffer = pCanvas->allocPixelBuffer();
	pCanvas->pushPixels(buffer);
	for (int y = 0; y < c_canvasSize.h; y++)
		std::memcpy(streamed.data() + y * c_canvasSize.w * 4, buffer.pixels + y * buffer.pitch, c_canvasSize.w * 4);
	pCanvas->freePixelBuffer(buffer);

	return compare(output, "Streamed lines", streamed, "streamed", direct, "direct", 0);
}

//____ cApiTest() _____________________________________________________________

bool LineRenderTest::cApiTest(std::ostream& output)
{
	auto cpp = [&](GfxDevice* pDevice)
	{
		pDevice->drawLine({ 64 * 10, 64 * 10 }, { 64 * 200 + 13, 64 * 60 + 40 }, c_lineColor, 64 * 5 + 7, { LineCap::Round, LineCap::Spear(175) });
		pDevice->drawLine({ 64 * 20, 64 * 100 }, Direction::Right, 64 * 150, c_lineColor, 64 * 4, { LineCap::Square, LineCap::Vertical });
		pDevice->drawLine({ 64 * 30, 64 * 120 }, { 64 * 230, 64 * 200 }, c_lineColor, 64 * 3);
	};

	auto c = [&](GfxDevice* pDevice)
	{
		wax_obj device = (wax_obj) static_cast<Object*>(pDevice);
		wax_color color;
		std::memcpy(&color, &c_lineColor, sizeof(color));

		wax_drawLineWithCaps(device, { 64 * 10, 64 * 10 }, { 64 * 200 + 13, 64 * 60 + 40 }, color, 64 * 5 + 7, { WAX_LINECAP_ROUND, 200 }, { WAX_LINECAP_SPEAR, 175 });
		wax_drawStraightLineWithCaps(device, { 64 * 20, 64 * 100 }, WAX_DIR_RIGHT, 64 * 150, color, 64 * 4, { WAX_LINECAP_SQUARE, 200 }, { WAX_LINECAP_VERTICAL, 200 });
		wax_drawLine(device, { 64 * 30, 64 * 120 }, { 64 * 230, 64 * 200 }, color, 64 * 3);
	};

	return compare(output, "C API", render(Backend::Soft, c), "C API", render(Backend::Soft, cpp), "C++", 0);
}
