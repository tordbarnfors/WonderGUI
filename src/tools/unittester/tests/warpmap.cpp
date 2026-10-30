#define _USE_MATH_DEFINES		// For M_PI in MSVC.

#include "warpmap.h"

#include <wondergui.h>
#include <wg_softwarpmap.h>
#include <wg_softwarpmapfactory.h>
#include <wg_circularwarp.h>
#include <wg_roundedrectwarp.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace wg;

// Tests of the Warpmap base class and the CircularWarp generator, which don't depend on any
// backend. Rendering through Warpmaps is tested by WarpRenderTest.

namespace
{
	bool near(double a, double b, double tolerance) { return std::abs(a - b) <= tolerance; }

	// Exact position (spx of the source) of CircularWarp at point x,y (pixels), u wrapped into the period.

	CoordF exactCircular(const CircularWarp::Blueprint& bp, double x, double y)
	{
		double dx = x - bp.center.x * bp.size.w;
		double dy = y - bp.center.y * bp.size.h;

		double angle = std::atan2(dx, -dy) * 180.0 / M_PI;
		double period = std::round(bp.sourceSize.w * 360.0 / std::abs(bp.sweep));
		double u = (angle - bp.startAngle) / bp.sweep * bp.sourceSize.w;
		u -= period * std::floor(u / period);

		double r = std::sqrt(dx * dx + dy * dy);
		double v = (r - bp.topRadius / 64.0) / (bp.bottomRadius / 64.0 - bp.topRadius / 64.0) * bp.sourceSize.h;

		return { float(u), float(v) };
	}

	const SizeSPX c_sourceSize = { 400 * 64, 32 * 64 };

	// Points along the outline of a rounded rectangle centered on 0,0, half hx wide and hy high with
	// corners of radius r, every step pixels clockwise from the middle of the top side, with their
	// position along the outline.

	struct OutlinePoint { double x, y, pos; };

	std::vector<OutlinePoint> sampleOutline(double hx, double hy, double r, double step)
	{
		std::vector<OutlinePoint> points;
		double ax = hx - r, ay = hy - r;
		double pos = 0;

		auto line = [&](double x0, double y0, double x1, double y1)
		{
			double len = std::hypot(x1 - x0, y1 - y0);
			int n = std::max(1, int(len / step));
			for (int i = 0; i < n; i++)
				points.push_back({ x0 + (x1 - x0) * i / n, y0 + (y1 - y0) * i / n, pos + len * i / n });
			pos += len;
		};

		auto corner = [&](double cx, double cy, double angle)				// Angle clockwise from up.
		{
			double len = r * M_PI / 2;
			int n = std::max(1, int(len / step));
			for (int i = 0; i < n; i++)
			{
				double a = angle + (M_PI / 2) * i / n;
				points.push_back({ cx + r * std::sin(a), cy - r * std::cos(a), pos + len * i / n });
			}
			pos += len;
		};

		line(0, -hy, ax, -hy);
		corner(ax, -ay, 0);
		line(hx, -ay, hx, ay);
		corner(ax, ay, M_PI / 2);
		line(ax, hy, -ax, hy);
		corner(-ax, ay, M_PI);
		line(-hx, ay, -hx, -ay);
		corner(-ax, -ay, M_PI * 3 / 2);
		line(-ax, -hy, 0, -hy);
		return points;
	}

	// Position along the outline of the nearest point of it, found among the sampled points, and
	// the distance to it, positive outside.

	void nearestOnOutline(const std::vector<OutlinePoint>& outline, double hx, double hy, double r, double x, double y, double& pos, double& dist)
	{
		double best = 1e30;
		for (auto& p : outline)
		{
			double d2 = (x - p.x) * (x - p.x) + (y - p.y) * (y - p.y);
			if (d2 < best)
			{
				best = d2;
				pos = p.pos;
			}
		}

		double qx = std::abs(x) - (hx - r), qy = std::abs(y) - (hy - r);
		bool bInside = std::abs(x) <= hx && std::abs(y) <= hy && !(qx > 0 && qy > 0 && std::hypot(qx, qy) > r);
		dist = bInside ? -std::sqrt(best) : std::sqrt(best);
	}

	// Difference in u, taking the period into account.

	double uDiff(double a, double b, double period)
	{
		double d = a - b;
		if (period > 0)
			d -= period * std::round(d / period);
		return std::abs(d);
	}
}

