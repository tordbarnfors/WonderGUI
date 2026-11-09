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
#include <wg_softwarpmap.h>

#include <algorithm>
#include <cstdint>
#include <limits>

namespace wg
{
	const TypeInfo SoftWarpmap::TYPEINFO = { "SoftWarpmap", &Warpmap::TYPEINFO };

	namespace
	{
		inline int64_t floorDiv(int64_t a, int64_t b)
		{
			int64_t q = a / b;
			return (q * b > a) ? q - 1 : q;
		}

		inline int64_t ceilDiv(int64_t a, int64_t b)
		{
			return -floorDiv(-a, b);
		}

		inline int64_t abs64(int64_t v) { return v < 0 ? -v : v; }

		//____ mulMulDivWide() ________________________________________________
		/*
			(a * b + c * d) / e rounded to nearest, e > 0, with the sum in 128 bits and the result
			clamped to int64. The portable version is a plain long division, which mulMulDiv() only
			needs for values too large for 64 bits.
		*/

#if defined(__SIZEOF_INT128__)

		int64_t mulMulDivWide(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e)
		{
			__int128 n = __int128(a) * b + __int128(c) * d;
			n += n < 0 ? -(e / 2) : e / 2;
			__int128 q = n / e;

			if (q > std::numeric_limits<int64_t>::max())
				return std::numeric_limits<int64_t>::max();
			if (q < std::numeric_limits<int64_t>::min())
				return std::numeric_limits<int64_t>::min();
			return int64_t(q);
		}

#else

		struct U128
		{
			uint64_t	hi = 0, lo = 0;
		};

		U128 mulU(uint64_t a, uint64_t b)
		{
			uint64_t a0 = a & 0xFFFFFFFF, a1 = a >> 32, b0 = b & 0xFFFFFFFF, b1 = b >> 32;

			uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
			uint64_t mid = (p00 >> 32) + (p01 & 0xFFFFFFFF) + (p10 & 0xFFFFFFFF);

			U128 r;
			r.lo = (p00 & 0xFFFFFFFF) | (mid << 32);
			r.hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
			return r;
		}

		U128 negate(U128 v)
		{
			v.hi = ~v.hi;
			v.lo = ~v.lo;
			v.lo++;
			if (v.lo == 0)
				v.hi++;
			return v;
		}

		U128 mulS(int64_t a, int64_t b)			// Two's complement.
		{
			bool bNegative = (a < 0) != (b < 0);
			U128 r = mulU(a < 0 ? uint64_t(0) - uint64_t(a) : uint64_t(a), b < 0 ? uint64_t(0) - uint64_t(b) : uint64_t(b));
			return bNegative ? negate(r) : r;
		}

		U128 add(U128 a, U128 b)
		{
			U128 r;
			r.lo = a.lo + b.lo;
			r.hi = a.hi + b.hi + (r.lo < a.lo ? 1 : 0);
			return r;
		}

		int64_t mulMulDivWide(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e)
		{
			U128 n = add(mulS(a, b), mulS(c, d));

			bool bNegative = (n.hi >> 63) != 0;
			if (bNegative)
				n = negate(n);

			n = add(n, { 0, uint64_t(e) / 2 });

			uint64_t divisor = uint64_t(e);
			if (n.hi >= divisor)
				return bNegative ? std::numeric_limits<int64_t>::min() : std::numeric_limits<int64_t>::max();

			uint64_t rem = n.hi, q = 0;
			for (int i = 63; i >= 0; i--)
			{
				bool bCarry = (rem >> 63) != 0;
				rem = (rem << 1) | ((n.lo >> i) & 1);
				if (bCarry || rem >= divisor)
				{
					rem -= divisor;
					q |= uint64_t(1) << i;
				}
			}

			if (q > uint64_t(std::numeric_limits<int64_t>::max()))
				return bNegative ? std::numeric_limits<int64_t>::min() : std::numeric_limits<int64_t>::max();

			return bNegative ? -int64_t(q) : int64_t(q);
		}

#endif

		//____ mulMulDiv() ____________________________________________________
		/*
			As mulMulDivWide(). Without 128 bit integers, in 64 bits where the products fit in 61 bits
			each, which they do for the triangles of most meshes, rounded the same way so that it
			gives the same results.
		*/

#if defined(__SIZEOF_INT128__)

