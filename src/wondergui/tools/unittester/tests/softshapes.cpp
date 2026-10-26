#include "softshapes.h"
#include "shapecoverage.h"

#include <wax_gfxutil.h>

#include <cmath>
#include <cstdlib>
#include <climits>
#include <algorithm>

using namespace wax;

namespace
{
	//____ CoverageGrid _______________________________________________________
	// Fill and outline coverage of a rounded box.

	class CoverageGrid
	{
	public:
		CoverageGrid(const RectI& area) : m_coverage(area, 2) {}

		void rasterize(SoftShapes& shapes, const SoftShapes::RoundedBox& box, const RectI& clip)
		{
			m_coverage.collect([&](SoftShapes::RowSink* pSink) { shapes.rasterizeRoundedBox(box, clip, pSink); });
		}

		int		fill(int x, int y) const { return m_coverage.coverage(0, x, y); }
		int		outline(int x, int y) const { return m_coverage.coverage(1, x, y); }

		bool			hasError() const { return m_coverage.hasError(); }
		std::string		error() const { return m_coverage.error(); }

	private:
		ShapeCoverage	m_coverage;
	};

	//____ Reference geometry _________________________________________________
	// Exact rounded rectangles in pixels, sampled.

	struct RefShape
	{
		bool	bEmpty = true;
		double	left, top, right, bottom;
		double	r[4];

		bool contains(double x, double y) const
		{
			if (bEmpty || x < left || x >= right || y < top || y >= bottom)
				return false;

			auto outsideCorner = [&](double cx, double cy, double radius, bool bLeftOf, bool bAbove)
			{
				bool bInSquare = (bLeftOf ? x < cx : x >= cx) && (bAbove ? y < cy : y >= cy);
				return bInSquare && (x - cx) * (x - cx) + (y - cy) * (y - cy) > radius * radius;
			};

			if (outsideCorner(left + r[0], top + r[0], r[0], true, true) ||
				outsideCorner(right - r[1], top + r[1], r[1], false, true) ||
				outsideCorner(right - r[2], bottom - r[2], r[2], false, false) ||
				outsideCorner(left + r[3], bottom - r[3], r[3], true, false))
				return false;

			return true;
		}

		// The shape is convex, so a square with all corners inside is inside.

		bool containsSquare(double x1, double y1, double x2, double y2) const
		{
			const double e = 1e-9;
			return contains(x1 + e, y1 + e) && contains(x2 - e, y1 + e) && contains(x1 + e, y2 - e) && contains(x2 - e, y2 - e);
		}

		double coverage(int px, int py, int samples = 64) const
		{
			if (containsSquare(px, py, px + 1, py + 1))
				return 1.0;

			int inside = 0;
			for (int sy = 0; sy < samples; sy++)
				for (int sx = 0; sx < samples; sx++)
					if (contains(px + (sx + 0.5) / samples, py + (sy + 0.5) / samples))
						inside++;

			return inside / double(samples * samples);
		}
	};

	struct RefBox
	{
		RefShape	outer;
		RefShape	inner;
	};

	RefBox makeReference(const SoftShapes::RoundedBox& box)
	{
		RefBox ref;

		spx radius[4] = { box.radius[0], box.radius[1], box.radius[2], box.radius[3] };
		GfxUtil::clampCornerRadii(box.rect.size(), radius);

		const RectSPX& r = box.rect;
		const BorderSPX& o = box.outline;

		ref.outer.bEmpty = (r.w <= 0 || r.h <= 0);
		ref.outer.left = r.x / 64.0;
		ref.outer.top = r.y / 64.0;
		ref.outer.right = (r.x + r.w) / 64.0;
		ref.outer.bottom = (r.y + r.h) / 64.0;

		for (int i = 0; i < 4; i++)
			ref.outer.r[i] = radius[i] / 64.0;

		RectSPX in(r.x + o.left, r.y + o.top, r.w - o.left - o.right, r.h - o.top - o.bottom);

		ref.inner.bEmpty = (in.w <= 0 || in.h <= 0);
		ref.inner.left = in.x / 64.0;
		ref.inner.top = in.y / 64.0;
		ref.inner.right = (in.x + in.w) / 64.0;
		ref.inner.bottom = (in.y + in.h) / 64.0;

		spx innerRadius[4];
		GfxUtil::innerCornerRadii(radius, o, innerRadius);

		for (int i = 0; i < 4; i++)
			ref.inner.r[i] = innerRadius[i] / 64.0;

		return ref;
	}

	RectI pixelBounds(const RectSPX& rect, int margin)
	{
		int x1 = (rect.x >> 6) - margin;
		int y1 = (rect.y >> 6) - margin;
		int x2 = ((rect.x + rect.w + 63) >> 6) + margin;
		int y2 = ((rect.y + rect.h + 63) >> 6) + margin;
		return RectI(x1, y1, x2 - x1, y2 - y1);
	}

	//____ Test cases _________________________________________________________

