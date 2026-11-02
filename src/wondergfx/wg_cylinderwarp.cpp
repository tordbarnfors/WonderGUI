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
#include <wg_cylinderwarp.h>
#include <wg_gfxbase.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace wg
{
	const TypeInfo CylinderWarp::TYPEINFO = { "CylinderWarp", &WarpGenerator::TYPEINFO };

	static const double c_degreesToRadians = 3.14159265358979323846 / 180.0;

	//____ create() ___________________________________________________________

	CylinderWarp_p CylinderWarp::create(const Blueprint& bp, WarpmapFactory* pFactory)
	{
		if (!_validateSettings(bp.size, bp.sourceSize, bp.cellSize, bp.precision) || (bp.axis != Axis::X && bp.axis != Axis::Y) ||
			!_isValidSpan(bp.span) || bp.sweep == 0.f)
			return nullptr;

		if (!pFactory)
			pFactory = GfxBase::defaultWarpmapFactory();

		if (!pFactory)
			return nullptr;

		return CylinderWarp_p(new CylinderWarp(bp, pFactory));
	}

	//____ constructor ________________________________________________________

	CylinderWarp::CylinderWarp(const Blueprint& bp, WarpmapFactory* pFactory)
		: WarpGenerator(bp.size, bp.sourceSize, bp.cellSize, bp.precision, pFactory)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		m_axis			= bp.axis;
		m_span			= bp.span;
		m_startAngle	= bp.startAngle;
		m_sweep			= bp.sweep;

		m_generatedAxis		= bp.axis;
		m_generatedSweep	= bp.sweep;
	}

	//____ destructor _________________________________________________________

	CylinderWarp::~CylinderWarp()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& CylinderWarp::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setAxis() __________________________________________________________

	bool CylinderWarp::setAxis(Axis axis)
	{
		if (axis != Axis::X && axis != Axis::Y)
			return false;

		if (axis != m_axis)
		{
			m_axis = axis;
			m_bDirty = true;
		}
		return true;
	}

	//____ setSpan() __________________________________________________________

	bool CylinderWarp::setSpan(float span)
	{
		if (!_isValidSpan(span))
			return false;

		if (span != m_span)
		{
			m_span = span;
			m_bDirty = true;
		}
		return true;
	}

	//____ setAngles() ________________________________________________________

	bool CylinderWarp::setAngles(float startAngle, float sweep)
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

	CoordSPX CylinderWarp::rotationOffset(float degrees) const
	{
		if (m_generatedAxis == Axis::X)
			return { 0, _offsetFor(degrees, m_generatedSweep, 360.0, m_generatedSourceSize.h) };
		else
			return { _offsetFor(degrees, m_generatedSweep, 360.0, m_generatedSourceSize.w), 0 };
	}

	//____ _calcCellSize() ____________________________________________________
	/*
		Along the axis positions change linearly, so one cell spans the cylinder. Across it, the
		angle seen at distance y from the middle is asin(y / R), R being the radius. Interpolating
		that over cells of c pixels is off by about c^2 / 8 times its second derivative, which in
		pixels on screen is c^2 * sin(a) / (8 * R * cos(a)^2) at angle a. That is largest at the
		rims, a = span / 2, so it is kept there, as for a circle of radius R * cos(a)^2 / sin(a).
	*/

	SizeI CylinderWarp::_calcCellSize() const
	{
		double rim = m_span / 2 * c_degreesToRadians;
		int across = m_axis == Axis::X ? m_size.h : m_size.w;

		double radius = across / 2.0 / std::sin(rim);
		int cell = _cellSizeForRadius(radius * std::cos(rim) * std::cos(rim) / std::sin(rim));

		return m_axis == Axis::X ? SizeI(m_size.w, cell) : SizeI(cell, m_size.h);
	}

	//____ _periodAcross() ____________________________________________________
	/*
		The length of the source across the axis covers |sweep| degrees, so a full turn is that
		length * 360 / |sweep|.
	*/

	spx CylinderWarp::_periodAcross() const
	{
		spx length = m_axis == Axis::X ? m_sourceSize.h : m_sourceSize.w;
		double period = std::round(length * 360.0 / std::abs(double(m_sweep)));
		return spx(std::clamp(period, 1.0, double(Warpmap::maxNodeValue)));
	}

	//____ _uPeriod() _________________________________________________________

	spx CylinderWarp::_uPeriod() const
	{
		return m_axis == Axis::Y ? _periodAcross() : 0;
	}

	//____ _vPeriod() _________________________________________________________

	spx CylinderWarp::_vPeriod() const
	{
		return m_axis == Axis::X ? _periodAcross() : 0;
	}

	//____ _generateNodes() ___________________________________________________
	/*
		Positions along the axis depend only on the column of nodes (axis X) or row (axis Y) and
		positions across it only on the other, so each is worked out once for its column or row.

		Along the axis the source is stretched over the length of the cylinder. Across it, at
		distance y from the middle, the angle is asin(y / R), from the side facing the viewer, and
		the position is (angle - startAngle) / sweep * length of the source, counted from startAngle
		in the direction of sweep within a turn, as for CircularWarp. The angle is found from y and
		the depth sqrt(R^2 - y^2) with integers, positions in 1/256 spx.
	*/

	void CylinderWarp::_generateNodes(CoordSPX* pNodes, SizeI nodes, SizeI cellSize) const
	{
		const int c_fractionBits = 8;
		const int64_t c_pixel = 64 << c_fractionBits;

		const bool bAxisX = m_axis == Axis::X;

		const int lengthAlong = bAxisX ? m_size.w : m_size.h;
		const int lengthAcross = bAxisX ? m_size.h : m_size.w;
		const int cellAlong = bAxisX ? cellSize.w : cellSize.h;
		const int cellAcross = bAxisX ? cellSize.h : cellSize.w;
		const int nodesAlong = bAxisX ? nodes.w : nodes.h;
		const int nodesAcross = bAxisX ? nodes.h : nodes.w;

		const spx sourceAlong = bAxisX ? m_sourceSize.w : m_sourceSize.h;
		const spx sourceAcross = bAxisX ? m_sourceSize.h : m_sourceSize.w;

		// Along the axis, rounded to spx.

		std::vector<spx> along(nodesAlong);
		for (int i = 0; i < nodesAlong; i++)
		{
			int64_t p = std::min(i * cellAlong, lengthAlong);
			along[i] = spx((p * sourceAlong * 2 + lengthAlong) / (2 * int64_t(lengthAlong)));
		}

		// Across the axis.

		double rim = m_span / 2 * c_degreesToRadians;
		int64_t radius = std::llround(lengthAcross * c_pixel / 2.0 / std::sin(rim));
		int64_t middle = lengthAcross * c_pixel / 2;

		int shift = 0;											// Keeps radius^2 within 64 bits.
		while ((radius >> shift) >= (int64_t(1) << 31))
			shift++;

		int64_t r = radius >> shift;

		uint32_t start = _toTurn(m_startAngle / 360.0);
		Factor factor = _factor(double(sourceAcross) * 360.0 / (std::abs(double(m_sweep)) * 0x1p32));
		bool bForward = m_sweep > 0.f;

		std::vector<spx> across(nodesAcross);
		for (int i = 0; i < nodesAcross; i++)
		{
			int64_t y = std::min(i * cellAcross, lengthAcross) * c_pixel - middle;
			int64_t absY = std::min((y < 0 ? -y : y) >> shift, r);

			int64_t depth = int64_t(_sqrt(uint64_t(r * r - absY * absY)));
			uint32_t angle = _polar(depth, y < 0 ? -absY : absY);

			across[i] = _toNodeValue(_multiply(bForward ? angle - start : start - angle, factor));
		}

		// Nodes, row by row.

		for (int row = 0; row < nodes.h; row++)
			for (int col = 0; col < nodes.w; col++)
				*pNodes++ = bAxisX ? CoordSPX(along[col], across[row]) : CoordSPX(across[col], along[row]);
	}

} // namespace wg
