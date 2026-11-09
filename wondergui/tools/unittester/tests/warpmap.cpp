#define _USE_MATH_DEFINES		// For M_PI in MSVC.

#include "warpmap.h"

#include <wondergui.h>
#include <wax_softwarpmap.h>
#include <wax_softwarpmapfactory.h>
#include <wax_circularwarp.h>
#include <wax_roundedrectwarp.h>
#include <wax_cylinderwarp.h>
#include <wax_perspectivewarp.h>
#include <wax_ballwarp.h>
#include <wax_lenswarp.h>
#include <wax_meshcompression.h>
#include <cstring>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace wg;
using namespace wax;

// Tests of the Warpmap base class and the generators, which don't depend on any backend.
// Rendering through Warpmaps is tested by WarpRenderTest.

namespace
{
	bool near(double a, double b, double tolerance) { return std::abs(a - b) <= tolerance; }

	// Warpmap that counts the vertices and triangles it sends on, as a backend uploads them, and a
	// factory for it.

	class CountingWarpmap : public Warpmap
	{
	public:
		CountingWarpmap(const Blueprint& bp, const Vertex* pVertices, const Triangle* pTriangles) : Warpmap(bp, pVertices, pTriangles)
		{
			_updated(0, m_nVertices, 0, m_nTriangles);
		}

		int		updates = 0;
		int		verticesSent = 0;
		int		trianglesSent = 0;

	protected:
		void _updated(int firstVertex, int nVertices, int firstTriangle, int nTriangles) override
		{
			updates++;
			verticesSent += nVertices;
			trianglesSent += nTriangles;
		}
	};

	class CountingWarpmapFactory : public WarpmapFactory
	{
	public:
		Warpmap_p createWarpmap(const Warpmap::Blueprint& bp) override { return _created(new CountingWarpmap(bp, nullptr, nullptr)); }
		Warpmap_p createWarpmap(const Warpmap::Blueprint& bp, const Warpmap::Vertex* pVertices, const Warpmap::Triangle* pTriangles) override { return _created(new CountingWarpmap(bp, pVertices, pTriangles)); }

		int		created = 0;

	private:
		Warpmap_p _created(Warpmap* p) { created++; return p; }
	};

	// Position at point x,y (pixels) of a Warpmap, from the triangle it is in. False outside them.

	bool positionAt(const Warpmap* pMap, double x, double y, CoordF& pos)
	{
		int triangle = pMap->triangleAt({ spx(std::floor(x * 64)), spx(std::floor(y * 64)) });
		return triangle >= 0 && pMap->positionIn(triangle, x, y, pos);
	}

	CoordF positionAt(const Warpmap* pMap, double x, double y)
	{
		CoordF pos = { -1e30f, -1e30f };
		positionAt(pMap, x, y, pos);
		return pos;
	}

	// If no triangle has any area, so nothing is drawn.

	bool drawsNothing(const Warpmap* pMap)
	{
		const Warpmap::Vertex* v = pMap->vertexData();
		for (int t = 0; t < pMap->triangleCount(); t++)
		{
			const Warpmap::Triangle& tri = pMap->triangleData()[t];
			int64_t area = int64_t(v[tri.b].x - v[tri.a].x) * (v[tri.c].y - v[tri.a].y) - int64_t(v[tri.b].y - v[tri.a].y) * (v[tri.c].x - v[tri.a].x);
			if (area != 0)
				return false;
		}
		return true;
	}

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
	ADD_TEST(defaultMeshTest);
	ADD_TEST(importExportTest);
	ADD_TEST(createWithMeshTest);
	ADD_TEST(periodTest);
	ADD_TEST(fillRuleTest);
	ADD_TEST(perspectivePositionTest);
	ADD_TEST(circularWarpTest);
	ADD_TEST(circularWarpGenerateTest);
	ADD_TEST(generateUpdatesTest);
	ADD_TEST(roundedRectWarpTest);
	ADD_TEST(meshCompressionTest);
	ADD_TEST(offsetTest);
	ADD_TEST(cylinderWarpTest);
	ADD_TEST(perspectiveWarpTest);
	ADD_TEST(perspectiveRotatedTest);
	ADD_TEST(limitsTest);
	ADD_TEST(ballWarpTest);
	ADD_TEST(lensWarpTest);
	ADD_TEST(precisionTrianglesTest);
	ADD_TEST(outlinePrecisionTest);
}

WarpmapTest::~WarpmapTest()
{
}

//____ defaultMeshTest() ______________________________________________________
// A new Warpmap maps its rectangle straight onto the source, pixel for pixel, with two triangles.

bool WarpmapTest::defaultMeshTest(std::ostream& output)
{
	auto pMap = SoftWarpmap::create({ .size = { 100, 50 } });
	TEST_ASSERT(pMap != nullptr);
	TEST_ASSERT(pMap->vertexCount() == 4 && pMap->triangleCount() == 2);

	const Warpmap::Vertex* v = pMap->vertexData();
	TEST_ASSERT(v[2].x == 100 * 64 && v[2].y == 50 * 64 && v[2].u == 100 * 64 && v[2].v == 50 * 64 && v[2].w == Warpmap::weightOne);

	for (double y : { 0.0, 3.5, 17.0, 49.9 })
		for (double x : { 0.0, 0.5, 15.9, 99.0 })
		{
			int triangle = pMap->triangleAt({ spx(x * 64), spx(y * 64) });
			TEST_ASSERT(triangle >= 0);

			CoordF pos;
			TEST_ASSERT(pMap->positionIn(triangle, x, y, pos));
			TEST_ASSERT(near(pos.x, x * 64, 1e-3) && near(pos.y, y * 64, 1e-3));
		}

	// Points on the right and bottom edges, and outside, are in no triangle.

	TEST_ASSERT(pMap->triangleAt({ 100 * 64, 10 * 64 }) == -1);
	TEST_ASSERT(pMap->triangleAt({ 10 * 64, 50 * 64 }) == -1);
	TEST_ASSERT(pMap->triangleAt({ -1, 10 * 64 }) == -1);

	// Room for less than the rectangle: all triangles in the first vertex, which draw nothing.

	auto pEmpty = SoftWarpmap::create({ .size = { 10, 10 }, .triangles = 1, .vertices = 3 });
	TEST_ASSERT(pEmpty != nullptr && pEmpty->triangleAt({ 32, 32 }) == -1);

	CoordF pos;
	TEST_ASSERT(!pEmpty->positionIn(0, 0.5, 0.5, pos));								// No area.

	// More vertices and triangles than the rectangle needs, the rest draw nothing.

	auto pMore = SoftWarpmap::create({ .size = { 10, 10 }, .triangles = 5, .vertices = 7 });
	TEST_ASSERT(pMore->triangleAt({ 32, 32 }) == 0 && pMore->triangleData()[4].a == 0 && pMore->triangleData()[4].c == 0);

	// Invalid blueprints.

	TEST_ASSERT(SoftWarpmap::create({ .size = { 0, 10 } }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 10, 10 }, .uPeriod = -1 }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 10, 10 }, .vPeriod = Warpmap::maxValue + 1 }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 10, 10 }, .vertices = 2 }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 10, 10 }, .triangles = 0 }) == nullptr);
	TEST_ASSERT(SoftWarpmap::create({ .size = { 10, 10 }, .vertices = Warpmap::maxVertices + 1 }) == nullptr);

	return true;
}

//____ importExportTest() _____________________________________________________

bool WarpmapTest::importExportTest(std::ostream& output)
{
	auto pMap = SoftWarpmap::create({ .size = { 30, 20 }, .triangles = 4, .vertices = 6 });

	// Two vertices into the middle, one with values out of range that are clamped.

	Warpmap::Vertex in[2] = { { 64, 128, 320, -320, 1000 }, { Warpmap::maxPosition + 5, -Warpmap::maxPosition - 5, Warpmap::maxValue + 5, -Warpmap::maxValue - 5, 0 } };
	TEST_ASSERT(pMap->importVertices(3, 2, in));

	Warpmap::Vertex all[6];
	TEST_ASSERT(pMap->exportVertices(0, 6, all));

	TEST_ASSERT(all[3].x == 64 && all[3].y == 128 && all[3].u == 320 && all[3].v == -320 && all[3].w == 1000);
	TEST_ASSERT(all[4].x == Warpmap::maxPosition && all[4].y == -Warpmap::maxPosition && all[4].u == Warpmap::maxValue && all[4].v == -Warpmap::maxValue && all[4].w == 1);
	TEST_ASSERT(all[2].x == 30 * 64 && all[2].v == 20 * 64);						// Untouched.

	// Triangles, with corners that must be vertices.

	Warpmap::Triangle tris[2] = { { 3, 4, 5 }, { 0, 1, 5 } };
	TEST_ASSERT(pMap->importTriangles(2, 2, tris));

	Warpmap::Triangle out[4];
	TEST_ASSERT(pMap->exportTriangles(0, 4, out));
	TEST_ASSERT(out[0].a == 0 && out[0].b == 1 && out[0].c == 2);
	TEST_ASSERT(out[2].a == 3 && out[2].b == 4 && out[2].c == 5 && out[3][2] == 5);

	Warpmap::Triangle bad[2] = { { 0, 1, 2 }, { 0, 1, 6 } };
	TEST_ASSERT(!pMap->importTriangles(0, 2, bad));
	TEST_ASSERT(pMap->triangleData()[0].c == 2 && pMap->triangleData()[1].c == 3);		// Nothing changed.

	bad[1] = { -1, 0, 1 };
	TEST_ASSERT(!pMap->importTriangles(0, 2, bad));

	// Ranges outside the Warpmap are refused.

	TEST_ASSERT(!pMap->importVertices(5, 2, in));
	TEST_ASSERT(!pMap->importVertices(-1, 1, in));
	TEST_ASSERT(!pMap->importVertices(0, 1, nullptr));
	TEST_ASSERT(!pMap->exportVertices(0, 7, all));
	TEST_ASSERT(!pMap->importTriangles(3, 2, tris));
	TEST_ASSERT(!pMap->exportTriangles(4, 1, out));
	TEST_ASSERT(pMap->importVertices(6, 0, nullptr));								// Nothing, at the end.

	// All at once, either part can be kept.

	Warpmap::Vertex verts[6];
	Warpmap::Triangle triangles[4];
	for (int i = 0; i < 6; i++)
		verts[i] = { i * 64, i * 128, i, -i, 100 + i };
	for (int i = 0; i < 4; i++)
		triangles[i] = { i, i + 1, i + 2 };

	TEST_ASSERT(pMap->importMesh(verts, nullptr, 64, 0));
	TEST_ASSERT(pMap->vertexData()[5].w == 105 && pMap->triangleData()[2].a == 3 && pMap->uPeriod() == 64);
	TEST_ASSERT(pMap->importMesh(nullptr, triangles, 0, 0));
	TEST_ASSERT(pMap->triangleData()[3].c == 5 && pMap->vertexData()[5].w == 105 && pMap->uPeriod() == 0);

	triangles[3].c = 6;
	TEST_ASSERT(!pMap->importMesh(verts, triangles, 0, 0));
	TEST_ASSERT(!pMap->importMesh(verts, nullptr, -1, 0));
	return true;
}

//____ createWithMeshTest() ___________________________________________________
// A Warpmap created with vertices and triangles starts out with them and sends them on once.

