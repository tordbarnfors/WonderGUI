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
#include <wg_warpgenerator.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace wg
{
	const TypeInfo WarpGenerator::TYPEINFO = { "WarpGenerator", &Object::TYPEINFO };

	static const int c_minCellSize = 4;
	static const int c_maxCellSize = 64;

	//____ constructor ________________________________________________________

	WarpGenerator::WarpGenerator(SizeI size, SizeSPX sourceSize, SizeI cellSize, float precision, WarpmapFactory* pFactory)
	{
		m_size		= size;
		m_sourceSize = sourceSize;
		m_generatedSourceSize = sourceSize;
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

		A Warpmap that only we hold has its nodes and periods written in place, then normalized and
		sent on once.
	*/

	Warpmap_p WarpGenerator::generate()
	{
		if (!m_bDirty && m_pWarpmap)
			return m_pWarpmap;

		SizeI cellSize = this->cellSize();
		spx uPeriod = _uPeriod();
		spx vPeriod = _vPeriod();

		if (!m_pWarpmap || m_pWarpmap->refcount() > 1 || m_pWarpmap->pixelSize() != m_size || m_pWarpmap->cellSize() != cellSize)
		{
			Warpmap::Blueprint bp;
			bp.size = m_size;
			bp.cellSize = cellSize;
			bp.uPeriod = uPeriod;
			bp.vPeriod = vPeriod;

			SizeI nodes = { (m_size.w + cellSize.w - 1) / cellSize.w + 1, (m_size.h + cellSize.h - 1) / cellSize.h + 1 };
			std::vector<CoordSPX> buffer(nodes.w * nodes.h);
			_generateNodes(buffer.data(), nodes, cellSize);

			m_pWarpmap = m_pFactory->createWarpmap(bp, buffer.data());
		}
		else
		{
			Warpmap* pMap = m_pWarpmap;
			SizeI nodes = pMap->nodes();

			if (pMap->m_pNodes)
			{
				RectI all = { 0, 0, nodes.w, nodes.h };

				_generateNodes(pMap->m_pNodes, nodes, cellSize);
				pMap->m_uPeriod = uPeriod;
				pMap->m_vPeriod = vPeriod;
				pMap->_normalizeNodes(all);
				pMap->_nodesUpdated(all);
			}
			else
			{
				// No nodes of its own to write to, they go through importAllNodes().

				std::vector<CoordSPX> buffer(nodes.w * nodes.h);
				_generateNodes(buffer.data(), nodes, cellSize);
				pMap->importAllNodes(buffer.data(), uPeriod, vPeriod);
			}
		}

		m_generatedSourceSize = m_sourceSize;
		_generated();

		m_bDirty = false;
		return m_pWarpmap;
	}

	//____ _validateSettings() ________________________________________________

	bool WarpGenerator::_validateSettings(SizeI size, SizeSPX sourceSize, SizeI cellSize, float precision)
	{
		return size.w > 0 && size.h > 0 && sourceSize.w > 0 && sourceSize.h > 0 && cellSize.w >= 0 && cellSize.h >= 0 && precision > 0.f;
	}

	//____ _cellSizeForRadius() _______________________________________________
	/*
		A curve of radius r drawn through cells of size c is off by about c^2 / (8 * r), so the
		largest cell size that keeps within precision is sqrt(8 * precision * r).
	*/

	int WarpGenerator::_cellSizeForRadius(double radius) const
	{
		return std::clamp(int(std::sqrt(8.0 * m_precision * radius)), c_minCellSize, c_maxCellSize);
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

	//____ _offsetFor() _______________________________________________________
	/*
		A position along the axis is (position - start) / sweep * length, so moving start by shift
		moves it by -shift / sweep * length. Whole turns move it by whole periods, which the renderer
		wraps away, so they are dropped first to keep the offset small and exact.
	*/

	spx WarpGenerator::_offsetFor(double shift, double sweep, double turn, spx length)
	{
		shift = std::fmod(shift, turn);
		if (!std::isfinite(shift) || sweep == 0.0)
			return 0;

		double offset = -shift / sweep * length;
		return spx(std::llround(std::clamp(offset, -double(Warpmap::maxNodeValue), double(Warpmap::maxNodeValue))));
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

	uint32_t WarpGenerator::_toTurn(double turns)
	{
		if (!std::isfinite(turns))
			return 0;

		return uint32_t(int64_t((turns - std::floor(turns)) * 0x1p32));		// 2^32 wraps to 0.
	}

	//____ _toNodeValue() _____________________________________________________

	spx WarpGenerator::_toNodeValue(int64_t value)
	{
		return spx(std::clamp(value, -int64_t(Warpmap::maxNodeValue), int64_t(Warpmap::maxNodeValue)));
	}

} // namespace wg
