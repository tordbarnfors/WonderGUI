#define _USE_MATH_DEFINES		// For M_PI in MSVC.

#include "warprender.h"

#include <wondergui.h>
#include <wax_gfxdevice_gen2.h>
#include <wax_softbackend.h>
#include <wax_linearbackend.h>
#include <wax_referencebackend.h>
#include <wax_softsurface.h>
#include <wax_softedgemap.h>
#include <wax_softedgemapfactory.h>
#include <wax_softwarpmap.h>
#include <wax_softwarpmapfactory.h>
#include <wax_softkernels_default.h>
#include <wax_circularwarp.h>
#include <wax_c.h>
#include <wax_c_object.h>
#include <wax_c_softgfx.h>
#include <wax_streambackend.h>
#include <wax_streamplayer.h>
#include <wax_streamencoder.h>
#include <wax_softsurfacefactory.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

using namespace wg;
using namespace wax;

/*
	Draws through Warpmaps with GfxDeviceGen2 and the software backends:

	- A Warpmap that maps its rectangle straight onto the source draws as drawEdgemap() and blit().
	- SoftBackend estimates coverage from the mapping, ReferenceBackend measures it by supersampling.
	  They are compared with a tolerance, allowing for a few pixels where edges have sharp corners.
	- LinearBackend draws exactly as SoftBackend.

	Canvases are ARGB_8 in linear color space.
*/

namespace
{
	const SizeI c_canvasSize = { 160, 96 };

	const HiColor c_background = HiColor(800, 1500, 2600, 4096);

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

	Pixels render(Backend backend, const Scene& scene, PixelFormat canvasFormat = PixelFormat::ARGB_8)
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

			RectSPX updateRects[2] = { { 0, 0, c_canvasSize.w * 64, 64 * 37 }, { 0, 64 * 37, c_canvasSize.w * 64, (c_canvasSize.h - 37) * 64 } };

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
		auto pCanvas = SoftSurface::create(WGBP(Surface, _.size = c_canvasSize, _.format = canvasFormat, _.colorSpace = ColorSpace::Linear, _.canvas = true));

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

		if (canvasFormat == PixelFormat::XRGB_8)
		{
			for (size_t i = 3; i < pixels.size(); i += 4)
				pixels[i] = 255;
		}

