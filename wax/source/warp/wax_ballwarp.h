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
#ifndef	WAX_BALLWARP_DOT_H
#define	WAX_BALLWARP_DOT_H
#pragma once

#include <wax_warpgenerator.h>

namespace wax
{
	class BallWarp;
	typedef	StrongPtr<BallWarp>	BallWarp_p;
	typedef	WeakPtr<BallWarp>	BallWarp_wp;

	//____ BallWarp ____________________________________________________________
	/**
	 * @brief Makes a Warpmap that wraps the source around a ball, such as a globe from a map of the
	 * world.
	 *
	 * The ball is centered in the Warpmap, as large as fits, with its outline antialiased within it.
	 * It is seen from far away, straight on, without perspective, so half of it shows.
	 *
	 * The source is a map of the whole ball in equirectangular projection: its width goes all the way
	 * around, from 180 degrees west at the left edge to 180 degrees east at the right, and its height
	 * from the north pole at the top to the south pole at the bottom. The middle of the map faces the
	 * viewer. The width is periodic, so the map wraps around seamlessly.
	 *
	 * tilt leans the north pole towards the viewer, roll turns the axis of the ball clockwise on
	 * screen. Spin the ball around its axis by drawing with rotationOffset() rather than generating
	 * again.
	 *
	 * The Warpmap is a grid of meridians and parallels, which positions are linear between, cut where
	 * the ball turns away and with a vertex of its own at the poles for each piece. Pieces are as
	 * large as keeps them within precision, the same all over the ball. A row of triangles around
	 * the outline goes from full coverage of the vertices to none, which antialiases it, as it isn't
	 * a border of the source. The number of vertices and triangles only depends on size and
	 * precision, so tilting and rolling updates the Warpmap. Shading, such as the ball getting
	 * darker towards its outline, is done with a tint when drawing.
	 */

	class BallWarp : public WarpGenerator
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			Finalizer_p			finalizer = nullptr;
			float				precision = 1.f;				// Max position error in pixels, of outlines at most WarpGenerator::c_outlinePrecision.
			float				roll = 0.f;						// Degrees the axis of the ball is turned clockwise on screen.
			SizeI				size;							// Mandatory. Size of the Warpmap in pixels, which the ball is centered in.
			SizeSPX				sourceSize;						// Mandatory. Size of the map (edgemap or source rectangle) in spx.
			float				tilt = 0.f;						// Degrees the north pole leans towards the viewer, -90 to 90.
		};

		//.____ Creation __________________________________________

		static BallWarp_p	create( const Blueprint& blueprint, WarpmapFactory * pFactory = nullptr );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________________

		bool		setOrientation( float tilt, float roll );
		float		tilt() const { return m_tilt; }
		float		roll() const { return m_roll; }

		spx			radius() const;									// Of the ball, in spx.

		//.____ Drawing _____________________________________________________

		// Source offset that turns the ball by degrees around its axis, the way the Earth turns (from
		// west to east) if positive. Pass it when drawing to spin the ball without generating it
		// again. Goes by the source size of the generated Warpmap.

		CoordSPX	rotationOffset( float degrees ) const;

	protected:

		BallWarp( const Blueprint& bp, WarpmapFactory * pFactory );
		virtual ~BallWarp();

		static bool	_isValidTilt( float tilt ) { return tilt >= -90.f && tilt <= 90.f; }

		void		_meshSize( int& nVertices, int& nTriangles ) const override;
		void		_generateMesh( Warpmap::Vertex * pVertices, Warpmap::Triangle * pTriangles ) const override;
		spx			_uPeriod() const override { return m_sourceSize.w; }
		void		_generated() override { m_offsetFactor = _offsetFactor(1.0, m_sourceSize.w); }

		// Meridians and parallels of the grid, and how many crossings of them with the outline there
		// can be, which the number of vertices and triangles is made for.

		void		_layout( int& nLon, int& nLat ) const;
		static int	_maxCrossings( int nLon, int nLat ) { return 2 * (nLon + 1) + 2 * (nLat - 1); }
		static void	_counts( int nLon, int nLat, int outlinePoints, int& nVertices, int& nTriangles );

		// Points added along the outline between crossings, where the grid alone would leave it off
		// by more than the outline precision: at most how many, and the longest arc between two
		// points (1/2^32 of a turn), 0 for none.

		int			_outlinePoints( uint32_t& longestArc ) const;

		float		m_tilt;
		float		m_roll;

		Factor		m_offsetFactor;								// For rotationOffset(), by the generated Warpmap.
	};

} // namespace wax
#endif // WAX_BALLWARP_DOT_H
