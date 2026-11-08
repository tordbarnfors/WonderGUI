#define _USE_MATH_DEFINES		// For M_PI in MSVC.

#include "warprender.h"

#include <wondergui.h>
#include <wax_device_gen2.h>
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
#include <wax_roundedrectwarp.h>
#include <wax_cylinderwarp.h>
#include <wax_c_cylinderwarp.h>
#include <wax_perspectivewarp.h>
#include <wax_c_perspectivewarp.h>
#include <wax_ballwarp.h>
#include <wax_c_ballwarp.h>
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

	enum class TestBackend
	{
		Soft,
		Linear,
		Reference
	};

	using Scene = std::function<void(GfxDevice* pDevice)>;
	using Pixels = std::vector<uint8_t>;

	//____ render() ___________________________________________________________
	// Renders a scene on a cleared canvas. LinearBackend renders in two update rectangles.

	Pixels render(TestBackend backend, const Scene& scene, PixelFormat canvasFormat = PixelFormat::ARGB_8)
	{
		Pixels pixels(c_canvasSize.w * c_canvasSize.h * 4, 0);

		if (backend == TestBackend::Linear)
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

		if (backend == TestBackend::Soft)
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

	// Sizes of the sources the Warpmaps are made for: edgemaps, surfaces and the source rectangles used.

	const SizeSPX c_edgemapSource = { 48 * 64, 24 * 64 };
	const SizeSPX c_surfaceSource = { 32 * 64, 16 * 64 };
	const SizeSPX c_rectSource = { 16 * 64, 16 * 64 };

	Warpmap_p makeRing(GfxDevice* pDevice, int size, SizeSPX source, float sweep = 360.f, float startAngle = 0.f)
	{
		return CircularWarp::create(WGBP(CircularWarp,
											_.size = { size, size },
											_.sourceSize = source,
											_.topRadius = (size / 2 - 2) * 64,
											_.bottomRadius = (size / 4) * 64,
											_.startAngle = startAngle,
											_.sweep = sweep), pDevice->warpmapFactory())->generate();
	}

	// Vertices and triangles for a grid of cells, with the position in the source at each node from
	// nodeAt(x, y) (pixels of the Warpmap): four vertices and two triangles for each cell, as
	// WarpGenerator makes them. With a period, values are moved by whole periods to be close to the
	// top-left corner of their cell.

	struct Mesh
	{
		Warpmap::Blueprint				bp;
		std::vector<Warpmap::Vertex>	vertices;
		std::vector<Warpmap::Triangle>	triangles;
	};

	Mesh gridMesh(SizeI size, SizeI cellSize, const std::function<CoordSPX(int x, int y)>& nodeAt, spx uPeriod = 0)
	{
		Mesh mesh;
		SizeI cells = { (size.w + cellSize.w - 1) / cellSize.w, (size.h + cellSize.h - 1) / cellSize.h };

		for (int row = 0; row < cells.h; row++)
			for (int col = 0; col < cells.w; col++)
			{
				int x0 = col * cellSize.w, x1 = std::min(x0 + cellSize.w, size.w);
				int y0 = row * cellSize.h, y1 = std::min(y0 + cellSize.h, size.h);
				const CoordI corners[4] = { { x0, y0 }, { x1, y0 }, { x1, y1 }, { x0, y1 } };

				int first = int(mesh.vertices.size());
				spx u0 = 0;

				for (int i = 0; i < 4; i++)
				{
					CoordSPX pos = nodeAt(corners[i].x, corners[i].y);

					if (uPeriod > 0)
					{
						if (i == 0)
							pos.x = u0 = ((pos.x % uPeriod) + uPeriod) % uPeriod;
						else
							pos.x -= spx(std::floor((pos.x - u0) / double(uPeriod) + 0.5)) * uPeriod;
					}

					mesh.vertices.push_back({ corners[i].x * 64, corners[i].y * 64, pos.x, pos.y });
				}

				mesh.triangles.push_back({ first, first + 1, first + 2 });
				mesh.triangles.push_back({ first, first + 2, first + 3 });
			}

		mesh.bp.size = size;
		mesh.bp.triangles = int(mesh.triangles.size());
		mesh.bp.uPeriod = uPeriod;
		mesh.bp.vertices = int(mesh.vertices.size());
		return mesh;
	}

	Warpmap_p createMesh(WarpmapFactory* pFactory, const Mesh& mesh)
	{
		return pFactory->createWarpmap(mesh.bp, mesh.vertices.data(), mesh.triangles.data());
	}

	// A smooth, non-periodic warp of a rectangle, partly outside the source.

	Warpmap_p makeWave(GfxDevice* pDevice, int size, SizeSPX source)
	{
		return createMesh(pDevice->warpmapFactory(), gridMesh({ size, size }, { 8, 8 }, [&](int px, int py)
		{
			float x = px / float(size);
			float y = py / float(size);
			float u = (x - 0.5f) * 1.3f + 0.5f + 0.1f * std::sin(y * 9.f);
			float v = (y - 0.5f) * 1.25f + 0.5f + 0.08f * std::sin(x * 7.f);
			return CoordSPX(spx(std::lround(u * source.w)), spx(std::lround(v * source.h)));
		}));
	}

	// Scene drawing all kinds of warps.

	void warpScene(GfxDevice* pDevice)
	{
		auto pEdgemap = makeEdgemap(pDevice, true);
		auto pTinted = makeEdgemap(pDevice, false, true);
		auto pRing = makeRing(pDevice, 44, c_edgemapSource);
		auto pArc = makeRing(pDevice, 44, c_edgemapSource, 270.f, -135.f);
		auto pWave = makeWave(pDevice, 44, c_edgemapSource);

		pDevice->warpDrawEdgemap({ 2 * 64, 2 * 64 }, pRing, pEdgemap);
		pDevice->warpDrawEdgemap({ 50 * 64, 2 * 64 }, pArc, pEdgemap, { 11 * 64, 3 * 64 });
		pDevice->warpDrawEdgemap({ 98 * 64, 2 * 64 }, pWave, pTinted);

		pDevice->setBlitSource(makeSurface(pDevice, true, SampleMethod::Bilinear));
		pDevice->warpBlit({ 2 * 64, 50 * 64 }, makeRing(pDevice, 44, c_surfaceSource));

		pDevice->setTint({ 50 * 64, 50 * 64, 44 * 64, 44 * 64 }, Tint::create(HiColor(4096, 4096, 4096, 4096), HiColor(4096, 1000, 1000, 2000)));
		pDevice->warpBlit({ 50 * 64, 50 * 64 }, makeRing(pDevice, 44, c_rectSource, 270.f, -135.f), { 8 * 64, 0, 16 * 64, 16 * 64 }, { 5 * 64, 0 });
		pDevice->clearTint();

		pDevice->setBlitSource(makeSurface(pDevice, false, SampleMethod::Nearest));
		pDevice->warpBlit({ 98 * 64, 50 * 64 }, makeWave(pDevice, 44, c_surfaceSource));
	}

	// Straight onto the source, with cells of cellSize, or the default Warpmap if 0.

	Warpmap_p identity(SizeI size, int cellSize)
	{
		if (cellSize == 0)
			return SoftWarpmapFactory::create()->createWarpmap(WGBP(Warpmap, _.size = size));

		return createMesh(SoftWarpmapFactory::create(), gridMesh(size, { cellSize, cellSize }, [](int x, int y) { return CoordSPX(x * 64, y * 64); }));
	}

	// Renders a scene through StreamBackend and StreamPlayer onto a SoftBackend. Resources are
	// created through the stream.

	Pixels renderStreamed(const Scene& scene)
	{
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

		return streamed;
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
	ADD_TEST(meshSeamTest);
	ADD_TEST(perspectiveWeightTest);
	ADD_TEST(vertexCoverageTest);
	ADD_TEST(softVsReferenceTest);
	ADD_TEST(blendModesTest);
	ADD_TEST(linearBackendTest);
	ADD_TEST(offsetPeriodTest);
	ADD_TEST(clipTest);
	ADD_TEST(opaqueCanvasTest);
	ADD_TEST(streamTest);
	ADD_TEST(streamContentTest);
	ADD_TEST(nonSquareCellsTest);
	ADD_TEST(cApiTest);
	ADD_TEST(seamColorTest);
	ADD_TEST(sourceRectTest);
	ADD_TEST(generateTest);
	ADD_TEST(rotationOffsetTest);
	ADD_TEST(roundedRectTest);
	ADD_TEST(cylinderTest);
	ADD_TEST(perspectiveTest);
	ADD_TEST(ballTest);
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

	bool bOk = compare(output, "SoftBackend", render(TestBackend::Soft, warped), "warp", render(TestBackend::Soft, direct), "drawEdgemap", 2);
	bOk = bOk && compare(output, "ReferenceBackend", render(TestBackend::Reference, warped), "warp", render(TestBackend::Reference, direct), "drawEdgemap", 2);

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

		bOk = bOk && compare(output, pWhat, render(TestBackend::Soft, warped), "SoftBackend warp", render(TestBackend::Soft, direct), "SoftBackend blit", 1);
		bOk = bOk && compare(output, pWhat, render(TestBackend::Reference, warped), "ReferenceBackend warp", render(TestBackend::Reference, direct), "ReferenceBackend blit", 1);
	}

	return bOk;
}

//____ meshSeamTest() _________________________________________________________
/*
	Triangles don't show where they meet: a linear mapping drawn through two triangles, through a
	grid of cells split along either diagonal and through a fan around a point that isn't on the
	grid draws the same. Edges cross the source and its edges in all directions, and the mesh goes
	past the source so that its borders are antialiased inside triangles.
*/

