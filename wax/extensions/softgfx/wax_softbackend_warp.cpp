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

	The Warpmap is processed one row of cells at a time. The corners of each cell are converted
	from spx to pixels of the source with 16 binals, and the cell is classified from the bounding
	box of its corners:

	- Outside: no part of the cell reaches into the source. Nothing is drawn.

	- Solid: the whole cell is inside the source and, for an edgemap, inside one segment. Flat
	  colored segments are drawn as runs with the fill kernels, like shapes. Tinted segments and
	  blits get colors for each pixel, from the position of its center.

	- Mixed: coverage is estimated for each pixel from the position of its center and how fast the
	  position changes over the pixel (the derivatives of the bilinear mapping). Each border (of the
	  source and of each edge) is taken as straight over the pixel, which gives the exact area on
	  each side of it without supersampling, see halfPlaneCoverage(). Coverage of the segments
	  between edges is the difference between the areas below them.

	Partly covered pixels and pixels with colors of their own are drawn with the coverage kernels,
	which blend by the rules in "Blending and transparency".

	Positions within a cell are stepped along each row of pixels, since the bilinear mapping is
	linear along a row of a cell, and the start of each row is stepped from the row above, since it
	is linear down a column too. Positions are int64 to cope with any Warpmap, but only added and
	compared per pixel, except where coverage is estimated. Divisions are made once per row of
	cells, not per row of pixels, which matters on CPUs without 64-bit division.
*/

#include <wax_softbackend.h>
#include <wax_softedgemap.h>
#include <wax_softwarpmap.h>
#include <wax_core.h>
#include <wax_util.h>

#include <algorithm>
#include <cstdlib>
#include <limits>

namespace wax
{
	namespace
	{
		const int64_t	c_one = 65536;					// 1.0 with 16 binals.
		const int64_t	c_maxPos = int64_t(1) << 40;	// Positions are kept within +-2^24 pixels.
		const int64_t	c_maxGradient = int64_t(1) << 31;	// Derivatives limited to 2^15 source pixels per pixel for coverage.
		const int		c_maxSolidColumns = 64;			// Cells spanning more edgemap columns are drawn as mixed.
		const int		c_warpExtraBits = 16;			// Extra binals of values stepped from row to row.
		const int64_t	c_warpExtraScale = int64_t(1) << c_warpExtraBits;

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

			// |f| < a here. Drop precision so that the products below fit.

			while (a >= (int64_t(1) << 20))
			{
				a >>= 1;
				b >>= 1;
				f >>= 1;
			}

			int64_t absF = abs64(f);

			if (2 * absF <= a - b)
				return int(2048 + (f * 4096) / a);

			int64_t t = a + b - 2 * absF;						// Twice the height of the corner triangle.
			int corner = int((t * t * 4096) / (8 * a * b));

			return f > 0 ? 4096 - corner : corner;
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

			Core::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
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

			if (!w.pReadOp)
			{
				char errorMsg[1024];

				snprintf(errorMsg, 1024, "Failed WarpBlit operation. SoftBackend is missing transform kernel reading %s into HiColor buffer.",
					toString(srcFormat));

				Core::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
				return false;
			}

			w.nSegments = 1;
			w.bTinted[0] = true;
			w.bSkip[0] = false;
		}

