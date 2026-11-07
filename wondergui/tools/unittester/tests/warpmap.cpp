#define _USE_MATH_DEFINES		// For M_PI in MSVC.

#include "warpmap.h"

#include <wondergui.h>
#include <wax_softwarpmap.h>
#include <wax_softwarpmapfactory.h>
#include <wax_circularwarp.h>
#include <wax_roundedrectwarp.h>
#include <wax_cylinderwarp.h>
#include <wax_perspectivewarp.h>
#include <wax_nodecompression.h>
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

	// The grid of nodes that a WarpGenerator made a Warpmap from, read back from its vertices: four
	// for each cell, cells row by row, corners top-left, top-right, bottom-right and bottom-left
	// (see WarpGenerator::_gridToMesh()). Values on periodic axes are brought into 0 -> period.
	// positionAt() reads the triangles, which split cells from top-left to bottom-right.

	class Grid
	{
	public:
		Grid(const Warpmap* pMap) : m_pMap(pMap)
		{
			const Warpmap::Vertex* p = pMap->vertexData();
			int nCells = pMap->vertexCount() / 4;

			m_cells.w = 0;
			while (m_cells.w < nCells && p[m_cells.w * 4].y == p[0].y)
				m_cells.w++;
			m_cells.h = nCells / m_cells.w;

			m_nodes = { m_cells.w + 1, m_cells.h + 1 };
			m_cellSize = { (p[1].x - p[0].x) / 64, (p[3].y - p[0].y) / 64 };
			m_data.resize(m_nodes.w * m_nodes.h);
			m_x.resize(m_nodes.w);
			m_y.resize(m_nodes.h);

			auto wrap = [](spx value, spx period) { return period > 0 ? ((value % period) + period) % period : value; };

			for (int row = 0; row < m_nodes.h; row++)
				for (int col = 0; col < m_nodes.w; col++)
				{
					int cellCol = std::min(col, m_cells.w - 1);
					int cellRow = std::min(row, m_cells.h - 1);
					bool bRight = col > cellCol, bBottom = row > cellRow;
					int corner = bBottom ? (bRight ? 2 : 3) : (bRight ? 1 : 0);

					const Warpmap::Vertex& v = p[(cellRow * m_cells.w + cellCol) * 4 + corner];
					m_data[row * m_nodes.w + col] = { wrap(v.u, pMap->uPeriod()), wrap(v.v, pMap->vPeriod()) };
					m_x[col] = v.x / 64;
					m_y[row] = v.y / 64;
				}
		}

		SizeI			nodes() const { return m_nodes; }
		SizeI			cellSize() const { return m_cellSize; }
		int				nodeX(int col) const { return m_x[col]; }
		int				nodeY(int row) const { return m_y[row]; }
		const CoordSPX*	nodeData() const { return m_data.data(); }

		CoordF positionAt(double x, double y) const
		{
			int col = 0, row = 0;
			while (col < m_cells.w - 1 && x >= m_x[col + 1])
				col++;
			while (row < m_cells.h - 1 && y >= m_y[row + 1])
				row++;

			double fx = (x - m_x[col]) / (m_x[col + 1] - m_x[col]);
			double fy = (y - m_y[row]) / (m_y[row + 1] - m_y[row]);
			int triangle = (row * m_cells.w + col) * 2 + (fx >= fy ? 0 : 1);

			CoordF pos;
			m_pMap->positionIn(triangle, x, y, pos);
			return pos;
		}

	private:
		const Warpmap*			m_pMap;
		SizeI					m_cells, m_nodes, m_cellSize;
		std::vector<CoordSPX>	m_data;
		std::vector<int>		m_x, m_y;
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
	ADD_TEST(nodeCompressionTest);
	ADD_TEST(offsetTest);
	ADD_TEST(nonSquareCellsTest);
	ADD_TEST(cylinderWarpTest);
	ADD_TEST(perspectiveWarpTest);
	ADD_TEST(perspectiveRotatedTest);
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

		// Cell size from precision: sqrt(4 * precision * r) at the smaller radius, or half the radius of a disc.

		double inner = std::min(c.bp.topRadius, c.bp.bottomRadius) / 64.0;
		double detail = inner > 0 ? inner : std::max(c.bp.topRadius, c.bp.bottomRadius) / 64.0 / 2;
		int expectedCell = std::clamp(int(std::sqrt(4.0 * c.bp.precision * detail)), 4, 64);
		Grid grid(pMap);
		TEST_ASSERT(grid.cellSize() == SizeI(expectedCell, expectedCell));
		TEST_ASSERT(pWarp->cellSize() == SizeI(expectedCell, expectedCell));

		// Exact at nodes.

		SizeI nodes = grid.nodes();
		for (int row = 0; row < nodes.h; row++)
			for (int col = 0; col < nodes.w; col++)
			{
				if (grid.nodeX(col) == c.bp.center.x * c.bp.size.w && grid.nodeY(row) == c.bp.center.y * c.bp.size.h)
					continue;																			// No angle at the center.

				CoordF exact = exactCircular(c.bp, grid.nodeX(col), grid.nodeY(row));
				CoordSPX node = grid.nodeData()[row * nodes.w + col];
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
				CoordF pos = grid.positionAt(x, y);

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
	TEST_ASSERT(Grid(pWarp->generate()).cellSize() == SizeI(7, 7));

	// Invalid blueprints.

	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 100 }, _.sweep = 0.f), pFactory) == nullptr);
	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 0, 100 }), pFactory) == nullptr);
	TEST_ASSERT(CircularWarp::create(WGBP(CircularWarp, _.size = { 100, 100 }), pFactory) == nullptr);		// No source size.

	// Equal radii have no band to map the source onto, the nodes are clamped far outside it.

	auto pFlat = CircularWarp::create(WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 40, 40 }, _.cellSize = { 10, 10 }), pFactory);
	Grid flat(pFlat->generate());
	for (int i = 0; i < flat.nodes().w * flat.nodes().h; i++)
		TEST_ASSERT(flat.nodeData()[i].y == -Warpmap::maxValue);

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

	CoordF pos = Grid(pMap1).positionAt(75, 50);
	TEST_ASSERT(near(pos.x, 0.0, 1.0) && near(pos.y, 0.0, 1.0));

	// A Warpmap held elsewhere, as by a draw that isn't rendered yet, is left as it is and a new
	// one is made. Source size scales the nodes and the period.

	int nVertices = pMap1->vertexCount();
	std::vector<Warpmap::Vertex> before(nVertices), after(nVertices);
	pMap1->exportVertices(0, nVertices, before.data());

	TEST_ASSERT(pWarp->generate() == pMap1);								// Nothing changed, held or not.

	TEST_ASSERT(pWarp->setSourceSize({ c_sourceSize.w / 2, c_sourceSize.h }));
	auto pMap2 = pWarp->generate();
	TEST_ASSERT(pMap2 != pMap1);
	TEST_ASSERT(pMap2->uPeriod() == c_sourceSize.w);
	TEST_ASSERT(pMap2->vertexData()[0].u != pMap1->vertexData()[0].u);

	pMap1->exportVertices(0, nVertices, after.data());
	TEST_ASSERT(pMap1->uPeriod() == c_sourceSize.w * 2);
	TEST_ASSERT(std::memcmp(before.data(), after.data(), nVertices * sizeof(Warpmap::Vertex)) == 0);
	pMap1 = nullptr;

	TEST_ASSERT(pWarp->setSourceSize(c_sourceSize));

	// Size and cell size give a new Warpmap.

	TEST_ASSERT(pWarp->setSize({ 120, 100 }));
	auto pMap3 = pWarp->generate();
	TEST_ASSERT(pMap3 != pMap2 && pMap3->pixelSize() == SizeI(120, 100));

	TEST_ASSERT(pWarp->setCellSize({ 0, 0 }));
	TEST_ASSERT(pWarp->setPrecision(1.f));
	auto pMap4 = pWarp->generate();
	TEST_ASSERT(pMap4 != pMap3 && Grid(pMap4).cellSize() == SizeI(int(std::sqrt(4.0 * 1.0 * 25)), int(std::sqrt(4.0 * 1.0 * 25))));		// At the smaller radius.

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
// generate() sends the content on once, whether it creates a Warpmap, updates one in place or changes
// its periods, and content updated in place is the same as that of a new Warpmap.

