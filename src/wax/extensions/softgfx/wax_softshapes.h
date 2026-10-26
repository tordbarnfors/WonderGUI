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
#ifndef	WG_SOFTSHAPES_DOT_H
#define	WG_SOFTSHAPES_DOT_H
#pragma once

#include <wg_geo.h>
#include <wg_gfxtypes.h>
#include <wg_gfxdevice.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace wg
{
	//____ SoftShapes _________________________________________________________
	/**
	 * @brief Rasterizes analytic shapes row by row for the software backends.
	 *
	 * Shapes are made of segments (fill, outline, slices...) that are drawn in one
	 * go, so that segments sharing an edge never let the background shine through.
	 *
	 * Each pixel row is delivered to a RowSink as runs of pixels fully covered by one
	 * segment, drawn with the fill kernels, and runs of partly covered pixels along the
	 * edges, with the colors of the segments covering them already mixed, drawn with the
	 * coverage kernels.
	 *
	 * Coverage is 0 -> 4096. Shapes are placed with subpixel (spx) precision.
	 *
	 * Coverage model:
	 *
	 *	- A pixel row is split into sub-rows where edges begin or end, like the top of
	 *	  a circle or the end of a slice boundary at the ring.
	 *	- Within a sub-row, edges cross it from top to bottom in order along the row,
	 *	  without crossing each other. Each is a polyline, split where it bends more
	 *	  than 1/64 pixel from a straight line or kinks. Circles become chords between
	 *	  points on the circle.
	 *	- The coverage of a pixel by what lies right of an edge is the exact area of
	 *	  the pixel on that side, so axis-aligned edges and slice boundaries are exact
	 *	  and circles are within 1/64 pixel.
	 *	- The segment between two edges gets the coverage right of the first minus that
	 *	  right of the second, so segments always add up to exactly the coverage of the
	 *	  shape. Sub-rows are weighted by their height and rounded cumulatively.
	 *
	 * Everything is integer math, so that it runs well on CPUs without FPU. Divisions
	 * are few: one for each piece of an edge spanning more than 16 pixels along a row,
	 * none per pixel.
	 */

	class SoftShapes
	{
	public:

		static constexpr int c_fullCoverage = 4096;

		//____ CoveragePixel ___________________________________________________
		/**
		 * A pixel only partly covered by a shape, for coverage kernels.
		 *
		 * Partly covered pixels are blended with the blend mode as if fully covered and
		 * the result is then mixed with the original pixel by the coverage, so that edges
		 * are blended against the background whatever the blend mode:
		 *
		 *   out = dst * (1 - coverage) + blend(src, dst) * coverage
		 *
		 * Several segments of a shape can share a pixel, so the color is a sum of the
		 * colors of the segments, each weighted by its coverage. That is exact, except
		 * for blend modes that saturate or select (Add, Subtract, Min, Max) when segments
		 * share a pixel. The colors of the segments are given by RowSink::segmentColors(),
		 * which decides how they are weighted, for example premultiplied by alpha:
		 *
		 *   b,g,r,a = sum( coverage * color )
		 *   coverage = sum( coverage )
		 *
		 * All values are linear, 0 -> 4096.
		 */

		struct CoveragePixel
		{
			int16_t		b;
			int16_t		g;
			int16_t		r;
			int16_t		a;
			int16_t		coverage;
			int16_t		padding;
		};

		//____ Run ____________________________________________________________

		struct Run
		{
			int			begin;			// First pixel column.
			int			end;			// Column after the last pixel.
			int			segment;		// Segment fully covering the pixels, or -1 for partly covered pixels.
		};

		//____ RowSink ________________________________________________________

		class RowSink
		{
		public:
			/**
			 * Colors of a segment for mixing partly covered pixels, see CoveragePixel: b, g, r
			 * and a for each pixel of row y from column begin to end, or the same for all pixels
			 * and rows if bFlat is set, in which case only the first four values are read.
			 * Returns null if the segment isn't drawn, which must not change from row to row.
			 * The colors are only read before the next call for the same segment.
			 */
			virtual const int16_t*	segmentColors(int segment, int y, int begin, int end, bool& bFlat) = 0;

			/**
			 * Receives the runs of one row, in increasing order, not overlapping and within
			 * the clip rectangle. Pixels in no run are not covered by any drawn segment.
			 * Partly covered pixels are in pPixels, one after another for all runs of them.
			 */
			virtual void	row(int y, int nRuns, const Run* pRuns, const CoveragePixel* pPixels) = 0;

			/**
			 * Receives runs that are the same for rows yBeg up to yEnd, like the straight
			 * sides of a box. Only given when all segments are flat. Handled one row at a
			 * time unless overridden.
			 */
			virtual void	rows(int yBeg, int yEnd, int nRuns, const Run* pRuns, const CoveragePixel* pPixels)
			{
				for (int y = yBeg; y < yEnd; y++)
					row(y, nRuns, pRuns, pPixels);
			}
		};

		//____ RoundedBox _____________________________________________________

		struct RoundedBox
		{
			RectSPX		rect;							// Outer edge, the outline lies inside.
			spx			radius[4] = { 0, 0, 0, 0 };		// Top-left, top-right, bottom-right, bottom-left. Clamped as by Util::clampCornerRadii().
			BorderSPX	outline;						// Thickness of outline along each side.
		};

		//.____ Rasterization _____________________________________________________

		/**
		 * Rasterizes a box with rounded corners into rows within clip (pixels).
		 *
		 * Two segments: 0 is the fill (inside the outline) and 1 the outline. The inner
		 * edge of the outline has corners with radius r - max(width of the two sides),
		 * circular and touching both sides, which equals CSS when the widths are equal.
		 */

		void	rasterizeRoundedBox(const RoundedBox& box, const RectI& clip, RowSink* pSink);

		//____ Ring ___________________________________________________________

		static constexpr uint32_t c_fullTurn = 0x80000000;	// Angles and slice sizes of Ring.

		struct Ring
		{
			RectSPX			rect;
			spx				thickness = 0;			// Width of ring from outer edge, outlines included. 0 or at least the radius for a full disc.
			spx				outline = 0;			// Outline along outer and inner edge.
			bool			bSquare = false;		// Outer edge is rect, not the largest circle centered in it.
			uint32_t		start = 0;				// Start of first slice, clockwise from 12 o'clock. c_fullTurn is a full turn.
			int				nSlices = 0;
			const uint32_t*	pSliceSizes = nullptr;	// Size of each slice, c_fullTurn is a full turn. Slices beyond a full turn are cut.
			bool			bBack = true;			// Rest of rect is covered by the back segment. Faster without, when back isn't drawn.
		};

		static constexpr int	c_ringBackSegment = 0;			// Rest of rect, and the part of the ring the slices don't reach.
		static constexpr int	c_ringOutlineSegment = 1;
		static constexpr int	c_ringHubSegment = 2;			// Inside the ring.
		static constexpr int	c_ringFirstSliceSegment = 3;	// Followed by the other slices.

		/**
		 * Rasterizes a ring, pie or disc into rows within clip (pixels).
		 *
		 * The circle is the largest centered in rect, or rect itself if bSquare. Slices are
		 * split along straight lines from the center, so neighboring slices always add up
		 * to the coverage of the ring.
		 */

		void	rasterizeRing(const Ring& ring, const RectI& clip, RowSink* pSink);

		//____ Line ___________________________________________________________

		struct Line
		{
			CoordSPX	begin;
			CoordSPX	end;
			spx			thickness = 64;
			LineCaps	caps;						// Not both LineCap::None, see LineCap.
		};

		/**
		 * How a line with caps is drawn, see prepareLine().
		 *
		 * Between its ends a line covers whole columns (rows if the line runs mostly along y)
		 * across its band, which can be drawn by the line kernels. Each column from bodyBeg up
		 * to bodyEnd is covered from pos to pos + bodyWidth across the line, in 16.16 pixels,
		 * where pos is bodyPos at the middle of column bodyBeg and changes by bodySlope from
		 * one column to the next (bodySlope24 with 24 binals, for placing columns further on).
		 *
		 * The rest, the parts around the ends or the whole line if it has no body, are drawn
		 * by rasterizeLinePart(), each within its columns from partBeg to partEnd.
		 */

		struct LineParts
		{
			bool		bAlongX = true;				// Columns are pixel columns, otherwise pixel rows.
			int			bodyBeg = 0;
			int			bodyEnd = 0;				// bodyBeg if no body.
			int64_t		bodyPos = 0;
			int			bodySlope = 0;
			int64_t		bodySlope24 = 0;
			int			bodyWidth = 0;
			int			nParts = 0;
			int			partBeg[2] = { 0, 0 };
			int			partEnd[2] = { 0, 0 };
		};

		/**
		 * Prepares a line with caps for drawing by line kernels and rasterizeLinePart().
		 * The geometry is as by ReferenceBackend, cap regions reaching more than 65536 pixels
		 * beyond the ends are cut there. Returns false if there is nothing to draw.
		 */

		bool	prepareLine(const Line& line, LineParts& parts);

		/**
		 * Rasterizes a part of the line from prepareLine(), into rows within clip (pixels)
		 * and the columns of the part. One segment, 0.
		 */

		void	rasterizeLinePart(int part, const RectI& clip, RowSink* pSink);

		//.____ Buffers ___________________________________________________________

		/**
		 * Sets the size of the buffer for rows of shapes symmetric upside down, kept to
		 * draw their lower half mirrored. Allocated to initialBytes right away, and later
		 * as needed up to maxBytes. Rows that don't fit are rasterized again instead, which
		 * gives the same pixels, only slower. Setting both the same allocates it once.
		 *
		 * Rows of a 2000 pixel high disc take about 100 KB, about twice that with an outline,
		 * and those of a button about 1 KB.
		 */

		void	setMirrorBuffer(int initialBytes, int maxBytes);

		static constexpr int	c_defaultMaxMirrorBuffer = 256 * 1024;

	protected:

		static constexpr int	c_maxSagitta = 1;			// Most a chord may stray from its circle, spx.
		static constexpr int	c_maxVertices = 64;			// Most vertices of an edge in a sub-row, below 64 spx high.
		static constexpr int	c_maxFlatRadius = 1 << 16;	// Largest corner flattened in advance, spx.

		// Edge flattened in advance, with vertices in order of increasing y.

		struct Polyline
		{
			const int64_t*	pY;
			const int64_t*	pX;
			int				n;
			int				line;			// Line last used, from vertex line to line + 1.
		};

		// Edge of a shape crossing a sub-row, as given by the shape.

		struct EdgeDef
		{
			int			id;					// Which of the edges of the shape, see EdgeShape.
			int			segment;			// Segment right of the edge, -1 for none.
			bool		bClamped;			// Straight edge kept between its neighbors where rounding makes them cross.
			int			mirror;				// Edge before it in the sub-row it mirrors, x = mirrorSum - x of that edge, or -1.
			int64_t		mirrorSum;
			Polyline*	pPolyline;			// Edge as polyline, instead of from x(), or null.
		};

		// Straight piece of an edge, part of a polyline from the top to the bottom of a sub-row.

		struct Piece
		{
			int64_t		lo, hi;				// Range of x, spx.
			int64_t		inv;				// 2^37 / (hi - lo), if hi > lo and the piece is within the clip rectangle.
			int			h;					// Height, spx.
			int			bandBeg, bandEnd;	// Pixels partly right of the piece, clamped to the clip rectangle +/- 1.
		};

		struct Edge
		{
			int			segment;
			int			first, n;			// Pieces.
			int			bandBeg, bandEnd;	// Pixels partly right of the edge.
		};

		struct SubRow
		{
			int			h;					// Height, spx.
			int			first, n;			// Edges.
			int			next;				// Next edge not passed while sweeping...
			int			segment;			// ...and the segment before it.
		};

		struct MixSource
		{
			const int16_t*	p;				// Colors of segment from RowSink::segmentColors(), null if not drawn.
			bool			bFlat;			// Same for all pixels, otherwise refetched each row.
			int				row;
		};

		class EdgeShape;
		class RingShape;
		class BoxShape;
		class PolygonShape;

		template<class Shape>
		void	_rasterize(Shape& shape, int nSegments, int rowBeg, int rowEnd, int clipBeg, int clipEnd, RowSink* pSink);

		template<class Shape>
		void	_rasterizeRow(Shape& shape, int row);

		template<class Shape>
		int		_subRowEdges(Shape& shape, int64_t ya, int64_t yb, Edge* pEdges);
		void	_simpleRow(const Edge* pEdges, int nEdges, int h);
		void	_generalRow(int nSubRows);
		void	_emitAccumulated(int beg, int end);

		template<class Shape>
		inline int64_t			_edgeX(Shape& shape, const EdgeDef& edge, int64_t y);

		inline int64_t			_edgeLeftOf(const Edge& edge, int64_t x) const;

		static inline int64_t	_polylineX(Polyline& polyline, int64_t y);

		struct Quarter
		{
			int64_t					r = -1;
			std::vector<int64_t>	dx, dy;
		};

		const Quarter&			_quarter(int64_t r);

		static inline int64_t	_area(const Piece& piece, int64_t x);

		inline const int16_t*	_mixColor(int segment, int x);
		inline void				_fillRun(int beg, int end, int segment);
		inline CoveragePixel&	_mixedPixel(int x);
		inline void				_mixedRun(int beg, int end, const CoveragePixel& pixel);
		inline void				_accumulate(int segment, int value);
		void					_mirrorRunsX();

		// Ring

		// Angles of boundaries are 2^32 to a turn, twice the scale of Ring, so that they
		// wrap around by themselves.

		struct Boundary
		{
			uint32_t	angle;				// Clockwise from 12 o'clock, 2^32 is a full turn.
			int32_t		sx, sy;				// Direction from center, 2.30 fixed point, y pointing down.
			bool		bFlat;				// Too close to horizontal to cross any row but the ones at the center.
			int64_t		slopeInt;			// sx / sy, integer part (rounded down)...
			uint32_t	slopeFrac;			// ...and 32 binals.
			int			element;			// Element (slice or gap) starting here.
			int			prevElement;		// Element ending here.
		};

		struct Ray							// Boundary between elements, as crossing rows above or below the center.
		{
			int64_t		yOuter;				// Where the ray leaves the area of the slices...
			int64_t		yInner;				// ...and enters the hole, or the center.
			int64_t		slopeInt;
			uint32_t	slopeFrac;
			int			segment;			// Segment right of the ray along a row.
			bool		bLeft;				// Left of the center.
		};

		// Line

		static constexpr int		c_lineUnitShift = 4;	// Lines are worked out in 1/16 spx.
		static constexpr int64_t	c_maxCapReach = int64_t(1) << 26;	// 65536 pixels in 1/16 spx.

		struct ProfilePoint
		{
			int64_t		w;					// Across the line.
			int64_t		a;					// Along the line, where the line begins at w.
		};

		struct LinePart
		{
			int						columnBeg, columnEnd;			// Columns (rows if not along x) to draw.
			int64_t					top, bottom, left, right;		// Bounds, spx.
			std::vector<int64_t>	leftY, leftX;					// Left and right edge from top to bottom.
			std::vector<int64_t>	rightY, rightX;
		};

		void	_capProfile(LineCap cap, int64_t r, int64_t dirX, int64_t dirY, std::vector<ProfilePoint>& profile);
		bool	_setLinePart(LinePart& part, std::vector<int64_t>& vx, std::vector<int64_t>& vy);

		std::vector<ProfilePoint>	m_profiles[2];
		std::vector<int64_t>		m_vertexX;
		std::vector<int64_t>		m_vertexY;
		LinePart					m_lineParts[2];
		bool						m_bLineAlongX = true;

		void	_setupRingAngles(const Ring& ring);
		int		_elementAt(int64_t x, int64_t y) const;

		static inline bool	_isBefore(int64_t ax, int64_t ay, int64_t bx, int64_t by);
		static int32_t	_sin30(uint32_t angle);

		std::vector<Boundary>		m_boundaries;		// Sorted by angle.
		std::vector<int>			m_elementSegments;	// Segment of each element.
		std::vector<uint64_t>		m_elementStarts;
		std::vector<int>			m_crossings[2];		// Boundaries crossing rows above and below center, in order along x...
		std::vector<Ray>			m_rays[2];			// ...as rays.

		// Rasterization

		std::vector<int64_t>		m_splits;			// Where sub-rows begin and end, sorted.
		std::vector<SubRow>			m_subRows;
		std::vector<EdgeDef>		m_edgeDefs;
		std::vector<Edge>			m_edges;
		std::vector<Piece>			m_pieces;
		std::vector<int64_t>		m_edgeTop;			// x of each edge of a sub-row at top and bottom.
		std::vector<int64_t>		m_edgeBottom;
		std::vector<int>			m_edgeTopLine;		// Line of polyline at top of sub-row.
		std::vector<int64_t>		m_cacheY;			// Last x computed for the bottom of a sub-row, for each edge id.
		std::vector<int64_t>		m_cacheX;
		std::vector<int64_t>		m_edgeLeftOf;		// Of edges of a sub-row at a pixel boundary.
		std::vector<int64_t>		m_polylineY;		// Vertices of polylines of a shape.
		std::vector<int64_t>		m_polylineX;

		static constexpr int		c_quarters = 4;
		Quarter						m_quarters[c_quarters];		// Recently used, see _quarter().
		int							m_nextQuarter = 0;

		std::vector<MixSource>		m_mix;
		std::vector<int>			m_accumulated;		// Coverage of each segment in a pixel, weighted by height of sub-rows.
		std::vector<int>			m_touched;			// Segments with coverage accumulated.
		std::vector<int>			m_pixelSegments;	// Segments covering a pixel and their coverage, see _emitAccumulated().
		std::vector<int>			m_pixelCoverage;
		std::vector<Run>			m_runs;
		std::vector<CoveragePixel>	m_pixels;

		// Mirror buffer, rows of the upper half kept to be mirrored into the lower half, see
		// _rasterize(). Holds the offset of each kept row by row, -1 if not kept, followed by
		// the rows: each a KeptRow, its runs and its pixels.

		struct KeptRow
		{
			int		nRuns;
			int		nPixels;
		};

		bool	_reserveMirrorBuffer(int bytes);

		std::unique_ptr<uint8_t[]>	m_pMirrorBuffer;
		int							m_mirrorBufferSize = 0;			// Bytes allocated...
		int							m_mirrorBufferMax = c_defaultMaxMirrorBuffer;
		int							m_mirrorBufferUsed = 0;			// ...and used.

		RowSink*		m_pSink = nullptr;
		int				m_clipBeg = 0;
		int				m_clipEnd = 0;
		int				m_row = 0;
		int				m_nRuns = 0;
		int				m_nPixels = 0;
		int				m_splitHint = 0;		// Split after the last row done.
		int				m_firstRun = 0;			// Of runs of row to emit, the ones before done for their mirror image.
		int				m_firstPixel = 0;

		bool			m_bMirrorX = false;		// Rows done up to the middle and mirrored, see _rasterize().
		int				m_mirrorSumX = 0;
		int				m_outBeg = 0;			// Pixels of row to emit.
		int				m_outEnd = 0;
		int				m_nTouched = 0;
		int				m_nPieces = 0;
	};

}

#endif //WG_SOFTSHAPES_DOT_H
