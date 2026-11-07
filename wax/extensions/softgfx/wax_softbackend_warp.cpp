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

/*
	Drawing through Warpmaps (Command::WarpBlit and Command::WarpDrawEdgemap) for SoftBackend
	and LinearBackend, following the definition in Warpmap.

	Triangles are set up when the content of the Warpmap changes, see SoftWarpmap::Setup, and
	drawn row by row: for each row of pixels, the pixels of each triangle on it are found by
	stepping the edges of the triangle from the row above, exactly and without divisions, by the
	rule for pixel centers on edges in Warpmap. They make spans of pixels along the row, with
	positions that are linear along the span, classified from the bounds of their positions:

	- Outside: no part of the span reaches into the source. Nothing is drawn.

	- Solid: the whole span is inside the source and, for an edgemap, inside one segment. Flat
	  colored segments are drawn as runs with the fill kernels, like shapes. Tinted segments and
	  blits get colors for each pixel, from the position of its center.

	- Mixed: coverage is estimated for each pixel from the position of its center and how fast the
	  position changes over the pixel (the derivatives of the mapping of the triangle). Each border
	  (of the source and of each edge) is taken as straight over the pixel, which gives the exact
	  area on each side of it without supersampling, see halfPlaneCoverage(). Coverage of the
	  segments between edges is the difference between the areas below them.

	Partly covered pixels and pixels with colors of their own are drawn with the coverage kernels,
	which blend by the rules in "Blending and transparency".

	Without perspective, positions are linear over a triangle, so a span takes them from the setup
	of the triangle and steps them along the row, and triangles all outside the source or all in
	one segment are classified once. With perspective, positions and their derivatives are divided
	out at most every 16 pixels and are linear in between, see _addWarpSpans(). Positions are int64
	to cope with any Warpmap, but only added and compared per pixel, except where coverage is
	estimated.
*/

#include <wax_softbackend.h>
#include <wax_softedgemap.h>
#include <wax_softwarpmap.h>
#include <wax_base.h>
#include <wax_util.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>

namespace wax
{
	namespace
	{
		const int64_t	c_one = 65536;					// 1.0 with 16 binals.
		const int64_t	c_maxPos = int64_t(1) << 40;	// Positions are kept within +-2^24 pixels.
		const int64_t	c_maxGradient = int64_t(1) << 31;	// Derivatives limited to 2^15 source pixels per pixel for coverage.
		const int		c_runShift = 0;					// Perspective runs keep within 1/4 >> c_runShift of a pixel,
		const int		c_mixedRunShift = 4;			// and those with borders or edges in them within 1/4 >> c_mixedRunShift.
		const int		c_maxSolidColumns = 64;			// Spans reaching over more edgemap columns are drawn as mixed.

		inline int64_t abs64(int64_t v) { return v < 0 ? -v : v; }

		// Division rounded towards minus infinity, b > 0.

		inline int64_t floorDiv(int64_t a, int64_t b)
		{
			int64_t q = a / b;
			return (q * b > a) ? q - 1 : q;
		}

		inline int64_t ceilDiv(int64_t a, int64_t b)
		{
			return -floorDiv(-a, b);
		}

		// a * num / den rounded down, without the overflow of a * num, 0 <= num <= den.

		inline int64_t mulDiv(int64_t a, int64_t num, int64_t den)
		{
			int64_t q = floorDiv(a, den);
			int64_t r = a - q * den;
			return q * num + (r * num) / den;
		}

		inline int64_t wrap(int64_t v, int64_t period)
		{
			if (period > 0 && (v < 0 || v >= period))
				v -= floorDiv(v, period) * period;
			return v;
		}

		inline int64_t toBinal(int64_t v)
		{
			if constexpr (BINAL_SHIFT >= 16)
				return v << (BINAL_SHIFT - 16);
			else
				return v >> (16 - BINAL_SHIFT);
		}

		//____ halfPlaneCoverage() ____________________________________________
		/*
			Part (0 -> 4096) of a pixel where f + gx * dx + gy * dy >= 0, dx and dy going from -0.5 to 0.5
			over the pixel. That is the area of a square on one side of a straight line, which depends on
			f and the larger (a) and smaller (b) of |gx| and |gy|. Over the pixel the value goes from
			f - (a + b) / 2 to f + (a + b) / 2. Within (a - b) / 2 of the center the area grows linearly,
			further out it is a triangle in the corner of the pixel.
		*/

		int halfPlaneCoverage(int64_t f, int64_t gx, int64_t gy)
		{
			int64_t a = abs64(gx);
			int64_t b = abs64(gy);
			if (a < b)
				std::swap(a, b);

			int64_t sum = a + b;

			if (2 * f >= sum)
				return 4096;
			if (2 * f <= -sum)
				return 0;

			// |f| < a here. Keep 16 bits of a, more than the 12 bits of the result need, so that
			// the rest fits in 32 bits and takes 32 bit divisions only. 32 bit CPUs divide those
			// in hardware or a short routine, 64 bit ones in a much longer one.

			while (a >= (int64_t(1) << 24))
			{
				a >>= 8;
				b >>= 8;
				f >>= 8;
			}

			while (a >= (int64_t(1) << 16))
			{
				a >>= 1;
				b >>= 1;
				f >>= 1;
			}

			int32_t a32 = int32_t(a);
			int32_t b32 = int32_t(b);
			int32_t f32 = int32_t(f);
			int32_t absF = f32 < 0 ? -f32 : f32;

			if (b32 == 0 || 2 * absF <= a32 - b32)
				return std::clamp(2048 + (f32 * 4096) / a32, 0, 4096);

			// The corner triangle has the area t^2 / (8ab), t being twice its height, up to 2b, which
			// rounding above can take it just outside of. t / a and t / b, at most 2, are taken with
			// 14 binals, so they multiply within 32 bits.

			uint32_t t = uint32_t(std::clamp(a32 + b32 - 2 * absF, 0, 2 * b32));
			uint32_t ta = (t << 14) / uint32_t(a32);
			uint32_t tb = (t << 14) / uint32_t(b32);
			int corner = int((ta * tb) >> 19);

			return f32 > 0 ? 4096 - corner : corner;
		}

		//____ forWrappedSpans() ______________________________________________
		/*
			Splits a span of positions (x,y), (x+dx, y+dy) ... into parts where positions wrapped into
			the periods stay linear, calling func(begin, length, x, y) with the wrapped start of each.
			Positions are wrapped into low -> low + period.
		*/

		template<class Func>
		void forWrappedSpans(const int64_t period[2], const int64_t low[2], int64_t x, int64_t y, int64_t dx, int64_t dy, int length, Func&& func)
		{
			int i = 0;
			while (i < length)
			{
				int64_t pos[2] = { x + dx * i, y + dy * i };
				int64_t step[2] = { dx, dy };
				int n = length - i;

				for (int a = 0; a < 2; a++)
				{
					int64_t p = period[a];
					if (p <= 0)
						continue;

					pos[a] = low[a] + wrap(pos[a] - low[a], p);

					int64_t last = pos[a] + step[a] * (n - 1);
					if (last >= low[a] && last < low[a] + p)
						continue;										// All of it within the period, the common case.

					int64_t m = n;
					if (step[a] > 0)
						m = ceilDiv(low[a] + p - pos[a], step[a]);
					else if (step[a] < 0)
						m = floorDiv(pos[a] - low[a], -step[a]) + 1;

					n = int(std::max(int64_t(1), std::min(int64_t(n), m)));
				}

				func(i, n, pos[0], pos[1]);
				i += n;
			}
		}

		//____ linearRange() __________________________________________________
		// Range of i (begin -> end) within 0 -> length where lo <= s + d * i <= hi.

		void linearRange(int64_t s, int64_t d, int64_t lo, int64_t hi, int length, int& begin, int& end)
		{
			int64_t last = s + d * (length - 1);
			if (s >= lo && s <= hi && last >= lo && last <= hi)
			{
				begin = 0;												// All of it within, the common case.
				end = length;
				return;
			}

			int64_t b, e;

			if (d > 0)
			{
				b = ceilDiv(lo - s, d);
				e = floorDiv(hi - s, d) + 1;
			}
			else if (d < 0)
			{
				b = ceilDiv(s - hi, -d);
				e = floorDiv(s - lo, -d) + 1;
			}
			else
			{
				b = 0;
				e = (s >= lo && s <= hi) ? length : 0;
			}

			begin = int(std::clamp(b, int64_t(0), int64_t(length)));
			end = int(std::clamp(e, int64_t(begin), int64_t(length)));
		}
	}

