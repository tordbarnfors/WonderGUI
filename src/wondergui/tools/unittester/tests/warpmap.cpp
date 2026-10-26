#define _USE_MATH_DEFINES		// For M_PI in MSVC.

#include "warpmap.h"

#include <wondergui.h>
#include <wax_softwarpmap.h>
#include <wax_softwarpmapfactory.h>
#include <wax_circularwarp.h>

#include <cmath>
#include <vector>

using namespace wg;
using namespace wax;

// Tests of the Warpmap base class and the CircularWarp generator, which don't depend on any
// backend. Rendering through Warpmaps is tested by WarpRenderTest.

namespace
{
	bool near(double a, double b, double tolerance) { return std::abs(a - b) <= tolerance; }

	// Exact position of CircularWarp at point x,y (pixels), u wrapped into the period.

	CoordF exactCircular(const CircularWarp::Blueprint& bp, double x, double y)
	{
		double dx = x - bp.center.x * bp.size.w;
		double dy = y - bp.center.y * bp.size.h;

		double angle = std::atan2(dx, -dy) * 180.0 / M_PI;
		double period = 360.0 / std::abs(bp.sweep);
		double u = (angle - bp.startAngle) / bp.sweep;
		u -= period * std::floor(u / period);

		double r = std::sqrt(dx * dx + dy * dy);
		double v = (r - bp.topRadius / 64.0) / (bp.bottomRadius / 64.0 - bp.topRadius / 64.0);

		return { float(u), float(v) };
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
	ADD_TEST(circularWarpRefreshTest);
}

WarpmapTest::~WarpmapTest()
{
}

//____ defaultNodesTest() _____________________________________________________
// A new Warpmap maps its rectangle straight onto the source.

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
			TEST_ASSERT(near(pos.x, x / 100, 1e-6) && near(pos.y, y / 50, 1e-6));
		}

	// Invalid blueprints.

	TEST_ASSERT(SoftWarpmap::create({ .cellSize = 0, .size = { 10, 10 } }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 0, 10 } }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 10, 10 }, .uPeriod = -1.f }) == nullptr);

	return true;
}

//____ importExportTest() _____________________________________________________

bool WarpmapTest::importExportTest(std::ostream& output)
{
	auto pMap = SoftWarpmap::create({ .cellSize = 10, .size = { 30, 20 } });		// 4 x 3 nodes.

	// Import a 2x2 region from a buffer with a pitch of 5.

	CoordF buffer[10];
	for (int i = 0; i < 10; i++)
		buffer[i] = { float(i), float(-i) };

	TEST_ASSERT(pMap->importNodes({ 1,1,2,2 }, buffer, 5));

	CoordF all[12];
	TEST_ASSERT(pMap->exportNodes({ 0,0,4,3 }, all));

	TEST_ASSERT(all[1 * 4 + 1] == CoordF(0, 0));
	TEST_ASSERT(all[1 * 4 + 2] == CoordF(1, -1));
	TEST_ASSERT(all[2 * 4 + 1] == CoordF(5, -5));
	TEST_ASSERT(all[2 * 4 + 2] == CoordF(6, -6));
	TEST_ASSERT(all[0] == CoordF(0, 0));								// Untouched.
	TEST_ASSERT(all[3] == CoordF(1, 0));

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
	auto pMap = SoftWarpmap::create({ .cellSize = 10, .size = { 10, 10 }, .uPeriod = 1.f });		// One cell.

	CoordF nodes[4] = { { 0.95f, 0.f }, { 1.05f, 0.f }, { -0.05f, 1.f }, { 2.95f, 1.f } };
	TEST_ASSERT(pMap->importNodes({ 0,0,2,2 }, nodes));

	CoordF stored[4];
	pMap->exportNodes({ 0,0,2,2 }, stored);

	TEST_ASSERT(near(stored[0].x, 0.95, 1e-6));
	TEST_ASSERT(near(stored[1].x, 0.05, 1e-6));
	TEST_ASSERT(near(stored[2].x, 0.95, 1e-6));
	TEST_ASSERT(near(stored[3].x, 0.95, 1e-6));
	TEST_ASSERT(stored[3].y == 1.f);										// v is not periodic.

	// Corners are unwrapped to the top-left corner.

	CoordF corners[4];
	pMap->cellCorners(0, 0, corners);

	TEST_ASSERT(near(corners[0].x, 0.95, 1e-6));
	TEST_ASSERT(near(corners[1].x, 1.05, 1e-6));
	TEST_ASSERT(near(corners[2].x, 0.95, 1e-6));
	TEST_ASSERT(near(corners[3].x, 0.95, 1e-6));

	// Halfway across the top edge we are on the seam, not halfway around.

	TEST_ASSERT(near(pMap->positionAt(5, 0).x, 1.0, 1e-6));

	// Changing the period renormalizes.

	TEST_ASSERT(pMap->setPeriods(0.5f, 0.f));
	pMap->exportNodes({ 0,0,2,2 }, stored);
	TEST_ASSERT(near(stored[0].x, 0.45, 1e-6));

	TEST_ASSERT(!pMap->setPeriods(-1.f, 0.f));
	return true;
}

//____ circularWarpTest() _____________________________________________________
// CircularWarp against exact polar math.

