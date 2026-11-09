#include "testsuite.h"

#include <wax_tint.h>
#include <wax_circularwarp.h>
#include <wax_roundedrectwarp.h>
#include <wax_cylinderwarp.h>
#include <wax_perspectivewarp.h>
#include <wax_ballwarp.h>
#include <wax_lenswarp.h>

#include <cmath>
#include <functional>
#include <vector>

// Edgemaps and surfaces drawn through Warpmaps, with GfxDevice::warpDrawEdgemap() and warpBlit().
// Edges are kept smooth, since coverage of sharp corners of edges within a pixel is only estimated
// by the backends (ReferenceBackend measures it).

class WarpTests : public TestSuite
{
public:
	WarpTests()
	{
		name = "WarpTests";

		addTest("Ring", &WarpTests::ring);
		addTest("Arcs and offsets", &WarpTests::arcs);
		addTest("Tinted segments", &WarpTests::tintedSegments);
		addTest("Device tint", &WarpTests::deviceTint);
		addTest("Blit ring", &WarpTests::blitRing);
		addTest("Blit nearest wave", &WarpTests::blitWave);
		addTest("Blit source rectangle", &WarpTests::blitSourceRect);
		addTest("Periodic V", &WarpTests::periodicV);
		addTest("Tiling not power of two", &WarpTests::tilingNPOT);
		addTest("Identity", &WarpTests::identity);
		addTest("BlendModes", &WarpTests::blendModes);
		addTest("Rounded rects", &WarpTests::roundedRects);
		addTest("Cylinders", &WarpTests::cylinders);
		addTest("Perspective", &WarpTests::perspective);
		addTest("Vertex coverage", &WarpTests::vertexCoverage);
		addTest("Balls", &WarpTests::balls);
		addTest("Lenses", &WarpTests::lenses);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		auto pEdgemapFactory = pDevice->edgemapFactory();
		auto pWarpmapFactory = pDevice->warpmapFactory();
		auto pSurfaceFactory = pDevice->surfaceFactory();

		if (!pEdgemapFactory || !pWarpmapFactory || !pSurfaceFactory)
			return false;

		// Gauge: white rim, blue band with a wave on its lower edge, orange band, transparent below.

		{
			const int w = 360, h = 60, nEdges = 3;
			std::vector<spx> samples(nEdges * (w + 1));

			for (int i = 0; i <= w; i++)
			{
				double a = 2 * 3.14159265358979323846 * i / w;
				samples[i] = spx(std::lround(h * 0.12 * 64));
				samples[(w + 1) + i] = spx(std::lround(h * (0.5 + 0.12 * std::sin(a * 5)) * 64));
				samples[2 * (w + 1) + i] = spx(std::lround(h * (0.82 + 0.04 * std::sin(a * 12)) * 64));
			}

			HiColor colors[4] = { HiColor(4096, 4096, 4096, 4096), HiColor(400, 1800, 4000, 3500), HiColor(4000, 2000, 300, 4096), HiColor::Transparent };

			m_pGauge = pEdgemapFactory->createEdgemap(WGBP(Edgemap, _.size = { w, h }, _.segments = 4, _.colors = colors, _.tiling = true),
													  SampleOrigo::Top, samples.data(), nEdges, w + 1);

			Tint_p tints[4];
			tints[1] = Tint::create(HiColor(4096, 4096, 0, 4096), HiColor(0, 4096, 4096, 2500), { 0.f, 0.f }, { 1.f, 0.f });
			tints[2] = Tint::create(WGBP(Tint, _.shape = TintShape::Radial, _.center = { 0.3f, 0.6f },
										 _.stops = { { 0.f, HiColor(4096, 0, 4096, 4096) }, { 1.f, HiColor(0, 4096, 0, 3000) } }));

			m_pTintedGauge = pEdgemapFactory->createEdgemap(WGBP(Edgemap, _.size = { w, h }, _.segments = 4, _.colors = colors, _.tints = tints, _.tiling = true),
															SampleOrigo::Top, samples.data(), nEdges, w + 1);
		}

		// Small edgemap for identity warps and blend modes.

		{
			const int w = 100, h = 80, nEdges = 2;
			std::vector<spx> samples(nEdges * (w + 1));

			for (int i = 0; i <= w; i++)
			{
				samples[i] = spx(std::lround((20 + 10 * std::sin(i * 0.08)) * 64));
				samples[(w + 1) + i] = spx(std::lround((55 + 8 * std::cos(i * 0.11)) * 64));
			}

			HiColor colors[3] = { HiColor(4096, 1000, 1000, 4096), HiColor(1000, 4096, 1000, 2500), HiColor(1000, 1000, 4096, 4096) };

			m_pSmall = pEdgemapFactory->createEdgemap(WGBP(Edgemap, _.size = { w, h }, _.segments = 3, _.colors = colors),
													  SampleOrigo::Top, samples.data(), nEdges, w + 1);
		}

		// Surfaces: a pattern with partly transparent pixels, tiling and not.

		{
			const int w = 64, h = 32;
			std::vector<Color8> pixels(w * h);

			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					bool bChecker = ((x / 8) + (y / 8)) & 1;
					pixels[y * w + x] = Color8(uint8_t(255 - y * 6), uint8_t(x * 4), bChecker ? 40 : 230, uint8_t(255 - ((x * 7 + y * 3) % 5) * 30));
				}
			}