	//____ _beginWarp() _________________________________________________________
	/*
		Prepares drawing of a WarpBlit (pEdgemap is null) or WarpDrawEdgemap. Returns false if
		there is nothing to draw.
	*/

	bool SoftBackend::_beginWarp(const WarpParams& params, SoftWarpmap* pWarpmap, SoftEdgemap* pEdgemap)
	{
		if (m_blendMode == BlendMode::Ignore)
			return false;

		if (!pEdgemap && (!m_pBlitSource || !m_pBlitSource->m_pData))
			return false;

		// Check that we have the kernels needed.

		auto pKernels = m_pKernels[(int)m_canvasSoftFormat];

		if (!pKernels || !pKernels->pCoverageKernels[(int)m_blendMode] || !pKernels->pFillKernels[(int)TintMode::Flat][(int)m_blendMode])
		{
			char errorMsg[1024];

			snprintf(errorMsg, 1024, "Failed warp operation. SoftBackend is missing fill or coverage kernel for BlendMode::%s onto surface of format %s.",
				toString(m_blendMode),
				toString(m_canvasSoftFormat));

			Base::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		m_pShapeCoverageOp = pKernels->pCoverageKernels[(int)m_blendMode];

		WarpSetup& w = m_warp;

		// Paint modes on a canvas without alpha give each part of a pixel the color of what covers
		// it, the result of the blend mode for it as if the canvas was opaque. For Replace that is
		// the color of the source, for Morph (by m) dst * (1 - q) + color * q, where
		// q = a * m / (1 - m + a * m) and a is the alpha of the source. Mixed by area, a pixel is then
		// dst * (1 - sum(part * q)) + sum(part * q * color), which the Replace coverage kernel gives
		// from colors weighted by q in place of alpha and their coverage set to the sum of q.

		w.bOpaquePaint = false;
		w.opaqueMorph = 4096;

		if ((m_blendMode == BlendMode::Replace || m_blendMode == BlendMode::Morph) && !SoftFormatInfo::hasAlpha(m_canvasSoftFormat) &&
			pKernels->pCoverageKernels[(int)BlendMode::Replace])
		{
			w.bOpaquePaint = true;
			w.opaqueMorph = (m_blendMode == BlendMode::Morph) ? m_colTrans.morphFactor : 4096;
			m_pShapeCoverageOp = pKernels->pCoverageKernels[(int)BlendMode::Replace];
		}

		w.pWarpmap = pWarpmap;
		w.pEdgemap = pEdgemap;
		w.dest = Util::roundToPixels(params.dest);

		if (pEdgemap)
		{
			w.size[0] = int64_t(pEdgemap->m_size.w) * c_one;
			w.size[1] = int64_t(pEdgemap->m_size.h) * c_one;
		}
		else
		{
			w.size[0] = int64_t(params.source.w) << 10;
			w.size[1] = int64_t(params.source.h) << 10;
			w.sourcePos[0] = int64_t(params.source.x) << 10;
			w.sourcePos[1] = int64_t(params.source.y) << 10;
		}

		if (w.size[0] <= 0 || w.size[1] <= 0)
			return false;

		int64_t periods[2] = { int64_t(pWarpmap->uPeriod()) << 10, int64_t(pWarpmap->vPeriod()) << 10 };
		int64_t offsets[2] = { int64_t(params.sourceOffset.x) << 10, int64_t(params.sourceOffset.y) << 10 };

		for (int a = 0; a < 2; a++)
		{
			int64_t period = std::clamp(periods[a], int64_t(0), c_maxPos);

			w.period[a] = period;
			w.inside[a] = period > 0 ? std::min(w.size[a], period) : w.size[a];
			w.colorLow[a] = period > 0 ? (w.inside[a] + period) / 2 + 1 - period : 0;
			w.bAlwaysInside[a] = period > 0 && period <= w.size[a];
			w.offset[a] = period > 0 ? wrap(offsets[a], period) : std::clamp(offsets[a], -c_maxPos, c_maxPos);
		}

		if (pEdgemap)
		{
			// Segments. Flat colored segments are prepared as shape segments, for runs drawn
			// with the fill kernels.

			int nSegments = pEdgemap->m_nbRenderSegments;
			w.nSegments = nSegments;

			if ((int)m_shapeSegments.size() < nSegments)
				m_shapeSegments.resize(nSegments);

			// Transparent segments can be skipped where a transparent source has no effect, as for
			// edgemaps, see _beginEdgemapTinting().

			bool bAlphaOnly = (m_canvasSoftFormat == SoftFormat::Alpha_8);
			bool bSkipTransparent = (m_blendMode != BlendMode::Replace && m_blendMode != BlendMode::BlendFixedColor && m_blendMode != BlendMode::Morph &&
									 !(bAlphaOnly && (m_blendMode == BlendMode::Multiply || m_blendMode == BlendMode::Min)));

			const bool bDeviceFlat = m_softTint.isFlat();
			const HiColor deviceColor = bDeviceFlat ? m_softTint.flatColor() : HiColor::White;

			for (int seg = 0; seg < nSegments; seg++)
			{
				const SoftTint& segTint = pEdgemap->m_segmentTints[seg];

				w.bSkip[seg] = bSkipTransparent && (segTint.isTransparent() || m_softTint.isTransparent());
				w.bTinted[seg] = !segTint.isFlat();

				const uint16_t* pNoWords = nullptr;
				const HiColor* pNoColors = nullptr;

				if (w.bTinted[seg])
					m_shapeSegments[seg].bActive = false;
				else
					_setupShapeSegment(m_shapeSegments[seg], true, pEdgemap->m_pFlatColors[seg], false, pNoWords, pNoColors);

				w.flat[seg] = segTint.flatColor();

				HiColor c = (w.bTinted[seg] ? HiColor::White : segTint.flatColor()) * deviceColor;

				w.mix[seg][0] = int16_t((c.b * c.a) >> 12);
				w.mix[seg][1] = int16_t((c.g * c.a) >> 12);
				w.mix[seg][2] = int16_t((c.r * c.a) >> 12);
				w.mix[seg][3] = c.a;
			}
		}
		else
		{
			// Source is read into a HiColor buffer with the first pass of two-pass blits, then
			// drawn with the coverage kernel.

			SoftSurface* pSource = m_pBlitSource;
			SoftFormat srcFormat = pSource->softFormat();

			if (srcFormat == SoftFormat::Undefined)
				return false;

			w.bBilinear = (pSource->sampleMethod() == SampleMethod::Bilinear);
			w.bTileRead = pSource->isTiling();

			// A non-tiling surface is read as if the edge pixels of the source rectangle continued
			// outside it, so positions are kept within the rectangle (and the surface): between the
			// centers of its edge pixels when bilinear, inside it when nearest. The kernels take
			// bilinear positions relative to pixel centers, which puts that range at lo -> hi - 1.

			int64_t surfaceSize[2] = { int64_t(pSource->pixelSize().w) << 16, int64_t(pSource->pixelSize().h) << 16 };

			for (int a = 0; a < 2; a++)
			{
				int64_t lo = std::max(w.sourcePos[a], int64_t(0));
				int64_t hi = std::min(w.sourcePos[a] + w.size[a], surfaceSize[a]);

				w.readMin[a] = lo;
				w.readMax[a] = w.bBilinear ? hi - c_one : hi - 1;

				if (w.readMax[a] < w.readMin[a])				// Less than a pixel wide.
				{
					w.readMin[a] = w.bBilinear ? (lo + hi) / 2 - c_one / 2 : lo;
					w.readMax[a] = w.readMin[a];
				}
			}

			SampleMethod sampleMethod = w.bBilinear ? SampleMethod::Bilinear : SampleMethod::Nearest;
			ReadOp readOp = w.bTileRead ? ReadOp::Tile : ReadOp::Normal;

			w.pReadOp = m_pTransformMoveToHiColorKernels[(int)srcFormat][(int)sampleMethod][(int)readOp];

			// Spans all inside the source are drawn straight onto the canvas by the one-pass kernel of
			// a transform blit or tile, when it has one, as fast as such a blit and without reading into
			// a buffer and blending by coverage, which costs about as much again. Not when tinted other
			// than flat, which blits do in spans of their own, nor for paint modes on a canvas without
			// alpha, which are drawn by their own rules, see above. The blit functions are up to date,
			// see processCommands().

			w.pDirectOp = nullptr;

			if (m_softTint.isFlat() && !m_bBlitTintRuns && !w.bOpaquePaint)
			{
				if (w.bTileRead && m_pTransformTileOp == &SoftBackend::_onePassTransformBlit)
					w.pDirectOp = m_pTransformTileFirstPassOp;
				else if (!w.bTileRead && m_pTransformBlitOp == &SoftBackend::_onePassTransformBlit)
					w.pDirectOp = m_pTransformBlitFirstPassOp;
			}

			if (!w.pReadOp)
			{
				char errorMsg[1024];

				snprintf(errorMsg, 1024, "Failed WarpBlit operation. SoftBackend is missing transform kernel reading %s into HiColor buffer.",
					toString(srcFormat));

				Base::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
				return false;
			}

			w.nSegments = 1;
			w.bTinted[0] = true;
			w.bSkip[0] = false;
		}

		return true;
	}

	//____ _drawWarp() __________________________________________________________
	/*
		Row by row, with the triangles that cover the row. A triangle starts being drawn on its first
		row, or the first row of the clip rectangle, and is dropped after its last. Triangles that are
		all outside the source, or all in a segment that is skipped, aren't drawn at all.

		The pixels of each triangle on a row are then sorted along the row, which they mostly are
		already, from the row above, and made into spans by _addWarpSpans().
	*/

	void SoftBackend::_drawWarp(const RectI& clip, uint8_t* pOrigin, int pitch)
	{
		WarpSetup& w = m_warp;
		SoftWarpmap* pMap = w.pWarpmap;

		RectI area = RectI::overlap(clip, RectI(w.dest, pMap->pixelSize()));
		if (area.w <= 0 || area.h <= 0)
			return;

		// Area in pixels of the Warpmap.

		int x0 = area.x - w.dest.x;
		int x1 = x0 + area.w;
		int y0 = area.y - w.dest.y;
		int y1 = y0 + area.h;

		if ((int)m_warpRuns.size() < area.w)
			m_warpRuns.resize(area.w);

		if ((int)m_warpPixels.size() < area.w)
			m_warpPixels.resize(area.w);

		const std::vector<SoftWarpmap::Setup>& setups = pMap->m_setups;
		const std::vector<int>& order = pMap->m_order;

		m_warpTriangles.clear();
		m_warpRowOrder.clear();
		size_t next = 0;

		for (int y = y0; y < y1; y++)
		{
			// Triangles starting on this row, or above it on the first row.

			while (next < order.size() && setups[order[next]].rowBeg <= y)
			{
				const SoftWarpmap::Setup& setup = setups[order[next++]];

				if (setup.rowEnd <= y || setup.colEnd <= x0 || setup.colBeg >= x1)
					continue;

				int kind = c_warpMixed;

				if (!setup.bPerspective)
				{
					int64_t lo[2], hi[2];
					for (int a = 0; a < 2; a++)
					{
						lo[a] = setup.lo[a] + w.offset[a];
						hi[a] = setup.hi[a] + w.offset[a];
					}

					kind = _warpClassify(lo, hi);

					if (kind == c_warpOutside || (kind >= 0 && w.pEdgemap && w.bSkip[kind]))
						continue;
				}

				WarpTriangle triangle;
				triangle.pSetup = &setup;
				triangle.kind = kind;
				_beginWarpTriangle(triangle, y);
				m_warpRowOrder.push_back(int(m_warpTriangles.size()));
				m_warpTriangles.push_back(triangle);
			}

			// Pixels of each triangle on this row, dropping those that have ended, then sorted along
			// the row. They are kept in the order of the row above, which mostly stays, so the
			// insertion sort is fast.

			int nTriangles = 0;

			for (int index : m_warpRowOrder)
			{
				WarpTriangle& triangle = m_warpTriangles[index];

				if (triangle.pSetup->rowEnd <= y)
					continue;

				_warpTriangleRow(triangle);
				triangle.xBeg = std::max(triangle.xBeg, x0);
				triangle.xEnd = std::min(triangle.xEnd, x1);

				m_warpRowOrder[nTriangles++] = index;
			}

			m_warpRowOrder.resize(nTriangles);

			for (int k = 1; k < nTriangles; k++)
			{
				int index = m_warpRowOrder[k];
				int xBeg = m_warpTriangles[index].xBeg;
				int j = k;
				while (j > 0 && m_warpTriangles[m_warpRowOrder[j - 1]].xBeg > xBeg)
				{
					m_warpRowOrder[j] = m_warpRowOrder[j - 1];
					j--;
				}
				m_warpRowOrder[j] = index;
			}

			if (nTriangles == 0)
				continue;

			m_warpSpans.clear();

			for (int index : m_warpRowOrder)
			{
				if (m_warpTriangles[index].xBeg < m_warpTriangles[index].xEnd)
					_addWarpSpans(m_warpTriangles[index], y);
			}

			if (m_warpSpans.empty())
				continue;

			int canvasY = y + w.dest.y;
			int prefetchRow = (canvasY + 1 < area.y + area.h) ? canvasY + 1 : -1;

			_drawWarpRow(canvasY, area.x, area.x + area.w, pOrigin, pitch, prefetchRow);
		}
	}

	//____ _beginWarpTriangle() _________________________________________________
	/*
		Sets up the edges of a triangle for drawing from a row on. Along a row, the edge function is
		value + edgeX * x. Where edgeX > 0 the edge is a lower bound, inside (with ties, which a left
		edge has) from ceil(-value / edgeX), where edgeX < 0 an upper bound, inside (without ties)
		up to ceil(value / -edgeX). Each row adds edgeY to value, so the bound is stepped by the
		quotient and remainder of that by the divisor, exactly, as SoftWarpmap::Setup describes.
	*/

	void SoftBackend::_beginWarpTriangle(WarpTriangle& triangle, int row)
	{
		const SoftWarpmap::Setup& s = *triangle.pSetup;

		WarpEdge edges[3];
		int kinds[3];

		for (int i = 0; i < 3; i++)
		{
			WarpEdge& e = edges[i];

			int64_t value = s.edge[i] + s.edgeY[i] * row;
			int64_t numerator, step;

			if (s.edgeX[i] > 0)
			{
				e.kind = 0;
				e.divisor = s.edgeX[i];
				numerator = -value;
				step = -s.edgeY[i];
			}
			else if (s.edgeX[i] < 0)
			{
				e.kind = 1;
				e.divisor = -s.edgeX[i];
				numerator = value;
				step = s.edgeY[i];
			}
			else
			{
				e.kind = 2;
				e.value = value;
				e.valueStep = s.edgeY[i];
				e.bTie = s.tie[i];
				kinds[i] = 2;
				continue;
			}

			e.col = ceilDiv(numerator, e.divisor);
			e.rem = e.col * e.divisor - numerator;
			e.stepCol = floorDiv(step, e.divisor);
			e.stepRem = step - e.stepCol * e.divisor;
			kinds[i] = e.kind;
		}

		// In order of kind, so that rows go through them without looking at it.

		int n = 0;
		for (int kind = 0; kind < 3; kind++)
		{
			for (int i = 0; i < 3; i++)
			{
				if (kinds[i] == kind)
					triangle.edges[n++] = edges[i];
			}

			if (kind == 0)
				triangle.nLower = n;
			else if (kind == 1)
				triangle.nUpper = n - triangle.nLower;
		}
	}

	//____ _warpTriangleRow() ___________________________________________________
	// Pixels of the triangle on the current row, then steps its edges to the next, without
	// branches that depend on the row.

	void SoftBackend::_warpTriangleRow(WarpTriangle& triangle)
	{
		const int64_t c_limit = int64_t(1) << 30;

		int64_t beg = -c_limit, end = c_limit;

		const int nLower = triangle.nLower;
		const int nBounds = nLower + triangle.nUpper;

		for (int i = 0; i < nBounds; i++)
		{
			WarpEdge& e = triangle.edges[i];

			if (i < nLower)
				beg = std::max(beg, e.col);
			else
				end = std::min(end, e.col);

			e.col += e.stepCol;
			e.rem -= e.stepRem;

			int64_t negative = e.rem >> 63;					// -1 if the remainder went below 0.
			e.rem += e.divisor & negative;
			e.col -= negative;
		}

		for (int i = nBounds; i < 3; i++)
		{
			WarpEdge& e = triangle.edges[i];
			if (!(e.value > 0 || (e.value == 0 && e.bTie)))
				end = beg;
			e.value += e.valueStep;
		}

		triangle.xBeg = int(std::clamp(beg, -c_limit, c_limit));
		triangle.xEnd = int(std::clamp(end, -c_limit, c_limit));
	}

	//____ _addWarpSpans() ______________________________________________________
	/*
		Spans of a triangle on a row of the Warpmap. Without perspective, positions are linear over
		the triangle and taken from SoftWarpmap::Setup at the start of each span. A triangle that
		isn't all inside or outside the source is split into spans of up to 32 pixels, so that most of
		them can be drawn as inside, or skipped, as a whole.

		With perspective, positions and their derivatives are divided out at the start of each run of
		1, 2, 4, 8 or 16 pixels and are linear in between. Interpolating over L pixels is off by about
		L^2 / (4 * d) pixels, d being the distance to the horizon, so a run is as long as keeps that
		within a quarter of a pixel (c_runShift) at its start and end: W at its start at least
		(L^2 + L) times the change of W per pixel. Runs that aren't all inside or outside the
		source, or in one segment, are split into shorter runs that keep within 1/64 of a pixel
		(c_mixedRunShift), since their coverage depends on where borders and edges are, but keep the
		derivatives interpolated over the whole run. A pixel next to the horizon, where W at the start
		or end of its run isn't positive, is drawn on its own, or not at all if its center is beyond
		the horizon.
	*/

	void SoftBackend::_addWarpSpans(const WarpTriangle& triangle, int row)
	{
		const WarpSetup& w = m_warp;
		const SoftWarpmap::Setup& s = *triangle.pSetup;

		const int xBeg = triangle.xBeg;
		const int xEnd = triangle.xEnd;
		const int64_t zero[2] = { 0, 0 };

		if (!s.bPerspective)
		{
			int64_t step[2], dy[2];
			for (int a = 0; a < 2; a++)
			{
				step[a] = (s.f[a][1] + (int64_t(1) << 15)) >> 16;
				dy[a] = (s.f[a][2] + (int64_t(1) << 15)) >> 16;
			}

			const int chunk = triangle.kind == c_warpMixed ? 32 : xEnd - xBeg;

			for (int x = xBeg; x < xEnd; x += chunk)
			{
				int64_t pos[2];
				for (int a = 0; a < 2; a++)
					pos[a] = ((s.f[a][0] + s.f[a][1] * (x - s.ox) + s.f[a][2] * (row - s.oy) + (int64_t(1) << 15)) >> 16) + w.offset[a];

				_addWarpSpan(triangle.kind, x + w.dest.x, std::min(chunk, xEnd - x), pos, step, step, dy, zero, zero, 4);
			}
			return;
		}

		auto divide = [](int64_t a, int64_t b)
		{
			return floorDiv(a + b / 2, b);
		};

		const int64_t Y = row - s.oy;
		const int64_t rowW = s.f[2][0] + s.f[2][2] * Y;
		const int64_t rowU[2] = { s.f[0][0] + s.f[0][2] * Y, s.f[1][0] + s.f[1][2] * Y };

		// Position at the center of pixel x of the row, and optionally its derivatives along x and
		// y. False beyond the horizon.

		auto position = [&](int x, int64_t* pPos, int64_t& W)
		{
			int64_t X = x - s.ox;
			W = rowW + s.f[2][1] * X;
			if (W <= 0)
				return false;

			for (int a = 0; a < 2; a++)
				pPos[a] = s.ref[a] + divide(rowU[a] + s.f[a][1] * X, W) + w.offset[a];
			return true;
		};

		auto evaluate = [&](int x, int64_t* pPos, int64_t* pDx, int64_t* pDy, int64_t& W)
		{
			if (!position(x, pPos, W))
				return false;

			for (int a = 0; a < 2; a++)
			{
				int64_t relative = pPos[a] - s.ref[a] - w.offset[a];
				pDx[a] = divide(s.f[a][1] - relative * s.f[2][1], W);
				pDy[a] = divide(s.f[a][2] - relative * s.f[2][2], W);
			}
			return true;
		};

		const int64_t gradient = s.gradientW;

		// Pixels from a run start with W that keep the error within 1/4 >> shift of a pixel.

		auto runLength = [gradient](int64_t W, int shift)
		{
			for (int length : { 16, 8, 4, 2 })
			{
				if (W >= (int64_t(length * length + length) * gradient) << shift)
					return length;
			}
			return 1;
		};

		int64_t pos[2], dx[2], dy[2], W;
		bool bValid = evaluate(xBeg, pos, dx, dy, W);

		for (int x = xBeg; x < xEnd; )
		{
			int n = bValid ? std::min(runLength(W, c_runShift), xEnd - x) : 1;

			int64_t nextPos[2], nextDx[2], nextDy[2], nextW;
			bool bNextValid = evaluate(x + n, nextPos, nextDx, nextDy, nextW);

			if (bValid && bNextValid)
			{
				int64_t step[2], dxStep[2], dyStep[2];
				for (int a = 0; a < 2; a++)
				{
					step[a] = n > 1 ? divide(nextPos[a] - pos[a], n) : dx[a];
					dxStep[a] = divide(nextDx[a] - dx[a], n);
					dyStep[a] = divide(nextDy[a] - dy[a], n);
				}

				if (n == 1 || _addWarpSpan(c_warpMixed, x + w.dest.x, n, pos, step, dx, dy, dxStep, dyStep, 6, false) != c_warpMixed)
				{
					if (n == 1)
						_addWarpSpan(c_warpMixed, x + w.dest.x, 1, pos, dx, dx, dy, zero, zero, 4);
				}
				else
				{
					// Borders or edges in the run: shorter runs, with positions divided out at their
					// ends and derivatives still interpolated over the whole run. W is positive all
					// along the run, since it is at both ends.

					int64_t subPos[2] = { pos[0], pos[1] };

					for (int k = 0; k < n; )
					{
						int64_t subW = rowW + s.f[2][1] * (x + k - s.ox);
						int m = std::min(runLength(subW, c_mixedRunShift), n - k);

						int64_t endPos[2], endW;
						if (k + m == n)
						{
							endPos[0] = nextPos[0];
							endPos[1] = nextPos[1];
						}
						else
							position(x + k + m, endPos, endW);

						int64_t subStep[2], d[2], e[2];
						for (int a = 0; a < 2; a++)
						{
							subStep[a] = divide(endPos[a] - subPos[a], m);
							d[a] = dx[a] + dxStep[a] * k;
							e[a] = dy[a] + dyStep[a] * k;
						}

						_addWarpSpan(c_warpMixed, x + k + w.dest.x, m, subPos, subStep, d, e, dxStep, dyStep, 5);

						subPos[0] = endPos[0];
						subPos[1] = endPos[1];
						k += m;
					}
				}
			}
			else
			{
				for (int k = 0; k < n; k++)
				{
					int64_t p[2], d[2], e[2], pixelW;
					if (evaluate(x + k, p, d, e, pixelW))
						_addWarpSpan(c_warpMixed, x + k + w.dest.x, 1, p, d, d, e, zero, zero, 4);
				}
			}

			x += n;
			bValid = bNextValid;
			W = nextW;
			for (int a = 0; a < 2; a++)
			{
				pos[a] = nextPos[a];
				dx[a] = nextDx[a];
				dy[a] = nextDy[a];
			}
		}
	}

	//____ _addWarpSpan() _______________________________________________________
	/*
		Adds a span to the row, positions brought into the period on periodic axes. Spans of
		c_warpMixed are classified from the bounds of their positions, with a margin for the area of
		the pixels: half of how much the position changes over a pixel (marginEighths 4), or more
		for positions that are only close to linear. Returns the kind of the span. A span that is
		mixed isn't added unless bAddMixed is set.
	*/

	int SoftBackend::_addWarpSpan(int kind, int x, int length, const int64_t pos[2], const int64_t step[2], const int64_t dx[2], const int64_t dy[2],
								  const int64_t dxStep[2], const int64_t dyStep[2], int64_t marginEighths, bool bAddMixed)
	{
		const WarpSetup& w = m_warp;

		WarpSpan span;
		span.x = x;
		span.w = length;

		for (int a = 0; a < 2; a++)
		{
			int64_t p = std::clamp(pos[a], -c_maxPos, c_maxPos);
			int64_t period = w.period[a];
			if (period > 0 && (p < 0 || p >= period))
			{
				// Usually within a period or two, without a division.

				if (p >= period && p < 2 * period)
					p -= period;
				else if (p < 0 && p >= -period)
					p += period;
				else
					p -= floorDiv(p, period) * period;
			}

			span.pos[a] = p;
			span.step[a] = std::clamp(step[a], -c_maxGradient, c_maxGradient);
			span.dx[a] = std::clamp(dx[a], -c_maxGradient, c_maxGradient);
			span.dy[a] = std::clamp(dy[a], -c_maxGradient, c_maxGradient);
			span.dxStep[a] = std::clamp(dxStep[a], -c_maxGradient, c_maxGradient);
			span.dyStep[a] = std::clamp(dyStep[a], -c_maxGradient, c_maxGradient);
		}

		if (kind == c_warpMixed)
		{
			int64_t lo[2], hi[2];

			for (int a = 0; a < 2; a++)
			{
				int64_t last = span.pos[a] + span.step[a] * (length - 1);
				int64_t dxLast = span.dx[a] + span.dxStep[a] * (length - 1);
				int64_t dyLast = span.dy[a] + span.dyStep[a] * (length - 1);
				int64_t margin = ((std::max(abs64(span.dx[a]), abs64(dxLast)) + std::max(abs64(span.dy[a]), abs64(dyLast))) * marginEighths) / 8 + 64;

				lo[a] = std::min(span.pos[a], last) - margin;
				hi[a] = std::max(span.pos[a], last) + margin;
			}

			kind = _warpClassify(lo, hi);
		}

		if (kind == c_warpOutside || (kind >= 0 && w.pEdgemap && w.bSkip[kind]) || (kind == c_warpMixed && !bAddMixed))
			return kind;

		span.kind = kind;
		m_warpSpans.push_back(span);
		return kind;
	}

	//____ _warpClassify() ______________________________________________________
	/*
		Kind of a span or triangle with positions within lo -> hi: c_warpOutside if none are inside
		the source, the segment (0 for a blit) that covers all of it if it is all inside, otherwise
		c_warpMixed. On a periodic axis, against the copy of the source in the period lo is in.
	*/

	int SoftBackend::_warpClassify(const int64_t lo[2], const int64_t hi[2]) const
	{
		const WarpSetup& w = m_warp;

		int64_t copy[2];
		int classes[2];

		for (int a = 0; a < 2; a++)
		{
			// Along the axis: 0 = outside, 1 = mixed, 2 = inside.

			int64_t period = w.period[a];
			int64_t base = 0;
			if (period > 0 && (lo[a] < 0 || lo[a] >= period))
				base = (lo[a] < 0 && lo[a] >= -period) ? -period : (lo[a] < 2 * period && lo[a] >= period) ? period : floorDiv(lo[a], period) * period;
			copy[a] = base;

			if (w.bAlwaysInside[a] || (lo[a] >= base && hi[a] <= base + w.inside[a]))
				classes[a] = 2;
			else if (period > 0 ? (lo[a] >= base + w.inside[a] && hi[a] <= base + period) : (hi[a] <= 0 || lo[a] >= w.inside[a]))
				classes[a] = 0;
			else
				classes[a] = 1;
		}

		if (classes[0] == 0 || classes[1] == 0)
			return c_warpOutside;

		if (classes[0] == 2 && classes[1] == 2)
		{
			if (!w.pEdgemap)
				return 0;

			int64_t loCopy[2] = { lo[0], lo[1] }, hiCopy[2] = { hi[0], hi[1] };
			return _warpSolidSegment(loCopy, hiCopy, copy);
		}

		return c_warpMixed;
	}

	//____ _warpSolidSegment() __________________________________________________
	/*
		Segment of the edgemap covering all of a box of positions inside the source (lo -> hi,
		copy being the start of the period it is in), or c_warpMixed.

		Edges are pushed down to the edges above them (effective edge k is the max of edges 0 -> k),
		so the box is in segment s if effective edge s - 1 is above the box and effective edge s below
		it, for all columns of the box. The latter is checked against the max of the minimums of
		edges 0 -> s, which is a lower bound of the minimum of the effective edge.
	*/

	int SoftBackend::_warpSolidSegment(const int64_t lo[2], const int64_t hi[2], const int64_t copy[2]) const
	{
		const WarpSetup& w = m_warp;
		const SoftEdgemap* pEdgemap = w.pEdgemap;

		const int nEdges = w.nSegments - 1;
		if (nEdges == 0)
			return 0;

		const int width = pEdgemap->m_size.w;
		const int samplePitch = pEdgemap->m_nbSegments - 1;

		int64_t ylo = lo[1] - copy[1];
		int64_t yhi = hi[1] - copy[1];

		if (yhi > w.inside[1])
			return c_warpMixed;					// Crosses the seam of a periodic axis.

		// Columns of samples touched, in one or two ranges if crossing the seam of a periodic axis.

		int64_t xlo = lo[0] - copy[0];
		int64_t xhi = hi[0] - copy[0];

		int ranges[2][2];
		int nRanges = 1;

		auto sampleRange = [width](int64_t beg, int64_t end, int* pRange)
		{
			pRange[0] = int(std::clamp(beg >> 16, int64_t(0), int64_t(width)));
			pRange[1] = int(std::clamp((end + c_one - 1) >> 16, int64_t(0), int64_t(width)));
		};

		if (w.period[0] > 0 && xhi > w.period[0])
		{
			if (xhi - w.period[0] >= xlo)
				return c_warpMixed;

			sampleRange(xlo, w.period[0], ranges[0]);
			sampleRange(0, xhi - w.period[0], ranges[1]);
			nRanges = 2;
		}
		else
			sampleRange(xlo, xhi, ranges[0]);

		int nColumns = 0;
		for (int r = 0; r < nRanges; r++)
			nColumns += ranges[r][1] - ranges[r][0] + 1;

		if (nColumns > c_maxSolidColumns)
			return c_warpMixed;

		spx colMin[Edgemap::maxSegments];
		spx colMax[Edgemap::maxSegments];

		for (int e = 0; e < nEdges; e++)
		{
			colMin[e] = std::numeric_limits<spx>::max();
			colMax[e] = std::numeric_limits<spx>::min();
		}

		for (int r = 0; r < nRanges; r++)
		{
			for (int col = ranges[r][0]; col <= ranges[r][1]; col++)
			{
				const spx* pSamples = pEdgemap->m_pSamples + col * samplePitch;

				for (int e = 0; e < nEdges; e++)
				{
					colMin[e] = std::min(colMin[e], pSamples[e]);
					colMax[e] = std::max(colMax[e], pSamples[e]);
				}
			}
		}

		int s = 0;
		int64_t cumMax = std::numeric_limits<int64_t>::min();

		while (s < nEdges)
		{
			cumMax = std::max(cumMax, int64_t(colMax[s]) << 10);
			if (cumMax > ylo)
				break;
			s++;
		}

		if (s < nEdges)
		{
			int64_t lowerBound = std::numeric_limits<int64_t>::min();
			for (int e = 0; e <= s; e++)
				lowerBound = std::max(lowerBound, int64_t(colMin[e]) << 10);

			if (lowerBound <= yhi)
				return c_warpMixed;
		}

		return s;
	}

	//____ _drawWarpRow() _______________________________________________________
	/*
		Draws pixels xBeg -> xEnd of canvas row y, from the spans of the row in m_warpSpans, which are
		in order along the row and don't overlap.
	*/

	void SoftBackend::_drawWarpRow(int y, int xBeg, int xEnd, uint8_t* pOrigin, int pitch, int prefetchRow)
	{
		WarpSetup& w = m_warp;

		SoftShapes::Run* pRuns = m_warpRuns.data();
		CoveragePixel* pPixels = m_warpPixels.data();
		int nRuns = 0;
		int nPixels = 0;

		auto addRun = [&](int begin, int end, int segment)
		{
			if (nRuns > 0 && pRuns[nRuns - 1].end == begin && pRuns[nRuns - 1].segment == segment)
				pRuns[nRuns - 1].end = end;
			else
				pRuns[nRuns++] = { begin, end, segment };
		};

		// Device tint along the row, unless flat and already in the colors.

		const HiColor* pDevice = nullptr;
		HiColor deviceFlat = m_softTint.flatColor();

		if (!m_softTint.isFlat())
		{
			HiColor* p = _warpBuffer(c_maxSegments + 1, xEnd - xBeg);
			m_softTint.generate(xBeg, y, 1, 0, xEnd - xBeg, p);
			pDevice = p - xBeg;
		}

		// Adds a pixel of a color (not premultiplied, without device tint), covered by coverage.

		auto addColoredPixel = [&](int x, HiColor c, int coverage = 4096)
		{
			c = pDevice ? c * pDevice[x] : c * deviceFlat;

			CoveragePixel& px = pPixels[nPixels++];

			if (w.bOpaquePaint)
			{
				int a = (_warpOpaqueAlpha(c.a) * coverage) >> 12;

				px.b = int16_t((c.b * a) >> 12);
				px.g = int16_t((c.g * a) >> 12);
				px.r = int16_t((c.r * a) >> 12);
				px.a = int16_t(a);
				px.coverage = int16_t(a);
			}
			else
				px = _coveragePixel(c, coverage);

			addRun(x, x + 1, -1);
		};


		const int nSpans = int(m_warpSpans.size());

		for (int i = 0; i < nSpans; i++)
		{
			const WarpSpan& span = m_warpSpans[i];

			int beg = std::max(span.x, xBeg);
			int end = std::min(span.x + span.w, xEnd);
			int length = end - beg;

			if (length <= 0)
				continue;

			const int kind = span.kind;

			if (kind == c_warpOutside)
				continue;

			if (kind >= 0 && w.pEdgemap)
			{
				if (w.bSkip[kind])
					continue;

				if (!w.bTinted[kind])
				{
					addRun(beg, end, kind);
					continue;
				}
			}

			// Positions and derivatives at the first pixel and steps along the row.

			int64_t pos[2], step[2], dx[2], dy[2];
			const int64_t* dxStep = span.dxStep;
			const int64_t* dyStep = span.dyStep;

			for (int a = 0; a < 2; a++)
			{
				pos[a] = span.pos[a] + span.step[a] * (beg - span.x);
				step[a] = span.step[a];
				dx[a] = span.dx[a] + span.dxStep[a] * (beg - span.x);
				dy[a] = span.dy[a] + span.dyStep[a] * (beg - span.x);
			}

			if (kind >= 0)
			{
				// Fully covered, colors for each pixel.

				HiColor* pColors = _warpBuffer(c_maxSegments, length);

				if (w.pEdgemap)
					_warpTintSpan(w.pEdgemap->m_segmentTints[kind], pos[0], pos[1], step[0], step[1], length, pColors);
				else if (w.pDirectOp)
				{
					// Straight onto the canvas, except pixels needing reads kept inside the source.

					if ((int)m_warpDrawn.size() < length)
						m_warpDrawn.resize(length);

					uint8_t* pDrawn = m_warpDrawn.data();
					std::memset(pDrawn, 0, length);

					_warpReadSpan(pos[0], pos[1], step[0], step[1], length, pColors, nullptr,
								  pOrigin + y * pitch + beg * m_canvasPixelBytes, { beg, y }, pDrawn);

					for (int k = 0; k < length; k++)
					{
						if (!pDrawn[k])
							addColoredPixel(beg + k, pColors[k]);
					}
					continue;
				}
				else
					_warpReadSpan(pos[0], pos[1], step[0], step[1], length, pColors, nullptr);

				for (int k = 0; k < length; k++)
					addColoredPixel(beg + k, pColors[k]);

				continue;
			}

			// Mixed span, coverage for each pixel.

			if (!w.pEdgemap)
			{
				// Coverage first, then the source for covered pixels.

				if ((int)m_warpCoverage.size() < length)
					m_warpCoverage.resize(length);

				int* pCoverage = m_warpCoverage.data();
				bool bAny = false;

				// Pixels known to be fully covered, worked out for the row rather than pixel by pixel.

				int insideBeg, insideEnd;
				_warpInsideRange(span, pos, length, insideBeg, insideEnd);

				int64_t sx = pos[0], sy = pos[1], ax = dx[0], ay = dx[1], bx = dy[0], by = dy[1];

				for (int k = 0; k < length; k++, sx += step[0], sy += step[1], ax += dxStep[0], ay += dxStep[1], bx += dyStep[0], by += dyStep[1])
				{
					if (k >= insideBeg && k < insideEnd)
					{
						pCoverage[k] = 4096;
						bAny = true;
						continue;
					}

					int64_t gax = std::clamp(ax, -c_maxGradient, c_maxGradient);
					int64_t gay = std::clamp(ay, -c_maxGradient, c_maxGradient);
					int64_t gbx = std::clamp(bx, -c_maxGradient, c_maxGradient);
					int64_t gby = std::clamp(by, -c_maxGradient, c_maxGradient);

					int covX = _warpAxisCoverage(0, wrap(sx, w.period[0]), gax, gbx);
					int cov = covX > 0 ? (covX * _warpAxisCoverage(1, wrap(sy, w.period[1]), gay, gby)) >> 12 : 0;

					pCoverage[k] = cov;
					bAny |= (cov > 0);
				}

				if (!bAny)
					continue;

				HiColor* pColors = _warpBuffer(c_maxSegments, length);

				// Runs of fully covered pixels go straight onto the canvas as for solid spans, when the
				// blit allows it, which long spans with the border of the source in them need to be fast.
				// Pixels drawn are left out of the rest.

				if (w.pDirectOp)
				{
					if ((int)m_warpDrawn.size() < length)
						m_warpDrawn.resize(length);

					uint8_t* pDrawn = m_warpDrawn.data();
					std::memset(pDrawn, 0, length);

					for (int k = 0; k < length; )
					{
						if (pCoverage[k] < 4096)
						{
							k++;
							continue;
						}

						int runEnd = k + 1;
						while (runEnd < length && pCoverage[runEnd] == 4096)
							runEnd++;

						_warpReadSpan(pos[0] + step[0] * k, pos[1] + step[1] * k, step[0], step[1], runEnd - k, pColors + k, nullptr,
									  pOrigin + y * pitch + (beg + k) * m_canvasPixelBytes, { beg + k, y }, pDrawn + k);

						for (int j = k; j < runEnd; j++)
						{
							if (pDrawn[j])
								pCoverage[j] = 0;
						}

						k = runEnd;
					}

					// The rest, in runs of pixels still covered.

					for (int k = 0; k < length; )
					{
						if (pCoverage[k] == 0)
						{
							k++;
							continue;
						}

						int runEnd = k + 1;
						while (runEnd < length && pCoverage[runEnd] != 0)
							runEnd++;

						_warpReadSpan(pos[0] + step[0] * k, pos[1] + step[1] * k, step[0], step[1], runEnd - k, pColors + k, pCoverage + k);
						k = runEnd;
					}
				}
				else
					_warpReadSpan(pos[0], pos[1], step[0], step[1], length, pColors, pCoverage);

				for (int k = 0; k < length; k++)
				{
					int cov = pCoverage[k];
					if (cov == 0)
						continue;

					addColoredPixel(beg + k, pColors[k], cov);
				}
				continue;
			}

			// Mixed span of an edgemap. Colors of tinted segments are generated for the span when first needed.

			bool bGenerated[c_maxSegments] = {};
			const int nSegments = w.nSegments;

			int64_t sx = pos[0], sy = pos[1], ax = dx[0], ay = dx[1], bx = dy[0], by = dy[1];

			for (int k = 0; k < length; k++, sx += step[0], sy += step[1], ax += dxStep[0], ay += dxStep[1], bx += dyStep[0], by += dyStep[1])
			{
				int coverage[c_maxSegments];

				int64_t gax = std::clamp(ax, -c_maxGradient, c_maxGradient);
				int64_t gay = std::clamp(ay, -c_maxGradient, c_maxGradient);
				int64_t gbx = std::clamp(bx, -c_maxGradient, c_maxGradient);
				int64_t gby = std::clamp(by, -c_maxGradient, c_maxGradient);

				int solidSegment;
				int total = _warpEdgemapCoverage(wrap(sx, w.period[0]), wrap(sy, w.period[1]), gax, gbx, gay, gby, coverage, solidSegment);
				if (total == 0)
					continue;

				if (solidSegment >= 0)
				{
					// Whole pixel in one segment, drawn as part of a run if flat.

					if (w.bSkip[solidSegment])
						continue;

					if (!w.bTinted[solidSegment])
					{
						addRun(beg + k, beg + k + 1, solidSegment);
						continue;
					}

					if (!bGenerated[solidSegment])
					{
						_warpTintSpan(w.pEdgemap->m_segmentTints[solidSegment], pos[0], pos[1], step[0], step[1], length, _warpBuffer(solidSegment, length));
						bGenerated[solidSegment] = true;
					}

					addColoredPixel(beg + k, m_warpBuffers[solidSegment][k]);
					continue;
				}

				int sumB = 0, sumG = 0, sumR = 0, sumA = 0;
				int x = beg + k;

				for (int seg = 0; seg < nSegments; seg++)
				{
					int cov = coverage[seg];
					if (cov == 0 || w.bSkip[seg])
						continue;

					if (w.bTinted[seg] && !bGenerated[seg])
					{
						_warpTintSpan(w.pEdgemap->m_segmentTints[seg], pos[0], pos[1], step[0], step[1], length, _warpBuffer(seg, length));
						bGenerated[seg] = true;
					}

					if (w.bOpaquePaint)
					{
						// Each segment by its own result, see _beginWarp(). Device tint included here.

						HiColor c = w.bTinted[seg] ? m_warpBuffers[seg][k] : w.flat[seg];
						c = pDevice ? c * pDevice[x] : c * deviceFlat;

						int a = _warpOpaqueAlpha(c.a);

						sumB += ((c.b * a) >> 12) * cov;
						sumG += ((c.g * a) >> 12) * cov;
						sumR += ((c.r * a) >> 12) * cov;
						sumA += a * cov;
					}
					else if (w.bTinted[seg])
					{
						HiColor c = m_warpBuffers[seg][k];
						if (!pDevice)
							c = c * deviceFlat;

						sumB += ((c.b * c.a) >> 12) * cov;
						sumG += ((c.g * c.a) >> 12) * cov;
						sumR += ((c.r * c.a) >> 12) * cov;
						sumA += c.a * cov;
					}
					else
					{
						const int16_t* pMix = w.mix[seg];

						sumB += pMix[0] * cov;
						sumG += pMix[1] * cov;
						sumR += pMix[2] * cov;
						sumA += pMix[3] * cov;
					}
				}

				CoveragePixel& px = pPixels[nPixels++];

				if (pDevice && !w.bOpaquePaint)
				{
					const HiColor& t = pDevice[x];
					int ta = t.a;

					px.b = int16_t((int64_t(sumB >> 12) * ((t.b * ta) >> 12)) >> 12);
					px.g = int16_t((int64_t(sumG >> 12) * ((t.g * ta) >> 12)) >> 12);
					px.r = int16_t((int64_t(sumR >> 12) * ((t.r * ta) >> 12)) >> 12);
					px.a = int16_t(((sumA >> 12) * ta) >> 12);
				}
				else
				{
					px.b = int16_t(sumB >> 12);
					px.g = int16_t(sumG >> 12);
					px.r = int16_t(sumR >> 12);
					px.a = int16_t(sumA >> 12);
				}

				px.coverage = w.bOpaquePaint ? px.a : int16_t(total);
				addRun(x, x + 1, -1);
			}
		}

		if (nRuns > 0)
			_drawShapeRuns(pOrigin, pitch, y, y + 1, nRuns, pRuns, pPixels, prefetchRow);
	}

	//____ _warpAxisCoverage() __________________________________________________
	/*
		Part of a pixel (0 -> 4096) that is inside the source along one axis, where s is the position
		(wrapped into the period) at the center of the pixel and gx, gy its derivatives along x and y
		of the canvas. On periodic axes the copies of the source in the periods next to it count too.
	*/

	int SoftBackend::_warpAxisCoverage(int axis, int64_t s, int64_t gx, int64_t gy) const
	{
		const WarpSetup& w = m_warp;

		if (w.bAlwaysInside[axis])
			return 4096;

		const int64_t extent = (abs64(gx) + abs64(gy)) / 2 + 1;
		const int64_t inside = w.inside[axis];
		const int64_t period = w.period[axis];

		if (s - extent >= 0 && s + extent <= inside)
			return 4096;

		if (period == 0)
		{
			if (s + extent <= 0 || s - extent >= inside)
				return 0;

			return halfPlaneCoverage(s, gx, gy) - halfPlaneCoverage(s - inside, gx, gy);
		}

		int coverage = 0;

		for (int k = -1; k <= 1; k++)
		{
			int64_t lo = k * period;

			if (s + extent <= lo || s - extent >= lo + inside)
				continue;

			coverage += halfPlaneCoverage(s - lo, gx, gy) - halfPlaneCoverage(s - lo - inside, gx, gy);
		}

		return std::min(coverage, 4096);
	}

	//____ _warpInsideRange() ___________________________________________________
	/*
		Range of pixels insideBeg -> insideEnd of a span along a row, starting at position pos and
		length pixels long, that are fully inside the source, which _warpAxisCoverage() would give
		4096 for. Positions are pos + step * k for pixel k, so per axis that is a range of k, found
		with the largest extent of a pixel over the span, which makes it at most as large as pixel by
		pixel. A periodic axis with a gap between copies isn't worked out, its range is empty.
	*/

	void SoftBackend::_warpInsideRange(const WarpSpan& span, const int64_t* pos, int length, int& insideBeg, int& insideEnd) const
	{
		const WarpSetup& w = m_warp;

		insideBeg = 0;
		insideEnd = length;

		for (int a = 0; a < 2 && insideBeg < insideEnd; a++)
		{
			if (w.bAlwaysInside[a])
				continue;

			if (w.period[a] > 0)
			{
				insideEnd = 0;
				return;
			}

			// Largest derivatives over the whole span, which covers the part of it drawn.

			int64_t dxEnd = span.dx[a] + span.dxStep[a] * (span.w - 1);
			int64_t dyEnd = span.dy[a] + span.dyStep[a] * (span.w - 1);
			int64_t extent = (std::max(abs64(span.dx[a]), abs64(dxEnd)) + std::max(abs64(span.dy[a]), abs64(dyEnd))) / 2 + 1;

			int64_t step = span.step[a];
			int64_t lo = extent - pos[a];							// step * k >= lo
			int64_t hi = w.inside[a] - extent - pos[a];				// step * k <= hi

			int64_t beg, end;

			if (step > 0)
			{
				beg = ceilDiv(lo, step);
				end = floorDiv(hi, step) + 1;
			}
			else if (step < 0)
			{
				beg = ceilDiv(-hi, -step);
				end = floorDiv(-lo, -step) + 1;
			}
			else
			{
				beg = 0;
				end = (lo <= 0 && hi >= 0) ? length : 0;
			}

			insideBeg = int(std::clamp(beg, int64_t(insideBeg), int64_t(length)));
			insideEnd = int(std::clamp(end, int64_t(0), int64_t(insideEnd)));
		}

		if (insideEnd < insideBeg)
			insideEnd = insideBeg;
	}

	//____ _warpEdgemapCoverage() _______________________________________________
	/*
		Coverage of each segment of the edgemap for a pixel whose center is at sx, sy (wrapped into the
		periods), with derivatives ax, ay along x and bx, by along y of the canvas. Returns the total.

		Each edge is linear across a column of the edgemap, so at the pixel the border of the area below
		it is sy - edge(sx) = 0, with derivatives from those of the position and the slope of the edge.
		The areas below the top of the source, each edge and the bottom of the source are measured, kept
		decreasing as edges are pushed down to the edges above them, and their differences give the
		coverage of the segments. That is then scaled by the coverage along x.
	*/

	int SoftBackend::_warpEdgemapCoverage(int64_t sx, int64_t sy, int64_t ax, int64_t bx, int64_t ay, int64_t by, int* pCoverage, int& solidSegment) const
	{
		const WarpSetup& w = m_warp;
		const int nSegments = w.nSegments;

		const SoftEdgemap* pEdgemap = w.pEdgemap;
		const int nEdges = nSegments - 1;
		const int samplePitch = pEdgemap->m_nbSegments - 1;

		solidSegment = -1;

		// Edges are linear across the column at the center of the pixel, extended past the first
		// and last columns for pixels at the left and right borders. A center in the gap between
		// copies on a periodic axis takes the edges of the nearest copy.

		if (sx >= w.inside[0] && w.period[0] > 0 && sx - w.inside[0] > w.period[0] - sx)
			sx -= w.period[0];

		int column = int(std::clamp(sx >> 16, int64_t(0), int64_t(pEdgemap->m_size.w - 1)));
		int64_t frac = sx - (int64_t(column) << 16);

		const spx* pLeft = pEdgemap->m_pSamples + column * samplePitch;
		const spx* pRight = pLeft + samplePitch;

		const int64_t extent = (abs64(ay) + abs64(by)) / 2 + 1;
		const int64_t inside = w.inside[1];
		const int64_t period = w.period[1];

		// Quick check for the common case of a pixel that is all in one segment: inside the source
		// and each edge passing above or below all of it, which is when the distance to the edge is
		// more than half the extent of the pixel along y, extended by the slope of the edge times the
		// extent along x. Same as the full calculation below would give, only faster.

		const int64_t extentX = (abs64(ax) + abs64(bx)) / 2 + 1;

		if ((w.bAlwaysInside[0] || (sx - extentX >= 0 && sx + extentX <= w.inside[0])) && sy - extent >= 0 && sy + extent <= inside)
		{
			int e = 0;

			for (; e < nEdges; e++)
			{
				int64_t diff = int64_t(pRight[e]) - pLeft[e];
				int64_t f = sy - (int64_t(pLeft[e]) << 10) - ((diff * frac) >> 6);
				int64_t margin = extent + ((abs64(diff << 10) * extentX) >> 16);

				if (f >= margin)
					continue;					// Below the edge.

				if (f <= -margin)
					break;						// Above the edge, so in the segment above it.

				e = -1;							// Edge crosses the pixel.
				break;
			}

			if (e >= 0)
			{
				solidSegment = e;
				return 4096;
			}
		}

		for (int seg = 0; seg < nSegments; seg++)
			pCoverage[seg] = 0;

		int covX = _warpAxisCoverage(0, sx, ax, bx);
		if (covX == 0)
			return 0;

		int kBeg = period > 0 ? -1 : 0;
		int kEnd = period > 0 ? 1 : 0;

		for (int k = kBeg; k <= kEnd; k++)
		{
			int64_t s = sy - k * period;

			if (s + extent <= 0 || s - extent >= inside)
				continue;

			int above = halfPlaneCoverage(s, ay, by);

			for (int e = 0; e < nEdges; e++)
			{
				int64_t diff = int64_t(pRight[e]) - pLeft[e];
				int64_t edge = (int64_t(pLeft[e]) << 10) + ((diff * frac) >> 6);
				int64_t slope = diff << 10;

				int64_t gx = ay - ((slope * ax) >> 16);
				int64_t gy = by - ((slope * bx) >> 16);

				int below = std::min(above, halfPlaneCoverage(s - edge, gx, gy));
				pCoverage[e] += above - below;
				above = below;
			}

			int below = std::min(above, halfPlaneCoverage(s - inside, ay, by));
			pCoverage[nEdges] += above - below;
		}

		int total = 0;

		for (int seg = 0; seg < nSegments; seg++)
		{
			int cov = (pCoverage[seg] * covX) >> 12;
			pCoverage[seg] = cov;
			total += cov;
		}

		return std::min(total, 4096);
	}

	//____ _warpTintSpan() ______________________________________________________
	// Colors of a segment tint for positions (x,y), (x+dx, y+dy) ... in edgemap pixels, wrapped into the periods
	// with a center in the gap between copies in the nearest one (see WarpSetup::colorLow).

	void SoftBackend::_warpTintSpan(const SoftTint& tint, int64_t x, int64_t y, int64_t dx, int64_t dy, int length, HiColor* pOutput)
	{
		if (tint.isFlat())
		{
			HiColor c = tint.flatColor();
			for (int i = 0; i < length; i++)
				pOutput[i] = c;
			return;
		}

		forWrappedSpans(m_warp.period, m_warp.colorLow, x, y, dx, dy, length, [&](int begin, int n, int64_t px, int64_t py)
		{
			tint.generateSpan(px, py, dx, dy, n, pOutput + begin);
		});
	}

	//____ _warpReadSpan() ______________________________________________________
	/*
		Reads the blit source for positions (x,y), (x+dx, y+dy) ... in pixels of the source rectangle,
		wrapped into the periods with a center in the gap between copies in the nearest one (see
		WarpSetup::colorLow). A non-tiling surface is read as if the edge pixels of the source
		rectangle continued outside it, which the kernels can't do, so positions outside are clamped
		and read one by one. If pCoverage is set, pixels without coverage are not read.

		If pDirect is set (the first pixel of the span on the canvas, at directPos), pixels that the
		kernel reads as they are are drawn straight onto the canvas by WarpSetup::pDirectOp instead,
		and marked in pDrawn. Only for spans all covered.
	*/

	void SoftBackend::_warpReadSpan(int64_t x, int64_t y, int64_t dx, int64_t dy, int length, HiColor* pOutput, const int* pCoverage,
									uint8_t* pDirect, CoordI directPos, uint8_t* pDrawn)
	{
		const WarpSetup& w = m_warp;
		const SoftSurface* pSource = m_pBlitSource;

		const int64_t half = w.bBilinear ? c_one / 2 : 0;		// Bilinear kernels take positions relative to pixel centers.

		binalInt matrix[2][2] = { { binalInt(toBinal(dx)), binalInt(toBinal(dy)) }, { 0, 0 } };

		forWrappedSpans(w.period, w.colorLow, x, y, dx, dy, length, [&](int begin, int n, int64_t px, int64_t py)
		{
			int64_t sx = w.sourcePos[0] + px - half;
			int64_t sy = w.sourcePos[1] + py - half;

			auto read = [&](int beg, int count, int64_t posX, int64_t posY)
			{
				BinalCoord pos = { binalInt(toBinal(posX)), binalInt(toBinal(posY)) };
				w.pReadOp(pSource, pos, matrix, (uint8_t*)(pOutput + begin + beg), 8, 0, 1, count, m_colTrans, { 0,0 });
			};

			// Draws straight onto the canvas if pDirect is set, otherwise reads.

			auto draw = [&](int beg, int count, int64_t posX, int64_t posY)
			{
				if (!pDirect)
				{
					read(beg, count, posX, posY);
					return;
				}

				int i = begin + beg;
				BinalCoord pos = { binalInt(toBinal(posX)), binalInt(toBinal(posY)) };
				w.pDirectOp(pSource, pos, matrix, pDirect + i * m_canvasPixelBytes, m_canvasPixelBytes, 0, 1, count, m_colTrans, { directPos.x + i, directPos.y });
				std::memset(pDrawn + i, 1, count);
			};

			if (w.bTileRead)
			{
				// Kernel wraps around the surface.

				const int64_t periodX = int64_t(pSource->pixelSize().w) * c_one;
				const int64_t periodY = int64_t(pSource->pixelSize().h) * c_one;

				sx %= periodX;
				sy %= periodY;

				draw(0, n, sx < 0 ? sx + periodX : sx, sy < 0 ? sy + periodY : sy);
				return;
			}

			// Positions the kernel can read as they are, see WarpSetup::readMin. A bilinear kernel
			// doesn't read the neighbor of a pixel it is exactly on.

			const int64_t minX = w.readMin[0], maxX = w.readMax[0];
			const int64_t minY = w.readMin[1], maxY = w.readMax[1];

			int begX, endX, begY, endY;
			linearRange(sx, dx, minX, maxX, n, begX, endX);
			linearRange(sy, dy, minY, maxY, n, begY, endY);

			int innerBeg = std::max(begX, begY);
			int innerEnd = std::min(endX, endY);

			if (innerBeg < innerEnd)
				draw(innerBeg, innerEnd - innerBeg, sx + dx * innerBeg, sy + dy * innerBeg);
			else
				innerBeg = innerEnd = n;

			for (int i = 0; i < n; i++)
			{
				if (i == innerBeg)
				{
					i = innerEnd - 1;
					continue;
				}

				if (pCoverage && pCoverage[begin + i] == 0)
					continue;

				read(i, 1, std::clamp(sx + dx * i, minX, maxX), std::clamp(sy + dy * i, minY, maxY));
			}
		});
	}

	//____ _warpOpaqueAlpha() __________________________________________________
	// Weight of a color of alpha a in a pixel painted on a canvas without alpha, see _beginWarp().

	int SoftBackend::_warpOpaqueAlpha(int a) const
	{
		const int m = m_warp.opaqueMorph;

		if (m >= 4096)
			return 4096;
		if (m <= 0 || a <= 0)
			return 0;

		// am * 4096 / den, divided by 16 above and below to fit a 32 bit division. den is at least
		// 4096 here, so den / 16 keeps plenty of precision.

		uint32_t am = uint32_t(a) * uint32_t(m);
		uint32_t den = uint32_t(4096 - m) * 4096 + am;

		return int((am << 8) / (den >> 4));
	}

	//____ _warpBuffer() ________________________________________________________

	HiColor* SoftBackend::_warpBuffer(int index, int length)
	{
		auto& buffer = m_warpBuffers[index];
		if ((int)buffer.size() < length)
			buffer.resize(length);
		return buffer.data();
	}

} // namespace wax
