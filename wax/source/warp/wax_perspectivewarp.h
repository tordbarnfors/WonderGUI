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
#ifndef	WAX_PERSPECTIVEWARP_DOT_H
#define	WAX_PERSPECTIVEWARP_DOT_H
#pragma once

#include <wax_warpgenerator.h>

#include <optional>

namespace wax
{
	class PerspectiveWarp;
	typedef	StrongPtr<PerspectiveWarp>	PerspectiveWarp_p;
	typedef	WeakPtr<PerspectiveWarp>	PerspectiveWarp_wp;

	//____ PerspectiveWarp _____________________________________________________
	/**
	 * @brief Makes a Warpmap that places the source on a plane seen in perspective, such as a floor
	 * leaning into the distance or a card turning around an axis.
	 *
	 * The four corners of the source are placed at the given corners in the Warpmap, in spx from its
	 * top-left corner, and the source is drawn between them as a plane seen in perspective: the
	 * further away a part is, the smaller it gets. Any convex quadrilateral can be given, so the
	 * source can also be pinned onto a shape, such as a sign seen at an angle. Corners in the reverse
	 * order (counter clockwise) mirror the source, as when the back of a turning card faces the viewer.
	 * Parts of the plane outside the Warpmap are cut off.
	 *
	 * rotated() gives the corners of a rectangle on screen rotated in 3D, for the Blueprint or
	 * setCorners(). Move the source along the plane, such as for text scrolling into the distance,
	 * with a source offset when drawing rather than by generating again.
	 *
	 * The Warpmap is the quadrilateral of the corners, made about a pixel larger for antialiased
	 * borders and cut off by the borders of the Warpmap and just before the horizon of the plane,
	 * in a fan of triangles with weights for perspective: at most 9 vertices and 7 triangles. That
	 * is exact everywhere, so cell size and precision don't apply.
	 */

	class PerspectiveWarp : public WarpGenerator
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			CornersT<CoordSPX>	corners;						// Mandatory. Where the corners of the source are placed, spx from the top-left of the Warpmap.
			Finalizer_p			finalizer = nullptr;
			SizeI				size;							// Mandatory. Size of the Warpmap in pixels.
			SizeSPX				sourceSize;						// Mandatory. Size of the source (edgemap or source rectangle) in spx.
		};

		//.____ Creation __________________________________________

		static PerspectiveWarp_p	create( const Blueprint& blueprint, WarpmapFactory * pFactory = nullptr );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________________

		// Corners must make a convex quadrilateral with some area, otherwise they are refused.

		bool						setCorners( const CornersT<CoordSPX>& corners );
		const CornersT<CoordSPX>&	corners() const { return m_corners; }

		//.____ Misc ________________________________________________________

		// Corners of rect (spx) after rotating it in 3D around a point of it, then seen from an eye
		// viewDistance spx in front of the screen. The rotations are made in this order, all in
		// degrees around the pivot (relative to rect, { 0.5f, 0.5f } being its center):
		//
		//   roll	Within the plane of the screen, clockwise if positive.
		//   turn	Around the vertical axis, the right side moving away if positive.
		//   tilt	Around the horizontal axis, the top moving away if positive.
		//
		// The eye is straight in front of the pivot unless given (spx, in the same space as rect).
		// Several rectangles seen from the same eye share their vanishing points. A viewDistance of
		// 0 or less gives no perspective. Parts that come closer than the eye give corners that are
		// refused by setCorners(), as do rectangles seen edge on.

		static CornersT<CoordSPX>	rotated( const RectSPX& rect, float tilt, float turn, float roll, spx viewDistance,
										 CoordF pivot = { 0.5f, 0.5f }, std::optional<CoordSPX> eye = {} );

	protected:

		PerspectiveWarp( const Blueprint& bp, WarpmapFactory * pFactory );
		virtual ~PerspectiveWarp();

		static const int	c_maxVertices = 9;			// The quadrilateral clipped by five lines.
		static const int	c_maxTriangles = 7;

		// Source position (spx) at x,y (pixels) of the Warpmap is (u[0]*x + u[1]*y + u[2]) / w, v
		// likewise, w = w[0]*x + w[1]*y + w[2], positive on the plane.

		struct Projection
		{
			double		u[3];
			double		v[3];
			double		w[3];
		};

		static bool	_isValidQuad( const CornersT<CoordSPX>& corners );
		Projection	_projection() const;

		void		_meshSize( int& nVertices, int& nTriangles ) const override;
		void		_generateMesh( Warpmap::Vertex * pVertices, Warpmap::Triangle * pTriangles ) const override;

		CornersT<CoordSPX>	m_corners;
	};

} // namespace wax
#endif // WAX_PERSPECTIVEWARP_DOT_H
