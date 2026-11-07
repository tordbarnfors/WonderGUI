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
	static const int c_maxCellSize = 64;

	//____ constructor ________________________________________________________

	WarpGenerator::WarpGenerator(SizeI size, SizeSPX sourceSize, SizeI cellSize, float precision, WarpmapFactory* pFactory)
	{
		m_size		= size;
		m_sourceSize = sourceSize;
		m_cellSize	= cellSize;
		m_precision	= precision;
		m_precisionFixed = int64_t(double(precision) * 65536.0 + 0.5);
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
			m_precisionFixed = int64_t(double(precision) * 65536.0 + 0.5);
			m_bDirty = true;
		}
		return true;
	}

	//____ generate() _________________________________________________________
	/*
		Draws are rendered when the canvas update ends and read the Warpmap then, so a Warpmap that
		a draw holds (or anyone else) is left as it is and a new one created in its place.

		A Warpmap that only we hold has its vertices, triangles and periods written in place and
		sent on once.
	*/

	Warpmap_p WarpGenerator::generate()
	{
		if (!m_bDirty && m_pWarpmap)
			return m_pWarpmap;

		SizeI cellSize = this->cellSize();
		spx uPeriod = std::clamp(_uPeriod(), 0, Warpmap::maxValue);
		spx vPeriod = std::clamp(_vPeriod(), 0, Warpmap::maxValue);

		SizeI nodes = { (m_size.w + cellSize.w - 1) / cellSize.w + 1, (m_size.h + cellSize.h - 1) / cellSize.h + 1 };
		int nCells = (nodes.w - 1) * (nodes.h - 1);

		std::vector<CoordSPX> buffer(nodes.w * nodes.h);
		_generateNodes(buffer.data(), nodes, cellSize);

		Warpmap* pMap = m_pWarpmap;

		if (!pMap || pMap->refcount() > 1 || pMap->pixelSize() != m_size || pMap->vertexCount() != nCells * 4 || pMap->triangleCount() != nCells * 2)
		{
			Warpmap::Blueprint bp;
			bp.size = m_size;
			bp.triangles = nCells * 2;
			bp.uPeriod = uPeriod;
			bp.vertices = nCells * 4;
			bp.vPeriod = vPeriod;

			std::vector<Warpmap::Vertex> vertices(nCells * 4);
			std::vector<Warpmap::Triangle> triangles(nCells * 2);
			_gridToMesh(buffer.data(), nodes, cellSize, uPeriod, vPeriod, vertices.data(), triangles.data());

			m_pWarpmap = m_pFactory->createWarpmap(bp, vertices.data(), triangles.data());
		}
		else if (pMap->m_pVertices && pMap->m_pTriangles)
		{
			_gridToMesh(buffer.data(), nodes, cellSize, uPeriod, vPeriod, pMap->m_pVertices, pMap->m_pTriangles);
			pMap->m_uPeriod = uPeriod;
			pMap->m_vPeriod = vPeriod;
			pMap->_updated(0, nCells * 4, 0, nCells * 2);
		}
		else
		{
			// No content of its own to write to, it goes through importMesh().

			std::vector<Warpmap::Vertex> vertices(nCells * 4);
			std::vector<Warpmap::Triangle> triangles(nCells * 2);
			_gridToMesh(buffer.data(), nodes, cellSize, uPeriod, vPeriod, vertices.data(), triangles.data());

			pMap->importMesh(vertices.data(), triangles.data(), uPeriod, vPeriod);
		}

		_generated();

		m_bDirty = false;
		return m_pWarpmap;
	}

	//____ _gridToMesh() ______________________________________________________
	/*
		Each cell gets four vertices of its own, top-left, top-right, bottom-right and bottom-left,
		and two triangles split along the diagonal from top-left to bottom-right. Neighbouring cells
		have vertices in the same positions along their common edges, so the mesh has no gaps. Values
		on a periodic axis are brought into 0 -> period at the top-left corner and the other corners
		moved by whole periods to be as close to it as possible, so that interpolation across the
		seam takes the short way. That is why vertices aren't shared between cells.
	*/

	void WarpGenerator::_gridToMesh(const CoordSPX* pNodes, SizeI nodes, SizeI cellSize, spx uPeriod, spx vPeriod, Warpmap::Vertex* pVertices, Warpmap::Triangle* pTriangles) const
	{
		auto floorDiv = [](int64_t a, int64_t b)
		{
			int64_t q = a / b;
			return (q * b > a) ? q - 1 : q;
		};

		auto normalize = [&](int64_t value, int64_t period)
		{
			return period > 0 ? value - floorDiv(value, period) * period : value;
		};

		auto unwrapTo = [&](int64_t value, int64_t reference, int64_t period)
		{
			return period > 0 ? value - floorDiv(2 * (value - reference) + period, 2 * period) * period : value;
		};

		int vertex = 0;

		for (int row = 0; row < nodes.h - 1; row++)
		{
			spx y0 = std::min(row * cellSize.h, m_size.h) * 64;
			spx y1 = std::min((row + 1) * cellSize.h, m_size.h) * 64;

			for (int col = 0; col < nodes.w - 1; col++)
			{
				spx x0 = std::min(col * cellSize.w, m_size.w) * 64;
				spx x1 = std::min((col + 1) * cellSize.w, m_size.w) * 64;

				const CoordSPX* p = pNodes + row * nodes.w + col;
				const CoordSPX corners[4] = { p[0], p[1], p[nodes.w + 1], p[nodes.w] };
				const CoordSPX positions[4] = { { x0, y0 }, { x1, y0 }, { x1, y1 }, { x0, y1 } };

				int64_t u0 = normalize(corners[0].x, uPeriod);
				int64_t v0 = normalize(corners[0].y, vPeriod);

				for (int i = 0; i < 4; i++)
				{
					Warpmap::Vertex v;
					v.x = positions[i].x;
					v.y = positions[i].y;
					v.u = spx(i == 0 ? u0 : unwrapTo(corners[i].x, u0, uPeriod));
					v.v = spx(i == 0 ? v0 : unwrapTo(corners[i].y, v0, vPeriod));
					v.w = Warpmap::weightOne;

					pVertices[vertex + i] = Warpmap::_clamped(v);
				}

				*pTriangles++ = { vertex, vertex + 1, vertex + 2 };
				*pTriangles++ = { vertex, vertex + 2, vertex + 3 };
				vertex += 4;
			}
		}
	}

	//____ _validateSettings() ________________________________________________

	bool WarpGenerator::_validateSettings(SizeI size, SizeSPX sourceSize, SizeI cellSize, float precision)
	{
		return size.w > 0 && size.h > 0 && sourceSize.w > 0 && sourceSize.h > 0 && cellSize.w >= 0 && cellSize.h >= 0 && precision > 0.f;
	}

	//____ _cellSizeForRadius() _______________________________________________
	/*
		Cells are split into two triangles, in which positions are linear. A curve of radius r drawn
		through them is off by about d^2 / (8 * r), d being the length of the diagonal, which is
		c^2 / (4 * r) for cells of size c. The largest cell size that keeps within precision is
		then sqrt(4 * precision * r).
	*/

	int WarpGenerator::_cellSizeForRadius(int64_t radius) const
	{
		// 4 * precision * radius in pixels^2 is precision (16 binals) * radius (spx) / 2^20. Its
		// square root rounded down is that of the value rounded down.

		if (radius <= 0)
			return c_minCellSize;

		const int64_t c_limit = int64_t(1) << 40;				// Far beyond the largest cell, and no overflow.
		int64_t value = m_precisionFixed > 0 && radius > c_limit / m_precisionFixed ? c_limit : (m_precisionFixed * radius) >> 20;

		return std::clamp(int(_sqrt(uint64_t(value))), c_minCellSize, c_maxCellSize);
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
		return _toNodeValue(-_multiply(int32_t(shift), factor));
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

	//____ _toNodeValue() _____________________________________________________

	spx WarpGenerator::_toNodeValue(int64_t value)
	{
		return spx(std::clamp(value, -int64_t(Warpmap::maxValue), int64_t(Warpmap::maxValue)));
	}

} // namespace wax
