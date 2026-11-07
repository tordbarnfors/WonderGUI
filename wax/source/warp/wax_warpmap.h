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
#ifndef WAX_WARPMAP_DOT_H
#define WAX_WARPMAP_DOT_H
#pragma once

#include <wax_object.h>
#include <wax_types.h>
#include <wax_geo.h>

namespace wax
{
	class Warpmap;
	typedef	StrongPtr<Warpmap>	Warpmap_p;
	typedef	WeakPtr<Warpmap>	Warpmap_wp;

	//____ Warpmap _____________________________________________________________
	/**
	 * @brief Bends a surface or an edgemap into another shape when drawn.
	 *
	 * A Warpmap covers a rectangle of size.w * size.h pixels with a mesh of triangles. Each vertex
	 * has a position in the Warpmap (x, y in spx from its top-left corner), the position in the
	 * source that is drawn there (u along the width of the source and v along its height, in spx of
	 * the source: 64 per pixel of the surface or edgemap, relative to the source rectangle for blits)
	 * and a weight w for perspective. Positions within a triangle are interpolated from its corners.
	 * Where a position falls outside the source nothing is drawn, and nothing is drawn outside the
	 * triangles.
	 *
	 * The number of vertices and triangles is set when created, while vertices, triangles and
	 * periods can be changed. Triangles take their corners by index in the vertices. Triangles that
	 * aren't needed can be left with all corners the same, which draws nothing. Triangles may not
	 * overlap: what is drawn where they do is undefined. Triangles can be given clockwise or counter
	 * clockwise. A new Warpmap with at least 4 vertices and 2 triangles maps its rectangle straight
	 * onto the source, pixel for pixel, with the first 4 vertices and 2 triangles.
	 *
	 * Vertex positions are kept within +-maxPosition, u and v within +-maxValue and w within
	 * 1 -> maxWeight, with weightOne being 1.0. Only the ratios between the weights of the corners
	 * of a triangle matter. They are the same (any value) for a triangle without perspective, and
	 * proportional to 1 / distance from the eye for a triangle seen in perspective. A vertex shared
	 * by triangles has the same weight in all of them, so a mesh is in perspective as a whole.
	 *
	 * Values of u and v are not limited to the source. Values outside it are what makes parts of a
	 * triangle transparent, with the border antialiased where the interpolated position crosses the
	 * border of the source. Since nothing is drawn outside the triangles, let the triangles reach
	 * about a pixel beyond the parts that are drawn, with u and v going on outside the source,
	 * otherwise borders along the edge of the mesh are cut off without antialiasing. Edges between
	 * triangles are not antialiased and don't show, as long as neighbouring triangles share the
	 * vertices along their common edge.
	 *
	 * A Warpmap is made for a source of a certain size. A source of another size is not stretched to
	 * fit, the vertices have to be changed.
	 *
	 * An axis can be periodic (uPeriod/vPeriod > 0, in spx of the source), as when u is the angle of
	 * a ring. Positions are then read modulo the period, after interpolation, so vertices of a
	 * triangle keep values that are close to each other also where they go past the period. With a
	 * period of the size of the source it wraps around, which for an edgemap is seamless if it is
	 * tiling. A longer period leaves a gap, such as a 270 degree arc of a source w spx wide having
	 * uPeriod = w * 360 / 270.
	 *
	 * When drawn, a source offset (in spx of the source) is added to the position before reading,
	 * so a ring can be rotated or a pattern scrolled without changing the Warpmap.
	 *
	 * There is no supersampling when the Warpmap shrinks a surface. Use a mipmapped surface for that,
	 * on backends that support mipmaps.
	 *
	 * Warpmaps are created by a WarpmapFactory, which is specific to the backend, or by a generator
	 * such as CircularWarp.
	 *
	 * Draws are rendered when the canvas update ends, and read the Warpmap then. Changing the content
	 * of a Warpmap changes all of its draws that aren't rendered yet, so use a Warpmap for each shape
	 * drawn within the same canvas update. Generators take care of that.
	 *
	 * How a Warpmap is drawn, which all backends follow:
	 *
	 * - The Warpmap is placed on whole pixels (dest is rounded to nearest pixel), like edgemaps.
	 *   Nothing is drawn outside of its rectangle.
	 *
	 * - A pixel is drawn by the triangle that its center (32 spx into the pixel along x and y) is
	 *   inside of, see triangleAt(). A center exactly on an edge belongs to the triangle that has it
	 *   as a left edge (the triangle is to the right of it) or as a horizontal top edge (the triangle
	 *   is below it), so that no pixel is drawn twice by triangles that share an edge. Pixels with
	 *   their center outside all triangles aren't drawn.
	 *
	 * - The position at a point (x,y) of the Warpmap is interpolated from the corners of the triangle
	 *   that draws the pixel, also for points of the pixel outside the triangle, see positionIn().
	 *   With b0, b1 and b2 being the barycentric coordinates of the point in the triangle:
	 *
	 *		u = (b0*w0*u0 + b1*w1*u1 + b2*w2*u2) / (b0*w0 + b1*w1 + b2*w2)
	 *
	 *   and the same for v. Where the divisor is 0 or less, which is beyond the horizon of a triangle
	 *   in perspective, the point is outside the source, and a pixel with its center there isn't
	 *   drawn. Adding the source offset gives the position in the source. A periodic axis is then
	 *   wrapped into 0 -> period.
	 *
	 * - The part of a pixel that is covered is the part of its area whose position is inside the
	 *   source: 0 -> width and 0 -> height. For an edgemap, the part covered by each segment is the
	 *   part whose position is in that segment. Coverage is then applied as for any anti-aliased edge,
	 *   see "Blending and transparency" in the guide. ReferenceBackend measures coverage by sampling
	 *   the pixel at many points, other backends estimate it from the distance to the border or edge
	 *   and how fast the position changes over the pixel.
	 *
	 * - Colors are taken at the position of the center of the pixel: the surface is sampled there
	 *   by its sample method, as if the edge pixels of the source rectangle continued outside it
	 *   (like stretchBlit() reads it), or wrapping around the surface if it is tiling. The tints of
	 *   edgemap segments are evaluated there (in pixels of the edgemap). A center in the gap between
	 *   copies on a periodic axis is taken in the nearest copy, before its start or after its end, so
	 *   colors don't jump where copies meet. The device tint is applied in canvas space as usual.
	 *
	 * - Backends other than ReferenceBackend may divide for perspective only once every few pixels
	 *   along a row, up to 16, interpolating linearly in between. They take shorter runs close to the
	 *   horizon, keeping within about a quarter of a pixel of the positions above.
	 */