bool WarpmapTest::createWithMeshTest(std::ostream& output)
{
	Warpmap::Blueprint bp = { .size = { 30, 20 }, .triangles = 3, .uPeriod = 640, .vertices = 5 };

	Warpmap::Vertex vertices[5];
	for (int i = 0; i < 5; i++)
		vertices[i] = { i * 300, i * 200, i * 100 - 700, -i, Warpmap::weightOne / (i + 1) };

	Warpmap::Triangle triangles[3] = { { 0, 1, 2 }, { 1, 2, 3 }, { 2, 3, 4 } };

	auto pFactory = StrongPtr<CountingWarpmapFactory>(new CountingWarpmapFactory());
	auto pMap = pFactory->createWarpmap(bp, vertices, triangles);
	auto pCounting = static_cast<CountingWarpmap*>(pMap.rawPtr());

	TEST_ASSERT(pCounting->updates == 1);
	TEST_ASSERT(pCounting->verticesSent == 5 && pCounting->trianglesSent == 3);

	// Values on periodic axes are kept as they are, they are wrapped after interpolation.

	for (int i = 0; i < 5; i++)
		TEST_ASSERT(pMap->vertexData()[i].u == vertices[i].u && pMap->vertexData()[i].w == vertices[i].w);

	// Same as a backend creating with content.

	auto pSoft = SoftWarpmap::create(bp, vertices, triangles);
	TEST_ASSERT(pSoft != nullptr);
	TEST_ASSERT(std::memcmp(pSoft->vertexData(), pMap->vertexData(), 5 * sizeof(Warpmap::Vertex)) == 0);
	TEST_ASSERT(std::memcmp(pSoft->triangleData(), pMap->triangleData(), 3 * sizeof(Warpmap::Triangle)) == 0);

	// Triangles with corners that aren't vertices, or no content, are refused.

	triangles[2].c = 5;
	TEST_ASSERT(SoftWarpmap::create(bp, vertices, triangles) == nullptr);
	TEST_ASSERT(SoftWarpmap::create(bp, vertices, nullptr) == nullptr);

	// Without content it maps straight onto the source, also sent on once.

	auto pDefault = pFactory->createWarpmap(bp);
	TEST_ASSERT(static_cast<CountingWarpmap*>(pDefault.rawPtr())->updates == 1);
	TEST_ASSERT(pFactory->created == 2);

	return true;
}

//____ periodTest() ___________________________________________________________
// Periods can be changed on their own, and positions are wrapped after interpolation, so vertices
// keep values close to each other across the seam.

bool WarpmapTest::periodTest(std::ostream& output)
{
	auto pFactory = StrongPtr<CountingWarpmapFactory>(new CountingWarpmapFactory());
	auto pMap = pFactory->createWarpmap({ .size = { 10, 10 }, .uPeriod = 640 });
	auto pCounting = static_cast<CountingWarpmap*>(pMap.rawPtr());

	Warpmap::Vertex vertices[4] = { { 0, 0, 608, 0 }, { 640, 0, 672, 0 }, { 640, 640, 672, 64 }, { 0, 640, 608, 64 } };
	TEST_ASSERT(pMap->importVertices(0, 4, vertices));
	TEST_ASSERT(pMap->vertexData()[1].u == 672);										// Not wrapped.

	// Halfway across the top edge we are on the seam, not halfway around.

	CoordF pos;
	TEST_ASSERT(pMap->positionIn(pMap->triangleAt({ 320, 32 }), 5, 0.5, pos));
	TEST_ASSERT(near(pos.x, 640, 1e-3));

	TEST_ASSERT(pMap->setPeriods(320, 64));
	TEST_ASSERT(pMap->uPeriod() == 320 && pMap->vPeriod() == 64);
	TEST_ASSERT(pCounting->updates == 3 && pCounting->verticesSent == 8);				// Created, imported, periods.

	// Invalid periods are refused and change nothing.

	TEST_ASSERT(!pMap->setPeriods(-1, 0));
	TEST_ASSERT(!pMap->setPeriods(0, Warpmap::maxValue + 1));
	TEST_ASSERT(pMap->uPeriod() == 320);
	return true;
}

//____ fillRuleTest() _________________________________________________________
// Each point of a mesh without gaps or overlaps is inside exactly one of its triangles, also on
// edges and at vertices shared by many triangles. Of the mesh's own border, left and top edges
// are inside and right and bottom ones outside. That doesn't depend on the order of corners.

bool WarpmapTest::fillRuleTest(std::ostream& output)
{
	// A square of 4 x 4 cells of 16 spx, every other cell split along the other diagonal, plus a
	// fan of 8 triangles around the middle of the middle 2 x 2 cells.

	std::vector<Warpmap::Vertex> vertices;
	std::vector<Warpmap::Triangle> triangles;

	auto vertex = [&](spx x, spx y)
	{
		for (int i = 0; i < int(vertices.size()); i++)
			if (vertices[i].x == x && vertices[i].y == y)
				return i;
		vertices.push_back({ x, y, x, y });
		return int(vertices.size()) - 1;
	};

	for (int row = 0; row < 4; row++)
		for (int col = 0; col < 4; col++)
		{
			if (row >= 1 && row <= 2 && col >= 1 && col <= 2)
				continue;

			int tl = vertex(col * 16, row * 16), tr = vertex(col * 16 + 16, row * 16);
			int br = vertex(col * 16 + 16, row * 16 + 16), bl = vertex(col * 16, row * 16 + 16);

			if ((row + col) % 2)
			{
				triangles.push_back({ tl, tr, br });
				triangles.push_back({ tl, br, bl });
			}
			else
			{
				triangles.push_back({ tl, tr, bl });
				triangles.push_back({ tr, br, bl });
			}
		}

	int center = vertex(32, 32);
	const CoordSPX ring[8] = { { 16, 16 }, { 32, 16 }, { 48, 16 }, { 48, 32 }, { 48, 48 }, { 32, 48 }, { 16, 48 }, { 16, 32 } };
	for (int i = 0; i < 8; i++)
		triangles.push_back({ center, vertex(ring[i].x, ring[i].y), vertex(ring[(i + 1) % 8].x, ring[(i + 1) % 8].y) });

	for (int pass = 0; pass < 2; pass++)
	{
		Warpmap::Blueprint bp;
		bp.size = { 1, 1 };
		bp.triangles = int(triangles.size());
		bp.vertices = int(vertices.size());

		auto pMap = SoftWarpmap::create(bp, vertices.data(), triangles.data());
		TEST_ASSERT(pMap != nullptr);

		for (spx y = -4; y <= 68; y++)
			for (spx x = -4; x <= 68; x++)
			{
				int count = 0;
				for (int t = 0; t < pMap->triangleCount(); t++)
					count += pMap->isInside(t, { x, y });

				int expected = (x >= 0 && x < 64 && y >= 0 && y < 64) ? 1 : 0;
				if (count != expected)
					output << "Point " << x << "," << y << " is inside " << count << " triangles" << std::endl;
				TEST_ASSERT(count == expected);
			}

		// Again with all triangles turned the other way.

		for (auto& t : triangles)
			std::swap(t.b, t.c);
	}

	return true;
}

//____ perspectivePositionTest() ______________________________________________
// Positions are interpolated with perspective: with weights following the divisor of a projective
// transform, they are that transform, also outside of the triangle. Beyond the horizon there is no
// position. With the same weight in all corners positions are linear, whatever the weight.

bool WarpmapTest::perspectivePositionTest(std::ostream& output)
{
	// u = (3x - y + 50) / d, v = (x + 2y - 20) / d, d = 1 + 0.004x + 0.008y, x and y in pixels, u and v in spx.

	auto divisor = [](double x, double y) { return 1.0 + 0.004 * x + 0.008 * y; };
	auto exactU = [&](double x, double y) { return (3 * x - y + 50) * 64 / divisor(x, y); };
	auto exactV = [&](double x, double y) { return (x + 2 * y - 20) * 64 / divisor(x, y); };

	const CoordI corners[4] = { { 0, 0 }, { 200, 0 }, { 200, 100 }, { 0, 100 } };
	Warpmap::Vertex vertices[4];
	for (int i = 0; i < 4; i++)
	{
		double x = corners[i].x, y = corners[i].y;
		vertices[i] = { spx(x * 64), spx(y * 64), spx(std::lround(exactU(x, y))), spx(std::lround(exactV(x, y))), int(std::lround(divisor(x, y) * 4096)) };
	}

	Warpmap::Triangle triangles[2] = { { 0, 1, 2 }, { 0, 2, 3 } };
	auto pMap = SoftWarpmap::create({ .size = { 200, 100 } }, vertices, triangles);
	TEST_ASSERT(pMap != nullptr);

	double maxError = 0;
	for (double y = 0.5; y < 100; y += 3.7)
		for (double x = 0.5; x < 200; x += 5.3)
		{
			int t = pMap->triangleAt({ spx(x * 64), spx(y * 64) });
			TEST_ASSERT(t >= 0);

			CoordF pos;
			TEST_ASSERT(pMap->positionIn(t, x, y, pos));
			maxError = std::max(maxError, std::max(std::abs(pos.x - exactU(x, y)), std::abs(pos.y - exactV(x, y))));

			// The other triangle gives the same, as it is the same plane.

			TEST_ASSERT(pMap->positionIn(1 - t, x, y, pos));
			maxError = std::max(maxError, std::max(std::abs(pos.x - exactU(x, y)), std::abs(pos.y - exactV(x, y))));
		}

	if (maxError > 0.5)
		output << "Max position error " << maxError << " spx" << std::endl;
	TEST_ASSERT(maxError <= 0.5);															// Rounding of u, v and weights.

	// Beyond the horizon, where the divisor is 0 (x + 2y = -250).

	CoordF pos;
	TEST_ASSERT(!pMap->positionIn(0, -300, 10, pos));
	TEST_ASSERT(pMap->positionIn(0, -200, 10, pos));

	// Same weight everywhere: linear.

	for (int i = 0; i < 4; i++)
		vertices[i].w = 777;
	TEST_ASSERT(pMap->importVertices(0, 4, vertices));
	TEST_ASSERT(pMap->positionIn(0, 150, 20, pos));

	double b0 = 1 - 150 / 200.0, b1 = 150 / 200.0 - 20 / 100.0, b2 = 20 / 100.0;			// Barycentric in triangle 0, 1, 2.
	TEST_ASSERT(near(pos.x, b0 * vertices[0].u + b1 * vertices[1].u + b2 * vertices[2].u, 1e-2));
	return true;
}

//____ circularWarpTest() _____________________________________________________
// CircularWarp against exact polar math: exact at the vertices, within 1 / precision pixels between them, and
// covering the ring and a pixel beyond it.

bool WarpmapTest::circularWarpTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();

	struct Case { CircularWarp::Blueprint bp; float tolerance; };

	Case cases[] = {
		{ WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 200, 200 }, _.topRadius = 98 * 64, _.bottomRadius = 60 * 64, _.precision = 4.f), 0.25f },	// Full ring, top outside.
		{ WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 300, 200 }, _.center = { 0.4f, 0.6f }, _.topRadius = 40 * 64, _.bottomRadius = 90 * 64,
			   _.startAngle = -135.f, _.sweep = 270.f, _.precision = 4.f), 0.25f },																	// Arc.
		{ WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 200, 200 }, _.topRadius = 30 * 64, _.bottomRadius = 99 * 64, _.sweep = -90.f, _.precision = 10.f), 0.1f },	// Counter clockwise, repeats 4 times.
		{ WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 160, 160 }, _.topRadius = 0, _.bottomRadius = 80 * 64, _.precision = 4.f), 0.25f },			// Disc.
		{ WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 400, 400 }, _.topRadius = 190 * 64, _.bottomRadius = 182 * 64, _.precision = 4.f), 0.25f }	// Thin and large.
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

		double rTop = c.bp.topRadius / 64.0, rBottom = c.bp.bottomRadius / 64.0;
		double rInner = std::min(rTop, rBottom), rOuter = std::max(rTop, rBottom);
		double cx = c.bp.center.x * c.bp.size.w, cy = c.bp.center.y * c.bp.size.h;

		// Exact at the vertices, other than in the center of a disc, where the angle is undefined.

		for (int i = 0; i < pMap->vertexCount(); i++)
		{
			const Warpmap::Vertex& v = pMap->vertexData()[i];
			double x = v.x / 64.0, y = v.y / 64.0;
			if (std::hypot(x - cx, y - cy) < 0.5)
				continue;

			// Measured on screen, since the vertex is rounded to spx.

			CoordF exact = exactCircular(c.bp, x, y);
			double r = std::hypot(x - cx, y - cy);
			double tangential = uDiff(v.u, exact.x, period) / c_sourceSize.w * std::abs(c.bp.sweep) / 180.0 * M_PI * r;
			double radial = std::abs(v.v - exact.y) / c_sourceSize.h * std::abs(rBottom - rTop);
			// u and v are rounded to spx of the source, which on a large ring is up to 0.03 pixels.

			if (tangential > 0.05 || radial > 0.05)
				output << "Vertex at " << x << ", " << y << " is " << v.u << ", " << v.v << ", expected " << exact.x << ", " << exact.y << std::endl;
			TEST_ASSERT(tangential <= 0.05 && radial <= 0.05);
		}

		// Within precision between them, as positions on screen, and every pixel of the ring and the
		// pixel around it is in a triangle.

		double maxError = 0;

		for (double y = 0.5; y < c.bp.size.h; y += 1.0)
			for (double x = 0.5; x < c.bp.size.w; x += 1.0)
			{
				double r = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
				if (r > rOuter + 0.7 || r < rInner - 0.7)
					continue;

				CoordF pos;
				if (!positionAt(pMap, x, y, pos))
					output << "Pixel " << x << ", " << y << " is in no triangle" << std::endl;
				TEST_ASSERT(positionAt(pMap, x, y, pos));

				if (r < std::max(rInner, 4.0) || r > rOuter)
					continue;

				CoordF exact = exactCircular(c.bp, x, y);
				double tangential = uDiff(pos.x, exact.x, period) / c_sourceSize.w * std::abs(c.bp.sweep) / 180.0 * M_PI * r;
				double radial = std::abs(pos.y - exact.y) / c_sourceSize.h * std::abs(rBottom - rTop);
				maxError = std::max(maxError, std::max(tangential, radial));
			}

		if (maxError > c.tolerance)
			output << "Max position error " << maxError << " px, allowed " << c.tolerance << std::endl;
		TEST_ASSERT(maxError <= c.tolerance);
	}

	// Higher precision, more triangles.

	auto pWarp = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.topRadius = 10 * 64, _.bottomRadius = 50 * 64, _.precision = 1.f), pFactory);
	int coarse = pWarp->generate()->triangleCount();
	TEST_ASSERT(pWarp->setPrecision(10.f));
	TEST_ASSERT(pWarp->generate()->triangleCount() > coarse * 3);

	// Invalid blueprints.

	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.sweep = 0.f), pFactory) == nullptr);
	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 0, 100 }), pFactory) == nullptr);
	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.size = { 100, 100 }), pFactory) == nullptr);		// No source size.

	// Equal radii have no band to map the source onto, nothing is drawn.

	auto pFlat = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 40, 40 }), pFactory);
	TEST_ASSERT(drawsNothing(pFlat->generate()));

	return true;
}

