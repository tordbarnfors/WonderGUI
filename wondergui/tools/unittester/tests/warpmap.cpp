#define _USE_MATH_DEFINES		// For M_PI in MSVC.

#include "warpmap.h"

#include <wondergui.h>
#include <wax_softwarpmap.h>
#include <wax_softwarpmapfactory.h>
#include <wax_circularwarp.h>
#include <wax_roundedrectwarp.h>
#include <wax_cylinderwarp.h>
#include <wax_nodecompression.h>
#include <cstring>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace wg;
using namespace wax;

// Tests of the Warpmap base class and the CircularWarp generator, which don't depend on any
// backend. Rendering through Warpmaps is tested by WarpRenderTest.

namespace
{
	bool near(double a, double b, double tolerance) { return std::abs(a - b) <= tolerance; }

	// Warpmap that counts the nodes it sends on, as a backend uploads them, and a factory for it.

	class CountingWarpmap : public Warpmap
	{
	public:
		CountingWarpmap(const Blueprint& bp, const CoordSPX* pNodes, int pitch) : Warpmap(bp, pNodes, pitch)
		{
			_nodesUpdated({ 0, 0, m_nodes.w, m_nodes.h });
		}

		int		updates = 0;
		int		nodesSent = 0;

	protected:
		void _nodesUpdated(const RectI& region) override
		{
			updates++;
			nodesSent += region.w * region.h;
		}
	};

	class CountingWarpmapFactory : public WarpmapFactory
	{
	public:
		Warpmap_p createWarpmap(const Warpmap::Blueprint& bp) override { return _created(new CountingWarpmap(bp, nullptr, 0)); }
		Warpmap_p createWarpmap(const Warpmap::Blueprint& bp, const CoordSPX* pNodes, int pitch) override { return _created(new CountingWarpmap(bp, pNodes, pitch)); }

		int		created = 0;

	private:
		Warpmap_p _created(Warpmap* p) { created++; return p; }
	};

	// Exact position (spx of the source) of CircularWarp at point x,y (pixels), u counted from
	// startAngle in the direction of sweep and wrapped into a turn. The period of the Warpmap is
	// rounded to spx, so u can only be exact within its first period.

	CoordF exactCircular(const CircularWarp::Blueprint& bp, double x, double y)
	{
		double dx = x - bp.center.x * bp.size.w;
		double dy = y - bp.center.y * bp.size.h;

		double angle = std::atan2(dx, -dy) * 180.0 / M_PI;
		double period = bp.sourceSize.w * 360.0 / std::abs(bp.sweep);
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
	ADD_TEST(createWithNodesTest);
	ADD_TEST(periodTest);
	ADD_TEST(circularWarpTest);
	ADD_TEST(circularWarpGenerateTest);
	ADD_TEST(generateUpdatesTest);
	ADD_TEST(roundedRectWarpTest);
	ADD_TEST(nodeCompressionTest);
	ADD_TEST(offsetTest);
	ADD_TEST(nonSquareCellsTest);
	ADD_TEST(cylinderWarpTest);
}

WarpmapTest::~WarpmapTest()
{
}

//____ defaultNodesTest() _____________________________________________________
// A new Warpmap maps its rectangle straight onto the source, pixel for pixel.

bool WarpmapTest::defaultNodesTest(std::ostream& output)
{
	auto pMap = SoftWarpmap::create({ .cellSize = { 16, 16 }, .size = { 100, 50 } });
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

	TEST_ASSERT(SoftWarpmap::create({ .cellSize = { 0, 0 }, .size = { 10, 10 } }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 0, 10 } }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 10, 10 }, .uPeriod = -1 }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 10, 10 }, .vPeriod = Warpmap::maxNodeValue + 1 }) == nullptr);

	return true;
}

//____ importExportTest() _____________________________________________________