	struct BoxCase
	{
		const char*				name;
		SoftShapes::RoundedBox	box;
		double					tolerance;			// Max difference from reference, 0.0 -> 1.0.
		RectI					area = {};			// Pixels to check, all of box if empty.
	};

	RectI caseArea(const BoxCase& c)
	{
		return c.area.w > 0 ? c.area : pixelBounds(c.box.rect, 1);
	}

	SoftShapes::RoundedBox makeBox(RectSPX rect, spx r0, spx r1, spx r2, spx r3, BorderSPX outline = {})
	{
		SoftShapes::RoundedBox box;
		box.rect = rect;
		box.radius[0] = r0;
		box.radius[1] = r1;
		box.radius[2] = r2;
		box.radius[3] = r3;
		box.outline = outline;
		return box;
	}

	std::vector<BoxCase> boxCases()
	{
		std::vector<BoxCase> cases;

		cases.push_back({ "aligned square box", makeBox({ 64 * 2, 64 * 3, 64 * 20, 64 * 10 }, 0, 0, 0, 0), 0.02 });
		cases.push_back({ "subpixel square box", makeBox({ 64 * 2 + 17, 64 * 3 + 40, 64 * 20 + 9, 64 * 10 + 50 }, 0, 0, 0, 0), 0.02 });
		cases.push_back({ "subpixel box, outline", makeBox({ 64 * 2 + 17, 64 * 3 + 40, 64 * 20 + 9, 64 * 10 + 50 }, 0, 0, 0, 0, { 64 + 20, 30, 64 * 2 + 5, 10 }), 0.02 });
		cases.push_back({ "tiny box", makeBox({ 64 * 4 + 10, 64 * 4 + 20, 30, 20 }, 0, 0, 0, 0), 0.02 });
		cases.push_back({ "thin horizontal box", makeBox({ 64 * 4 + 10, 64 * 4 + 20, 64 * 12 + 3, 20 }, 0, 0, 0, 0), 0.02 });

		cases.push_back({ "rounded, aligned", makeBox({ 64 * 2, 64 * 2, 64 * 40, 64 * 30 }, 64 * 8, 64 * 8, 64 * 8, 64 * 8), 0.04 });
		cases.push_back({ "rounded, subpixel", makeBox({ 64 * 2 + 21, 64 * 2 + 50, 64 * 40 + 13, 64 * 30 + 7 }, 64 * 8 + 11, 64 * 8 + 11, 64 * 8 + 11, 64 * 8 + 11), 0.04 });
		cases.push_back({ "mixed radii", makeBox({ 64 * 2 + 5, 64 * 2 + 33, 64 * 50 + 13, 64 * 30 + 7 }, 64 * 3 + 20, 64 * 12, 0, 64 * 25), 0.04 });
		cases.push_back({ "small radii", makeBox({ 64 * 2 + 5, 64 * 2 + 33, 64 * 20 + 13, 64 * 10 + 7 }, 40, 64, 100, 140), 0.04 });
		cases.push_back({ "pill", makeBox({ 64 * 3 + 10, 64 * 3 + 30, 64 * 60, 64 * 16 + 30 }, 64 * 100, 64 * 100, 64 * 100, 64 * 100), 0.04 });
		cases.push_back({ "circle", makeBox({ 64 * 3 + 10, 64 * 3 + 30, 64 * 31 + 20, 64 * 31 + 20 }, 64 * 100, 64 * 100, 64 * 100, 64 * 100), 0.04 });
		cases.push_back({ "tall pill, outline", makeBox({ 64 * 3 + 10, 64 * 3 + 30, 64 * 12 + 30, 64 * 40 }, 64 * 100, 64 * 100, 64 * 100, 64 * 100, { 64 * 2 + 10, 64 * 2 + 10, 64 * 2 + 10, 64 * 2 + 10 }), 0.04 });

		// Symmetric around a pixel boundary or the middle of a pixel, rows done half and mirrored.

		cases.push_back({ "aligned, odd width, outline", makeBox({ 64 * 2, 64 * 2, 64 * 41, 64 * 30 }, 64 * 8, 64 * 8, 64 * 8, 64 * 8, { 64, 64, 64, 64 }), 0.04 });
		cases.push_back({ "half pixel, outline", makeBox({ 64 * 2 + 32, 64 * 2 + 32, 64 * 40, 64 * 24 }, 64 * 6, 64 * 6, 64 * 6, 64 * 6, { 32, 32, 32, 32 }), 0.04 });
		cases.push_back({ "aligned circle, odd width", makeBox({ 64 * 3, 64 * 3, 64 * 31, 64 * 31 }, 64 * 100, 64 * 100, 64 * 100, 64 * 100), 0.04 });
		cases.push_back({ "half pixel pill, outline", makeBox({ 64 * 3 + 32, 64 * 3, 64 * 40, 64 * 15 }, 64 * 100, 64 * 100, 64 * 100, 64 * 100, { 64 * 2, 64 * 2, 64 * 2, 64 * 2 }), 0.04 });

		cases.push_back({ "rounded, 1px outline", makeBox({ 64 * 2 + 21, 64 * 2 + 50, 64 * 120, 64 * 32 }, 64 * 6, 64 * 6, 64 * 6, 64 * 6, { 64, 64, 64, 64 }), 0.04 });
		cases.push_back({ "rounded, subpixel outline", makeBox({ 64 * 2 + 21, 64 * 2 + 50, 64 * 40, 64 * 32 }, 64 * 6, 64 * 6, 64 * 6, 64 * 6, { 23, 23, 23, 23 }), 0.04 });
		cases.push_back({ "rounded, per-side outline", makeBox({ 64 * 2 + 21, 64 * 2 + 50, 64 * 40, 64 * 32 }, 64 * 8, 64 * 8, 64 * 8, 64 * 8, { 64, 64 * 3 + 20, 64 * 4, 32 }), 0.04 });
		cases.push_back({ "outline thicker than radius", makeBox({ 64 * 2 + 21, 64 * 2 + 50, 64 * 40, 64 * 32 }, 64 * 4, 64 * 4, 64 * 4, 64 * 4, { 64 * 6, 64 * 6, 64 * 6, 64 * 6 }), 0.04 });
		cases.push_back({ "outline fills box", makeBox({ 64 * 2 + 21, 64 * 2 + 50, 64 * 20, 64 * 12 }, 64 * 4, 64 * 4, 64 * 4, 64 * 4, { 64 * 7, 64 * 7, 64 * 7, 64 * 7 }), 0.04 });
		cases.push_back({ "large radius", makeBox({ 64 * 1 + 21, 64 * 1 + 50, 64 * 700, 64 * 500 }, 64 * 240, 64 * 240, 64 * 240, 64 * 240, { 64 * 3, 64 * 3, 64 * 3, 64 * 3 }), 0.04 });
		cases.push_back({ "negative position", makeBox({ -64 * 10 - 21, -64 * 5 - 13, 64 * 40, 64 * 30 }, 64 * 12, 64 * 12, 64 * 12, 64 * 12, { 64 * 2, 64 * 2, 64 * 2, 64 * 2 }), 0.04 });

		// Diagonal corners reaching past each other.

		cases.push_back({ "leaf", makeBox({ 64 * 3 + 13, 64 * 2 + 40, 64 * 24, 64 * 24 }, 0, 64 * 24, 0, 64 * 24), 0.04 });
		cases.push_back({ "other leaf, outline", makeBox({ 64 * 3 + 13, 64 * 2 + 40, 64 * 40 + 20, 64 * 40 + 20 }, 64 * 40, 0, 64 * 40, 0, { 64 * 3, 64 * 3, 64 * 3, 64 * 3 }), 0.04 });
		cases.push_back({ "large diagonal radii", makeBox({ 64 * 2 + 5, 64 * 2 + 33, 64 * 60, 64 * 50 }, 64 * 10, 64 * 45, 64 * 5, 64 * 40), 0.04 });

		// Inner corner that would reach past the inside of a thick outline.

		cases.push_back({ "thick outline on one side", makeBox({ 64 * 2, 64 * 2, 64 * 30, 64 * 100 }, 0, 0, 64 * 30, 0, { 64, 64, 64, 64 * 10 + 32 }), 0.04 });

		// Corners too large to be flattened in advance, checked around part of a corner.

		const SoftShapes::RoundedBox huge = makeBox({ 64 * 1 + 21, 64 * 1 + 50, 64 * 2300, 64 * 2200 }, 64 * 1080, 64 * 1080, 64 * 1080, 64 * 1080, { 64 * 3, 64 * 3, 64 * 3, 64 * 3 });

		cases.push_back({ "huge corners, diagonal", huge, 0.04, { 150, 150, 360, 360 } });
		cases.push_back({ "huge corners, top", huge, 0.04, { 900, 0, 400, 40 } });
		cases.push_back({ "huge mixed corners", makeBox({ 64 * 2 + 5, 64 * 2 + 33, 64 * 2400, 64 * 2300 }, 64 * 1100, 64 * 1500, 64 * 1030, 64 * 1200), 0.04, { 1800, 300, 300, 300 } });

		return cases;
	}
}