//____ circularWarpGenerateTest() ______________________________________________

bool WarpmapTest::circularWarpGenerateTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();

	auto pWarp = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.topRadius = 20 * 64, _.bottomRadius = 50 * 64), pFactory);

	TEST_ASSERT(pWarp->isInstanceOf(WarpGenerator::TYPEINFO));

	// A Warpmap that nothing but the CircularWarp holds is updated in place, as long as it has as
	// many vertices and triangles, which angles don't change.

	Warpmap* pFirst = pWarp->generate().rawPtr();
	TEST_ASSERT(pWarp->generate().rawPtr() == pFirst);						// Nothing changed.

	TEST_ASSERT(pWarp->setRadii(25 * 64, 45 * 64));
	Warpmap* pResized = pWarp->generate().rawPtr();

	TEST_ASSERT(pWarp->setAngles(90.f, 180.f));
	auto pMap1 = pWarp->generate();
	TEST_ASSERT(pMap1.rawPtr() == pResized);
	TEST_ASSERT(pMap1->uPeriod() == c_sourceSize.w * 2);

	// Point straight right of center at radius 25 is u = 0 (90 degrees), v = 0.

	CoordF pos = positionAt(pMap1, 75, 50);
	TEST_ASSERT(uDiff(pos.x, 0.0, pMap1->uPeriod()) <= 1.0 && near(pos.y, 0.0, 1.0));

	// A Warpmap held elsewhere, as by a draw that isn't rendered yet, is left as it is and a new
	// one is made. Source size scales the content and the period.

	int nVertices = pMap1->vertexCount();
	std::vector<Warpmap::Vertex> before(nVertices), after(nVertices);
	pMap1->exportVertices(0, nVertices, before.data());

	TEST_ASSERT(pWarp->generate() == pMap1);								// Nothing changed, held or not.

	TEST_ASSERT(pWarp->setSourceSize({ c_sourceSize.w / 2, c_sourceSize.h }));
	auto pMap2 = pWarp->generate();
	TEST_ASSERT(pMap2 != pMap1);
	TEST_ASSERT(pMap2->uPeriod() == c_sourceSize.w);
	TEST_ASSERT(pMap2->vertexData()[1].u != pMap1->vertexData()[1].u);

	pMap1->exportVertices(0, nVertices, after.data());
	TEST_ASSERT(pMap1->uPeriod() == c_sourceSize.w * 2);
	TEST_ASSERT(std::memcmp(before.data(), after.data(), nVertices * sizeof(Warpmap::Vertex)) == 0);
	pMap1 = nullptr;

	TEST_ASSERT(pWarp->setSourceSize(c_sourceSize));

	// Size and precision give a new Warpmap.

	TEST_ASSERT(pWarp->setSize({ 120, 100 }));
	auto pMap3 = pWarp->generate();
	TEST_ASSERT(pMap3 != pMap2 && pMap3->pixelSize() == SizeI(120, 100));

	TEST_ASSERT(pWarp->setPrecision(0.5f));
	auto pMap4 = pWarp->generate();
	TEST_ASSERT(pMap4 != pMap3 && pMap4->triangleCount() < pMap3->triangleCount());

	// Invalid values are refused.

	TEST_ASSERT(!pWarp->setRadii(10 * 64, 10 * 64));
	TEST_ASSERT(!pWarp->setAngles(0.f, 0.f));
	TEST_ASSERT(!pWarp->setSize({ 0, 10 }));
	TEST_ASSERT(!pWarp->setPrecision(0.f));
	TEST_ASSERT(!pWarp->setSourceSize({ 0, 10 }));
	return true;
}

//____ generateUpdatesTest() __________________________________________________
// generate() sends the content on once, whether it creates a Warpmap, updates one in place or changes
// its periods, and content updated in place is the same as that of a new Warpmap.

bool WarpmapTest::generateUpdatesTest(std::ostream& output)
{
	auto pCounting = StrongPtr<CountingWarpmapFactory>(new CountingWarpmapFactory());
	auto pSoft = SoftWarpmapFactory::create();

	auto bp = WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 80 }, _.topRadius = 20 * 64, _.bottomRadius = 45 * 64);
	auto pWarp = CircularWarp::create(bp, pCounting);

	auto counts = [](Warpmap* p) { return static_cast<CountingWarpmap*>(p); };

	// Created with its content.

	Warpmap* pMap = pWarp->generate();
	int vertices = pMap->vertexCount();
	int triangles = pMap->triangleCount();
	TEST_ASSERT(pCounting->created == 1);
	TEST_ASSERT(counts(pMap)->updates == 1 && counts(pMap)->verticesSent == vertices && counts(pMap)->trianglesSent == triangles);

	// Updated in place, also with new periods.

	auto sameAsNew = [&](Warpmap* pMap, const CircularWarp::Blueprint& bp)
	{
		auto pFresh = CircularWarp::create(bp, pSoft)->generate();
		return pFresh->uPeriod() == pMap->uPeriod() && pFresh->vertexCount() == vertices && pFresh->triangleCount() == triangles &&
			   std::memcmp(pMap->vertexData(), pFresh->vertexData(), vertices * sizeof(Warpmap::Vertex)) == 0 &&
			   std::memcmp(pMap->triangleData(), pFresh->triangleData(), triangles * sizeof(Warpmap::Triangle)) == 0;
	};

	TEST_ASSERT(pWarp->setAngles(10.f, 360.f));
	TEST_ASSERT(pWarp->generate() == pMap);
	TEST_ASSERT(counts(pMap)->updates == 2 && counts(pMap)->verticesSent == 2 * vertices);
	bp.startAngle = 10.f;
	TEST_ASSERT(sameAsNew(pMap, bp));

	spx oldPeriod = pMap->uPeriod();
	TEST_ASSERT(pWarp->setAngles(30.f, 120.f));
	TEST_ASSERT(pWarp->generate() == pMap);
	TEST_ASSERT(pMap->uPeriod() != oldPeriod);
	TEST_ASSERT(counts(pMap)->updates == 3 && counts(pMap)->verticesSent == 3 * vertices);
	bp.startAngle = 30.f;
	bp.sweep = 120.f;
	TEST_ASSERT(sameAsNew(pMap, bp));

	// Nothing changed, nothing sent.

	TEST_ASSERT(pWarp->generate() == pMap);
	TEST_ASSERT(counts(pMap)->updates == 3);

	// A held Warpmap is left alone, a new one is created with the content.

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

	// Outline of 100 x 60 pixels with corners of 12, in a band from 6 pixels inside it to 8 outside,
	// and the same with a band reaching 20 pixels inside, where corners are mitered.

	const double cx = 70, cy = 50, hx = 50, hy = 30, r = 12, bottom = 8;
	const double perimeter = 4 * (hx - r) + 4 * (hy - r) + 2 * M_PI * r;
	auto outline = sampleOutline(hx, hy, r, 0.01);

	for (double top : { -6.0, -20.0 })
	{
		auto pWarp = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 140, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 100 * 64, 60 * 64 },
												  _.cornerRadius = 12 * 64, _.topDistance = spx(top * 64), _.bottomDistance = spx(bottom * 64), _.precision = 4.f), pFactory);
		TEST_ASSERT(pWarp != nullptr);
		TEST_ASSERT(pWarp->isInstanceOf(WarpGenerator::TYPEINFO));

		auto pMap = pWarp->generate();
		TEST_ASSERT(pMap->uPeriod() == c_sourceSize.w);

		// Vertices are where the nearest point of the outline is. Those on the diagonals of mitered
		// corners, where the nearest point can be either side, and in the centers of corners, where
		// the position along the outline is that of the middle of their sector, are skipped.

		for (int i = 0; i < pMap->vertexCount(); i++)
		{
			const Warpmap::Vertex& v = pMap->vertexData()[i];
			double x = v.x / 64.0 - cx, y = v.y / 64.0 - cy;

			double qx = std::abs(x) - (hx - r), qy = std::abs(y) - (hy - r);
			if (std::abs(qx - qy) < 0.1 && qx <= 0.05 && qy <= 0.05)
				continue;
			if (std::hypot(qx, qy) < 0.1)
				continue;

			double pos, dist;
			nearestOnOutline(outline, hx, hy, r, x, y, pos, dist);

			double vertexPos = v.u / double(c_sourceSize.w) * perimeter;
			double vertexDist = v.v / double(c_sourceSize.h) * (bottom - top) + top;

			if (uDiff(vertexPos, pos, perimeter) > 0.05 || !near(vertexDist, dist, 0.05))
				output << "Vertex at " << x << ", " << y << " is " << vertexPos << ", " << vertexDist << ", nearest point " << pos << ", " << dist << std::endl;
			TEST_ASSERT(uDiff(vertexPos, pos, perimeter) <= 0.05 && near(vertexDist, dist, 0.05));
		}

		// Within precision between them, away from the mitered diagonals, and every pixel of the band
		// and around it is in a triangle.

		double maxError = 0;
		for (double y = 0.5; y < 100; y += 1.0)
			for (double x = 0.5; x < 140; x += 1.0)
			{
				double pos, dist;
				nearestOnOutline(outline, hx, hy, r, x - cx, y - cy, pos, dist);
				if (dist < top - 0.7 || dist > bottom + 0.7)
					continue;

				CoordF p;
				if (!positionAt(pMap, x, y, p))
					output << "Pixel " << x << ", " << y << " is in no triangle" << std::endl;
				TEST_ASSERT(positionAt(pMap, x, y, p));

				double qx = std::abs(x - cx) - (hx - r), qy = std::abs(y - cy) - (hy - r);
				if (dist < top || dist > bottom || (qx < 1 && qy < 1 && std::abs(qx - qy) < 1.5))
					continue;

				// Along the outline as on screen: around a corner a position along the outline moves
				// on screen by the distance to the center of the corner over its radius.

				double along = uDiff(p.x / double(c_sourceSize.w) * perimeter, pos, perimeter);
				if (qx > 0 && qy > 0)
					along *= std::hypot(qx, qy) / r;
				double across = std::abs(p.y / double(c_sourceSize.h) * (bottom - top) + top - dist);
				maxError = std::max(maxError, std::max(along, across));
			}

		if (maxError > 0.25)
			output << "Max position error " << maxError << " px" << std::endl;
		TEST_ASSERT(maxError <= 0.25);
	}

	// Start and sweep. The middle of the right side is a quarter of the way round, where the source
	// starts with a start of 0.25. A sweep of 0.5 makes the period twice the width of the source.

	const double top = -6;
	auto pWarp = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 140, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 100 * 64, 60 * 64 },
											  _.cornerRadius = 12 * 64, _.topDistance = -6 * 64, _.bottomDistance = 8 * 64), pFactory);

	TEST_ASSERT(pWarp->setSpan(0.25f, 0.5f));
	auto pMap = pWarp->generate();
	TEST_ASSERT(pMap->uPeriod() == c_sourceSize.w * 2);

	CoordF right = positionAt(pMap, cx + hx - 0.5, cy);
	TEST_ASSERT(uDiff(right.x, 0, pMap->uPeriod()) <= 1 && near(right.y, -(top + 0.5) / (bottom - top) * c_sourceSize.h, 1));

	CoordF below = positionAt(pMap, cx, cy + hy + 4);							// Middle of the bottom side, half way round.
	TEST_ASSERT(uDiff(below.x, c_sourceSize.w * 0.5, pMap->uPeriod()) <= 1 && near(below.y, (4 - top) / (bottom - top) * c_sourceSize.h, 1));

	// A square with corners of half its side is a circle, the same as a CircularWarp.

	auto pCircle = CircularWarp::create(WGBP(CircularWarp, _.size = { 100, 100 }, _.sourceSize = c_sourceSize, _.topRadius = 45 * 64, _.bottomRadius = 20 * 64,
											 _.startAngle = 30.f, _.sweep = -270.f), pFactory)->generate();
	auto pRounded = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 100, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 60 * 64, 60 * 64 },
												 _.cornerRadius = 30 * 64, _.topDistance = 15 * 64, _.bottomDistance = -10 * 64,
												 _.start = 30.f / 360, _.sweep = -0.75f), pFactory)->generate();

	TEST_ASSERT(pRounded->uPeriod() == pCircle->uPeriod());
	TEST_ASSERT(pRounded->vertexCount() == pCircle->vertexCount() && pRounded->triangleCount() == pCircle->triangleCount());

	for (int i = 0; i < pCircle->vertexCount(); i++)
	{
		const Warpmap::Vertex& a = pCircle->vertexData()[i];
		const Warpmap::Vertex& b = pRounded->vertexData()[i];
		TEST_ASSERT(std::abs(a.x - b.x) <= 1 && std::abs(a.y - b.y) <= 1 && uDiff(a.u, b.u, pCircle->uPeriod()) <= 1 && std::abs(a.v - b.v) <= 1);
	}

	// Less precision, fewer triangles. The straight sides take the same anyway.

	auto pAuto = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 140, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 100 * 64, 60 * 64 },
											  _.cornerRadius = 12 * 64, _.topDistance = -6 * 64, _.bottomDistance = 8 * 64, _.precision = 0.5f), pFactory);
	TEST_ASSERT(pAuto->generate()->triangleCount() < pWarp->generate()->triangleCount());

	// Equal distances have no band to map the source onto, nothing is drawn.

	auto pFlat = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 40, 40 }, _.sourceSize = c_sourceSize, _.rect = { 640, 640, 1280, 1280 }), pFactory);
	TEST_ASSERT(drawsNothing(pFlat->generate()));

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