bool WarpmapTest::importExportTest(std::ostream& output)
{
	auto pMap = SoftWarpmap::create({ .cellSize = { 10, 10 }, .size = { 30, 20 } });		// 4 x 3 nodes.

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

//____ createWithNodesTest() __________________________________________________
// A Warpmap created with nodes starts out with them, normalized, and sends them on once.

bool WarpmapTest::createWithNodesTest(std::ostream& output)
{
	Warpmap::Blueprint bp = { .cellSize = { 10, 10 }, .size = { 30, 20 }, .uPeriod = 640 };		// 4 x 3 nodes.

	// Nodes in a buffer with a pitch of 5, some outside the period.

	CoordSPX buffer[15];
	for (int i = 0; i < 15; i++)
		buffer[i] = { i * 100 - 700, -i };

	auto pFactory = StrongPtr<CountingWarpmapFactory>(new CountingWarpmapFactory());
	auto pMap = pFactory->createWarpmap(bp, buffer, 5);
	auto pCounting = static_cast<CountingWarpmap*>(pMap.rawPtr());

	TEST_ASSERT(pCounting->updates == 1);
	TEST_ASSERT(pCounting->nodesSent == 12);

	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 4; col++)
		{
			CoordSPX expected = buffer[row * 5 + col];
			expected.x = ((expected.x % 640) + 640) % 640;
			TEST_ASSERT(pMap->nodeData()[row * 4 + col] == expected);
		}

	// Same as a backend creating with nodes.

	auto pSoft = SoftWarpmap::create(bp, buffer, 5);
	TEST_ASSERT(pSoft != nullptr);
	TEST_ASSERT(std::equal(pSoft->nodeData(), pSoft->nodeData() + 12, pMap->nodeData()));

	// Without nodes it maps straight onto the source, also sent on once.

	auto pDefault = pFactory->createWarpmap(bp);
	TEST_ASSERT(static_cast<CountingWarpmap*>(pDefault.rawPtr())->updates == 1);
	TEST_ASSERT(pFactory->created == 2);

	return true;
}

//____ periodTest() ___________________________________________________________
// Values on periodic axes are normalized on import and cells take the short way across the seam.

bool WarpmapTest::periodTest(std::ostream& output)
{
	auto pMap = SoftWarpmap::create({ .cellSize = { 10, 10 }, .size = { 10, 10 }, .uPeriod = 640 });		// One cell, period of 10 pixels.

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

	// New periods come with all nodes, normalized into them.

	TEST_ASSERT(pMap->importAllNodes(nodes, 320, 0));
	TEST_ASSERT(pMap->uPeriod() == 320 && pMap->vPeriod() == 0);
	pMap->exportNodes({ 0,0,2,2 }, stored);
	TEST_ASSERT(stored[0].x == 288 && stored[1].x == 32 && stored[2].x == 288 && stored[3].x == 288);

	// Again, from the same nodes and not from the stored ones.

	TEST_ASSERT(pMap->importAllNodes(nodes, 640, 0));
	pMap->exportNodes({ 0,0,2,2 }, stored);
	TEST_ASSERT(stored[0].x == 608 && stored[1].x == 32 && stored[2].x == 608 && stored[3].x == 608);

	// Invalid periods are refused and change nothing.

	TEST_ASSERT(!pMap->importAllNodes(nodes, -1, 0));
	TEST_ASSERT(!pMap->importAllNodes(nodes, 0, Warpmap::maxNodeValue + 1));
	TEST_ASSERT(!pMap->importAllNodes(nullptr, 320, 0));
	TEST_ASSERT(pMap->uPeriod() == 640);
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
		TEST_ASSERT(pMap->cellSize() == SizeI(expectedCell, expectedCell));
		TEST_ASSERT(pWarp->cellSize() == SizeI(expectedCell, expectedCell));

		// Exact at nodes.

		SizeI nodes = pMap->nodes();
		for (int row = 0; row < nodes.h; row++)
			for (int col = 0; col < nodes.w; col++)
			{
				if (pMap->nodeX(col) == c.bp.center.x * c.bp.size.w && pMap->nodeY(row) == c.bp.center.y * c.bp.size.h)
					continue;																			// No angle at the center.

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

	auto pWarp = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.topRadius = 10 * 64, _.bottomRadius = 50 * 64, _.cellSize = { 7, 7 }), pFactory);
	TEST_ASSERT(pWarp->generate()->cellSize() == SizeI(7, 7));

	// Invalid blueprints.

	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.sweep = 0.f), pFactory) == nullptr);
	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 0, 100 }), pFactory) == nullptr);
	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.size = { 100, 100 }), pFactory) == nullptr);		// No source size.

	// Equal radii have no band to map the source onto, the nodes are clamped far outside it.

	auto pFlat = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 40, 40 }, _.cellSize = { 10, 10 }), pFactory);
	auto pFlatMap = pFlat->generate();
	for (int i = 0; i < pFlatMap->nodes().w * pFlatMap->nodes().h; i++)
		TEST_ASSERT(pFlatMap->nodeData()[i].y == -Warpmap::maxNodeValue);

	return true;
}

