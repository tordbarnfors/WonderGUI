/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#include <wax_warpgenerator.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace wax
{
	const TypeInfo WarpGenerator::TYPEINFO = { "WarpGenerator", &Object::TYPEINFO };

	static const int c_minCellSize = 4;

	//____ constructor ________________________________________________________

	WarpGenerator::WarpGenerator(SizeI size, SizeSPX sourceSize, SizeI cellSize, float precision, WarpmapFactory* pFactory)
	{
		m_size		= size;
		m_sourceSize = sourceSize;
		m_cellSize	= cellSize;
		m_precision	= precision;
		m_pFactory	= pFactory;
	}

	//____ destructor _________________________________________________________

	WarpGenerator::~WarpGenerator()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& WarpGenerator::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setSize() __________________________________________________________

	bool WarpGenerator::setSize(SizeI size)
	{
		if (size.w <= 0 || size.h <= 0)
			return false;

		if (size != m_size)
		{
			m_size = size;
			m_bDirty = true;
		}
		return true;
	}

	//____ setSourceSize() ____________________________________________________

	bool WarpGenerator::setSourceSize(SizeSPX sourceSize)
	{
		if (sourceSize.w <= 0 || sourceSize.h <= 0)
			return false;

		if (sourceSize != m_sourceSize)
		{
			m_sourceSize = sourceSize;
			m_bDirty = true;
		}
		return true;
	}

	//____ setCellSize() ______________________________________________________

	bool WarpGenerator::setCellSize(SizeI cellSize)
	{
		if (cellSize.w < 0 || cellSize.h < 0)
			return false;

		if (cellSize != m_cellSize)
		{
			m_cellSize = cellSize;
			m_bDirty = true;
		}
		return true;
	}

	//____ cellSize() _________________________________________________________

	SizeI WarpGenerator::cellSize() const
	{
		if (m_cellSize.w > 0 && m_cellSize.h > 0)
			return m_cellSize;

		SizeI calculated = _calcCellSize();

		return { m_cellSize.w > 0 ? m_cellSize.w : std::clamp(calculated.w, c_minCellSize, std::max(c_minCellSize, m_size.w)),
				 m_cellSize.h > 0 ? m_cellSize.h : std::clamp(calculated.h, c_minCellSize, std::max(c_minCellSize, m_size.h)) };
	}

	//____ setPrecision() _____________________________________________________

	bool WarpGenerator::setPrecision(float precision)
	{
		if (precision <= 0.f)
			return false;

		if (precision != m_precision)
		{
			m_precision = precision;
			m_bDirty = true;
		}
		return true;
	}

	//____ generate() _________________________________________________________
	/*
		Draws are rendered when the canvas update ends and read the Warpmap then, so a Warpmap that
		a draw holds (or anyone else) is left as it is and a new one created in its place.

		A Warpmap that only we hold has its vertices, triangles and periods written in place and
		sent on once. The counts of vertices and triangles are fixed for a Warpmap, so it is
		created anew when they change.
	*/

	Warpmap_p WarpGenerator::generate()
	{
		if (!m_bDirty && m_pWarpmap)
			return m_pWarpmap;

		spx uPeriod = std::clamp(_uPeriod(), 0, Warpmap::maxValue);
		spx vPeriod = std::clamp(_vPeriod(), 0, Warpmap::maxValue);

		int nVertices, nTriangles;
		_meshSize(nVertices, nTriangles);

		auto generateInto = [&](Warpmap::Vertex* pVertices, Warpmap::Triangle* pTriangles)
		{
			_generateMesh(pVertices, pTriangles);
			for (int i = 0; i < nVertices; i++)
				pVertices[i] = Warpmap::_clamped(pVertices[i]);
		};

		Warpmap* pMap = m_pWarpmap;

		if (!pMap || pMap->refcount() > 1 || pMap->pixelSize() != m_size || pMap->vertexCount() != nVertices || pMap->triangleCount() != nTriangles)
		{
			Warpmap::Blueprint bp;
			bp.size = m_size;
			bp.triangles = nTriangles;
			bp.uPeriod = uPeriod;
			bp.vertices = nVertices;
			bp.vPeriod = vPeriod;

			std::vector<Warpmap::Vertex> vertices(nVertices);
			std::vector<Warpmap::Triangle> triangles(nTriangles);
			generateInto(vertices.data(), triangles.data());

			m_pWarpmap = m_pFactory->createWarpmap(bp, vertices.data(), triangles.data());
		}
		else if (pMap->m_pVertices && pMap->m_pTriangles)
		{
			generateInto(pMap->m_pVertices, pMap->m_pTriangles);
			pMap->m_uPeriod = uPeriod;
			pMap->m_vPeriod = vPeriod;
			pMap->_updated(0, nVertices, 0, nTriangles);
		}
		else
		{
			// No content of its own to write to, it goes through importMesh().

			std::vector<Warpmap::Vertex> vertices(nVertices);
			std::vector<Warpmap::Triangle> triangles(nTriangles);
			generateInto(vertices.data(), triangles.data());

			pMap->importMesh(vertices.data(), triangles.data(), uPeriod, vPeriod);
		}

		_generated();

		m_bDirty = false;
		return m_pWarpmap;
	}

	//____ _calcCellSize() ____________________________________________________

	SizeI WarpGenerator::_calcCellSize() const
	{
		return m_size;
	}

	//____ Outline bands ______________________________________________________
	/*
		The band covers the whole outline, whatever the sweep, since offsets can move the source
		anywhere along it, from a pixel inside its inner edge to a pixel outside its outer edge, for
		borders antialiased within its triangles. It is made of columns across the band, from the
		outline outwards along its normal, walked clockwise from the start of the top side: one at
		each end of the straight sides and nArc + 1 around each corner, the last one in the same place
		as the first. Each column has nAcross + 1 vertices, at the same distances from the outline.

		Along the straight sides positions are linear, so their quads are exact. Around the corners,
		triangles are off where the outline curves, by r * a^2 / 8 for sectors of a radians at radius
		r (largest at the outer edge), and along their diagonals, by a * h / 4 for parts h across the
		band, from how the position along the outline changes across it. _bandLayout() shares
		precision between the two as gives the fewest triangles.

		Where the band reaches further inside than the radius of the corners, the inside of the outline
		is split along the diagonals from the centers of the corners, as the nearest point of the outline
		is on one side or another, and positions are linear within each part. Those parts are added as
		polygons, clipped to the band. Columns of corners then start in the center of the corner, and
		vertices there take the position along the outline of the middle of their sector, which is all
		that is drawn of them.
	*/

	namespace
	{
		const double c_pi = 3.14159265358979323846;
		const int64_t c_bandPixel = 64 << 8;					// A pixel in 1/256 spx.

		// Distances from the outline: the inner and outer edge of the mesh, a pixel beyond the band
		// but not beyond the deepest point inside, and where columns start.

		void bandDistances(int64_t top, int64_t bottom, int64_t ax, int64_t ay, int64_t radius, int64_t& dIn, int64_t& dColumns, int64_t& dOut)
		{
			dOut = std::max(top, bottom) + c_bandPixel;
			dIn = std::max(std::min(top, bottom) - c_bandPixel, -(radius + std::min(ax, ay)));
			dColumns = std::max(dIn, -radius);
		}
	}

	//____ _bandLayout() ______________________________________________________

	void WarpGenerator::_bandLayout(const OutlineBand& band, int& nArc, int& nAcross) const
	{
		int64_t dIn, dColumns, dOut;
		bandDistances(band.top, band.bottom, band.ax, band.ay, band.radius, dIn, dColumns, dOut);

		double outer = double(band.radius + dOut) / c_bandPixel;
		double across = double(dOut - dColumns) / c_bandPixel;

		nArc = 0;
		nAcross = 0;

		if (across <= 0.0 || outer <= 0.0)
			return;

		const double quarter = c_pi / 2;
		const double precision = std::max(double(m_precision), 0.01);

		// Kept within Warpmap::maxVertices: columns at the start of the top side and around the
		// four corners, nAcross + 1 vertices each, the vertices at the centers of fans and up to six
		// for each of the four parts inside the centers of the corners. Coarser across first, as long
		// as pieces stay longer across than along.

		auto cap = [&]()
		{
			auto vertices = [](int n, int m) { return int64_t(4 * n + 5) * (m + 1) + 4 * n + 24; };

			while (vertices(nArc, nAcross) > Warpmap::maxVertices && (nArc > 2 || nAcross > 1))
			{
				if (nAcross > 1 && (nArc <= 2 || across / nAcross < outer * quarter / nArc))
					nAcross = std::max(1, int(nAcross * 7 / 8));
				else
					nArc = std::max(2, int(nArc * 7 / 8));
			}
		};

		auto arcs = [&](double angle) { return int(std::clamp(std::ceil(quarter / std::max(angle, 1e-6)), 2.0, 1024.0)); };
		auto parts = [&](double angle, double budget) { return int(std::clamp(std::ceil(across * angle / (4 * budget)), 1.0, 256.0)); };

		if (m_cellSize.w > 0 && m_cellSize.h > 0)
		{
			nArc = arcs(m_cellSize.w / outer);
			nAcross = int(std::clamp(std::ceil(across / m_cellSize.h), 1.0, 256.0));
			return cap();
		}

		if (m_cellSize.w > 0)
		{
			nArc = arcs(m_cellSize.w / outer);
			double angle = quarter / nArc;
			nAcross = parts(angle, std::max(precision - outer * angle * angle / 8, precision / 4));
			return cap();
		}

		// Least work drawing for a share of precision kept along the outline. A software backend
		// spends about as much on each row a triangle covers, so that goes by the number of quads
		// times their length plus how far across they go, which favours quads about as long as
		// they are across over thin slivers.

		double best = -1.0;
		for (int i = 1; i < 20; i++)
		{
			double share = i / 20.0;
			int n = arcs(std::sqrt(8 * share * precision / outer));
			double angle = quarter / n;

			int m = m_cellSize.h > 0 ? int(std::clamp(std::ceil(across / m_cellSize.h), 1.0, 256.0))
									 : parts(angle, std::max(precision - outer * angle * angle / 8, precision / 20));

			double cost = double(n + 1) * m * (outer * angle + across / m);
			if (best < 0.0 || cost < best)
			{
				best = cost;
				nArc = n;
				nAcross = m;
			}
		}

		cap();
	}

	//____ _bandCellSize() ____________________________________________________

	SizeI WarpGenerator::_bandCellSize(const OutlineBand& band) const
	{
		int nArc, nAcross;
		_bandLayout(band, nArc, nAcross);

		int64_t dIn, dColumns, dOut;
		bandDistances(band.top, band.bottom, band.ax, band.ay, band.radius, dIn, dColumns, dOut);

		if (nArc == 0)
			return m_size;

		double outer = double(band.radius + dOut) / c_bandPixel;
		double across = double(dOut - dColumns) / c_bandPixel;

		return { std::max(1, int(outer * c_pi / 2 / nArc + 0.5)), std::max(1, int(across / nAcross + 0.5)) };
	}

	//____ _bandMesh() ________________________________________________________

	void WarpGenerator::_bandMesh(const OutlineBand& band, std::vector<Warpmap::Vertex>& vertices, std::vector<Warpmap::Triangle>& triangles) const
	{
		vertices.clear();
		triangles.clear();

		const int64_t ax = std::max(band.ax, int64_t(0));
		const int64_t ay = std::max(band.ay, int64_t(0));
		const int64_t r = std::max(band.radius, int64_t(0));

		if (band.top == band.bottom)
		{
			// No band to map the source onto, nothing to draw.

			vertices.assign(3, { 0, 0, 0, 0 });
			triangles.push_back({ 0, 0, 0 });
			return;
		}

		int64_t dIn, dColumns, dOut;
		bandDistances(band.top, band.bottom, ax, ay, r, dIn, dColumns, dOut);

		int nArc, nAcross;
		_bandLayout(band, nArc, nAcross);

		// Along the outline. pos is in 1/256 spx, from the middle of the top side.

		const int64_t arc = _multiply(r, _factor(c_pi / 2));
		int64_t perimeter = 4 * ax + 4 * ay + 4 * arc;
		if (perimeter <= 0)
			perimeter = 1;

		const bool bClockwise = band.sweep > 0.0;
		const double sweep = std::abs(band.sweep);
		const int64_t pos0 = -ax;

		uint32_t part0 = uint32_t(_multiply(pos0, _factor(0x1p32 / double(perimeter))));
		const int64_t u0 = _multiply(bClockwise ? part0 - band.start : band.start - part0, _factor(double(m_sourceSize.w) / (sweep * 0x1p32)));
		const Factor uFactor = _factor(double(m_sourceSize.w) / (sweep * double(perimeter)));
		const Factor vFactor = _factor(double(m_sourceSize.h) / double(band.bottom - band.top));

		auto uAt = [&](int64_t pos) { int64_t du = _multiply(pos - pos0, uFactor); return u0 + (bClockwise ? du : -du); };
		auto vAt = [&](int64_t distance) { return _multiply(distance - band.top, vFactor); };

		auto toSpx = [](int64_t value) { return spx(std::clamp((value + 128) >> 8, -int64_t(Warpmap::maxPosition), int64_t(Warpmap::maxPosition))); };

		auto addVertex = [&](int64_t x, int64_t y, int64_t u, int64_t v)
		{
			vertices.push_back({ toSpx(band.cx + x), toSpx(band.cy + y), _toSourceValue(u), _toSourceValue(v) });
			return int(vertices.size()) - 1;
		};

		// Columns across the band.

		if (dOut > dColumns)
		{
			struct Column
			{
				int64_t		x, y;			// Point of the outline.
				int32_t		nx, ny;			// Normal, outwards, with 20 binals.
				int64_t		pos;
				int			corner;			// Corner it is on, -1 for the start of the top side.
			};

			std::vector<Column> columns;

			if (ax > 0)
				columns.push_back({ -ax, -ay - r, 0, -(1 << 20), pos0, -1 });

			const int64_t cornerX[4] = { ax, ax, -ax, -ax };
			const int64_t cornerY[4] = { -ay, ay, ay, -ay };
			const int64_t cornerPos[4] = { ax, ax + arc + 2 * ay, 3 * ax + 2 * arc + 2 * ay, 3 * ax + 3 * arc + 4 * ay };

			for (int k = 0; k < 4; k++)
			{
				for (int i = 0; i <= nArc; i++)
				{
					int64_t pos = cornerPos[k] + arc * i / nArc;

					if (i == 0 && !columns.empty() && columns.back().pos == pos)
						continue;					// The side before has no length.

					int32_t sin, cos;
					_sinCos((uint32_t(k) << 30) + uint32_t((uint64_t(1) << 30) * i / nArc), sin, cos);

					Column c;
					c.nx = sin >> 10;
					c.ny = -(cos >> 10);
					c.x = cornerX[k] + ((r * c.nx) >> 20);
					c.y = cornerY[k] + ((r * c.ny) >> 20);
					c.pos = pos;
					c.corner = k;
					columns.push_back(c);
				}
			}

			// Vertices, nAcross + 1 for each column.

			const int nColumns = int(columns.size());
			const int64_t span = dOut - dColumns;
			const bool bApex = dColumns == -r;

			for (int j = 0; j < nColumns; j++)
			{
				const Column& c = columns[j];
				int64_t u = uAt(c.pos);

				for (int i = 0; i <= nAcross; i++)
				{
					int64_t d = dColumns + span * i / nAcross;
					addVertex(c.x + ((d * c.nx) >> 20), c.y + ((d * c.ny) >> 20), u, vAt(d));
				}
			}

			// Sectors of corners that start in their center are fans with a vertex of their own
			// there, which takes the position along the outline of the middle of the sector.

			for (int j = 0; j + 1 < nColumns; j++)
			{
				bool bFan = bApex && columns[j].corner >= 0 && columns[j].corner == columns[j + 1].corner;

				for (int i = 0; i < nAcross; i++)
				{
					int a = j * (nAcross + 1) + i;
					int b = a + nAcross + 1;

					if (bFan && i == 0)
					{
						const Warpmap::Vertex& center = vertices[a];
						Warpmap::Vertex apex = center;
						apex.u = _toSourceValue((int64_t(vertices[a].u) + vertices[b].u) / 2);
						vertices.push_back(apex);

						triangles.push_back({ int(vertices.size()) - 1, b + 1, a + 1 });
						continue;
					}

					triangles.push_back({ a, b, b + 1 });
					triangles.push_back({ a, b + 1, a + 1 });
				}
			}
		}

		// Parts inside the centers of the corners, where the band reaches further in than their
		// radius. Each is the part of the inside nearest to one side, cut along the diagonals from the
		// centers of the corners, clipped to the band. Their edges are along the axes or diagonals,
		// so the points where they are clipped are exact.

		if (dIn < -r)
		{
			const int64_t m = std::min(ax, ay);
			const int64_t hi = std::min(dOut, -r);

			struct Point { int64_t x, y; };

			struct Part
			{
				Point	corners[4];
				int		axis;				// Of the line depth is measured along: 0 = x, 1 = y.
				int64_t	edge;				// Where along it the depth is -r.
				int		sign;				// Depth decreasing (going inwards) along the axis: +1 or -1.
			};

			const Part sides[4] = {
				{ { { -ax, -ay }, { ax, -ay }, { ax - m, -ay + m }, { -ax + m, -ay + m } }, 1, -ay, 1 },		// Top, deeper as y grows.
				{ { { ax, -ay }, { ax, ay }, { ax - m, ay - m }, { ax - m, -ay + m } }, 0, ax, -1 },			// Right.
				{ { { ax, ay }, { -ax, ay }, { -ax + m, ay - m }, { ax - m, ay - m } }, 1, ay, -1 },			// Bottom.
				{ { { -ax, ay }, { -ax, -ay }, { -ax + m, -ay + m }, { -ax + m, ay - m } }, 0, -ax, 1 }		// Left.
			};

			for (int k = 0; k < 4; k++)
			{
				const Part& part = sides[k];

				// Depth at a point, and the position along the outline.

				auto depth = [&](const Point& p) { return -r - (part.axis == 1 ? p.y - part.edge : p.x - part.edge) * part.sign; };
				auto pos = [&](const Point& p)
				{
					switch (k)
					{
						case 0:	return p.x;
						case 1:	return ax + arc + (p.y + ay);
						case 2:	return ax + 2 * arc + 2 * ay + (ax - p.x);
						default: return 3 * ax + 3 * arc + 2 * ay + (ay - p.y);
					}
				};

				// Clipped to depth >= dIn and <= hi, which are lines across the axis.

				std::vector<Point> poly(part.corners, part.corners + 4);

				auto clip = [&](int64_t limit, bool bKeepDeeper)
				{
					std::vector<Point> out;
					int n = int(poly.size());
					for (int i = 0; i < n; i++)
					{
						const Point& a = poly[i];
						const Point& b = poly[(i + 1) % n];
						int64_t da = depth(a) - limit, db = depth(b) - limit;
						bool bInA = bKeepDeeper ? da <= 0 : da >= 0;
						bool bInB = bKeepDeeper ? db <= 0 : db >= 0;

						if (bInA)
							out.push_back(a);

						if (bInA != bInB)
						{
							// Along the axis depth is measured on, edges change by the same or nothing across it.

							Point q;
							int64_t at = part.edge - (limit + r) * part.sign;			// Where depth is limit.
							if (part.axis == 1)
							{
								int64_t dx = b.x - a.x, dy = b.y - a.y;
								q.y = at;
								q.x = dy != 0 ? a.x + (at - a.y) * (dx / dy) : a.x;
							}
							else
							{
								int64_t dx = b.x - a.x, dy = b.y - a.y;
								q.x = at;
								q.y = dx != 0 ? a.y + (at - a.x) * (dy / dx) : a.y;
							}
							out.push_back(q);
						}
					}
					poly = out;
				};

				clip(dIn, false);
				clip(hi, true);

				// Without corners in the same place, a fan from the first.

				std::vector<Point> unique;
				for (const Point& p : poly)
				{
					if (unique.empty() || unique.back().x != p.x || unique.back().y != p.y)
						unique.push_back(p);
				}
				while (unique.size() > 1 && unique.front().x == unique.back().x && unique.front().y == unique.back().y)
					unique.pop_back();

				if (unique.size() < 3)
					continue;

				int first = int(vertices.size());
				for (const Point& p : unique)
					addVertex(p.x, p.y, uAt(pos(p)), vAt(depth(p)));

				for (int i = 1; i + 1 < int(unique.size()); i++)
					triangles.push_back({ first, first + i, first + i + 1 });
			}
		}

		if (triangles.empty())
		{
			vertices.assign(3, { 0, 0, 0, 0 });
			triangles.push_back({ 0, 0, 0 });
		}
	}

	//____ _validateSettings() ________________________________________________

	bool WarpGenerator::_validateSettings(SizeI size, SizeSPX sourceSize, SizeI cellSize, float precision)
	{
		return size.w > 0 && size.h > 0 && sourceSize.w > 0 && sourceSize.h > 0 && cellSize.w >= 0 && cellSize.h >= 0 && precision > 0.f;
	}

	//____ _factor() __________________________________________________________

	WarpGenerator::Factor WarpGenerator::_factor(double factor)
	{
		Factor f;

		if (!(factor == factor) || factor == 0.0)
			return f;

		f.bNegative = factor < 0.0;
		factor = std::abs(factor);

		if (factor > 0x1p62)
		{
			f.mul = 0x80000000;
			f.shift = -31;										// 2^62, which is what results are clamped to anyway.
			return f;
		}

		int exp;
		double mantissa = std::frexp(factor, &exp);				// 0.5 -> 1.
		uint64_t mul = uint64_t(std::ldexp(mantissa, 32) + 0.5);
		if (mul >> 32)
		{
			mul >>= 1;
			exp++;
		}

		if (32 - exp > 127)
			return Factor();									// Rounds to 0 for any value.

		f.mul = uint32_t(mul);
		f.shift = 32 - exp;
		return f;
	}

	//____ _multiply() ________________________________________________________

	int64_t WarpGenerator::_multiply(int64_t value, Factor factor)
	{
		const uint64_t c_max = uint64_t(1) << 62;

		bool bNegative = (value < 0) != factor.bNegative;
		uint64_t a = value < 0 ? 0 - uint64_t(value) : uint64_t(value);
		int shift = factor.shift;

		while (a >> 39)											// Keeps the product within 63 bits.
		{
			a >>= 8;
			shift -= 8;
		}
		while (a >> 31)
		{
			a >>= 1;
			shift--;
		}

		uint64_t product = uint64_t(uint32_t(a)) * factor.mul;
		uint64_t result;

		if (shift > 0)
			result = shift < 64 ? (product + (uint64_t(1) << (shift - 1))) >> shift : 0;
		else if (product == 0 || shift == 0)
			result = product;
		else
			result = (-shift >= 62 || (product >> (62 + shift)) != 0) ? c_max : product << -shift;

		result = std::min(result, c_max);
		return bNegative ? -int64_t(result) : int64_t(result);
	}

	//____ _polar() ___________________________________________________________
	/*
		CORDIC in vectoring mode: x,y is rotated towards the x axis by angles of atan(2^-i), adding
		them up, which leaves the length times a fixed gain in x. Values are scaled to 29 bits first
		so all of it fits in 32 bit integers.
	*/

	uint32_t WarpGenerator::_polar(int64_t x, int64_t y, int64_t* pLength)
	{
		static const int32_t c_atan[] = {	536870912, 316933406, 167458907, 85004756, 42667331, 21354465, 10679838, 5340245,
											2670163, 1335087, 667544, 333772, 166886, 83443, 41722, 20861, 10430, 5215, 2608,
											1304, 652, 326, 163, 81, 41, 20, 10, 5, 3, 1 };	// atan(2^-i) in 1/2^32 of a turn.

		const uint64_t c_invGain = 2608131496;					// 2^32 / gain of the rotations.

		uint32_t angle = 0;
		if (x < 0)
		{
			x = -x;
			y = -y;
			angle = 0x80000000;									// Half a turn, the rest is within +-90 degrees.
		}

		uint64_t ax = uint64_t(x);
		uint64_t ay = y < 0 ? 0 - uint64_t(y) : uint64_t(y);
		uint64_t max = std::max(ax, ay);

		if (max == 0)
		{
			if (pLength)
				*pLength = 0;
			return 0;
		}

		int shift = 0;											// Right shift that brings max to 2^28 -> 2^29.
		while (max < (uint64_t(1) << 20))
		{
			max <<= 8;
			shift -= 8;
		}
		while (max < (uint64_t(1) << 28))
		{
			max <<= 1;
			shift--;
		}
		while (max >= (uint64_t(1) << 37))
		{
			max >>= 8;
			shift += 8;
		}
		while (max >= (uint64_t(1) << 29))
		{
			max >>= 1;
			shift++;
		}

		int32_t cx = int32_t(shift >= 0 ? ax >> shift : ax << -shift);
		int32_t cy = int32_t(shift >= 0 ? ay >> shift : ay << -shift);
		if (y < 0)
			cy = -cy;

		int32_t z = 0;
		for (int i = 0; i < 30; i++)
		{
			int32_t sign = cy >> 31;							// Rotate towards the x axis: -1 below it, 0 above.
			int32_t dx = ((cy >> i) ^ sign) - sign;				// Value negated if sign is -1.
			int32_t dy = ((cx >> i) ^ sign) - sign;

			cx += dx;
			cy -= dy;
			z += (c_atan[i] ^ sign) - sign;
		}

		if (pLength)
		{
			uint64_t length = uint64_t(cx) * c_invGain;			// Length with 32 fraction bits, scaled by shift.
			int down = 32 - shift;

			if (down > 0)
				*pLength = int64_t((length + (uint64_t(1) << (down - 1))) >> down);
			else if ((length >> (63 + down)) == 0)
				*pLength = int64_t(length << -down);
			else
				*pLength = INT64_MAX;							// Beyond int64, only for x or y close to its limits.
		}

		return angle + uint32_t(z);
	}

	//____ _offsetFactor() ____________________________________________________
	/*
		A position along the axis is (position - start) / sweep * length, so moving start by shift
		moves it by -shift / sweep * length. With shift in 1/2^32 of a turn that is shift times
		length / (sweep * 2^32), sweep in turns.
	*/

	WarpGenerator::Factor WarpGenerator::_offsetFactor(double sweepTurns, spx length)
	{
		return _factor(double(length) / (sweepTurns * 0x1p32));
	}

	//____ _offsetFor() _______________________________________________________
	/*
		The shift is taken within half a turn either way, so the offset stays small. Whole turns
		move positions by whole periods, which the renderer wraps away.
	*/

	spx WarpGenerator::_offsetFor(uint32_t shift, Factor factor)
	{
		return _toSourceValue(-_multiply(int32_t(shift), factor));
	}

	//____ _sqrt() ____________________________________________________________

	uint64_t WarpGenerator::_sqrt(uint64_t value)
	{
		uint64_t result = 0;
		uint64_t bit = uint64_t(1) << 62;

		while (bit > value)
			bit >>= 2;

		while (bit != 0)
		{
			if (value >= result + bit)
			{
				value -= result + bit;
				result = (result >> 1) + bit;
			}
			else
				result >>= 1;

			bit >>= 2;
		}
		return result;
	}

	//____ _toTurn() __________________________________________________________
	/*
		turns * 2^32 as an integer, of which the low 32 bits are the fraction of a turn, without
		floor() and the like.
	*/

	uint32_t WarpGenerator::_toTurn(double turns)
	{
		if (!(turns > -0x1p30 && turns < 0x1p30))				// Also NaN.
			return 0;

		return uint32_t(uint64_t(int64_t(turns * 0x1p32)));
	}

	//____ _sinCos() __________________________________________________________
	/*
		CORDIC in rotation mode: a vector of length 1 (prescaled by the gain of the rotations) is
		rotated by angles of atan(2^-i) until the angle is used up. The angle is first brought
		within a quarter turn either way, the rest is half a turn and negates both.
	*/

	void WarpGenerator::_sinCos(uint32_t angle, int32_t& sin, int32_t& cos)
	{
		static const int32_t c_atan[] = {	536870912, 316933406, 167458907, 85004756, 42667331, 21354465, 10679838, 5340245,
											2670163, 1335087, 667544, 333772, 166886, 83443, 41722, 20861, 10430, 5215, 2608,
											1304, 652, 326, 163, 81, 41, 20, 10, 5, 3, 1 };	// atan(2^-i) in 1/2^32 of a turn.

		int32_t z = int32_t(angle);
		bool bHalfTurn = false;

		if (z > 0x40000000 || z < -0x40000000)
		{
			z = int32_t(angle + 0x80000000u);
			bHalfTurn = true;
		}

		int32_t x = 652032874;									// 2^30 / gain of the rotations.
		int32_t y = 0;

		for (int i = 0; i < 30; i++)
		{
			int32_t sign = z >> 31;								// Rotate towards the angle left: -1 if negative.
			int32_t dx = ((y >> i) ^ sign) - sign;
			int32_t dy = ((x >> i) ^ sign) - sign;

			x -= dx;
			y += dy;
			z -= (c_atan[i] ^ sign) - sign;
		}

		cos = bHalfTurn ? -x : x;
		sin = bHalfTurn ? -y : y;
	}

	//____ _toSourceValue() ___________________________________________________

	spx WarpGenerator::_toSourceValue(int64_t value)
	{
		return spx(std::clamp(value, -int64_t(Warpmap::maxValue), int64_t(Warpmap::maxValue)));
	}

} // namespace wax
