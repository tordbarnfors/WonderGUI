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
		if (!_validateSettings(bp.size, bp.sourceSize, bp.precision) || bp.sweep == 0.f)
			return nullptr;

		if (!pFactory)
			pFactory = Base::defaultWarpmapFactory();

		if (!pFactory)
			return nullptr;

		return CircularWarp_p(new CircularWarp(bp, pFactory));
	}

	//____ constructor ________________________________________________________

	CircularWarp::CircularWarp(const Blueprint& bp, WarpmapFactory* pFactory)
		: WarpGenerator(bp.size, bp.sourceSize, bp.precision, pFactory)
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

	//____ _band() ___________________________________________________________
	/*
		The outline is the circle of the larger radius, a rounded rectangle with nothing but corners,
		and the radii are distances from it.
	*/

	WarpGenerator::OutlineBand CircularWarp::_band() const
	{
		const double c_limit = 0x1p52;							// Keeps far away centers within int64 maths.
		auto toFixed = [c_limit](double value) { return value == value ? int64_t(std::clamp(value + (value < 0 ? -0.5 : 0.5), -c_limit, c_limit)) : 0; };

		OutlineBand band;
		band.cx = toFixed(m_center.x * m_size.w * double(64 << 8));
		band.cy = toFixed(m_center.y * m_size.h * double(64 << 8));
		band.ax = 0;
		band.ay = 0;
		band.radius = int64_t(std::max(m_topRadius, m_bottomRadius)) << 8;
		band.top = (int64_t(m_topRadius) << 8) - band.radius;
		band.bottom = (int64_t(m_bottomRadius) << 8) - band.radius;
		band.start = _toTurn(m_startAngle / 360.0);
		band.sweep = m_sweep / 360.0;
		return band;
	}

	//____ _meshSize() ________________________________________________________

	void CircularWarp::_meshSize(int& nVertices, int& nTriangles) const
	{
		_bandMesh(_band(), nullptr, nullptr, nVertices, nTriangles);
	}

	//____ _generateMesh() ____________________________________________________
	/*
		Angle is measured clockwise from straight up. u goes from 0 at startAngle to the width of
		the source at startAngle + sweep (counter clockwise for negative sweep) and is periodic with
		a period of width * 360 / |sweep|. v goes from 0 at topRadius to the height of the source at
		bottomRadius.
	*/

	void CircularWarp::_generateMesh(Warpmap::Vertex* pVertices, Warpmap::Triangle* pTriangles) const
	{
		int nVertices, nTriangles;
		_bandMesh(_band(), pVertices, pTriangles, nVertices, nTriangles);
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

} // namespace wax