//____ Constructor ____________________________________________________________

SoftShapesTest::SoftShapesTest()
{
	ADD_TEST(roundedBoxCoverageTest);
	ADD_TEST(roundedBoxSeamTest);
	ADD_TEST(roundedBoxSymmetryTest);
	ADD_TEST(roundedBoxClipTest);
	ADD_TEST(clampCornerRadiiTest);
	ADD_TEST(ringCoverageTest);
	ADD_TEST(ringClipTest);
	ADD_TEST(ringWithoutBackTest);
	ADD_TEST(mirrorBufferTest);
}

SoftShapesTest::~SoftShapesTest()
{
}

//____ roundedBoxCoverageTest() _______________________________________________
// Coverage of fill and outline compared to supersampled exact geometry.

bool SoftShapesTest::roundedBoxCoverageTest(std::ostream& output)
{
	SoftShapes shapes;

	for (auto& c : boxCases())
	{
		RectI area = caseArea(c);
		CoverageGrid grid(area);
		grid.rasterize(shapes, c.box, area);

		if (grid.hasError())
		{
			output << c.name << ": " << grid.error() << std::endl;
			return false;
		}

		RefBox ref = makeReference(c.box);

		double worst = 0.0;
		int worstX = 0, worstY = 0;
		double worstGot = 0, worstWanted = 0;
		const char* worstWhat = "";

		for (int y = area.y; y < area.y + area.h; y++)
		{
			for (int x = area.x; x < area.x + area.w; x++)
			{
				double outer = ref.outer.coverage(x, y);
				double inner = ref.inner.coverage(x, y);

				double fill = grid.fill(x, y) / 4096.0;
				double outline = grid.outline(x, y) / 4096.0;

				double diffFill = std::abs(fill - inner);
				double diffOutline = std::abs(outline - (outer - inner));

				if (diffFill > worst)
				{
					worst = diffFill; worstX = x; worstY = y; worstGot = fill; worstWanted = inner; worstWhat = "fill";
				}

				if (diffOutline > worst)
				{
					worst = diffOutline; worstX = x; worstY = y; worstGot = outline; worstWanted = outer - inner; worstWhat = "outline";
				}
			}
		}

		if (worst > c.tolerance)
		{
			output << c.name << ": " << worstWhat << " of pixel " << worstX << "," << worstY << " is " << worstGot
				   << ", expected " << worstWanted << " (tolerance " << c.tolerance << ")." << std::endl;
			return false;
		}
	}

	return true;
}