//____ circularWarpGenerateTest() ______________________________________________

bool WarpmapTest::circularWarpGenerateTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();

	auto pWarp = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.topRadius = 20 * 64, _.bottomRadius = 50 * 64, _.cellSize = { 10, 10 }), pFactory);

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

	TEST_ASSERT(pWarp->setCellSize({ 0, 0 }));
	TEST_ASSERT(pWarp->setPrecision(1.f));
	auto pMap4 = pWarp->generate();
	TEST_ASSERT(pMap4 != pMap3 && pMap4->cellSize() == SizeI(int(std::sqrt(8.0 * 1.0 * 25)), int(std::sqrt(8.0 * 1.0 * 25))));		// At the smaller radius.

	// Invalid values are refused.

	TEST_ASSERT(!pWarp->setRadii(10 * 64, 10 * 64));
	TEST_ASSERT(!pWarp->setAngles(0.f, 0.f));
	TEST_ASSERT(!pWarp->setSize({ 0, 10 }));
	TEST_ASSERT(!pWarp->setCellSize({ -1, 0 }));
	TEST_ASSERT(!pWarp->setPrecision(0.f));
	TEST_ASSERT(!pWarp->setSourceSize({ 0, 10 }));
	return true;
}

//____ generateUpdatesTest() __________________________________________________
// generate() sends the nodes on once, whether it creates a Warpmap, updates one in place or changes
// its periods, and nodes updated in place are the same as those of a new Warpmap.

bool WarpmapTest::generateUpdatesTest(std::ostream& output)
{
	auto pCounting = StrongPtr<CountingWarpmapFactory>(new CountingWarpmapFactory());
	auto pSoft = SoftWarpmapFactory::create();

	auto bp = WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 80 }, _.topRadius = 20 * 64, _.bottomRadius = 45 * 64, _.cellSize = { 10, 10 });
	auto pWarp = CircularWarp::create(bp, pCounting);

	auto counts = [](Warpmap* p) { return static_cast<CountingWarpmap*>(p); };

	// Created with its nodes.

	Warpmap* pMap = pWarp->generate();
	int nodes = pMap->nodes().w * pMap->nodes().h;
	TEST_ASSERT(pCounting->created == 1);
	TEST_ASSERT(counts(pMap)->updates == 1 && counts(pMap)->nodesSent == nodes);

	// Updated in place, and with new periods.

	auto sameAsNew = [&](Warpmap* pMap, const CircularWarp::Blueprint& bp)
	{
		auto pFresh = CircularWarp::create(bp, pSoft)->generate();
		return pFresh->uPeriod() == pMap->uPeriod() && std::equal(pMap->nodeData(), pMap->nodeData() + nodes, pFresh->nodeData());
	};

	TEST_ASSERT(pWarp->setRadii(25 * 64, 40 * 64));
	TEST_ASSERT(pWarp->generate() == pMap);
	TEST_ASSERT(counts(pMap)->updates == 2 && counts(pMap)->nodesSent == 2 * nodes);
	bp.topRadius = 25 * 64;
	bp.bottomRadius = 40 * 64;
	TEST_ASSERT(sameAsNew(pMap, bp));

	spx oldPeriod = pMap->uPeriod();
	TEST_ASSERT(pWarp->setAngles(30.f, 120.f));
	TEST_ASSERT(pWarp->generate() == pMap);
	TEST_ASSERT(pMap->uPeriod() != oldPeriod);
	TEST_ASSERT(counts(pMap)->updates == 3 && counts(pMap)->nodesSent == 3 * nodes);
	bp.startAngle = 30.f;
	bp.sweep = 120.f;
	TEST_ASSERT(sameAsNew(pMap, bp));

	// Nothing changed, nothing sent.

	TEST_ASSERT(pWarp->generate() == pMap);
	TEST_ASSERT(counts(pMap)->updates == 3);

	// A held Warpmap is left alone, a new one is created with the nodes.

	Warpmap_p pHeld = pMap;
	TEST_ASSERT(pWarp->setRadii(10 * 64, 40 * 64));
	Warpmap* pNew = pWarp->generate();
	TEST_ASSERT(pNew != pMap && pCounting->created == 2);
	TEST_ASSERT(counts(pNew)->updates == 1 && counts(pHeld)->updates == 3);

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
											  _.cornerRadius = 12 * 64, _.topDistance = -6 * 64, _.bottomDistance = 8 * 64, _.cellSize = { 5, 5 }), pFactory);
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
											 _.cellSize = { 6, 6 }, _.startAngle = 30.f, _.sweep = -270.f), pFactory)->generate();
	auto pRounded = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 100, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 60 * 64, 60 * 64 },
												 _.cornerRadius = 30 * 64, _.topDistance = 15 * 64, _.bottomDistance = -10 * 64, _.cellSize = { 6, 6 },
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
	TEST_ASSERT(pAuto->cellSize() == SizeI(int(std::sqrt(8.0 * 2 * 6)), int(std::sqrt(8.0 * 2 * 6))));
	TEST_ASSERT(pAuto->setOutline({ 20 * 64, 20 * 64, 100 * 64, 60 * 64 }, 0));
	TEST_ASSERT(pAuto->setDistances(0, 8 * 64));
	TEST_ASSERT(pAuto->cellSize() == SizeI(int(std::sqrt(8.0 * 2 * 4)), int(std::sqrt(8.0 * 2 * 4))));

	// Equal distances have no band to map the source onto, the nodes are clamped far outside it.

	auto pFlat = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 40, 40 }, _.sourceSize = c_sourceSize, _.rect = { 640, 640, 1280, 1280 }, _.cellSize = { 10, 10 }), pFactory);
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