		pCanvas->freePixelBuffer(buffer);
		return pixels;
	}

	//____ compare() __________________________________________________________
	// Compares two renderings. Channels may differ by tolerance, and by up to maxDiff on at most
	// maxOutliers pixels. Colors are compared premultiplied by alpha.

	bool compare(std::ostream& output, const char* pWhat, const Pixels& a, const char* pNameA, const Pixels& b, const char* pNameB,
				 int tolerance, int maxOutliers = 0, int maxDiff = 0)
	{
		int outliers = 0;

		for (int y = 0; y < c_canvasSize.h; y++)
		{
			for (int x = 0; x < c_canvasSize.w; x++)
			{
				int diff = 0;
				int channel = 0;

				// Colors are compared premultiplied, as their precision drops with alpha.

				int alphaA = a[(y * c_canvasSize.w + x) * 4 + 3];
				int alphaB = b[(y * c_canvasSize.w + x) * 4 + 3];

				for (int ch = 0; ch < 4; ch++)
				{
					int i = (y * c_canvasSize.w + x) * 4 + ch;
					int d = (ch == 3) ? std::abs(alphaA - alphaB) : std::abs(int(a[i]) * alphaA / 255 - int(b[i]) * alphaB / 255);
					if (d > diff)
					{
						diff = d;
						channel = ch;
					}
				}

				if (diff <= tolerance)
					continue;

				int i = (y * c_canvasSize.w + x) * 4 + channel;

				if (diff <= maxDiff && ++outliers <= maxOutliers)
					continue;

				int p = (y * c_canvasSize.w + x) * 4;

				output << pWhat << ": pixel " << x << "," << y << " channel " << channel << " (BGRA) is " << int(a[i]) << " with " << pNameA
					   << ", " << int(b[i]) << " with " << pNameB << " (" << outliers << " outliers allowed up to " << maxOutliers << "). BGRA "
					   << int(a[p]) << "," << int(a[p + 1]) << "," << int(a[p + 2]) << "," << int(a[p + 3]) << " vs "
					   << int(b[p]) << "," << int(b[p + 1]) << "," << int(b[p + 2]) << "," << int(b[p + 3]) << "." << std::endl;
				return false;
			}
		}
		return true;
	}

	//____ Resources __________________________________________________________

	// An edgemap with three edges: straight, a slow wave and a faster one.

	Edgemap_p makeEdgemap(GfxDevice* pDevice, bool bTiling, bool bTinted = false)
	{
		const int w = 48, h = 24, nEdges = 3;
		std::vector<spx> samples(nEdges * (w + 1));

		for (int i = 0; i <= w; i++)
		{
			double a = 2 * M_PI * i / w;
			samples[i] = spx(std::lround(h * 0.2 * 64));
			samples[(w + 1) + i] = spx(std::lround(h * (0.5 + 0.15 * std::sin(a * 2)) * 64));
			samples[2 * (w + 1) + i] = spx(std::lround(h * (0.8 + 0.08 * std::sin(a * 4)) * 64));
		}

		HiColor colors[4] = { HiColor(4096, 4096, 4096, 4096), HiColor(400, 1800, 4000, 3500), HiColor(4000, 2000, 300, 4096), HiColor(0, 0, 0, 0) };

		Tint_p tints[4];
		if (bTinted)
		{
			tints[1] = Tint::create(HiColor(4096, 4096, 0, 4096), HiColor(0, 4096, 4096, 2500), { 0.f, 0.f }, { 1.f, 0.f });

			Tint::Blueprint radial;
			radial.shape = TintShape::Radial;
			radial.center = { 0.3f, 0.6f };
			radial.stops = { { 0.f, HiColor(4096, 0, 4096, 4096) }, { 1.f, HiColor(0, 4096, 0, 3000) } };
			tints[2] = Tint::create(radial);
		}

		Edgemap::Blueprint bp;
		bp.size = { w, h };
		bp.segments = 4;
		bp.colors = colors;
		bp.tints = bTinted ? tints : nullptr;
		bp.tiling = bTiling;

		return pDevice->edgemapFactory()->createEdgemap(bp, SampleOrigo::Top, samples.data(), nEdges, w + 1);
	}

	// A surface with a pattern that has no two neighbors alike.

	Surface_p makeSurface(GfxDevice* pDevice, bool bTiling, SampleMethod sampleMethod)
	{
		const int w = 32, h = 16;

		std::vector<uint8_t> pixels(w * h * 4);

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				uint8_t* p = pixels.data() + (y * w + x) * 4;
				bool bChecker = ((x / 4) + (y / 4)) & 1;
				p[0] = bChecker ? 40 : 230;
				p[1] = uint8_t(x * 8);
				p[2] = uint8_t(255 - y * 14);
				p[3] = uint8_t(255 - ((x * 7 + y * 3) % 5) * 30);
			}
		}

		return pDevice->surfaceFactory()->createSurface(WGBP(Surface, _.format = PixelFormat::ARGB_8, _.sampleMethod = sampleMethod, _.size = { w, h }, _.tiling = bTiling),
														pixels.data(), PixelFormat::ARGB_8);
	}

	Warpmap_p makeRing(GfxDevice* pDevice, int size, float sweep = 360.f, float startAngle = 0.f)
	{
		return CircularWarp::create(WGBP(CircularWarp,
											_.size = { size, size },
											_.topRadius = (size / 2 - 2) * 64,
											_.bottomRadius = (size / 4) * 64,
											_.startAngle = startAngle,
											_.sweep = sweep), pDevice->warpmapFactory())->refresh();
	}

	// A smooth, non-periodic warp of a rectangle, partly outside the source.

	Warpmap_p makeWave(GfxDevice* pDevice, int size)
	{
		auto pWarpmap = pDevice->warpmapFactory()->createWarpmap(WGBP(Warpmap, _.size = { size, size }, _.cellSize = 8));

		SizeI n = pWarpmap->nodes();
		std::vector<CoordF> nodes(n.w * n.h);

		for (int r = 0; r < n.h; r++)
		{
			for (int c = 0; c < n.w; c++)
			{
				float x = pWarpmap->nodeX(c) / float(size);
				float y = pWarpmap->nodeY(r) / float(size);
				nodes[r * n.w + c] = { (x - 0.5f) * 1.3f + 0.5f + 0.1f * std::sin(y * 9.f), (y - 0.5f) * 1.25f + 0.5f + 0.08f * std::sin(x * 7.f) };
			}
		}

		pWarpmap->importNodes({ 0, 0, n.w, n.h }, nodes.data());
		return pWarpmap;
	}

	// Scene drawing all kinds of warps.

	void warpScene(GfxDevice* pDevice)
	{
		auto pEdgemap = makeEdgemap(pDevice, true);
		auto pTinted = makeEdgemap(pDevice, false, true);
		auto pRing = makeRing(pDevice, 44);
		auto pArc = makeRing(pDevice, 44, 270.f, -135.f);
		auto pWave = makeWave(pDevice, 44);

		pDevice->warpDrawEdgemap({ 2 * 64, 2 * 64 }, pRing, pEdgemap);
		pDevice->warpDrawEdgemap({ 50 * 64, 2 * 64 }, pArc, pEdgemap, { 11 * 64, 3 * 64 });
		pDevice->warpDrawEdgemap({ 98 * 64, 2 * 64 }, pWave, pTinted);

		pDevice->setBlitSource(makeSurface(pDevice, true, SampleMethod::Bilinear));
		pDevice->warpBlit({ 2 * 64, 50 * 64 }, pRing);

		pDevice->setTint({ 50 * 64, 50 * 64, 44 * 64, 44 * 64 }, Tint::create(HiColor(4096, 4096, 4096, 4096), HiColor(4096, 1000, 1000, 2000)));
		pDevice->warpBlit({ 50 * 64, 50 * 64 }, pArc, { 8 * 64, 0, 16 * 64, 16 * 64 }, { 5 * 64, 0 });
		pDevice->clearTint();

		pDevice->setBlitSource(makeSurface(pDevice, false, SampleMethod::Nearest));
		pDevice->warpBlit({ 98 * 64, 50 * 64 }, pWave);
	}

	// The default Warpmap: straight onto the source.

	Warpmap_p identity(SizeI size, int cellSize)
	{
		return SoftWarpmapFactory::create()->createWarpmap(WGBP(Warpmap, _.size = size, _.cellSize = cellSize));
	}

	// Device used to create resources shared by renderings on all software backends.

	GfxDevice_p resourceDevice()
	{
		auto pBackend = SoftBackend::create();
		addDefaultSoftKernels(pBackend);
		return GfxDeviceGen2::create(pBackend);
	}
}

