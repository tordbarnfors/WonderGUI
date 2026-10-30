#include "testsuite.h"

#include <wg_tint.h>
#include <wg_circularwarp.h>

#include <cmath>
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
		addTest("Identity", &WarpTests::identity);
		addTest("BlendModes", &WarpTests::blendModes);
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
			m_pNearest = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { w, h }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Nearest),
														(uint8_t*)pixels.data(), PixelFormat::ARGB_8);
			m_pBilinear = pSurfaceFactory->createSurface(WGBP(Surface, _.size = { w, h }, _.format = PixelFormat::ARGB_8, _.sampleMethod = SampleMethod::Bilinear),
														 (uint8_t*)pixels.data(), PixelFormat::ARGB_8);
		}

		// Warpmaps, a set for each size of source they are drawn with.

		m_forGauge = _makeWarps(pWarpmapFactory, m_pGauge->pixelSize() * 64);
		m_forSmall = _makeWarps(pWarpmapFactory, m_pSmall->pixelSize() * 64);
		m_forSurface = _makeWarps(pWarpmapFactory, m_pTiling->pixelSize() * 64);
		m_forRect = _makeWarps(pWarpmapFactory, { 64 * 32, 64 * 24 });
		m_forWideRect = _makeWarps(pWarpmapFactory, { 64 * 48, 64 * 32 });
		m_forSmallRect = _makeWarps(pWarpmapFactory, { 64 * 24, 64 * 16 });
		m_forBlendRect = _makeWarps(pWarpmapFactory, { 64 * 40, 64 * 24 });

		m_pIdentity = pWarpmapFactory->createWarpmap(WGBP(Warpmap, _.size = m_pSmall->pixelSize(), _.cellSize = 9));
		m_pIdentityBlit = pWarpmapFactory->createWarpmap(WGBP(Warpmap, _.size = { 64, 32 }, _.cellSize = 7));

		return m_pGauge && m_pTintedGauge && m_pSmall && m_pTiling && m_pNearest && m_pBilinear && m_forGauge.ring && m_forGauge.wave && m_forGauge.stripes;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pGauge = nullptr;
		m_pTintedGauge = nullptr;
		m_pSmall = nullptr;
		m_pTiling = nullptr;
		m_pNearest = nullptr;
		m_pBilinear = nullptr;
		m_forGauge = {};
		m_forSmall = {};
		m_forSurface = {};
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

private:

	Edgemap_p	m_pGauge;
	Edgemap_p	m_pTintedGauge;
	Edgemap_p	m_pSmall;

	Surface_p	m_pTiling;
	Surface_p	m_pNearest;
	Surface_p	m_pBilinear;

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

	Warps _makeWarps(WarpmapFactory * pFactory, SizeSPX source)
	{
		Warps warps;

		warps.ring = CircularWarp::create(WGBP(CircularWarp, _.size = { 240, 240 }, _.sourceSize = source, _.topRadius = 118 * 64, _.bottomRadius = 60 * 64), pFactory)->refresh();
		warps.smallRing = CircularWarp::create(WGBP(CircularWarp, _.size = { 120, 120 }, _.sourceSize = source, _.topRadius = 58 * 64, _.bottomRadius = 25 * 64, _.cellSize = 8), pFactory)->refresh();
		warps.arc = CircularWarp::create(WGBP(CircularWarp, _.size = { 240, 240 }, _.sourceSize = source, _.topRadius = 118 * 64, _.bottomRadius = 80 * 64, _.startAngle = -135.f, _.sweep = 270.f), pFactory)->refresh();
		warps.quarter = CircularWarp::create(WGBP(CircularWarp, _.size = { 240, 240 }, _.sourceSize = source, _.center = { 0.f, 1.f }, _.topRadius = 230 * 64, _.bottomRadius = 150 * 64, _.sweep = 90.f), pFactory)->refresh();
		warps.blendRing = CircularWarp::create(WGBP(CircularWarp, _.size = { 150, 150 }, _.sourceSize = source, _.topRadius = 74 * 64, _.bottomRadius = 30 * 64), pFactory)->refresh();

		{
			warps.wave = pFactory->createWarpmap(WGBP(Warpmap, _.size = { 240, 240 }, _.cellSize = 12));

			SizeI n = warps.wave->nodes();
			std::vector<CoordSPX> nodes(n.w * n.h);

			for (int r = 0; r < n.h; r++)
			{
				for (int c = 0; c < n.w; c++)
				{
					float x = warps.wave->nodeX(c) / 240.f;
					float y = warps.wave->nodeY(r) / 240.f;
					float u = (x - 0.5f) * 1.2f + 0.5f + 0.08f * std::sin(y * 7.f);
					float v = (y - 0.5f) * 1.15f + 0.5f + 0.06f * std::sin(x * 6.f);
					nodes[r * n.w + c] = { spx(std::lround(u * source.w)), spx(std::lround(v * source.h)) };
				}
			}

			warps.wave->importNodes({ 0, 0, n.w, n.h }, nodes.data());
		}

		{
			warps.stripes = pFactory->createWarpmap(WGBP(Warpmap, _.size = { 200, 240 }, _.cellSize = 10, _.vPeriod = source.h));

			SizeI n = warps.stripes->nodes();
			std::vector<CoordSPX> nodes(n.w * n.h);

			for (int r = 0; r < n.h; r++)
			{
				for (int c = 0; c < n.w; c++)
				{
					float x = warps.stripes->nodeX(c) / 200.f;
					float y = warps.stripes->nodeY(r) / 240.f;
					nodes[r * n.w + c] = { spx(std::lround(x * source.w)), spx(std::lround((y * 3.f + x * 0.7f) * source.h)) };
				}
			}

			warps.stripes->importNodes({ 0, 0, n.w, n.h }, nodes.data());
		}

		return warps;
	}

	Warps		m_forGauge;
	Warps		m_forSmall;
	Warps		m_forSurface;
	Warps		m_forRect;
	Warps		m_forWideRect;
	Warps		m_forSmallRect;
	Warps		m_forBlendRect;

	Warpmap_p	m_pIdentity;
	Warpmap_p	m_pIdentityBlit;
};