bool WarpRenderTest::meshSeamTest(std::ostream& output)
{
	auto pRes = resourceDevice();
	auto pEdgemap = makeEdgemap(pRes, false, true);
	auto pSurface = makeSurface(pRes, false, SampleMethod::Bilinear);

	const SizeI size = { 64, 40 };
	auto nodeAt = [](int x, int y) { return CoordSPX(52 * x + 17 * y - 400, -11 * x + 47 * y - 160); };

	auto vertex = [&](int x, int y) { CoordSPX uv = nodeAt(x, y); return Warpmap::Vertex{ x * 64, y * 64, uv.x, uv.y }; };

	// Two triangles.

	std::vector<Warpmap::Vertex> quadVertices = { vertex(0, 0), vertex(size.w, 0), vertex(size.w, size.h), vertex(0, size.h) };
	std::vector<Warpmap::Triangle> quadTriangles = { { 0, 1, 2 }, { 0, 2, 3 } };

	// Cells of 8 x 8, split along alternating diagonals, except the middle 2 x 2 cells which are a
	// fan of 8 triangles around a point off the grid.

	std::vector<Warpmap::Vertex> gridVertices;
	std::vector<Warpmap::Triangle> gridTriangles;

	auto index = [&](int x, int y)
	{
		for (int i = 0; i < int(gridVertices.size()); i++)
			if (gridVertices[i].x == x * 64 && gridVertices[i].y == y * 64)
				return i;
		gridVertices.push_back(vertex(x, y));
		return int(gridVertices.size()) - 1;
	};

	for (int row = 0; row < size.h / 8; row++)
		for (int col = 0; col < size.w / 8; col++)
		{
			if ((col == 3 || col == 4) && (row == 2 || row == 3))
				continue;

			int tl = index(col * 8, row * 8), tr = index(col * 8 + 8, row * 8), br = index(col * 8 + 8, row * 8 + 8), bl = index(col * 8, row * 8 + 8);

			if ((row + col) % 2)
			{
				gridTriangles.push_back({ tl, tr, br });
				gridTriangles.push_back({ tl, bl, br });				// Counter clockwise.
			}
			else
			{
				gridTriangles.push_back({ tl, tr, bl });
				gridTriangles.push_back({ tr, br, bl });
			}
		}

	gridVertices.push_back(vertex(31, 22));
	int center = int(gridVertices.size()) - 1;

	const CoordI ring[8] = { { 24, 16 }, { 32, 16 }, { 40, 16 }, { 40, 24 }, { 40, 32 }, { 32, 32 }, { 24, 32 }, { 24, 24 } };
	for (int i = 0; i < 8; i++)
		gridTriangles.push_back({ center, index(ring[i].x, ring[i].y), index(ring[(i + 1) % 8].x, ring[(i + 1) % 8].y) });

	auto create = [&](const std::vector<Warpmap::Vertex>& vertices, const std::vector<Warpmap::Triangle>& triangles)
	{
		Warpmap::Blueprint bp;
		bp.size = size;
		bp.triangles = int(triangles.size());
		bp.vertices = int(vertices.size());
		return SoftWarpmapFactory::create()->createWarpmap(bp, vertices.data(), triangles.data());
	};

	auto pQuad = create(quadVertices, quadTriangles);
	auto pGrid = create(gridVertices, gridTriangles);
	TEST_ASSERT(pQuad && pGrid);

	auto scene = [&](Warpmap* pWarpmap)
	{
		return [=](GfxDevice* pDevice)
		{
			pDevice->warpDrawEdgemap({ 4 * 64, 4 * 64 }, pWarpmap, pEdgemap);
			pDevice->setBlitSource(pSurface);
			pDevice->warpBlit({ 80 * 64, 4 * 64 }, pWarpmap, { 0, 0, 24 * 64, 16 * 64 });
			pDevice->setBlendMode(BlendMode::Add);
			pDevice->warpBlit({ 40 * 64, 50 * 64 }, pWarpmap, { 0, 0, 24 * 64, 16 * 64 });
		};
	};

	for (TestBackend backend : { TestBackend::Soft, TestBackend::Linear, TestBackend::Reference })
	{
		if (!compare(output, "Mesh of many triangles", render(backend, scene(pGrid)), "many triangles", render(backend, scene(pQuad)), "two triangles", 1))
			return false;
	}

	return true;
}

//____ perspectiveWeightTest() ________________________________________________
/*
	A quad in perspective drawn through two triangles with weights draws as a fine mesh without
	weights, whose positions are exact at its vertices and linear in between.
*/

bool WarpRenderTest::perspectiveWeightTest(std::ostream& output)
{
	auto pRes = resourceDevice();
	auto pEdgemap = makeEdgemap(pRes, false, true);
	auto pSurface = makeSurface(pRes, true, SampleMethod::Bilinear);

	// The source from x = 5 to 60, shrinking to the right towards a horizon at x = 105, partly
	// outside the Warpmap at the top and bottom. Positions in the source are (u, v) / d.

	const SizeI size = { 70, 50 };
	const SizeSPX source = c_edgemapSource;

	auto divisor = [](double x, double y) { return 1.0 - 0.01 * (x - 5); };
	auto exact = [&](double x, double y)
	{
		double d = divisor(x, y);
		double u = source.w * (x - 5) * 0.45 / 55;
		double v = source.h / 2 + source.h * (y - 25) / 40;
		return CoordSPX(spx(std::lround(u / d)), spx(std::lround((v - source.h / 2 * (1 - d)) / d)));
	};

	Warpmap::Vertex vertices[4];
	const CoordI corners[4] = { { 0, 0 }, { size.w, 0 }, { size.w, size.h }, { 0, size.h } };
	for (int i = 0; i < 4; i++)
	{
		CoordSPX uv = exact(corners[i].x, corners[i].y);
		vertices[i] = { corners[i].x * 64, corners[i].y * 64, uv.x, uv.y, int(std::lround(divisor(corners[i].x, corners[i].y) * Warpmap::weightOne)) };
	}

	Warpmap::Triangle triangles[2] = { { 0, 1, 3 }, { 1, 2, 3 } };
	auto pQuad = SoftWarpmapFactory::create()->createWarpmap(WGBP(Warpmap, _.size = size), vertices, triangles);
	auto pFine = createMesh(SoftWarpmapFactory::create(), gridMesh(size, { 1, 1 }, [&](int x, int y) { return exact(x, y); }));
	TEST_ASSERT(pQuad && pFine);

	auto scene = [&](Warpmap* pWarpmap)
	{
		return [=](GfxDevice* pDevice)
		{
			pDevice->warpDrawEdgemap({ 4 * 64, 4 * 64 }, pWarpmap, pEdgemap);
			pDevice->setBlitSource(pSurface);
			pDevice->warpBlit({ 84 * 64, 4 * 64 }, pWarpmap, { 64 * 4, 0 });
		};
	};

	// SoftBackend and LinearBackend divide for perspective every few pixels, keeping within a
	// quarter of a pixel, which shows by more along the edges of the checkers of the surface.

	for (TestBackend backend : { TestBackend::Soft, TestBackend::Linear, TestBackend::Reference })
	{
		bool bExact = backend == TestBackend::Reference;
		if (!compare(output, "Perspective by weights", render(backend, scene(pQuad)), "weights", render(backend, scene(pFine)), "fine mesh",
											3, bExact ? 20 : 500, bExact ? 12 : 32))
			return false;
	}

	return true;
}

//____ vertexCoverageTest() ___________________________________________________
// Coverage of the vertices multiplies the coverage of pixels: a disc with its outline going from
// full coverage half a pixel inside to none half a pixel outside is antialiased the same by all
// backends, half coverage everywhere draws as a tint of half alpha, and none draws nothing.

bool WarpRenderTest::vertexCoverageTest(std::ostream& output)
{
	auto pRes = resourceDevice();
	auto pEdgemap = makeEdgemap(pRes, false, true);
	auto pSurface = makeSurface(pRes, true, SampleMethod::Bilinear);

	// A disc of radius 23 pixels over a Warpmap of 60 x 50, mapped straight onto the source.

	const SizeI size = { 60, 50 };
	const double cx = 30.3, cy = 24.6, radius = 23;
	const int nSides = 64;

	auto disc = [&](int innerCoverage, int outerCoverage)
	{
		Mesh mesh;
		auto add = [&](double x, double y, int coverage)
		{
			mesh.vertices.push_back({ spx(std::lround(x * 64)), spx(std::lround(y * 64)), spx(std::lround(x * 64)), spx(std::lround(y * 64)), Warpmap::weightOne, coverage });
			return int(mesh.vertices.size()) - 1;
		};

		int center = add(cx, cy, innerCoverage);
		for (int i = 0; i < nSides; i++)
		{
			double a = 2 * M_PI * i / nSides;
			add(cx + std::cos(a) * (radius - 0.5), cy + std::sin(a) * (radius - 0.5), innerCoverage);
			add(cx + std::cos(a) * (radius + 0.5), cy + std::sin(a) * (radius + 0.5), outerCoverage);
		}

		for (int i = 0; i < nSides; i++)
		{
			int in0 = 1 + i * 2, out0 = in0 + 1;
			int in1 = 1 + ((i + 1) % nSides) * 2, out1 = in1 + 1;
			mesh.triangles.push_back({ center, in0, in1 });
			mesh.triangles.push_back({ in0, out0, out1 });
			mesh.triangles.push_back({ in0, out1, in1 });
		}

		mesh.bp.size = size;
		mesh.bp.triangles = int(mesh.triangles.size());
		mesh.bp.vertices = int(mesh.vertices.size());
		return createMesh(SoftWarpmapFactory::create(), mesh);
	};

	auto pFaded = disc(Warpmap::coverageOne, 0);
	auto pHalf = disc(Warpmap::coverageOne / 2, Warpmap::coverageOne / 2);
	auto pFull = disc(Warpmap::coverageOne, Warpmap::coverageOne);
	auto pNone = disc(0, 0);
	TEST_ASSERT(pFaded && pHalf && pFull && pNone);

	auto scene = [&](Warpmap* pWarpmap, HiColor tint = HiColor::White)
	{
		return [=](GfxDevice* pDevice)
		{
			pDevice->setTintColor(tint);
			pDevice->warpDrawEdgemap({ 4 * 64, 4 * 64 }, pWarpmap, pEdgemap);
			pDevice->setBlitSource(pSurface);
			pDevice->warpBlit({ 84 * 64, 4 * 64 }, pWarpmap);
		};
	};

	// Faded outline: the backends that estimate coverage against the one that measures it. Along
	// the outline coverage is linear across a pixel, as for any straight edge.

	Pixels reference = render(TestBackend::Reference, scene(pFaded));

	for (TestBackend backend : { TestBackend::Soft, TestBackend::Linear })
	{
		if (!compare(output, "Faded outline", render(backend, scene(pFaded)), backend == TestBackend::Soft ? "soft" : "linear", reference, "reference", 6, 40, 16))
			return false;
	}

	// The faded outline differs from the full one only along the outline.

	if (compare(output, "Faded outline", reference, "faded", render(TestBackend::Reference, scene(pFull)), "full", 3, 0, 0))
	{
		output << "Coverage of the vertices made no difference" << std::endl;
		return false;
	}

	// Half coverage everywhere as half alpha, and none as nothing.

	for (TestBackend backend : { TestBackend::Soft, TestBackend::Linear, TestBackend::Reference })
	{
		if (!compare(output, "Half coverage", render(backend, scene(pHalf)), "half coverage", render(backend, scene(pFull, HiColor(4096, 4096, 4096, 2048))), "half alpha", 2, 0, 2))
			return false;

		if (!compare(output, "No coverage", render(backend, scene(pNone)), "no coverage", render(backend, [](GfxDevice*) {}), "nothing drawn", 0))
			return false;
	}

	return true;
}