	class Warpmap : public Object
	{
		friend class WarpGenerator;							// Writes content in place when generating.

	public:

		struct Blueprint
		{
			Finalizer_p		finalizer	= nullptr;
			SizeI			size;							// Mandatory. Size in pixels.
			int				triangles	= 2;				// Number of triangles, 1 -> maxTriangles.
			spx				uPeriod		= 0;				// In spx of the source, 0 = not periodic.
			int				vertices	= 4;				// Number of vertices, 3 -> maxVertices.
			spx				vPeriod		= 0;				// In spx of the source, 0 = not periodic.
		};

		struct Vertex
		{
			spx				x, y;							// Position in spx of the Warpmap, from its top-left corner.
			spx				u, v;							// Position in spx of the source.
			int				w = weightOne;					// Weight for perspective, 1 -> maxWeight.
		};

		struct Triangle
		{
			int				a, b, c;						// Indices of the corners in the vertices.

			int				operator[]( int i ) const { return i == 0 ? a : (i == 1 ? b : c); }
		};

		static constexpr spx	maxPosition = 1 << 26;			// Vertex positions are kept within +-maxPosition (1 million pixels).
		static constexpr spx	maxValue = 1 << 30;				// u, v and periods are kept within +-maxValue (16 million pixels).
		static constexpr int	weightOne = 1 << 16;			// A weight of 1.0.
		static constexpr int	maxWeight = 1 << 24;			// Weights are kept within 1 -> maxWeight.
		static constexpr int	maxVertices = 1 << 20;
		static constexpr int	maxTriangles = 1 << 21;