//____ constructor ____________________________________________________________

WarpRenderTest::WarpRenderTest()
{
	ADD_TEST(identityEdgemapTest);
	ADD_TEST(identityBlitTest);
	ADD_TEST(softVsReferenceTest);
	ADD_TEST(blendModesTest);
	ADD_TEST(linearBackendTest);
	ADD_TEST(offsetPeriodTest);
	ADD_TEST(clipTest);
	ADD_TEST(opaqueCanvasTest);
	ADD_TEST(streamTest);
	ADD_TEST(cApiTest);
	ADD_TEST(seamColorTest);
	ADD_TEST(sourceRectTest);
}

WarpRenderTest::~WarpRenderTest()
{
}

//____ identityEdgemapTest() __________________________________________________
// A Warpmap mapping straight onto an edgemap draws it as drawEdgemap() does.

bool WarpRenderTest::identityEdgemapTest(std::ostream& output)
{
	auto pRes = resourceDevice();

	auto pEdgemap = makeEdgemap(pRes, false, true);
	auto pWarpmap = identity(pEdgemap->pixelSize(), 7);

	auto direct = [&](GfxDevice* pDevice)
	{
		pDevice->drawEdgemap({ 5 * 64, 9 * 64 }, pEdgemap);
		pDevice->setTint({ 0, 0, 160 * 64, 96 * 64 }, Tint::create(HiColor(4096, 4096, 4096, 4096), HiColor(1000, 4096, 2000, 3000), { 0.f, 0.f }, { 1.f, 1.f }));
		pDevice->drawEdgemap({ 70 * 64 + 20, 40 * 64 - 10 }, pEdgemap);
	};

	auto warped = [&](GfxDevice* pDevice)
	{
		pDevice->warpDrawEdgemap({ 5 * 64, 9 * 64 }, pWarpmap, pEdgemap);
		pDevice->setTint({ 0, 0, 160 * 64, 96 * 64 }, Tint::create(HiColor(4096, 4096, 4096, 4096), HiColor(1000, 4096, 2000, 3000), { 0.f, 0.f }, { 1.f, 1.f }));
		pDevice->warpDrawEdgemap({ 70 * 64 + 20, 40 * 64 - 10 }, pWarpmap, pEdgemap);
	};

	bool bOk = compare(output, "SoftBackend", render(Backend::Soft, warped), "warp", render(Backend::Soft, direct), "drawEdgemap", 2);
	bOk = bOk && compare(output, "ReferenceBackend", render(Backend::Reference, warped), "warp", render(Backend::Reference, direct), "drawEdgemap", 2);

	return bOk;
}