//____ roundedBoxSeamTest() ___________________________________________________
// Where the outer shape fully covers a pixel and its neighbors, fill and outline
// must add up to exactly full coverage, so no background can show through.

bool SoftShapesTest::roundedBoxSeamTest(std::ostream& output)
{
	SoftShapes shapes;

	for (auto& c : boxCases())
	{
		RectI area = caseArea(c);
		CoverageGrid grid(area);
		grid.rasterize(shapes, c.box, area);

		if (grid.hasError())
		{
			output << c.name << ": " << grid.error() << std::endl;
			return false;
		}

		RefBox ref = makeReference(c.box);

		for (int y = area.y + 1; y < area.y + area.h - 1; y++)
		{
			for (int x = area.x + 1; x < area.x + area.w - 1; x++)
			{
				bool bDeepInside = ref.outer.containsSquare(x - 1, y - 1, x + 2, y + 2);

				int sum = grid.fill(x, y) + grid.outline(x, y);

				if (sum > 4096 || (bDeepInside && sum != 4096))
				{
					output << c.name << ": fill + outline of pixel " << x << "," << y << " is " << sum << ", expected 4096." << std::endl;
					return false;
				}
			}
		}
	}

	return true;
}

//____ roundedBoxSymmetryTest() _______________________________________________
// Symmetric boxes, centered on a pixel edge, must give symmetric coverage.

bool SoftShapesTest::roundedBoxSymmetryTest(std::ostream& output)
{
	SoftShapes shapes;

	std::vector<SoftShapes::RoundedBox> boxes;

	// Centered on x = 32 px, y = 24 px.

	boxes.push_back(makeBox({ 64 * 32 - 64 * 20 - 13, 64 * 24 - 64 * 15 - 29, (64 * 20 + 13) * 2, (64 * 15 + 29) * 2 }, 64 * 7 + 3, 64 * 7 + 3, 64 * 7 + 3, 64 * 7 + 3, { 64 + 9, 64 + 9, 64 + 9, 64 + 9 }));
	boxes.push_back(makeBox({ 64 * 32 - 64 * 20, 64 * 24 - 64 * 15, 64 * 40, 64 * 30 }, 64 * 15, 64 * 15, 64 * 15, 64 * 15, { 64 * 3, 64 * 3, 64 * 3, 64 * 3 }));
	boxes.push_back(makeBox({ 64 * 32 - 64 * 9 - 40, 64 * 24 - 64 * 9 - 40, (64 * 9 + 40) * 2, (64 * 9 + 40) * 2 }, 64 * 20, 64 * 20, 64 * 20, 64 * 20));

	for (auto& box : boxes)
	{
		RectI area(0, 0, 64, 48);
		CoverageGrid grid(area);
		grid.rasterize(shapes, box, area);

		if (grid.hasError())
		{
			output << grid.error() << std::endl;
			return false;
		}

		for (int y = 0; y < 48; y++)
		{
			for (int x = 0; x < 64; x++)
			{
				int mx = 63 - x;
				int my = 47 - y;

				for (auto [ox, oy] : { std::pair<int,int>(mx, y), std::pair<int,int>(x, my), std::pair<int,int>(mx, my) })
				{
					if (std::abs(grid.fill(x, y) - grid.fill(ox, oy)) > 4 || std::abs(grid.outline(x, y) - grid.outline(ox, oy)) > 4)
					{
						output << "Pixel " << x << "," << y << " (" << grid.fill(x, y) << "/" << grid.outline(x, y) << ") differs from mirrored pixel "
							   << ox << "," << oy << " (" << grid.fill(ox, oy) << "/" << grid.outline(ox, oy) << ")." << std::endl;
						return false;
					}
				}
			}
		}
	}

	return true;
}

