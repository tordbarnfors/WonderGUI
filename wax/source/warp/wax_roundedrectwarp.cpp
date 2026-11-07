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
#include <wax_roundedrectwarp.h>
#include <wax_base.h>

#include <algorithm>
#include <cmath>

namespace wax
{
	const TypeInfo RoundedRectWarp::TYPEINFO = { "RoundedRectWarp", &WarpGenerator::TYPEINFO };

	//____ create() ___________________________________________________________

	RoundedRectWarp_p RoundedRectWarp::create(const Blueprint& bp, WarpmapFactory* pFactory)
	{
		if (!_validateSettings(bp.size, bp.sourceSize, bp.cellSize, bp.precision) || bp.rect.w < 0 || bp.rect.h < 0
			|| bp.cornerRadius < 0 || bp.sweep == 0.f)
			return nullptr;

		if (!pFactory)
			pFactory = Base::defaultWarpmapFactory();

		if (!pFactory)
			return nullptr;

		return RoundedRectWarp_p(new RoundedRectWarp(bp, pFactory));
	}

	//____ constructor ________________________________________________________

	RoundedRectWarp::RoundedRectWarp(const Blueprint& bp, WarpmapFactory* pFactory)
		: WarpGenerator(bp.size, bp.sourceSize, bp.cellSize, bp.precision, pFactory)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		m_rect				= bp.rect;
		m_cornerRadius		= bp.cornerRadius;
		m_topDistance		= bp.topDistance;
		m_bottomDistance	= bp.bottomDistance;
		m_start				= bp.start;
		m_sweep				= bp.sweep;
		m_offsetFactor		= _offsetFactor(bp.sweep, bp.sourceSize.w);
	}

	//____ destructor _________________________________________________________

	RoundedRectWarp::~RoundedRectWarp()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& RoundedRectWarp::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setOutline() _______________________________________________________

	bool RoundedRectWarp::setOutline(const RectSPX& rect, spx cornerRadius)
	{
		if (rect.w < 0 || rect.h < 0 || cornerRadius < 0)
			return false;

		if (rect != m_rect || cornerRadius != m_cornerRadius)
		{
			m_rect = rect;
			m_cornerRadius = cornerRadius;
			m_bDirty = true;
		}
		return true;
	}

	//____ setDistances() _____________________________________________________

	bool RoundedRectWarp::setDistances(spx topDistance, spx bottomDistance)
	{
		if (topDistance == bottomDistance)
			return false;

		if (topDistance != m_topDistance || bottomDistance != m_bottomDistance)
		{
			m_topDistance = topDistance;
			m_bottomDistance = bottomDistance;
			m_bDirty = true;
		}
		return true;
	}

	//____ setSpan() __________________________________________________________

	bool RoundedRectWarp::setSpan(float start, float sweep)
	{
		if (sweep == 0.f)
			return false;

		if (start != m_start || sweep != m_sweep)
		{
			m_start = start;
			m_sweep = sweep;
			m_bDirty = true;
		}
		return true;
	}

	//____ shiftOffset() ______________________________________________________

	CoordSPX RoundedRectWarp::shiftOffset(float part) const
	{
		return { _offsetFor(_toTurn(part), m_offsetFactor), 0 };
	}

	//____ _cornerRadius() ____________________________________________________
	// Radius of the corners of the outline in pixels, at most half its shorter side.

	spx RoundedRectWarp::_cornerRadius() const
	{
		return std::min(m_cornerRadius, std::min(m_rect.w, m_rect.h) / 2);
	}

	//____ _band() ___________________________________________________________

	WarpGenerator::OutlineBand RoundedRectWarp::_band() const
	{
		OutlineBand band;
		band.radius = int64_t(_cornerRadius()) << 8;
		band.cx = (int64_t(m_rect.x) << 8) + (int64_t(m_rect.w) << 7);
		band.cy = (int64_t(m_rect.y) << 8) + (int64_t(m_rect.h) << 7);
		band.ax = (int64_t(m_rect.w) << 7) - band.radius;
		band.ay = (int64_t(m_rect.h) << 7) - band.radius;
		band.top = int64_t(m_topDistance) << 8;
		band.bottom = int64_t(m_bottomDistance) << 8;
		band.start = _toTurn(m_start);
		band.sweep = m_sweep;
		return band;
	}

	//____ _calcCellSize() ____________________________________________________

	SizeI RoundedRectWarp::_calcCellSize() const
	{
		return _bandCellSize(_band());
	}

	//____ _meshSize() ________________________________________________________

	void RoundedRectWarp::_meshSize(int& nVertices, int& nTriangles) const
	{
		std::vector<Warpmap::Vertex> vertices;
		std::vector<Warpmap::Triangle> triangles;
		_bandMesh(_band(), vertices, triangles);

		nVertices = int(vertices.size());
		nTriangles = int(triangles.size());
	}

	//____ _generateMesh() ____________________________________________________
	/*
		Each position is placed by the point of the outline nearest to it: its position along the
		outline (clockwise from the middle of the top side) and the signed distance to it (positive
		outside). u goes from 0 at start to the width of the source at start + sweep and is periodic
		with a period of width / |sweep|. v goes from 0 at topDistance to the height of the source at
		bottomDistance. See WarpGenerator::_bandMesh().
	*/

	void RoundedRectWarp::_generateMesh(Warpmap::Vertex* pVertices, Warpmap::Triangle* pTriangles) const
	{
		std::vector<Warpmap::Vertex> vertices;
		std::vector<Warpmap::Triangle> triangles;
		_bandMesh(_band(), vertices, triangles);

		std::copy(vertices.begin(), vertices.end(), pVertices);
		std::copy(triangles.begin(), triangles.end(), pTriangles);
	}

	//____ _uPeriod() _________________________________________________________
	/*
		The width of the source covers |sweep| of the outline, so all of it is width / |sweep|.
	*/

	spx RoundedRectWarp::_uPeriod() const
	{
		double period = m_sourceSize.w / std::abs(double(m_sweep)) + 0.5;
		return spx(std::clamp(period, 1.0, double(Warpmap::maxValue)));
	}

} // namespace wax