//____ softVsReferenceTest() __________________________________________________
// SoftBackend estimates coverage close to what ReferenceBackend measures. Corners of edges
// and the source within a pixel are where estimates are off the most.

bool WarpRenderTest::softVsReferenceTest(std::ostream& output)
{
	return compare(output, "Warp scene", render(TestBackend::Soft, warpScene), "SoftBackend", render(TestBackend::Reference, warpScene), "ReferenceBackend", 6, 60, 48);
}

//____ blendModesTest() _______________________________________________________

bool WarpRenderTest::blendModesTest(std::ostream& output)
{
	auto pRes = resourceDevice();

	const BlendMode modes[] = { BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::Subtract, BlendMode::Multiply,
								BlendMode::Invert, BlendMode::Min, BlendMode::Max, BlendMode::Morph, BlendMode::BlendFixedColor };

	auto pEdgemap = makeEdgemap(pRes, true);
	auto pRing = makeRing(pRes, 40, c_edgemapSource);
	auto pBlitRing = makeRing(pRes, 40, c_surfaceSource);
	auto pWave = makeWave(pRes, 40, c_surfaceSource);
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
			pDevice->warpBlit({ 110 * 64, 4 * 64 }, pBlitRing);
		};

		std::string what = std::string("BlendMode::") + toString(mode);

		if (!compare(output, what.c_str(), render(TestBackend::Soft, scene), "SoftBackend", render(TestBackend::Reference, scene), "ReferenceBackend", 6, 40, 48))
			return false;
	}

	return true;
}

//____ linearBackendTest() ____________________________________________________
// LinearBackend draws warps exactly as SoftBackend, also across update rectangles.

bool WarpRenderTest::linearBackendTest(std::ostream& output)
{
	return compare(output, "Warp scene", render(TestBackend::Linear, warpScene), "LinearBackend", render(TestBackend::Soft, warpScene), "SoftBackend", 0);
}

//____ offsetPeriodTest() _____________________________________________________
// A source offset of a whole period draws the same as none, also for negative offsets.

bool WarpRenderTest::offsetPeriodTest(std::ostream& output)
{
	auto pRes = resourceDevice();

	auto pEdgemap = makeEdgemap(pRes, true);
	auto pArc = makeRing(pRes, 44, c_edgemapSource, 270.f, -135.f);
	auto pBlitArc = makeRing(pRes, 44, { 24 * 64, 16 * 64 }, 270.f, -135.f);
	auto pSurface = makeSurface(pRes, true, SampleMethod::Bilinear);

	// Edgemap is 48 pixels wide, period is 360 / 270 of that: 64 pixels. Source rectangle of the
	// blit is 24 pixels wide, period 32 pixels.

	auto scene = [&](int edgemapOffset, int blitOffset)
	{
		return [=](GfxDevice* pDevice)
		{
			pDevice->warpDrawEdgemap({ 2 * 64, 2 * 64 }, pArc, pEdgemap, { edgemapOffset + 7 * 64, 0 });
			pDevice->setBlitSource(pSurface);
			pDevice->warpBlit({ 60 * 64, 2 * 64 }, pBlitArc, { 4 * 64, 0, 24 * 64, 16 * 64 }, { blitOffset + 3 * 64, 0 });
		};
	};

	auto base = render(TestBackend::Soft, scene(0, 0));

	return compare(output, "Offset one period", render(TestBackend::Soft, scene(64 * 64, 32 * 64)), "offset", base, "no offset", 0) &&
		   compare(output, "Offset minus two periods", render(TestBackend::Soft, scene(-128 * 64, -64 * 64)), "offset", base, "no offset", 0);
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

	auto background = render(TestBackend::Soft, [](GfxDevice*) {});

	for (TestBackend backend : { TestBackend::Soft, TestBackend::Linear })
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

		if (!compare(output, backend == TestBackend::Soft ? "SoftBackend" : "LinearBackend", pixels, "clipped", expected, "expected", 0))
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

		if (!compare(output, what.c_str(), render(TestBackend::Soft, scene, PixelFormat::XRGB_8), "SoftBackend",
					 render(TestBackend::Reference, scene, PixelFormat::XRGB_8), "ReferenceBackend", 6, 60, 48))
			return false;
	}

	return true;
}

//____ streamTest() ___________________________________________________________
// Warps streamed through StreamBackend and StreamPlayer draw exactly as directly. The resources
// are created through the stream, so Warpmaps, their content and periods are streamed too.

bool WarpRenderTest::streamTest(std::ostream& output)
{
	auto scene = [](GfxDevice* pDevice)
	{
		warpScene(pDevice);

		// Periods changed after creation.

		auto pWarpmap = makeRing(pDevice, 40, c_edgemapSource, 270.f, -135.f);
		pWarpmap->setPeriods(c_edgemapSource.w * 2, 0);
		pDevice->warpDrawEdgemap({ 64 * 110, 64 * 50 }, pWarpmap, makeEdgemap(pDevice, true), { 64 * 30, 0 });
	};

	return compare(output, "Streamed warps", renderStreamed(scene), "streamed", render(TestBackend::Soft, scene), "direct", 0);
}

//____ streamContentTest() ____________________________________________________
// Vertices and triangles are streamed compressed, one update at a time, and the receiver creates a
// Warpmap with its content and applies each update at once, exactly.

namespace
{
	// Receiving end: SoftWarpmaps that count their updates, from a SoftBackend that makes them.

	class CountingSoftWarpmap : public SoftWarpmap
	{
	public:
		CountingSoftWarpmap(const Blueprint& bp, const Vertex* pVertices, const Triangle* pTriangles) : SoftWarpmap(bp, pVertices, pTriangles) {}

		int		updates = 0;

	protected:
		void _updated(int firstVertex, int nVertices, int firstTriangle, int nTriangles) override { updates++; }
	};

	class CountingSoftWarpmapFactory : public WarpmapFactory
	{
	public:
		Warpmap_p createWarpmap(const Warpmap::Blueprint& bp) override { return _created(new CountingSoftWarpmap(bp, nullptr, nullptr), false); }
		Warpmap_p createWarpmap(const Warpmap::Blueprint& bp, const Warpmap::Vertex* pVertices, const Warpmap::Triangle* pTriangles) override { return _created(new CountingSoftWarpmap(bp, pVertices, pTriangles), true); }

		std::vector<CountingSoftWarpmap*>	created;		// Kept alive by the test through the stream.
		int									createdWithContent = 0;

	private:
		Warpmap_p _created(CountingSoftWarpmap* p, bool bWithContent) { created.push_back(p); createdWithContent += bWithContent; return p; }
	};

	class CountingSoftBackend : public SoftBackend
	{
	public:
		WarpmapFactory_p warpmapFactory() override { return pWarpmapFactory; }

		StrongPtr<CountingSoftWarpmapFactory>	pWarpmapFactory = new CountingSoftWarpmapFactory();
	};

	// Counts chunks and their bytes by type on their way to the player.

	class ChunkCounter : public Object, protected StreamSink::Holder
	{
	public:
		ChunkCounter(StreamSink_p next) : input(this), m_next(next) {}

		StreamSink	input;

		int		chunks[256] = {};
		int		bytes[256] = {};

		int count(GfxStream::ChunkId id) const { return chunks[int(id)]; }
		int size(GfxStream::ChunkId id) const { return bytes[int(id)]; }
		void clear() { std::fill(std::begin(chunks), std::end(chunks), 0); std::fill(std::begin(bytes), std::end(bytes), 0); }

	protected:
		void _processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd) override
		{
			for (auto p = pBegin; p < pEnd; p += GfxStream::chunkSize(p))
			{
				chunks[p[0]]++;
				bytes[p[0]] += GfxStream::chunkSize(p);
			}
			m_next->processChunks(pBegin, pEnd);
		}

		StreamSink_p	m_next;
	};
}