//____ meshCompressionTest() __________________________________________________
// MeshCompressor restores sequences exactly, also when decompressed in place from the end of a
// buffer of maxCompressedSize() as GfxStream does, without writing past it. Meshes of generators
// compress to a fraction, data that doesn't to its raw size plus a byte, and broken data is
// refused without writing outside the buffer.

bool WarpmapTest::meshCompressionTest(std::ostream& output)
{
	auto pCompressor = MeshCompressor::create();
	auto pDecompressor = MeshDecompressor::create();

	TEST_ASSERT(wax::Base::getDecompressor(MeshCompressor::ID_TOKEN) != nullptr);
	TEST_ASSERT(!pCompressor->setSequences(0));

	const int guard = 16;

	// Compresses and decompresses in place, returns compressed size or -1 if not restored.

	auto roundTrip = [&](const std::vector<int32_t>& values, int sequences) -> int
	{
		pCompressor->setSequences(sequences);

		int raw = int(values.size() * 4);
		int bufferSize = pCompressor->maxCompressedSize(raw);
		std::vector<uint8_t> compressed(bufferSize);
		int size = pCompressor->compress(compressed.data(), values.data(), values.data() + values.size());
		if (size > bufferSize)
			return -1;

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

	// The mesh of a thick ring as GfxStream sends it: vertices as sequences of all x, y, u and v,
	// triangles of all a, b and c.

	auto pFactory = StrongPtr<CountingWarpmapFactory>(new CountingWarpmapFactory());
	auto pRing = CircularWarp::create(WGBP(CircularWarp, _.size = { 300, 300 }, _.sourceSize = c_sourceSize, _.topRadius = 64 * 148, _.bottomRadius = 64 * 90), pFactory);
	Warpmap_p pMap = pRing->generate();

	const int nV = pMap->vertexCount(), nT = pMap->triangleCount();
	std::vector<int32_t> vertices(nV * 4), triangles(nT * 3);
	for (int i = 0; i < nV; i++)
	{
		const Warpmap::Vertex& v = pMap->vertexData()[i];
		vertices[i] = v.x;
		vertices[nV + i] = v.y;
		vertices[nV * 2 + i] = v.u;
		vertices[nV * 3 + i] = v.v;
	}
	for (int i = 0; i < nT; i++)
	{
		const Warpmap::Triangle& t = pMap->triangleData()[i];
		triangles[i] = t.a;
		triangles[nT + i] = t.b;
		triangles[nT * 2 + i] = t.c;
	}

	int vertexBytes = roundTrip(vertices, 4);
	int triangleBytes = roundTrip(triangles, 3);
	if (vertexBytes * 4 >= nV * 16 || triangleBytes * 20 >= nT * 6)
		output << "Thick ring of " << nV << " vertices and " << nT << " triangles compressed to " << vertexBytes << " and " << triangleBytes << " bytes" << std::endl;
	TEST_ASSERT(vertexBytes > 0 && vertexBytes * 4 < nV * 16);				// Under a quarter of 16 bytes a vertex.
	TEST_ASSERT(triangleBytes > 0 && triangleBytes * 20 < nT * 6);			// Under a twentieth of 6 bytes a triangle.

	// As one sequence, and with the sequences cut in other places, still exact.

	TEST_ASSERT(roundTrip(vertices, 1) > 0);
	TEST_ASSERT(roundTrip(vertices, 2) > 0);

	// Random and extreme values, which are stored as they are, constant ones and long runs.

	uint32_t seed = 77;
	auto random = [&seed]() { seed = seed * 1664525 + 1013904223; return seed; };

	for (int i = 0; i < 300; i++)
	{
		int sequences = 1 + random() % 5, length = 1 + random() % 300;
		std::vector<int32_t> values(sequences * length);

		for (int k = 0; k < int(values.size()); k++)
		{
			int32_t& v = values[k];
			switch (i % 5)
			{
				case 0: v = int32_t(random()); break;
				case 1: v = (random() & 1) ? INT32_MIN : INT32_MAX; break;
				case 2: v = int32_t(random() % 256) - 128; break;
				case 3: v = (random() % 7 == 0) ? int32_t(random() % 9) : 0; break;				// Runs of zeros and short breaks.
				default: v = 12345 + (k % length) * int32_t(random() % 3); break;
			}
		}

		int size = roundTrip(values, sequences);
		TEST_ASSERT(size > 0 && size <= 1 + int(values.size() * 4));
	}

	int constant = roundTrip(std::vector<int32_t>(100000, -7), 4);
	TEST_ASSERT(constant > 0 && constant < 50);

	// Not whole sequences or 32-bit values, stored as they are.

	TEST_ASSERT(roundTrip(std::vector<int32_t>(14, 5), 4) == 1 + 56);
	pCompressor->setSequences(1);
	std::vector<uint8_t> odd(7, 3), oddOut(pCompressor->maxCompressedSize(7));
	TEST_ASSERT(pCompressor->compress(oddOut.data(), odd.data(), odd.data() + 7) == 1 + 7);

	// Broken data: cut short, or claiming more than fits in the buffer. Refused with an error, and
	// nothing is written past the end of the buffer.

	int errors = 0;
	auto previousHandler = wax::Base::errorHandler();
	wax::Base::setErrorHandler([&](wax::Error&) { errors++; });

	pCompressor->setSequences(4);
	int bufferSize = pCompressor->maxCompressedSize(int(vertices.size() * 4));
	std::vector<uint8_t> compressed(bufferSize);
	int size = pCompressor->compress(compressed.data(), vertices.data(), vertices.data() + vertices.size());

	for (int cut : { 1, 2, 5, size / 2, size - 1 })
	{
		std::vector<uint8_t> buffer(bufferSize + guard, 0xCD);
		std::memcpy(buffer.data() + bufferSize - cut, compressed.data(), cut);
		int before = errors;
		int restored = pDecompressor->decompress(buffer.data(), buffer.data() + bufferSize - cut, buffer.data() + bufferSize);
		TEST_ASSERT(errors > before && restored < int(vertices.size() * 4));
		for (int i = 0; i < guard; i++)
			TEST_ASSERT(buffer[bufferSize + i] == 0xCD);
	}

	{
		// One sequence of 10000 zeros, a run longer than the buffer of 64 bytes holds.

		const uint8_t huge[] = { 1, 1, 0x80 | (10000 >> 8), 10000 & 0xFF, 0, 0, 0x80 | (9999 >> 8), 9999 & 0xFF };
		std::vector<uint8_t> buffer(64 + guard, 0xCD);
		std::memcpy(buffer.data() + 64 - sizeof(huge), huge, sizeof(huge));
		int before = errors;
		TEST_ASSERT(pDecompressor->decompress(buffer.data(), buffer.data() + 64 - sizeof(huge), buffer.data() + 64) == 0);
		TEST_ASSERT(errors > before);
		for (int i = 0; i < guard; i++)
			TEST_ASSERT(buffer[64 + i] == 0xCD);
	}

	wax::Base::setErrorHandler(previousHandler);
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

	auto sameButOffset = [](Warpmap* pMoved, Warpmap* pBase, spx offset)
	{
		if (pMoved->vertexCount() != pBase->vertexCount())
			return false;

		for (int i = 0; i < pBase->vertexCount(); i++)
		{
			const Warpmap::Vertex& a = pBase->vertexData()[i];
			const Warpmap::Vertex& b = pMoved->vertexData()[i];
			if (a.x != b.x || a.y != b.y || uDiff(b.u, a.u + offset, pBase->uPeriod()) > 1 || std::abs(b.v - a.v) > 1)
				return false;
		}
		return true;
	};

	auto ringBP = WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.topRadius = 45 * 64, _.bottomRadius = 20 * 64,
					   _.startAngle = 10.f, _.sweep = 300.f);
	auto pBase = CircularWarp::create(ringBP, pFactory);
	spx offset = pBase->rotationOffset(45.f).x;
	TEST_ASSERT(offset == -width * 45 / 300);
	ringBP.startAngle += 45.f;
	TEST_ASSERT(sameButOffset(CircularWarp::create(ringBP, pFactory)->generate(), pBase->generate(), offset));

	// The same along a rounded rectangle, in parts of the outline.

	auto frameBP = WGBP(RoundedRectWarp, _.size = { 140, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 100 * 64, 60 * 64 },
						_.cornerRadius = 15 * 64, _.topDistance = -10 * 64, _.bottomDistance = 10 * 64, _.start = 0.1f, _.sweep = 0.5f);
	auto pFrame = RoundedRectWarp::create(frameBP, pFactory);
	TEST_ASSERT(pFrame->shiftOffset(0.25f) == CoordSPX(-width / 2, 0));
	TEST_ASSERT(pFrame->shiftOffset(3.25f) == CoordSPX(-width / 2, 0));

	offset = pFrame->shiftOffset(0.125f).x;
	TEST_ASSERT(offset == -width / 4);
	frameBP.start += 0.125f;
	TEST_ASSERT(sameButOffset(RoundedRectWarp::create(frameBP, pFactory)->generate(), pFrame->generate(), offset));

	frameBP.sweep = -0.5f;
	TEST_ASSERT(RoundedRectWarp::create(frameBP, pFactory)->shiftOffset(0.25f) == CoordSPX(width / 2, 0));
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
		WGBP(CylinderWarp, _.size = { 60, 120 }, _.sourceSize = source, _.span = 60.f, _.startAngle = -45.f, _.sweep = 90.f, _.precision = 10.f)
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

		// Strips along the axis, from one end to the other, narrower towards the rims.

		int nStrips = pMap->vertexCount() / 2 - 1;
		TEST_ASSERT(pMap->vertexCount() == (nStrips + 1) * 2 && pMap->triangleCount() == nStrips * 2);

		auto acrossAt = [&](int i) { const Warpmap::Vertex& v = pMap->vertexData()[i * 2]; return (bX ? v.y : v.x) / 64.0; };
		double middleStrip = 0, rimStrip = acrossAt(1) - acrossAt(0);
		for (int i = 0; i < nStrips; i++)
		{
			double a0 = acrossAt(i), a1 = acrossAt(i + 1);
			if (a0 <= g.lengthAcross / 2 && a1 >= g.lengthAcross / 2)
				middleStrip = a1 - a0;
		}
		TEST_ASSERT(middleStrip > rimStrip);

		// Exact at the vertices.

		for (int i = 0; i < pMap->vertexCount(); i++)
		{
			const Warpmap::Vertex& v = pMap->vertexData()[i];
			CoordF exact = exactCylinder(bp, v.x / 64.0, v.y / 64.0);
			TEST_ASSERT(uDiff(v.u, exact.x, pMap->uPeriod()) <= 0.51 && uDiff(v.v, exact.y, pMap->vPeriod()) <= 0.51);
		}

		// Within precision between them, as positions on screen: an error in angle of da (radians)
		// is R * cos(angle) * da pixels across.

		double maxError = 0.0;
		for (double a = 0.5; a < g.lengthAcross; a += 1.0)
			for (double l : { 0.5, g.lengthAlong / 2 })
			{
				double x = bX ? l : a, y = bX ? a : l;
				CoordF pos = positionAt(pMap, x, y);
				CoordF exact = exactCylinder(bp, x, y);
				double diff = bX ? uDiff(pos.y, exact.y, pMap->vPeriod()) : uDiff(pos.x, exact.x, pMap->uPeriod());
				double angle = cylinderAngle(g, a) * M_PI / 180;
				double error = diff / g.sourceAcross * std::abs(bp.sweep) * M_PI / 180 * g.radius * std::cos(angle);
				maxError = std::max(maxError, error);

				double alongDiff = std::abs((bX ? pos.x : pos.y) - (bX ? exact.x : exact.y));
				TEST_ASSERT(alongDiff <= 0.51);
			}

		const double allowed = 1.0 / bp.precision;
		if (maxError > allowed)
			output << "Max position error " << maxError << " px, allowed " << allowed << std::endl;
		TEST_ASSERT(maxError <= allowed);

		// Turning by offset gives the nodes of a larger start, within rounding.

		CoordSPX offset = pWarp->rotationOffset(20.f);
		double expected = -20.0 / bp.sweep * g.sourceAcross;
		TEST_ASSERT(bX ? (offset.x == 0 && std::abs(offset.y - expected) <= 0.5) : (offset.y == 0 && std::abs(offset.x - expected) <= 0.5));

		auto turnedBP = bp;
		turnedBP.startAngle += 20.f;
		auto pTurned = CylinderWarp::create(turnedBP, pFactory)->generate();
		TEST_ASSERT(pTurned->vertexCount() == pMap->vertexCount());
		for (int i = 0; i < pMap->vertexCount(); i++)
		{
			const Warpmap::Vertex& a = pMap->vertexData()[i];
			const Warpmap::Vertex& b = pTurned->vertexData()[i];
			TEST_ASSERT(uDiff(b.u, a.u + offset.x, pMap->uPeriod()) <= 1 && uDiff(b.v, a.v + offset.y, pMap->vPeriod()) <= 1);
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

//____ perspectiveWarpTest() __________________________________________________
// PerspectiveWarp against a projective transform solved from the four corners as a linear system,
// which is how it's usually done rather than how PerspectiveWarp does it. The Warpmap is a fan of
// triangles with weights, exact everywhere, covering every pixel the plane reaches into.

namespace
{
	// x' = (h[0]*x + h[1]*y + h[2]) / (h[6]*x + h[7]*y + 1), y' likewise with h[3..5].

	struct Homography
	{
		double h[8];

		CoordF map(double x, double y, double* pW = nullptr) const
		{
			double w = h[6] * x + h[7] * y + 1.0;
			if (pW)
				*pW = w;
			return { float((h[0] * x + h[1] * y + h[2]) / w), float((h[3] * x + h[4] * y + h[5]) / w) };
		}
	};

	Homography solveHomography(const CoordF from[4], const CoordF to[4])
	{
		double m[8][9] = {};
		for (int i = 0; i < 4; i++)
		{
			double x = from[i].x, y = from[i].y, u = to[i].x, v = to[i].y;
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

		Homography hm;
		for (int i = 0; i < 8; i++)
			hm.h[i] = m[i][8] / m[i][i];
		return hm;
	}

	// Corners of the Warpmap (pixels) to corners of the source (spx), and back.

	void perspectiveHomographies(const PerspectiveWarp::Blueprint& bp, Homography& toSource, Homography& toScreen)
	{
		CoordF screen[4], source[4] = { { 0, 0 }, { float(bp.sourceSize.w), 0 }, { float(bp.sourceSize.w), float(bp.sourceSize.h) }, { 0, float(bp.sourceSize.h) } };
		for (int i = 0; i < 4; i++)
			screen[i] = { bp.corners[i].x / 64.f, bp.corners[i].y / 64.f };

		toSource = solveHomography(screen, source);
		toScreen = solveHomography(source, screen);
	}
}

bool WarpmapTest::perspectiveWarpTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();
	const SizeSPX source = { 256 * 64, 128 * 64 };

	auto corners = [](CoordI a, CoordI b, CoordI c, CoordI d) { return CornersT<CoordSPX>(a * 64, b * 64, c * 64, d * 64); };

	PerspectiveWarp::Blueprint cases[] = {
		// A floor leaning back, a card turned and rolled seen from aside, a sign pinned at an angle, the same
		// mirrored, very close to the horizon, without perspective, and a floor reaching far outside the Warpmap.

		WGBP(PerspectiveWarp, _.size = { 240, 160 }, _.sourceSize = source, _.corners = PerspectiveWarp::rotated({ 20 * 64, 20 * 64, 200 * 64, 120 * 64 }, 50.f, 0.f, 0.f, 300 * 64)),
		WGBP(PerspectiveWarp, _.size = { 200, 200 }, _.sourceSize = source,
			 _.corners = PerspectiveWarp::rotated({ 40 * 64, 50 * 64, 120 * 64, 90 * 64 }, 10.f, 35.f, 15.f, 400 * 64, { 0.3f, 0.6f }, CoordSPX(150 * 64, 30 * 64))),
		WGBP(PerspectiveWarp, _.size = { 160, 120 }, _.sourceSize = source, _.corners = corners({ 10, 20 }, { 150, 5 }, { 130, 110 }, { 30, 90 })),
		WGBP(PerspectiveWarp, _.size = { 160, 120 }, _.sourceSize = source, _.corners = corners({ 150, 5 }, { 10, 20 }, { 30, 90 }, { 130, 110 })),
		WGBP(PerspectiveWarp, _.size = { 160, 120 }, _.sourceSize = source, _.corners = corners({ 70, 5 }, { 90, 5 }, { 155, 115 }, { 5, 115 })),
		WGBP(PerspectiveWarp, _.size = { 100, 80 }, _.sourceSize = source, _.corners = corners({ 10, 10 }, { 90, 20 }, { 80, 70 }, { 0, 60 })),
		WGBP(PerspectiveWarp, _.size = { 160, 120 }, _.sourceSize = source,
			 _.corners = CornersT<CoordSPX>({ 60 * 64, 10 * 64 }, { 100 * 64, 10 * 64 }, { 1 << 29, 1 << 29 }, { -(1 << 29), 1 << 29 }))
	};

	for (auto& bp : cases)
	{
		auto pWarp = PerspectiveWarp::create(bp, pFactory);
		TEST_ASSERT(pWarp != nullptr);

		auto pMap = pWarp->generate();
		TEST_ASSERT(pMap->uPeriod() == 0 && pMap->vPeriod() == 0);
		TEST_ASSERT(pMap->vertexCount() == 9 && pMap->triangleCount() == 7);

		Homography toSource, toScreen;
		perspectiveHomographies(bp, toSource, toScreen);

		// w of the corners, with the sign it has on the plane.

		double sign, lowest = 1e300;
		toSource.map(bp.corners[0].x / 64.0, bp.corners[0].y / 64.0, &sign);
		for (int i = 0; i < 4; i++)
		{
			double w;
			toSource.map(bp.corners[i].x / 64.0, bp.corners[i].y / 64.0, &w);
			lowest = std::min(lowest, w * sign);
		}

		// Vertices within the Warpmap, at their exact positions in the source give or take rounding.

		for (int i = 0; i < pMap->vertexCount(); i++)
		{
			const Warpmap::Vertex& v = pMap->vertexData()[i];
			TEST_ASSERT(v.x >= 0 && v.y >= 0 && v.x <= bp.size.w * 64 && v.y <= bp.size.h * 64);

			double w;
			CoordF exact = toSource.map(v.x / 64.0, v.y / 64.0, &w);
			TEST_ASSERT(w * sign > 0);

			double tolerance = std::max(1.0, std::max(std::abs(exact.x), std::abs(exact.y)) * 1e-6);
			if (std::abs(v.u - exact.x) > tolerance || std::abs(v.v - exact.y) > tolerance)
				output << "Vertex " << i << " is " << v.u << "," << v.v << ", expected " << exact.x << "," << exact.y << std::endl;
			TEST_ASSERT(std::abs(v.u - exact.x) <= tolerance && std::abs(v.v - exact.y) <= tolerance);
		}

		// Each pixel with any part of it on the plane is in a triangle, except a pixel by the horizon,
		// and positions are exact wherever there is a triangle. On screen, the position read is taken
		// back to the screen and compared with where it was read.

		auto onPlane = [&](double x, double y)
		{
			double w;
			CoordF pos = toSource.map(x, y, &w);
			return w * sign >= lowest * 0.6 && pos.x >= 0 && pos.y >= 0 && pos.x <= source.w && pos.y <= source.h;
		};

		double maxError = 0.0;
		CoordI worst;
		CoordF worstErrors;
		for (int y = 0; y < bp.size.h; y++)
			for (int x = 0; x < bp.size.w; x++)
			{
				double cx = x + 0.5, cy = y + 0.5;
				int t = pMap->triangleAt({ x * 64 + 32, y * 64 + 32 });

				bool bTouches = onPlane(cx, cy) || onPlane(cx - 0.7, cy - 0.7) || onPlane(cx + 0.7, cy - 0.7) || onPlane(cx - 0.7, cy + 0.7) || onPlane(cx + 0.7, cy + 0.7);
				if (bTouches && t < 0)
					output << "Pixel " << x << "," << y << " touches the plane but is in no triangle" << std::endl;
				TEST_ASSERT(t >= 0 || !bTouches);

				if (t < 0)
					continue;

				// Within 0.02 pixels on screen, or within the rounding of u and v to spx at the
				// vertices, which shows on screen where the source is enlarged.

				CoordF pos;
				TEST_ASSERT(pMap->positionIn(t, cx, cy, pos));
				CoordF back = toScreen.map(pos.x, pos.y);
				CoordF exact = toSource.map(cx, cy);

				double onScreen = std::hypot(back.x - cx, back.y - cy);
				double inSource = std::max(std::abs(pos.x - exact.x), std::abs(pos.y - exact.y));
				double error = std::min(onScreen / 0.02, inSource / 1.5);
				if (error > maxError)
				{
					maxError = error;
					worst = { x, y };
					worstErrors = { float(onScreen), float(inSource) };
				}
			}

		if (maxError > 1.0)
			output << "Case " << (&bp - cases) << ": pixel " << worst.x << "," << worst.y << " is off by " << worstErrors.x << " px on screen, "
				   << worstErrors.y << " spx in the source" << std::endl;
		TEST_ASSERT(maxError <= 1.0);
	}

	// Corners: refused unless convex with some area, and new ones are generated in place.

	auto pFlat = PerspectiveWarp::create(cases[5], pFactory);
	Warpmap* pFirst = pFlat->generate();

	TEST_ASSERT(!pFlat->setCorners(corners({ 10, 10 }, { 90, 60 }, { 90, 10 }, { 10, 60 })));		// Crossed.
	TEST_ASSERT(!pFlat->setCorners(corners({ 10, 10 }, { 50, 10 }, { 90, 10 }, { 10, 60 })));		// Three in a line.
	TEST_ASSERT(!pFlat->setCorners(corners({ 10, 10 }, { 90, 10 }, { 50, 20 }, { 10, 60 })));		// Not convex.
	TEST_ASSERT(!pFlat->setCorners(CornersT<CoordSPX>()));
	TEST_ASSERT(pFlat->corners() == cases[5].corners);

	TEST_ASSERT(pFlat->setCorners(cases[2].corners) && pFlat->corners() == cases[2].corners);
	TEST_ASSERT(pFlat->generate().rawPtr() == pFirst);
	TEST_ASSERT(pFirst->vertexData()[0].w != pFirst->vertexData()[2].w);					// Now in perspective.

	// Entirely outside the Warpmap: nothing to draw.

	TEST_ASSERT(pFlat->setCorners(corners({ 200, 10 }, { 300, 10 }, { 300, 60 }, { 200, 60 })));
	pFlat->generate();
	for (int t = 0; t < pFirst->triangleCount(); t++)
	{
		const Warpmap::Triangle& tri = pFirst->triangleData()[t];
		TEST_ASSERT(tri.a == tri.b && tri.b == tri.c);
	}

	TEST_ASSERT(PerspectiveWarp::create(WGBP(PerspectiveWarp, _.size = { 40, 40 }, _.sourceSize = source), pFactory) == nullptr);		// No corners.
	TEST_ASSERT(PerspectiveWarp::create(WGBP(PerspectiveWarp, _.size = { 40, 40 }, _.corners = cases[2].corners), pFactory) == nullptr);	// No source size.
	return true;
}

//____ perspectiveRotatedTest() _______________________________________________
// PerspectiveWarp::rotated() against rotation matrices and a projection in floating point.

namespace
{
	CornersT<CoordF> exactRotated(const RectSPX& rect, double tilt, double turn, double roll, double distance, CoordF pivot, CoordF eye)
	{
		const double r = M_PI / 180;
		double px = rect.x + pivot.x * rect.w, py = rect.y + pivot.y * rect.h;

		CoordF points[4] = { { float(rect.x), float(rect.y) }, { float(rect.x + rect.w), float(rect.y) },
							 { float(rect.x + rect.w), float(rect.y + rect.h) }, { float(rect.x), float(rect.y + rect.h) } };

		CornersT<CoordF> out;
		for (int i = 0; i < 4; i++)
		{
			double x = points[i].x - px, y = points[i].y - py, z = 0;

			double x1 = x * std::cos(roll * r) - y * std::sin(roll * r), y1 = x * std::sin(roll * r) + y * std::cos(roll * r), z1 = z;
			double x2 = x1 * std::cos(turn * r) - z1 * std::sin(turn * r), y2 = y1, z2 = x1 * std::sin(turn * r) + z1 * std::cos(turn * r);
			double x3 = x2, y3 = y2 * std::cos(tilt * r) + z2 * std::sin(tilt * r), z3 = z2 * std::cos(tilt * r) - y2 * std::sin(tilt * r);

			double sx = px + x3, sy = py + y3;
			if (distance > 0)
			{
				sx = eye.x + (sx - eye.x) * distance / (distance + z3);
				sy = eye.y + (sy - eye.y) * distance / (distance + z3);
			}
			out[i] = { float(sx), float(sy) };
		}
		return out;
	}
}

bool WarpmapTest::perspectiveRotatedTest(std::ostream& output)
{
	const RectSPX rect = { 30 * 64, 40 * 64, 200 * 64, 120 * 64 };

	struct Case { float tilt, turn, roll; spx distance; CoordF pivot; std::optional<CoordSPX> eye; };

	Case cases[] = {
		{ 0.f, 0.f, 0.f, 500 * 64, { 0.5f, 0.5f }, {} },
		{ 50.f, 0.f, 0.f, 300 * 64, { 0.5f, 1.f }, {} },
		{ 0.f, -70.f, 0.f, 250 * 64, { 0.f, 0.5f }, {} },
		{ 20.f, 30.f, -25.f, 400 * 64, { 0.25f, 0.75f }, CoordSPX(300 * 64, -40 * 64) },
		{ -35.f, 160.f, 400.f, 900 * 64, { 0.5f, 0.5f }, {} },
		{ 60.f, 0.f, 0.f, 0, { 0.5f, 0.5f }, {} }													// No perspective.
	};

	for (auto& c : cases)
	{
		CornersT<CoordSPX> corners = PerspectiveWarp::rotated(rect, c.tilt, c.turn, c.roll, c.distance, c.pivot, c.eye);

		CoordF eye = c.eye ? CoordF(float(c.eye->x), float(c.eye->y)) : CoordF(rect.x + c.pivot.x * rect.w, rect.y + c.pivot.y * rect.h);
		CornersT<CoordF> exact = exactRotated(rect, c.tilt, c.turn, c.roll, c.distance, c.pivot, eye);

		for (int i = 0; i < 4; i++)
		{
			if (std::abs(corners[i].x - exact[i].x) > 1.0 || std::abs(corners[i].y - exact[i].y) > 1.0)
				output << "Corner " << i << " is " << corners[i].x << "," << corners[i].y << ", expected " << exact[i].x << "," << exact[i].y << std::endl;
			TEST_ASSERT(std::abs(corners[i].x - exact[i].x) <= 1.0 && std::abs(corners[i].y - exact[i].y) <= 1.0);
		}
	}

	// Not rotated, the rectangle itself.

	TEST_ASSERT(PerspectiveWarp::rotated(rect, 0.f, 0.f, 0.f, 500 * 64) == CornersT<CoordSPX>(rect.pos(), CoordSPX(rect.x + rect.w, rect.y),
				CoordSPX(rect.x + rect.w, rect.y + rect.h), CoordSPX(rect.x, rect.y + rect.h)));

	// Seen edge on, or partly behind the eye, the corners are refused.

	auto pFactory = SoftWarpmapFactory::create();
	auto pWarp = PerspectiveWarp::create(WGBP(PerspectiveWarp, _.size = { 260, 200 }, _.sourceSize = { 64 * 64, 64 * 64 },
		_.corners = PerspectiveWarp::rotated(rect, 0.f, 0.f, 0.f, 500 * 64)), pFactory);
	TEST_ASSERT(pWarp != nullptr);

	TEST_ASSERT(!pWarp->setCorners(PerspectiveWarp::rotated(rect, 0.f, 90.f, 0.f, 500 * 64)));
	TEST_ASSERT(!pWarp->setCorners(PerspectiveWarp::rotated(rect, 0.f, 90.f, 0.f, 0)));
	TEST_ASSERT(!pWarp->setCorners(PerspectiveWarp::rotated(rect, 80.f, 0.f, 0.f, 50 * 64)));

	// Turned past 90 degrees, the back faces the viewer: corners in the reverse order, which mirror the source.

	CornersT<CoordSPX> back = PerspectiveWarp::rotated(rect, 0.f, 150.f, 0.f, 500 * 64);
	TEST_ASSERT(pWarp->setCorners(back) && back.topLeft.x > back.topRight.x);
	return true;
}

//____ limitsTest() ___________________________________________________________
// Corners of triangles are 16 bits, which limits vertices to maxVertices. Generators keep within
// it however large the shape and high the precision, by making the pieces larger.

namespace
{
	// Checks blueprints as the factories of the backends do.

	class BlueprintCheck : public Warpmap
	{
	public:
		static bool valid(const Blueprint& bp) { return _validateBlueprint(bp); }
	};
}

bool WarpmapTest::limitsTest(std::ostream& output)
{
	auto pFactory = StrongPtr<CountingWarpmapFactory>(new CountingWarpmapFactory());

	TEST_ASSERT(sizeof(Warpmap::Triangle) == 3 * sizeof(uint16_t));

	TEST_ASSERT(BlueprintCheck::valid(WGBP(Warpmap, _.size = { 10, 10 }, _.triangles = Warpmap::maxTriangles, _.vertices = Warpmap::maxVertices)));
	TEST_ASSERT(!BlueprintCheck::valid(WGBP(Warpmap, _.size = { 10, 10 }, _.vertices = Warpmap::maxVertices + 1)));
	TEST_ASSERT(!BlueprintCheck::valid(WGBP(Warpmap, _.size = { 10, 10 }, _.triangles = Warpmap::maxTriangles + 1)));
	TEST_ASSERT(SoftWarpmapFactory::create()->createWarpmap(WGBP(Warpmap, _.size = { 10, 10 }, _.vertices = Warpmap::maxVertices + 1)) == nullptr);

	// The last vertex can be a corner, and corners beyond the vertices are refused.

	auto pMap = pFactory->createWarpmap(WGBP(Warpmap, _.size = { 10, 10 }, _.vertices = Warpmap::maxVertices));
	Warpmap::Triangle last = { 0, 1, Warpmap::maxVertices - 1 };
	TEST_ASSERT(pMap->importTriangles(1, 1, &last));

	Warpmap::Triangle back;
	TEST_ASSERT(pMap->exportTriangles(1, 1, &back));
	TEST_ASSERT(back.a == 0 && back.b == 1 && back[2] == Warpmap::maxVertices - 1);

	auto pSmall = pFactory->createWarpmap(WGBP(Warpmap, _.size = { 10, 10 }, _.vertices = 100));
	Warpmap::Triangle outside = { 0, 1, 100 };
	TEST_ASSERT(!pSmall->importTriangles(0, 1, &outside));

	// Generators with more pieces than fit.

	auto fits = [&](Warpmap* p, const char* pWhat)
	{
		if (!p || p->vertexCount() > Warpmap::maxVertices || p->triangleCount() > Warpmap::maxTriangles)
		{
			output << pWhat << ": " << (p ? p->vertexCount() : -1) << " vertices, " << (p ? p->triangleCount() : -1) << " triangles" << std::endl;
			return false;
		}
		return true;
	};

	auto pRing = CircularWarp::create(WGBP(CircularWarp, _.size = { 4000, 4000 }, _.sourceSize = { 64 * 64, 64 * 64 }, _.topRadius = 64 * 1995,
										   _.bottomRadius = 64 * 5, _.precision = 100.f), pFactory);
	Warpmap_p pRingMap = pRing->generate();
	TEST_ASSERT(fits(pRingMap, "Fine ring"));
	TEST_ASSERT(pRingMap->vertexCount() > Warpmap::maxVertices / 2);		// Close to the limit, not much coarser.

	auto pFrame = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 4000, 3000 }, _.sourceSize = { 64 * 64, 64 * 64 },
											   _.rect = { 64 * 100, 64 * 100, 64 * 3800, 64 * 2800 }, _.cornerRadius = 64 * 900,
											   _.topDistance = 64 * 50, _.bottomDistance = -64 * 1300, _.precision = 100.f), pFactory);
	TEST_ASSERT(fits(pFrame->generate(), "Fine frame"));

	auto pTallCylinder = CylinderWarp::create(WGBP(CylinderWarp, _.size = { 100, 120000 }, _.sourceSize = { 64 * 64, 64 * 64 }, _.precision = 100.f), pFactory);
	TEST_ASSERT(fits(pTallCylinder->generate(), "Very long cylinder"));

	return true;
}