//____ roundedBoxClipTest() ___________________________________________________
// Rasterizing in tiles must give the same result as all at once.

bool SoftShapesTest::roundedBoxClipTest(std::ostream& output)
{
	SoftShapes shapes;

	for (auto& c : boxCases())
	{
		RectI area = caseArea(c);

		CoverageGrid whole(area);
		whole.rasterize(shapes, c.box, area);

		CoverageGrid tiled(area);

		const int tileW = 7, tileH = 5;

		for (int ty = area.y; ty < area.y + area.h; ty += tileH)
			for (int tx = area.x; tx < area.x + area.w; tx += tileW)
			{
				RectI tile = RectI::overlap(RectI(tx, ty, tileW, tileH), area);
				tiled.rasterize(shapes, c.box, tile);
			}

		if (tiled.hasError())
		{
			output << c.name << ": " << tiled.error() << std::endl;
			return false;
		}

		for (int y = area.y; y < area.y + area.h; y++)
			for (int x = area.x; x < area.x + area.w; x++)
			{
				if (std::abs(whole.fill(x, y) - tiled.fill(x, y)) > 2 || std::abs(whole.outline(x, y) - tiled.outline(x, y)) > 2)
				{
					output << c.name << ": pixel " << x << "," << y << " is " << tiled.fill(x, y) << "/" << tiled.outline(x, y)
						   << " when tiled, " << whole.fill(x, y) << "/" << whole.outline(x, y) << " when not." << std::endl;
					return false;
				}
			}
	}

	return true;
}

//____ clampCornerRadiiTest() _________________________________________________

bool SoftShapesTest::clampCornerRadiiTest(std::ostream& output)
{
	{
		spx r[4] = { 10, 20, 30, 40 };
		GfxUtil::clampCornerRadii({ 1000, 1000 }, r);
		TEST_ASSERT(r[0] == 10 && r[1] == 20 && r[2] == 30 && r[3] == 40);
	}

	{
		spx r[4] = { -10, 20, 30, 40 };
		GfxUtil::clampCornerRadii({ 1000, 1000 }, r);
		TEST_ASSERT(r[0] == 0);
	}

	{
		// Left side needs 60 but has 40, all radii scaled by 40/60.

		spx r[4] = { 30, 15, 0, 30 };
		GfxUtil::clampCornerRadii({ 100, 40 }, r);
		TEST_ASSERT(r[0] == 20 && r[1] == 10 && r[2] == 0 && r[3] == 20);
	}

	{
		// Pill.

		spx r[4] = { 1000, 1000, 1000, 1000 };
		GfxUtil::clampCornerRadii({ 300, 100 }, r);
		TEST_ASSERT(r[0] == 50 && r[1] == 50 && r[2] == 50 && r[3] == 50);
	}

	return true;
}

//____ Ring tests ______________________________________________________________

namespace
{
	// Coverage of each segment of a ring.

	class SegmentGrid
	{
	public:
		SegmentGrid(const RectI& area, int nSegments) : m_coverage(area, nSegments) {}

		void rasterize(SoftShapes& shapes, const SoftShapes::Ring& ring, const RectI& clip)
		{
			m_coverage.collect([&](SoftShapes::RowSink* pSink) { shapes.rasterizeRing(ring, clip, pSink); });
		}

		int coverage(int segment, int x, int y) const { return m_coverage.coverage(segment, x, y); }

		bool			hasError() const { return m_coverage.hasError(); }
		std::string		error() const { return m_coverage.error(); }

	private:
		ShapeCoverage	m_coverage;
	};

	struct RingCase
	{
		const char*			name;
		RectSPX				rect;
		spx					thickness;
		spx					outline;
		bool				bSquare;
		double				start;				// Turns
		std::vector<double>	slices;				// Turns
		std::vector<double>	ends = {};			// End of each slice, set by ringCases().
	};