bool WarpRenderTest::streamContentTest(std::ostream& output)
{
	using Id = GfxStream::ChunkId;

	StrongPtr<CountingSoftBackend> pReceiver = new CountingSoftBackend();
	auto pReceived = pReceiver->pWarpmapFactory;

	auto pPlayer = StreamPlayer::create(pReceiver, SoftSurfaceFactory::create(), SoftEdgemapFactory::create());
	StrongPtr<ChunkCounter> pCounter = new ChunkCounter({ pPlayer, pPlayer->input });
	auto pEncoder = StreamEncoder::create({ pCounter, pCounter->input });
	auto pStreamBackend = StreamBackend::create(pEncoder);

	auto pWarp = CircularWarp::create(WGBP(CircularWarp, _.size = { 200, 200 }, _.sourceSize = c_edgemapSource, _.topRadius = 98 * 64, _.bottomRadius = 40 * 64,
										   _.cellSize = { 4, 4 }), pStreamBackend->warpmapFactory());

	auto sameContent = [&](Warpmap* pSent, CountingSoftWarpmap* pGot)
	{
		return pGot->vertexCount() == pSent->vertexCount() && pGot->triangleCount() == pSent->triangleCount() &&
			pGot->uPeriod() == pSent->uPeriod() && pGot->vPeriod() == pSent->vPeriod() &&
			std::memcmp(pSent->vertexData(), pGot->vertexData(), pSent->vertexCount() * sizeof(Warpmap::Vertex)) == 0 &&
			std::memcmp(pSent->triangleData(), pGot->triangleData(), pSent->triangleCount() * sizeof(Warpmap::Triangle)) == 0;
	};

	// Created with all its content, compressed.

	pCounter->clear();
	Warpmap_p pSent = pWarp->generate();
	pEncoder->flush();

	int rawVertexBytes = pSent->vertexCount() * int(sizeof(Warpmap::Vertex));
	int rawTriangleBytes = pSent->triangleCount() * int(sizeof(Warpmap::Triangle));

	TEST_ASSERT(pCounter->count(Id::CreateWarpmap) == 1);
	TEST_ASSERT(pCounter->count(Id::WarpmapUpdate) == 1);
	TEST_ASSERT(pCounter->count(Id::WarpmapVertices) >= 1 && pCounter->count(Id::WarpmapTriangles) >= 1);

	// Vertices to less than half, triangles of 6 bytes to less than 4.

	if (pCounter->size(Id::WarpmapVertices) * 2 >= rawVertexBytes || pCounter->size(Id::WarpmapTriangles) * 3 >= rawTriangleBytes * 2)
		output << "Compressed to " << pCounter->size(Id::WarpmapVertices) << " of " << rawVertexBytes << " bytes of vertices and "
			   << pCounter->size(Id::WarpmapTriangles) << " of " << rawTriangleBytes << " bytes of triangles" << std::endl;
	TEST_ASSERT(pCounter->size(Id::WarpmapVertices) * 2 < rawVertexBytes);
	TEST_ASSERT(pCounter->size(Id::WarpmapTriangles) * 3 < rawTriangleBytes * 2);
	TEST_ASSERT(pReceived->created.size() == 1 && pReceived->createdWithContent == 1);

	CountingSoftWarpmap* pGot = pReceived->created[0];
	TEST_ASSERT(pGot->updates == 0);
	TEST_ASSERT(sameContent(pSent, pGot));

	// Updated in place, applied at once.

	pSent = nullptr;
	pCounter->clear();
	TEST_ASSERT(pWarp->setAngles(30.f, 360.f));
	pSent = pWarp->generate();
	pEncoder->flush();

	TEST_ASSERT(pCounter->count(Id::CreateWarpmap) == 0 && pCounter->count(Id::WarpmapUpdate) == 1);
	TEST_ASSERT(pReceived->created.size() == 1);
	TEST_ASSERT(pGot->updates == 1);
	TEST_ASSERT(sameContent(pSent, pGot));

	// New periods are sent with the content and applied together with it.

	pSent = nullptr;
	pCounter->clear();
	spx oldPeriod = pGot->uPeriod();
	TEST_ASSERT(pWarp->setAngles(-60.f, 210.f));
	pSent = pWarp->generate();
	pEncoder->flush();

	TEST_ASSERT(pCounter->count(Id::WarpmapUpdate) == 1);
	TEST_ASSERT(pReceived->created.size() == 1);
	TEST_ASSERT(pGot->uPeriod() != oldPeriod);
	TEST_ASSERT(pGot->updates == 2);
	TEST_ASSERT(sameContent(pSent, pGot));

	// Some of the vertices, some of the triangles and periods alone.

	std::vector<Warpmap::Vertex> patch(21);
	for (int i = 0; i < int(patch.size()); i++)
		patch[i] = { i * 640, -i * 64, i * 7, i * 3, 1000 + i, i * 3000 };				// Weights and coverage.

	std::vector<Warpmap::Triangle> triangles = { { 1, 2, 3 }, { 7, 8, 9 } };

	int updates = pGot->updates;
	pCounter->clear();
	TEST_ASSERT(pSent->importVertices(100, int(patch.size()), patch.data()));
	TEST_ASSERT(pSent->importTriangles(30, 2, triangles.data()));
	TEST_ASSERT(pSent->setPeriods(pSent->uPeriod() + 64, 640));
	pEncoder->flush();

	TEST_ASSERT(pCounter->count(Id::WarpmapUpdate) == 3);
	TEST_ASSERT(pCounter->count(Id::WarpmapVertices) == 1 && pCounter->count(Id::WarpmapTriangles) == 1);
	TEST_ASSERT(pGot->updates == updates + 3);
	TEST_ASSERT(sameContent(pSent, pGot));

	// An odd number of triangles, which go two at a time.

	std::vector<Warpmap::Triangle> odd = { { 4, 5, 6 }, { 10, 11, 12 }, { 20, 21, 22 } };

	updates = pGot->updates;
	TEST_ASSERT(pSent->importTriangles(41, int(odd.size()), odd.data()));
	TEST_ASSERT(pSent->importTriangles(pSent->triangleCount() - 1, 1, odd.data()));
	pEncoder->flush();
	TEST_ASSERT(pGot->updates == updates + 2);
	TEST_ASSERT(sameContent(pSent, pGot));

	// Weights are only sent when there are any other than 1.0. Back to all 1.0, a ring takes less.

	pSent = nullptr;
	pCounter->clear();
	TEST_ASSERT(pWarp->setAngles(10.f, 360.f));
	pSent = pWarp->generate();
	pEncoder->flush();
	TEST_ASSERT(sameContent(pSent, pGot));
	int unweighted = pCounter->size(Id::WarpmapVertices);

	std::vector<Warpmap::Vertex> weighted(pSent->vertexData(), pSent->vertexData() + pSent->vertexCount());
	for (auto& v : weighted)
		v.w = Warpmap::weightOne * 2;

	pCounter->clear();
	TEST_ASSERT(pSent->importVertices(0, int(weighted.size()), weighted.data()));
	pEncoder->flush();
	TEST_ASSERT(sameContent(pSent, pGot));
	TEST_ASSERT(pCounter->size(Id::WarpmapVertices) > unweighted);

	// A mesh in perspective, created with its weights.

	auto pCard = PerspectiveWarp::create(WGBP(PerspectiveWarp, _.size = { 120, 100 }, _.sourceSize = c_edgemapSource,
											  _.corners = PerspectiveWarp::rotated({ 64 * 10, 64 * 10, 64 * 100, 64 * 80 }, 30.f, 40.f, 0.f, 64 * 200)),
										  pStreamBackend->warpmapFactory());
	Warpmap_p pCardMap = pCard->generate();
	pEncoder->flush();
	TEST_ASSERT(sameContent(pCardMap, pReceived->created.back()));

	// Created without content, with the default one.

	auto pDefault = pStreamBackend->warpmapFactory()->createWarpmap({ .size = { 30, 20 }, .triangles = 5, .uPeriod = 1000, .vertices = 9 });
	pEncoder->flush();
	TEST_ASSERT(pReceived->created.size() == 3 && pReceived->createdWithContent == 3);
	TEST_ASSERT(sameContent(pDefault, pReceived->created[2]));

	return true;
}

//____ nonSquareCellsTest() ___________________________________________________
// A sheared and scaled mapping is linear, so a mesh of any cells, with another width than height,
// gives the same positions and draws the same, in all backends and streamed. Edges between
// triangles don't show. SoftBackend steps positions from the corners of each cell in fixed point,
// so rounding can differ by a unit and colors by a level.

bool WarpRenderTest::nonSquareCellsTest(std::ostream& output)
{
	const SizeI size = { 60, 50 };

	// Linear in x and y, whole spx at every pixel, partly outside the source.

	auto sheared = [&](GfxDevice* pDevice, SizeI cellSize)
	{
		return createMesh(pDevice->warpmapFactory(), gridMesh(size, cellSize, [](int x, int y) { return CoordSPX(59 * x + 13 * y - 320, 9 * x + 31 * y - 200); }));
	};

	auto scene = [&](SizeI cellSize)
	{
		return [=](GfxDevice* pDevice)
		{
			// Resources made by the device drawn with, so they are streamed too.

			pDevice->warpDrawEdgemap({ 4 * 64, 4 * 64 }, sheared(pDevice, cellSize), makeEdgemap(pDevice, true));
			pDevice->setBlitSource(makeSurface(pDevice, true, SampleMethod::Bilinear));
			pDevice->warpBlit({ 80 * 64, 4 * 64 }, sheared(pDevice, cellSize), { 64 * 2, 64 * 1, 64 * 24, 64 * 12 }, { 64 * 3, 0 });
		};
	};

	const SizeI shapes[] = { { 60, 4 }, { 3, 50 }, { 17, 11 }, { 100, 100 } };

	for (TestBackend backend : { TestBackend::Soft, TestBackend::Linear, TestBackend::Reference })
	{
		auto square = render(backend, scene({ 5, 5 }));

		for (SizeI cellSize : shapes)
		{
			std::string what = "Cells of " + std::to_string(cellSize.w) + " x " + std::to_string(cellSize.h);
			if (!compare(output, what.c_str(), render(backend, scene(cellSize)), "these cells", square, "5 x 5 cells", 1))
				return false;
		}
	}

	return compare(output, "Cells of 60 x 4, streamed", renderStreamed(scene({ 60, 4 })), "streamed", render(TestBackend::Soft, scene({ 60, 4 })), "direct", 0);
}

//____ cApiTest() _____________________________________________________________
// Warpmaps, CircularWarp and warp calls through the C API draw exactly as through C++.