//____ constructor ____________________________________________________________

WarpmapTest::WarpmapTest()
{
	ADD_TEST(defaultNodesTest);
	ADD_TEST(importExportTest);
	ADD_TEST(periodTest);
	ADD_TEST(circularWarpTest);
	ADD_TEST(circularWarpGenerateTest);
	ADD_TEST(roundedRectWarpTest);
}

WarpmapTest::~WarpmapTest()
{
}

//____ defaultNodesTest() _____________________________________________________
// A new Warpmap maps its rectangle straight onto the source, pixel for pixel.

bool WarpmapTest::defaultNodesTest(std::ostream& output)
{
	auto pMap = SoftWarpmap::create({ .cellSize = 16, .size = { 100, 50 } });
	TEST_ASSERT(pMap != nullptr);

	// 100 / 16 rounded up is 7 cells, 50 / 16 is 4, plus one node.

	TEST_ASSERT(pMap->nodes() == SizeI(8, 5));
	TEST_ASSERT(pMap->nodeX(7) == 100);
	TEST_ASSERT(pMap->nodeY(4) == 50);

	for (double y : { 0.0, 3.5, 17.0, 49.9, 50.0 })
		for (double x : { 0.0, 0.5, 15.9, 99.0, 100.0 })
		{
			CoordF pos = pMap->positionAt(x, y);
			TEST_ASSERT(near(pos.x, x * 64, 1e-3) && near(pos.y, y * 64, 1e-3));
		}

	// Invalid blueprints.

	TEST_ASSERT(SoftWarpmap::create({ .cellSize = 0, .size = { 10, 10 } }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 0, 10 } }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 10, 10 }, .uPeriod = -1 }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 10, 10 }, .vPeriod = Warpmap::maxNodeValue + 1 }) == nullptr);

	return true;
}

//____ importExportTest() _____________________________________________________

bool WarpmapTest::importExportTest(std::ostream& output)
{
	auto pMap = SoftWarpmap::create({ .cellSize = 10, .size = { 30, 20 } });		// 4 x 3 nodes.

	// Import a 2x2 region from a buffer with a pitch of 5.

	CoordSPX buffer[10];
	for (int i = 0; i < 10; i++)
		buffer[i] = { i, -i };

	TEST_ASSERT(pMap->importNodes({ 1,1,2,2 }, buffer, 5));

	CoordSPX all[12];
	TEST_ASSERT(pMap->exportNodes({ 0,0,4,3 }, all));

	TEST_ASSERT(all[1 * 4 + 1] == CoordSPX(0, 0));
	TEST_ASSERT(all[1 * 4 + 2] == CoordSPX(1, -1));
	TEST_ASSERT(all[2 * 4 + 1] == CoordSPX(5, -5));
	TEST_ASSERT(all[2 * 4 + 2] == CoordSPX(6, -6));
	TEST_ASSERT(all[0] == CoordSPX(0, 0));								// Untouched.
	TEST_ASSERT(all[3] == CoordSPX(30 * 64, 0));

	// Interpolation in the middle of cell 1,1, between nodes 0,0 / 1,-1 / 5,-5 / 6,-6.

	CoordF mid = pMap->positionAt(15, 15);
	TEST_ASSERT(near(mid.x, 3.0, 1e-6) && near(mid.y, -3.0, 1e-6));

	// Regions outside the nodes are refused.

	TEST_ASSERT(!pMap->importNodes({ 3,0,2,1 }, buffer));
	TEST_ASSERT(!pMap->importNodes({ -1,0,1,1 }, buffer));
	TEST_ASSERT(!pMap->exportNodes({ 0,2,1,2 }, all));
	TEST_ASSERT(!pMap->importNodes({ 0,0,1,1 }, nullptr));

	return true;
}

//____ periodTest() ___________________________________________________________
// Values on periodic axes are normalized on import and cells take the short way across the seam.

