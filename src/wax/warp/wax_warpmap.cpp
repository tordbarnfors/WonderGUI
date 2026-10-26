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
#include <wax_warpmap.h>

#include <cmath>
#include <cstring>

namespace wax
{

	const TypeInfo Warpmap::TYPEINFO = { "Warpmap", &Object::TYPEINFO };

	//____ constructor ________________________________________________________

	Warpmap::Warpmap(const Blueprint& bp) : m_size(bp.size), m_cellSize(bp.cellSize), m_uPeriod(bp.uPeriod), m_vPeriod(bp.vPeriod)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		m_nodes.w = (m_size.w + m_cellSize - 1) / m_cellSize + 1;
		m_nodes.h = (m_size.h + m_cellSize - 1) / m_cellSize + 1;

		m_pNodes = new CoordF[m_nodes.w * m_nodes.h];

		// Start out mapping the rectangle straight onto the source.

		for (int row = 0; row < m_nodes.h; row++)
			for (int col = 0; col < m_nodes.w; col++)
				m_pNodes[row * m_nodes.w + col] = { float(nodeX(col)) / m_size.w, float(nodeY(row)) / m_size.h };

		_normalizeNodes({ 0,0,m_nodes.w,m_nodes.h });
	}

	//____ destructor _________________________________________________________

	Warpmap::~Warpmap()
	{
		delete[] m_pNodes;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Warpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setPeriods() _______________________________________________________

	bool Warpmap::setPeriods(float uPeriod, float vPeriod)
	{
		if (uPeriod < 0.f || vPeriod < 0.f)
			return false;

		m_uPeriod = uPeriod;
		m_vPeriod = vPeriod;

		RectI all = { 0,0,m_nodes.w,m_nodes.h };

		if (m_pNodes)
			_normalizeNodes(all);

		_nodesUpdated(all);
		return true;
	}

	//____ importNodes() ______________________________________________________

	bool Warpmap::importNodes(const RectI& region, const CoordF* pSource, int pitch)
	{
		if (!pSource || region.x < 0 || region.y < 0 || region.w < 0 || region.h < 0 ||
			region.x + region.w > m_nodes.w || region.y + region.h > m_nodes.h)
			return false;

		if (pitch == 0)
			pitch = region.w;

		if (m_pNodes)
		{
			for (int row = 0; row < region.h; row++)
				std::memcpy(m_pNodes + (region.y + row) * m_nodes.w + region.x, pSource + row * pitch, region.w * sizeof(CoordF));

			_normalizeNodes(region);
		}

		_nodesUpdated(region);
		return true;
	}

	//____ exportNodes() ______________________________________________________

	bool Warpmap::exportNodes(const RectI& region, CoordF* pDestination, int pitch) const
	{
		if (!pDestination || !m_pNodes || region.x < 0 || region.y < 0 || region.w < 0 || region.h < 0 ||
			region.x + region.w > m_nodes.w || region.y + region.h > m_nodes.h)
			return false;

		if (pitch == 0)
			pitch = region.w;

		for (int row = 0; row < region.h; row++)
			std::memcpy(pDestination + row * pitch, m_pNodes + (region.y + row) * m_nodes.w + region.x, region.w * sizeof(CoordF));

		return true;
	}

	//____ cellCorners() ______________________________________________________

	void Warpmap::cellCorners(int column, int row, CoordF* pCorners) const
	{
		const CoordF* p = m_pNodes + row * m_nodes.w + column;

		pCorners[0] = p[0];
		pCorners[1] = p[1];
		pCorners[2] = p[m_nodes.w];
		pCorners[3] = p[m_nodes.w + 1];

		for (int i = 1; i < 4; i++)
		{
			pCorners[i].x = _unwrapTo(pCorners[i].x, pCorners[0].x, m_uPeriod);
			pCorners[i].y = _unwrapTo(pCorners[i].y, pCorners[0].y, m_vPeriod);
		}
	}

	//____ positionAt() _______________________________________________________

	CoordF Warpmap::positionAt(double x, double y) const
	{
		int column = std::clamp(int(x / m_cellSize), 0, m_nodes.w - 2);
		int row = std::clamp(int(y / m_cellSize), 0, m_nodes.h - 2);

		CoordF c[4];
		cellCorners(column, row, c);

		double x0 = nodeX(column), x1 = nodeX(column + 1);
		double y0 = nodeY(row), y1 = nodeY(row + 1);

		double fx = (x - x0) / (x1 - x0);
		double fy = (y - y0) / (y1 - y0);

		double u = (c[0].x * (1 - fx) + c[1].x * fx) * (1 - fy) + (c[2].x * (1 - fx) + c[3].x * fx) * fy;
		double v = (c[0].y * (1 - fx) + c[1].y * fx) * (1 - fy) + (c[2].y * (1 - fx) + c[3].y * fx) * fy;

		return { float(u), float(v) };
	}

	//____ _validateBlueprint() _______________________________________________

	bool Warpmap::_validateBlueprint(const Blueprint& bp)
	{
		if (bp.size.w <= 0 || bp.size.h <= 0)
			return false;

		if (bp.cellSize < 1)
			return false;

		if (bp.uPeriod < 0.f || bp.vPeriod < 0.f)
			return false;

		return true;
	}

	//____ _unwrapTo() ________________________________________________________
	/*
		Moves value by whole periods to be as close as possible to reference.
	*/

	float Warpmap::_unwrapTo(float value, float reference, float period)
	{
		if (period <= 0.f)
			return value;

		return value - period * std::round((value - reference) / period);
	}

	//____ _normalizeNodes() __________________________________________________
	/*
		Brings values on periodic axes into 0 -> period.
	*/

	void Warpmap::_normalizeNodes(const RectI& region)
	{
		if (m_uPeriod <= 0.f && m_vPeriod <= 0.f)
			return;

		for (int row = region.y; row < region.y + region.h; row++)
		{
			CoordF* p = m_pNodes + row * m_nodes.w + region.x;

			for (int col = 0; col < region.w; col++)
			{
				if (m_uPeriod > 0.f)
				{
					p->x -= m_uPeriod * std::floor(p->x / m_uPeriod);
					if (p->x >= m_uPeriod)
						p->x = 0.f;
				}

				if (m_vPeriod > 0.f)
				{
					p->y -= m_vPeriod * std::floor(p->y / m_vPeriod);
					if (p->y >= m_vPeriod)
						p->y = 0.f;
				}
				p++;
			}
		}
	}

} // namespace wax
