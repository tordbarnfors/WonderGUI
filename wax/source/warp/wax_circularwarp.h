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
#ifndef WAX_CIRCULARWARP_DOT_H
#define	WAX_CIRCULARWARP_DOT_H
#pragma once

#include <wax_warpgenerator.h>

namespace wax
{
	class CircularWarp;
	typedef	StrongPtr<CircularWarp>	CircularWarp_p;
	typedef	WeakPtr<CircularWarp>	CircularWarp_wp;

	//____ CircularWarp ________________________________________________________
	/**
	 * @brief Generates Warpmaps that bend a source into a ring, an arc or a disc.
	 *
	 * The width of the source goes around the circle, starting at startAngle and covering sweep
	 * degrees. Its height goes from topRadius (top of the source) to bottomRadius (bottom of the
	 * source), either of which can be the larger. With a sweep of 360 degrees the source wraps
	 * around and an edgemap or surface that is tiling joins seamlessly. A smaller sweep gives an arc.
	 *
	 * A radius of 0 gives a disc. The angle is undefined in the center, so only use a disc with
	 * content that doesn't change from left to right in the cells closest to the center. Tints that
	 * vary horizontally will look blocky there. A flat color is fine, as long as no edge comes within
	 * two cells of the center.
	 *
	 * The cell size of the Warpmap decides how closely it follows the circle. It is calculated from
	 * precision (max position error in pixels) and detailRadius (the smallest radius where that
	 * precision is needed) unless set explicitly.
	 *
	 * Like other WarpGenerators, CircularWarp keeps its Warpmap and updates it when refresh() is called.
	 */

	class CircularWarp : public WarpGenerator
	{
	public:

		struct Blueprint
		{
			spx					bottomRadius = 0;				// Radius the bottom of the source is placed at.
			int					cellSize = 0;					// Pixels between nodes of the Warpmap. 0 = calculated, see above.
			CoordF				center = { 0.5f, 0.5f };		// Center of circle, relative to size.
			spx					detailRadius = 0;				// Smallest radius where precision is needed. 0 = smallest of topRadius and bottomRadius,
																// which needs to be set for discs.
			Finalizer_p			finalizer = nullptr;
			float				precision = 0.25f;				// Max position error in pixels when cellSize is calculated.
			SizeI				size;							// Mandatory. Size of the Warpmap in pixels.
			float				startAngle = 0.f;				// Degrees. Where the left side of the source is placed. 0 = up, increasing clockwise.
			float				sweep = 360.f;					// Degrees covered by the width of the source. Negative goes counter clockwise.
			spx					topRadius = 0;					// Radius the top of the source is placed at.
		};

		//.____ Creation __________________________________________

		static CircularWarp_p	create( const Blueprint& blueprint, WarpmapFactory * pFactory = nullptr );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________________

		bool		setCenter( CoordF center );
		CoordF		center() const { return m_center; }

		bool		setRadii( spx topRadius, spx bottomRadius );
		spx			topRadius() const { return m_topRadius; }
		spx			bottomRadius() const { return m_bottomRadius; }

		bool		setAngles( float startAngle, float sweep );
		float		startAngle() const { return m_startAngle; }
		float		sweep() const { return m_sweep; }

		//.____ Precision ___________________________________________________

		bool		setDetailRadius( spx detailRadius );			// 0 = smallest of topRadius and bottomRadius.
		spx			detailRadius() const { return m_detailRadius; }

	protected:

		CircularWarp( const Blueprint& bp, WarpmapFactory * pFactory );
		virtual ~CircularWarp();

		int			_calcCellSize() const override;
		void		_generateNodes( CoordF * pNodes, SizeI nodes, int cellSize ) const override;
		float		_uPeriod() const override;

		CoordF		m_center;
		spx			m_topRadius;
		spx			m_bottomRadius;
		float		m_startAngle;
		float		m_sweep;

		spx			m_detailRadius;
	};

} // namespace wax
#endif // WAX_CIRCULARWARP_DOT_H
