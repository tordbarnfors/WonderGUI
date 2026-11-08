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
#ifndef	WAX_LENSWARP_DOT_H
#define	WAX_LENSWARP_DOT_H
#pragma once

#include <wax_warpgenerator.h>

#include <vector>

namespace wax
{
	class LensWarp;
	typedef	StrongPtr<LensWarp>	LensWarp_p;
	typedef	WeakPtr<LensWarp>	LensWarp_wp;

	//____ LensWarp ____________________________________________________________
	/**
	 * @brief Makes a Warpmap of a round lens that magnifies what is under it, such as a
	 * magnifying glass held over some graphics.
	 *
	 * The lens is centered in the Warpmap, as large as fits, with its outline antialiased within
	 * it. The source is what is under the lens: normally a source rectangle of the same size as the
	 * Warpmap, in the surface that holds the graphics, so that the source is as large as the Warpmap
	 * where there is no magnification. Move the lens by drawing it somewhere else with the source
	 * rectangle moved along, which doesn't take generating again.
	 *
	 * In the middle of the lens the source is magnified by magnification. Over the outer part of its
	 * radius given by falloff, the magnification falls off to none at the rim, where the source then
	 * meets what is around the lens without a seam, squeezed more the shorter the falloff is:
	 *
	 * - falloff 0 magnifies all of the lens evenly, like a magnifying glass, with a sharp edge.
	 * - falloff 0.2 to 0.4 magnifies the middle evenly and squeezes the source towards the rim, like
	 *   thick glass.
	 * - falloff 1 magnifies the most in the middle and less and less out to the rim, like a bubble.
	 *
	 * The Warpmap is the middle of the lens as one polygon, where positions are linear, rings
	 * through the falloff as close as keeps them within precision, at least two pixels apart and
	 * every other one turned by half a piece, and a ring of triangles around the outline that goes
	 * from full coverage of the vertices to none, which antialiases it. Pieces along the rim are as
	 * long as keeps the outline within precision. Without falloff, magnification only changes
	 * positions in the source, so the Warpmap is updated in place. A frame, a highlight or a shadow
	 * are drawn separately.
	 *
	 * The lens is drawn over what is already there, so graphics with transparency in them are
	 * blended twice where it is. Draw it from a surface with the finished graphics on it.
	 */

	class LensWarp : public WarpGenerator
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			float				falloff = 0.f;					// Part of the radius, 0 to 1, over which magnification falls off to none at the rim.
			Finalizer_p			finalizer = nullptr;
			float				magnification = 2.f;			// In the middle of the lens, 1 or more.
			float				precision = 0.25f;				// Max position error in pixels.
			SizeI				size;							// Mandatory. Size of the Warpmap in pixels, which the lens is centered in.
			SizeSPX				sourceSize;						// Mandatory. Size of the source under the lens in spx, normally size * 64.
		};

		//.____ Creation __________________________________________

		static LensWarp_p	create( const Blueprint& blueprint, WarpmapFactory * pFactory = nullptr );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________________

		bool		setMagnification( float magnification );
		float		magnification() const { return m_magnification; }

		bool		setFalloff( float falloff );
		float		falloff() const { return m_falloff; }

		spx			radius() const;									// Of the lens, in spx.

	protected:

		LensWarp( const Blueprint& bp, WarpmapFactory * pFactory );
		virtual ~LensWarp();

		static bool	_isValidMagnification( float magnification ) { return magnification >= 1.f && magnification <= 65536.f; }
		static bool	_isValidFalloff( float falloff ) { return falloff >= 0.f && falloff <= 1.f; }

		void		_meshSize( int& nVertices, int& nTriangles ) const override;
		void		_generateMesh( Warpmap::Vertex * pVertices, Warpmap::Triangle * pTriangles ) const override;

		// Rings of vertices from the middle out, each with as many vertices as there are pieces
		// along the rim. The first is the outline of the middle polygon, the last half a pixel
		// outside the outline. Worked out when the settings change.

		struct Ring
		{
			int64_t		radius;										// In 1/256 spx of the Warpmap.
			int64_t		u, v;										// Distance from the middle of the source, in 1/256 spx.
			int			coverage;
		};

		struct Layout
		{
			bool		bValid = false;
			SizeI		size;
			SizeSPX		sourceSize;
			float		precision;
			float		magnification;
			float		falloff;

			int					pieces;
			std::vector<Ring>	rings;
		};

		const Layout&	_layout() const;
		void			_calcLayout( Layout& layout ) const;
		static void		_counts( int pieces, int rings, int& nVertices, int& nTriangles );

		float		m_magnification;
		float		m_falloff;

		mutable Layout	m_layout;
	};

} // namespace wax
#endif // WAX_LENSWARP_DOT_H