//____ nodeCompressionTest() __________________________________________________
// NodeCompressor restores nodes exactly, also when decompressed in place from the end of a buffer
// of maxCompressedSize() as GfxStream does, compresses smooth nodes well and stores what it can't
// compress as it is.

bool WarpmapTest::nodeCompressionTest(std::ostream& output)
{
	auto pCompressor = NodeCompressor::create();
	auto pDecompressor = NodeDecompressor::create();

	TEST_ASSERT(wax::Base::getDecompressor(NodeCompressor::ID_TOKEN) != nullptr);
	TEST_ASSERT(!pCompressor->setRowLength(0));

	// Compresses and decompresses in place, returns compressed size or -1 if not restored.

	auto roundTrip = [&](const std::vector<int32_t>& values, int rowLength) -> int
	{
		pCompressor->setRowLength(rowLength);

		int raw = int(values.size() * 4);
		int bufferSize = pCompressor->maxCompressedSize(raw);
		std::vector<uint8_t> compressed(bufferSize);
		int size = pCompressor->compress(compressed.data(), values.data(), values.data() + values.size());
		if (size > bufferSize)
			return -1;

		const int guard = 16;
		std::vector<uint8_t> buffer(bufferSize + guard, 0xCD);
		std::memcpy(buffer.data() + bufferSize - size, compressed.data(), size);

		int restored = pDecompressor->decompress(buffer.data(), buffer.data() + bufferSize - size, buffer.data() + bufferSize);
		if (restored != raw || std::memcmp(buffer.data(), values.data(), raw) != 0)
			return -1;

		for (int i = 0; i < guard; i++)
			if (buffer[bufferSize + i] != 0xCD)
				return -1;

		return size;
	};

	// Nodes of a ring, less than a third of their raw size (about 2.3 of 8 bytes per node).

	auto pMap = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 200, 160 }, _.topRadius = 78 * 64, _.bottomRadius = 30 * 64,
		_.startAngle = 20.f, _.sweep = 300.f, _.cellSize = { 6, 6 }), SoftWarpmapFactory::create())->generate();

	SizeI nodes = pMap->nodes();
	std::vector<int32_t> ring((const int32_t*) pMap->nodeData(), (const int32_t*) (pMap->nodeData() + nodes.w * nodes.h));

	int size = roundTrip(ring, nodes.w);
	TEST_ASSERT(size > 0);
	TEST_ASSERT(size * 3 < int(ring.size() * 4));
	TEST_ASSERT(roundTrip(ring, nodes.w * nodes.h) > 0);						// As one row.

	// Random and extreme values, which are stored as they are, and constant ones.

	uint32_t seed = 77;
	auto random = [&seed]() { seed = seed * 1664525 + 1013904223; return seed; };

	for (int i = 0; i < 200; i++)
	{
		int w = 1 + random() % 23, h = 1 + random() % 17;
		std::vector<int32_t> values(w * h * 2);

		for (auto& v : values)
		{
			switch (i % 4)
			{
				case 0: v = int32_t(random()); break;
				case 1: v = (random() & 1) ? INT32_MIN : INT32_MAX; break;
				case 2: v = int32_t(random() % 256) - 128; break;
				default: v = 12345; break;
			}
		}

		size = roundTrip(values, w);
		TEST_ASSERT(size > 0 && size <= 1 + int(values.size() * 4));
	}

	// Not whole rows or pairs, stored as they are.

	TEST_ASSERT(roundTrip(std::vector<int32_t>(12, 5), 4) == 1 + 48);
	TEST_ASSERT(roundTrip(std::vector<int32_t>{ 1, 2, 3 }, 1) == 1 + 12);

	return true;
}