//____ identityBlitTest() _____________________________________________________
// A Warpmap mapping straight onto a surface draws it as blit() does, for both sample methods.

bool WarpRenderTest::identityBlitTest(std::ostream& output)
{
	auto pRes = resourceDevice();

	bool bOk = true;

	for (auto sampleMethod : { SampleMethod::Nearest, SampleMethod::Bilinear })
	{
		auto pSurface = makeSurface(pRes, false, sampleMethod);
		auto pWarpmap = identity({ 16, 12 }, 5);

		auto direct = [&](GfxDevice* pDevice)
		{
			pDevice->setBlitSource(pSurface);
			pDevice->blit({ 3 * 64, 4 * 64 }, { 64 * 7, 64 * 2, 64 * 16, 64 * 12 });
			pDevice->setTintColor(HiColor(4096, 2000, 4096, 3000));
			pDevice->blit({ 40 * 64, 50 * 64 }, { 64 * 7, 64 * 2, 64 * 16, 64 * 12 });
		};

		auto warped = [&](GfxDevice* pDevice)
		{
			pDevice->setBlitSource(pSurface);
			pDevice->warpBlit({ 3 * 64, 4 * 64 }, pWarpmap, { 64 * 7, 64 * 2, 64 * 16, 64 * 12 });
			pDevice->setTintColor(HiColor(4096, 2000, 4096, 3000));
			pDevice->warpBlit({ 40 * 64, 50 * 64 }, pWarpmap, { 64 * 7, 64 * 2, 64 * 16, 64 * 12 });
		};

		const char* pWhat = sampleMethod == SampleMethod::Nearest ? "Nearest" : "Bilinear";

		bOk = bOk && compare(output, pWhat, render(Backend::Soft, warped), "SoftBackend warp", render(Backend::Soft, direct), "SoftBackend blit", 1);
		bOk = bOk && compare(output, pWhat, render(Backend::Reference, warped), "ReferenceBackend warp", render(Backend::Reference, direct), "ReferenceBackend blit", 1);
	}

	return bOk;
}

//____ softVsReferenceTest() __________________________________________________
// SoftBackend estimates coverage close to what ReferenceBackend measures. Corners of edges
// and the source within a pixel are where estimates are off the most.

bool WarpRenderTest::softVsReferenceTest(std::ostream& output)
{
	return compare(output, "Warp scene", render(Backend::Soft, warpScene), "SoftBackend", render(Backend::Reference, warpScene), "ReferenceBackend", 6, 60, 48);
}

//____ blendModesTest() _______________________________________________________

