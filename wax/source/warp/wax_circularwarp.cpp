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
#include <wax_base.h>

#include <algorithm>
#include <cmath>

namespace wax
{
	const TypeInfo CircularWarp::TYPEINFO = { "CircularWarp", &WarpGenerator::TYPEINFO };

	//____ create() ___________________________________________________________

	CircularWarp_p CircularWarp::create(const Blueprint& bp, WarpmapFactory* pFactory)
	{
		if (!_validateSettings(bp.size, bp.sourceSize, bp.cellSize, bp.precision) || bp.sweep == 0.f)
			return nullptr;

		if (!pFactory)
			pFactory = Base::defaultWarpmapFactory();

		if (!pFactory)
			return nullptr;

		return CircularWarp_p(new CircularWarp(bp, pFactory));
	}

	//____ constructor ________________________________________________________

	CircularWarp::CircularWarp(const Blueprint& bp, WarpmapFactory* pFactory)
		: WarpGenerator(bp.size, bp.sourceSize, bp.cellSize, bp.precision, pFactory)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		m_center		= bp.center;
		m_topRadius		= bp.topRadius;
		m_bottomRadius	= bp.bottomRadius;
		m_startAngle	= bp.startAngle;
		m_sweep			= bp.sweep;
		m_offsetFactor = _offsetFactor(bp.sweep / 360.0, bp.sourceSize.w);
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

	//____ rotationOffset() ___________________________________________________

	CoordSPX CircularWarp::rotationOffset(float degrees) const
	{
		return { _offsetFor(_toTurn(degrees * (1.0 / 360)), m_offsetFactor), 0 };
	}

	//____ _calcCellSize() ____________________________________________________
	/*
		Precision is kept at the smaller radius, where the error is largest. A disc has no smaller
		radius and can't show detail close to its center anyway, so half its radius is used instead.
	*/

	SizeI CircularWarp::_calcCellSize() const
	{
		spx inner = std::min(m_topRadius, m_bottomRadius);
		spx outer = std::max(m_topRadius, m_bottomRadius);

		int cellSize = _cellSizeForRadius(inner > 0 ? inner : outer / 2);
		return { cellSize, cellSize };
	}

	//____ _uPeriod() _________________________________________________________
	/*
		The width of the source covers |sweep| degrees, so a full turn is width * 360 / |sweep|.
	*/

	spx CircularWarp::_uPeriod() const
	{
		double period = m_sourceSize.w * 360.0 / std::abs(double(m_sweep)) + 0.5;
		return spx(std::clamp(period, 1.0, double(Warpmap::maxValue)));
	}

	//____ _generateNodes() ___________________________________________________
	/*
		Angle is measured clockwise from straight up. u goes from 0 at startAngle to the width of
		the source at startAngle + sweep (counter clockwise for negative sweep) and is periodic with
		a period of width * 360 / |sweep|. v goes from 0 at topRadius to the height of the source at
		bottomRadius. Positions are clamped to what a Warpmap can hold, far outside the source.

		Nodes are calculated with integers only, positions in 1/256 spx and angles in 1/2^32 of a
		turn. u is calculated from the angle to the node from startAngle in the direction of sweep,
		within a turn, so the first period is exact even though the period of the Warpmap is
		rounded to spx.
	*/

	void CircularWarp::_generateNodes(CoordSPX* pNodes, SizeI nodes, SizeI cellSize) const
	{
		const int c_fractionBits = 8;							// Of spx, so the center isn't rounded.
		const int64_t c_pixel = 64 << c_fractionBits;
		const double c_limit = 0x1p52;							// Keeps far away centers within int64 maths.

		auto toFixed = [c_limit](double value) { return value == value ? int64_t(std::clamp(value + (value < 0 ? -0.5 : 0.5), -c_limit, c_limit)) : 0; };

		int64_t cx = toFixed(m_center.x * m_size.w * double(c_pixel));
		int64_t cy = toFixed(m_center.y * m_size.h * double(c_pixel));
		int64_t topRadius = int64_t(m_topRadius) << c_fractionBits;

		uint32_t start = _toTurn(m_startAngle / 360.0);

		Factor uFactor = _factor(double(m_sourceSize.w) * 360.0 / (std::abs(double(m_sweep)) * 0x1p32));
		bool bClockwise = m_sweep > 0.f;
		Factor vFactor = _factor(double(m_sourceSize.h) / (double(m_bottomRadius - m_topRadius) * (1 << c_fractionBits)));
		bool bFlat = m_bottomRadius == m_topRadius;

		for (int row = 0; row < nodes.h; row++)
		{
			int64_t y = std::min(row * cellSize.h, m_size.h) * c_pixel - cy;

			for (int col = 0; col < nodes.w; col++)
			{
				int64_t x = std::min(col * cellSize.w, m_size.w) * c_pixel - cx;

				int64_t r;
				uint32_t angle = _polar(-y, x, &r);				// Clockwise from up.

				int64_t u = _multiply(bClockwise ? angle - start : start - angle, uFactor);
				int64_t v = bFlat ? -Warpmap::maxValue : _multiply(r - topRadius, vFactor);

				*pNodes++ = { _toNodeValue(u), _toNodeValue(v) };
			}
		}
	}

} // namespace wax
