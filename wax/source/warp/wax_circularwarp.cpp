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
#include <wax_circularwarp.h>
#include <wax_core.h>

#include <algorithm>
#include <cmath>

namespace wax
{
	const TypeInfo CircularWarp::TYPEINFO = { "CircularWarp", &WarpGenerator::TYPEINFO };

	//____ create() ___________________________________________________________

	CircularWarp_p CircularWarp::create(const Blueprint& bp, WarpmapFactory* pFactory)
	{
		if (!_validateSettings(bp.size, bp.cellSize, bp.precision) || bp.sweep == 0.f)
			return nullptr;

		if (!pFactory)
			pFactory = Core::defaultWarpmapFactory();

		if (!pFactory)
			return nullptr;

		return CircularWarp_p(new CircularWarp(bp, pFactory));
	}

	//____ constructor ________________________________________________________

	CircularWarp::CircularWarp(const Blueprint& bp, WarpmapFactory* pFactory)
		: WarpGenerator(bp.size, bp.cellSize, bp.precision, pFactory)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		m_center		= bp.center;
		m_topRadius		= bp.topRadius;
		m_bottomRadius	= bp.bottomRadius;
		m_startAngle	= bp.startAngle;
		m_sweep			= bp.sweep;
		m_detailRadius	= bp.detailRadius;
	}

	//____ destructor _________________________________________________________

	CircularWarp::~CircularWarp()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& CircularWarp::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setCenter() ________________________________________________________

	bool CircularWarp::setCenter(CoordF center)
	{
		if (center != m_center)
		{
			m_center = center;
			m_bDirty = true;
		}
		return true;
	}

	//____ setRadii() _________________________________________________________

	bool CircularWarp::setRadii(spx topRadius, spx bottomRadius)
	{
		if (topRadius < 0 || bottomRadius < 0 || topRadius == bottomRadius)
			return false;

		if (topRadius != m_topRadius || bottomRadius != m_bottomRadius)
		{
			m_topRadius = topRadius;
			m_bottomRadius = bottomRadius;
			m_bDirty = true;
		}
		return true;
	}

	//____ setAngles() ________________________________________________________

	bool CircularWarp::setAngles(float startAngle, float sweep)
	{
		if (sweep == 0.f)
			return false;

		if (startAngle != m_startAngle || sweep != m_sweep)
		{
			m_startAngle = startAngle;
			m_sweep = sweep;
			m_bDirty = true;
		}
		return true;
	}

	//____ setDetailRadius() __________________________________________________

	bool CircularWarp::setDetailRadius(spx detailRadius)
	{
		if (detailRadius < 0)
			return false;

		if (detailRadius != m_detailRadius)
		{
			m_detailRadius = detailRadius;
			m_bDirty = true;
		}
		return true;
	}

	//____ _calcCellSize() ____________________________________________________

	int CircularWarp::_calcCellSize() const
	{
		spx detail = m_detailRadius > 0 ? m_detailRadius : std::min(m_topRadius, m_bottomRadius);

		return _cellSizeForRadius(detail / 64.0);
	}

	//____ _uPeriod() _________________________________________________________

	float CircularWarp::_uPeriod() const
	{
		return 360.f / std::abs(m_sweep);
	}

	//____ _generateNodes() ___________________________________________________
	/*
		Angle is measured clockwise from straight up. u goes from 0 at startAngle to 1 at
		startAngle + sweep (counter clockwise for negative sweep) and is periodic with a period
		of 360 / |sweep|. v goes from 0 at topRadius to 1 at bottomRadius.
	*/

	void CircularWarp::_generateNodes(CoordF* pNodes, SizeI nodes, int cellSize) const
	{
		double cx = m_center.x * m_size.w;
		double cy = m_center.y * m_size.h;

		double rTop = m_topRadius / 64.0;
		double rBottom = m_bottomRadius / 64.0;

		double sweep = m_sweep;
		double period = 360.0 / std::abs(sweep);

		for (int row = 0; row < nodes.h; row++)
		{
			double y = std::min(row * cellSize, m_size.h) - cy;

			for (int col = 0; col < nodes.w; col++)
			{
				double x = std::min(col * cellSize, m_size.w) - cx;

				double angle = std::atan2(x, -y) * 180.0 / 3.14159265358979323846;		// Clockwise from up, -180 -> 180.
				double u = (angle - m_startAngle) / sweep;
				u -= period * std::floor(u / period);

				double r = std::sqrt(x * x + y * y);
				double v = (r - rTop) / (rBottom - rTop);

				*pNodes++ = { float(u), float(v) };
			}
		}
	}

} // namespace wax