//____ ballWarpTest() _________________________________________________________
// A ball covered by a map: positions within the tolerance of the exact ones, seen from any side,
// no holes, an outline fading as a straight edge would, updated in place when tilted and rolled,
// and spun by an offset.

bool WarpmapTest::ballWarpTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();
	const SizeSPX source = { 512 * 64, 256 * 64 };

	TEST_ASSERT(BallWarp::create(WGBP(BallWarp, _.size = { 100, 100 }, _.sourceSize = source, _.tilt = 91.f), pFactory) == nullptr);
	TEST_ASSERT(BallWarp::create(WGBP(BallWarp, _.size = { 100, 100 }, _.sourceSize = {} ), pFactory) == nullptr);

	auto pBall = BallWarp::create(WGBP(BallWarp, _.size = { 241, 260 }, _.sourceSize = source, _.precision = 4.f), pFactory);
	TEST_ASSERT(pBall);
	TEST_ASSERT(pBall->radius() == 241 * 32 - 32);
	TEST_ASSERT(!pBall->setOrientation(-90.5f, 0.f));

	const double radius = pBall->radius() / 64.0;
	const double cx = 241 / 2.0, cy = 260 / 2.0;

	// Where a point of the ball (longitude and latitude in radians) is seen, and the reverse.

	auto toScreen = [&](double lon, double lat, double tilt, double roll, double& sx, double& sy)
	{
		double x = std::cos(lat) * std::sin(lon), y = -std::sin(lat), z = std::cos(lat) * std::cos(lon);
		double y1 = y * std::cos(tilt) + z * std::sin(tilt);
		double zt = z * std::cos(tilt) - y * std::sin(tilt);
		sx = cx + radius * (x * std::cos(roll) - y1 * std::sin(roll));
		sy = cy + radius * (x * std::sin(roll) + y1 * std::cos(roll));
		return zt;
	};

	auto toBall = [&](double sx, double sy, double tilt, double roll, double& lon, double& lat)
	{
		double px = (sx - cx) / radius, py = (sy - cy) / radius;
		double x = px * std::cos(roll) + py * std::sin(roll);
		double y1 = -px * std::sin(roll) + py * std::cos(roll);
		double zt = std::sqrt(std::max(0.0, 1 - x * x - y1 * y1));
		double y = y1 * std::cos(tilt) - zt * std::sin(tilt);
		double z = y1 * std::sin(tilt) + zt * std::cos(tilt);
		lon = std::atan2(x, z);
		lat = std::asin(std::clamp(-y, -1.0, 1.0));
	};

	int nVertices = -1, nTriangles = -1;
	const Warpmap* pFirst = nullptr;

	for (float tilt : { 0.f, 23.5f, -50.f, 90.f, -90.f, 75.f })
	{
		for (float roll : { 0.f, 30.f, -135.f })
		{
			TEST_ASSERT(pBall->setOrientation(tilt, roll));
			Warpmap_p pMap = pBall->generate();

			// The same Warpmap and counts whatever the orientation.

			if (!pFirst)
			{
				pFirst = pMap;
				nVertices = pMap->vertexCount();
				nTriangles = pMap->triangleCount();
			}
			TEST_ASSERT(pMap == pFirst && pMap->vertexCount() == nVertices && pMap->triangleCount() == nTriangles);

			double t = tilt * M_PI / 180, r = roll * M_PI / 180;
			double worst = 0;

			for (int k = 0; k < 4000; k++)
			{
				// Points within the ball, more of them close to the outline.

				double a = k * 2.399963, d = radius * (1 - std::pow((k % 97) / 97.0, 3)) - 1.0;
				double sx = cx + std::cos(a) * std::max(d, 0.0), sy = cy + std::sin(a) * std::max(d, 0.0);

				int triangle = pMap->triangleAt({ spx(std::floor(sx * 64)), spx(std::floor(sy * 64)) });
				if (triangle < 0)
				{
					output << "No triangle at " << sx << ", " << sy << " (tilt " << tilt << ", roll " << roll << ")" << std::endl;
					return false;
				}

				CoordF pos;
				TEST_ASSERT(pMap->positionIn(triangle, sx, sy, pos));

				// Where the position from the Warpmap is on the ball, against the point.

				double u = std::fmod(pos.x / double(source.w), 1.0);
				if (u < 0)
					u += 1;
				double lon = (u - 0.5) * 2 * M_PI, lat = (0.5 - pos.y / double(source.h)) * M_PI;
				double px, py;
				toScreen(lon, lat, t, r, px, py);
				worst = std::max(worst, std::hypot(px - sx, py - sy));
			}

			if (worst > 0.25)
			{
				output << "Position off by " << worst << " pixels (tilt " << tilt << ", roll " << roll << ")" << std::endl;
				return false;
			}

			// Coverage across the outline as for a straight edge: 0.5 - distance outside it, off by as
			// much as the outline is, straight between crossings of the grid, within precision.

			for (int k = 0; k < 720; k++)
			{
				double a = k * M_PI / 360;
				for (double d : { -0.45, -0.3, -0.1, 0.0, 0.15, 0.3, 0.45 })
				{
					double sx = cx + std::cos(a) * (radius + d), sy = cy + std::sin(a) * (radius + d);
					const double tolerance = 0.25 + 0.03;
					double expected = std::clamp(0.5 - d, 0.0, 1.0);

					int triangle = pMap->triangleAt({ spx(std::lround(sx * 64)), spx(std::lround(sy * 64)) });
					if (triangle < 0)
					{
						if (expected <= tolerance)
							continue;							// Just outside the outline, which is within precision.

						output << "No triangle at the outline, " << d << " pixels out" << std::endl;
						return false;
					}

					const Warpmap::Triangle& tri = pMap->triangleData()[triangle];
					const Warpmap::Vertex* c[3] = { &pMap->vertexData()[tri.a], &pMap->vertexData()[tri.b], &pMap->vertexData()[tri.c] };
					double e1x = c[1]->x - c[0]->x, e1y = c[1]->y - c[0]->y, e2x = c[2]->x - c[0]->x, e2y = c[2]->y - c[0]->y;
					double area = e1x * e2y - e1y * e2x;
					double dx = sx * 64 - c[0]->x, dy = sy * 64 - c[0]->y;
					double b1 = (dx * e2y - dy * e2x) / area, b2 = (e1x * dy - e1y * dx) / area;
					double coverage = ((1 - b1 - b2) * c[0]->coverage + b1 * c[1]->coverage + b2 * c[2]->coverage) / Warpmap::coverageOne;

					if (std::abs(std::clamp(coverage, 0.0, 1.0) - expected) > tolerance)
					{
						output << "Coverage " << coverage << " at " << d << " pixels out (tilt " << tilt << ", roll " << roll << ")" << std::endl;
						return false;
					}
				}

				// Nothing beyond the outline.

				double sx = cx + std::cos(a) * (radius + 0.75), sy = cy + std::sin(a) * (radius + 0.75);
				TEST_ASSERT(pMap->triangleAt({ spx(std::lround(sx * 64)), spx(std::lround(sy * 64)) }) < 0);
			}
		}
	}

	// Spun by an offset, the way the Earth turns: what faces the viewer moves west.

	CoordSPX offset = pBall->rotationOffset(90.f);
	TEST_ASSERT(offset.y == 0 && std::abs(offset.x + source.w / 4) <= 1);
	TEST_ASSERT(pBall->generate()->uPeriod() == source.w && pBall->generate()->vPeriod() == 0);

	// Within the limits of Warpmap however large and fine.

	auto pHuge = BallWarp::create(WGBP(BallWarp, _.size = { 4000, 4000 }, _.sourceSize = source, _.precision = 100.f), pFactory);
	Warpmap_p pHugeMap = pHuge->generate();
	TEST_ASSERT(pHugeMap && pHugeMap->vertexCount() <= Warpmap::maxVertices && pHugeMap->triangleCount() <= Warpmap::maxTriangles);

	return true;
}