//____ offsetTest() ___________________________________________________________
// rotationOffset() and shiftOffset() give the offset that moves u as moving the start would, by the
// settings of the generated Warpmap.

bool WarpmapTest::offsetTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();
	const spx width = c_sourceSize.w;											// 25600 spx.

	// Offsets by sweep and direction, whole turns dropped.

	auto pRing = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.topRadius = 45 * 64, _.bottomRadius = 20 * 64), pFactory);
	TEST_ASSERT(pRing->rotationOffset(90.f) == CoordSPX(-width / 4, 0));
	TEST_ASSERT(pRing->rotationOffset(-90.f) == CoordSPX(width / 4, 0));
	TEST_ASSERT(pRing->rotationOffset(90.f + 360.f * 1000) == pRing->rotationOffset(90.f));
	TEST_ASSERT(pRing->rotationOffset(std::nanf("")) == CoordSPX(0, 0));

	// Settings count once generated.

	TEST_ASSERT(pRing->setAngles(0.f, -90.f));
	TEST_ASSERT(pRing->rotationOffset(45.f) == CoordSPX(-width / 8, 0));
	pRing->generate();
	TEST_ASSERT(pRing->rotationOffset(45.f) == CoordSPX(width / 2, 0));			// Counter clockwise source, turned clockwise.

	TEST_ASSERT(pRing->setSourceSize({ width * 2, c_sourceSize.h }));
	TEST_ASSERT(pRing->rotationOffset(45.f) == CoordSPX(width / 2, 0));
	pRing->generate();
	TEST_ASSERT(pRing->rotationOffset(45.f) == CoordSPX(width, 0));

	// Turning by offset gives the nodes of a larger start, give or take rounding.

	auto sameButOffset = [](Warpmap* pMoved, Warpmap* pBase, spx offset, CoordF skip)
	{
		SizeI nodes = pBase->nodes();
		for (int row = 0; row < nodes.h; row++)
			for (int col = 0; col < nodes.w; col++)
			{
				if (pBase->nodeX(col) == skip.x && pBase->nodeY(row) == skip.y)
					continue;

				CoordSPX a = pBase->nodeData()[row * nodes.w + col];
				CoordSPX b = pMoved->nodeData()[row * nodes.w + col];
				if (uDiff(b.x, a.x + offset, pBase->uPeriod()) > 1 || std::abs(b.y - a.y) > 1)
					return false;
			}
		return true;
	};

	auto ringBP = WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.topRadius = 45 * 64, _.bottomRadius = 20 * 64,
					   _.startAngle = 10.f, _.sweep = 300.f, _.cellSize = { 5, 5 });
	auto pBase = CircularWarp::create(ringBP, pFactory);
	spx offset = pBase->rotationOffset(45.f).x;
	TEST_ASSERT(offset == -width * 45 / 300);
	ringBP.startAngle += 45.f;
	TEST_ASSERT(sameButOffset(CircularWarp::create(ringBP, pFactory)->generate(), pBase->generate(), offset, { 50, 50 }));

	// The same along a rounded rectangle, in parts of the outline.

	auto frameBP = WGBP(RoundedRectWarp, _.size = { 140, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 100 * 64, 60 * 64 },
						_.cornerRadius = 15 * 64, _.topDistance = -10 * 64, _.bottomDistance = 10 * 64, _.start = 0.1f, _.sweep = 0.5f, _.cellSize = { 5, 5 });
	auto pFrame = RoundedRectWarp::create(frameBP, pFactory);
	TEST_ASSERT(pFrame->shiftOffset(0.25f) == CoordSPX(-width / 2, 0));
	TEST_ASSERT(pFrame->shiftOffset(3.25f) == CoordSPX(-width / 2, 0));

	offset = pFrame->shiftOffset(0.125f).x;
	TEST_ASSERT(offset == -width / 4);
	frameBP.start += 0.125f;
	TEST_ASSERT(sameButOffset(RoundedRectWarp::create(frameBP, pFactory)->generate(), pFrame->generate(), offset, { -1, -1 }));

	frameBP.sweep = -0.5f;
	TEST_ASSERT(RoundedRectWarp::create(frameBP, pFactory)->shiftOffset(0.25f) == CoordSPX(width / 2, 0));
	return true;
}