bool WarpmapTest::circularWarpTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();

	struct Case { CircularWarp::Blueprint bp; float tolerance; };

	Case cases[] = {
		{ WGBP(CircularWarp, _.size = { 200, 200 }, _.topRadius = 98 * 64, _.bottomRadius = 60 * 64), 0.25f },						// Full ring, top outside.
		{ WGBP(CircularWarp, _.size = { 300, 200 }, _.center = { 0.4f, 0.6f }, _.topRadius = 40 * 64, _.bottomRadius = 90 * 64,
			   _.startAngle = -135.f, _.sweep = 270.f), 0.25f },																	// Arc.
		{ WGBP(CircularWarp, _.size = { 200, 200 }, _.topRadius = 30 * 64, _.bottomRadius = 99 * 64, _.sweep = -90.f, _.precision = 0.1f), 0.1f },	// Counter clockwise, repeats 4 times.
		{ WGBP(CircularWarp, _.size = { 160, 160 }, _.topRadius = 0, _.bottomRadius = 80 * 64, _.detailRadius = 40 * 64), 0.25f }	// Disc.
	};

	for (auto& c : cases)
	{
		auto pWarp = CircularWarp::create(c.bp, pFactory);
		TEST_ASSERT(pWarp != nullptr);

		auto pMap = pWarp->refresh();
		TEST_ASSERT(pMap != nullptr);
		TEST_ASSERT(pMap->pixelSize() == c.bp.size);

		double period = 360.0 / std::abs(c.bp.sweep);
		TEST_ASSERT(near(pMap->uPeriod(), period, 1e-5));

		// Cell size from precision: sqrt(8 * precision * detail radius), detail radius being the smallest radius by default.

		double detail = c.bp.detailRadius > 0 ? c.bp.detailRadius / 64.0 : std::min(c.bp.topRadius, c.bp.bottomRadius) / 64.0;
		int expectedCell = std::clamp(int(std::sqrt(8.0 * c.bp.precision * detail)), 4, 64);
		TEST_ASSERT(pMap->cellSize() == expectedCell);
		TEST_ASSERT(pWarp->cellSize() == expectedCell);

		// Exact at nodes.

		SizeI nodes = pMap->nodes();
		for (int row = 0; row < nodes.h; row++)
			for (int col = 0; col < nodes.w; col++)
			{
				CoordF exact = exactCircular(c.bp, pMap->nodeX(col), pMap->nodeY(row));
				CoordF node = pMap->nodeData()[row * nodes.w + col];
				TEST_ASSERT(uDiff(node.x, exact.x, period) < 1e-5 && near(node.y, exact.y, 1e-5));
			}

		// Within precision between nodes, where at least the detail radius from the center. Precision is
		// in pixels, so we convert the errors in u and v to pixels.

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

				double tangential = uDiff(pos.x, exact.x, period) * std::abs(c.bp.sweep) / 180.0 * M_PI * r;
				double radial = std::abs(pos.y - exact.y) * std::abs(rBottom - rTop);
				maxError = std::max(maxError, std::max(tangential, radial));
			}

		if (maxError > c.tolerance * 1.5)
			output << "Max position error " << maxError << " px, precision " << c.tolerance << std::endl;
		TEST_ASSERT(maxError <= c.tolerance * 1.5);			// Estimate cell^2/(8r) is for the radial error, allow some slack.
	}

	// Explicit cell size overrides precision.

	auto pWarp = CircularWarp::create(WGBP(CircularWarp, _.size = { 100, 100 }, _.topRadius = 10 * 64, _.bottomRadius = 50 * 64, _.cellSize = 7), pFactory);
	TEST_ASSERT(pWarp->refresh()->cellSize() == 7);

	// Invalid blueprints.

	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.size = { 100, 100 }, _.sweep = 0.f), pFactory) == nullptr);
	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.size = { 0, 100 }), pFactory) == nullptr);

	return true;
}

//____ circularWarpRefreshTest() ______________________________________________

bool WarpmapTest::circularWarpRefreshTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();

	auto pWarp = CircularWarp::create(WGBP(CircularWarp, _.size = { 100, 100 }, _.topRadius = 20 * 64, _.bottomRadius = 50 * 64, _.cellSize = 10), pFactory);

	auto pMap1 = pWarp->refresh();
	TEST_ASSERT(pWarp->refresh() == pMap1);								// Nothing changed.

	// Radii and angles update the nodes of the same Warpmap.

	TEST_ASSERT(pWarp->setRadii(25 * 64, 45 * 64));
	TEST_ASSERT(pWarp->setAngles(90.f, 180.f));
	auto pMap2 = pWarp->refresh();
	TEST_ASSERT(pMap2 == pMap1);
	TEST_ASSERT(near(pMap2->uPeriod(), 2.0, 1e-6));

	// Point straight right of center at radius 25 is u = 0 (90 degrees), v = 0.

	CoordF pos = pMap2->positionAt(75, 50);
	TEST_ASSERT(near(pos.x, 0.0, 1e-4) && near(pos.y, 0.0, 1e-4));

	// Size and cell size give a new Warpmap.

	TEST_ASSERT(pWarp->setSize({ 120, 100 }));
	auto pMap3 = pWarp->refresh();
	TEST_ASSERT(pMap3 != pMap2 && pMap3->pixelSize() == SizeI(120, 100));

	TEST_ASSERT(pWarp->setCellSize(0));
	TEST_ASSERT(pWarp->setPrecision(0.5f, 40 * 64));
	auto pMap4 = pWarp->refresh();
	TEST_ASSERT(pMap4 != pMap3 && pMap4->cellSize() == int(std::sqrt(8.0 * 0.5 * 40)));

	// Invalid values are refused.

	TEST_ASSERT(!pWarp->setRadii(10 * 64, 10 * 64));
	TEST_ASSERT(!pWarp->setAngles(0.f, 0.f));
	TEST_ASSERT(!pWarp->setSize({ 0, 10 }));
	return true;
}