bool WarpRenderTest::cApiTest(std::ostream& output)
{
	auto pRes = resourceDevice();
	auto pEdgemap = makeEdgemap(pRes, true);
	auto pSurface = makeSurface(pRes, true, SampleMethod::Bilinear);

	// Vertices and triangles of a bent rectangle, given to both.

	const SizeI warpSize = { 40, 36 };
	const int cellSize = 6;
	const spx bentPeriod = c_surfaceSource.w * 3 / 2;

	Mesh bent = gridMesh(warpSize, { cellSize, cellSize }, [&](int px, int py)
	{
		float x = px / float(warpSize.w);
		float y = py / float(warpSize.h);
		float u = x * 1.1f - 0.05f + 0.07f * std::sin(y * 6.f);
		float v = y + 0.05f * std::sin(x * 5.f);
		return CoordSPX(spx(std::lround(u * c_surfaceSource.w)), spx(std::lround(v * c_surfaceSource.h)));
	}, bentPeriod);

	auto cpp = [&](GfxDevice* pDevice)
	{
		auto pRing = CircularWarp::create(WGBP(CircularWarp, _.size = { 44, 44 }, _.sourceSize = c_edgemapSource, _.topRadius = 21 * 64, _.bottomRadius = 10 * 64,
											   _.startAngle = 30.f, _.sweep = 300.f), SoftWarpmapFactory::create());
		auto pBent = createMesh(SoftWarpmapFactory::create(), bent);

		pDevice->warpDrawEdgemap({ 64 * 3, 64 * 4 }, pRing->generate(), pEdgemap, { 64 * 5, 0 });

		auto pFrame = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 44, 40 }, _.sourceSize = c_edgemapSource, _.rect = { 64 * 6, 64 * 6, 64 * 32, 64 * 28 },
												   _.cornerRadius = 64 * 7, _.topDistance = 64 * 5, _.bottomDistance = -64 * 4, _.start = 0.2f, _.sweep = -0.8f), SoftWarpmapFactory::create());
		pDevice->warpDrawEdgemap({ 64 * 3, 64 * 50 }, pFrame->generate(), pEdgemap);
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
		ringBP.sourceSize = { c_edgemapSource.w, c_edgemapSource.h };
		ringBP.center = { 0.5f, 0.5f };
		ringBP.topRadius = 21 * 64;
		ringBP.bottomRadius = 10 * 64;
		ringBP.startAngle = 30.f;
		ringBP.sweep = 300.f;

		wax_obj ring = wax_createCircularWarp(&ringBP, factory);

		wax_warpmapBP bentBP = {};
		bentBP.size = { warpSize.w, warpSize.h };
		bentBP.triangles = bent.bp.triangles;
		bentBP.uPeriod = bentPeriod;
		bentBP.vertices = bent.bp.vertices;

		auto pVertices = (const wax_warpVertex*) bent.vertices.data();
		auto pTriangles = (const wax_warpTriangle*) bent.triangles.data();

		wax_obj cBent = wax_createWarpmapFromMesh(factory, &bentBP, pVertices, pTriangles);

		// The same, given its content and periods after creation, some at a time.

		wax_warpmapBP plainBP = bentBP;
		plainBP.uPeriod = 0;
		wax_obj bentLater = wax_createWarpmap(factory, &plainBP);
		wax_importWarpmapMesh(bentLater, pVertices, nullptr, bentPeriod, 0);
		wax_importWarpmapTriangles(bentLater, 0, 10, pTriangles);
		wax_importWarpmapTriangles(bentLater, 10, bent.bp.triangles - 10, pTriangles + 10);

		wax_warpDrawEdgemap(device, { 64 * 3, 64 * 4 }, wax_generateWarpGenerator(ring), (wax_obj) static_cast<Object*>(pEdgemap.rawPtr()), { 64 * 5, 0 });

		wax_roundedRectWarpBP frameBP = {};
		frameBP.size = { 44, 40 };
		frameBP.sourceSize = { c_edgemapSource.w, c_edgemapSource.h };
		frameBP.rect = { 64 * 6, 64 * 6, 64 * 32, 64 * 28 };
		frameBP.cornerRadius = 64 * 7;
		frameBP.topDistance = 64 * 5;
		frameBP.bottomDistance = -64 * 4;
		frameBP.start = 0.2f;
		frameBP.sweep = -0.8f;

		wax_obj frame = wax_createRoundedRectWarp(&frameBP, factory);
		wax_warpDrawEdgemap(device, { 64 * 3, 64 * 50 }, wax_generateWarpGenerator(frame), (wax_obj) static_cast<Object*>(pEdgemap.rawPtr()), { 0, 0 });
		wax_release(frame);
		pDevice->setBlitSource(pSurface);
		wax_warpBlit(device, { 64 * 60, 64 * 4 }, cBent, { 64 * 2, 64 * 3 });
		wax_rectSPX source = { 64 * 4, 64 * 2, 64 * 20, 64 * 12 };
		wax_warpBlitRect(device, { 64 * 110, 64 * 4 }, bentLater, &source, { 0, 0 });

		wax_release(bentLater);
		wax_release(cBent);
		wax_release(ring);
		wax_release(factory);
	};

	if (!compare(output, "C API", render(TestBackend::Soft, c), "C API", render(TestBackend::Soft, cpp), "C++", 0))
		return false;

	// Content and positions through the C API.

	wax_obj factory = wax_createSoftWarpmapFactory();

	wax_warpmapBP bentBP = {};
	bentBP.size = { warpSize.w, warpSize.h };
	bentBP.triangles = bent.bp.triangles;
	bentBP.vertices = bent.bp.vertices;

	auto pVertices = (const wax_warpVertex*) bent.vertices.data();
	auto pTriangles = (const wax_warpTriangle*) bent.triangles.data();

	wax_obj cBent = wax_createWarpmapFromMesh(factory, &bentBP, pVertices, pTriangles);
	TEST_ASSERT(cBent != nullptr);
	TEST_ASSERT(wax_warpmapVertexCount(cBent) == bent.bp.vertices && wax_warpmapTriangleCount(cBent) == bent.bp.triangles);
	TEST_ASSERT(wax_warpmapPixelSize(cBent).w == warpSize.w && wax_warpmapPixelSize(cBent).h == warpSize.h);

	TEST_ASSERT(wax_setWarpmapPeriods(cBent, bentPeriod, 0) && wax_warpmapUPeriod(cBent) == bentPeriod && wax_warpmapVPeriod(cBent) == 0);
	TEST_ASSERT(!wax_importWarpmapTriangles(cBent, 10, bent.bp.triangles, pTriangles));
	TEST_ASSERT(wax_importWarpmapVertices(cBent, 3, 2, pVertices + 7));

	wax_warpVertex readBack;
	TEST_ASSERT(wax_exportWarpmapVertices(cBent, 4, 1, &readBack) && readBack.u == bent.vertices[8].u && readBack.w == Warpmap::weightOne);

	wax_warpTriangle readTriangle;
	TEST_ASSERT(wax_exportWarpmapTriangles(cBent, 9, 1, &readTriangle) && readTriangle.c == bent.triangles[9].c);

	wax_coordF pos;
	int triangle = wax_warpmapTriangleAt(cBent, { 64 * 20 + 32, 64 * 20 + 32 });
	TEST_ASSERT(triangle >= 0 && wax_warpmapPositionIn(cBent, triangle, 20.5, 20.5, &pos));

	CoordF cppPos;
	auto pCpp = createMesh(SoftWarpmapFactory::create(), bent);
	TEST_ASSERT(pCpp->triangleAt({ 64 * 20 + 32, 64 * 20 + 32 }) == triangle && pCpp->positionIn(triangle, 20.5, 20.5, cppPos));
	TEST_ASSERT(pos.x == cppPos.x && pos.y == cppPos.y);

	wax_release(cBent);
	wax_release(factory);
	return true;
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

	auto pArc = makeRing(pRes, 44, c_surfaceSource, 270.f, -135.f);
	auto pRectArc = makeRing(pRes, 44, c_rectSource, 270.f, -135.f);
	auto pEdgemapArc = makeRing(pRes, 44, c_edgemapSource, 270.f, -135.f);
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
		pDevice->warpBlit({ 50 * 64, 2 * 64 }, pRectArc, { 8 * 64, 0, 16 * 64, 16 * 64 });

		pDevice->setBlitSource(pClamped);
		pDevice->warpBlit({ 98 * 64, 2 * 64 }, pArc);

		pDevice->warpDrawEdgemap({ 2 * 64, 50 * 64 }, pEdgemapArc, pEdgemap);
		pDevice->warpDrawEdgemap({ 50 * 64, 50 * 64 }, pEdgemapArc, pEdgemap, { 7 * 64, 0 });
	};

	return compare(output, "Seams", render(TestBackend::Soft, scene), "SoftBackend", render(TestBackend::Reference, scene), "ReferenceBackend", 6, 4, 12);
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

	auto pArc = makeRing(pRes, 44, { w * 64, h * 64 }, 270.f, -135.f);
	auto pWave = makeWave(pRes, 44, { w * 64, h * 64 });

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

		for (TestBackend backend : { TestBackend::Soft, TestBackend::Linear, TestBackend::Reference })
		{
			if (!compare(output, pWhat, render(backend, scene(true)), "rectangle", render(backend, scene(false)), "cropped surface", 0))
				return false;
		}
	}

	return true;
}

//____ generateTest() _________________________________________________________
/*
	Draws are rendered when the canvas update ends, so a CircularWarp drawn several times with
	different settings within one update must not change the Warpmaps of the earlier draws.
	generate() makes a new Warpmap when the one it has is held by a draw. Drawn on each backend
	and through a stream, which sends the content as it changes.
*/

