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
	 * The box is made of several pieces. Most of them run along canvas rows, while short
	 * pieces at the top and bottom of each corner run along columns, so that corners stay
	 * round where they are nearly horizontal, as do rows with a horizontal edge between
	 * pixels.
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
		};

		struct Area							// Cells running along columns or rows.
		{
			int		x0, y0, x1, y1;
			bool	bRows;
		};

		bool		_setSpec( const RoundedBoxSpec& spec );
		void		_regions( const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& regions ) const;
		int			_orientation( const RectI& cell ) const;
		uint32_t	_classify( double x, double y, bool bRow, int& color ) const;

		EdgemapFactory_p	m_pFactory;
		RoundedBoxSpec		m_spec;

		Box					m_outer;
		Box					m_inner;							// Inner edge of the outline.
		bool				m_bOutline = false;					// Has an outline, otherwise m_inner isn't used.
		double				m_centerX = 0, m_centerY = 0;

		HiColor				m_colors[2];						// Fill and outline.
		bool				m_bFillDrawn = false;
		bool				m_bOutlineDrawn = false;

		std::vector<Area>	m_areas;							// Ahead of others: strips first, then caps.
	};

}

#endif //WG_EDGEMAPBOX_DOT_H
