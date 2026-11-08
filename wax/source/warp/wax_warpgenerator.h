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

#include <vector>

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
	 * rectangle of the blit, in spx. Generators make a mesh of triangles that follows the shape.
	 *
	 * Precision (max position error in pixels) decides how closely the Warpmap follows the shape,
	 * through the size of the pieces of the mesh, which each generator chooses for its shape.
	 *
	 * When subclassing, implement _meshSize() and _generateMesh(), sizing the pieces from
	 * precision. Override _uPeriod() and _vPeriod() for periodic axes and set m_bDirty when a
	 * setting changes.
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

		bool		setPrecision( float precision );
		float		precision() const { return m_precision; }

		//.____ Content _____________________________________________________

		// Generates the Warpmap from current settings, if they have changed since last time, and
		// returns it. The content of the Warpmap is updated in place, without copying and sent on to
		// the backend once, if nothing but the WarpGenerator holds it and its size and number of
		// vertices and triangles are unchanged. Otherwise a new Warpmap is created, so a Warpmap
		// held elsewhere, such as by a draw that isn't rendered yet, never changes. Draw what
		// generate() returns rather than keeping it.

		Warpmap_p	generate();

	protected:

		WarpGenerator( SizeI size, SizeSPX sourceSize, float precision, WarpmapFactory * pFactory );
		virtual ~WarpGenerator();

		static bool		_validateSettings( SizeI size, SizeSPX sourceSize, float precision );

		// Integer maths for _generateMesh(), which runs once per vertex and so should avoid floating
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

		// Sine and cosine of an angle in 1/2^32 of a turn, with 30 binals. Accurate to a few units.

		static void		_sinCos( uint32_t angle, int32_t& sin, int32_t& cos );

		// Square root rounded down.

		static uint64_t	_sqrt( uint64_t value );

		// Fraction of a turn in 1/2^32 of a turn, whole turns dropped. Not finite gives 0.

		static uint32_t	_toTurn( double turns );

		// A value of u or v clamped to +-Warpmap::maxValue.

		static spx		_toSourceValue( int64_t value );

		// Number of vertices and triangles of the Warpmap, at most Warpmap::maxVertices and
		// Warpmap::maxTriangles, and the vertices and triangles themselves. Vertices are clamped to
		// the limits of Warpmap afterwards.

		virtual void	_meshSize( int& nVertices, int& nTriangles ) const = 0;
		virtual void	_generateMesh( Warpmap::Vertex * pVertices, Warpmap::Triangle * pTriangles ) const = 0;

		// A band along the outline of a rounded rectangle, or a circle with ax and ay 0, with the
		// width of the source along the outline and its height across, for CircularWarp and
		// RoundedRectWarp. A position is placed by the point of the outline nearest to it: its
		// position along the outline and the distance to it. Lengths are in 1/256 spx.

		struct OutlineBand
		{
			int64_t		cx, cy;			// Center of the outline, from the top-left corner of the Warpmap.
			int64_t		ax, ay;			// Half the width and height of the outline, less the radius of the corners.
			int64_t		radius;			// Of the corners.
			int64_t		top, bottom;	// Distances from the outline of the top and bottom of the source, positive outwards.
			uint32_t	start;			// Where the left side of the source is, in 1/2^32 of the outline clockwise from the middle of the top side.
			double		sweep;			// Part of the outline covered by the width of the source, negative counter clockwise.
		};

		// Corners split into nArc sectors and the band into nAcross parts across it, from precision.

		void			_bandLayout( const OutlineBand& band, int& nArc, int& nAcross ) const;
		void			_calcBandLayout( const OutlineBand& band, int& nArc, int& nAcross ) const;

		// The mesh of a band, written to pVertices and pTriangles, or only counted if pVertices is
		// null, as for _meshSize().

		void			_bandMesh( const OutlineBand& band, Warpmap::Vertex * pVertices, Warpmap::Triangle * pTriangles, int& nVertices, int& nTriangles ) const;

		// The layout of the band last worked out, kept as long as what it depends on is the same.

		struct LayoutCache
		{
			bool		bValid = false;
			int64_t		ax, ay, radius, top, bottom;
			float		precision;
			int			nArc, nAcross;
		};

		mutable LayoutCache	m_layoutCache;

		virtual spx		_uPeriod() const { return 0; }			// In spx of the source, 0 = not periodic.
		virtual spx		_vPeriod() const { return 0; }

		// Called when the Warpmap has been generated, to keep settings that what is drawn with it
		// depends on, such as for offsets.

		virtual void	_generated() {}

		// Offsets along an axis of the source that move it as if its start was moved: _offsetFactor()
		// once per generate, from sweep in turns and the length of the source along the axis (spx),
		// then _offsetFor() for a shift in 1/2^32 of a turn (see _toTurn()), with integers only.

		static Factor	_offsetFactor( double sweepTurns, spx length );
		static spx		_offsetFor( uint32_t shift, Factor factor );

		SizeI		m_size;
		SizeSPX		m_sourceSize;
		float		m_precision;

		bool		m_bDirty = true;

		WarpmapFactory_p	m_pFactory;
		Warpmap_p			m_pWarpmap;
	};

} // namespace wax
#endif // WAX_WARPGENERATOR_DOT_H