bool WarpRenderTest::generateTest(std::ostream& output)
{
	const SizeSPX source = c_edgemapSource;

	auto oneGenerator = [=](GfxDevice* pDevice)
	{
		auto pEdgemap = makeEdgemap(pDevice, true);
		auto pWarp = CircularWarp::create(WGBP(CircularWarp, _.size = { 44, 44 }, _.sourceSize = source, _.topRadius = 21 * 64, _.bottomRadius = 10 * 64,
											   _.sweep = 270.f), pDevice->warpmapFactory());

		pDevice->warpDrawEdgemap({ 2 * 64, 2 * 64 }, pWarp->generate(), pEdgemap);
		pWarp->setAngles(90.f, 180.f);
		pDevice->warpDrawEdgemap({ 50 * 64, 2 * 64 }, pWarp->generate(), pEdgemap);
		pWarp->setRadii(20 * 64, 6 * 64);
		pDevice->warpDrawEdgemap({ 98 * 64, 2 * 64 }, pWarp->generate(), pEdgemap);
	};

	auto separateGenerators = [=](GfxDevice* pDevice)
	{
		auto pEdgemap = makeEdgemap(pDevice, true);
		auto pFactory = pDevice->warpmapFactory();

		auto pFirst = CircularWarp::create(WGBP(CircularWarp, _.size = { 44, 44 }, _.sourceSize = source, _.topRadius = 21 * 64, _.bottomRadius = 10 * 64,
												_.sweep = 270.f), pFactory);
		auto pSecond = CircularWarp::create(WGBP(CircularWarp, _.size = { 44, 44 }, _.sourceSize = source, _.topRadius = 21 * 64, _.bottomRadius = 10 * 64,
												 _.startAngle = 90.f, _.sweep = 180.f), pFactory);
		auto pThird = CircularWarp::create(WGBP(CircularWarp, _.size = { 44, 44 }, _.sourceSize = source, _.topRadius = 20 * 64, _.bottomRadius = 6 * 64,
												_.startAngle = 90.f, _.sweep = 180.f), pFactory);

		pDevice->warpDrawEdgemap({ 2 * 64, 2 * 64 }, pFirst->generate(), pEdgemap);
		pDevice->warpDrawEdgemap({ 50 * 64, 2 * 64 }, pSecond->generate(), pEdgemap);
		pDevice->warpDrawEdgemap({ 98 * 64, 2 * 64 }, pThird->generate(), pEdgemap);
	};

	for (TestBackend backend : { TestBackend::Soft, TestBackend::Linear, TestBackend::Reference })
	{
		if (!compare(output, "One CircularWarp drawn three times", render(backend, oneGenerator), "one generator", render(backend, separateGenerators), "separate generators", 0))
			return false;
	}

	return compare(output, "One CircularWarp drawn three times, streamed", renderStreamed(oneGenerator), "one generator", render(TestBackend::Soft, separateGenerators), "separate generators", 0);
}

//____ rotationOffsetTest() ___________________________________________________
// A ring turned by rotationOffset() and a frame moved by shiftOffset() draw as generated with a
// larger start, also through the C API.

bool WarpRenderTest::rotationOffsetTest(std::ostream& output)
{
	auto pRes = resourceDevice();
	auto pEdgemap = makeEdgemap(pRes, true);
	auto pFactory = SoftWarpmapFactory::create();

	auto ringBP = WGBP(CircularWarp, _.size = { 44, 44 }, _.sourceSize = c_edgemapSource, _.topRadius = 21 * 64, _.bottomRadius = 10 * 64,
					   _.startAngle = 20.f, _.sweep = 300.f);
	auto frameBP = WGBP(RoundedRectWarp, _.size = { 60, 44 }, _.sourceSize = c_edgemapSource, _.rect = { 64 * 8, 64 * 8, 64 * 44, 64 * 28 },
						_.cornerRadius = 64 * 8, _.topDistance = 64 * 5, _.bottomDistance = -64 * 4, _.start = 0.1f, _.sweep = 0.8f);

	auto pRing = CircularWarp::create(ringBP, pFactory);
	auto pFrame = RoundedRectWarp::create(frameBP, pFactory);

	ringBP.startAngle += 75.f;
	frameBP.start += 0.3f;
	auto pTurnedRing = CircularWarp::create(ringBP, pFactory);
	auto pMovedFrame = RoundedRectWarp::create(frameBP, pFactory);

	auto turned = [&](GfxDevice* pDevice)
	{
		pDevice->warpDrawEdgemap({ 2 * 64, 2 * 64 }, pRing->generate(), pEdgemap, pRing->rotationOffset(75.f));
		pDevice->warpDrawEdgemap({ 60 * 64, 2 * 64 }, pFrame->generate(), pEdgemap, pFrame->shiftOffset(0.3f));
	};

	auto generated = [&](GfxDevice* pDevice)
	{
		pDevice->warpDrawEdgemap({ 2 * 64, 2 * 64 }, pTurnedRing->generate(), pEdgemap);
		pDevice->warpDrawEdgemap({ 60 * 64, 2 * 64 }, pMovedFrame->generate(), pEdgemap);
	};

	// Vertices of the two differ by rounding to spx, so a few pixels by a level or two.

	if (!compare(output, "Turned by offset", render(TestBackend::Soft, turned), "offset", render(TestBackend::Soft, generated), "generated", 2, 40, 4))
		return false;

	// C API gives the same offsets.

	wax_coordSPX ringOffset = wax_circularWarpRotationOffset((wax_obj) static_cast<Object*>(pRing.rawPtr()), 75.f);
	wax_coordSPX frameOffset = wax_roundedRectWarpShiftOffset((wax_obj) static_cast<Object*>(pFrame.rawPtr()), 0.3f);

	TEST_ASSERT(ringOffset.x == pRing->rotationOffset(75.f).x && ringOffset.y == 0);
	TEST_ASSERT(frameOffset.x == pFrame->shiftOffset(0.3f).x && frameOffset.y == 0);
	return true;
}

//____ roundedRectTest() ______________________________________________________
/*
	RoundedRectWarp drawn by SoftBackend close to what ReferenceBackend measures, and by
	LinearBackend exactly as SoftBackend: an edgemap in a band across the outline, a surface
	outside an outline with sharp corners and an arc of an edgemap going counter clockwise.
*/

bool WarpRenderTest::roundedRectTest(std::ostream& output)
{
	auto scene = [](GfxDevice* pDevice)
	{
		auto pEdgemap = makeEdgemap(pDevice, true);
		auto pFactory = pDevice->warpmapFactory();

		auto pBand = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 48, 40 }, _.sourceSize = c_edgemapSource, _.rect = { 64 * 8, 64 * 8, 64 * 32, 64 * 24 },
												  _.cornerRadius = 64 * 8, _.topDistance = 64 * 6, _.bottomDistance = -64 * 5), pFactory);

		auto pSharp = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 48, 40 }, _.sourceSize = c_surfaceSource, _.rect = { 64 * 10, 64 * 10, 64 * 28, 64 * 20 },
												   _.topDistance = 64 * 7, _.bottomDistance = 0, _.start = 0.1f), pFactory);

		auto pArc = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 48, 40 }, _.sourceSize = c_edgemapSource, _.rect = { 64 * 6, 64 * 6, 64 * 36, 64 * 28 },
												 _.cornerRadius = 64 * 12, _.topDistance = 64 * 4, _.bottomDistance = -64 * 6, _.start = 0.3f, _.sweep = -0.6f), pFactory);

		pDevice->warpDrawEdgemap({ 2 * 64, 2 * 64 }, pBand->generate(), pEdgemap);

		pDevice->setBlitSource(makeSurface(pDevice, true, SampleMethod::Bilinear));
		pDevice->warpBlit({ 54 * 64, 2 * 64 }, pSharp->generate());

		pDevice->warpDrawEdgemap({ 106 * 64, 2 * 64 }, pArc->generate(), pEdgemap, { 64 * 9, 0 });
		pDevice->warpDrawEdgemap({ 2 * 64, 48 * 64 }, pBand->generate(), makeEdgemap(pDevice, false, true));
	};

	return compare(output, "Rounded rects", render(TestBackend::Soft, scene), "SoftBackend", render(TestBackend::Reference, scene), "ReferenceBackend", 6, 60, 48) &&
		   compare(output, "Rounded rects", render(TestBackend::Linear, scene), "LinearBackend", render(TestBackend::Soft, scene), "SoftBackend", 0);
}

//____ cylinderTest() _________________________________________________________
// Cylinders lying and standing, turned by offset, as SoftBackend draws them against
// ReferenceBackend, streamed and through the C API.