bool WarpmapTest::periodTest(std::ostream& output)
{
	auto pMap = SoftWarpmap::create({ .cellSize = 10, .size = { 10, 10 }, .uPeriod = 640 });		// One cell, period of 10 pixels.

	CoordSPX nodes[4] = { { 608, 0 }, { 672, 0 }, { -32, 64 }, { 1888, 64 } };
	TEST_ASSERT(pMap->importNodes({ 0,0,2,2 }, nodes));

	CoordSPX stored[4];
	pMap->exportNodes({ 0,0,2,2 }, stored);

	TEST_ASSERT(stored[0].x == 608);
	TEST_ASSERT(stored[1].x == 32);
	TEST_ASSERT(stored[2].x == 608);
	TEST_ASSERT(stored[3].x == 608);
	TEST_ASSERT(stored[3].y == 64);										// v is not periodic.

	// Corners are unwrapped to the top-left corner.

	CoordSPX corners[4];
	pMap->cellCorners(0, 0, corners);

	TEST_ASSERT(corners[0].x == 608);
	TEST_ASSERT(corners[1].x == 672);
	TEST_ASSERT(corners[2].x == 608);
	TEST_ASSERT(corners[3].x == 608);

	// Halfway across the top edge we are on the seam, not halfway around.

	TEST_ASSERT(near(pMap->positionAt(5, 0).x, 640, 1e-3));

	// A corner exactly half a period away is taken below the top-left corner.

	CoordSPX half[4] = { { 0, 0 }, { 320, 0 }, { 0, 64 }, { 0, 64 } };
	pMap->importNodes({ 0,0,2,2 }, half);
	pMap->cellCorners(0, 0, corners);
	TEST_ASSERT(corners[1].x == -320);

	// Changing the period renormalizes.

	pMap->importNodes({ 0,0,2,2 }, nodes);
	TEST_ASSERT(pMap->setPeriods(320, 0));
	pMap->exportNodes({ 0,0,2,2 }, stored);
	TEST_ASSERT(stored[0].x == 288);

	TEST_ASSERT(!pMap->setPeriods(-1, 0));
	TEST_ASSERT(!pMap->setPeriods(0, Warpmap::maxNodeValue + 1));
	return true;
}

//____ circularWarpTest() _____________________________________________________
// CircularWarp against exact polar math.