		inline int64_t mulMulDiv(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e)
		{
			return mulMulDivWide(a, b, c, d, e);
		}

#else

		inline int bitLength(int64_t value)
		{
			uint64_t v = value < 0 ? 0 - uint64_t(value) : uint64_t(value);
#	if defined(__GNUC__)
			return v == 0 ? 0 : 64 - __builtin_clzll(v);
#	else
			int n = 0;
			if (v >> 32) { n += 32; v >>= 32; }
			if (v >> 16) { n += 16; v >>= 16; }
			if (v >> 8) { n += 8; v >>= 8; }
			if (v >> 4) { n += 4; v >>= 4; }
			if (v >> 2) { n += 2; v >>= 2; }
			if (v >> 1) { n += 1; v >>= 1; }
			return n + int(v);
#	endif
		}

		int64_t mulMulDiv(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e)
		{
			if (bitLength(a) + bitLength(b) > 61 || bitLength(c) + bitLength(d) > 61)
				return mulMulDivWide(a, b, c, d, e);

			int64_t n = a * b + c * d;
			n += n < 0 ? -(e / 2) : e / 2;
			return n / e;
		}

#endif
	}

	//____ create() ___________________________________________________________

	SoftWarpmap_p SoftWarpmap::create( const Blueprint& bp )
	{
		if (!_validateBlueprint(bp))
			return nullptr;

		return SoftWarpmap_p(new SoftWarpmap(bp));
	}

	SoftWarpmap_p SoftWarpmap::create( const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles )
	{
		if (!pVertices || !pTriangles || !_validateBlueprint(bp, pTriangles))
			return nullptr;

		return SoftWarpmap_p(new SoftWarpmap(bp, pVertices, pTriangles));
	}

	//____ constructor ________________________________________________________

