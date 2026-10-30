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
#ifndef WAX_ROUNDEDRECTWARP_DOT_H
#define	WAX_ROUNDEDRECTWARP_DOT_H
#pragma once

#include <wax_warpgenerator.h>

namespace wax
{
	class RoundedRectWarp;
	typedef	StrongPtr<RoundedRectWarp>	RoundedRectWarp_p;
	typedef	WeakPtr<RoundedRectWarp>	RoundedRectWarp_wp;

	//____ RoundedRectWarp _____________________________________________________
	/**
	 * @brief Generates Warpmaps that bend a source around the outline of a rounded rectangle.
	 *
	 * The outline is rect with corners of cornerRadius. The width of the source goes along it,
	 * clockwise from start (as a part of the outline, 0 being the middle of the top side), covering
	 * sweep of it (1.0 being all of it, negative going counter clockwise). Its height goes from
	 * topDistance (top of the source) to bottomDistance (bottom of the source), which are distances
	 * from the outline, positive outwards, negative inwards. With a sweep of 1.0 the source wraps
	 * around and an edgemap or surface that is tiling joins seamlessly.
	 *
	 * Positions along the outline are those of the nearest point on it, so the source keeps its
	 * width along the straight sides and is spread or squeezed around the corners, more the further
	 * from the outline. Place the outline in the middle of the band for the most even result.
	 *
	 * Corners stay round on the outside of the outline. Inside, deeper than cornerRadius, they become
	 * mitered like a picture frame, where positions along the outline jump. A Warpmap interpolates
	 * across that, so keep the band within cornerRadius on the inside, or use small cells there.
	 *
	 * cornerRadius is limited to half the shorter side of rect, so a square with a cornerRadius of
	 * half its side gives a circle, as CircularWarp.
	 *
	 * The Warpmap is made for a source of sourceSize (see WarpGenerator). The cell size decides how
	 * closely it follows the corners, the straight sides are exact. Unless set explicitly it is
	 * calculated from precision (max position error in pixels) at the tightest corner of the band.
	 *
	 * Like other WarpGenerators, RoundedRectWarp keeps its Warpmap and updates it when generate() is called.
	 */

	class RoundedRectWarp : public WarpGenerator
	{
	public:

		struct Blueprint
		{
			spx					bottomDistance = 0;				// Distance from the outline of the bottom of the source, positive outwards.
			int					cellSize = 0;					// Pixels between nodes of the Warpmap. 0 = calculated, see above.
			spx					cornerRadius = 0;				// Radius of the corners of the outline, at most half its shorter side.
			Finalizer_p			finalizer = nullptr;
			float				precision = 0.25f;				// Max position error in pixels when cellSize is calculated.
			RectSPX				rect;							// Mandatory. The outline, relative to the top-left corner of the Warpmap.
			SizeI				size;							// Mandatory. Size of the Warpmap in pixels.
			SizeSPX				sourceSize;						// Mandatory. Size of the source (edgemap or source rectangle) in spx.
			float				start = 0.f;					// Where the left side of the source is placed, as a part of the outline from the middle of the top side.
			float				sweep = 1.f;					// Part of the outline covered by the width of the source. Negative goes counter clockwise.
			spx					topDistance = 0;				// Distance from the outline of the top of the source, positive outwards.
		};

		//.____ Creation __________________________________________

		static RoundedRectWarp_p	create( const Blueprint& blueprint, WarpmapFactory * pFactory = nullptr );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________________

		bool		setOutline( const RectSPX& rect, spx cornerRadius );
		RectSPX		rect() const { return m_rect; }
		spx			cornerRadius() const { return m_cornerRadius; }

		bool		setDistances( spx topDistance, spx bottomDistance );
		spx			topDistance() const { return m_topDistance; }
		spx			bottomDistance() const { return m_bottomDistance; }

		bool		setSpan( float start, float sweep );
		float		start() const { return m_start; }
		float		sweep() const { return m_sweep; }

	protected:

		RoundedRectWarp( const Blueprint& bp, WarpmapFactory * pFactory );
		virtual ~RoundedRectWarp();

		int			_calcCellSize() const override;
		void		_generateNodes( CoordSPX * pNodes, SizeI nodes, int cellSize ) const override;
		spx			_uPeriod() const override;

		double		_cornerRadius() const;

		RectSPX		m_rect;
		spx			m_cornerRadius;
		spx			m_topDistance;
		spx			m_bottomDistance;
		float		m_start;
		float		m_sweep;
	};

} // namespace wax
#endif // WAX_ROUNDEDRECTWARP_DOT_H
