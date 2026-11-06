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
#ifndef WG_EDGEMAPBOX_DOT_H
#define WG_EDGEMAPBOX_DOT_H
#pragma once

#include <wg_edgemapshape.h>
#include <wg_gfxdevice.h>

namespace wg
{
	class EdgemapBox;
	typedef	StrongPtr<EdgemapBox>	EdgemapBox_p;
	typedef	WeakPtr<EdgemapBox>		EdgemapBox_wp;

	//____ EdgemapBox _________________________________________________________
	/**
	 * @brief A box with rounded corners and an outline made into edgemaps.
	 *
	 * The box is described by a RoundedBoxSpec, exactly as for GfxDevice::drawRoundedBox(),
	 * and looks the same when drawn, apart from small differences along the corners.
	 *
	 * The box is made of one edgemap running along canvas rows, or up to three if it has an
	 * outline: one for the outline above the inside, one for the rest and one for the
	 * outline below. Since edges are straight between rows, pixels where a corner is nearly
	 * horizontal are less exact. In the row where a corner meets a horizontal edge, the corner
	 * gets a slope that looks like it instead of following it, since a straight edge from the
	 * end of the horizontal edge would make a point. Horizontal edges are moved to the nearest
	 * pixel line.
	 *
	 * Tints are not supported yet. Tints of the spec are ignored and the colors used as they
	 * are. Colors left undefined are not drawn, but are drawn as transparent in
	 * BlendMode::Replace, like any edgemap segment.
	 */

	class EdgemapBox : public EdgemapShape
	{
	public:

		struct Blueprint
		{
			EdgemapFactory_p	factory;								// Default edgemap factory of GfxBase if not set.
			Finalizer_p			finalizer = nullptr;
		};

		//.____ Creation __________________________________________

		static EdgemapBox_p	create( const Blueprint& blueprint, const RoundedBoxSpec& spec );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _____________________________________________________

		bool					update( const RoundedBoxSpec& spec );
		const RoundedBoxSpec&	spec() const { return m_spec; }

	protected:

		EdgemapBox( const Blueprint& bp );
		virtual ~EdgemapBox() {}

		struct Box							// A box with circular corners, in pixels.
		{
			double	left = 0, top = 0, right = 0, bottom = 0;
			double	radius[4] = { 0, 0, 0, 0 };				// Top-left, top-right, bottom-right and bottom-left.

			double	centerX(int corner) const { return (corner == 0 || corner == 3) ? left + radius[corner] : right - radius[corner]; }
			double	centerY(int corner) const { return (corner == 0 || corner == 1) ? top + radius[corner] : bottom - radius[corner]; }

			bool	contains( double x, double y ) const;
			bool	span( double y, double& left, double& right ) const;	// Where a row at y is inside, false if nowhere.
		};

		struct Band							// Rows of a piece and the edges they are between.
		{
			int		top, bottom;
			double	topEdge, bottomEdge;
		};

		bool		_setSpec( const RoundedBoxSpec& spec );
		void		_regions( const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& regions ) const;
		uint32_t	_classify( double x, double y, bool bRow, int& color ) const;
		void		_adjustSamples( const Layout& layout, int nEdges, spx * pSamples ) const override;
		bool		_isSameSpec( const RoundedBoxSpec& spec ) const;
		bool		_isSameGeometry( const RoundedBoxSpec& spec ) const;
		void		_cacheCrossings( const std::vector<Layout>& layout );
		void		_buildDirectly( const std::vector<Layout>& layout );

		EdgemapFactory_p	m_pFactory;
		RoundedBoxSpec		m_spec;

		Box					m_outer;
		Box					m_inner;							// Inner edge of the outline.
		bool				m_bOutline = false;					// Has an outline, otherwise m_inner isn't used.
		double				m_centerX = 0, m_centerY = 0;

		HiColor				m_colors[2];						// Fill and outline.
		bool				m_bFillDrawn = false;
		bool				m_bOutlineDrawn = false;

		std::vector<Band>	m_bands;

		// Built directly, where the edges of the box cross each line is kept until its geometry
		// changes, as edges in spx: outside left, outline left, fill right, outline right.

		struct DirectPiece
		{
			std::vector<spx>	crossings;
			uint32_t			present = 0;	// One bit per layer, if it has any width on any line.
		};

		struct EdgeSource
		{
			const spx*	pSamples;
			int			stride;
		};

		std::vector<DirectPiece>	m_directPieces;
		bool						m_bCrossingsCached = false;
		std::vector<Build>			m_directBuilds;		// Written, then swapped with m_builds, so they all get reused.
		std::vector<EdgeSource>		m_edgeSources;
		std::vector<Layout>			m_layout;			// Of the pieces, as last laid out.
		bool						m_bHasSpec = false;
	};

}

#endif //WG_EDGEMAPBOX_DOT_H
