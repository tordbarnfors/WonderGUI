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
#ifndef WAX_WARPGENERATOR_DOT_H
#define	WAX_WARPGENERATOR_DOT_H
#pragma once

#include <wax_warpmap.h>
#include <wax_warpmapfactory.h>

namespace wax
{
	class WarpGenerator;
	typedef	StrongPtr<WarpGenerator>	WarpGenerator_p;
	typedef	WeakPtr<WarpGenerator>	WarpGenerator_wp;

	//____ WarpGenerator _______________________________________________________
	/**
	 * @brief Base class for generators that make a Warpmap from a few settings, such as CircularWarp.
	 *
	 * A WarpGenerator keeps its Warpmap and updates it when generate() is called, see generate().
	 *
	 * The Warpmap is made for a source of sourceSize: the size of the edgemap or of the source
	 * rectangle of the blit, in spx. The nodes are positions in that source, see Warpmap.
	 *
	 * The cell size of the Warpmap decides how closely it follows the shape. It is calculated by the
	 * generator from precision (max position error in pixels) unless set explicitly, and kept within
	 * 4 -> 64 pixels.
	 *
	 * When subclassing, implement _generateNodes() and _calcCellSize(), override _uPeriod() and
	 * _vPeriod() for periodic axes and set m_bDirty when a setting changes.
	 */

	class WarpGenerator : public Object
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________________

		bool		setSize( SizeI size );
		SizeI		size() const { return m_size; }

		bool		setSourceSize( SizeSPX sourceSize );
		SizeSPX		sourceSize() const { return m_sourceSize; }

		//.____ Precision ___________________________________________________

		bool		setCellSize( int cellSize );					// 0 = calculated from precision.
		int			cellSize() const;								// Cell size in use, calculated or set.

		bool		setPrecision( float precision );
		float		precision() const { return m_precision; }

		//.____ Content _____________________________________________________

		// Generates the Warpmap from current settings, if they have changed since last time, and
		// returns it. The nodes of the Warpmap are updated in place if nothing but the WarpGenerator
		// holds it and size and cell size are unchanged. Otherwise a new Warpmap is created, so a
		// Warpmap held elsewhere, such as by a draw that isn't rendered yet, never changes. Draw
		// what generate() returns rather than keeping it.

		Warpmap_p	generate();

	protected:

		WarpGenerator( SizeI size, SizeSPX sourceSize, int cellSize, float precision, WarpmapFactory * pFactory );
		virtual ~WarpGenerator();

		static bool		_validateSettings( SizeI size, SizeSPX sourceSize, int cellSize, float precision );

		// Largest cell size that keeps a curve of the given radius (in pixels) within precision.

		int				_cellSizeForRadius( double radius ) const;

		// Cell size from precision and the shape. Clamped by cellSize().

		virtual int		_calcCellSize() const = 0;

		// Fills pNodes with nodes.w * nodes.h positions in spx of the source, row by row, nodes being
		// cellSize pixels apart (the last row and column at the edge). Keep them within Warpmap::maxNodeValue.

		virtual void	_generateNodes( CoordSPX * pNodes, SizeI nodes, int cellSize ) const = 0;

		virtual spx		_uPeriod() const { return 0; }			// In spx of the source, 0 = not periodic.
		virtual spx		_vPeriod() const { return 0; }

		SizeI		m_size;
		SizeSPX		m_sourceSize;
		int			m_cellSize;										// As set, 0 = calculated.
		float		m_precision;

		bool		m_bDirty = true;

		WarpmapFactory_p	m_pFactory;
		Warpmap_p			m_pWarpmap;
	};

} // namespace wax
#endif // WAX_WARPGENERATOR_DOT_H