bool WarpRenderTest::cylinderTest(std::ostream& output)
{
	auto reel = WGBP(CylinderWarp, _.size = { 30, 60 }, _.sourceSize = { 24 * 64, 48 * 64 }, _.span = 110.f);
	auto can = WGBP(CylinderWarp, _.size = { 60, 36 }, _.sourceSize = c_edgemapSource, _.axis = Axis::Y, _.span = 150.f, _.startAngle = -40.f, _.sweep = 240.f);

	auto scene = [=](GfxDevice* pDevice)
	{
		auto pFactory = pDevice->warpmapFactory();
		auto pReel = CylinderWarp::create(reel, pFactory);
		auto pCan = CylinderWarp::create(can, pFactory);

		pDevice->setBlitSource(makeSurface(pDevice, true, SampleMethod::Bilinear));
		pDevice->warpBlit({ 2 * 64, 2 * 64 }, pReel->generate(), pReel->rotationOffset(35.f));
		pDevice->warpBlit({ 36 * 64, 2 * 64 }, pReel->generate(), pReel->rotationOffset(-200.f));

		auto pEdgemap = makeEdgemap(pDevice, true);
		pDevice->warpDrawEdgemap({ 70 * 64, 2 * 64 }, pCan->generate(), pEdgemap);
		pDevice->warpDrawEdgemap({ 70 * 64, 42 * 64 }, pCan->generate(), pEdgemap, pCan->rotationOffset(60.f));
	};

	if (!compare(output, "Cylinders", render(TestBackend::Soft, scene), "SoftBackend", render(TestBackend::Reference, scene), "ReferenceBackend", 6, 60, 48) ||
		!compare(output, "Cylinders", render(TestBackend::Linear, scene), "LinearBackend", render(TestBackend::Soft, scene), "SoftBackend", 0) ||
		!compare(output, "Cylinders, streamed", renderStreamed(scene), "streamed", render(TestBackend::Soft, scene), "direct", 0))
		return false;

	// Through the C API.

	auto c = [&](GfxDevice* pDevice)
	{
		wax_obj factory = (wax_obj) static_cast<Object*>(pDevice->warpmapFactory().rawPtr());

		wax_cylinderWarpBP reelBP = {};
		reelBP.size = { reel.size.w, reel.size.h };
		reelBP.sourceSize = { reel.sourceSize.w, reel.sourceSize.h };
		reelBP.span = reel.span;

		wax_cylinderWarpBP canBP = {};
		canBP.axis = WAX_AXIS_Y;
		canBP.size = { can.size.w, can.size.h };
		canBP.sourceSize = { can.sourceSize.w, can.sourceSize.h };
		canBP.span = can.span;
		canBP.startAngle = can.startAngle;
		canBP.sweep = can.sweep;

		wax_obj cReel = wax_createCylinderWarp(&reelBP, factory);
		wax_obj cCan = wax_createCylinderWarp(&canBP, factory);
		wax_obj device = (wax_obj) static_cast<Object*>(pDevice);

		pDevice->setBlitSource(makeSurface(pDevice, true, SampleMethod::Bilinear));
		wax_warpBlit(device, { 2 * 64, 2 * 64 }, wax_generateWarpGenerator(cReel), wax_cylinderWarpRotationOffset(cReel, 35.f));
		wax_warpBlit(device, { 36 * 64, 2 * 64 }, wax_generateWarpGenerator(cReel), wax_cylinderWarpRotationOffset(cReel, -200.f));

		auto pEdgemap = makeEdgemap(pDevice, true);
		wax_obj edgemap = (wax_obj) static_cast<Object*>(pEdgemap.rawPtr());
		wax_warpDrawEdgemap(device, { 70 * 64, 2 * 64 }, wax_generateWarpGenerator(cCan), edgemap, { 0, 0 });
		wax_warpDrawEdgemap(device, { 70 * 64, 42 * 64 }, wax_generateWarpGenerator(cCan), edgemap, wax_cylinderWarpRotationOffset(cCan, 60.f));

		TEST_ASSERT(wax_cylinderWarpAxis(cCan) == WAX_AXIS_Y && wax_cylinderWarpSpan(cReel) == 110.f && wax_cylinderWarpSweep(cReel) == 360.f);

		wax_release(cCan);
		wax_release(cReel);
		return true;
	};

	return compare(output, "Cylinders through the C API", render(TestBackend::Soft, [&](GfxDevice* p) { c(p); }), "C API", render(TestBackend::Soft, scene), "C++", 0);
}

//____ perspectiveTest() ______________________________________________________
// A floor scrolling into the distance, cards turned and pinned, blitted and as edgemaps. The cards
// draw as a fine mesh of their exact positions, also along their antialiased borders.

namespace
{
	// Position in the source (spx) at x,y (pixels) of the Warpmap for a source of size source placed
	// at corners, from the projective transform solved as a linear system.

	std::function<CoordSPX(int x, int y)> exactPerspective(const CornersT<CoordSPX>& corners, SizeSPX source)
	{
		double from[4][2], to[4][2] = { { 0, 0 }, { double(source.w), 0 }, { double(source.w), double(source.h) }, { 0, double(source.h) } };
		for (int i = 0; i < 4; i++)
		{
			from[i][0] = corners[i].x / 64.0;
			from[i][1] = corners[i].y / 64.0;
		}

		double m[8][9] = {};
		for (int i = 0; i < 4; i++)
		{
			double x = from[i][0], y = from[i][1], u = to[i][0], v = to[i][1];
			double r1[9] = { x, y, 1, 0, 0, 0, -x * u, -y * u, u };
			double r2[9] = { 0, 0, 0, x, y, 1, -x * v, -y * v, v };
			std::copy(r1, r1 + 9, m[i * 2]);
			std::copy(r2, r2 + 9, m[i * 2 + 1]);
		}

		for (int col = 0; col < 8; col++)
		{
			int pivot = col;
			for (int row = col + 1; row < 8; row++)
				if (std::abs(m[row][col]) > std::abs(m[pivot][col]))
					pivot = row;
			std::swap(m[col], m[pivot]);

			for (int row = 0; row < 8; row++)
			{
				if (row == col)
					continue;
				double f = m[row][col] / m[col][col];
				for (int k = col; k < 9; k++)
					m[row][k] -= f * m[col][k];
			}
		}

		std::vector<double> h(8);
		for (int i = 0; i < 8; i++)
			h[i] = m[i][8] / m[i][i];

		return [h](int x, int y)
		{
			double w = h[6] * x + h[7] * y + 1.0;
			return CoordSPX(spx(std::lround((h[0] * x + h[1] * y + h[2]) / w)), spx(std::lround((h[3] * x + h[4] * y + h[5]) / w)));
		};
	}
}

bool WarpRenderTest::perspectiveTest(std::ostream& output)
{
	const RectSPX cardRect = { 6 * 64, 8 * 64, c_edgemapSource.w, c_edgemapSource.h };
	const CoordSPX sharedEye = { 40 * 64, 20 * 64 };

	auto floor = WGBP(PerspectiveWarp, _.size = { 70, 40 }, _.sourceSize = { 64 * 64, 32 * 64 },
					  _.corners = PerspectiveWarp::rotated({ 0, 0, 70 * 64, 40 * 64 }, 55.f, 0.f, 0.f, 90 * 64, { 0.5f, 1.f }));
	auto card = WGBP(PerspectiveWarp, _.size = { 60, 40 }, _.sourceSize = c_edgemapSource,
					 _.corners = PerspectiveWarp::rotated(cardRect, -15.f, 40.f, 10.f, 120 * 64));
	auto shared = WGBP(PerspectiveWarp, _.size = { 60, 40 }, _.sourceSize = c_edgemapSource,
					   _.corners = PerspectiveWarp::rotated(cardRect, 0.f, -50.f, 0.f, 100 * 64, { 0.f, 0.5f }, sharedEye));
	auto pinned = WGBP(PerspectiveWarp, _.size = { 70, 44 }, _.sourceSize = { 32 * 64, 16 * 64 },
					   _.corners = CornersT<CoordSPX>({ 60 * 64, 3 * 64 }, { 4 * 64, 8 * 64 }, { 10 * 64, 40 * 64 }, { 66 * 64, 30 * 64 }));

	auto scene = [=](GfxDevice* pDevice)
	{
		auto pFactory = pDevice->warpmapFactory();
		auto pFloor = PerspectiveWarp::create(floor, pFactory);
		auto pCard = PerspectiveWarp::create(card, pFactory);
		auto pShared = PerspectiveWarp::create(shared, pFactory);
		auto pPinned = PerspectiveWarp::create(pinned, pFactory);

		pDevice->setBlitSource(makeSurface(pDevice, true, SampleMethod::Bilinear));
		pDevice->warpBlit({ 2 * 64, 2 * 64 }, pFloor->generate(), { 0, 10 * 64 });
		pDevice->warpBlit({ 2 * 64, 48 * 64 }, pPinned->generate());

		auto pEdgemap = makeEdgemap(pDevice, false);
		pDevice->warpDrawEdgemap({ 80 * 64, 2 * 64 }, pCard->generate(), pEdgemap);
		pDevice->warpDrawEdgemap({ 80 * 64, 50 * 64 }, pShared->generate(), pEdgemap);
	};

	if (!compare(output, "Perspective", render(TestBackend::Soft, scene), "SoftBackend", render(TestBackend::Reference, scene), "ReferenceBackend", 6, 60, 48) ||
		!compare(output, "Perspective", render(TestBackend::Linear, scene), "LinearBackend", render(TestBackend::Soft, scene), "SoftBackend", 0) ||
		!compare(output, "Perspective, streamed", renderStreamed(scene), "streamed", render(TestBackend::Soft, scene), "direct", 0))
		return false;

	// Cards against fine meshes of their exact positions. Both round u and v to spx at their
	// vertices, so positions differ by up to about a spx, which shows by a few levels where the
	// source has sharp edges. SoftBackend and LinearBackend also divide for perspective every few
	// pixels, see perspectiveWeightTest(). A border cut off or misplaced differs much more.

	auto cards = [=](bool bExact)
	{
		return [=](GfxDevice* pDevice)
		{
			auto pFactory = pDevice->warpmapFactory();
			auto pEdgemap = makeEdgemap(pDevice, false);
			pDevice->setBlitSource(makeSurface(pDevice, false, SampleMethod::Bilinear));

			int i = 0;
			for (auto& bp : { card, shared, pinned })
			{
				Warpmap_p pWarpmap = bExact ? createMesh(pFactory, gridMesh(bp.size, { 1, 1 }, exactPerspective(bp.corners, bp.sourceSize)))
											: PerspectiveWarp::create(bp, pFactory)->generate();

				CoordSPX pos = { (2 + i * 50) * 64, 2 * 64 };
				if (bp.sourceSize == c_edgemapSource)
					pDevice->warpDrawEdgemap(pos, pWarpmap, pEdgemap);
				else
					pDevice->warpBlit(pos, pWarpmap);

				pDevice->warpBlit({ pos.x, 50 * 64 }, pWarpmap, { 0, 0, bp.sourceSize.w, bp.sourceSize.h });
				i++;
			}
		};
	};

	for (TestBackend backend : { TestBackend::Soft, TestBackend::Linear, TestBackend::Reference })
	{
		bool bExact = backend == TestBackend::Reference;
		if (!compare(output, "Perspective cards", render(backend, cards(false)), "PerspectiveWarp", render(backend, cards(true)), "fine mesh",
											3, bExact ? 150 : 600, bExact ? 10 : 32))
			return false;
	}

	// Through the C API.

	auto toC = [](const CornersT<CoordSPX>& corners, wax_coordSPX out[4])
	{
		for (int i = 0; i < 4; i++)
			out[i] = { corners[i].x, corners[i].y };
	};

	auto c = [&](GfxDevice* pDevice)
	{
		wax_obj factory = (wax_obj) static_cast<Object*>(pDevice->warpmapFactory().rawPtr());
		wax_obj device = (wax_obj) static_cast<Object*>(pDevice);

		wax_perspectiveWarpBP floorBP = {};
		floorBP.size = { floor.size.w, floor.size.h };
		floorBP.sourceSize = { floor.sourceSize.w, floor.sourceSize.h };
		wax_rectSPX floorRect = { 0, 0, 70 * 64, 40 * 64 };
		wax_perspectiveWarpRotated(&floorRect, 55.f, 0.f, 0.f, 90 * 64, { 0.5f, 1.f }, nullptr, floorBP.corners);

		wax_perspectiveWarpBP pinnedBP = {};
		pinnedBP.size = { pinned.size.w, pinned.size.h };
		pinnedBP.sourceSize = { pinned.sourceSize.w, pinned.sourceSize.h };
		toC(pinned.corners, pinnedBP.corners);

		// The cards start out as something else and get their corners set.

		wax_perspectiveWarpBP cardBP = pinnedBP;
		cardBP.size = { card.size.w, card.size.h };
		cardBP.sourceSize = { card.sourceSize.w, card.sourceSize.h };

		wax_obj cFloor = wax_createPerspectiveWarp(&floorBP, factory);
		wax_obj cPinned = wax_createPerspectiveWarp(&pinnedBP, factory);
		wax_obj cCard = wax_createPerspectiveWarp(&cardBP, factory);
		wax_obj cShared = wax_createPerspectiveWarp(&cardBP, factory);

		wax_rectSPX rect = { cardRect.x, cardRect.y, cardRect.w, cardRect.h };
		wax_coordSPX corners[4];
		wax_perspectiveWarpRotated(&rect, -15.f, 40.f, 10.f, 120 * 64, { 0.5f, 0.5f }, nullptr, corners);
		TEST_ASSERT(wax_setPerspectiveWarpCorners(cCard, corners));

		wax_coordSPX eye = { sharedEye.x, sharedEye.y };
		wax_perspectiveWarpRotated(&rect, 0.f, -50.f, 0.f, 100 * 64, { 0.f, 0.5f }, &eye, corners);
		TEST_ASSERT(wax_setPerspectiveWarpCorners(cShared, corners));

		wax_coordSPX read[4];
		wax_perspectiveWarpCorners(cShared, read);
		TEST_ASSERT(read[2].x == shared.corners.bottomRight.x && read[2].y == shared.corners.bottomRight.y);

		pDevice->setBlitSource(makeSurface(pDevice, true, SampleMethod::Bilinear));
		wax_warpBlit(device, { 2 * 64, 2 * 64 }, wax_generateWarpGenerator(cFloor), { 0, 10 * 64 });
		wax_warpBlit(device, { 2 * 64, 48 * 64 }, wax_generateWarpGenerator(cPinned), { 0, 0 });

		auto pEdgemap = makeEdgemap(pDevice, false);
		wax_obj edgemap = (wax_obj) static_cast<Object*>(pEdgemap.rawPtr());
		wax_warpDrawEdgemap(device, { 80 * 64, 2 * 64 }, wax_generateWarpGenerator(cCard), edgemap, { 0, 0 });
		wax_warpDrawEdgemap(device, { 80 * 64, 50 * 64 }, wax_generateWarpGenerator(cShared), edgemap, { 0, 0 });

		wax_release(cShared);
		wax_release(cCard);
		wax_release(cPinned);
		wax_release(cFloor);
		return true;
	};

	return compare(output, "Perspective through the C API", render(TestBackend::Soft, [&](GfxDevice* p) { c(p); }), "C API", render(TestBackend::Soft, scene), "C++", 0);
}