	std::vector<RingCase> ringCases()
	{
		std::vector<RingCase> cases;

		cases.push_back({ "pie", { 64 * 4 + 13, 64 * 3 + 37, 64 * 60 + 10, 64 * 60 + 10 }, 0, 0, false, 0.03, { 0.1, 0.2, 0.05, 0.15, 0.3, 0.12, 0.08 } });
		cases.push_back({ "donut with outline", { 64 * 4 + 13, 64 * 3 + 37, 64 * 60, 64 * 60 }, 64 * 12, 64 + 30, false, 0.0, { 0.25, 0.25, 0.3, 0.2 } });
		cases.push_back({ "gauge", { 64 * 2 + 50, 64 * 3 + 5, 64 * 64, 64 * 64 }, 64 * 8, 0, false, 0.7, { 0.2, 0.3, 0.1 } });
		cases.push_back({ "square pie", { 64 * 4 + 13, 64 * 3 + 37, 64 * 50, 64 * 40 }, 0, 0, true, 0.1, { 0.3, 0.3, 0.2, 0.2 } });
		cases.push_back({ "many slices", { 64 * 2, 64 * 2, 64 * 70, 64 * 70 }, 0, 0, false, 0.0, std::vector<double>(64, 1.0 / 64) });
		cases.push_back({ "single slice ring", { 64 * 4 + 13, 64 * 3 + 37, 64 * 50, 64 * 50 }, 64 * 10, 64 * 2, false, 0.4, { 1.0 } });
		cases.push_back({ "no slices", { 64 * 4 + 13, 64 * 3 + 37, 64 * 50, 64 * 50 }, 64 * 10, 64 * 2, false, 0.0, {} });
		cases.push_back({ "wide rect", { 64 * 2 + 9, 64 * 5 + 21, 64 * 70, 64 * 40 }, 64 * 6, 0, false, 0.85, { 0.4, 0.35 } });
		cases.push_back({ "tiny pie", { 64 * 3 + 21, 64 * 3 + 40, 64 * 6 + 20, 64 * 6 + 20 }, 0, 0, false, 0.2, { 0.2, 0.3, 0.1, 0.2, 0.2 } });
		cases.push_back({ "horizontal boundaries", { 64 * 4, 64 * 4, 64 * 50, 64 * 50 }, 0, 0, false, 0.25, { 0.5, 0.5 } });
		cases.push_back({ "no slices, aligned, odd width", { 64 * 4, 64 * 3, 64 * 51, 64 * 51 }, 64 * 10, 64 * 2, false, 0.0, {} });
		cases.push_back({ "square, half pixel", { 64 * 4 + 32, 64 * 3, 64 * 50, 64 * 40 }, 64 * 8, 64 + 20, true, 0.0, {} });
		cases.push_back({ "256 slices", { 64 * 3 + 17, 64 * 2 + 45, 64 * 150, 64 * 150 }, 0, 0, false, 0.0, std::vector<double>(256, 1.0 / 256) });
		cases.push_back({ "256 slices turned", { 64 * 3 + 17, 64 * 2 + 45, 64 * 150, 64 * 150 }, 64 * 40, 0, false, 0.37, std::vector<double>(256, 1.0 / 256) });

		for (auto& c : cases)
		{
			double sum = 0;
			for (double size : c.slices)
				c.ends.push_back(sum += size);
		}

		return cases;
	}

	SoftShapes::Ring makeRing(const RingCase& c, std::vector<uint32_t>& sizes)
	{
		sizes.clear();
		for (double size : c.slices)
			sizes.push_back(uint32_t(size * SoftShapes::c_fullTurn + 0.5));

		SoftShapes::Ring ring;
		ring.rect = c.rect;
		ring.thickness = c.thickness;
		ring.outline = c.outline;
		ring.bSquare = c.bSquare;
		ring.start = uint32_t(c.start * SoftShapes::c_fullTurn + 0.5) % SoftShapes::c_fullTurn;
		ring.nSlices = (int)sizes.size();
		ring.pSliceSizes = sizes.data();
		return ring;
	}

	// Segment at a point, from exact geometry.

	int refRingSegment(const RingCase& c, double x, double y)
	{
		double left = c.rect.x / 64.0, top = c.rect.y / 64.0;
		double right = (c.rect.x + c.rect.w) / 64.0, bottom = (c.rect.y + c.rect.h) / 64.0;

		if (x < left || x >= right || y < top || y >= bottom)
			return -1;

		double cx = (c.rect.x + c.rect.w / 2) / 64.0;
		double cy = (c.rect.y + c.rect.h / 2) / 64.0;
		double r0 = std::min(c.rect.w, c.rect.h) / 2 / 64.0;

		bool bHole = c.thickness > 0 && c.thickness / 64.0 < r0;
		double ri = bHole ? r0 - c.thickness / 64.0 : 0;
		double o = std::min(c.outline / 64.0, bHole ? (c.thickness / 2) / 64.0 : r0);

		double dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));

		if (c.bSquare)
		{
			if (x < left + o || x >= right - o || y < top + o || y >= bottom - o)
				return SoftShapes::c_ringOutlineSegment;
		}
		else
		{
			if (dist >= r0)
				return SoftShapes::c_ringBackSegment;
			if (dist >= r0 - o)
				return SoftShapes::c_ringOutlineSegment;
		}

		if (bHole)
		{
			if (dist < ri)
				return SoftShapes::c_ringHubSegment;
			if (dist < ri + o)
				return SoftShapes::c_ringOutlineSegment;
		}

		double angle = std::atan2(x - cx, -(y - cy)) / (2 * 3.14159265358979323846);
		angle = angle - std::floor(angle);

		double pos = angle - c.start;
		pos -= std::floor(pos);

		auto it = std::upper_bound(c.ends.begin(), c.ends.end(), pos);
		if (it != c.ends.end())
			return SoftShapes::c_ringFirstSliceSegment + int(it - c.ends.begin());

		return SoftShapes::c_ringBackSegment;
	}
}