			m_pTiling = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { w, h }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Bilinear, _.tiling = true),
													   (uint8_t*)pixels.data(), PixelFormat::ARGB_8);

			// The same pattern cut to a size that isn't a power of two, tiling.

			const int nw = 60, nh = 28;
			std::vector<Color8> cut(nw * nh);

			for (int y = 0; y < nh; y++)
				for (int x = 0; x < nw; x++)
					cut[y * nw + x] = pixels[y * w + x];

			m_pTilingNPOT = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { nw, nh }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Bilinear, _.tiling = true),
														   (uint8_t*)cut.data(), PixelFormat::ARGB_8);
			m_pTilingNPOTNearest = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { nw, nh }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Nearest, _.tiling = true),
																  (uint8_t*)cut.data(), PixelFormat::ARGB_8);
			m_pNearest = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { w, h }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Nearest),
														(uint8_t*)pixels.data(), PixelFormat::ARGB_8);
			m_pBilinear = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { w, h }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Bilinear),
														 (uint8_t*)pixels.data(), PixelFormat::ARGB_8);
		}

		// A reel strip of 8 slots, each a colored diamond between dark separators, and the same
		// turned on its side as a label. Even slots that get closer towards the rims is what shows
		// a cylinder.

		{
			const int w = 64, h = 256, slot = 32;
			const Color8 hues[8] = { Color8(220, 40, 40, 255), Color8(240, 160, 20, 255), Color8(220, 210, 30, 255), Color8(60, 180, 60, 255),
									 Color8(40, 170, 200, 255), Color8(50, 80, 220, 255), Color8(150, 60, 200, 255), Color8(220, 70, 160, 255) };

			std::vector<Color8> strip(w * h), label(h * w);

			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					int ly = y % slot;
					Color8 c = Color8(235, 228, 210, 255);

					if (ly < 2 || ly >= slot - 2)
						c = Color8(40, 36, 32, 255);
					else if (std::abs(x - w / 2) * slot / w + std::abs(ly - slot / 2) < slot / 2 - 4)
						c = hues[y / slot];

					strip[y * w + x] = c;
					label[x * h + y] = c;
				}
			}

			m_pReelStrip = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { w, h }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Bilinear, _.tiling = true),
														  (uint8_t*)strip.data(), PixelFormat::ARGB_8);
			m_pReelLabel = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { h, w }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Bilinear, _.tiling = true),
														  (uint8_t*)label.data(), PixelFormat::ARGB_8);
		}

		// A floor of checkered tiles with grout, and a card with a border, a diamond and a red mark
		// in its top-left corner, so that turns and mirroring show.

		{
			const int w = 128, h = 128, tile = 32;
			std::vector<Color8> floor(w * h);

			for (int y = 0; y < h; y++)
				for (int x = 0; x < w; x++)
				{
					int tx = x % tile, ty = y % tile;
					bool bGrout = tx < 2 || ty < 2;
					bool bDark = ((x / tile) + (y / tile)) & 1;
					floor[y * w + x] = bGrout ? Color8(70, 66, 60, 255) : bDark ? Color8(150, 60, 50, 255) : Color8(225, 215, 190, 255);
				}

			m_pFloor = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { w, h }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Bilinear, _.tiling = true),
													  (uint8_t*)floor.data(), PixelFormat::ARGB_8);
		}

		{
			const int w = 64, h = 96;
			std::vector<Color8> card(w * h);

			for (int y = 0; y < h; y++)
				for (int x = 0; x < w; x++)
				{
					Color8 c = Color8(240, 236, 225, 255);
					if (x < 3 || y < 3 || x >= w - 3 || y >= h - 3)
						c = Color8(40, 50, 90, 255);
					else if (x >= 7 && x < 17 && y >= 7 && y < 17)
						c = Color8(220, 40, 40, 255);
					else if (std::abs(x - w / 2) * 3 + std::abs(y - h / 2) * 2 < 64)
						c = Color8(40, 120, 200, 255);
					card[y * w + x] = c;
				}

			m_pCard = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { w, h }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Bilinear),
													 (uint8_t*)card.data(), PixelFormat::ARGB_8);
		}

		// A world map of made up continents in equirectangular projection, seamless around, with ice
		// towards the poles, a grid every 30 degrees and a red prime meridian, so that turns show.

		{
			const int w = 256, h = 128;
			const double pi = 3.14159265358979323846;
			std::vector<Color8> map(w * h);

			for (int y = 0; y < h; y++)
				for (int x = 0; x < w; x++)
				{
					double lon = 2 * pi * (x + 0.5) / w, lat = pi * (0.5 - (y + 0.5) / h);
					double land = std::sin(lon * 2 + 1.0) * std::cos(lat * 3) + 0.6 * std::sin(lon * 3 - lat * 4) + 0.4 * std::cos(lon * 5 + lat * 7) - 0.35;

					Color8 c = land > 0.2 ? Color8(150, 130, 80, 255) : land > 0 ? Color8(60, 140, 60, 255) : Color8(30, 70, 160, 255);
					if (std::abs(lat) > pi * 7 / 18)
						c = Color8(235, 240, 245, 255);
					if (x % (w / 12) == 0 || y % (h / 6) == 0)
						c = Color8(220, 220, 220, 255);
					if (x == 0 || x == 1)
						c = Color8(220, 40, 40, 255);
					map[y * w + x] = c;
				}

			m_pMap = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { w, h }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Bilinear, _.tiling = true),
													(uint8_t*)map.data(), PixelFormat::ARGB_8);
			m_pMapNearest = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { w, h }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Nearest, _.tiling = true),
														   (uint8_t*)map.data(), PixelFormat::ARGB_8);
		}

		// Warpmaps, a set for each size of source they are drawn with.

		m_forGauge = _makeWarps(pWarpmapFactory, m_pGauge->pixelSize() * 64);
		m_forSmall = _makeWarps(pWarpmapFactory, m_pSmall->pixelSize() * 64);
		m_forSurface = _makeWarps(pWarpmapFactory, m_pTiling->pixelSize() * 64);
		m_forNPOT = _makeWarps(pWarpmapFactory, m_pTilingNPOT->pixelSize() * 64);
		m_forRect = _makeWarps(pWarpmapFactory, { 64 * 32, 64 * 24 });
		m_forWideRect = _makeWarps(pWarpmapFactory, { 64 * 48, 64 * 32 });
		m_forSmallRect = _makeWarps(pWarpmapFactory, { 64 * 24, 64 * 16 });
		m_forBlendRect = _makeWarps(pWarpmapFactory, { 64 * 40, 64 * 24 });

		m_pIdentity = _gridWarpmap(pWarpmapFactory, m_pSmall->pixelSize(), { 9, 9 }, [](int x, int y) { return CoordSPX(x * 64, y * 64); });
		m_pIdentityBlit = _gridWarpmap(pWarpmapFactory, { 64, 32 }, { 7, 7 }, [](int x, int y) { return CoordSPX(x * 64, y * 64); });

		return m_pGauge && m_pTintedGauge && m_pSmall && m_pTiling && m_pTilingNPOT && m_pTilingNPOTNearest && m_pNearest && m_pBilinear && m_pReelStrip && m_pReelLabel && m_pFloor && m_pCard &&
			   m_forGauge.ring && m_forGauge.wave && m_forGauge.stripes;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pGauge = nullptr;
		m_pTintedGauge = nullptr;
		m_pSmall = nullptr;
		m_pTiling = nullptr;
		m_pTilingNPOT = nullptr;
		m_pTilingNPOTNearest = nullptr;
		m_pNearest = nullptr;
		m_pBilinear = nullptr;
		m_pReelStrip = nullptr;
		m_pReelLabel = nullptr;
		m_pFloor = nullptr;
		m_pCard = nullptr;
		m_forGauge = {};
		m_forSmall = {};
		m_forSurface = {};
		m_forNPOT = {};
		m_forRect = {};
		m_forWideRect = {};
		m_forSmallRect = {};
		m_forBlendRect = {};
		m_pIdentity = nullptr;
		m_pIdentityBlit = nullptr;
		return true;
	}

	bool ring(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 10 }, m_forGauge.ring, m_pGauge);
		pDevice->warpDrawEdgemap({ 64 * 270 + 20, 64 * 20 - 10 }, m_forGauge.smallRing, m_pGauge);			// Rounded to whole pixels.
		pDevice->warpDrawEdgemap({ 64 * 270, 64 * 260 }, m_forGauge.ring, m_pGauge, { 64 * 45, 0 });
		return true;
	}

	bool arcs(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 10 }, m_forGauge.arc, m_pGauge);
		pDevice->warpDrawEdgemap({ 64 * 260, 64 * 10 }, m_forGauge.arc, m_pGauge, { 64 * 100, 0 });
		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 260 }, m_forGauge.arc, m_pGauge, { -64 * 300, 64 * 6 });
		pDevice->warpDrawEdgemap({ 64 * 260, 64 * 260 }, m_forGauge.quarter, m_pGauge);
		return true;
	}

	bool tintedSegments(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 10 }, m_forGauge.ring, m_pTintedGauge);
		pDevice->warpDrawEdgemap({ 64 * 260, 64 * 10 }, m_forGauge.arc, m_pTintedGauge, { 64 * 30, 0 });
		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 260 }, m_forGauge.wave, m_pTintedGauge);
		pDevice->warpDrawEdgemap({ 64 * 260, 64 * 260 }, m_forGauge.stripes, m_pTintedGauge);
		return true;
	}

	bool deviceTint(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, Tint::create(HiColor(4096, 4096, 4096, 4096), HiColor(4096, 1500, 500, 2500), { 0.f, 0.f }, { 1.f, 1.f }));
		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 10 }, m_forGauge.ring, m_pTintedGauge);
		pDevice->setBlitSource(m_pTiling);
		pDevice->warpBlit({ 64 * 260, 64 * 260 }, m_forSurface.ring);
		pDevice->clearTint();

		pDevice->setTintColor(HiColor(2000, 4096, 4096, 3000));
		pDevice->warpDrawEdgemap({ 64 * 260, 64 * 10 }, m_forGauge.arc, m_pGauge);
		pDevice->warpBlit({ 64 * 10, 64 * 260 }, m_forSurface.wave);
		pDevice->clearTintColor();
		return true;
	}

	bool blitRing(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pTiling);
		pDevice->warpBlit({ 64 * 10, 64 * 10 }, m_forSurface.ring);
		pDevice->warpBlit({ 64 * 260, 64 * 10 }, m_forSurface.ring, { 64 * 13, 64 * 5 });
		pDevice->setBlitSource(m_pBilinear);
		pDevice->warpBlit({ 64 * 10, 64 * 260 }, m_forSurface.ring);
		pDevice->warpBlit({ 64 * 260, 64 * 260 }, m_forSurface.smallRing);
		return true;
	}

	bool blitWave(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pNearest);
		pDevice->warpBlit({ 64 * 10, 64 * 10 }, m_forSurface.wave);
		pDevice->setBlitSource(m_pBilinear);
		pDevice->warpBlit({ 64 * 260, 64 * 10 }, m_forSurface.wave);
		pDevice->setBlitSource(m_pTiling);
		pDevice->warpBlit({ 64 * 10, 64 * 260 }, m_forSurface.wave);
		pDevice->warpBlit({ 64 * 260, 64 * 260 }, m_forSurface.stripes);
		return true;
	}

	bool blitSourceRect(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pBilinear);
		pDevice->warpBlit({ 64 * 10, 64 * 10 }, m_forRect.arc, { 64 * 16, 64 * 4, 64 * 32, 64 * 24 });
		pDevice->warpBlit({ 64 * 260, 64 * 10 }, m_forRect.ring, { 64 * 16, 64 * 4, 64 * 32, 64 * 24 }, { 64 * 7, 64 * 3 });
		pDevice->setBlitSource(m_pNearest);
		pDevice->warpBlit({ 64 * 10, 64 * 260 }, m_forWideRect.quarter, { 64 * 8, 0, 64 * 48, 64 * 32 });
		pDevice->warpBlit({ 64 * 260, 64 * 260 }, m_forSmallRect.wave, { 64 * 20, 64 * 8, 64 * 24, 64 * 16 });
		return true;
	}

	bool periodicV(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 10 }, m_forGauge.stripes, m_pGauge);
		pDevice->warpDrawEdgemap({ 64 * 260, 64 * 10 }, m_forGauge.stripes, m_pGauge, { 64 * 33, 64 * 17 });
		pDevice->setBlitSource(m_pTiling);
		pDevice->warpBlit({ 64 * 10, 64 * 260 }, m_forSurface.stripes, { 64 * 5, 64 * 3 });
		return true;
	}

	// Tiling sources whose sizes aren't powers of two, read outside of the source by the wave
	// and repeated by the stripes, with offsets in both directions.

	bool tilingNPOT(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pTilingNPOT);
		pDevice->warpBlit({ 64 * 10, 64 * 10 }, m_forNPOT.wave);
		pDevice->warpBlit({ 64 * 260, 64 * 10 }, m_forNPOT.stripes, { 64 * -17, 64 * 9 });
		pDevice->setBlitSource(m_pTilingNPOTNearest);
		pDevice->warpBlit({ 64 * 10, 64 * 260 }, m_forNPOT.wave, { 64 * 23, 64 * -5 });
		pDevice->warpBlit({ 64 * 260, 64 * 260 }, m_forNPOT.stripes, { 64 * 7, 64 * -13 });
		return true;
	}

	// Warpmaps mapping straight onto the source draw as drawEdgemap() and blit() do.

	bool identity(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->drawEdgemap({ 64 * 10, 64 * 10 }, m_pSmall);
		pDevice->warpDrawEdgemap({ 64 * 130, 64 * 10 }, m_pIdentity, m_pSmall);

		pDevice->setBlitSource(m_pNearest);
		pDevice->blit({ 64 * 10, 64 * 120 });
		pDevice->warpBlit({ 64 * 130, 64 * 120 }, m_pIdentityBlit);

		pDevice->setBlitSource(m_pBilinear);
		pDevice->blit({ 64 * 10, 64 * 170 });
		pDevice->warpBlit({ 64 * 130, 64 * 170 }, m_pIdentityBlit);
		return true;
	}

	bool blendModes(GfxDevice * pDevice, const RectSPX& canvas)
	{
		const BlendMode modes[9] = { BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::Subtract, BlendMode::Multiply,
									 BlendMode::Invert, BlendMode::Min, BlendMode::Max, BlendMode::Morph };

		// Stripes to blend onto.

		for (int i = 0; i < 8; i++)
			pDevice->fill({ 64 * i * 64, 0, 64 * 32, canvas.h }, HiColor(4096 - i * 400, 1000 + i * 300, 2000, 4096));

		pDevice->setMorphFactor(0.4f);

		for (int i = 0; i < 9; i++)
		{
			CoordSPX pos = { 64 * (10 + (i % 3) * 165), 64 * (10 + (i / 3) * 165) };

			pDevice->setBlendMode(modes[i]);
			pDevice->warpDrawEdgemap(pos, m_forSmall.blendRing, m_pSmall);
			pDevice->setBlitSource(m_pBilinear);
			pDevice->warpBlit(pos + CoordSPX(64 * 45, 64 * 45), m_forBlendRect.smallRing, { 64 * 10, 64 * 4, 64 * 40, 64 * 24 });
		}

		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

	// RoundedRectWarps: a band across the outline, a band outside an outline with sharp corners, an
	// arc and a band inside deeper than the corners, which are mitered.

	bool roundedRects(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pFactory = pDevice->warpmapFactory();
		SizeSPX gauge = m_pGauge->pixelSize() * 64;
		SizeSPX surface = m_pTiling->pixelSize() * 64;

		auto pBand = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 240, 240 }, _.sourceSize = gauge, _.rect = { 64 * 30, 64 * 50, 64 * 180, 64 * 140 },
												  _.cornerRadius = 64 * 30, _.topDistance = 64 * 25, _.bottomDistance = -64 * 20), pFactory);
		auto pSharp = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 240, 240 }, _.sourceSize = surface, _.rect = { 64 * 30, 64 * 50, 64 * 180, 64 * 140 },
												   _.topDistance = 64 * 24, _.bottomDistance = 0), pFactory);
		auto pArc = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 240, 240 }, _.sourceSize = gauge, _.rect = { 64 * 30, 64 * 50, 64 * 180, 64 * 140 },
												 _.cornerRadius = 64 * 50, _.topDistance = 64 * 25, _.bottomDistance = -64 * 20, _.start = -0.3f, _.sweep = 0.75f), pFactory);
		auto pFrame = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 240, 240 }, _.sourceSize = surface, _.rect = { 64 * 10, 64 * 30, 64 * 220, 64 * 180 },
												  _.cornerRadius = 64 * 8, _.topDistance = 0, _.bottomDistance = -64 * 36, _.precision = 10.f), pFactory);

		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 10 }, pBand->generate(), m_pGauge);
		pDevice->setBlitSource(m_pTiling);
		pDevice->warpBlit({ 64 * 260, 64 * 10 }, pSharp->generate());
		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 260 }, pArc->generate(), m_pTintedGauge);
		pDevice->warpBlit({ 64 * 260, 64 * 260 }, pFrame->generate());
		return true;
	}

	// Top: the reel strip flat, then wrapped around reels: plain, turned and shaded darker towards
	// the rims as a cylinder would be drawn, and one seen almost to its rims. Bottom: the strip on
	// its side around a standing cylinder, plain and turned and shaded.

	bool cylinders(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pFactory = pDevice->warpmapFactory();
		SizeSPX strip = m_pReelStrip->pixelSize() * 64;
		SizeSPX label = m_pReelLabel->pixelSize() * 64;

		auto pReel = CylinderWarp::create(WGBP(CylinderWarp, _.size = { 90, 240 }, _.sourceSize = strip, _.span = 110.f), pFactory);
		auto pWide = CylinderWarp::create(WGBP(CylinderWarp, _.size = { 90, 240 }, _.sourceSize = strip, _.span = 170.f), pFactory);
		auto pStanding = CylinderWarp::create(WGBP(CylinderWarp, _.size = { 240, 110 }, _.sourceSize = label, _.axis = Axis::Y, _.span = 150.f), pFactory);

		HiColor rim(1200, 1200, 1300, 4096), light = HiColor::White;
		auto shadeAcross = [&](CoordSPX pos, SizeI size, bool bVertical)
		{
			pDevice->setTint({ pos, size * 64 }, Tint::create({ { 0.f, rim }, { 0.5f, light }, { 1.f, rim } }, { 0.f, 0.f }, bVertical ? CoordF(0.f, 1.f) : CoordF(1.f, 0.f)));
		};

		// The strip flat, for comparison.

		pDevice->setBlitSource(m_pReelStrip);
		pDevice->blit({ 64 * 10, 64 * 10 }, { 0, 0, 64 * 64, 64 * 240 });

		// Reels.

		pDevice->warpBlit({ 64 * 90, 64 * 10 }, pReel->generate());

		shadeAcross({ 64 * 195, 64 * 10 }, { 90, 240 }, true);
		pDevice->warpBlit({ 64 * 195, 64 * 10 }, pReel->generate(), pReel->rotationOffset(20.f));

		shadeAcross({ 64 * 300, 64 * 10 }, { 90, 240 }, true);
		pDevice->warpBlit({ 64 * 300, 64 * 10 }, pWide->generate(), pWide->rotationOffset(20.f));
		pDevice->clearTint();

		// Standing.

		pDevice->setBlitSource(m_pReelLabel);
		pDevice->warpBlit({ 64 * 10, 64 * 270 }, pStanding->generate());

		shadeAcross({ 64 * 260, 64 * 270 }, { 240, 110 }, false);
		pDevice->warpBlit({ 64 * 260, 64 * 270 }, pStanding->generate(), pStanding->rotationOffset(20.f));
		pDevice->clearTint();
		return true;
	}

	// Discs with their outline going from full coverage half a pixel inside to none half a pixel
	// outside, which antialiases it, and with half coverage everywhere.

	bool vertexCoverage(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto disc = [&](SizeSPX source, int radius, int innerCoverage, int outerCoverage)
		{
			const int nSides = 72;
			const double c = radius + 1.3;

			std::vector<Warpmap::Vertex> vertices;
			std::vector<Warpmap::Triangle> triangles;

			auto add = [&](double x, double y, int coverage)
			{
				spx sx = spx(std::lround(x * 64)), sy = spx(std::lround(y * 64));
				vertices.push_back({ sx, sy, spx(int64_t(sx) * source.w / (64 * 2 * (radius + 2))), spx(int64_t(sy) * source.h / (64 * 2 * (radius + 2))), Warpmap::weightOne, coverage });
				return int(vertices.size()) - 1;
			};

			int center = add(c, c, innerCoverage);
			for (int i = 0; i < nSides; i++)
			{
				double a = 2 * 3.14159265358979323846 * i / nSides;
				add(c + std::cos(a) * (radius - 0.5), c + std::sin(a) * (radius - 0.5), innerCoverage);
				add(c + std::cos(a) * (radius + 0.5), c + std::sin(a) * (radius + 0.5), outerCoverage);
			}

			for (int i = 0; i < nSides; i++)
			{
				int in0 = 1 + i * 2, out0 = in0 + 1, in1 = 1 + ((i + 1) % nSides) * 2, out1 = in1 + 1;
				triangles.push_back({ center, in0, in1 });
				triangles.push_back({ in0, out0, out1 });
				triangles.push_back({ in0, out1, in1 });
			}

			int size = 2 * (radius + 2);
			return pDevice->warpmapFactory()->createWarpmap(WGBP(Warpmap, _.size = { size, size }, _.triangles = int(triangles.size()), _.vertices = int(vertices.size())),
															 vertices.data(), triangles.data());
		};

		SizeSPX gauge = m_pGauge->pixelSize() * 64, surface = m_pBilinear->pixelSize() * 64;

		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 10 }, disc(gauge, 110, Warpmap::coverageOne, 0), m_pGauge);
		pDevice->setBlitSource(m_pBilinear);
		pDevice->warpBlit({ 64 * 260, 64 * 10 }, disc(surface, 110, Warpmap::coverageOne, 0));
		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 260 + 20 }, disc(gauge, 57, Warpmap::coverageOne / 2, Warpmap::coverageOne / 2), m_pGauge);
		pDevice->warpBlit({ 64 * 260 + 30, 64 * 260 }, disc(surface, 13, Warpmap::coverageOne, 0));
		return true;
	}

	// Top: a tiled floor leaning into the distance, seen from above and scrolled along with a source
	// offset, and the gauge leaning back like credits. Middle: cards in a row seen from one eye, turned towards the
	// middle. Bottom: a card turned 0, 60 and 120 degrees around its middle (the last one mirrored,
	// its back facing the viewer), one tilted, turned and rolled, and one pinned onto a free shape.

	bool perspective(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pFactory = pDevice->warpmapFactory();
		SizeSPX cardSize = m_pCard->pixelSize() * 64;

		// Floor, tilted back around its front edge and seen from above, so it goes towards a horizon
		// at the top. The source rectangle covers many tiles of the tiling surface.

		const SizeSPX floorSource = { 64 * 512, 64 * 256 };

		auto pFloor = PerspectiveWarp::create(WGBP(PerspectiveWarp, _.size = { 300, 150 }, _.sourceSize = floorSource,
			_.corners = PerspectiveWarp::rotated({ 0, 0, 64 * 300, 64 * 150 }, 70.f, 0.f, 0.f, 64 * 150, { 0.5f, 1.f }, CoordSPX(64 * 150, 0))), pFactory);

		pDevice->setBlitSource(m_pFloor);
		pDevice->warpBlit({ 64 * 10, 64 * 10 }, pFloor->generate(), { 0, 0, floorSource.w, floorSource.h }, { 0, 64 * 40 });

		// Credits, likewise.

		auto pCredits = PerspectiveWarp::create(WGBP(PerspectiveWarp, _.size = { 180, 150 }, _.sourceSize = m_pGauge->pixelSize() * 64,
			_.corners = PerspectiveWarp::rotated({ 0, 64 * 60, 64 * 180, 64 * 90 }, 60.f, 0.f, 0.f, 64 * 150, { 0.5f, 1.f }, CoordSPX(64 * 90, 0))), pFactory);

		pDevice->warpDrawEdgemap({ 64 * 320, 64 * 10 }, pCredits->generate(), m_pGauge);

		// A row of cards seen from one eye in the middle of the row, the outer ones turned away.

		pDevice->setBlitSource(m_pCard);

		const SizeI row = { 490, 140 };
		const CoordSPX eye = { 64 * row.w / 2, 64 * 70 };
		const float turns[5] = { -25.f, -15.f, 0.f, 15.f, 25.f };

		for (int i = 0; i < 5; i++)
		{
			RectSPX rect = { 64 * (20 + i * 95), 64 * 22, cardSize.w, cardSize.h };
			auto pCard = PerspectiveWarp::create(WGBP(PerspectiveWarp, _.size = row, _.sourceSize = cardSize,
				_.corners = PerspectiveWarp::rotated(rect, 0.f, turns[i], 0.f, 64 * 300, { 0.5f, 0.5f }, eye)), pFactory);

			pDevice->warpBlit({ 64 * 10, 64 * 175 }, pCard->generate());
		}

		// Turned, turned and rolled, and pinned.

		const SizeI cell = { 95, 160 };
		const RectSPX cardRect = { 64 * 15, 64 * 32, cardSize.w, cardSize.h };

		CornersT<CoordSPX> shapes[5] = {
			PerspectiveWarp::rotated(cardRect, 0.f, 0.f, 0.f, 64 * 250),
			PerspectiveWarp::rotated(cardRect, 0.f, 60.f, 0.f, 64 * 250),
			PerspectiveWarp::rotated(cardRect, 0.f, 120.f, 0.f, 64 * 250),
			PerspectiveWarp::rotated(cardRect, 35.f, 25.f, 20.f, 64 * 200),
			CornersT<CoordSPX>({ 64 * 8, 64 * 40 }, { 64 * 86, 64 * 14 }, { 64 * 80, 64 * 150 }, { 64 * 22, 64 * 118 })
		};

		for (int i = 0; i < 5; i++)
		{
			auto pCard = PerspectiveWarp::create(WGBP(PerspectiveWarp, _.size = cell, _.sourceSize = cardSize, _.corners = shapes[i]), pFactory);
			pDevice->warpBlit({ 64 * (10 + i * 100), 64 * 335 }, pCard->generate());
		}
		return true;
	}

	// Top: the map flat, a globe with the north pole leaning towards the viewer and lit from the
	// top left, and one leaning away, rolled and turned. Bottom: the gauge around a ball, globes seen
	// from straight above and below the poles, and globes getting smaller, turned by offset.

	bool balls(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pFactory = pDevice->warpmapFactory();
		SizeSPX map = m_pMap->pixelSize() * 64;

		auto globe = [&](int size, float tilt, float roll = 0.f)
		{
			return BallWarp::create(WGBP(BallWarp, _.size = { size, size }, _.sourceSize = map, _.tilt = tilt, _.roll = roll), pFactory);
		};

		pDevice->setBlitSource(m_pMap);
		pDevice->blit({ 64 * 10, 64 * 10 }, { 0, 0, map.w, map.h });

		auto pEarth = globe(200, 23.5f);
		pDevice->setTint({ { 64 * 270, 64 * 10 }, SizeSPX(64 * 200, 64 * 200) },
						 Tint::create({ { 0.f, HiColor::White }, { 1.f, HiColor(900, 900, 1100, 4096) } }, { 0.f, 0.f }, { 1.f, 1.f }));
		pDevice->warpBlit({ 64 * 270, 64 * 10 }, pEarth->generate(), pEarth->rotationOffset(30.f));
		pDevice->clearTint();

		auto pRolled = globe(110, -40.f, 30.f);
		pDevice->warpBlit({ 64 * 140, 64 * 150 }, pRolled->generate(), pRolled->rotationOffset(120.f));

		auto pMarble = BallWarp::create(WGBP(BallWarp, _.size = { 200, 200 }, _.sourceSize = m_pGauge->pixelSize() * 64, _.tilt = 60.f, _.roll = -20.f), pFactory);
		pDevice->warpDrawEdgemap({ 64 * 10, 64 * 270 }, pMarble->generate(), m_pGauge);

		auto pNorth = globe(110, 90.f), pSouth = globe(110, -90.f);
		pDevice->warpBlit({ 64 * 230, 64 * 270 }, pNorth->generate());
		pDevice->warpBlit({ 64 * 350, 64 * 270 }, pSouth->generate(), pSouth->rotationOffset(45.f));

		int x = 230;
		for (int size : { 80, 50, 30, 16, 8 })
		{
			auto pSmall = globe(size, 23.5f);
			pDevice->warpBlit({ 64 * x, 64 * 400 }, pSmall->generate(), pSmall->rotationOffset(float(size) * 3));
			x += size + 10;
		}
		return true;
	}

	// Top: floor tiles under, from the left, a magnifying glass, thick glass, a bubble and a strong
	// bubble, and smaller lenses below them. Bottom: the world map under a pixel loupe, a glass, a
	// bubble and a large thick glass, and the gauge with part of it magnified beside it. The
	// tiles and the map are drawn straight from their surfaces, so what is under a lens is the same
	// rectangle of the surface.

	bool lenses(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pFactory = pDevice->warpmapFactory();

		auto lens = [&](int size, float magnification, float falloff)
		{
			return LensWarp::create(WGBP(LensWarp, _.size = { size, size }, _.sourceSize = SizeSPX(64 * size, 64 * size),
										 _.magnification = magnification, _.falloff = falloff), pFactory);
		};

		auto over = [&](CoordI pos, CoordI origin, LensWarp * pLens)
		{
			SizeI size = pLens->size();
			pDevice->warpBlit(pos * 64, pLens->generate(), { 64 * (pos.x - origin.x), 64 * (pos.y - origin.y), 64 * size.w, 64 * size.h });
		};

		pDevice->setBlitSource(m_pFloor);
		pDevice->tile({ 0, 0, 64 * 512, 64 * 250 });

		over({ 15, 15 }, { 0, 0 }, lens(110, 2.f, 0.f));
		over({ 135, 15 }, { 0, 0 }, lens(110, 3.f, 0.3f));
		over({ 255, 15 }, { 0, 0 }, lens(110, 2.5f, 1.f));
		over({ 375, 15 }, { 0, 0 }, lens(110, 6.f, 0.6f));

		int x = 30;
		for (int size : { 80, 50, 30, 16 })
		{
			over({ x, 140 }, { 0, 0 }, lens(size, 2.f, 0.3f));
			x += size + 40;
		}

		const CoordI mapOrigin = { 0, 256 };

		pDevice->setBlitSource(m_pMap);
		pDevice->tile({ 0, 64 * 256, 64 * 512, 64 * 176 });

		pDevice->setBlitSource(m_pMapNearest);
		over({ 15, 290 }, mapOrigin, lens(100, 6.f, 0.f));

		pDevice->setBlitSource(m_pMap);
		over({ 135, 290 }, mapOrigin, lens(100, 2.f, 0.2f));
		over({ 255, 290 }, mapOrigin, lens(100, 3.f, 1.f));
		over({ 375, 280 }, mapOrigin, lens(120, 1.5f, 0.5f));

		// The gauge flat, and a part of it magnified beside it, still sharp. Its blue is translucent,
		// so a lens over it would blend it twice.

		pDevice->drawEdgemap({ 64 * 10, 64 * 444 }, m_pGauge);
		auto pGaugeLens = LensWarp::create(WGBP(LensWarp, _.size = { 60, 60 }, _.sourceSize = SizeSPX(64 * 60, 64 * 60), _.magnification = 3.f, _.falloff = 0.4f), pFactory);
		pDevice->warpDrawEdgemap({ 64 * 400, 64 * 444 }, pGaugeLens->generate(), m_pGauge, { 64 * 150, 0 });
		return true;
	}