//____ nonSquareCellsTest() ___________________________________________________
// Cells can have another width than height, for Warpmaps and generators, and generators can
// calculate one and have the other set.

bool WarpmapTest::nonSquareCellsTest(std::ostream& output)
{
	auto pMap = SoftWarpmap::create({ .cellSize = { 20, 5 }, .size = { 50, 12 } });
	TEST_ASSERT(pMap != nullptr);
	TEST_ASSERT(pMap->cellSize() == SizeI(20, 5));
	TEST_ASSERT(pMap->nodes() == SizeI(4, 4));
	TEST_ASSERT(pMap->nodeX(1) == 20 && pMap->nodeX(3) == 50);
	TEST_ASSERT(pMap->nodeY(1) == 5 && pMap->nodeY(3) == 12);

	for (double y : { 0.0, 4.9, 5.0, 11.5, 12.0 })
		for (double x : { 0.0, 19.5, 20.0, 41.0, 50.0 })
		{
			CoordF pos = pMap->positionAt(x, y);
			TEST_ASSERT(near(pos.x, x * 64, 1e-3) && near(pos.y, y * 64, 1e-3));
		}

	TEST_ASSERT(SoftWarpmap::create({ .cellSize = { 0, 5 }, .size = { 10, 10 } }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .cellSize = { 5, 0 }, .size = { 10, 10 } }) == nullptr);

	// Generators: one set, the other calculated, and nodes at the same places whatever the cells.

	auto pFactory = SoftWarpmapFactory::create();
	auto bp = WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 96, 96 }, _.topRadius = 46 * 64, _.bottomRadius = 20 * 64, _.cellSize = { 6, 6 });
	auto pSquare = CircularWarp::create(bp, pFactory)->generate();

	bp.cellSize = { 12, 0 };
	auto pWarp = CircularWarp::create(bp, pFactory);
	int calculated = int(std::sqrt(8.0 * 0.25 * 20));
	TEST_ASSERT(pWarp->cellSize() == SizeI(12, calculated));

	TEST_ASSERT(pWarp->setCellSize({ 12, 6 }));
	auto pWide = pWarp->generate();
	TEST_ASSERT(pWide->cellSize() == SizeI(12, 6));
	TEST_ASSERT(pWide->nodes() == SizeI(9, 17));

	for (int row = 0; row < pWide->nodes().h; row++)
		for (int col = 0; col < pWide->nodes().w; col++)
			TEST_ASSERT(pWide->nodeData()[row * pWide->nodes().w + col] == pSquare->nodeData()[row * pSquare->nodes().w + col * 2]);

	TEST_ASSERT(!pWarp->setCellSize({ 4, -1 }));

	// A calculated size is at most the size of the Warpmap.

	auto pSmall = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 30, 10 }, _.topRadius = 5000 * 64, _.bottomRadius = 4000 * 64), pFactory);
	TEST_ASSERT(pSmall->cellSize() == SizeI(30, 10));
	return true;
}