bool WarpRenderTest::blendModesTest(std::ostream& output)
{
	auto pRes = resourceDevice();

	const BlendMode modes[] = { BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::Subtract, BlendMode::Multiply,
								BlendMode::Invert, BlendMode::Min, BlendMode::Max, BlendMode::Morph, BlendMode::BlendFixedColor };

	auto pEdgemap = makeEdgemap(pRes, true);
	auto pRing = makeRing(pRes, 40);
	auto pWave = makeWave(pRes, 40);
	auto pSurface = makeSurface(pRes, true, SampleMethod::Bilinear);

	for (BlendMode mode : modes)
	{
		// BlendFixedColor blends onto the fixed color in SoftBackend, which gives the same as Blend,
		// as ReferenceBackend draws it, where the canvas is that color.

		const HiColor fixedColor = HiColor(4096, 0, 2000, 4096);

		auto scene = [&](GfxDevice* pDevice)
		{
			if (mode == BlendMode::BlendFixedColor)
				pDevice->fill(fixedColor);

			pDevice->setBlendMode(mode);
			pDevice->setMorphFactor(0.3f);
			pDevice->setFixedBlendColor(fixedColor);

			pDevice->warpDrawEdgemap({ 4 * 64, 4 * 64 }, pRing, pEdgemap);
			pDevice->setBlitSource(pSurface);
			pDevice->warpBlit({ 60 * 64, 4 * 64 }, pWave);
			pDevice->warpBlit({ 110 * 64, 4 * 64 }, pRing);
		};

		std::string what = std::string("BlendMode::") + toString(mode);

		if (!compare(output, what.c_str(), render(Backend::Soft, scene), "SoftBackend", render(Backend::Reference, scene), "ReferenceBackend", 6, 40, 48))
			return false;
	}

	return true;
}

//____ linearBackendTest() ____________________________________________________
// LinearBackend draws warps exactly as SoftBackend, also across update rectangles.

bool WarpRenderTest::linearBackendTest(std::ostream& output)
{
	return compare(output, "Warp scene", render(Backend::Linear, warpScene), "LinearBackend", render(Backend::Soft, warpScene), "SoftBackend", 0);
}

//____ offsetPeriodTest() _____________________________________________________
// A source offset of a whole period draws the same as none, also for negative offsets.

bool WarpRenderTest::offsetPeriodTest(std::ostream& output)
{
	auto pRes = resourceDevice();

	auto pEdgemap = makeEdgemap(pRes, true);
	auto pArc = makeRing(pRes, 44, 270.f, -135.f);
	auto pSurface = makeSurface(pRes, true, SampleMethod::Bilinear);

	// Edgemap is 48 pixels wide, period is 360 / 270 of that: 64 pixels. Source rectangle of the
	// blit is 24 pixels wide, period 32 pixels.

	auto scene = [&](int edgemapOffset, int blitOffset)
	{
		return [=](GfxDevice* pDevice)
		{
			pDevice->warpDrawEdgemap({ 2 * 64, 2 * 64 }, pArc, pEdgemap, { edgemapOffset + 7 * 64, 0 });
			pDevice->setBlitSource(pSurface);
			pDevice->warpBlit({ 60 * 64, 2 * 64 }, pArc, { 4 * 64, 0, 24 * 64, 16 * 64 }, { blitOffset + 3 * 64, 0 });
		};
	};

	auto base = render(Backend::Soft, scene(0, 0));

	return compare(output, "Offset one period", render(Backend::Soft, scene(64 * 64, 32 * 64)), "offset", base, "no offset", 0) &&
		   compare(output, "Offset minus two periods", render(Backend::Soft, scene(-128 * 64, -64 * 64)), "offset", base, "no offset", 0);
}

//____ clipTest() _____________________________________________________________
// Clipped warps draw the same pixels as unclipped within the clip rectangles and nothing outside.