//____ ballTest() _____________________________________________________________
// Globes tilted, rolled and spun by offset, blitted and as edgemaps, as SoftBackend draws them
// against ReferenceBackend, streamed, against much finer meshes and through the C API.

bool WarpRenderTest::ballTest(std::ostream& output)
{
	auto globe = WGBP(BallWarp, _.size = { 44, 44 }, _.sourceSize = { 32 * 64, 16 * 64 }, _.tilt = 23.5f);
	auto rolled = WGBP(BallWarp, _.size = { 40, 46 }, _.sourceSize = { 32 * 64, 16 * 64 }, _.tilt = -60.f, _.roll = 30.f);
	auto marble = WGBP(BallWarp, _.size = { 48, 40 }, _.sourceSize = c_edgemapSource, _.tilt = 80.f, _.roll = -135.f);

	auto scene = [=](GfxDevice* pDevice, float precision)
	{
		auto pFactory = pDevice->warpmapFactory();

		auto bp = globe;
		bp.precision = precision;
		auto pGlobe = BallWarp::create(bp, pFactory);

		bp = rolled;
		bp.precision = precision;
		auto pRolled = BallWarp::create(bp, pFactory);

		bp = marble;
		bp.precision = precision;
		auto pMarble = BallWarp::create(bp, pFactory);

		pDevice->setBlitSource(makeSurface(pDevice, true, SampleMethod::Bilinear));
		pDevice->warpBlit({ 2 * 64, 2 * 64 }, pGlobe->generate(), pGlobe->rotationOffset(40.f));
		pDevice->warpBlit({ 2 * 64, 48 * 64 }, pGlobe->generate(), pGlobe->rotationOffset(-150.f));
		pDevice->warpBlit({ 50 * 64, 2 * 64 }, pRolled->generate());

		auto pEdgemap = makeEdgemap(pDevice, true);
		pDevice->warpDrawEdgemap({ 100 * 64, 2 * 64 }, pMarble->generate(), pEdgemap);
		pDevice->warpDrawEdgemap({ 100 * 64, 50 * 64 }, pMarble->generate(), pEdgemap, pMarble->rotationOffset(100.f));
	};

	auto normal = [=](GfxDevice* pDevice) { scene(pDevice, 0.25f); };

	if (!compare(output, "Balls", render(TestBackend::Soft, normal), "SoftBackend", render(TestBackend::Reference, normal), "ReferenceBackend", 6, 60, 48) ||
		!compare(output, "Balls", render(TestBackend::Linear, normal), "LinearBackend", render(TestBackend::Soft, normal), "SoftBackend", 0) ||
		!compare(output, "Balls, streamed", renderStreamed(normal), "streamed", render(TestBackend::Soft, normal), "direct", 0))
		return false;

	// Against meshes many times finer. Positions differ by up to a quarter of a pixel, which
	// shows by tens of levels where the source has sharp edges, most around the poles where the
	// map is squeezed the most. A ball misplaced, cut or with a gap differs much more.

	auto fine = [=](GfxDevice* pDevice) { scene(pDevice, 0.02f); };

	if (!compare(output, "Balls", render(TestBackend::Reference, normal), "precision 0.25", render(TestBackend::Reference, fine), "precision 0.02", 20, 200, 96))
		return false;

	// Through the C API.

	auto c = [&](GfxDevice* pDevice)
	{
		wax_obj factory = (wax_obj) static_cast<Object*>(pDevice->warpmapFactory().rawPtr());
		wax_obj device = (wax_obj) static_cast<Object*>(pDevice);

		auto toC = [](const BallWarp::Blueprint& bp)
		{
			wax_ballWarpBP c = {};
			c.size = { bp.size.w, bp.size.h };
			c.sourceSize = { bp.sourceSize.w, bp.sourceSize.h };
			c.tilt = bp.tilt;
			c.roll = bp.roll;
			return c;
		};

		auto globeBP = toC(globe);
		auto rolledBP = toC(rolled);
		auto marbleBP = toC(globe);					// Starts out as the globe and gets turned.
		marbleBP.sourceSize = { marble.sourceSize.w, marble.sourceSize.h };
		marbleBP.size = { marble.size.w, marble.size.h };

		wax_obj cGlobe = wax_createBallWarp(&globeBP, factory);
		wax_obj cRolled = wax_createBallWarp(&rolledBP, factory);
		wax_obj cMarble = wax_createBallWarp(&marbleBP, factory);

		TEST_ASSERT(!wax_setBallWarpOrientation(cMarble, 91.f, 0.f));
		TEST_ASSERT(wax_setBallWarpOrientation(cMarble, marble.tilt, marble.roll));
		TEST_ASSERT(wax_ballWarpTilt(cMarble) == marble.tilt && wax_ballWarpRoll(cMarble) == marble.roll);
		TEST_ASSERT(wax_ballWarpRadius(cGlobe) == 44 * 32 - 32);

		pDevice->setBlitSource(makeSurface(pDevice, true, SampleMethod::Bilinear));
		wax_warpBlit(device, { 2 * 64, 2 * 64 }, wax_generateWarpGenerator(cGlobe), wax_ballWarpRotationOffset(cGlobe, 40.f));
		wax_warpBlit(device, { 2 * 64, 48 * 64 }, wax_generateWarpGenerator(cGlobe), wax_ballWarpRotationOffset(cGlobe, -150.f));
		wax_warpBlit(device, { 50 * 64, 2 * 64 }, wax_generateWarpGenerator(cRolled), { 0, 0 });

		auto pEdgemap = makeEdgemap(pDevice, true);
		wax_obj edgemap = (wax_obj) static_cast<Object*>(pEdgemap.rawPtr());
		wax_warpDrawEdgemap(device, { 100 * 64, 2 * 64 }, wax_generateWarpGenerator(cMarble), edgemap, { 0, 0 });
		wax_warpDrawEdgemap(device, { 100 * 64, 50 * 64 }, wax_generateWarpGenerator(cMarble), edgemap, wax_ballWarpRotationOffset(cMarble, 100.f));

		wax_release(cMarble);
		wax_release(cRolled);
		wax_release(cGlobe);
		return true;
	};

	return compare(output, "Balls through the C API", render(TestBackend::Soft, [&](GfxDevice* p) { c(p); }), "C API", render(TestBackend::Soft, normal), "C++", 0);
}
