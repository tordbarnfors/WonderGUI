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
#include <wg_roundedrectwarp.h>
#include <wg_gfxbase.h>

#include <algorithm>
#include <cmath>

namespace wg
{
	const TypeInfo RoundedRectWarp::TYPEINFO = { "RoundedRectWarp", &WarpGenerator::TYPEINFO };

	//____ create() ___________________________________________________________

	RoundedRectWarp_p RoundedRectWarp::create(const Blueprint& bp, WarpmapFactory* pFactory)
	{
		if (!_validateSettings(bp.size, bp.sourceSize, bp.cellSize, bp.precision) || bp.rect.w < 0 || bp.rect.h < 0
			|| bp.cornerRadius < 0 || bp.sweep == 0.f)
			return nullptr;

		if (!pFactory)
			pFactory = GfxBase::defaultWarpmapFactory();

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

	//____ _cornerRadius() ____________________________________________________
	// Radius of the corners of the outline in pixels, at most half its shorter side.

	double RoundedRectWarp::_cornerRadius() const
	{
		return std::min(m_cornerRadius / 64.0, std::min(m_rect.w, m_rect.h) / 128.0);
	}

	//____ _calcCellSize() ____________________________________________________
	/*
		Precision is kept at the tightest corner of the band, the one nearest to the center of the
		corners. If the band reaches the center of the corners, as for sharp corners, half the radius
		of its outer edge is used instead, as for a disc.
	*/

	int RoundedRectWarp::_calcCellSize() const
	{
		double r = _cornerRadius();
		double inner = r + std::min(m_topDistance, m_bottomDistance) / 64.0;
		double outer = r + std::max(m_topDistance, m_bottomDistance) / 64.0;

		return _cellSizeForRadius(inner > 0 ? inner : std::max(outer, 0.0) / 2);
	}

	//____ _uPeriod() _________________________________________________________
	/*
		The width of the source covers |sweep| of the outline, so all of it is width / |sweep|.
	*/

	spx RoundedRectWarp::_uPeriod() const
	{
		double period = std::round(m_sourceSize.w / std::abs(double(m_sweep)));
		return spx(std::clamp(period, 1.0, double(Warpmap::maxNodeValue)));
	}

	//____ _generateNodes() ___________________________________________________
	/*
		Each node is placed by the point of the outline nearest to it: its position along the
		outline (P, in pixels clockwise from the middle of the top side) and the signed distance to
		it (d, in pixels, positive outside).

		The outline is made of straight parts, ax and ay from the middle of the sides to where the
		corners start, and corners of radius r around the corners of the inner rectangle (ax, ay)
		from the center. Using the distances (qx, qy) beyond that inner rectangle, a node is nearest
		to a corner if both are positive and to a side otherwise, the side of the larger one.
		Inside the inner rectangle that splits it along its diagonals, which is where corners are
		mitered.

		u goes from 0 at start to the width of the source at start + sweep and is periodic with a
		period of width / |sweep|. v goes from 0 at topDistance to the height of the source at
		bottomDistance.
	*/

	void RoundedRectWarp::_generateNodes(CoordSPX* pNodes, SizeI nodes, int cellSize) const
	{
		const double pi = 3.14159265358979323846;

		double cx = (m_rect.x + m_rect.w / 2.0) / 64.0;
		double cy = (m_rect.y + m_rect.h / 2.0) / 64.0;

		double r = _cornerRadius();
		double ax = m_rect.w / 128.0 - r;
		double ay = m_rect.h / 128.0 - r;
		double arc = r * pi / 2;								// Length of a corner.

		double perimeter = 4 * ax + 4 * ay + 4 * arc;
		if (perimeter <= 0.0)
			perimeter = 1.0;									// A point, every position along it is 0.

		double width = m_sourceSize.w;
		double height = m_sourceSize.h;
		double period = _uPeriod();

		double top = m_topDistance;
		double bottom = m_bottomDistance;

		for (int row = 0; row < nodes.h; row++)
		{
			double y = std::min(row * cellSize, m_size.h) - cy;

			for (int col = 0; col < nodes.w; col++)
			{
				double x = std::min(col * cellSize, m_size.w) - cx;

				double qx = std::abs(x) - ax;
				double qy = std::abs(y) - ay;

				double pos, dist;

				if (qx > 0 && qy > 0)
				{
					// Corner, angle 0 -> pi/2 clockwise from where it starts.

					dist = std::sqrt(qx * qx + qy * qy) - r;

					if (x >= 0 && y < 0)
						pos = ax + r * std::atan2(qx, qy);
					else if (x >= 0)
						pos = ax + arc + 2 * ay + r * std::atan2(qy, qx);
					else if (y >= 0)
						pos = 3 * ax + 2 * arc + 2 * ay + r * std::atan2(qx, qy);
					else
						pos = 3 * ax + 3 * arc + 4 * ay + r * std::atan2(qy, qx);
				}
				else if (qx > qy)
				{
					// Left or right side.

					dist = qx - r;
					double sy = std::clamp(y, -ay, ay);
					pos = x >= 0 ? ax + arc + (sy + ay) : 3 * ax + 3 * arc + 2 * ay + (ay - sy);
				}
				else
				{
					// Top or bottom side. Left of the middle of the top side is negative, which the
					// period takes care of.

					dist = qy - r;
					double sx = std::clamp(x, -ax, ax);
					pos = y >= 0 ? ax + 2 * arc + 2 * ay + (ax - sx) : sx;
				}

				double u = (pos / perimeter - m_start) / m_sweep * width;
				u -= period * std::floor(u / period);

				double v = bottom != top ? (dist * 64 - top) / (bottom - top) * height : -Warpmap::maxNodeValue;

				*pNodes++ = { _toNodeValue(u), _toNodeValue(v) };
			}
		}
	}

} // namespace wg