//____ ringCoverageTest() _____________________________________________________

bool SoftShapesTest::ringCoverageTest(std::ostream& output)
{
	SoftShapes shapes;
	std::vector<uint32_t> sizes;

	for (auto& c : ringCases())
	{
		RectI area = pixelBounds(c.rect, 1);
		int nSegments = SoftShapes::c_ringFirstSliceSegment + (int)c.slices.size();

		SegmentGrid grid(area, nSegments);
		grid.rasterize(shapes, makeRing(c, sizes), area);

		if (grid.hasError())
		{
			output << c.name << ": " << grid.error() << std::endl;
			return false;
		}

		double worst = 0;
		int worstX = 0, worstY = 0, worstSegment = 0;

		const int samples = 16;
		std::vector<int> counts(nSegments);

		for (int y = area.y; y < area.y + area.h; y++)
		{
			for (int x = area.x; x < area.x + area.w; x++)
			{
				// Segments must add up to exactly the coverage of rect.

				int64_t ox = std::min<int64_t>(c.rect.x + c.rect.w, x * 64 + 64) - std::max<int64_t>(c.rect.x, x * 64);
				int64_t oy = std::min<int64_t>(c.rect.y + c.rect.h, y * 64 + 64) - std::max<int64_t>(c.rect.y, y * 64);
				int rectCoverage = int(std::max<int64_t>(ox, 0) * std::max<int64_t>(oy, 0));

				int sum = 0;
				for (int s = 0; s < nSegments; s++)
					sum += grid.coverage(s, x, y);

				if (sum != rectCoverage)
				{
					output << c.name << ": segments of pixel " << x << "," << y << " add up to " << sum << ", expected " << rectCoverage << "." << std::endl;
					return false;
				}

				// Compare with sampled exact geometry.

				std::fill(counts.begin(), counts.end(), 0);

				for (int sy = 0; sy < samples; sy++)
					for (int sx = 0; sx < samples; sx++)
					{
						int seg = refRingSegment(c, x + (sx + 0.5) / samples, y + (sy + 0.5) / samples);
						if (seg >= 0)
							counts[seg]++;
					}

				for (int s = 0; s < nSegments; s++)
				{
					double diff = std::abs(grid.coverage(s, x, y) / 4096.0 - counts[s] / double(samples * samples));

					if (diff > worst)
					{
						worst = diff;
						worstX = x;
						worstY = y;
						worstSegment = s;
					}
				}
			}
		}

		if (worst > 0.08)
		{
			output << c.name << ": segment " << worstSegment << " of pixel " << worstX << "," << worstY << " is " << grid.coverage(worstSegment, worstX, worstY) / 4096.0
				   << ", differs by " << worst << " from reference." << std::endl;
			return false;
		}
	}

	return true;
}

//____ ringClipTest() _________________________________________________________

bool SoftShapesTest::ringClipTest(std::ostream& output)
{
	SoftShapes shapes;
	std::vector<uint32_t> sizes;

	for (auto& c : ringCases())
	{
		RectI area = pixelBounds(c.rect, 1);
		int nSegments = SoftShapes::c_ringFirstSliceSegment + (int)c.slices.size();

		SegmentGrid whole(area, nSegments);
		whole.rasterize(shapes, makeRing(c, sizes), area);

		SegmentGrid tiled(area, nSegments);

		for (int ty = area.y; ty < area.y + area.h; ty += 5)
			for (int tx = area.x; tx < area.x + area.w; tx += 7)
				tiled.rasterize(shapes, makeRing(c, sizes), RectI::overlap(RectI(tx, ty, 7, 5), area));

		for (const SegmentGrid* pGrid : { &whole, &tiled })
			if (pGrid->hasError())
			{
				output << c.name << ": " << pGrid->error() << std::endl;
				return false;
			}

		for (int y = area.y; y < area.y + area.h; y++)
			for (int x = area.x; x < area.x + area.w; x++)
				for (int s = 0; s < nSegments; s++)
					if (std::abs(whole.coverage(s, x, y) - tiled.coverage(s, x, y)) > 2)
					{
						output << c.name << ": segment " << s << " of pixel " << x << "," << y << " is " << tiled.coverage(s, x, y)
							   << " when tiled, " << whole.coverage(s, x, y) << " when not." << std::endl;
						return false;
					}
	}

	return true;
}