bool WarpmapTest::generateUpdatesTest(std::ostream& output)
{
	auto pCounting = StrongPtr<CountingWarpmapFactory>(new CountingWarpmapFactory());
	auto pSoft = SoftWarpmapFactory::create();

	auto bp = WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 100, 80 }, _.topRadius = 20 * 64, _.bottomRadius = 45 * 64, _.cellSize = { 10, 10 });
	auto pWarp = CircularWarp::create(bp, pCounting);

	auto counts = [](Warpmap* p) { return static_cast<CountingWarpmap*>(p); };

	// Created with its content, four vertices and two triangles for each cell.

	Warpmap* pMap = pWarp->generate();
	int vertices = pMap->vertexCount();
	TEST_ASSERT(vertices == 10 * 8 * 4 && pMap->triangleCount() == 10 * 8 * 2);
	TEST_ASSERT(pCounting->created == 1);
	TEST_ASSERT(counts(pMap)->updates == 1 && counts(pMap)->verticesSent == vertices && counts(pMap)->trianglesSent == vertices / 2);

	// Updated in place, and with new periods.

	auto sameAsNew = [&](Warpmap* pMap, const CircularWarp::Blueprint& bp)
	{
		auto pFresh = CircularWarp::create(bp, pSoft)->generate();
		return pFresh->uPeriod() == pMap->uPeriod() && std::memcmp(pMap->vertexData(), pFresh->vertexData(), vertices * sizeof(Warpmap::Vertex)) == 0 &&
			   std::memcmp(pMap->triangleData(), pFresh->triangleData(), vertices / 2 * sizeof(Warpmap::Triangle)) == 0;
	};

	TEST_ASSERT(pWarp->setRadii(25 * 64, 40 * 64));
	TEST_ASSERT(pWarp->generate() == pMap);
	TEST_ASSERT(counts(pMap)->updates == 2 && counts(pMap)->verticesSent == 2 * vertices);
	bp.topRadius = 25 * 64;
	bp.bottomRadius = 40 * 64;
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

	Grid grid(pMap);
	SizeI nodes = grid.nodes();
	for (int row = 0; row < nodes.h; row++)
		for (int col = 0; col < nodes.w; col++)
		{
			double x = grid.nodeX(col) - cx, y = grid.nodeY(row) - cy;
			if (std::abs(x) < hx - r && std::abs(y) < hy - r)
				continue;

			double pos, dist;
			nearestOnOutline(outline, hx, hy, r, x, y, pos, dist);

			CoordSPX node = grid.nodeData()[row * nodes.w + col];
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

	CoordF right = Grid(pMap).positionAt(cx + hx, cy);
	TEST_ASSERT(near(right.x, 0, 1) && near(right.y, -top / (bottom - top) * c_sourceSize.h, 1));

	CoordF below = Grid(pMap).positionAt(cx, cy + hy + 4);							// Middle of the bottom side, half way round.
	TEST_ASSERT(near(below.x, c_sourceSize.w * 0.5, 1) && near(below.y, (4 - top) / (bottom - top) * c_sourceSize.h, 1));

	// A square with corners of half its side is a circle, the same as a CircularWarp.

	auto pCircle = CircularWarp::create(WGBP(CircularWarp, _.size = { 100, 100 }, _.sourceSize = c_sourceSize, _.topRadius = 45 * 64, _.bottomRadius = 20 * 64,
											 _.cellSize = { 6, 6 }, _.startAngle = 30.f, _.sweep = -270.f), pFactory)->generate();
	auto pRounded = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 100, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 60 * 64, 60 * 64 },
												 _.cornerRadius = 30 * 64, _.topDistance = 15 * 64, _.bottomDistance = -10 * 64, _.cellSize = { 6, 6 },
												 _.start = 30.f / 360, _.sweep = -0.75f), pFactory)->generate();

	TEST_ASSERT(pRounded->uPeriod() == pCircle->uPeriod());
	Grid circle(pCircle), rounded(pRounded);
	TEST_ASSERT(rounded.nodes() == circle.nodes());

	nodes = circle.nodes();
	for (int i = 0; i < nodes.w * nodes.h; i++)
	{
		CoordSPX a = circle.nodeData()[i], b = rounded.nodeData()[i];
		TEST_ASSERT(uDiff(a.x, b.x, pCircle->uPeriod()) <= 1 && std::abs(a.y - b.y) <= 1);
	}

	// Cell size from precision at the tightest corner of the band: the corner radius less the
	// distance inwards. Sharp corners take half the radius of the outer edge, as a disc.

	auto pAuto = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 140, 100 }, _.sourceSize = c_sourceSize, _.rect = { 20 * 64, 20 * 64, 100 * 64, 60 * 64 },
											  _.cornerRadius = 12 * 64, _.topDistance = -6 * 64, _.bottomDistance = 8 * 64, _.precision = 2.f), pFactory);
	TEST_ASSERT(pAuto->cellSize() == SizeI(int(std::sqrt(4.0 * 2 * 6)), int(std::sqrt(4.0 * 2 * 6))));
	TEST_ASSERT(pAuto->setOutline({ 20 * 64, 20 * 64, 100 * 64, 60 * 64 }, 0));
	TEST_ASSERT(pAuto->setDistances(0, 8 * 64));
	TEST_ASSERT(pAuto->cellSize() == SizeI(int(std::sqrt(4.0 * 2 * 4)), int(std::sqrt(4.0 * 2 * 4))));

	// Equal distances have no band to map the source onto, the nodes are clamped far outside it.

	auto pFlat = RoundedRectWarp::create(WGBP(RoundedRectWarp, _.size = { 40, 40 }, _.sourceSize = c_sourceSize, _.rect = { 640, 640, 1280, 1280 }, _.cellSize = { 10, 10 }), pFactory);
	Grid flat(pFlat->generate());
	for (int i = 0; i < flat.nodes().w * flat.nodes().h; i++)
		TEST_ASSERT(flat.nodeData()[i].y == -Warpmap::maxValue);

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

	Grid grid(pMap);
	SizeI nodes = grid.nodes();
	std::vector<int32_t> ring((const int32_t*) grid.nodeData(), (const int32_t*) (grid.nodeData() + nodes.w * nodes.h));

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

	auto sameButOffset = [](Warpmap* pMovedMap, Warpmap* pBaseMap, spx offset, CoordF skip)
	{
		Grid base(pBaseMap), moved(pMovedMap);
		SizeI nodes = base.nodes();
		for (int row = 0; row < nodes.h; row++)
			for (int col = 0; col < nodes.w; col++)
			{
				if (base.nodeX(col) == skip.x && base.nodeY(row) == skip.y)
					continue;

				CoordSPX a = base.nodeData()[row * nodes.w + col];
				CoordSPX b = moved.nodeData()[row * nodes.w + col];
				if (uDiff(b.x, a.x + offset, pBaseMap->uPeriod()) > 1 || std::abs(b.y - a.y) > 1)
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
// Cells of generators can have another width than height, and generators can calculate one and
// have the other set.

bool WarpmapTest::nonSquareCellsTest(std::ostream& output)
{
	// One set, the other calculated, and nodes at the same places whatever the cells.

	auto pFactory = SoftWarpmapFactory::create();
	auto bp = WGBP(CircularWarp, _.sourceSize = c_sourceSize, _.size = { 96, 96 }, _.topRadius = 46 * 64, _.bottomRadius = 20 * 64, _.cellSize = { 6, 6 });
	auto pSquare = CircularWarp::create(bp, pFactory)->generate();

	bp.cellSize = { 12, 0 };
	auto pWarp = CircularWarp::create(bp, pFactory);
	int calculated = int(std::sqrt(4.0 * 0.25 * 20));
	TEST_ASSERT(pWarp->cellSize() == SizeI(12, calculated));

	TEST_ASSERT(pWarp->setCellSize({ 12, 6 }));
	Grid wide(pWarp->generate()), square(pSquare);
	TEST_ASSERT(wide.cellSize() == SizeI(12, 6));
	TEST_ASSERT(wide.nodes() == SizeI(9, 17));

	for (int row = 0; row < wide.nodes().h; row++)
		for (int col = 0; col < wide.nodes().w; col++)
			TEST_ASSERT(wide.nodeData()[row * wide.nodes().w + col] == square.nodeData()[row * square.nodes().w + col * 2]);

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
		int across = std::clamp(int(std::sqrt(4.0 * bp.precision * g.radius * std::cos(rim) * std::cos(rim) / std::sin(rim))), 4, 64);
		Grid grid(pMap);
		TEST_ASSERT(grid.cellSize() == (bX ? SizeI(bp.size.w, across) : SizeI(across, bp.size.h)));
		TEST_ASSERT(grid.nodes() == (bX ? SizeI(2, grid.nodes().h) : SizeI(grid.nodes().w, 2)));

		// Exact at nodes.

		SizeI nodes = grid.nodes();
		for (int row = 0; row < nodes.h; row++)
			for (int col = 0; col < nodes.w; col++)
			{
				CoordF exact = exactCylinder(bp, grid.nodeX(col), grid.nodeY(row));
				CoordSPX node = grid.nodeData()[row * nodes.w + col];
				TEST_ASSERT(uDiff(node.x, exact.x, pMap->uPeriod()) <= 0.51 && uDiff(node.y, exact.y, pMap->vPeriod()) <= 0.51);
			}

		// Within precision between nodes, as positions on screen: an error in angle of da (radians)
		// is R * cos(angle) * da pixels across.

		double maxError = 0.0;
		for (double a = 0.5; a < g.lengthAcross; a += 1.0)
			for (double l : { 0.5, g.lengthAlong / 2 })
			{
				double x = bX ? l : a, y = bX ? a : l;
				CoordF pos = grid.positionAt(x, y);
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
		Grid turned(CylinderWarp::create(turnedBP, pFactory)->generate());
		for (int i = 0; i < nodes.w * nodes.h; i++)
		{
			CoordSPX a = grid.nodeData()[i], b = turned.nodeData()[i];
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
					worstErrors = { onScreen, inSource };
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