//____ cylinderWarpTest() _____________________________________________________
// CylinderWarp against exact geometry: along the axis the source is stretched, across it the angle
// at distance y from the middle is asin(y / R), R = half the size across / sin(span / 2).

namespace
{
	struct CylinderGeometry
	{
		bool	bAxisX;
		double	lengthAlong, lengthAcross, sourceAlong, sourceAcross, radius, period;
	};

	CylinderGeometry cylinderGeometry(const CylinderWarp::Blueprint& bp)
	{
		CylinderGeometry g;
		g.bAxisX = bp.axis == Axis::X;
		g.lengthAlong = g.bAxisX ? bp.size.w : bp.size.h;
		g.lengthAcross = g.bAxisX ? bp.size.h : bp.size.w;
		g.sourceAlong = g.bAxisX ? bp.sourceSize.w : bp.sourceSize.h;
		g.sourceAcross = g.bAxisX ? bp.sourceSize.h : bp.sourceSize.w;
		g.radius = g.lengthAcross / 2 / std::sin(bp.span / 2 * M_PI / 180);
		g.period = g.sourceAcross * 360 / std::abs(bp.sweep);
		return g;
	}

	// Angle in degrees at a position across, and the exact position in the source (spx), the part
	// across wrapped into the true period.

	double cylinderAngle(const CylinderGeometry& g, double across)
	{
		return std::asin((across - g.lengthAcross / 2) / g.radius) * 180 / M_PI;
	}

	CoordF exactCylinder(const CylinderWarp::Blueprint& bp, double x, double y)
	{
		CylinderGeometry g = cylinderGeometry(bp);
		double along = (g.bAxisX ? x : y) / g.lengthAlong * g.sourceAlong;
		double across = (cylinderAngle(g, g.bAxisX ? y : x) - bp.startAngle) / bp.sweep * g.sourceAcross;
		across -= g.period * std::floor(across / g.period);
		return g.bAxisX ? CoordF(float(along), float(across)) : CoordF(float(across), float(along));
	}
}