bool WarpmapTest::circularWarpTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();

	struct Case { CircularWarp::Blueprint bp; float tolerance; };

	Case cases[] = {
		{ WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 200, 200 }, _.topRadius = 98 * 64, _.bottomRadius = 60 * 64), 0.25f },						// Full ring, top outside.
		{ WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 300, 200 }, _.center = { 0.4f, 0.6f }, _.topRadius = 40 * 64, _.bottomRadius = 90 * 64,
			   _.startAngle = -135.f, _.sweep = 270.f), 0.25f },																	// Arc.
		{ WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 200, 200 }, _.topRadius = 30 * 64, _.bottomRadius = 99 * 64, _.sweep = -90.f, _.precision = 0.1f), 0.1f },	// Counter clockwise, repeats 4 times.
		{ WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 160, 160 }, _.topRadius = 0, _.bottomRadius = 80 * 64), 0.25f }								// Disc.
	};

	for (auto& c : cases)
	{
		auto pWarp = CircularWarp::create(c.bp, pFactory);
		TEST_ASSERT(pWarp != nullptr);

		auto pMap = pWarp->generate();
		TEST_ASSERT(pMap != nullptr);
		TEST_ASSERT(pMap->pixelSize() == c.bp.size);

		double period = std::round(c_sourceSize.w * 360.0 / std::abs(c.bp.sweep));
		TEST_ASSERT(pMap->uPeriod() == spx(period));

		// Cell size from precision: sqrt(8 * precision * r) at the smaller radius, or half the radius of a disc.

		double inner = std::min(c.bp.topRadius, c.bp.bottomRadius) / 64.0;
		double detail = inner > 0 ? inner : std::max(c.bp.topRadius, c.bp.bottomRadius) / 64.0 / 2;
		int expectedCell = std::clamp(int(std::sqrt(8.0 * c.bp.precision * detail)), 4, 64);
		TEST_ASSERT(pMap->cellSize() == expectedCell);
		TEST_ASSERT(pWarp->cellSize() == expectedCell);

		// Exact at nodes.

		SizeI nodes = pMap->nodes();
		for (int row = 0; row < nodes.h; row++)
			for (int col = 0; col < nodes.w; col++)
			{
				CoordF exact = exactCircular(c.bp, pMap->nodeX(col), pMap->nodeY(row));
				CoordSPX node = pMap->nodeData()[row * nodes.w + col];
				TEST_ASSERT(uDiff(node.x, exact.x, period) <= 0.51 && near(node.y, exact.y, 0.51));		// Rounded to spx.
			}

		// Within precision between nodes, outside of the radius precision is kept from. Precision is
		// in pixels, so we convert the errors in u and v (spx of the source) to pixels.

		double rTop = c.bp.topRadius / 64.0, rBottom = c.bp.bottomRadius / 64.0;
		double cx = c.bp.center.x * c.bp.size.w, cy = c.bp.center.y * c.bp.size.h;
		double maxError = 0;

		for (double y = 0.5; y < c.bp.size.h; y += 1.0)
			for (double x = 0.5; x < c.bp.size.w; x += 1.0)
			{
				double r = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
				if (r < detail || r > std::max(rTop, rBottom))
					continue;

				CoordF exact = exactCircular(c.bp, x, y);
				CoordF pos = pMap->positionAt(x, y);

				double tangential = uDiff(pos.x, exact.x, period) / c_sourceSize.w * std::abs(c.bp.sweep) / 180.0 * M_PI * r;
				double radial = std::abs(pos.y - exact.y) / c_sourceSize.h * std::abs(rBottom - rTop);
				maxError = std::max(maxError, std::max(tangential, radial));
			}

		if (maxError > c.tolerance * 1.5)
			output << "Max position error " << maxError << " px, precision " << c.tolerance << std::endl;
		TEST_ASSERT(maxError <= c.tolerance * 1.5);			// Estimate cell^2/(8r) is for the radial error, allow some slack.
	}

	// Explicit cell size overrides precision.

	auto pWarp = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.topRadius = 10 * 64, _.bottomRadius = 50 * 64, _.cellSize = 7), pFactory);
	TEST_ASSERT(pWarp->generate()->cellSize() == 7);

	// Invalid blueprints.

	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.sweep = 0.f), pFactory) == nullptr);
	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 0, 100 }), pFactory) == nullptr);
	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.size = { 100, 100 }), pFactory) == nullptr);		// No source size.

	// Equal radii have no band to map the source onto, the nodes are clamped far outside it.

	auto pFlat = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 40, 40 }, _.cellSize = 10), pFactory);
	auto pFlatMap = pFlat->generate();
	for (int i = 0; i < pFlatMap->nodes().w * pFlatMap->nodes().h; i++)
		TEST_ASSERT(pFlatMap->nodeData()[i].y == -Warpmap::maxNodeValue);

	return true;
}

//____ circularWarpGenerateTest() ______________________________________________

