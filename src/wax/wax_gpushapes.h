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
#ifndef	WAX_GPUSHAPES_DOT_H
#define	WAX_GPUSHAPES_DOT_H
#pragma once

#include <wax_gfxbackend.h>

namespace wax
{
	/**
	 * @brief Shapes (GfxBackend::Command::DrawShape) prepared for the shape shaders of GPU backends.
	 *
	 * A shape becomes entries of four floats, which the shader reads from the entry given with
	 * each vertex, and works out the coverage of each pixel from. Geometry is in canvas pixels.
	 *
	 *   0		kind (1 rounded box, 2 ring), flags, number of elements and entry of the first (ring)
	 *
	 *   Rounded box, flags: c_boxOutline.
	 *
	 *   1		outer edge: left, top, right, bottom
	 *   2		radii of outer corners: top-left, top-right, bottom-right, bottom-left, 0 for square
	 *   3		inner edge of the outline
	 *   4		radii of inner corners
	 *   5, 6	segments fill and outline
	 *
	 *   Ring, flags: c_ringSquare, c_ringBack, c_ringHole, c_ringOutline, c_ringSlices.
	 *
	 *   1		center x, y, radius 0 (outer edge), radius 1 (inside of outer outline)
	 *   2		radius 2 (outside of inner outline), radius 3 (inner edge), 0, 0
	 *   3		rect: left, top, right, bottom, the outer edge if square
	 *   4		rect inside the outer outline, if square
	 *   5...	segments back, outline, hub and the slices
	 *   ...	elements, the slices and the gap after them, in order of angle: start in turns
	 *			clockwise from 12 o'clock, entry of segment, 0, 0
	 *
	 * Rings are made of circles 0 to 3 as SoftShapes::Ring: without outline circle 1 is circle 0
	 * and circle 2 is circle 3, without hole circle 3 has radius 0. The outlines lie between
	 * circles 0 and 1 and between 2 and 3, the slices between 1 and 2, the hub inside 3. Without
	 * c_ringSlices there is no room for slices and everything from circle 0 to 3 is outline. The
	 * back segment covers rect outside circle 0 if c_ringBack, and the gap after the slices.
	 *
	 * The entries of segments are filled in by the backend: offset of color and tint block in its
	 * color buffer, -1 if not drawn or not tinted, 0, 0.
	 *
	 * Lines with caps (GfxBackend::Command::Line) are shapes too, of kind 3, see writeLine():
	 *
	 *   0		kind 3, cap of begin, cap of end, offset of color in color buffer (filled in by backend)
	 *   1		begin x, y, end x, y
	 *   2		direction x, y (of length 1), length, half the thickness
	 *
	 * Caps are LineCapShape, never None, plus 8 times the length of a spear in quarters of
	 * the thickness. A line of no length runs along x.
	 */

	namespace GpuShapes
	{
		const int	c_boxOutline	= 1;

		const int	c_ringSquare	= 1;
		const int	c_ringBack		= 2;
		const int	c_ringHole		= 4;
		const int	c_ringOutline	= 8;
		const int	c_ringSlices	= 16;

		struct Segment
		{
			int				entry;				// Entry to fill in.
			bool			bDrawn;
			HiColor			color;				// Straight sRGB, as given.
			const uint16_t*	pTint;				// Encoded tint, null if not tinted...
			const HiColor*	pTintColors;		// ...and its colors.
		};

		/**
		 * Most entries and number of segments of a shape. Supported shapes have at least one
		 * entry, others none.
		 */

		void	measure(GfxBackend::ShapeKind kind, const uint16_t* pPayload, int& maxEntries, int& nSegments);

		/**
		 * Most entries for a session, see GfxBackend::SessionInfo.
		 */

		inline int	maxSessionEntries(const GfxBackend::SessionInfo& info) { return info.nShapeDraws * 3 + info.nShapeSegments * 2; }

		/**
		 * Writes the entries of a shape and its segments, returns the number of entries.
		 * pColors are the colors of the command.
		 */

		int		write(GfxBackend::ShapeKind kind, const uint16_t* pPayload, const HiColor* pColors, float* pEntries, Segment* pSegments);

		const int	c_lineKind = 3;
		const int	c_lineEntries = 3;

		/**
		 * Writes the entries of a line with caps and the corners of a quad on the canvas, in
		 * pixels and in order around it, that holds every pixel it touches. Returns the number
		 * of entries, 0 if there is nothing to draw.
		 */

		int		writeLine(CoordSPX begin, CoordSPX end, spx thickness, LineCaps caps, float* pEntries, CoordF* pQuad);
	}
}

#endif //WAX_GPUSHAPES_DOT_H
