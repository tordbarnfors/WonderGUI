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
#ifndef WG_EDGEMAPRING_DOT_H
#define WG_EDGEMAPRING_DOT_H
#pragma once

#include <wg_edgemapshape.h>
#include <wg_gfxdevice.h>

namespace wg
{
	class EdgemapRing;
	typedef	StrongPtr<EdgemapRing>	EdgemapRing_p;
	typedef	WeakPtr<EdgemapRing>	EdgemapRing_wp;

	//____ EdgemapRing ________________________________________________________
	/**
	 * @brief A ring, pie or donut chart, gauge or disc made into edgemaps.
	 *
	 * The ring is described by a RingSpec, exactly as for GfxDevice::drawRing(), and looks
	 * the same when drawn, apart from small differences along edges. A pixel where a line
	 * between slices meets a circle can differ more, as can pixels where slices meet in the
	 * center.
	 *
	 * The ring is made of one edgemap running along canvas rows, or four if it has slices, one
	 * for each quarter around the center. A square ring with an outline gets one more above
	 * and one below, for the outline. Since edges are straight between rows, pixels where
	 * a circle is nearly horizontal, at its top and bottom, are less exact. Straight horizontal
	 * edges, of the rect with a back color and of a square ring, are moved to the nearest
	 * pixel line, as are lines between slices running straight along a pixel row or column
	 * through a center between pixels.
	 *
	 * Tints are not supported yet. Tints of the spec are ignored and the colors used as they
	 * are. Colors left undefined are not drawn, but are drawn as transparent in
	 * BlendMode::Replace, like any edgemap segment.
	 *
	 * An edgemap has at most Edgemap::maxSegments segments, so a quarter can only hold so many
	 * slices. Creating or updating a ring with too many slices in a quarter fails.
	 */

	class EdgemapRing : public EdgemapShape
	{
	public:

		struct Blueprint
		{
			EdgemapFactory_p	factory;								// Default edgemap factory of GfxBase if not set.
			Finalizer_p			finalizer = nullptr;
		};

		//.____ Creation __________________________________________

		static EdgemapRing_p	create( const Blueprint& blueprint, const RingSpec& spec );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _____________________________________________________

		bool				update( const RingSpec& spec );
		const RingSpec&		spec() const { return m_spec; }

	protected:

		EdgemapRing( const Blueprint& bp );
		virtual ~EdgemapRing() {}

		struct Interval						// Slice or gap between slices.
		{
			double		end;				// Turns from start.
			int			color;				// Index of color, -1 if not drawn.
		};

		bool		_setSpec( const RingSpec& spec );
		void		_prepare( const RingSpec& spec );
		void		_regions( const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& regions ) const;
		uint32_t	_classify( double x, double y, bool bRow, double reference, int& color ) const;

		EdgemapFactory_p		m_pFactory;

		RingSpec				m_spec;
		std::vector<RingSlice>	m_slices;

		// Geometry in pixels, as ReferenceBackend draws a ring.

		double		m_left = 0, m_top = 0, m_right = 0, m_bottom = 0;
		double		m_centerX = 0, m_centerY = 0;
		double		m_radius[4] = { 0, 0, 0, 0 };					// Outer edge, inside of outer outline, outside of inner outline, hole.
		bool		m_bSquare = false;
		double		m_insetLeft = 0, m_insetTop = 0, m_insetRight = 0, m_insetBottom = 0;
		double		m_rowTop = 0, m_rowBottom = 0, m_rowInsetTop = 0, m_rowInsetBottom = 0;		// Horizontal edges, on pixel lines.
		bool		m_bSlices = false;								// Room for slices between the outlines.
		double		m_start = 0;									// Turns clockwise from 12 o'clock.

		std::vector<Interval>	m_intervals;
		std::vector<double>		m_boundaries;						// Angles of lines between intervals, in turns.

		std::vector<HiColor>	m_colors;							// Back, outline, hub and slices.

		std::vector<Layout>		m_layout;
		int			m_middleTop = 0, m_middleBottom = 0;			// Rows between the pieces of a square outline.
		bool		m_bQuarters = false;							// The middle has a piece for each quarter, split at m_splitX, m_splitY.
		int			m_splitX = 0, m_splitY = 0;
	};

}

#endif //WG_EDGEMAPRING_DOT_H