bool WarpmapTest::circularWarpGenerateTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();

	auto pWarp = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.topRadius = 20 * 64, _.bottomRadius = 50 * 64, _.cellSize = 10), pFactory);

	TEST_ASSERT(pWarp->isInstanceOf(WarpGenerator::TYPEINFO));

	// A Warpmap that nothing but the CircularWarp holds is updated in place.

	Warpmap* pFirst = pWarp->generate().rawPtr();
	TEST_ASSERT(pWarp->generate().rawPtr() == pFirst);						// Nothing changed.

	TEST_ASSERT(pWarp->setRadii(25 * 64, 45 * 64));
	TEST_ASSERT(pWarp->setAngles(90.f, 180.f));
	auto pMap1 = pWarp->generate();
	TEST_ASSERT(pMap1.rawPtr() == pFirst);
	TEST_ASSERT(pMap1->uPeriod() == c_sourceSize.w * 2);

	// Point straight right of center at radius 25 is u = 0 (90 degrees), v = 0.

	CoordF pos = pMap1->positionAt(75, 50);
	TEST_ASSERT(near(pos.x, 0.0, 1.0) && near(pos.y, 0.0, 1.0));

	// A Warpmap held elsewhere, as by a draw that isn't rendered yet, is left as it is and a new
	// one is made. Source size scales the nodes and the period.

	SizeI nodes = pMap1->nodes();
	std::vector<CoordSPX> before(nodes.w * nodes.h), after(nodes.w * nodes.h);
	pMap1->exportNodes({ 0, 0, nodes.w, nodes.h }, before.data());

	TEST_ASSERT(pWarp->generate() == pMap1);								// Nothing changed, held or not.

	TEST_ASSERT(pWarp->setSourceSize({ c_sourceSize.w / 2, c_sourceSize.h }));
	auto pMap2 = pWarp->generate();
	TEST_ASSERT(pMap2 != pMap1);
	TEST_ASSERT(pMap2->uPeriod() == c_sourceSize.w);
	TEST_ASSERT(pMap2->nodeData()[0].x != pMap1->nodeData()[0].x);

	pMap1->exportNodes({ 0, 0, nodes.w, nodes.h }, after.data());
	TEST_ASSERT(pMap1->uPeriod() == c_sourceSize.w * 2);
	TEST_ASSERT(before == after);
	pMap1 = nullptr;

	TEST_ASSERT(pWarp->setSourceSize(c_sourceSize));

	// Size and cell size give a new Warpmap.

	TEST_ASSERT(pWarp->setSize({ 120, 100 }));
	auto pMap3 = pWarp->generate();
	TEST_ASSERT(pMap3 != pMap2 && pMap3->pixelSize() == SizeI(120, 100));

	TEST_ASSERT(pWarp->setCellSize(0));
	TEST_ASSERT(pWarp->setPrecision(1.f));
	auto pMap4 = pWarp->generate();
	TEST_ASSERT(pMap4 != pMap3 && pMap4->cellSize() == int(std::sqrt(8.0 * 1.0 * 25)));		// At the smaller radius.

	// Invalid values are refused.

	TEST_ASSERT(!pWarp->setRadii(10 * 64, 10 * 64));
	TEST_ASSERT(!pWarp->setAngles(0.f, 0.f));
	TEST_ASSERT(!pWarp->setSize({ 0, 10 }));
	TEST_ASSERT(!pWarp->setCellSize(-1));
	TEST_ASSERT(!pWarp->setPrecision(0.f));
	TEST_ASSERT(!pWarp->setSourceSize({ 0, 10 }));
	return true;
}

//____ roundedRectWarpTest() __________________________________________________
// RoundedRectWarp against the nearest points of a sampled outline, and as a circle against CircularWarp.