//____ lensWarpTest() _________________________________________________________
// A lens magnifying what is under it: positions within the tolerance of the exact ones for any
// magnification and falloff, no holes, an outline fading as a straight edge would, meeting what
// is around it at the rim with falloff, and updated in place without.

bool WarpmapTest::lensWarpTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();
	const SizeI size = { 241, 260 };
	const SizeSPX source = { 241 * 64 * 2, 260 * 64 };					// Stretched sideways, which the lens follows.

	TEST_ASSERT(LensWarp::create(WGBP(LensWarp, _.size = size, _.sourceSize = source, _.magnification = 0.9f), pFactory) == nullptr);
	TEST_ASSERT(LensWarp::create(WGBP(LensWarp, _.size = size, _.sourceSize = source, _.falloff = 1.5f), pFactory) == nullptr);
	TEST_ASSERT(LensWarp::create(WGBP(LensWarp, _.size = size, _.sourceSize = source, _.falloff = -0.1f), pFactory) == nullptr);
	TEST_ASSERT(LensWarp::create(WGBP(LensWarp, _.size = size, _.sourceSize = {}), pFactory) == nullptr);

	auto pLens = LensWarp::create(WGBP(LensWarp, _.size = size, _.sourceSize = source, _.precision = 4.f), pFactory);
	TEST_ASSERT(pLens);
	TEST_ASSERT(pLens->radius() == 241 * 32 - 32);
	TEST_ASSERT(!pLens->setMagnification(0.5f) && !pLens->setFalloff(2.f));
	TEST_ASSERT(pLens->magnification() == 2.f && pLens->falloff() == 0.f);

	const double R = pLens->radius() / 64.0;
	const double cx = size.w / 2.0, cy = size.h / 2.0;
	const double toDestX = double(size.w) * 64 / source.w, toDestY = double(size.h) * 64 / source.h;

	// How far from the middle of the source a point r pixels from the middle of the lens shows,
	// in pixels of the Warpmap, and the reverse.

	auto profile = [&](double r, double m, double falloff)
	{
		double w = R * falloff, r0 = R - w;
		if (r <= r0 || w == 0)
			return r / m;
		if (r >= R)
			return r;
		double t = (r - r0) / w;
		return (2 * t * t * t - 3 * t * t + 1) * r0 / m + (t * t * t - 2 * t * t + t) * w / m + (-2 * t * t * t + 3 * t * t) * R + (t * t * t - t * t) * w;
	};

	auto inverse = [&](double s, double m, double falloff)
	{
		double lo = 0, hi = R + 1;
		for (int i = 0; i < 60; i++)
		{
			double mid = (lo + hi) / 2;
			(profile(mid, m, falloff) < s ? lo : hi) = mid;
		}
		return (lo + hi) / 2;
	};

	for (float falloff : { 0.f, 0.02f, 0.1f, 0.3f, 1.f })
	{
		int nVertices = -1, nTriangles = -1;
		const Warpmap* pFirst = nullptr;

		for (float m : { 1.f, 2.f, 4.5f, 12.f })
		{
			TEST_ASSERT(pLens->setFalloff(falloff) && pLens->setMagnification(m));
			Warpmap_p pMap = pLens->generate();

			TEST_ASSERT(pMap->uPeriod() == 0 && pMap->vPeriod() == 0);

			// Without falloff, magnification only changes positions in the source.

			if (falloff == 0.f)
			{
				if (!pFirst)
				{
					pFirst = pMap;
					nVertices = pMap->vertexCount();
					nTriangles = pMap->triangleCount();
				}
				TEST_ASSERT(pMap == pFirst && pMap->vertexCount() == nVertices && pMap->triangleCount() == nTriangles);
			}

			double worst = 0, worstSqueezed = 0;

			for (int k = 0; k < 4000; k++)
			{
				// Points within the lens, more of them towards the rim.

				double a = k * 2.399963, d = std::max(R * (1 - std::pow((k % 97) / 97.0, 3)) - 0.6, 0.0);
				double sx = cx + std::cos(a) * d, sy = cy + std::sin(a) * d;

				int triangle = pMap->triangleAt({ spx(std::floor(sx * 64)), spx(std::floor(sy * 64)) });
				if (triangle < 0)
				{
					output << "No triangle at " << sx << ", " << sy << " (magnification " << m << ", falloff " << falloff << ")" << std::endl;
					return false;
				}

				CoordF pos;
				TEST_ASSERT(pMap->positionIn(triangle, sx, sy, pos));

				// Where the position from the Warpmap is shown by the exact lens, against the point.
				// Rings are at least two pixels apart, so over the first pixel of the falloff and where
				// it squeezes the source into each pixel, positions follow the falloff less closely:
				// within 1.5 times the tolerance where it is squeezed up to 4 times, not measured where
				// it is squeezed more. They are still continuous, so it doesn't show.

				double r0 = R * (1 - falloff);
				double squeeze = (profile(d + 0.05, m, falloff) - profile(std::max(d - 0.05, 0.0), m, falloff)) / (d + 0.05 - std::max(d - 0.05, 0.0));
				if (squeeze > 4.0 || (falloff > 0.f && d > r0 - 0.5 && d < r0 + 1.0))
					continue;

				double ux = (pos.x - source.w / 2.0) * toDestX / 64, uy = (pos.y - source.h / 2.0) * toDestY / 64;
				double r = inverse(std::hypot(ux, uy), m, falloff), angle = std::atan2(uy, ux);
				double error = std::hypot(cx + std::cos(angle) * r - sx, cy + std::sin(angle) * r - sy);
				if (squeeze > 1.0)
					worstSqueezed = std::max(worstSqueezed, error);
				else
					worst = std::max(worst, error);
			}

			if (worst > 0.25 || worstSqueezed > 0.25 * 1.5)
			{
				worst = std::max(worst, worstSqueezed);
				output << "Position off by " << worst << " pixels (magnification " << m << ", falloff " << falloff << ")" << std::endl;
				return false;
			}

			// With falloff, the rim shows what is around it.

			if (falloff > 0.f)
			{
				for (int i = 0; i < pMap->vertexCount(); i++)
				{
					const Warpmap::Vertex& v = pMap->vertexData()[i];
					if (v.coverage == Warpmap::coverageOne / 2)
						TEST_ASSERT(std::abs(v.u - v.x / toDestX) <= 2 && std::abs(v.v - v.y / toDestY) <= 2);
				}
			}

			// Coverage across the outline as for a straight edge, off by as much as the outline is.

			for (int k = 0; k < 720; k++)
			{
				double a = k * M_PI / 360;
				for (double d : { -0.45, -0.3, -0.1, 0.0, 0.15, 0.3, 0.45 })
				{
					double sx = cx + std::cos(a) * (R + d), sy = cy + std::sin(a) * (R + d);
					const double tolerance = 0.25 + 0.03;
					double expected = std::clamp(0.5 - d, 0.0, 1.0);

					int triangle = pMap->triangleAt({ spx(std::lround(sx * 64)), spx(std::lround(sy * 64)) });
					if (triangle < 0)
					{
						if (expected <= tolerance)
							continue;

						output << "No triangle at the outline, " << d << " pixels out" << std::endl;
						return false;
					}

					const Warpmap::Triangle& tri = pMap->triangleData()[triangle];
					const Warpmap::Vertex* c[3] = { &pMap->vertexData()[tri.a], &pMap->vertexData()[tri.b], &pMap->vertexData()[tri.c] };
					double e1x = c[1]->x - c[0]->x, e1y = c[1]->y - c[0]->y, e2x = c[2]->x - c[0]->x, e2y = c[2]->y - c[0]->y;
					double area = e1x * e2y - e1y * e2x;
					double dx = sx * 64 - c[0]->x, dy = sy * 64 - c[0]->y;
					double b1 = (dx * e2y - dy * e2x) / area, b2 = (e1x * dy - e1y * dx) / area;
					double coverage = ((1 - b1 - b2) * c[0]->coverage + b1 * c[1]->coverage + b2 * c[2]->coverage) / Warpmap::coverageOne;

					if (std::abs(std::clamp(coverage, 0.0, 1.0) - expected) > tolerance)
					{
						output << "Coverage " << coverage << " at " << d << " pixels out (magnification " << m << ", falloff " << falloff << ")" << std::endl;
						return false;
					}
				}

				double sx = cx + std::cos(a) * (R + 0.75), sy = cy + std::sin(a) * (R + 0.75);
				TEST_ASSERT(pMap->triangleAt({ spx(std::lround(sx * 64)), spx(std::lround(sy * 64)) }) < 0);
			}
		}
	}

	// Within the limits of Warpmap however large and fine, and with tiny lenses.

	auto pHuge = LensWarp::create(WGBP(LensWarp, _.size = { 4000, 4000 }, _.sourceSize = { 4000 * 64, 4000 * 64 }, _.falloff = 0.2f, _.magnification = 30.f, _.precision = 100.f), pFactory);
	Warpmap_p pHugeMap = pHuge->generate();
	TEST_ASSERT(pHugeMap && pHugeMap->vertexCount() <= Warpmap::maxVertices && pHugeMap->triangleCount() <= Warpmap::maxTriangles);

	for (int tiny : { 1, 2, 3 })
	{
		auto pTiny = LensWarp::create(WGBP(LensWarp, _.size = { tiny, tiny }, _.sourceSize = { tiny * 64, tiny * 64 }, _.falloff = 0.5f), pFactory);
		TEST_ASSERT(pTiny && pTiny->generate());
	}

	return true;
}