		return true;
	}

	//____ _drawWarp() __________________________________________________________

	void SoftBackend::_drawWarp(const RectI& clip, uint8_t* pOrigin, int pitch)
	{
		WarpSetup& w = m_warp;
		SoftWarpmap* pMap = w.pWarpmap;

		RectI area = RectI::overlap(clip, RectI(w.dest, pMap->pixelSize()));
		if (area.w <= 0 || area.h <= 0)
			return;

		const int cellSize = pMap->cellSize();
		const SizeI nodes = pMap->nodes();

		// Area in pixels of the Warpmap.

		int x0 = area.x - w.dest.x;
		int x1 = x0 + area.w;
		int y0 = area.y - w.dest.y;
		int y1 = y0 + area.h;

		int firstColumn = std::min(x0 / cellSize, nodes.w - 2);
		int lastColumn = std::min((x1 - 1) / cellSize, nodes.w - 2);
		int firstRow = std::min(y0 / cellSize, nodes.h - 2);
		int lastRow = std::min((y1 - 1) / cellSize, nodes.h - 2);

		int nColumns = lastColumn - firstColumn + 1;

		m_warpCells.resize(nColumns);

		if ((int)m_warpRuns.size() < area.w)
			m_warpRuns.resize(area.w);

		if ((int)m_warpPixels.size() < area.w)
			m_warpPixels.resize(area.w);

		for (int row = firstRow; row <= lastRow; row++)
		{
			_setupWarpCells(row, firstColumn, nColumns);

			int cellY = pMap->nodeY(row);
			int cellH = pMap->nodeY(row + 1) - cellY;

			int rowBeg = std::max(cellY, y0);
			int rowEnd = std::min(cellY + cellH, y1);

			_beginWarpCellRow(rowBeg, area.x, cellY, cellH);

			for (int y = rowBeg; y < rowEnd; y++)
			{
				int canvasY = y + w.dest.y;
				int prefetchRow = (canvasY + 1 < area.y + area.h) ? canvasY + 1 : -1;

				_drawWarpRow(canvasY, area.x, area.x + area.w, pOrigin, pitch, prefetchRow);

				for (WarpCell& cell : m_warpCells)
				{
					for (int a = 0; a < 2; a++)
					{
						cell.pos[a] += cell.posStep[a];
						cell.step[a] += cell.stepStep[a];
					}
				}
			}
		}
	}

	//____ _setupWarpCells() ____________________________________________________
	/*
		Converts the corners of cells firstColumn -> firstColumn + nColumns of a row of cells to
		16 binals and classifies the cells, see top of file.
	*/

	void SoftBackend::_setupWarpCells(int row, int firstColumn, int nColumns)
	{
		WarpSetup& w = m_warp;
		SoftWarpmap* pMap = w.pWarpmap;

		const int cellH = pMap->nodeY(row + 1) - pMap->nodeY(row);

		for (int i = 0; i < nColumns; i++)
		{
			WarpCell& cell = m_warpCells[i];
			int column = firstColumn + i;

			cell.x = pMap->nodeX(column);
			cell.w = pMap->nodeX(column + 1) - cell.x;

			CoordSPX corners[4];
			pMap->cellCorners(column, row, corners);

			int64_t lo[2], hi[2], copy[2];
			int classes[2];

			for (int a = 0; a < 2; a++)
			{
				int64_t v[4];

				for (int k = 0; k < 4; k++)
				{
					int64_t pos = int64_t(a == 0 ? corners[k].x : corners[k].y) << 10;
					v[k] = std::clamp(pos, -c_maxPos, c_maxPos) + w.offset[a];
				}

				// Top-left corner into the period, the others follow.

				if (w.period[a] > 0)
				{
					int64_t shift = floorDiv(v[0], w.period[a]) * w.period[a];
					for (int k = 0; k < 4; k++)
						v[k] -= shift;
				}

				cell.tl[a] = v[0];
				cell.right[a] = v[1] - v[0];
				cell.down[a] = v[2] - v[0];
				cell.twist[a] = v[3] - v[2] - v[1] + v[0];

				// Bounding box of the cell, with a margin for linearizing the pixels.

				int64_t margin = 64 + abs64(cell.twist[a]) / (4 * int64_t(cell.w) * cellH);

				lo[a] = std::min(std::min(v[0], v[1]), std::min(v[2], v[3])) - margin;
				hi[a] = std::max(std::max(v[0], v[1]), std::max(v[2], v[3])) + margin;

				// Classify along the axis: 0 = outside, 1 = mixed, 2 = inside.

				int64_t period = w.period[a];
				int64_t base = period > 0 ? floorDiv(lo[a], period) * period : 0;
				copy[a] = base;

				if (w.bAlwaysInside[a] || (lo[a] >= base && hi[a] <= base + w.inside[a]))
					classes[a] = 2;
				else if (period > 0 ? (lo[a] >= base + w.inside[a] && hi[a] <= base + period) : (hi[a] <= 0 || lo[a] >= w.inside[a]))
					classes[a] = 0;
				else
					classes[a] = 1;
			}

			if (classes[0] == 0 || classes[1] == 0)
				cell.kind = c_warpOutside;
			else if (classes[0] == 2 && classes[1] == 2)
				cell.kind = w.pEdgemap ? _warpSolidSegment(lo, hi, copy) : 0;
			else
				cell.kind = c_warpMixed;
		}
	}

	//____ _beginWarpCellRow() __________________________________________________
	/*
		Sets up the cells of a row of cells (cellRowH pixels high, from row cellRowY of the Warpmap)
		for drawing from pixel column xBeg of canvas rows, starting at row firstRow of the Warpmap:
		positions and derivatives at the center of the first pixel of the cell in the row, and how
		they change from one row to the next. All of it is linear down the cell, so _drawWarp() steps
		pos and step after each row instead of working them out again. They have c_warpExtraBits
		more binals so that the steps add up to within a fraction of a unit over the cell.
	*/

	void SoftBackend::_beginWarpCellRow(int firstRow, int xBeg, int cellRowY, int cellRowH)
	{
		const WarpSetup& w = m_warp;

		const int64_t fyNum = 2 * int64_t(firstRow - cellRowY) + 1;		// Center of first row within cell, over fyDen.
		const int64_t fyDen = 2 * int64_t(cellRowH);

		for (WarpCell& cell : m_warpCells)
		{
			// Only cells drawn pixel by pixel need it, see _drawWarpRow().

			const int kind = cell.kind;

			if (kind == c_warpOutside || (kind >= 0 && w.pEdgemap && (w.bSkip[kind] || !w.bTinted[kind])))
			{
				for (int a = 0; a < 2; a++)
				{
					cell.posStep[a] = 0;
					cell.stepStep[a] = 0;
				}
				continue;
			}

			int beg = std::max(cell.x + w.dest.x, xBeg);

			const int64_t fxNum = 2 * int64_t(beg - w.dest.x - cell.x) + 1;		// Center of first pixel within cell, over fxDen.
			const int64_t fxDen = 2 * int64_t(cell.w);

			for (int a = 0; a < 2; a++)
			{
				// down * fyNum / fyDen and the step of it from row to row (fyNum + 2), from one
				// division of down, likewise for twist.

				int64_t downQ = floorDiv(cell.down[a] * c_warpExtraScale, fyDen);
				int64_t downR = cell.down[a] * c_warpExtraScale - downQ * fyDen;
				int64_t twistQ = floorDiv(cell.twist[a] * c_warpExtraScale, fyDen);
				int64_t twistR = cell.twist[a] * c_warpExtraScale - twistQ * fyDen;

				int64_t left = cell.tl[a] * c_warpExtraScale + downQ * fyNum + (downR * fyNum) / fyDen;
				int64_t across = cell.right[a] * c_warpExtraScale + twistQ * fyNum + (twistR * fyNum) / fyDen;

				int64_t leftStep = downQ * 2 + (downR * 2) / fyDen;
				int64_t acrossStep = twistQ * 2 + (twistR * 2) / fyDen;

				cell.pos[a] = left + mulDiv(across, fxNum, fxDen);
				cell.posStep[a] = leftStep + mulDiv(acrossStep, fxNum, fxDen);
				cell.step[a] = floorDiv(across, cell.w);
				cell.stepStep[a] = floorDiv(acrossStep, cell.w);

				cell.dy[a] = (cell.down[a] * fxDen + cell.twist[a] * fxNum) / (fxDen * cellRowH);
				cell.dyStep[a] = cell.twist[a] / (int64_t(cell.w) * cellRowH);
			}

		}
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
		Draws pixels xBeg -> xEnd of canvas row y. Cells are set up by _setupWarpCells() and
		_beginWarpCellRow() and stepped to the row by _drawWarp().
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


		const int nCells = int(m_warpCells.size());

		for (int i = 0; i < nCells; i++)
		{
			const WarpCell& cell = m_warpCells[i];

			int beg = std::max(cell.x + w.dest.x, xBeg);
			int end = std::min(cell.x + cell.w + w.dest.x, xEnd);
			int length = end - beg;

			if (length <= 0)
			{
				if (cell.x + w.dest.x >= xEnd)
					break;
				continue;
			}

			const int kind = cell.kind;

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

			// Positions and derivatives at the first pixel and steps along the row, see _beginWarpCellRow().

			int64_t pos[2], step[2];
			const int64_t* dy = cell.dy;
			const int64_t* dyStep = cell.dyStep;

			for (int a = 0; a < 2; a++)
			{
				pos[a] = cell.pos[a] >> c_warpExtraBits;
				step[a] = (cell.step[a] + (int64_t(1) << (c_warpExtraBits - 1))) >> c_warpExtraBits;
			}

			if (kind >= 0)
			{
				// Fully covered, colors for each pixel.

				HiColor* pColors = _warpBuffer(c_maxSegments, length);

				if (w.pEdgemap)
					_warpTintSpan(w.pEdgemap->m_segmentTints[kind], pos[0], pos[1], step[0], step[1], length, pColors);
				else
					_warpReadSpan(pos[0], pos[1], step[0], step[1], length, pColors, nullptr);

				for (int k = 0; k < length; k++)
					addColoredPixel(beg + k, pColors[k]);

				continue;
			}

			// Mixed cell, coverage for each pixel.

			const int64_t ax = std::clamp(step[0], -c_maxGradient, c_maxGradient);
			const int64_t ay = std::clamp(step[1], -c_maxGradient, c_maxGradient);

			if (!w.pEdgemap)
			{
				// Coverage first, then the source for covered pixels.

				if ((int)m_warpCoverage.size() < length)
					m_warpCoverage.resize(length);

				int* pCoverage = m_warpCoverage.data();
				bool bAny = false;

				int64_t sx = pos[0], sy = pos[1], bx = dy[0], by = dy[1];

				for (int k = 0; k < length; k++)
				{
					int64_t gbx = std::clamp(bx, -c_maxGradient, c_maxGradient);
					int64_t gby = std::clamp(by, -c_maxGradient, c_maxGradient);

					int covX = _warpAxisCoverage(0, wrap(sx, w.period[0]), ax, gbx);
					int cov = covX > 0 ? (covX * _warpAxisCoverage(1, wrap(sy, w.period[1]), ay, gby)) >> 12 : 0;

					pCoverage[k] = cov;
					bAny |= (cov > 0);

					sx += step[0];
					sy += step[1];
					bx += dyStep[0];
					by += dyStep[1];
				}

				if (!bAny)
					continue;

				HiColor* pColors = _warpBuffer(c_maxSegments, length);
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

			// Mixed cell of an edgemap. Colors of tinted segments are generated for the span when first needed.

			bool bGenerated[c_maxSegments] = {};
			const int nSegments = w.nSegments;

			int64_t sx = pos[0], sy = pos[1], bx = dy[0], by = dy[1];

			for (int k = 0; k < length; k++, sx += step[0], sy += step[1], bx += dyStep[0], by += dyStep[1])
			{
				int coverage[c_maxSegments];

				int64_t gbx = std::clamp(bx, -c_maxGradient, c_maxGradient);
				int64_t gby = std::clamp(by, -c_maxGradient, c_maxGradient);

				int solidSegment;
				int total = _warpEdgemapCoverage(wrap(sx, w.period[0]), wrap(sy, w.period[1]), ax, gbx, ay, gby, coverage, solidSegment);
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
	*/

	void SoftBackend::_warpReadSpan(int64_t x, int64_t y, int64_t dx, int64_t dy, int length, HiColor* pOutput, const int* pCoverage)
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

			if (w.bTileRead)
			{
				// Kernel wraps around the surface.

				int64_t maskX = (int64_t(pSource->tileMaskX()) << 16) | (c_one - 1);
				int64_t maskY = (int64_t(pSource->tileMaskY()) << 16) | (c_one - 1);

				read(0, n, sx & maskX, sy & maskY);
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
				read(innerBeg, innerEnd - innerBeg, sx + dx * innerBeg, sy + dy * innerBeg);
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

		int64_t am = int64_t(a) * m;
		int64_t den = int64_t(4096 - m) * 4096 + am;

		return den > 0 ? int((am * 4096) / den) : 0;
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