bool WarpmapTest::roundedRectWarpTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();

	// Outline of 100 x 60 pixels with corners of 12, in a band from 6 pixels inside it to 8 outside.

	const double cx = 70, cy = 50, hx = 50, hy = 30, r = 12, top = -6, bottom = 8;

	auto pWarp = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 140, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 100 * 64, 60 * 64 },
											  _.cornerRadius = 12 * 64, _.topDistance = -6 * 64, _.bottomDistance = 8 * 64, _.cellSize = 5), pFactory);
	TEST_ASSERT(pWarp != nullptr);
	TEST_ASSERT(pWarp->isInstanceOf(WarpGenerator::TYPEINFO));

	auto pMap = pWarp->generate();
	TEST_ASSERT(pMap->uPeriod() == c_sourceSize.w);

	// Nodes are where the nearest point of the outline is. Nodes deeper inside than the corners,
	// where they are mitered and the nearest point can be either side, are skipped.

	const double perimeter = 4 * (hx - r) + 4 * (hy - r) + 2 * M_PI * r;
	auto outline = sampleOutline(hx, hy, r, 0.01);

	SizeI nodes = pMap->nodes();
	for (int row = 0; row < nodes.h; row++)
		for (int col = 0; col < nodes.w; col++)
		{
			double x = pMap->nodeX(col) - cx, y = pMap->nodeY(row) - cy;
			if (std::abs(x) < hx - r && std::abs(y) < hy - r)
				continue;

			double pos, dist;
			nearestOnOutline(outline, hx, hy, r, x, y, pos, dist);

			CoordSPX node = pMap->nodeData()[row * nodes.w + col];
			double nodePos = node.x / double(c_sourceSize.w) * perimeter;
			double nodeDist = node.y / double(c_sourceSize.h) * (bottom - top) + top;

			if (uDiff(nodePos, pos, perimeter) > 0.05 || !near(nodeDist, dist, 0.05))
				output << "Node at " << x << ", " << y << " is " << nodePos << ", " << nodeDist << ", nearest point " << pos << ", " << dist << std::endl;
			TEST_ASSERT(uDiff(nodePos, pos, perimeter) <= 0.05 && near(nodeDist, dist, 0.05));
		}

	// Start and sweep. The middle of the right side is a quarter of the way round, where the source
	// starts with a start of 0.25. A sweep of 0.5 makes the period twice the width of the source.

	TEST_ASSERT(pWarp->setSpan(0.25f, 0.5f));
	pMap = pWarp->generate();
	TEST_ASSERT(pMap->uPeriod() == c_sourceSize.w * 2);

	CoordF right = pMap->positionAt(cx + hx, cy);
	TEST_ASSERT(near(right.x, 0, 1) && near(right.y, -top / (bottom - top) * c_sourceSize.h, 1));

	CoordF below = pMap->positionAt(cx, cy + hy + 4);							// Middle of the bottom side, half way round.
	TEST_ASSERT(near(below.x, c_sourceSize.w * 0.5, 1) && near(below.y, (4 - top) / (bottom - top) * c_sourceSize.h, 1));

	// A square with corners of half its side is a circle, the same as a CircularWarp.

	auto pCircle = CircularWarp::create(WGBP(CircularWarp, _.size = { 100, 100 }, _.sourceSize = c_sourceSize, _.topRadius = 45 * 64, _.bottomRadius = 20 * 64,
											 _.cellSize = 6, _.startAngle = 30.f, _.sweep = -270.f), pFactory)->generate();
	auto pRounded = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 100, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 60 * 64, 60 * 64 },
												 _.cornerRadius = 30 * 64, _.topDistance = 15 * 64, _.bottomDistance = -10 * 64, _.cellSize = 6,
												 _.start = 30.f / 360, _.sweep = -0.75f), pFactory)->generate();

	TEST_ASSERT(pRounded->uPeriod() == pCircle->uPeriod());
	TEST_ASSERT(pRounded->nodes() == pCircle->nodes());

	nodes = pCircle->nodes();
	for (int i = 0; i < nodes.w * nodes.h; i++)
	{
		CoordSPX a = pCircle->nodeData()[i], b = pRounded->nodeData()[i];
		TEST_ASSERT(uDiff(a.x, b.x, pCircle->uPeriod()) <= 1 && std::abs(a.y - b.y) <= 1);
	}

	// Cell size from precision at the tightest corner of the band: the corner radius less the
	// distance inwards. Sharp corners take half the radius of the outer edge, as a disc.

	auto pAuto = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 140, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 100 * 64, 60 * 64 },
											  _.cornerRadius = 12 * 64, _.topDistance = -6 * 64, _.bottomDistance = 8 * 64, _.precision = 2.f), pFactory);
	TEST_ASSERT(pAuto->cellSize() == int(std::sqrt(8.0 * 2 * 6)));
	TEST_ASSERT(pAuto->setOutline({ 20 * 64, 20 * 64, 100 * 64, 60 * 64 }, 0));
	TEST_ASSERT(pAuto->setDistances(0, 8 * 64));
	TEST_ASSERT(pAuto->cellSize() == int(std::sqrt(8.0 * 2 * 4)));

	// Equal distances have no band to map the source onto, the nodes are clamped far outside it.

	auto pFlat = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 40, 40 }, _.sourceSize = c_sourceSize, _.rect = { 640, 640, 1280, 1280 }, _.cellSize = 10), pFactory);
	auto pFlatMap = pFlat->generate();
	for (int i = 0; i < pFlatMap->nodes().w * pFlatMap->nodes().h; i++)
		TEST_ASSERT(pFlatMap->nodeData()[i].y == -Warpmap::maxNodeValue);

	// Invalid values are refused.

	TEST_ASSERT(RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 40, 40 }, _.sourceSize = c_sourceSize, _.sweep = 0.f), pFactory) == nullptr);
	TEST_ASSERT(RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 40, 40 }, _.sourceSize = c_sourceSize, _.rect = { 0, 0, -64, 64 }), pFactory) == nullptr);
	TEST_ASSERT(RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 40, 40 }, _.sourceSize = c_sourceSize, _.cornerRadius = -1), pFactory) == nullptr);
	TEST_ASSERT(RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 40, 40 }), pFactory) == nullptr);			// No source size.

	TEST_ASSERT(!pWarp->setOutline({ 0, 0, 64, -64 }, 0));
	TEST_ASSERT(!pWarp->setOutline({ 0, 0, 64, 64 }, -1));
	TEST_ASSERT(!pWarp->setDistances(64, 64));
	TEST_ASSERT(!pWarp->setSpan(0.f, 0.f));
	return true;
}