private:

	Edgemap_p	m_pGauge;
	Edgemap_p	m_pTintedGauge;
	Edgemap_p	m_pSmall;

	Surface_p	m_pTiling;
	Surface_p	m_pTilingNPOT;
	Surface_p	m_pTilingNPOTNearest;
	Surface_p	m_pNearest;
	Surface_p	m_pBilinear;
	Surface_p	m_pReelStrip;
	Surface_p	m_pReelLabel;
	Surface_p	m_pFloor;
	Surface_p	m_pCard;
	Surface_p	m_pMap;
	Surface_p	m_pMapNearest;

	// Warpmaps are made for a source of a certain size.

	struct Warps
	{
		Warpmap_p	ring;
		Warpmap_p	smallRing;
		Warpmap_p	arc;
		Warpmap_p	quarter;
		Warpmap_p	blendRing;
		Warpmap_p	wave;				// A smooth bend of a rectangle, partly outside the source.
		Warpmap_p	stripes;			// Periodic along v, the source repeated three times down a sloping band.
	};

	// A Warpmap from a grid of cells, with the position in the source at each node from nodeAt(x, y)
	// (pixels of the Warpmap), four vertices and two triangles for each cell.

	static Warpmap_p _gridWarpmap(WarpmapFactory * pFactory, SizeI size, SizeI cellSize, const std::function<CoordSPX(int x, int y)>& nodeAt, spx vPeriod = 0)
	{
		std::vector<Warpmap::Vertex> vertices;
		std::vector<Warpmap::Triangle> triangles;

		for (int y0 = 0; y0 < size.h; y0 += cellSize.h)
			for (int x0 = 0; x0 < size.w; x0 += cellSize.w)
			{
				int x1 = std::min(x0 + cellSize.w, size.w), y1 = std::min(y0 + cellSize.h, size.h);
				const CoordI corners[4] = { { x0, y0 }, { x1, y0 }, { x1, y1 }, { x0, y1 } };

				int first = int(vertices.size());
				for (auto& c : corners)
				{
					CoordSPX uv = nodeAt(c.x, c.y);
					vertices.push_back({ c.x * 64, c.y * 64, uv.x, uv.y });
				}

				triangles.push_back({ first, first + 1, first + 2 });
				triangles.push_back({ first, first + 2, first + 3 });
			}

		return pFactory->createWarpmap(WGBP(Warpmap, _.size = size, _.triangles = int(triangles.size()), _.vertices = int(vertices.size()), _.vPeriod = vPeriod),
									   vertices.data(), triangles.data());
	}

	Warps _makeWarps(WarpmapFactory * pFactory, SizeSPX source)
	{
		Warps warps;

		warps.ring = CircularWarp::create(WGBP(CircularWarp, _.size = { 240, 240 }, _.sourceSize = source, _.topRadius = 118 * 64, _.bottomRadius = 60 * 64), pFactory)->generate();
		warps.smallRing = CircularWarp::create(WGBP(CircularWarp, _.size = { 120, 120 }, _.sourceSize = source, _.topRadius = 58 * 64, _.bottomRadius = 25 * 64, _.precision = 1.f), pFactory)->generate();
		warps.arc = CircularWarp::create(WGBP(CircularWarp, _.size = { 240, 240 }, _.sourceSize = source, _.topRadius = 118 * 64, _.bottomRadius = 80 * 64, _.startAngle = -135.f, _.sweep = 270.f), pFactory)->generate();
		warps.quarter = CircularWarp::create(WGBP(CircularWarp, _.size = { 240, 240 }, _.sourceSize = source, _.center = { 0.f, 1.f }, _.topRadius = 230 * 64, _.bottomRadius = 150 * 64, _.sweep = 90.f), pFactory)->generate();
		warps.blendRing = CircularWarp::create(WGBP(CircularWarp, _.size = { 150, 150 }, _.sourceSize = source, _.topRadius = 74 * 64, _.bottomRadius = 30 * 64), pFactory)->generate();

		warps.wave = _gridWarpmap(pFactory, { 240, 240 }, { 12, 12 }, [&](int px, int py)
		{
			float x = px / 240.f;
			float y = py / 240.f;
			float u = (x - 0.5f) * 1.2f + 0.5f + 0.08f * std::sin(y * 7.f);
			float v = (y - 0.5f) * 1.15f + 0.5f + 0.06f * std::sin(x * 6.f);
			return CoordSPX(spx(std::lround(u * source.w)), spx(std::lround(v * source.h)));
		});

		// v goes on past the period, it is wrapped after interpolation.

		warps.stripes = _gridWarpmap(pFactory, { 200, 240 }, { 10, 10 }, [&](int px, int py)
		{
			float x = px / 200.f;
			float y = py / 240.f;
			return CoordSPX(spx(std::lround(x * source.w)), spx(std::lround((y * 3.f + x * 0.7f) * source.h)));
		}, source.h);

		return warps;
	}

	Warps		m_forGauge;
	Warps		m_forSmall;
	Warps		m_forSurface;
	Warps		m_forNPOT;
	Warps		m_forRect;
	Warps		m_forWideRect;
	Warps		m_forSmallRect;
	Warps		m_forBlendRect;

	Warpmap_p	m_pIdentity;
	Warpmap_p	m_pIdentityBlit;
};