//____ precisionTrianglesTest() _______________________________________________
// Precision decides the number of triangles the same way for all generators that use it: never
// fewer for higher precision, and clearly more from 1 to 10. How close each keeps to its
// precision is tested with the generator.

bool WarpmapTest::precisionTrianglesTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();

	struct Generator { const char* pName; WarpGenerator_p pGenerator; };

	Generator generators[] = {
		{ "ring", CircularWarp::create(WGBP(CircularWarp, _.size = { 200, 200 }, _.sourceSize = c_sourceSize, _.topRadius = 98 * 64, _.bottomRadius = 60 * 64), pFactory) },
		{ "disc", CircularWarp::create(WGBP(CircularWarp, _.size = { 160, 160 }, _.sourceSize = c_sourceSize, _.bottomRadius = 80 * 64), pFactory) },
		{ "frame", RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 140, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 100 * 64, 60 * 64 },
												 _.cornerRadius = 12 * 64, _.topDistance = -6 * 64, _.bottomDistance = 8 * 64), pFactory) },
		{ "reel", CylinderWarp::create(WGBP(CylinderWarp, _.size = { 120, 200 }, _.sourceSize = { 1024 * 64, 1024 * 64 }, _.span = 170.f), pFactory) },
		{ "ball", BallWarp::create(WGBP(BallWarp, _.size = { 100, 100 }, _.sourceSize = { 512 * 64, 256 * 64 }, _.tilt = 23.5f), pFactory) },
		{ "glass", LensWarp::create(WGBP(LensWarp, _.size = { 120, 120 }, _.sourceSize = { 120 * 64, 120 * 64 }), pFactory) },
		{ "thick glass", LensWarp::create(WGBP(LensWarp, _.size = { 120, 120 }, _.sourceSize = { 120 * 64, 120 * 64 }, _.magnification = 3.f, _.falloff = 0.3f), pFactory) }
	};

	for (auto& g : generators)
	{
		TEST_ASSERT(g.pGenerator);

		int previous = 0, coarse = 0;
		for (float precision : { 1.f, 2.f, 4.f, 10.f })
		{
			TEST_ASSERT(g.pGenerator->setPrecision(precision));
			int triangles = g.pGenerator->generate()->triangleCount();
			if (triangles < previous)
			{
				output << g.pName << ": " << triangles << " triangles at precision " << precision << ", fewer than " << previous << " at a lower one" << std::endl;
				return false;
			}
			if (precision == 1.f)
				coarse = triangles;
			previous = triangles;
		}

		if (previous < coarse * 3 / 2)
		{
			output << g.pName << ": " << previous << " triangles at precision 10, " << coarse << " at 1" << std::endl;
			return false;
		}
	}
	return true;
}

