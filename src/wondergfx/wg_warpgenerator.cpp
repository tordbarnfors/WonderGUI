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

	WarpGenerator::WarpGenerator(SizeI size, SizeSPX sourceSize, int cellSize, float precision, WarpmapFactory* pFactory)
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

	bool WarpGenerator::setCellSize(int cellSize)
	{
		if (cellSize < 0)
			return false;

		if (cellSize != m_cellSize)
		{
			m_cellSize = cellSize;
			m_bDirty = true;
		}
		return true;
	}

	//____ cellSize() _________________________________________________________

	int WarpGenerator::cellSize() const
	{
		return m_cellSize > 0 ? m_cellSize : std::clamp(_calcCellSize(), c_minCellSize, c_maxCellSize);
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
	*/

	Warpmap_p WarpGenerator::generate()
	{
		if (!m_bDirty && m_pWarpmap)
			return m_pWarpmap;

		int cellSize = this->cellSize();
		spx uPeriod = _uPeriod();
		spx vPeriod = _vPeriod();

		if (!m_pWarpmap || m_pWarpmap->refcount() > 1 || m_pWarpmap->pixelSize() != m_size || m_pWarpmap->cellSize() != cellSize)
		{
			Warpmap::Blueprint bp;
			bp.size = m_size;
			bp.cellSize = cellSize;
			bp.uPeriod = uPeriod;
			bp.vPeriod = vPeriod;

			SizeI nodes = { (m_size.w + cellSize - 1) / cellSize + 1, (m_size.h + cellSize - 1) / cellSize + 1 };
			std::vector<CoordSPX> buffer(nodes.w * nodes.h);
			_generateNodes(buffer.data(), nodes, cellSize);

			m_pWarpmap = m_pFactory->createWarpmap(bp, buffer.data());
		}
		else
		{
			SizeI nodes = m_pWarpmap->nodes();
			std::vector<CoordSPX> buffer(nodes.w * nodes.h);
			_generateNodes(buffer.data(), nodes, cellSize);

			if (m_pWarpmap->uPeriod() != uPeriod || m_pWarpmap->vPeriod() != vPeriod)
				m_pWarpmap->setPeriods(uPeriod, vPeriod);

			m_pWarpmap->importNodes({ 0,0,nodes.w,nodes.h }, buffer.data());
		}

		m_bDirty = false;
		return m_pWarpmap;
	}

	//____ _validateSettings() ________________________________________________

	bool WarpGenerator::_validateSettings(SizeI size, SizeSPX sourceSize, int cellSize, float precision)
	{
		return size.w > 0 && size.h > 0 && sourceSize.w > 0 && sourceSize.h > 0 && cellSize >= 0 && precision > 0.f;
	}

	//____ _cellSizeForRadius() _______________________________________________
	/*
		A curve of radius r drawn through cells of size c is off by about c^2 / (8 * r), so the
		largest cell size that keeps within precision is sqrt(8 * precision * r).
	*/

	int WarpGenerator::_cellSizeForRadius(double radius) const
	{
		return int(std::sqrt(8.0 * m_precision * radius));
	}

} // namespace wg