bool WarpRenderTest::clipTest(std::ostream& output)
{
	RectSPX clips[3] = { { 10 * 64, 10 * 64, 30 * 64, 20 * 64 }, { 60 * 64, 5 * 64, 25 * 64, 80 * 64 }, { 100 * 64, 60 * 64, 50 * 64, 20 * 64 } };

	auto clipped = [&](GfxDevice* pDevice)
	{
		pDevice->setClipList(3, clips);
		warpScene(pDevice);
	};

	auto background = render(Backend::Soft, [](GfxDevice*) {});

	for (Backend backend : { Backend::Soft, Backend::Linear })
	{
		auto full = render(backend, warpScene);
		auto pixels = render(backend, clipped);

		// Expected: full scene within clip rectangles, background elsewhere.

		Pixels expected = background;

		for (auto& clip : clips)
		{
			RectI r = clip / 64;
			for (int y = r.y; y < r.y + r.h; y++)
				std::memcpy(expected.data() + (y * c_canvasSize.w + r.x) * 4, full.data() + (y * c_canvasSize.w + r.x) * 4, r.w * 4);
		}

		if (!compare(output, backend == Backend::Soft ? "SoftBackend" : "LinearBackend", pixels, "clipped", expected, "expected", 0))
			return false;
	}

	return true;
}

//____ opaqueCanvasTest() _____________________________________________________
// On a canvas without alpha, paint modes give each part of a pixel the color of what covers it,
// which for Replace is the color of the segment as if opaque, also where a transparent segment
// shares the pixel.

bool WarpRenderTest::opaqueCanvasTest(std::ostream& output)
{
	for (BlendMode mode : { BlendMode::Replace, BlendMode::Blend, BlendMode::Morph })
	{
		auto scene = [&](GfxDevice* pDevice)
		{
			pDevice->setBlendMode(mode);
			pDevice->setMorphFactor(0.6f);
			warpScene(pDevice);
		};

		std::string what = std::string("XRGB_8 canvas, BlendMode::") + toString(mode);

		if (!compare(output, what.c_str(), render(Backend::Soft, scene, PixelFormat::XRGB_8), "SoftBackend",
					 render(Backend::Reference, scene, PixelFormat::XRGB_8), "ReferenceBackend", 6, 60, 48))
			return false;
	}

	return true;
}

//____ streamTest() ___________________________________________________________
// Warps streamed through StreamBackend and StreamPlayer draw exactly as directly. The resources
// are created through the stream, so Warpmaps, their nodes and periods are streamed too.

bool WarpRenderTest::streamTest(std::ostream& output)
{
	auto scene = [](GfxDevice* pDevice)
	{
		warpScene(pDevice);

		// Periods changed after creation, sent before the nodes they normalize.

		auto pWarpmap = makeRing(pDevice, 40, 270.f, -135.f);
		pWarpmap->setPeriods(2.f, 0.f);
		pDevice->warpDrawEdgemap({ 64 * 110, 64 * 50 }, pWarpmap, makeEdgemap(pDevice, true), { 64 * 30, 0 });
	};

	auto direct = render(Backend::Soft, scene);

	// Through a stream

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

	return compare(output, "Streamed warps", streamed, "streamed", direct, "direct", 0);
}

//____ cApiTest() _____________________________________________________________
// Warpmaps, CircularWarp and warp calls through the C API draw exactly as through C++.