	SoftWarpmap::SoftWarpmap( const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles ) : Warpmap(bp, pVertices, pTriangles)
	{
		_setupTriangles();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SoftWarpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _updated() _________________________________________________________
	// A vertex can be a corner of any triangle, so all are set up again unless only periods changed.

	void SoftWarpmap::_updated( int firstVertex, int nVertices, int firstTriangle, int nTriangles )
	{
		if (nVertices > 0 || nTriangles > 0)
			_setupTriangles();
	}

	//____ _setupTriangles() __________________________________________________
	/*
		Corners are put in the order that gives the triangle a positive area A, where the edge
		function of the edge from corner a to corner b, e = dx * (py - ay) - dy * (px - ax), is
		positive inside. A center on an edge is inside if it is a left edge (dy < 0) or a horizontal
		top edge (dx > 0), as Warpmap::isInside(). Edge i, from corner i + 1 to i + 2, is A at corner i,
		so edge[i] / A is the barycentric coordinate of corner i, which gives the values f at any
		point from those at the corners: f0 + ((f1 - f0) * e1 + (f2 - f0) * e2) / A. Their changes per
		pixel are exact up to rounding, with 128 bit products where 64 bits aren't enough.

		Along a row, the edge function is value + edgeX * x. Where edgeX > 0 the edge is a lower bound,
		inside (with ties, which a left edge has) from ceil(-value / edgeX), where edgeX < 0 an upper
		bound, inside (without ties) up to ceil(value / -edgeX). Each row adds edgeY to value, so the
		bound is stepped by the quotient and remainder of that by the divisor, exactly. An edge along a
		row (edgeX = 0) only decides which rows are inside, so it limits rowBeg and rowEnd.

		With vertex positions within +-2^23 spx, the divisor (64 * the height of the edge) is within
		2^30, as is the change of the column from one row to the next.

		Values are set up at the pixel containing the first corner, so that they stay small for the
		pixels of the triangle, which are the only ones they are evaluated at.

		With perspective, the weights are scaled so that the largest is 2^28, and W * (u - u0) is what
		is interpolated, u0 being u of the first corner. With 16 binals and u - u0 within +-2^31
		(32768 pixels of the source), which positions are clamped to, that is within 2^59.
	*/

	void SoftWarpmap::_setupTriangles()
	{
		m_setups.clear();
		m_setups.reserve(m_nTriangles);
		m_perspective.clear();
		m_fades.clear();

		const int64_t c_maxRelative = int64_t(1) << 31;

		for (int t = 0; t < m_nTriangles; t++)
		{
			const Triangle& tri = m_pTriangles[t];
			const Vertex* c[3] = { &m_pVertices[tri.a], &m_pVertices[tri.b], &m_pVertices[tri.c] };

			int64_t area = int64_t(c[1]->x - c[0]->x) * (c[2]->y - c[0]->y) - int64_t(c[1]->y - c[0]->y) * (c[2]->x - c[0]->x);
			if (area == 0)
				continue;

			if (area < 0)
			{
				std::swap(c[1], c[2]);
				area = -area;
			}

			Setup s;

			// Pixels with centers that can be inside.

			int64_t minX = std::min({ c[0]->x, c[1]->x, c[2]->x }), maxX = std::max({ c[0]->x, c[1]->x, c[2]->x });
			int64_t minY = std::min({ c[0]->y, c[1]->y, c[2]->y }), maxY = std::max({ c[0]->y, c[1]->y, c[2]->y });

			s.colBeg = int(std::clamp(ceilDiv(minX - 32, 64), int64_t(0), int64_t(m_size.w)));
			s.colEnd = int(std::clamp(floorDiv(maxX - 32, 64) + 1, int64_t(s.colBeg), int64_t(m_size.w)));
			s.rowBeg = int(std::clamp(ceilDiv(minY - 32, 64), int64_t(0), int64_t(m_size.h)));
			s.rowEnd = int(std::clamp(floorDiv(maxY - 32, 64) + 1, int64_t(s.rowBeg), int64_t(m_size.h)));

			// Edge functions, edge[i] + edgeX[i] * x + edgeY[i] * y at the center of pixel x,y.

			int64_t edge[3], edgeX[3], edgeY[3];
			bool tie[3];

			for (int i = 0; i < 3; i++)
			{
				const Vertex* a = c[(i + 1) % 3];
				const Vertex* b = c[(i + 2) % 3];

				int64_t dx = int64_t(b->x) - a->x;
				int64_t dy = int64_t(b->y) - a->y;

				edge[i] = dx * (32 - int64_t(a->y)) - dy * (32 - int64_t(a->x));
				edgeX[i] = -64 * dy;
				edgeY[i] = 64 * dx;
				tie[i] = dy < 0 || (dy == 0 && dx > 0);
			}

			// Rows that edges along a row allow: value + edgeY * row > 0, or 0 with a tie.

			for (int i = 0; i < 3; i++)
			{
				if (edgeX[i] != 0)
					continue;

				if (edgeY[i] > 0)
				{
					int64_t first = tie[i] ? ceilDiv(-edge[i], edgeY[i]) : floorDiv(-edge[i], edgeY[i]) + 1;
					s.rowBeg = int(std::clamp(first, int64_t(s.rowBeg), int64_t(s.rowEnd)));
				}
				else
				{
					int64_t last = tie[i] ? floorDiv(edge[i], -edgeY[i]) : ceilDiv(edge[i], -edgeY[i]) - 1;
					s.rowEnd = int(std::clamp(last + 1, int64_t(s.rowBeg), int64_t(s.rowEnd)));
				}
			}

			if (s.colBeg >= s.colEnd || s.rowBeg >= s.rowEnd)
				continue;

			// The other edges, at rowBeg, lower bounds first.

			s.nLower = 0;
			s.nUpper = 0;

			for (int kind = 0; kind < 2; kind++)
			{
				for (int i = 0; i < 3; i++)
				{
					if (kind == 0 ? edgeX[i] <= 0 : edgeX[i] >= 0)
						continue;

					int64_t value = edge[i] + edgeY[i] * s.rowBeg;
					int64_t divisor = kind == 0 ? edgeX[i] : -edgeX[i];
					int64_t numerator = kind == 0 ? -value : value;
					int64_t step = kind == 0 ? -edgeY[i] : edgeY[i];

					Edge& e = s.edges[s.nLower + s.nUpper];
					e.col = ceilDiv(numerator, divisor);
					e.rem = int32_t(e.col * divisor - numerator);
					e.divisor = int32_t(divisor);
					e.stepCol = int32_t(floorDiv(step, divisor));
					e.stepRem = int32_t(step - int64_t(e.stepCol) * divisor);

					if (kind == 0)
						s.nLower++;
					else
						s.nUpper++;
				}
			}

			// Values at the center of the pixel containing the first corner, and their changes.

			s.ox = int(floorDiv(c[0]->x, 64));
			s.oy = int(floorDiv(c[0]->y, 64));

			int64_t e[3];
			for (int i = 0; i < 3; i++)
				e[i] = edge[i] + edgeX[i] * s.ox + edgeY[i] * s.oy;

			auto setup = [&](int64_t* pF, const int64_t values[3])
			{
				int64_t d1 = values[1] - values[0];
				int64_t d2 = values[2] - values[0];

				pF[0] = values[0] + mulMulDiv(d1, e[1], d2, e[2], area);
				pF[1] = mulMulDiv(d1, edgeX[1], d2, edgeX[2], area);
				pF[2] = mulMulDiv(d1, edgeY[1], d2, edgeY[2], area);
			};

			// Coverage, interpolated without perspective.

			if (c[0]->coverage == coverageOne && c[1]->coverage == coverageOne && c[2]->coverage == coverageOne)
				s.fade = -1;
			else
			{
				s.fade = int(m_fades.size());
				Fade& fade = m_fades.emplace_back();

				int64_t values[3];
				for (int i = 0; i < 3; i++)
					values[i] = int64_t(c[i]->coverage) << 12;

				setup(fade.f, values);
			}

			if (c[0]->w == c[1]->w && c[1]->w == c[2]->w)
			{
				s.perspective = -1;

				for (int a = 0; a < 2; a++)
				{
					int64_t values[3];
					for (int i = 0; i < 3; i++)
						values[i] = int64_t(a == 0 ? c[i]->u : c[i]->v) << 26;

					setup(s.f[a], values);

					// Bounds over the area of the pixels: the corners give the bounds over the triangle,
					// and the pixels reach half of their change along x and y outside it.

					int64_t lo = std::min({ values[0], values[1], values[2] }) >> 16;
					int64_t hi = std::max({ values[0], values[1], values[2] }) >> 16;
					int64_t margin = ((abs64(s.f[a][1]) + abs64(s.f[a][2])) >> 17) + 64;

					s.lo[a] = lo - margin;
					s.hi[a] = hi + margin;
				}
			}
			else
			{
				s.perspective = int(m_perspective.size());
				Perspective& p = m_perspective.emplace_back();

				int largest = std::max({ c[0]->w, c[1]->w, c[2]->w });
				int shift = 0;
				while ((int64_t(largest) << (shift + 1)) <= (int64_t(1) << 28))
					shift++;

				int64_t weights[3];
				for (int i = 0; i < 3; i++)
					weights[i] = int64_t(c[i]->w) << shift;

				setup(p.f[2], weights);

				for (int a = 0; a < 2; a++)
				{
					int64_t ref = int64_t(a == 0 ? c[0]->u : c[0]->v) << 10;
					p.ref[a] = ref;

					int64_t values[3];
					for (int i = 0; i < 3; i++)
					{
						int64_t pos = int64_t(a == 0 ? c[i]->u : c[i]->v) << 10;
						values[i] = weights[i] * std::clamp(pos - ref, -c_maxRelative, c_maxRelative);
					}

					setup(p.f[a], values);
				}

				p.gradientW = abs64(p.f[2][1]) + abs64(p.f[2][2]);
			}

			m_setups.push_back(s);
		}

		// By first row, counted per row, keeping the order of the triangles within a row.

		std::vector<int>& rowStart = m_rowStart;
		rowStart.assign(m_size.h + 1, 0);

		for (const Setup& s : m_setups)
			rowStart[s.rowBeg + 1]++;

		for (int row = 0; row < m_size.h; row++)
			rowStart[row + 1] += rowStart[row];

		m_order.resize(m_setups.size());
		for (int i = 0; i < int(m_setups.size()); i++)
			m_order[rowStart[m_setups[i].rowBeg]++] = i;
	}

} // namespace wg
