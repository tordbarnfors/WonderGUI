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
#ifndef WG_WARPGENERATOR_DOT_H
#define	WG_WARPGENERATOR_DOT_H
#pragma once

#include <wg_warpmap.h>
#include <wg_warpmapfactory.h>

namespace wg
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
	 * The cell size of the Warpmap decides how closely it follows the shape. Width and height of
	 * cells are calculated by the generator from precision (max position error in pixels) unless
	 * set explicitly, at least 4 and at most the size of the Warpmap. Along curves it is kept within
	 * 4 -> 64 pixels, along straight axes cells can span the Warpmap.
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

		bool		setCellSize( SizeI cellSize );					// 0 for width or height = calculated from precision.
		SizeI		cellSize() const;								// Cell size in use, calculated or set.

		bool		setPrecision( float precision );
		float		precision() const { return m_precision; }

		//.____ Content _____________________________________________________

		// Generates the Warpmap from current settings, if they have changed since last time, and
		// returns it. The nodes of the Warpmap are updated in place, without copying and sent on to
		// the backend once, if nothing but the WarpGenerator holds it and size and cell size are
		// unchanged. Otherwise a new Warpmap is created, so a
		// Warpmap held elsewhere, such as by a draw that isn't rendered yet, never changes. Draw
		// what generate() returns rather than keeping it.

		Warpmap_p	generate();

	protected:

		WarpGenerator( SizeI size, SizeSPX sourceSize, SizeI cellSize, float precision, WarpmapFactory * pFactory );
		virtual ~WarpGenerator();

		static bool		_validateSettings( SizeI size, SizeSPX sourceSize, SizeI cellSize, float precision );

		// Largest cell size that keeps a curve of the given radius (in pixels) within precision,
		// within 4 -> 64 pixels.

		int				_cellSizeForRadius( double radius ) const;

		// Integer maths for _generateNodes(), which runs once per node and so should avoid floating
		// point on hardware without an FPU. Set up factors with _factor() once per generate.

		struct Factor
		{
			uint32_t	mul = 0;									// value * mul / 2^shift.
			int			shift = 0;
			bool		bNegative = false;
		};

		static Factor	_factor( double factor );					// NaN gives a factor of 0.

		// Value multiplied by factor, rounded and clamped to +-2^62.

		static int64_t	_multiply( int64_t value, Factor factor );

		// Angle of x,y in 1/2^32 of a turn, from the x axis towards the y axis, and optionally its
		// length, rounded. Accurate to a few units of the angle and better than 1/2^26 of the length.

		static uint32_t	_polar( int64_t x, int64_t y, int64_t * pLength = nullptr );

		// Square root rounded down.

		static uint64_t	_sqrt( uint64_t value );

		// Fraction of a turn in 1/2^32 of a turn, whole turns dropped. Not finite gives 0.

		static uint32_t	_toTurn( double turns );

		// A value clamped to +-Warpmap::maxNodeValue.

		static spx		_toNodeValue( int64_t value );

		// Cell width and height from precision and the shape. Kept within 4 -> size of the Warpmap
		// by cellSize().

		virtual SizeI	_calcCellSize() const = 0;

		// Fills pNodes with nodes.w * nodes.h positions in spx of the source, row by row, nodes being
		// cellSize.w pixels apart horizontally and cellSize.h vertically (the last row and column at
		// the edge). Keep them within Warpmap::maxNodeValue.

		virtual void	_generateNodes( CoordSPX * pNodes, SizeI nodes, SizeI cellSize ) const = 0;

		virtual spx		_uPeriod() const { return 0; }			// In spx of the source, 0 = not periodic.
		virtual spx		_vPeriod() const { return 0; }

		// Called when the Warpmap has been generated, to keep settings that what is drawn with it
		// depends on, such as for offsets. m_generatedSourceSize is kept already.

		virtual void	_generated() {}

		// Offset along an axis of the source that moves it as if its start was moved by shift, given
		// in the same unit as sweep, which turn is a whole round of. length is the length of the
		// source along the axis, in spx. Rounded to spx.

		static spx		_offsetFor( double shift, double sweep, double turn, spx length );

		SizeI		m_size;
		SizeSPX		m_sourceSize;
		SizeSPX		m_generatedSourceSize;							// Source size of the generated Warpmap.
		SizeI		m_cellSize;										// As set, 0 = calculated.
		float		m_precision;

		bool		m_bDirty = true;

		WarpmapFactory_p	m_pFactory;
		Warpmap_p			m_pWarpmap;
	};

} // namespace wg
#endif // WG_WARPGENERATOR_DOT_H