//____ outlinePrecisionTest() _________________________________________________
// Curved outlines stay within WarpGenerator's outline tolerance, half a pixel, however low the
// precision: where a ring crosses the top of its source, and where the coverage of the outline of
// a ball and a lens is a half, found along rays from the middle.

bool WarpmapTest::outlinePrecisionTest(std::ostream& output)
{
	auto pFactory = SoftWarpmapFactory::create();
	const double tolerance = 0.5 + 0.03;

	// Distance along a ray at angle a from (cx, cy) between lo and hi where value() crosses target.

	auto crossing = [](double cx, double cy, double a, double lo, double hi, const std::function<double(double x, double y)>& value, double target)
	{
		bool bLow = value(cx + std::cos(a) * lo, cy + std::sin(a) * lo) > target;
		for (int i = 0; i < 40; i++)
		{
			double mid = (lo + hi) / 2;
			if ((value(cx + std::cos(a) * mid, cy + std::sin(a) * mid) > target) == bLow)
				lo = mid;
			else
				hi = mid;
		}
		return (lo + hi) / 2;
	};

	auto coverageAt = [](const Warpmap* pMap, double x, double y)
	{
		int triangle = pMap->triangleAt({ spx(std::lround(x * 64)), spx(std::lround(y * 64)) });
		if (triangle < 0)
			return 0.0;
		const Warpmap::Triangle& tri = pMap->triangleData()[triangle];
		const Warpmap::Vertex* c[3] = { &pMap->vertexData()[tri.a], &pMap->vertexData()[tri.b], &pMap->vertexData()[tri.c] };
		double e1x = c[1]->x - c[0]->x, e1y = c[1]->y - c[0]->y, e2x = c[2]->x - c[0]->x, e2y = c[2]->y - c[0]->y;
		double area = e1x * e2y - e1y * e2x, dx = x * 64 - c[0]->x, dy = y * 64 - c[0]->y;
		double b1 = (dx * e2y - dy * e2x) / area, b2 = (e1x * dy - e1y * dx) / area;
		return ((1 - b1 - b2) * c[0]->coverage + b1 * c[1]->coverage + b2 * c[2]->coverage) / Warpmap::coverageOne;
	};

	auto check = [&](const char* pWhat, const Warpmap* pMap, double c, double exact, double lo, double hi, const std::function<double(double x, double y)>& value, double target)
	{
		double worst = 0;
		for (int k = 0; k < 720; k++)
			worst = std::max(worst, std::abs(crossing(c, c, 2 * M_PI * (k + 0.37) / 720, lo, hi, value, target) - exact));
		if (worst > tolerance)
			output << pWhat << ": outline off by " << worst << " pixels at precision 0.5" << std::endl;
		return worst <= tolerance;
	};

	Warpmap_p pRing = CircularWarp::create(WGBP(CircularWarp, _.size = { 120, 120 }, _.sourceSize = c_sourceSize, _.topRadius = 58 * 64, _.bottomRadius = 44 * 64, _.precision = 0.5f), pFactory)->generate();
	TEST_ASSERT(check("Ring", pRing, 60, 58, 51, 58.9, [&](double x, double y) { return positionAt(pRing, x, y).y; }, 0.0));

	Warpmap_p pBall = BallWarp::create(WGBP(BallWarp, _.size = { 160, 160 }, _.sourceSize = { 256 * 64, 128 * 64 }, _.tilt = 23.5f, _.roll = 10.f, _.precision = 0.5f), pFactory)->generate();
	double R = (160 * 32 - 32) / 64.0;
	TEST_ASSERT(check("Ball", pBall, 80, R, R - 3, R + 0.9, [&](double x, double y) { return coverageAt(pBall, x, y); }, 0.5));

	for (float falloff : { 0.f, 0.3f })
	{
		Warpmap_p pLens = LensWarp::create(WGBP(LensWarp, _.size = { 160, 160 }, _.sourceSize = { 160 * 64, 160 * 64 }, _.falloff = falloff, _.precision = 0.5f), pFactory)->generate();
		TEST_ASSERT(check("Lens", pLens, 80, R, R - 3, R + 0.9, [&](double x, double y) { return coverageAt(pLens, x, y); }, 0.5));
	}
	return true;
}