//____ ringWithoutBackTest() __________________________________________________
// Without back, the other segments get the same coverage, except where the outer
// edge touches the edge of rect and isn't cut by it. The back segment only gets the
// part of the ring not reached by slices.

bool SoftShapesTest::ringWithoutBackTest(std::ostream& output)
{
	SoftShapes shapes;
	std::vector<uint32_t> sizes;

	for (auto& c : ringCases())
	{
		RectI area = pixelBounds(c.rect, 1);
		int nSegments = SoftShapes::c_ringFirstSliceSegment + (int)c.slices.size();

		SegmentGrid withBack(area, nSegments);
		withBack.rasterize(shapes, makeRing(c, sizes), area);

		SoftShapes::Ring ring = makeRing(c, sizes);
		ring.bBack = false;

		SegmentGrid withoutBack(area, nSegments);
		withoutBack.rasterize(shapes, ring, area);

		for (const SegmentGrid* pGrid : { &withBack, &withoutBack })
			if (pGrid->hasError())
			{
				output << c.name << ": " << pGrid->error() << std::endl;
				return false;
			}

		double sliceSum = 0;
		for (double size : c.slices)
			sliceSum += size;

		bool bGap = sliceSum < 0.9999;

		for (int y = area.y; y < area.y + area.h; y++)
			for (int x = area.x; x < area.x + area.w; x++)
				for (int s = 0; s < nSegments; s++)
				{
					int a = withBack.coverage(s, x, y);
					int b = withoutBack.coverage(s, x, y);

					bool bOk = (s == SoftShapes::c_ringBackSegment) ? (b <= a && (bGap || b == 0)) : std::abs(a - b) <= 16;

					if (!bOk)
					{
						output << c.name << ": segment " << s << " of pixel " << x << "," << y << " is " << b
							   << " without back, " << a << " with." << std::endl;
						return false;
					}
				}
	}

	return true;
}

//____ mirrorBufferTest() _____________________________________________________
// Rows kept for the mirrored lower half of shapes symmetric upside down only save
// time, pixels are the same whatever the size of the buffer, also when only some rows
// fit or it grows while rasterizing.

bool SoftShapesTest::mirrorBufferTest(std::ostream& output)
{
	struct BufferSize { int initial, max; };
	const int large = SoftShapes::c_defaultMaxMirrorBuffer;
	const BufferSize sizes[] = { { 0, 0 }, { 200, 200 }, { 0, 3000 }, { 2000, 2000 }, { large, large } };

	SoftShapes reference;

	for (auto& c : boxCases())
	{
		RectI area = caseArea(c);

		CoverageGrid wanted(area);
		wanted.rasterize(reference, c.box, area);

		for (const BufferSize& size : sizes)
		{
			SoftShapes shapes;
			shapes.setMirrorBuffer(size.initial, size.max);

			CoverageGrid grid(area);
			grid.rasterize(shapes, c.box, area);

			if (grid.hasError())
			{
				output << c.name << ", mirror buffer " << size.initial << "/" << size.max << ": " << grid.error() << std::endl;
				return false;
			}

			for (int y = area.y; y < area.y + area.h; y++)
				for (int x = area.x; x < area.x + area.w; x++)
					if (grid.fill(x, y) != wanted.fill(x, y) || grid.outline(x, y) != wanted.outline(x, y))
					{
						output << c.name << ", mirror buffer " << size.initial << "/" << size.max << ": pixel " << x << "," << y << " is "
							   << grid.fill(x, y) << "/" << grid.outline(x, y) << ", expected " << wanted.fill(x, y) << "/" << wanted.outline(x, y) << "." << std::endl;
						return false;
					}
		}
	}

	std::vector<uint32_t> sliceSizes;

	for (auto& c : ringCases())
	{
		RectI area = pixelBounds(c.rect, 1);
		int nSegments = SoftShapes::c_ringFirstSliceSegment + (int)c.slices.size();

		SegmentGrid wanted(area, nSegments);
		wanted.rasterize(reference, makeRing(c, sliceSizes), area);

		for (const BufferSize& size : sizes)
		{
			SoftShapes shapes;
			shapes.setMirrorBuffer(size.initial, size.max);

			SegmentGrid grid(area, nSegments);
			grid.rasterize(shapes, makeRing(c, sliceSizes), area);

			if (grid.hasError())
			{
				output << c.name << ", mirror buffer " << size.initial << "/" << size.max << ": " << grid.error() << std::endl;
				return false;
			}

			for (int y = area.y; y < area.y + area.h; y++)
				for (int x = area.x; x < area.x + area.w; x++)
					for (int s = 0; s < nSegments; s++)
						if (grid.coverage(s, x, y) != wanted.coverage(s, x, y))
						{
							output << c.name << ", mirror buffer " << size.initial << "/" << size.max << ": segment " << s << " of pixel " << x << "," << y
								   << " is " << grid.coverage(s, x, y) << ", expected " << wanted.coverage(s, x, y) << "." << std::endl;
							return false;
						}
		}
	}

	return true;
}
