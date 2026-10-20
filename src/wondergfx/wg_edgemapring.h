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
	 * The ring is made of several pieces. Most of them run along canvas rows, while short
	 * pieces at the top and bottom of each circle run along columns, so that the circles
	 * stay round where they are nearly horizontal.
	 *
	 * Tints are not supported yet. Tints of the spec are ignored and the colors used as they
	 * are. Colors left undefined are not drawn, but are drawn as transparent in
	 * BlendMode::Replace, like any edgemap segment.
	 *
	 * A piece has at most Edgemap::maxSegments segments. Pieces where more slices meet are
	 * made smaller, down to single pixels near the center of a pie with many slices, where
	 * the narrowest slices are left out.
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

		struct Cap							// Cells at the top or bottom of a circle running along columns.
		{
			int			x0, y0, x1, y1;
		};

		bool		_setSpec( const RingSpec& spec );
		void		_regions( const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& regions ) const;
		int			_orientation( const RectI& cell ) const;
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
		bool		m_bSlices = false;								// Room for slices between the outlines.
		double		m_start = 0;									// Turns clockwise from 12 o'clock.

		std::vector<Interval>	m_intervals;
		std::vector<double>		m_boundaries;						// Angles of lines between intervals, in turns.

		std::vector<HiColor>	m_colors;							// Back, outline, hub and slices.

		// Layout

		bool				m_bCenterColumn = false;				// Cells of the pixel column of a center between pixels run along rows.
		bool				m_bCenterRow = false;					// Cells of the pixel row of a center between pixels run along columns.
		std::vector<Cap>	m_caps;
		std::vector<Cap>	m_strips;								// Rows with a horizontal edge between pixels, running along columns.
	};

}

#endif //WG_EDGEMAPRING_DOT_H