bool WarpmapTest::cylinderWarpTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();
	const SizeSPX source = { 48 * 64, 240 * 64 };

	CylinderWarp::Blueprint cases[] = {
		WGBP(CylinderWarp, _.size = { 80, 200 }, _.sourceSize = source),															// A reel.
		WGBP(CylinderWarp, _.size = { 200, 90 }, _.sourceSize = { 240 * 64, 32 * 64 }, _.axis = Axis::Y, _.span = 150.f, _.startAngle = 30.f, _.sweep = -200.f),
		WGBP(CylinderWarp, _.size = { 60, 120 }, _.sourceSize = source, _.span = 60.f, _.startAngle = -45.f, _.sweep = 90.f, _.precision = 0.1f)
	};

	for (auto& bp : cases)
	{
		auto pWarp = CylinderWarp::create(bp, pFactory);
		TEST_ASSERT(pWarp != nullptr);

		auto pMap = pWarp->generate();
		CylinderGeometry g = cylinderGeometry(bp);
		bool bX = g.bAxisX;

		// Periodic across the axis only.

		spx period = spx(std::round(g.period));
		TEST_ASSERT(pMap->uPeriod() == (bX ? 0 : period) && pMap->vPeriod() == (bX ? period : 0));

		// One cell along the axis, across it from precision at the rims.

		double rim = bp.span / 2 * M_PI / 180;
		int across = std::clamp(int(std::sqrt(8.0 * bp.precision * g.radius * std::cos(rim) * std::cos(rim) / std::sin(rim))), 4, 64);
		TEST_ASSERT(pMap->cellSize() == (bX ? SizeI(bp.size.w, across) : SizeI(across, bp.size.h)));
		TEST_ASSERT(pMap->nodes() == (bX ? SizeI(2, pMap->nodes().h) : SizeI(pMap->nodes().w, 2)));

		// Exact at nodes.

		SizeI nodes = pMap->nodes();
		for (int row = 0; row < nodes.h; row++)
			for (int col = 0; col < nodes.w; col++)
			{
				CoordF exact = exactCylinder(bp, pMap->nodeX(col), pMap->nodeY(row));
				CoordSPX node = pMap->nodeData()[row * nodes.w + col];
				TEST_ASSERT(uDiff(node.x, exact.x, pMap->uPeriod()) <= 0.51 && uDiff(node.y, exact.y, pMap->vPeriod()) <= 0.51);
			}

		// Within precision between nodes, as positions on screen: an error in angle of da (radians)
		// is R * cos(angle) * da pixels across.

		double maxError = 0.0;
		for (double a = 0.5; a < g.lengthAcross; a += 1.0)
			for (double l : { 0.5, g.lengthAlong / 2 })
			{
				double x = bX ? l : a, y = bX ? a : l;
				CoordF pos = pMap->positionAt(x, y);
				CoordF exact = exactCylinder(bp, x, y);
				double diff = bX ? uDiff(pos.y, exact.y, pMap->vPeriod()) : uDiff(pos.x, exact.x, pMap->uPeriod());
				double angle = cylinderAngle(g, a) * M_PI / 180;
				double error = diff / g.sourceAcross * std::abs(bp.sweep) * M_PI / 180 * g.radius * std::cos(angle);
				maxError = std::max(maxError, error);

				double alongDiff = std::abs((bX ? pos.x : pos.y) - (bX ? exact.x : exact.y));
				TEST_ASSERT(alongDiff <= 0.51);
			}

		if (maxError > bp.precision * 1.5)
			output << "Max position error " << maxError << " px, precision " << bp.precision << std::endl;
		TEST_ASSERT(maxError <= bp.precision * 1.5);

		// Turning by offset gives the nodes of a larger start, within rounding.

		CoordSPX offset = pWarp->rotationOffset(20.f);
		double expected = -20.0 / bp.sweep * g.sourceAcross;
		TEST_ASSERT(bX ? (offset.x == 0 && std::abs(offset.y - expected) <= 0.5) : (offset.y == 0 && std::abs(offset.x - expected) <= 0.5));

		auto turnedBP = bp;
		turnedBP.startAngle += 20.f;
		auto pTurned = CylinderWarp::create(turnedBP, pFactory)->generate();
		for (int i = 0; i < nodes.w * nodes.h; i++)
		{
			CoordSPX a = pMap->nodeData()[i], b = pTurned->nodeData()[i];
			TEST_ASSERT(uDiff(b.x, a.x + offset.x, pMap->uPeriod()) <= 1 && uDiff(b.y, a.y + offset.y, pMap->vPeriod()) <= 1);
		}
	}

	// Settings count once generated.

	auto pWarp = CylinderWarp::create(cases[0], pFactory);
	CoordSPX before = pWarp->rotationOffset(90.f);
	TEST_ASSERT(pWarp->setAxis(Axis::Y));
	TEST_ASSERT(pWarp->setAngles(0.f, 180.f));
	TEST_ASSERT(pWarp->rotationOffset(90.f) == before);
	pWarp->generate();
	TEST_ASSERT(pWarp->rotationOffset(90.f) == CoordSPX(-source.w / 2, 0));

	// Settings, invalid ones refused.

	TEST_ASSERT(pWarp->setSpan(170.f) && pWarp->span() == 170.f);
	TEST_ASSERT(!pWarp->setSpan(180.f) && !pWarp->setSpan(0.f));
	TEST_ASSERT(!pWarp->setAxis(Axis::Undefined));
	TEST_ASSERT(!pWarp->setAngles(10.f, 0.f));

	TEST_ASSERT(CylinderWarp::create(WGBP(CylinderWarp, _.size = { 40, 40 }, _.sourceSize = source, _.span = 180.f), pFactory) == nullptr);
	TEST_ASSERT(CylinderWarp::create(WGBP(CylinderWarp, _.size = { 40, 40 }, _.sourceSize = source, _.axis = Axis::Undefined), pFactory) == nullptr);
	TEST_ASSERT(CylinderWarp::create(WGBP(CylinderWarp, _.size = { 40, 40 }, _.sourceSize = source, _.sweep = 0.f), pFactory) == nullptr);
	TEST_ASSERT(CylinderWarp::create(WGBP(CylinderWarp, _.size = { 40, 40 }), pFactory) == nullptr);			// No source size.
	return true;
}