bool WarpRenderTest::cApiTest(std::ostream& output)
{
	auto pRes = resourceDevice();
	auto pEdgemap = makeEdgemap(pRes, true);
	auto pSurface = makeSurface(pRes, true, SampleMethod::Bilinear);

	// Nodes of a bent rectangle, given to both.

	const SizeI warpSize = { 40, 36 };
	const int cellSize = 6;
	const SizeI nodes = { (warpSize.w + cellSize - 1) / cellSize + 1, (warpSize.h + cellSize - 1) / cellSize + 1 };

	std::vector<CoordF> nodeData(nodes.w * nodes.h);
	for (int r = 0; r < nodes.h; r++)
		for (int c = 0; c < nodes.w; c++)
		{
			float x = std::min(c * cellSize, warpSize.w) / float(warpSize.w);
			float y = std::min(r * cellSize, warpSize.h) / float(warpSize.h);
			nodeData[r * nodes.w + c] = { x * 1.1f - 0.05f + 0.07f * std::sin(y * 6.f), y + 0.05f * std::sin(x * 5.f) };
		}

	auto cpp = [&](GfxDevice* pDevice)
	{
		auto pRing = CircularWarp::create(WGBP(CircularWarp, _.size = { 44, 44 }, _.topRadius = 21 * 64, _.bottomRadius = 10 * 64,
											   _.startAngle = 30.f, _.sweep = 300.f), SoftWarpmapFactory::create());
		auto pBent = SoftWarpmapFactory::create()->createWarpmap(WGBP(Warpmap, _.size = warpSize, _.cellSize = cellSize, _.uPeriod = 1.5f), nodeData.data());

		pDevice->warpDrawEdgemap({ 64 * 3, 64 * 4 }, pRing->refresh(), pEdgemap, { 64 * 5, 0 });
		pDevice->setBlitSource(pSurface);
		pDevice->warpBlit({ 64 * 60, 64 * 4 }, pBent, { 64 * 2, 64 * 3 });
		pDevice->warpBlit({ 64 * 110, 64 * 4 }, pBent, { 64 * 4, 64 * 2, 64 * 20, 64 * 12 }, { 0, 0 });
	};

	auto c = [&](GfxDevice* pDevice)
	{
		wax_obj device = (wax_obj) static_cast<Object*>(pDevice);
		wax_obj factory = wax_createSoftWarpmapFactory();

		wax_circularWarpBP ringBP = {};
		ringBP.size = { 44, 44 };
		ringBP.center = { 0.5f, 0.5f };
		ringBP.topRadius = 21 * 64;
		ringBP.bottomRadius = 10 * 64;
		ringBP.startAngle = 30.f;
		ringBP.sweep = 300.f;

		wax_obj ring = wax_createCircularWarp(&ringBP, factory);

		wax_warpmapBP bentBP = {};
		bentBP.size = { warpSize.w, warpSize.h };
		bentBP.cellSize = cellSize;
		bentBP.uPeriod = 1.5f;

		wax_obj bent = wax_createWarpmapFromNodes(factory, &bentBP, (const wax_coordF*) nodeData.data(), 0);

		wax_warpDrawEdgemap(device, { 64 * 3, 64 * 4 }, wax_refreshWarpGenerator(ring), (wax_obj) static_cast<Object*>(pEdgemap.rawPtr()), { 64 * 5, 0 });
		pDevice->setBlitSource(pSurface);
		wax_warpBlit(device, { 64 * 60, 64 * 4 }, bent, { 64 * 2, 64 * 3 });
		wax_rectSPX source = { 64 * 4, 64 * 2, 64 * 20, 64 * 12 };
		wax_warpBlitRect(device, { 64 * 110, 64 * 4 }, bent, &source, { 0, 0 });

		wax_release(bent);
		wax_release(ring);
		wax_release(factory);
	};

	return compare(output, "C API", render(Backend::Soft, c), "C API", render(Backend::Soft, cpp), "C++", 0);
}

//____ seamColorTest() ________________________________________________________
/*
	A pixel center in the gap between the copies of the source on a periodic axis takes its colors
	from the nearest copy, so they don't jump where the copies meet. The start of a 270 degree arc
	of a 44 pixel ring runs exactly through pixel centers, along the diagonal down to the left of
	the center. Rounding puts them on either side of the seam, at the start of the source or just
	before the end of the gap, which used to read colors from wherever the position wrapped to in
	the gap. Now both sides give colors from around the start of the source, and SoftBackend and
	ReferenceBackend agree there.
*/

bool WarpRenderTest::seamColorTest(std::ostream& output)
{
	auto pRes = resourceDevice();

	auto pArc = makeRing(pRes, 44, 270.f, -135.f);
	auto pTiling = makeSurface(pRes, true, SampleMethod::Bilinear);
	auto pClamped = makeSurface(pRes, false, SampleMethod::Bilinear);

	// An edgemap without edges, its one segment a gradient across it.

	HiColor color = HiColor::White;
	Tint_p pGradient = Tint::create(HiColor(4096, 0, 0, 4096), HiColor(0, 4096, 1000, 4096), { 0.f, 0.f }, { 1.f, 0.f });

	Edgemap::Blueprint bp;
	bp.size = { 48, 24 };
	bp.segments = 1;
	bp.colors = &color;
	bp.tints = &pGradient;

	auto pEdgemap = pRes->edgemapFactory()->createEdgemap(bp);

	auto scene = [&](GfxDevice* pDevice)
	{
		pDevice->setBlitSource(pTiling);
		pDevice->warpBlit({ 2 * 64, 2 * 64 }, pArc);
		pDevice->warpBlit({ 50 * 64, 2 * 64 }, pArc, { 8 * 64, 0, 16 * 64, 16 * 64 });

		pDevice->setBlitSource(pClamped);
		pDevice->warpBlit({ 98 * 64, 2 * 64 }, pArc);

		pDevice->warpDrawEdgemap({ 2 * 64, 50 * 64 }, pArc, pEdgemap);
		pDevice->warpDrawEdgemap({ 50 * 64, 50 * 64 }, pArc, pEdgemap, { 7 * 64, 0 });
	};

	return compare(output, "Seams", render(Backend::Soft, scene), "SoftBackend", render(Backend::Reference, scene), "ReferenceBackend", 6, 4, 12);
}