		//.____ Identification __________________________________________

		const TypeInfo&     typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

		//.____ Geometry ______________________________________________________

		inline SizeI	pixelSize() const { return m_size; }

		//.____ Content _______________________________________________________

		inline int		vertexCount() const { return m_nVertices; }
		inline int		triangleCount() const { return m_nTriangles; }

		inline spx		uPeriod() const { return m_uPeriod; }
		inline spx		vPeriod() const { return m_vPeriod; }

		bool			setPeriods( spx uPeriod, spx vPeriod );

		// Values out of range are clamped. Triangles with corners that aren't vertices of the Warpmap
		// fail the import, which then changes nothing.

		bool			importVertices( int first, int count, const Vertex * pSource );
		bool			importTriangles( int first, int count, const Triangle * pSource );

		// All vertices and triangles, and periods, at once. Either of pVertices and pTriangles can be
		// null to keep what is there.

		bool			importMesh( const Vertex * pVertices, const Triangle * pTriangles, spx uPeriod, spx vPeriod );

		bool			exportVertices( int first, int count, Vertex * pDestination ) const;
		bool			exportTriangles( int first, int count, Triangle * pDestination ) const;

		inline const Vertex*	vertexData() const { return m_pVertices; }
		inline const Triangle*	triangleData() const { return m_pTriangles; }

		//.____ Misc __________________________________________________________

		// The triangle that a point (in spx of the Warpmap) is inside of, by the rule for pixel centers
		// on edges, or -1 if none. Searches all triangles. Overlapping triangles give any of them.

		int				triangleAt( CoordSPX point ) const;

		// If a point (in spx of the Warpmap) is inside a triangle, by the rule for pixel centers on edges.

		bool			isInside( int triangle, CoordSPX point ) const;

		// Interpolated position at point x,y (in pixels) of the Warpmap, as given by a triangle, also
		// outside of it. In spx of the source with fractions, not wrapped into the period. False where
		// the point is beyond the horizon of the triangle, or the triangle has no area.

		bool			positionIn( int triangle, double x, double y, CoordF& position ) const;

	protected:
		Warpmap() {};

		// Content is taken from pVertices and pTriangles if given, otherwise it maps the rectangle
		// straight onto the source as described above. Subclasses send it on with _updated() when
		// constructed, so creating with content costs a single upload.

		Warpmap( const Blueprint& bp, const Vertex * pVertices = nullptr, const Triangle * pTriangles = nullptr );
		virtual ~Warpmap();

		// Checks the blueprint, and if given that the triangles have corners that are vertices.

		static bool		_validateBlueprint( const Blueprint& bp, const Triangle * pTriangles = nullptr );

		static Vertex	_clamped( const Vertex& vertex );
		static bool		_isValid( const Triangle * pTriangles, int count, int nVertices );

		// Called when content has changed: the vertices and triangles given (count 0 for none), and
		// possibly the periods. Typically all you need to override when subclassing.

		virtual void	_updated( int firstVertex, int nVertices, int firstTriangle, int nTriangles ) = 0;

		SizeI			m_size;
		int				m_nVertices = 0;
		int				m_nTriangles = 0;
		spx				m_uPeriod = 0;
		spx				m_vPeriod = 0;

		Vertex*			m_pVertices = nullptr;
		Triangle*		m_pTriangles = nullptr;
	};

} // namespace wax
#endif // WAX_WARPMAP_DOT_H