//____ sourceRectTest() _______________________________________________________
/*
	A surface that isn't tiling is read as if the edge pixels of the source rectangle continued
	outside it, so nothing outside the rectangle shows, also where bilinear sampling and partly
	covered pixels at the borders reach past it. Blitting a rectangle out of a surface with other
	colors around it draws exactly as blitting a surface that is just that rectangle.
*/

bool WarpRenderTest::sourceRectTest(std::ostream& output)
{
	auto pRes = resourceDevice();

	const int w = 32, h = 16, border = 8;

	std::vector<uint8_t> inner(w * h * 4);
	std::vector<uint8_t> outer((w + border * 2) * (h + border * 2) * 4);

	for (int y = 0; y < h + border * 2; y++)
	{
		for (int x = 0; x < w + border * 2; x++)
		{
			uint8_t* p = outer.data() + (y * (w + border * 2) + x) * 4;
			p[0] = 255; p[1] = 0; p[2] = 255; p[3] = 255;			// Magenta around the rectangle.
		}
	}

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			uint8_t* p = inner.data() + (y * w + x) * 4;
			bool bChecker = ((x / 4) + (y / 4)) & 1;
			p[0] = bChecker ? 40 : 230;
			p[1] = uint8_t(x * 8);
			p[2] = uint8_t(255 - y * 14);
			p[3] = uint8_t(255 - ((x * 7 + y * 3) % 5) * 30);

			std::memcpy(outer.data() + ((y + border) * (w + border * 2) + x + border) * 4, p, 4);
		}
	}

	auto pArc = makeRing(pRes, 44, 270.f, -135.f);
	auto pWave = makeWave(pRes, 44);

	for (SampleMethod method : { SampleMethod::Bilinear, SampleMethod::Nearest })
	{
		auto pInner = pRes->surfaceFactory()->createSurface(WGBP(Surface, _.format = PixelFormat::ARGB_8, _.sampleMethod = method, _.size = { w, h }),
															inner.data(), PixelFormat::ARGB_8);
		auto pOuter = pRes->surfaceFactory()->createSurface(WGBP(Surface, _.format = PixelFormat::ARGB_8, _.sampleMethod = method, _.size = { w + border * 2, h + border * 2 }),
															outer.data(), PixelFormat::ARGB_8);

		auto scene = [&](bool bRect)
		{
			return [=](GfxDevice* pDevice)
			{
				pDevice->setBlitSource(bRect ? pOuter : pInner);
				RectSPX source = bRect ? RectSPX(border * 64, border * 64, w * 64, h * 64) : RectSPX(0, 0, w * 64, h * 64);

				pDevice->warpBlit({ 2 * 64, 2 * 64 }, pArc, source);
				pDevice->warpBlit({ 50 * 64, 2 * 64 }, pWave, source);
				pDevice->warpBlit({ 98 * 64, 2 * 64 }, pArc, source, { 5 * 64, 3 * 64 });
			};
		};

		const char* pWhat = method == SampleMethod::Bilinear ? "Bilinear source rectangle" : "Nearest source rectangle";

		for (Backend backend : { Backend::Soft, Backend::Linear, Backend::Reference })
		{
			if (!compare(output, pWhat, render(backend, scene(true)), "rectangle", render(backend, scene(false)), "cropped surface", 0))
				return false;
		}
	}

	return true;
}
