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

#include <wax_softshapes.h>
#include <wax_util.h>

#include <algorithm>
#include <climits>
#include <cstring>

namespace wax
{
	namespace
	{
		const int64_t c_one = int64_t(1) << 32;			// 1.0 in 32.32 fixed point.

		const int16_t c_noColor[4] = { 0, 0, 0, 0 };

		// Pixel index of spx value, rounded down or up, for values that can be far outside any canvas.

		inline int floorPixel(int64_t value)
		{
			int64_t pixel = value >> 6;
			return int(pixel < -0x40000000 ? -0x40000000 : pixel > 0x40000000 ? 0x40000000 : pixel);
		}

		inline int ceilPixel(int64_t value) { return floorPixel(value + 63); }

		//____ Sine table _____________________________________________________
		// sin(i/1024 * pi/2) in 2.30 fixed point, generated at compile time.

		constexpr double constexprSin(double x)
		{
			double term = x, sum = x;
			for (int n = 1; n < 20; n++)
			{
				term *= -x * x / ((2 * n) * (2 * n + 1));
				sum += term;
			}
			return sum;
		}

		struct SinTable
		{
			int32_t v[1026];

			constexpr SinTable() : v{}
			{
				for (int i = 0; i <= 1025; i++)
				{
					double angle = (i > 1024 ? 1024 : i) * (3.14159265358979323846 / 2) / 1024;
					v[i] = int32_t(constexprSin(angle) * 1073741824.0 + 0.5);
				}
			}
		};

		constexpr SinTable c_sinTable;

		//____ Reciprocal table _______________________________________________
		// 2^37 / n for edges spanning up to 16 pixels along a row, to avoid divisions.

		struct RecipTable
		{
			int64_t v[1024];

			constexpr RecipTable() : v{}
			{
				for (int i = 1; i < 1024; i++)
					v[i] = (int64_t(1) << 37) / i;
			}
		};

		constexpr RecipTable c_recipTable;

		inline int64_t recip37(int64_t n)
		{
			return n < 1024 ? c_recipTable.v[n] : (int64_t(1) << 37) / n;
		}

		//____ isqrt() ________________________________________________________
		// Square root rounded to nearest, digit by digit.

		inline int64_t isqrt(uint64_t value)
		{
			if (value == 0)
				return 0;

#if defined(__GNUC__) || defined(__clang__)
			int topBit = 63 - __builtin_clzll(value);
#else
			int topBit = 0;
			for (int step = 32; step > 0; step >>= 1)
				if ((value >> (topBit + step)) != 0)
					topBit += step;
#endif

			uint64_t root = 0;
			uint64_t bit = uint64_t(1) << (topBit & ~1);

			while (bit != 0)
			{
				if (value >= root + bit)
				{
					value -= root + bit;
					root = (root >> 1) + bit;
				}
				else
					root >>= 1;

				bit >>= 2;
			}

			return int64_t(root + (value > root ? 1 : 0));
		}

		//____ mulFrac() ______________________________________________________
		// value * frac / 2^32, without overflow for values up to 2^47.

		inline int64_t mulFrac(int64_t value, uint32_t frac)
		{
			return ((value * int64_t(frac >> 16)) >> 16) + ((value * int64_t(frac & 0xFFFF)) >> 32);
		}

		//____ splitPoint() ___________________________________________________
		/*
			Point strictly between a and b, which are at least 2 apart, where the most low
			bits are zero. It is near the middle and, unlike the middle rounded down, where
			the mirror image of the interval would be split, so that symmetric shapes stay
			symmetric.
		*/

		inline int64_t splitPoint(int64_t a, int64_t b)
		{
			int bits = 0;
			while ((int64_t(2) << bits) <= b - a)
				bits++;

			int64_t mid = ((a >> bits) + 1) << bits;
			while (mid >= b)
			{
				bits--;
				mid = ((a >> bits) + 1) << bits;
			}

			return mid;
		}
	}

	//____ EdgeShape __________________________________________________________
	/*
		A shape as edges crossing sub-rows, given by the shape to the rasterizer. Shapes
		derive from it and provide:

		int		edges(int64_t ya, int64_t yb, EdgeDef* pEdges) const

			Edges crossing the sub-row from ya to yb, in order along x. Returns their number.

		int64_t	x(const EdgeDef& edge, int64_t y)

			x of an edge at y. Edges must not cross, except clamped edges by rounding. Each
			edge has an id, the same wherever it is, so x can be reused from the bottom of
			one sub-row for the top of the next.

		bool	split(const EdgeDef& edge, int64_t ya, int64_t xa, int64_t yb, int64_t xb, int64_t& y)

			Tells if the edge from (ya, xa) to (yb, xb) needs to be split into a polyline,
			where it bends or kinks, and at which y strictly between ya and yb.

		int		uniformEnd(int row) const

			Row after the rows from row on that are the same as row.

		Shapes symmetric around a vertical or horizontal axis can tell so, see bSymmetricX
		and bSymmetricY. Edges and segments on one side of the axis must then be the mirror
		image of those on the other.
	*/

	class SoftShapes::EdgeShape
	{
	public:
		const int64_t*	pSplits = nullptr;		// Where sub-rows must begin and end, sorted.
		int				nSplits = 0;
		int				maxEdges = 0;			// Most edges in a sub-row.
		int				nIds = 0;				// Ids of edges are 0 up to nIds.

		bool			bSymmetricX = false;	// Rows are their own mirror images, x mirrored to
		int64_t			symmetrySumX = 0;		// symmetrySumX - x.

		bool			bSymmetricY = false;	// Shape is its own mirror image upside down, y mirrored
		int64_t			symmetrySumY = 0;		// to symmetrySumY - y.
	};

	//____ _polylineX() _______________________________________________________
	// x of polyline at y, between its first and last vertex.

	inline int64_t SoftShapes::_polylineX(Polyline& polyline, int64_t y)
	{
		const int64_t* pY = polyline.pY;
		int k = polyline.line;

		while (k > 0 && pY[k] > y)
			k--;

		while (k < polyline.n - 2 && pY[k + 1] < y)
			k++;

		polyline.line = k;

		const int64_t y1 = pY[k], y2 = pY[k + 1];
		const int64_t x1 = polyline.pX[k], x2 = polyline.pX[k + 1];

		if (y <= y1 || x1 == x2)
			return x1;

		if (y >= y2)
			return x2;

		// Weighted by distance to the other vertex, the same whichever vertex comes first,
		// so that mirrored corners stay mirrored. Rounded to nearest. Lines of corners are
		// short, divisions rare.

		const int64_t base = std::min(x1, x2);
		const int64_t h = y2 - y1;
		const int64_t d = (x1 - base) * (y2 - y) + (x2 - base) * (y - y1);

		return base + (h < 1024 ? (d * c_recipTable.v[h] + (int64_t(1) << 36)) >> 37 : (d + h / 2) / h);
	}

	//____ _area() ____________________________________________________________
	/*
		Area right of a piece in a sub-row as high as the piece, left of x, 64 for each
		spx along x right of the piece. A pixel gets _area(right side) - _area(left side),
		which is 4096 if fully right of the piece.

		The part of a vertical line at x right of the piece grows linearly from lo to hi,
		so the area is quadratic between them.
	*/

	inline int64_t SoftShapes::_area(const Piece& piece, int64_t x)
	{
		if (x <= piece.lo)
			return 0;

		if (x >= piece.hi)
			return (piece.hi - piece.lo) * 32 + (x - piece.hi) * 64;

		int64_t d = x - piece.lo;
		return (d * ((d * piece.inv) >> 5)) >> 27;			// d^2 * 32 / (hi - lo)
	}

	//____ _edgeLeftOf() ______________________________________________________
	// _area() of all pieces of edge, weighted by their height. Pixels get the difference
	// between their sides, rounded: (_edgeLeftOf(right side) - _edgeLeftOf(left side) + 32) >> 6.

	inline int64_t SoftShapes::_edgeLeftOf(const Edge& edge, int64_t x) const
	{
		const Piece* pPiece = m_pieces.data() + edge.first;

		int64_t area = 0;
		for (int i = 0; i < edge.n; i++)
			area += pPiece[i].h * _area(pPiece[i], x);

		return area;
	}

	//____ _edgeX() ___________________________________________________________

	template<class Shape>
	inline int64_t SoftShapes::_edgeX(Shape& shape, const EdgeDef& edge, int64_t y)
	{
		if (m_cacheY[edge.id] == y)
			return m_cacheX[edge.id];

		return shape.x(edge, y);
	}

	//____ _mixColor() ________________________________________________________
	// Colors of a drawn segment at x on current row, null if not drawn.

	inline const int16_t* SoftShapes::_mixColor(int segment, int x)
	{
		MixSource& mix = m_mix[segment];

		if (mix.bFlat)
			return mix.p;

		if (mix.row != m_row)
		{
			bool bFlat;
			mix.p = m_pSink->segmentColors(segment, m_row, m_clipBeg, m_clipEnd, bFlat);
			mix.row = m_row;
		}

		return mix.p + (x - m_clipBeg) * 4;
	}

	//____ _fillRun() _________________________________________________________

	inline void SoftShapes::_fillRun(int beg, int end, int segment)
	{
		if (!m_mix[segment].p)
			return;

		if (m_nRuns > 0)
		{
			Run& last = m_runs[m_nRuns - 1];
			if (last.segment == segment && last.end == beg)
			{
				last.end = end;
				return;
			}
		}

		m_runs[m_nRuns++] = { beg, end, segment };
	}

	//____ _mixedPixel() ______________________________________________________

	inline SoftShapes::CoveragePixel& SoftShapes::_mixedPixel(int x)
	{
		if (m_nRuns > 0)
		{
			Run& last = m_runs[m_nRuns - 1];
			if (last.segment < 0 && last.end == x)
			{
				last.end = x + 1;
				return m_pixels[m_nPixels++];
			}
		}

		m_runs[m_nRuns++] = { x, x + 1, -1 };
		return m_pixels[m_nPixels++];
	}

	//____ _mixedRun() ________________________________________________________

	inline void SoftShapes::_mixedRun(int beg, int end, const CoveragePixel& pixel)
	{
		Run* pLast = m_nRuns > 0 ? &m_runs[m_nRuns - 1] : nullptr;

		if (pLast && pLast->segment < 0 && pLast->end == beg)
			pLast->end = end;
		else
			m_runs[m_nRuns++] = { beg, end, -1 };

		CoveragePixel* pPixels = m_pixels.data() + m_nPixels;
		const int n = end - beg;

		if (n <= 8)
		{
			for (int i = 0; i < n; i++)
				pPixels[i] = pixel;
		}
		else
		{
			pPixels[0] = pixel;
			for (int done = 1; done < n; done *= 2)
				std::memcpy(pPixels + done, pPixels, sizeof(CoveragePixel) * std::min(done, n - done));
		}

		m_nPixels += n;
	}

	//____ setMirrorBuffer() _________________________________________________

	void SoftShapes::setMirrorBuffer(int initialBytes, int maxBytes)
	{
		m_mirrorBufferMax = std::min(std::max(maxBytes, 0), INT_MAX / 2);
		initialBytes = std::min(std::max(initialBytes, 0), m_mirrorBufferMax);

		if (m_mirrorBufferSize > m_mirrorBufferMax || m_mirrorBufferSize < initialBytes)
		{
			m_pMirrorBuffer.reset(initialBytes > 0 ? new uint8_t[initialBytes] : nullptr);
			m_mirrorBufferSize = initialBytes;
			m_mirrorBufferUsed = 0;
		}
	}

	//____ _reserveMirrorBuffer() _____________________________________________
	// Makes room for bytes in the mirror buffer, keeping those used, unless above the most allowed.

	bool SoftShapes::_reserveMirrorBuffer(int bytes)
	{
		if (bytes <= m_mirrorBufferSize)
			return true;

		if (bytes > m_mirrorBufferMax)
			return false;

		const int size = std::min(std::max({ bytes, m_mirrorBufferSize * 2, 4096 }), m_mirrorBufferMax);

		uint8_t* p = new uint8_t[size];
		if (m_mirrorBufferUsed > 0)
			std::memcpy(p, m_pMirrorBuffer.get(), m_mirrorBufferUsed);

		m_pMirrorBuffer.reset(p);
		m_mirrorBufferSize = size;
		return true;
	}

	//____ _mirrorRunsX() _____________________________________________________
	/*
		Adds the mirror image of the runs of a row done up to its middle, pixel x of it at
		m_mirrorSumX - 1 - x, as far as m_outEnd. Runs left of m_outBeg, done for their
		mirror image, are skipped by starting at m_firstRun and m_firstPixel.
	*/

	void SoftShapes::_mirrorRunsX()
	{
		const int sum = m_mirrorSumX;
		const int clipBeg = m_outBeg;
		const int clipEnd = m_outEnd;
		const int middle = m_clipEnd;
		const int nRuns = m_nRuns;
		int pixelEnd = m_nPixels;

		for (int i = nRuns - 1; i >= 0; i--)
		{
			const Run run = m_runs[i];
			const int beg = std::max(sum - run.end, std::max(middle, clipBeg));
			const int end = std::min(sum - run.begin, clipEnd);

			if (run.segment >= 0)
			{
				if (beg < end)
					_fillRun(beg, end, run.segment);
				continue;
			}

			const int pixelBeg = pixelEnd - (run.end - run.begin);

			for (int x = beg; x < end; x++)
				_mixedPixel(x) = m_pixels[pixelBeg + (sum - 1 - x - run.begin)];

			pixelEnd = pixelBeg;
		}

		int firstRun = 0;
		int firstPixel = 0;

		while (firstRun < m_nRuns && m_runs[firstRun].end <= clipBeg)
		{
			const Run& run = m_runs[firstRun++];
			if (run.segment < 0)
				firstPixel += run.end - run.begin;
		}

		if (firstRun < m_nRuns && m_runs[firstRun].begin < clipBeg)
		{
			Run& run = m_runs[firstRun];
			if (run.segment < 0)
				firstPixel += clipBeg - run.begin;
			run.begin = clipBeg;
		}

		m_firstRun = firstRun;
		m_firstPixel = firstPixel;
	}

	//____ _accumulate() ______________________________________________________

	inline void SoftShapes::_accumulate(int segment, int value)
	{
		if (segment < 0 || value <= 0)
			return;

		if (m_accumulated[segment] == 0)
			m_touched[m_nTouched++] = segment;

		m_accumulated[segment] += value;
	}

	//____ _rasterize() _______________________________________________________
	/*
		Rasterizes a shape into rows rowBeg up to rowEnd, columns clipBeg up to clipEnd.
	*/

	template<class Shape>
	void SoftShapes::_rasterize(Shape& shape, int nSegments, int rowBeg, int rowEnd, int clipBeg, int clipEnd, RowSink* pSink)
	{
		if (rowBeg >= rowEnd || clipBeg >= clipEnd)
			return;

		m_pSink = pSink;
		m_clipBeg = clipBeg;
		m_clipEnd = clipEnd;

		// Colors of segments. Rows can only be done together if all are flat.

		if ((int)m_mix.size() < nSegments)
			m_mix.resize(nSegments);

		bool bAnyDrawn = false;
		bool bAllFlat = true;

		for (int i = 0; i < nSegments; i++)
		{
			bool bFlat = true;
			const int16_t* p = pSink->segmentColors(i, rowBeg, clipBeg, clipEnd, bFlat);

			m_mix[i] = { p, !p || bFlat, rowBeg };

			if (p)
			{
				bAnyDrawn = true;
				if (!bFlat)
					bAllFlat = false;
			}
		}

		if (!bAnyDrawn)
			return;

		// Rows of shapes symmetric around a pixel boundary or the middle of a pixel, with
		// flat colors, are done up to the middle and mirrored, from further left if clipped
		// more on the right, so that pixels are the same however clipped.

		m_bMirrorX = false;
		m_outBeg = clipBeg;
		m_outEnd = clipEnd;

		if (bAllFlat && shape.bSymmetricX && (shape.symmetrySumX & 63) == 0)
		{
			const int sum = int(shape.symmetrySumX >> 6);
			const int middle = (sum + 1) >> 1;	// Rounded up, also if negative.

			if (clipEnd > middle)
			{
				m_bMirrorX = true;
				m_mirrorSumX = sum;
				m_clipBeg = std::min(clipBeg, sum - clipEnd);
				m_clipEnd = middle;
			}
		}

		const int width = clipEnd - m_clipBeg;

		if ((int)m_runs.size() < width + 1)
		{
			m_runs.resize(width + 1);
			m_pixels.resize(width + 1);
		}

		if ((int)m_accumulated.size() < nSegments)
		{
			m_accumulated.resize(nSegments, 0);
			m_touched.resize(nSegments);
			m_pixelSegments.resize(nSegments);
			m_pixelCoverage.resize(nSegments);
		}

		if ((int)m_edgeDefs.size() < shape.maxEdges)
		{
			m_edgeDefs.resize(shape.maxEdges);
			m_edgeTop.resize(shape.maxEdges);
			m_edgeBottom.resize(shape.maxEdges);
			m_edgeLeftOf.resize(shape.maxEdges);
			m_edgeTopLine.resize(shape.maxEdges);
		}

		m_cacheY.assign(shape.nIds, INT64_MIN);
		m_cacheX.resize(shape.nIds);

		if (m_pieces.size() < 64)
			m_pieces.resize(64);

		m_splitHint = 0;

		// Rows of shapes symmetric around a row boundary or the middle of a row, with flat
		// colors, are done for the upper half and the lower half is their mirror image, also
		// if the upper half is clipped. Upper rows are kept for it, as far as there is room
		// in the mirror buffer, see setMirrorBuffer().

		bool bMirrorY = false;
		int mirrorSumY = 0;						// Row + its mirror image + 1.
		int middleY = 0;
		bool bKeep = false;
		int keptBeg = 0;						// Row of first entry of index of kept rows...
		int nKeptIndex = 0;						// ...and their number.

		if (bAllFlat && shape.bSymmetricY && (shape.symmetrySumY & 63) == 0)
		{
			mirrorSumY = int(shape.symmetrySumY >> 6);
			middleY = (mirrorSumY + 1) >> 1;

			if (rowEnd > middleY)
			{
				bMirrorY = true;
				keptBeg = mirrorSumY - rowEnd;
				nKeptIndex = rowEnd - std::max(rowBeg, middleY);

				const int indexBytes = nKeptIndex * int(sizeof(int));

				m_mirrorBufferUsed = 0;
				bKeep = _reserveMirrorBuffer(indexBytes);
				if (bKeep)
				{
					int* pIndex = reinterpret_cast<int*>(m_pMirrorBuffer.get());
					std::fill(pIndex, pIndex + nKeptIndex, -1);
					m_mirrorBufferUsed = indexBytes;
				}
			}
		}

		auto emit = [&](int row, int rowsEnd, int nRuns, const Run* pRuns, const CoveragePixel* pPixels)
		{
			if (nRuns == 0)
				return;

			if (rowsEnd > row + 1)
				pSink->rows(row, rowsEnd, nRuns, pRuns, pPixels);
			else
				pSink->row(row, nRuns, pRuns, pPixels);
		};

		for (int row = rowBeg; row < rowEnd; )
		{
			int rowsEnd = row + 1;
			if (bAllFlat)
				rowsEnd = std::max(row + 1, std::min(shape.uniformEnd(row), rowEnd));

			if (bMirrorY && row >= middleY)
			{
				const int source = mirrorSumY - 1 - row;
				const int k = source - keptBeg;
				const int kept = (bKeep && k >= 0 && k < nKeptIndex) ? reinterpret_cast<const int*>(m_pMirrorBuffer.get())[k] : -1;

				if (kept >= 0)
				{
					const uint8_t* p = m_pMirrorBuffer.get() + kept;
					const KeptRow& keptRow = *reinterpret_cast<const KeptRow*>(p);
					const Run* pRuns = reinterpret_cast<const Run*>(p + sizeof(KeptRow));
					const CoveragePixel* pPixels = reinterpret_cast<const CoveragePixel*>(p + sizeof(KeptRow) + keptRow.nRuns * sizeof(Run));

					emit(row, rowsEnd, keptRow.nRuns, pRuns, pPixels);
				}
				else
				{
					_rasterizeRow(shape, source);
					emit(row, rowsEnd, m_nRuns - m_firstRun, m_runs.data() + m_firstRun, m_pixels.data() + m_firstPixel);
				}

				row = rowsEnd;
				continue;
			}

			_rasterizeRow(shape, row);

			const int nRuns = m_nRuns - m_firstRun;
			const int nPixels = m_nPixels - m_firstPixel;

			emit(row, rowsEnd, nRuns, m_runs.data() + m_firstRun, m_pixels.data() + m_firstPixel);

			// Kept for the rows mirroring them.

			if (bKeep)
			{
				const int keepBeg = std::max(row - keptBeg, 0);
				const int keepEnd = std::min(std::min(rowsEnd, middleY) - keptBeg, nKeptIndex);
				const int runBytes = nRuns * int(sizeof(Run));
				const int bytes = int(sizeof(KeptRow)) + runBytes + nPixels * int(sizeof(CoveragePixel));

				if (keepBeg < keepEnd && _reserveMirrorBuffer(m_mirrorBufferUsed + bytes))
				{
					uint8_t* p = m_pMirrorBuffer.get() + m_mirrorBufferUsed;

					*reinterpret_cast<KeptRow*>(p) = { nRuns, nPixels };
					std::memcpy(p + sizeof(KeptRow), m_runs.data() + m_firstRun, runBytes);
					std::memcpy(p + sizeof(KeptRow) + runBytes, m_pixels.data() + m_firstPixel, nPixels * sizeof(CoveragePixel));

					int* pIndex = reinterpret_cast<int*>(m_pMirrorBuffer.get());
					for (int i = keepBeg; i < keepEnd; i++)
						pIndex[i] = m_mirrorBufferUsed;

					m_mirrorBufferUsed += bytes;
				}
			}

			row = rowsEnd;
		}
	}

	//____ _rasterizeRow() ____________________________________________________
	// Runs of row in m_runs and m_pixels, from m_firstRun and m_firstPixel.

	template<class Shape>
	void SoftShapes::_rasterizeRow(Shape& shape, int row)
	{
		const int64_t y0 = int64_t(row) * 64;
		const int64_t y1 = y0 + 64;

		m_row = row;

		// Sub-rows, split where edges begin and end.

		const int64_t* pSplits = shape.pSplits;
		const int nSplits = shape.nSplits;

		// First split below top of row, from where the row before ended unless out of order.

		int splitBeg = m_splitHint;

		if (splitBeg > nSplits || (splitBeg > 0 && pSplits[splitBeg - 1] > y0))
			splitBeg = int(std::upper_bound(pSplits, pSplits + nSplits, y0) - pSplits);
		else
		{
			while (splitBeg < nSplits && pSplits[splitBeg] <= y0)
				splitBeg++;
		}

		int splitEnd = splitBeg;
		while (splitEnd < nSplits && pSplits[splitEnd] < y1)
			splitEnd++;

		m_splitHint = splitEnd;

		const int nSubRows = splitEnd - splitBeg + 1;

		if ((int)m_subRows.size() < nSubRows)
			m_subRows.resize(nSubRows);

		if ((int)m_edges.size() < nSubRows * shape.maxEdges)
			m_edges.resize(nSubRows * shape.maxEdges);

		m_nPieces = 0;
		int nEdges = 0;

		for (int k = 0; k < nSubRows; k++)
		{
			const int64_t ya = k == 0 ? y0 : pSplits[splitBeg + k - 1];
			const int64_t yb = k == nSubRows - 1 ? y1 : pSplits[splitBeg + k];

			SubRow& subRow = m_subRows[k];
			subRow.h = int(yb - ya);
			subRow.first = nEdges;
			subRow.n = _subRowEdges(shape, ya, yb, m_edges.data() + nEdges);
			nEdges += subRow.n;
		}

		// Runs

		m_nRuns = 0;
		m_nPixels = 0;

		int nFilled = 0;
		int filled = 0;

		for (int k = 0; k < nSubRows; k++)
			if (m_subRows[k].n > 0)
			{
				nFilled++;
				filled = k;
			}

		if (nFilled == 1)
			_simpleRow(m_edges.data() + m_subRows[filled].first, m_subRows[filled].n, m_subRows[filled].h);
		else if (nFilled > 0)
			_generalRow(nSubRows);

		m_firstRun = 0;
		m_firstPixel = 0;

		if (m_bMirrorX)
			_mirrorRunsX();
	}

	//____ _subRowEdges() _____________________________________________________
	/*
		Edges of a sub-row as polylines, from the top to the bottom of the sub-row, split
		where the shape needs it. Pieces are added to m_pieces.
	*/

	template<class Shape>
	int SoftShapes::_subRowEdges(Shape& shape, int64_t ya, int64_t yb, Edge* pEdges)
	{
		EdgeDef* pDefs = m_edgeDefs.data();
		const int nEdges = shape.edges(ya, yb, pDefs);

		int64_t* pTop = m_edgeTop.data();
		int64_t* pBottom = m_edgeBottom.data();
		int* pTopLine = m_edgeTopLine.data();

		for (int i = 0; i < nEdges; i++)
		{
			const EdgeDef& def = pDefs[i];

			if (def.mirror >= 0)
			{
				pTop[i] = def.mirrorSum - pTop[def.mirror];
				pBottom[i] = def.mirrorSum - pBottom[def.mirror];
			}
			else if (def.pPolyline)
			{
				pTop[i] = _polylineX(*def.pPolyline, ya);
				pTopLine[i] = def.pPolyline->line;
				pBottom[i] = _polylineX(*def.pPolyline, yb);
			}
			else
			{
				pTop[i] = _edgeX(shape, def, ya);
				pBottom[i] = _edgeX(shape, def, yb);
			}

			m_cacheY[def.id] = yb;
			m_cacheX[def.id] = pBottom[i];
		}

		// Clamped edges between their neighbors.

		for (int i = 1; i < nEdges; i++)
			if (pDefs[i].bClamped)
			{
				pTop[i] = std::max(pTop[i], pTop[i - 1]);
				pBottom[i] = std::max(pBottom[i], pBottom[i - 1]);
			}

		for (int i = nEdges - 2; i >= 0; i--)
			if (pDefs[i].bClamped)
			{
				pTop[i] = std::min(pTop[i], pTop[i + 1]);
				pBottom[i] = std::min(pBottom[i], pBottom[i + 1]);
			}

		// Polylines

		const int64_t clipBeg64 = int64_t(m_clipBeg) * 64;
		const int64_t clipEnd64 = int64_t(m_clipEnd) * 64;

		if ((int)m_pieces.size() < m_nPieces + nEdges * c_maxVertices)
			m_pieces.resize(m_nPieces + nEdges * c_maxVertices + 64);

		Piece* pPieces = m_pieces.data();

		auto addPiece = [&](Edge& edge, int64_t yTop, int64_t xTop, int64_t yBottom, int64_t xBottom)
		{
			Piece& piece = pPieces[m_nPieces++];

			const int64_t lo = std::min(xTop, xBottom);
			const int64_t hi = std::max(xTop, xBottom);

			piece.lo = lo;
			piece.hi = hi;
			piece.h = int(yBottom - yTop);
			piece.bandBeg = lo < clipBeg64 ? m_clipBeg - 1 : lo >= clipEnd64 ? m_clipEnd + 1 : int(lo >> 6);
			piece.bandEnd = hi < clipBeg64 ? m_clipBeg - 1 : hi >= clipEnd64 ? m_clipEnd + 1 : int((hi + 63) >> 6);
			piece.inv = (hi > lo && piece.bandEnd > m_clipBeg && piece.bandBeg < m_clipEnd) ? recip37(hi - lo) : 0;

			edge.bandBeg = std::min(edge.bandBeg, piece.bandBeg);
			edge.bandEnd = std::max(edge.bandEnd, piece.bandEnd);
		};

		for (int i = 0; i < nEdges; i++)
		{
			Edge& edge = pEdges[i];
			edge.segment = pDefs[i].segment;
			edge.first = m_nPieces;
			edge.bandBeg = INT_MAX;
			edge.bandEnd = INT_MIN;

			// Mirror of an edge before it, pieces mirrored.

			if (pDefs[i].mirror >= 0)
			{
				const Edge& source = pEdges[pDefs[i].mirror];
				const int64_t sum = pDefs[i].mirrorSum;

				for (int k = 0; k < source.n; k++)
				{
					const Piece& from = pPieces[source.first + k];
					Piece& piece = pPieces[m_nPieces++];

					const int64_t lo = sum - from.hi;
					const int64_t hi = sum - from.lo;

					piece.lo = lo;
					piece.hi = hi;
					piece.h = from.h;
					piece.inv = from.inv;
					piece.bandBeg = lo < clipBeg64 ? m_clipBeg - 1 : lo >= clipEnd64 ? m_clipEnd + 1 : int(lo >> 6);
					piece.bandEnd = hi < clipBeg64 ? m_clipBeg - 1 : hi >= clipEnd64 ? m_clipEnd + 1 : int((hi + 63) >> 6);

					if (hi > lo && piece.inv == 0 && piece.bandEnd > m_clipBeg && piece.bandBeg < m_clipEnd)
						piece.inv = recip37(hi - lo);

					edge.bandBeg = std::min(edge.bandBeg, piece.bandBeg);
					edge.bandEnd = std::max(edge.bandEnd, piece.bandEnd);
				}

				edge.n = m_nPieces - edge.first;
				continue;
			}

			// Polyline, from top through its vertices to bottom.

			if (pDefs[i].pPolyline)
			{
				const Polyline& polyline = *pDefs[i].pPolyline;

				int64_t y = ya;
				int64_t x = pTop[i];

				for (int k = pTopLine[i] + 1; k < polyline.n && polyline.pY[k] < yb; k++)
				{
					if (polyline.pY[k] > y)
					{
						addPiece(edge, y, x, polyline.pY[k], polyline.pX[k]);
						y = polyline.pY[k];
						x = polyline.pX[k];
					}
				}

				addPiece(edge, y, x, yb, pBottom[i]);

				edge.n = m_nPieces - edge.first;
				continue;
			}

			// Vertices still to reach, last one first.

			int64_t stackY[c_maxVertices], stackX[c_maxVertices];
			int nStack = 1;
			stackY[0] = yb;
			stackX[0] = pBottom[i];

			int64_t y = ya;
			int64_t x = pTop[i];

			while (nStack > 0)
			{
				const int64_t y2 = stackY[nStack - 1];
				const int64_t x2 = stackX[nStack - 1];

				int64_t ySplit;
				if (!pDefs[i].bClamped && nStack < c_maxVertices && y2 - y > 1 && shape.split(pDefs[i], y, x, y2, x2, ySplit))
				{
					stackY[nStack] = ySplit;
					stackX[nStack++] = shape.x(pDefs[i], ySplit);
					continue;
				}

				addPiece(edge, y, x, y2, x2);

				y = y2;
				x = x2;
				nStack--;
			}

			edge.n = m_nPieces - edge.first;
		}

		return nEdges;
	}

	//____ _simpleRow() _______________________________________________________
	/*
		A row where only one sub-row has edges, the common case. Pixels between bands of
		partly covered pixels are covered by one segment. In a band of one edge, pixels are
		split between the segments on either side of it, where bands overlap between the
		segments of all edges reaching the pixel.
	*/

	void SoftShapes::_simpleRow(const Edge* pEdges, int nEdges, int h)
	{
		const int clipBeg = m_clipBeg;
		const int clipEnd = m_clipEnd;
		const int full = h * 64;					// Coverage of pixel by the sub-row.

		// Pixels between bands, fully covered if the sub-row is the whole row.

		auto between = [&](int beg, int end, int segment)
		{
			if (full == c_fullCoverage)
				_fillRun(beg, end, segment);
			else
			{
				const int16_t* pColor = _mixColor(segment, beg);
				if (!pColor)
					return;

				auto mixed = [&](const int16_t* pColor) -> CoveragePixel
				{
					return { int16_t((pColor[0] * full) >> 12), int16_t((pColor[1] * full) >> 12), int16_t((pColor[2] * full) >> 12),
							 int16_t((pColor[3] * full) >> 12), int16_t(full), 0 };
				};

				if (m_mix[segment].bFlat)
					_mixedRun(beg, end, mixed(pColor));
				else
				{
					for (int p = beg; p < end; p++)
						_mixedPixel(p) = mixed(_mixColor(segment, p));
				}
			}
		};

		int x = clipBeg;
		int segLeft = -1;

		for (int k = 0; k < nEdges; )
		{
			// Edges with overlapping bands.

			int groupEnd = pEdges[k].bandEnd;
			int j = k + 1;

			while (j < nEdges && pEdges[j].bandBeg < groupEnd)
			{
				groupEnd = std::max(groupEnd, pEdges[j].bandEnd);
				j++;
			}

			const int bandBeg = std::max(pEdges[k].bandBeg, clipBeg);
			const int bandEnd = std::min(groupEnd, clipEnd);

			if (bandBeg > x && x < clipEnd && segLeft >= 0)
				between(x, std::min(bandBeg, clipEnd), segLeft);

			if (j == k + 1)
			{
				// One edge, pixels split between the segments on either side.

				const Edge& edge = pEdges[k];
				const int segRight = edge.segment;
				const Piece& piece = m_pieces[edge.first];
				const bool bStraight = edge.n == 1;

				// Area left of pixel boundaries, unweighted if straight.

				int64_t prevArea = bStraight ? _area(piece, int64_t(bandBeg) * 64) : _edgeLeftOf(edge, int64_t(bandBeg) * 64);

				for (int p = bandBeg; p < bandEnd; p++)
				{
					int right;

					if (bStraight)
					{
						int64_t area = _area(piece, int64_t(p + 1) * 64);
						right = int(area - prevArea);
						prevArea = area;

						if (h != 64)
							right = (right * h + 32) >> 6;
					}
					else
					{
						int64_t area = _edgeLeftOf(edge, int64_t(p + 1) * 64);
						right = int((area - prevArea + 32) >> 6);
						prevArea = area;
					}

					if (right <= 0)
					{
						if (segLeft >= 0)
							between(p, p + 1, segLeft);
						continue;
					}

					if (right >= full)
					{
						if (segRight >= 0)
							between(p, p + 1, segRight);
						continue;
					}

					const int16_t* pLeft = segLeft >= 0 ? _mixColor(segLeft, p) : nullptr;
					const int16_t* pRight = segRight >= 0 ? _mixColor(segRight, p) : nullptr;

					int left = full - right;

					if (!pLeft)
					{
						if (!pRight)
							continue;

						pLeft = c_noColor;
						left = 0;
					}
					else if (!pRight)
					{
						pRight = c_noColor;
						right = 0;
					}

					CoveragePixel& px = _mixedPixel(p);
					px.b = int16_t((pLeft[0] * left + pRight[0] * right) >> 12);
					px.g = int16_t((pLeft[1] * left + pRight[1] * right) >> 12);
					px.r = int16_t((pLeft[2] * left + pRight[2] * right) >> 12);
					px.a = int16_t((pLeft[3] * left + pRight[3] * right) >> 12);
					px.coverage = int16_t(left + right);
				}
			}
			else
			{
				// Several edges. Each segment gets the coverage right of the edge before it
				// minus that right of the edge after it.

				int64_t* pPrevArea = m_edgeLeftOf.data();

				for (int i = k; i < j; i++)
					pPrevArea[i - k] = _edgeLeftOf(pEdges[i], int64_t(bandBeg) * 64);

				for (int p = bandBeg; p < bandEnd; p++)
				{
					int b = 0, g = 0, r = 0, a = 0, c = 0;
					int whole = -2;							// Segment covering all of pixel, -1 if none.

					int prevRight = full;
					int segment = segLeft;

					for (int i = k; i <= j; i++)
					{
						int right = 0;
						if (i < j)
						{
							int64_t area = _edgeLeftOf(pEdges[i], int64_t(p + 1) * 64);
							right = std::min(std::max(int((area - pPrevArea[i - k] + 32) >> 6), 0), prevRight);
							pPrevArea[i - k] = area;
						}

						int coverage = prevRight - right;

						if (coverage > 0 && segment >= 0)
						{
							if (coverage == full)
								whole = segment;
							else if (const int16_t* pColor = _mixColor(segment, p))
							{
								b += pColor[0] * coverage;
								g += pColor[1] * coverage;
								r += pColor[2] * coverage;
								a += pColor[3] * coverage;
								c += coverage;
							}
						}
						else if (coverage == full)
							whole = -1;

						prevRight = right;
						if (i < j)
							segment = pEdges[i].segment;
					}

					if (whole >= 0)
						between(p, p + 1, whole);
					else if (c > 0)
						_mixedPixel(p) = { int16_t(b >> 12), int16_t(g >> 12), int16_t(r >> 12), int16_t(a >> 12), int16_t(c), 0 };
				}
			}

			x = std::max(x, bandEnd);
			segLeft = pEdges[j - 1].segment;
			k = j;
		}

		if (x < clipEnd && segLeft >= 0)
			between(x, clipEnd, segLeft);
	}

	//____ _generalRow() ______________________________________________________
	/*
		A row where several sub-rows have edges. Coverage of each pixel is accumulated per
		segment over the sub-rows, weighted by their height. Stretches of pixels outside all
		bands have the same coverage throughout.
	*/

	void SoftShapes::_generalRow(int nSubRows)
	{
		SubRow* pSubRows = m_subRows.data();
		const Edge* pEdges = m_edges.data();

		for (int k = 0; k < nSubRows; k++)
		{
			pSubRows[k].next = pSubRows[k].first;
			pSubRows[k].segment = -1;
		}

		int x = m_clipBeg;

		while (x < m_clipEnd)
		{
			bool bPartial = false;
			int nextX = m_clipEnd;

			for (int k = 0; k < nSubRows; k++)
			{
				SubRow& subRow = pSubRows[k];
				const int end = subRow.first + subRow.n;

				while (subRow.next < end && pEdges[subRow.next].bandEnd <= x)
					subRow.segment = pEdges[subRow.next++].segment;

				if (subRow.next < end)
				{
					if (pEdges[subRow.next].bandBeg <= x)
						bPartial = true;
					else
						nextX = std::min(nextX, pEdges[subRow.next].bandBeg);
				}
			}

			if (!bPartial)
			{
				for (int k = 0; k < nSubRows; k++)
					_accumulate(pSubRows[k].segment, pSubRows[k].h * c_fullCoverage);

				_emitAccumulated(x, nextX);
				x = nextX;
				continue;
			}

			for (int k = 0; k < nSubRows; k++)
			{
				const SubRow& subRow = pSubRows[k];
				const int end = subRow.first + subRow.n;

				int prevArea = subRow.h * c_fullCoverage;
				int segment = subRow.segment;

				const int64_t x64 = int64_t(x) * 64;

				for (int i = subRow.next; i < end && pEdges[i].bandBeg <= x; i++)
				{
					const int right = int(_edgeLeftOf(pEdges[i], x64 + 64) - _edgeLeftOf(pEdges[i], x64));		// Area of pixel right of edge.
					const int area = std::min(std::max(right, 0), prevArea);
					_accumulate(segment, prevArea - area);
					prevArea = area;
					segment = pEdges[i].segment;
				}

				_accumulate(segment, prevArea);
			}

			_emitAccumulated(x, x + 1);
			x++;
		}
	}

	//____ _emitAccumulated() _________________________________________________
	/*
		Turns coverage accumulated over sub-rows into pixels beg up to end, rounded
		cumulatively so that segments add up to the rounded total.
	*/

	void SoftShapes::_emitAccumulated(int beg, int end)
	{
		int* pSegments = m_pixelSegments.data();
		int* pCoverage = m_pixelCoverage.data();

		int nSegments = 0;
		int full = -1;
		int total = 0;
		int prev = 0;

		for (int i = 0; i < m_nTouched; i++)
		{
			int segment = m_touched[i];
			total += m_accumulated[segment];
			m_accumulated[segment] = 0;

			int rounded = (total + 32) >> 6;
			int coverage = rounded - prev;
			prev = rounded;

			if (coverage == c_fullCoverage)
				full = segment;

			if (coverage > 0)
			{
				pSegments[nSegments] = segment;
				pCoverage[nSegments++] = coverage;
			}
		}

		m_nTouched = 0;

		if (full >= 0)
		{
			_fillRun(beg, end, full);
			return;
		}

		bool bFlat = true;
		for (int i = 0; i < nSegments; i++)
			if (!m_mix[pSegments[i]].bFlat)
				bFlat = false;

		for (int x = beg; x < end; x++)
		{
			int b = 0, g = 0, r = 0, a = 0, c = 0;

			for (int i = 0; i < nSegments; i++)
			{
				const int16_t* pColor = _mixColor(pSegments[i], x);
				if (!pColor)
					continue;

				int coverage = pCoverage[i];
				b += pColor[0] * coverage;
				g += pColor[1] * coverage;
				r += pColor[2] * coverage;
				a += pColor[3] * coverage;
				c += coverage;
			}

			if (c == 0)
				return;

			CoveragePixel px = { int16_t(b >> 12), int16_t(g >> 12), int16_t(r >> 12), int16_t(a >> 12), int16_t(c), 0 };

			if (bFlat)
			{
				_mixedRun(x, end, px);
				return;
			}

			_mixedPixel(x) = px;
		}
	}

	//____ _quarter() _________________________________________________________
	/*
		Vertices of a quarter circle of radius r, as offsets from its center, from 12 o'clock
		to 3 o'clock. Split evenly by angle into chords with a sagitta of at most
		c_maxSagitta, straying at most half of that inside or outside the circle. Kept for
		the last few radii, as boxes mostly have the same corners.
	*/

	const SoftShapes::Quarter& SoftShapes::_quarter(int64_t r)
	{
		for (const Quarter& quarter : m_quarters)
			if (quarter.r == r)
				return quarter;

		Quarter& quarter = m_quarters[m_nextQuarter];
		m_nextQuarter = (m_nextQuarter + 1) % c_quarters;

		// Angle per chord, with sagitta r * (1 - cos(angle / 2)) ~ r * angle^2 / 8. Chords of
		// large corners are longer than pixels, so they stray along more of them and get half
		// the sagitta.

		const int64_t halves = r >= 64 * 64 ? 2 : 1;

		int n = int((isqrt(uint64_t(r * halves / c_maxSagitta)) * 569 + 1023) >> 10);	// pi/2 * sqrt(r / 8s)
		n = std::max(n, 1);

		quarter.r = r;
		quarter.dx.resize(n + 1);
		quarter.dy.resize(n + 1);

		// Vertices between the ends are moved out by half the sagitta, 1/256 spx precision,
		// so that chords stray as much outside as inside instead of always inside.

		const int64_t cosHalfStep = _sin30(uint32_t((int64_t(1) << 29) / n) + 0x40000000);
		const int64_t rOut = (r << 39) / ((int64_t(1) << 30) + cosHalfStep);		// 2r / (1 + cos(angle / 2))

		for (int k = 0; k <= n; k++)
		{
			const uint32_t angle = uint32_t((int64_t(k) << 30) / n);

			if (k == 0 || k == n)
			{
				quarter.dx[k] = (r * _sin30(angle) + (1 << 29)) >> 30;
				quarter.dy[k] = (r * _sin30(angle + 0x40000000) + (1 << 29)) >> 30;
			}
			else
			{
				quarter.dx[k] = (rOut * _sin30(angle) + (int64_t(1) << 37)) >> 38;
				quarter.dy[k] = (rOut * _sin30(angle + 0x40000000) + (int64_t(1) << 37)) >> 38;
			}
		}

		return quarter;
	}

	//____ BoxShape ___________________________________________________________
	/*
		Box with rounded corners and an outline inside it, as two contours: the outer edge
		and the inner edge of the outline.

		Regular boxes, where the inner contour stays inside the outer one and doesn't close,
		as with even outlines, have their edges flattened into polylines in advance, the
		corners as from _quarter(). Right edges with the same corners as the left are their
		mirror images.

		Other boxes have edges evaluated row by row. Along a row, a contour reaches from its
		left edge, the rightmost of its left side and the left corners whose quarter the row
		is in, to its right edge, the leftmost of its right side and right corners. The fill
		reaches from the rightmost of the left edges of both contours to the leftmost of
		their right edges, so where an inner corner reaches past the outer edge, as with
		uneven outlines, the outline just gets thin. Where left and right edge of a contour
		pass each other it is closed. Edges are split where corners begin and end, and
		bisected where they bend too much or kink, where another side or corner takes over
		or the contour closes.

		Edge ids: 0 outer left, 1 outer right, 2 inner left, 3 inner right.
	*/

	class SoftShapes::BoxShape : public SoftShapes::EdgeShape
	{
	public:

		struct Contour
		{
			bool		bExists;
			int64_t		left, top, right, bottom;
			int64_t		r[4];					// Top-left, top-right, bottom-right, bottom-left. 0 for square corner.
			int64_t		cx[4], cy[4];			// Center of circle.
			uint64_t	r2[4];
			int64_t		limit[4];				// Most squared length of a chord.
		};

		// Edges of both contours at y, with the candidates they are the max (left) or min
		// (right) of: side, top corner and bottom corner.

		struct Eval
		{
			int64_t		y;
			int64_t		cand[2][2][3];			// Contour, left/right, candidate.
			bool		bValid[2][2][3];
			int64_t		x[4];					// Edges, see ids.
			bool		bClosed[2];				// Contour closed, left and right edge put between them.
		};

		Contour		outer;
		Contour		inner;
		bool		bOutline;
		bool		bRegular;					// Inner contour stays inside outer and doesn't close, as with even outlines.
		Polyline*	pPolylines[4] = { nullptr, nullptr, nullptr, nullptr };	// Edges flattened in advance, by id.

		int			nEvents = 0;
		int64_t		eventBeg[12], eventEnd[12];	// Where rows aren't all the same, see uniformEnd().

		static constexpr int	c_evals = 4;
		Eval		evals[c_evals];				// Recently evaluated, see eval().
		int			nextEval = 0;

		BoxShape()
		{
			for (auto& e : evals)
				e.y = INT64_MIN;
		}

		//____ setupContour() _________________________________________________

		static void setupContour(Contour& c, const RectSPX& rect, const spx radius[4])
		{
			c.bExists = rect.w > 0 && rect.h > 0;
			c.left = rect.x;
			c.top = rect.y;
			c.right = int64_t(rect.x) + rect.w;
			c.bottom = int64_t(rect.y) + rect.h;

			for (int i = 0; i < 4; i++)
			{
				int64_t r = radius[i] >= 4 ? radius[i] : 0;			// Smaller corners are square, within 1/256 pixel.
				c.r[i] = r;
				c.r2[i] = uint64_t(r) * uint64_t(r);
				c.limit[i] = 8 * r * c_maxSagitta;
				c.cx[i] = (i == 0 || i == 3) ? c.left + r : c.right - r;
				c.cy[i] = (i == 0 || i == 1) ? c.top + r : c.bottom - r;
			}
		}

		//____ flatten() ______________________________________________________
		/*
			Adds vertices of the left or right edge of a contour to y and x, from top to
			bottom: the corner on top, the side and the corner at the bottom, corners as
			from _quarter().
		*/

		static void flatten(SoftShapes& shapes, const Contour& c, bool bLeft, std::vector<int64_t>& vy, std::vector<int64_t>& vx)
		{
			const int top = bLeft ? 0 : 1;
			const int bottom = bLeft ? 3 : 2;
			const size_t first = vy.size();

			auto add = [&](int64_t y, int64_t x)
			{
				if (vy.size() == first || y > vy.back())
				{
					vy.push_back(y);
					vx.push_back(x);
				}
			};

			auto corner = [&](int i, bool bTop)
			{
				const Quarter& quarter = shapes._quarter(c.r[i]);
				const int n = (int)quarter.dx.size() - 1;

				for (int j = 0; j <= n; j++)
				{
					const int k = bTop ? j : n - j;		// From 12 or 6 o'clock to the side.
					add(bTop ? c.cy[i] - quarter.dy[k] : c.cy[i] + quarter.dy[k], bLeft ? c.cx[i] - quarter.dx[k] : c.cx[i] + quarter.dx[k]);
				}
			};

			const int64_t side = bLeft ? c.left : c.right;

			if (c.r[top] > 0)
				corner(top, true);
			else
				add(c.top, side);

			if (c.r[bottom] > 0)
				corner(bottom, false);
			else
				add(c.bottom, side);
		}

		//____ addEvents() ____________________________________________________
		// Splits where contours begin and end, events for uniformEnd().

		void addEvents(const Contour& c, std::vector<int64_t>& splits)
		{
			if (!c.bExists)
				return;

			splits.push_back(c.top);
			splits.push_back(c.bottom);
			addEvent(c.top, c.top);
			addEvent(c.bottom, c.bottom);

			for (int i = 0; i < 4; i++)
			{
				if (c.r[i] == 0)
					continue;

				if (i == 0 || i == 1)
					addEvent(c.top, std::min(c.cy[i], c.bottom));
				else
					addEvent(std::max(c.cy[i], c.top), c.bottom);
			}
		}

		void addEvent(int64_t beg, int64_t end)
		{
			eventBeg[nEvents] = beg;
			eventEnd[nEvents++] = end;
		}

		//____ eval() _________________________________________________________
		// Edges at y, kept for the last few y as edges are evaluated at the same y in turn.

		const Eval& eval(int64_t y)
		{
			for (auto& e : evals)
				if (e.y == y)
					return e;

			Eval& e = evals[nextEval];
			nextEval = (nextEval + 1) % c_evals;

			e.y = y;
			evalContour(outer, 0, y, e);

			int64_t& left = e.x[0];
			int64_t& right = e.x[1];
			e.bClosed[0] = closeContour(left, right, INT64_MIN, INT64_MAX);

			if (bOutline && inner.bExists && y >= inner.top && y <= inner.bottom)
			{
				evalContour(inner, 1, y, e);

				int64_t& innerLeft = e.x[2];
				int64_t& innerRight = e.x[3];
				innerLeft = std::max(innerLeft, left);
				innerRight = std::min(innerRight, right);
				e.bClosed[1] = closeContour(innerLeft, innerRight, left, right);
			}
			else
				e.bClosed[1] = false;

			return e;
		}

		static void evalContour(const Contour& c, int k, int64_t y, Eval& e)
		{
			for (int side = 0; side < 2; side++)
			{
				const bool bLeft = side == 0;
				const int corners[2] = { bLeft ? 0 : 1, bLeft ? 3 : 2 };

				int64_t* pCand = e.cand[k][side];
				bool* pValid = e.bValid[k][side];

				pCand[0] = bLeft ? c.left : c.right;
				pValid[0] = true;

				int64_t x = pCand[0];

				for (int j = 0; j < 2; j++)
				{
					const int i = corners[j];
					const bool bInQuarter = c.r[i] > 0 && (j == 0 ? y <= c.cy[i] : y >= c.cy[i]) && y >= c.top && y <= c.bottom;

					pValid[1 + j] = bInQuarter;

					if (bInQuarter)
					{
						int64_t dy = y - c.cy[i];
						int64_t dx = isqrt(c.r2[i] - uint64_t(dy * dy));
						int64_t cand = bLeft ? c.cx[i] - dx : c.cx[i] + dx;

						pCand[1 + j] = cand;
						x = bLeft ? std::max(x, cand) : std::min(x, cand);
					}
				}

				e.x[k * 2 + side] = x;
			}
		}

		// Where left and right edge have passed each other the contour is closed, both are
		// put between them, within outer left and right.

		static bool closeContour(int64_t& left, int64_t& right, int64_t outerLeft, int64_t outerRight)
		{
			if (left <= right)
				return false;

			left = right = std::min(std::max((left + right) >> 1, outerLeft), outerRight);
			return true;
		}

		//____ edges() ________________________________________________________

		int edges(int64_t ya, int64_t yb, EdgeDef* pEdges) const
		{
			if (!within(outer, ya, yb))
				return 0;

			int n = 0;
			edge(pEdges[n++], 0, bOutline ? 1 : 0);

			if (bOutline && within(inner, ya, yb))
			{
				edge(pEdges[n++], 2, 0);
				edge(pEdges[n++], 3, 1);
				mirror(pEdges[n - 1], inner, n - 2);
			}

			edge(pEdges[n++], 1, -1);
			mirror(pEdges[n - 1], outer, 0);
			return n;
		}

		// Right edge of a regular contour with corners the same as on the left is the mirror
		// image of the left edge.

		void mirror(EdgeDef& def, const Contour& c, int left) const
		{
			if (bRegular && c.r[0] == c.r[1] && c.r[3] == c.r[2])
			{
				def.mirror = left;
				def.mirrorSum = c.left + c.right;
			}
		}

		bool within(const Contour& c, int64_t ya, int64_t yb) const
		{
			return c.bExists && ya >= c.top && yb <= c.bottom;
		}

		void edge(EdgeDef& def, int id, int segment) const
		{
			def.id = id;
			def.segment = segment;
			def.bClamped = false;
			def.mirror = -1;
			def.pPolyline = pPolylines[id] && pPolylines[id]->n > 0 ? pPolylines[id] : nullptr;
		}

		//____ x() ____________________________________________________________

		int64_t x(const EdgeDef& edge, int64_t y)
		{
			return eval(y).x[edge.id];
		}

		//____ split() ________________________________________________________

		bool split(const EdgeDef& edge, int64_t ya, int64_t xa, int64_t yb, int64_t xb, int64_t& y)
		{
			const int k = edge.id >> 1;				// Contour
			const int side = edge.id & 1;
			const bool bLeft = side == 0;

			// Corners begin and end.

			const int corners[2] = { bLeft ? 0 : 1, bLeft ? 3 : 2 };

			for (int c = 0; c <= k; c++)
			{
				const Contour& contour = c == 0 ? outer : inner;

				for (int i : corners)
				{
					if (contour.r[i] > 0 && contour.cy[i] > ya && contour.cy[i] < yb)
					{
						y = contour.cy[i];
						return true;
					}
				}
			}

			const Eval a = eval(ya);
			const Eval& b = eval(yb);

			// Contour closes or opens.

			if (a.bClosed[0] != b.bClosed[0] || (k == 1 && a.bClosed[1] != b.bClosed[1]))
			{
				y = splitPoint(ya, yb);
				return true;
			}

			// The same candidate must be the edge at both ends, otherwise the edge kinks.
			// The max (or min) of the candidates before closing is the edge, unless closed.

			if ((k == 0 && a.bClosed[0]) || (k == 1 && (a.bClosed[0] || a.bClosed[1])))
				return false;

			int64_t limit = -1;

			for (int c = 0; c <= k && limit < 0; c++)
			{
				for (int j = 0; j < 3; j++)
				{
					if (a.bValid[c][side][j] && b.bValid[c][side][j] && a.cand[c][side][j] == xa && b.cand[c][side][j] == xb)
					{
						limit = j == 0 ? 0 : (c == 0 ? outer : inner).limit[corners[j - 1]];
						if (limit == 0)
							break;
					}
				}
			}

			if (limit < 0 || (limit > 0 && yb - ya > 4 && (xa - xb) * (xa - xb) + (yb - ya) * (yb - ya) > limit))
			{
				y = splitPoint(ya, yb);
				return true;
			}

			return false;
		}

		//____ uniformEnd() ___________________________________________________
		// Rows not reached by corners and not cut by the top or bottom of a contour are the same.

		int uniformEnd(int row) const
		{
			const int64_t y0 = int64_t(row) * 64;
			const int64_t y1 = y0 + 64;

			int end = INT_MAX;

			for (int i = 0; i < nEvents; i++)
			{
				if (eventBeg[i] < y1 && eventEnd[i] > y0)
					return row + 1;

				if (eventBeg[i] >= y1)
					end = std::min(end, floorPixel(eventBeg[i]));
			}

			return end;
		}
	};

	//____ rasterizeRoundedBox() ______________________________________________

	void SoftShapes::rasterizeRoundedBox(const RoundedBox& box, const RectI& clip, RowSink* pSink)
	{
		const RectSPX& rect = box.rect;

		if (rect.w <= 0 || rect.h <= 0 || clip.w <= 0 || clip.h <= 0)
			return;

		spx radius[4] = { box.radius[0], box.radius[1], box.radius[2], box.radius[3] };
		Util::clampCornerRadii(rect.size(), radius);

		BorderSPX outline = { std::max(box.outline.top, 0), std::max(box.outline.right, 0),
							  std::max(box.outline.bottom, 0), std::max(box.outline.left, 0) };

		BoxShape shape;
		shape.bOutline = (outline.top | outline.right | outline.bottom | outline.left) != 0;

		BoxShape::setupContour(shape.outer, rect, radius);

		if (shape.bOutline)
		{
			RectSPX innerRect(rect.x + outline.left, rect.y + outline.top,
							  rect.w - outline.left - outline.right, rect.h - outline.top - outline.bottom);

			spx innerRadius[4];
			Util::innerCornerRadii(radius, outline, innerRadius);

			BoxShape::setupContour(shape.inner, innerRect, innerRadius);

			// With an even outline, the inner contour stays inside the outer one. It doesn't
			// close either, unless its corners reach past each other.

			const int64_t* r = shape.inner.r;
			const int64_t w = innerRect.w, h = innerRect.h;

			shape.bRegular = outline.top == outline.right && outline.top == outline.bottom && outline.top == outline.left &&
							 r[0] + r[1] <= w && r[3] + r[2] <= w && r[0] + r[3] <= h && r[1] + r[2] <= h;
		}
		else
		{
			shape.inner.bExists = false;
			shape.bRegular = true;
		}

		// Regular boxes have their edges flattened in advance, unless corners are too large,
		// right edges mirrored from left if corners are the same.

		Polyline polylines[4];

		bool bFlatten = shape.bRegular;
		for (int i = 0; i < 4; i++)
			if (shape.outer.r[i] > c_maxFlatRadius)
				bFlatten = false;

		if (bFlatten)
		{
			m_polylineY.clear();
			m_polylineX.clear();

			int ofs[5] = { 0, 0, 0, 0, 0 };

			for (int id = 0; id < 4; id++)
			{
				const BoxShape::Contour& c = id < 2 ? shape.outer : shape.inner;
				const bool bLeft = (id & 1) == 0;
				const bool bMirrored = !bLeft && c.r[0] == c.r[1] && c.r[3] == c.r[2];

				if (c.bExists && !bMirrored)
					BoxShape::flatten(*this, c, bLeft, m_polylineY, m_polylineX);

				ofs[id + 1] = (int)m_polylineY.size();
			}

			for (int id = 0; id < 4; id++)
			{
				polylines[id] = { m_polylineY.data() + ofs[id], m_polylineX.data() + ofs[id], ofs[id + 1] - ofs[id], 0 };
				shape.pPolylines[id] = &polylines[id];
			}
		}

		// Symmetric if regular with the same corners on both sides, or top and bottom.

		auto isSymmetricX = [&](const BoxShape::Contour& c)
		{
			return !c.bExists || (c.r[0] == c.r[1] && c.r[3] == c.r[2] && c.left + c.right == shape.outer.left + shape.outer.right);
		};

		if (shape.bRegular && isSymmetricX(shape.outer) && isSymmetricX(shape.inner))
		{
			shape.bSymmetricX = true;
			shape.symmetrySumX = shape.outer.left + shape.outer.right;
		}

		auto isSymmetricY = [&](const BoxShape::Contour& c)
		{
			return !c.bExists || (c.r[0] == c.r[3] && c.r[1] == c.r[2] && c.top + c.bottom == shape.outer.top + shape.outer.bottom);
		};

		if (shape.bRegular && isSymmetricY(shape.outer) && isSymmetricY(shape.inner))
		{
			shape.bSymmetricY = true;
			shape.symmetrySumY = shape.outer.top + shape.outer.bottom;
		}

		m_splits.clear();
		shape.addEvents(shape.outer, m_splits);
		shape.addEvents(shape.inner, m_splits);

		std::sort(m_splits.begin(), m_splits.end());
		m_splits.erase(std::unique(m_splits.begin(), m_splits.end()), m_splits.end());

		shape.pSplits = m_splits.data();
		shape.nSplits = (int)m_splits.size();
		shape.maxEdges = 4;
		shape.nIds = 4;

		int clipBeg = std::max(clip.x, floorPixel(rect.x));
		int clipEnd = std::min(clip.x + clip.w, ceilPixel(int64_t(rect.x) + rect.w));
		int rowBeg = std::max(clip.y, floorPixel(rect.y));
		int rowEnd = std::min(clip.y + clip.h, ceilPixel(int64_t(rect.y) + rect.h));

		_rasterize(shape, 2, rowBeg, rowEnd, clipBeg, clipEnd, pSink);
	}

	//____ _sin30() ___________________________________________________________
	// Sine in 2.30 fixed point, of angle where 2^32 is a full turn.

	int32_t SoftShapes::_sin30(uint32_t angle)
	{
		uint32_t quadrant = angle >> 30;
		uint32_t a = angle & 0x3FFFFFFF;

		if (quadrant & 1)
			a = 0x40000000 - a;

		uint32_t idx = a >> 20;
		uint32_t frac = a & 0xFFFFF;

		int32_t v0 = c_sinTable.v[idx];
		int32_t v1 = c_sinTable.v[idx + 1];
		int32_t v = v0 + int32_t((int64_t(v1 - v0) * frac) >> 20);

		return quadrant >= 2 ? -v : v;
	}

	//____ _isBefore() ________________________________________________________
	/*
		Tells if direction a (y pointing down) comes before direction b clockwise from
		12 o'clock. Needs no trigonometry or division, just the half turn each is in and
		the sign of their cross product.
	*/

	inline bool SoftShapes::_isBefore(int64_t ax, int64_t ay, int64_t bx, int64_t by)
	{
		bool aFirstHalf = ax > 0 || (ax == 0 && ay < 0);			// From 12 o'clock up to 6 o'clock.
		bool bFirstHalf = bx > 0 || (bx == 0 && by < 0);

		if (aFirstHalf != bFirstHalf)
			return aFirstHalf;

		return ax * by - ay * bx > 0;
	}

	//____ _setupRingAngles() _________________________________________________
	/*
		Splits the turn into elements: the slices, followed by a gap (back segment) if they
		don't make a full turn. Each element starts at a boundary.
	*/

	void SoftShapes::_setupRingAngles(const Ring& ring)
	{
		m_boundaries.clear();
		m_elementSegments.clear();
		m_crossings[0].clear();
		m_crossings[1].clear();

		const uint64_t fullTurn = uint64_t(1) << 32;

		std::vector<uint64_t>& starts = m_elementStarts;
		starts.clear();

		uint64_t pos = 0;
		for (int i = 0; i < ring.nSlices && pos < fullTurn; i++)
		{
			uint64_t size = uint64_t(std::min(ring.pSliceSizes[i], c_fullTurn)) << 1;
			if (size == 0)
				continue;

			starts.push_back(pos);
			m_elementSegments.push_back(c_ringFirstSliceSegment + i);
			pos += size;
		}

		if (pos < fullTurn)
		{
			starts.push_back(pos);
			m_elementSegments.push_back(c_ringBackSegment);
		}

		int nElements = (int)starts.size();
		if (nElements <= 1)
			return;

		uint32_t start = (ring.start % c_fullTurn) << 1;

		m_boundaries.resize(nElements);

		for (int e = 0; e < nElements; e++)
		{
			Boundary& b = m_boundaries[e];
			b.angle = uint32_t(start + starts[e]);
			b.element = e;
			b.prevElement = (e == 0) ? nElements - 1 : e - 1;
		}

		std::sort(m_boundaries.begin(), m_boundaries.end(), [](const Boundary& a, const Boundary& b) { return a.angle < b.angle; });

		for (auto& b : m_boundaries)
		{
			b.sx = _sin30(b.angle);
			b.sy = -_sin30(b.angle + 0x40000000);

			// Rays too close to horizontal to cross rows within any canvas, except at the center,
			// are taken as horizontal.

			b.bFlat = std::abs(b.sy) < 1024;

			if (!b.bFlat)
			{
				int64_t slope = (int64_t(b.sx) * c_one) / b.sy;		// 32.32
				b.slopeInt = slope >> 32;
				b.slopeFrac = uint32_t(slope & 0xFFFFFFFF);
			}
		}

		// Boundaries crossing rows above and below the center, in order of their crossings
		// from left to right. Rays from the center don't cross, so that is clockwise from
		// 9 o'clock above and counter-clockwise from 9 o'clock below.

		int nBoundaries = (int)m_boundaries.size();

		for (int pass = 0; pass < 2; pass++)
			for (int i = 0; i < nBoundaries; i++)
			{
				const Boundary& b = m_boundaries[i];
				if (!b.bFlat && b.sy < 0 && (b.angle >= 0xC0000000) == (pass == 0))
					m_crossings[0].push_back(i);
			}

		for (int i = nBoundaries - 1; i >= 0; i--)
		{
			const Boundary& b = m_boundaries[i];
			if (!b.bFlat && b.sy > 0)
				m_crossings[1].push_back(i);
		}
	}

	//____ _elementAt() _______________________________________________________

	// Element in direction x, y from the center. The center itself counts as 12 o'clock.

	int SoftShapes::_elementAt(int64_t x, int64_t y) const
	{
		if (m_boundaries.empty())
			return 0;

		if (x == 0 && y == 0)
			y = -1;

		// Last boundary at or before the direction, wrapping around to the last one.

		int lo = 0, hi = (int)m_boundaries.size();
		while (lo < hi)
		{
			int mid = (lo + hi) / 2;
			const Boundary& b = m_boundaries[mid];
			if (!_isBefore(x, y, b.sx, b.sy))
				lo = mid + 1;
			else
				hi = mid;
		}

		int idx = lo - 1;
		if (idx < 0)
			idx = (int)m_boundaries.size() - 1;

		return m_boundaries[idx].element;
	}

	//____ RingShape __________________________________________________________
	/*
		Ring as up to four circles, with slices between the middle two and rays between the
		slices. Circle 0 is the outer edge, 1 the inside of the outer outline, 2 the outside
		of the inner outline and 3 the inner edge. Without outline, 1 and 2 are left out and
		0 and 3 limit the slices. In a square ring 0 and 1 are rectangles.

		A ray crosses rows where it is between the circles limiting the slices. Along a row
		in the upper half they come clockwise, in the lower half counter-clockwise, and the
		slice right of each is known. Rows are split where rays end and at the center, so
		within a sub-row each ray is either left of the slices, between them, in the hole
		or right of them.

		Edge ids: 0-3 left side of circle, 4-7 right side, 8 and 9 left and right side of
		rect, then the rays above the center followed by those below.
	*/

	class SoftShapes::RingShape : public SoftShapes::EdgeShape
	{
	public:

		int64_t		cx, cy;
		int64_t		left, top, right, bottom;	// Rect.
		bool		bBack, bSquare, bHole, bOutline, bSlices;

		int64_t		radius[4];					// 0 for no circle.
		uint64_t	radius2[4];
		int64_t		limit[4];					// Most squared length of a chord.

		int64_t		sqLeft[2], sqTop[2], sqRight[2], sqBottom[2];	// Square ring.

		int			outerLimit, innerLimit;		// Circles limiting slices.

		const Ray*	pRays[2];
		int			nRays[2];
		int			startSegment[2];			// Segment at left end of rows above and below center.

		int64_t		halfWidthY[4] = { INT64_MIN, INT64_MIN, INT64_MIN, INT64_MIN };	// Last half width of each circle and its y.
		int64_t		lastHalfWidth[4];

		static constexpr int	c_firstRayId = 10;

		//____ halfWidth() ____________________________________________________

		int64_t halfWidth(int i, int64_t y)
		{
			if (halfWidthY[i] != y)
			{
				int64_t r = radius[i];
				int64_t dy = y - cy;
				halfWidthY[i] = y;
				lastHalfWidth[i] = (dy <= -r || dy >= r) ? 0 : isqrt(radius2[i] - uint64_t(dy * dy));
			}

			return lastHalfWidth[i];
		}

		bool isCircle(int id) const
		{
			return id < 8 && !(bSquare && (id & 3) < 2);
		}

		//____ x() ____________________________________________________________

		int64_t x(const EdgeDef& edge, int64_t y)
		{
			const int id = edge.id;

			if (id < 8)
			{
				const int i = id & 3;
				const bool bLeft = id < 4;

				if (bSquare && i < 2)
					return bLeft ? sqLeft[i] : sqRight[i];

				int64_t w = halfWidth(i, y);
				return bLeft ? cx - w : cx + w;
			}

			if (id < c_firstRayId)
				return id == 8 ? left : right;

			int ray = id - c_firstRayId;
			const Ray& r = ray < nRays[0] ? pRays[0][ray] : pRays[1][ray - nRays[0]];

			int64_t dy = y - cy;
			return cx + dy * r.slopeInt + mulFrac(dy, r.slopeFrac);
		}

		int uniformEnd(int row) const { return row + 1; }

		//____ split() ________________________________________________________
		// Circles where chords get too long, everything else is straight.

		bool split(const EdgeDef& edge, int64_t ya, int64_t xa, int64_t yb, int64_t xb, int64_t& y) const
		{
			const int64_t lim = isCircle(edge.id) ? limit[edge.id & 3] : 0;
			const int64_t h = yb - ya;

			if (lim == 0 || h <= 4 || (xa - xb) * (xa - xb) + h * h <= lim)
				return false;

			y = splitPoint(ya, yb);
			return true;
		}

		//____ edges() ________________________________________________________

		bool within(int i, int64_t ya, int64_t yb) const
		{
			if (bSquare && i < 2)
				return ya >= sqTop[i] && yb <= sqBottom[i];

			return radius[i] > 0 && ya >= cy - radius[i] && yb <= cy + radius[i];
		}

		int edges(int64_t ya, int64_t yb, EdgeDef* pEdges) const
		{
			const int half = ya < cy ? 0 : 1;
			const Ray* pRay = pRays[half];
			const int nRay = nRays[half];
			const int firstId = c_firstRayId + (half == 0 ? 0 : nRays[0]);

			int n = 0;

			int leftOfCircle[4] = { -1, -1, -1, -1 };		// Where left side of each circle is.

			auto put = [&](int id, int segment)
			{
				int mirror = -1;
				if (isCircle(id))
				{
					if (id < 4)
						leftOfCircle[id & 3] = n;
					else
						mirror = leftOfCircle[id & 3];
				}

				pEdges[n++] = { id, segment, false, mirror, cx * 2, nullptr };
			};

			auto putRay = [&](int i)
			{
				pEdges[n++] = { firstId + i, pRay[i].segment, true, -1, 0, nullptr };
			};

			// 0: beyond the slices, 1: between, 2: in the hole.

			auto rayClass = [&](const Ray& ray) -> int
			{
				if (half == 0)
					return yb <= ray.yOuter ? 0 : (bHole && ya >= ray.yInner) ? 2 : 1;
				else
					return ya >= ray.yOuter ? 0 : (bHole && yb <= ray.yInner) ? 2 : 1;
			};

			const bool bInRect = bBack && !bSquare && ya >= top && yb <= bottom;
			const int segmentRight = (bBack && !bSquare) ? c_ringBackSegment : -1;

			if (bInRect)
				put(8, c_ringBackSegment);

			if (within(0, ya, yb))
			{
				const bool bInHub = bHole && within(3, ya, yb);

				if (!bSlices)
				{
					put(0, c_ringOutlineSegment);

					if (bInHub)
					{
						put(3, c_ringHubSegment);
						put(7, c_ringOutlineSegment);
					}
				}
				else
				{
					const bool bInSlices = within(outerLimit, ya, yb);
					const bool bInHole = bInSlices && bHole && within(innerLimit, ya, yb);

					int segment = startSegment[half];
					int i = 0;

					if (bOutline)
						put(0, c_ringOutlineSegment);

					if (bInSlices)
					{
						while (i < nRay && pRay[i].bLeft && rayClass(pRay[i]) == 0)
							segment = pRay[i++].segment;

						put(outerLimit, segment);

						if (bInHole)
						{
							while (i < nRay && pRay[i].bLeft && rayClass(pRay[i]) == 1)
							{
								putRay(i);
								segment = pRay[i++].segment;
							}

							while (i < nRay && rayClass(pRay[i]) == 2)
								segment = pRay[i++].segment;

							if (bOutline)
							{
								put(2, c_ringOutlineSegment);

								if (bInHub)
								{
									put(3, c_ringHubSegment);
									put(7, c_ringOutlineSegment);
								}

								put(6, segment);
							}
							else
							{
								put(3, c_ringHubSegment);
								put(7, segment);
							}
						}

						for (; i < nRay; i++)
							if (rayClass(pRay[i]) == 1)
								putRay(i);

						if (bOutline)
							put(5, c_ringOutlineSegment);
					}
				}

				put(4, segmentRight);
			}

			if (bInRect)
				put(9, -1);

			return n;
		}
	};

	//____ rasterizeRing() ____________________________________________________

	void SoftShapes::rasterizeRing(const Ring& ring, const RectI& clip, RowSink* pSink)
	{
		const RectSPX& rect = ring.rect;

		if (rect.w <= 0 || rect.h <= 0 || clip.w <= 0 || clip.h <= 0)
			return;

		RingShape shape;

		shape.cx = int64_t(rect.x) + rect.w / 2;
		shape.cy = int64_t(rect.y) + rect.h / 2;
		shape.left = rect.x;
		shape.top = rect.y;
		shape.right = int64_t(rect.x) + rect.w;
		shape.bottom = int64_t(rect.y) + rect.h;

		const int64_t cx = shape.cx, cy = shape.cy;

		// Radial geometry

		const int64_t outerRadius = std::min(rect.w, rect.h) / 2;

		const bool bHole = ring.thickness > 0 && ring.thickness < outerRadius;
		const int64_t innerRadius = bHole ? outerRadius - ring.thickness : 0;

		int64_t outline = std::max(ring.outline, 0);
		outline = std::min(outline, bHole ? int64_t(ring.thickness / 2) : outerRadius);

		shape.bBack = ring.bBack;
		shape.bSquare = ring.bSquare;
		shape.bHole = bHole;
		shape.bOutline = outline > 0;

		int64_t* radius = shape.radius;

		radius[0] = ring.bSquare ? 0 : outerRadius;
		radius[1] = (shape.bOutline && !ring.bSquare) ? std::max(outerRadius - outline, int64_t(0)) : 0;
		radius[2] = (shape.bOutline && bHole) ? innerRadius + outline : 0;
		radius[3] = innerRadius;

		for (int i = 0; i < 4; i++)
		{
			shape.radius2[i] = uint64_t(radius[i]) * uint64_t(radius[i]);
			shape.limit[i] = 8 * radius[i] * c_maxSagitta;
		}

		for (int i = 0; i < 2; i++)
		{
			int64_t inset = i == 0 ? 0 : outline;
			shape.sqLeft[i] = shape.left + inset;
			shape.sqTop[i] = shape.top + inset;
			shape.sqRight[i] = shape.right - inset;
			shape.sqBottom[i] = shape.bottom - inset;
		}

		shape.outerLimit = shape.bOutline ? 1 : 0;
		shape.innerLimit = shape.bOutline ? 2 : 3;

		// Slices between circles outerLimit and innerLimit.

		if (ring.bSquare)
			shape.bSlices = shape.sqRight[shape.outerLimit] > shape.sqLeft[shape.outerLimit] && shape.sqBottom[shape.outerLimit] > shape.sqTop[shape.outerLimit];
		else
			shape.bSlices = radius[shape.outerLimit] > 0;

		if (bHole)
		{
			int64_t holeRadius = radius[shape.innerLimit];
			int64_t sliceRadius = ring.bSquare ? std::min(cx - shape.sqLeft[shape.outerLimit], cy - shape.sqTop[shape.outerLimit]) : radius[shape.outerLimit];
			if (sliceRadius <= holeRadius)
				shape.bSlices = false;
		}

		// Rays

		_setupRingAngles(ring);

		for (int half = 0; half < 2; half++)
		{
			auto& rays = m_rays[half];
			rays.clear();

			for (int idx : m_crossings[half])
			{
				const Boundary& b = m_boundaries[idx];

				Ray ray;
				ray.slopeInt = b.slopeInt;
				ray.slopeFrac = b.slopeFrac;
				ray.segment = m_elementSegments[half == 0 ? b.element : b.prevElement];
				ray.bLeft = b.sx < 0;

				// Where the ray leaves the slices, through the circle or a side of the square.

				if (ring.bSquare)
				{
					int o = shape.outerLimit;
					int64_t halfW = b.sx < 0 ? cx - shape.sqLeft[o] : shape.sqRight[o] - cx;
					int64_t halfH = b.sy < 0 ? cy - shape.sqTop[o] : shape.sqBottom[o] - cy;

					if (int64_t(std::abs(b.sx)) * halfH > int64_t(std::abs(b.sy)) * halfW)
						ray.yOuter = cy + (int64_t(b.sy) * halfW) / std::abs(b.sx);
					else
						ray.yOuter = b.sy < 0 ? shape.sqTop[o] : shape.sqBottom[o];
				}
				else
					ray.yOuter = cy + ((radius[shape.outerLimit] * b.sy) >> 30);

				ray.yInner = bHole ? cy + ((radius[shape.innerLimit] * b.sy) >> 30) : cy;
				rays.push_back(ray);
			}

			shape.pRays[half] = rays.data();
			shape.nRays[half] = (int)rays.size();

			if (rays.empty())
				shape.startSegment[half] = m_elementSegments[_elementAt(0, half == 0 ? -1 : 1)];
			else
			{
				const Boundary& b = m_boundaries[m_crossings[half][0]];
				shape.startSegment[half] = m_elementSegments[half == 0 ? b.prevElement : b.element];
			}
		}

		// Sub-rows split where edges begin and end.

		m_splits.clear();

		if (ring.bBack || ring.bSquare)
		{
			m_splits.push_back(shape.top);
			m_splits.push_back(shape.bottom);
		}

		if (ring.bSquare && shape.bOutline)
		{
			m_splits.push_back(shape.sqTop[1]);
			m_splits.push_back(shape.sqBottom[1]);
		}

		for (int i = 0; i < 4; i++)
		{
			if (radius[i] > 0)
			{
				m_splits.push_back(cy - radius[i]);
				m_splits.push_back(cy + radius[i]);
			}
		}

		if (!m_boundaries.empty())
			m_splits.push_back(cy);

		if (shape.bSlices)
		{
			for (int half = 0; half < 2; half++)
				for (auto& ray : m_rays[half])
				{
					m_splits.push_back(ray.yOuter);
					if (bHole)
						m_splits.push_back(ray.yInner);
				}
		}

		std::sort(m_splits.begin(), m_splits.end());
		m_splits.erase(std::unique(m_splits.begin(), m_splits.end()), m_splits.end());

		shape.pSplits = m_splits.data();
		shape.nSplits = (int)m_splits.size();
		shape.maxEdges = 10 + (int)std::max(m_rays[0].size(), m_rays[1].size());
		shape.nIds = RingShape::c_firstRayId + (int)(m_rays[0].size() + m_rays[1].size());

		// Rows and columns. Without back, the outer edge bounds the ring instead of rect.

		int64_t boundsLeft = shape.left, boundsTop = shape.top, boundsRight = shape.right, boundsBottom = shape.bottom;

		if (!ring.bBack && !ring.bSquare)
		{
			boundsLeft = cx - outerRadius;
			boundsTop = cy - outerRadius;
			boundsRight = cx + outerRadius;
			boundsBottom = cy + outerRadius;
		}

		// Symmetric without rays, if centered, also upside down if the same above and below.

		const bool bNoRays = m_rays[0].empty() && m_rays[1].empty();

		if (bNoRays && boundsLeft + boundsRight == 2 * cx)
		{
			shape.bSymmetricX = true;
			shape.symmetrySumX = 2 * cx;
		}

		if (bNoRays && shape.startSegment[0] == shape.startSegment[1] && boundsTop + boundsBottom == 2 * cy)
		{
			shape.bSymmetricY = true;
			shape.symmetrySumY = 2 * cy;
		}

		int clipBeg = std::max(clip.x, floorPixel(boundsLeft));
		int clipEnd = std::min(clip.x + clip.w, ceilPixel(boundsRight));
		int rowBeg = std::max(clip.y, floorPixel(boundsTop));
		int rowEnd = std::min(clip.y + clip.h, ceilPixel(boundsBottom));

		_rasterize(shape, c_ringFirstSliceSegment + ring.nSlices, rowBeg, rowEnd, clipBeg, clipEnd, pSink);
	}


	//____ PolygonShape _______________________________________________________
	/*
		A convex polygon, given as its left and right edge flattened in advance from its top
		to its bottom. Edge ids: 0 left, 1 right.
	*/

	class SoftShapes::PolygonShape : public SoftShapes::EdgeShape
	{
	public:
		Polyline	left;
		Polyline	right;
		int64_t		splits[2];

		int edges(int64_t ya, int64_t yb, EdgeDef* pEdges) const
		{
			if (ya < splits[0] || yb > splits[1])
				return 0;

			pEdges[0] = { 0, 0, false, -1, 0, const_cast<Polyline*>(&left) };
			pEdges[1] = { 1, -1, false, -1, 0, const_cast<Polyline*>(&right) };
			return 2;
		}

		int64_t x(const EdgeDef& edge, int64_t y)
		{
			return _polylineX(edge.id == 0 ? left : right, y);
		}

		bool split(const EdgeDef& edge, int64_t ya, int64_t xa, int64_t yb, int64_t xb, int64_t& y)
		{
			return false;
		}

		int uniformEnd(int row) const
		{
			return row + 1;
		}
	};

	//____ Lines ______________________________________________________________
	/*
		Lines with caps, see LineCap, are worked out in the frame of an end: 'a' along the line
		towards the other end and 'w' across it, to the left of the direction of the line as
		seen on the canvas (y pointing down). In that frame the line is the band |w| <= r, cut
		by each end at a = f(w), the profile of its cap. Profiles are convex and piecewise
		straight, round caps flattened into chords.

		Where the profile of the first end, the one with the lower column, last reaches
		(its highest column) the band covers whole columns until the profile of the other end
		begins. Those columns are drawn by the line kernels, the parts around the ends as
		convex polygons: the band cut by the profile of the end and a straight cut beyond its
		columns. If the ends leave no whole columns between them, the whole line is one
		polygon, the band between both profiles.

		All values are in 1/16 spx, from the end and the direction of the line scaled to
		2^29 -> 2^30, so integer math neither overflows nor loses precision. Directions and
		vertices are rounded, to within 1/256 pixel.
	*/

	namespace
	{
		// Position on the canvas, spx, of the point a along and w across a line from px, py
		// in direction ux, uy (2.30), in 1/16 spx.

		inline void lineToCanvas(int64_t px, int64_t py, int64_t ux, int64_t uy, int64_t a, int64_t w, int64_t& x, int64_t& y)
		{
			const int64_t half = int64_t(1) << 33;
			x = px + ((a * ux - w * uy + half) >> 34);
			y = py + ((a * uy + w * ux + half) >> 34);
		}

		// Value of a profile at w, between its first and last point.

		template<class Profile>
		inline int64_t profileAt(const Profile& profile, int64_t w)
		{
			size_t i = 1;
			while (i < profile.size() - 1 && profile[i].w < w)
				i++;

			const auto& p0 = profile[i - 1];
			const auto& p1 = profile[i];

			if (p1.w == p0.w)
				return std::min(p0.a, p1.a);

			return p0.a + (p1.a - p0.a) * (w - p0.w) / (p1.w - p0.w);
		}
	}

	//____ _capProfile() ______________________________________________________
	/*
		Profile of a cap from w = -r to r. dirX and dirY is the direction of the line, either
		way. Vertical and Horizontal ends need it to have some extent along x or y.
	*/

	void SoftShapes::_capProfile(LineCap cap, int64_t r, int64_t dirX, int64_t dirY, std::vector<ProfilePoint>& profile)
	{
		profile.clear();

		auto add = [&](int64_t w, int64_t a)
		{
			profile.push_back({ w, std::min(std::max(a, -c_maxCapReach), c_maxCapReach) });
		};

		switch (cap.shape)
		{
			case LineCapShape::Square:
				add(-r, -r);
				add(r, -r);
				break;

			case LineCapShape::Vertical:					// Through the end along y: a = w * dirY / dirX.
			{
				const int64_t a = r * dirY / dirX;
				add(-r, -a);
				add(r, a);
				break;
			}

			case LineCapShape::Horizontal:					// Through the end along x: a = -w * dirX / dirY.
			{
				const int64_t a = r * dirX / dirY;
				add(-r, a);
				add(r, -a);
				break;
			}

			case LineCapShape::Spear:						// Tip on the end, full width at spearLength percent of thickness.
			{
				const int64_t length = r * std::min(std::max(int(cap.spearLength), 25), 800) / 50;
				add(-r, length);
				add(0, 0);
				add(r, length);
				break;
			}

			case LineCapShape::Round:
			{
				// Half circle into chords as in _quarter(), vertices between the ends moved out
				// by half the sagitta.

				const int64_t rSpx = r >> c_lineUnitShift;
				const int64_t halves = rSpx >= 64 * 64 ? 2 : 1;

				int n = 2 * std::max(int((isqrt(uint64_t(rSpx * halves / c_maxSagitta)) * 569 + 1023) >> 10), 1);

				const int64_t cosHalfStep = _sin30(uint32_t((int64_t(1) << 30) / n) + 0x40000000);
				const int64_t rOut = (r << 31) / ((int64_t(1) << 30) + cosHalfStep);		// 2r / (1 + cos(angle / 2)), 1/2^30.

				for (int k = 0; k <= n; k++)
				{
					const uint32_t angle = uint32_t((int64_t(k) << 31) / n);				// Half a turn over the profile.
					const int64_t sin30 = _sin30(angle);
					const int64_t cos30 = _sin30(angle + 0x40000000);

					if (k == 0 || k == n)
						add((-r * cos30 + (1 << 29)) >> 30, 0);
					else
						add((-rOut * cos30 + (int64_t(1) << 29)) >> 30, (-rOut * sin30 + (int64_t(1) << 29)) >> 30);
				}
				break;
			}

			default:										// Flat
				add(-r, 0);
				add(r, 0);
				break;
		}
	}

	//____ _setLinePart() _____________________________________________________
	/*
		Sets the left and right edge of a part of a line from the vertices of its convex polygon,
		in either order around it. Returns false if it covers no area.
	*/

	bool SoftShapes::_setLinePart(LinePart& part, std::vector<int64_t>& vx, std::vector<int64_t>& vy)
	{
		// Drop vertices repeated by rounding.

		size_t n = 0;
		for (size_t i = 0; i < vx.size(); i++)
		{
			if (n > 0 && vx[i] == vx[n - 1] && vy[i] == vy[n - 1])
				continue;
			vx[n] = vx[i];
			vy[n] = vy[i];
			n++;
		}

		while (n > 1 && vx[n - 1] == vx[0] && vy[n - 1] == vy[0])
			n--;

		if (n < 3)
			return false;

		// Twice the area, positive if clockwise on the canvas, which has y pointing down.

		int64_t area = 0;
		for (size_t i = 0; i < n; i++)
		{
			const size_t j = (i + 1) % n;
			area += (vx[i] - vx[0]) * (vy[j] - vy[0]) - (vx[j] - vx[0]) * (vy[i] - vy[0]);
		}

		if (area == 0)
			return false;

		size_t top = 0;
		part.top = vy[0];
		part.bottom = vy[0];
		part.left = vx[0];
		part.right = vx[0];

		for (size_t i = 1; i < n; i++)
		{
			if (vy[i] < part.top)
			{
				part.top = vy[i];
				top = i;
			}
			part.bottom = std::max(part.bottom, vy[i]);
			part.left = std::min(part.left, vx[i]);
			part.right = std::max(part.right, vx[i]);
		}

		if (part.top == part.bottom)
			return false;

		// From the top along each way around to the bottom. Clockwise goes down the right edge.

		auto walk = [&](int step, std::vector<int64_t>& ys, std::vector<int64_t>& xs)
		{
			ys.clear();
			xs.clear();

			size_t i = top;
			auto next = [&](size_t k) { return (k + n + step) % n; };

			while (vy[next(i)] == part.top && next(i) != top)
				i = next(i);

			ys.push_back(vy[i]);
			xs.push_back(vx[i]);

			while (vy[i] != part.bottom)
			{
				i = next(i);

				if (vy[i] <= ys.back())
					xs.back() = vx[i];				// Kink flattened by rounding.
				else
				{
					ys.push_back(vy[i]);
					xs.push_back(vx[i]);
				}
			}
		};

		walk(area > 0 ? 1 : -1, part.rightY, part.rightX);
		walk(area > 0 ? -1 : 1, part.leftY, part.leftX);
		return true;
	}

	//____ prepareLine() ______________________________________________________

	bool SoftShapes::prepareLine(const Line& line, LineParts& parts)
	{
		parts.nParts = 0;
		parts.bodyBeg = 0;
		parts.bodyEnd = 0;

		if (line.thickness <= 0)
			return false;

		CoordSPX beg = line.begin;
		CoordSPX end = line.end;
		LineCap caps[2] = { line.caps.begin, line.caps.end };

		int64_t dx = int64_t(end.x) - beg.x;
		int64_t dy = int64_t(end.y) - beg.y;

		const bool bZero = dx == 0 && dy == 0;
		const bool bAlongX = bZero || std::abs(dx) > std::abs(dy);

		// The first end is the one with the lowest column.

		if ((bAlongX ? dx : dy) < 0)
		{
			std::swap(beg, end);
			std::swap(caps[0], caps[1]);
			dx = -dx;
			dy = -dy;
		}

		// Direction, along x if no length, scaled to 2^29 -> 2^30.

		int64_t dirX = bZero ? 1 : dx;
		int64_t dirY = bZero ? 0 : dy;

		for (LineCap& cap : caps)
		{
			if (cap.shape == LineCapShape::None)
				cap.shape = bAlongX ? LineCapShape::Vertical : LineCapShape::Horizontal;

			if ((cap.shape == LineCapShape::Vertical && dirX == 0) || (cap.shape == LineCapShape::Horizontal && dirY == 0))
				return false;
		}

		while (std::max(std::abs(dirX), std::abs(dirY)) >= (int64_t(1) << 30))
		{
			dirX /= 2;
			dirY /= 2;
		}

		while (std::max(std::abs(dirX), std::abs(dirY)) < (int64_t(1) << 29))
		{
			dirX *= 2;
			dirY *= 2;
		}

		const int64_t dirLength = isqrt(uint64_t(dirX * dirX + dirY * dirY));
		const int64_t ux = dirX * (int64_t(1) << 30) / dirLength;
		const int64_t uy = dirY * (int64_t(1) << 30) / dirLength;

		const int64_t r = int64_t(line.thickness) << (c_lineUnitShift - 1);

		_capProfile(caps[0], r, dirX, dirY, m_profiles[0]);
		_capProfile(caps[1], r, dirX, dirY, m_profiles[1]);

		// Frames of the ends: position and direction towards the other end.

		const int64_t endX[2] = { beg.x, end.x };
		const int64_t endY[2] = { beg.y, end.y };
		const int64_t endUX[2] = { ux, -ux };
		const int64_t endUY[2] = { uy, -uy };

		// Columns each end reaches, from lo to hi, spx.

		int64_t lo[2], hi[2];

		for (int e = 0; e < 2; e++)
		{
			lo[e] = INT64_MAX;
			hi[e] = INT64_MIN;

			for (const ProfilePoint& point : m_profiles[e])
			{
				int64_t x, y;
				lineToCanvas(endX[e], endY[e], endUX[e], endUY[e], point.a, point.w, x, y);

				const int64_t column = bAlongX ? x : y;
				lo[e] = std::min(lo[e], column);
				hi[e] = std::max(hi[e], column);
			}
		}

		m_bLineAlongX = bAlongX;
		parts.bAlongX = bAlongX;

		const int bodyBeg = ceilPixel(hi[0]);
		const int bodyEnd = floorPixel(lo[1]);

		if (!bZero && bodyBeg < bodyEnd)
		{
			// Body, columns centered on the line with the band across them.

			const int64_t dMajor = bAlongX ? dx : dy;			// Positive.
			const int64_t dMinor = bAlongX ? dy : dx;
			const int64_t major = bAlongX ? beg.x : beg.y;
			const int64_t minor = bAlongX ? beg.y : beg.x;

			const int64_t slope24 = dMinor * (int64_t(1) << 24) / dMajor;
			const int64_t width = int64_t(line.thickness) * 1024 * dirLength / std::abs(bAlongX ? dirX : dirY);

			parts.bodyBeg = bodyBeg;
			parts.bodyEnd = bodyEnd;
			parts.bodySlope24 = slope24;
			parts.bodySlope = int((slope24 + 128) >> 8);
			parts.bodyWidth = int(width);
			parts.bodyPos = minor * 1024 + (((int64_t(bodyBeg) * 64 + 32 - major) * slope24) >> 14) - width / 2;

			// Ends, the band from the profile of the end to a cut beyond its columns.

			for (int e = 0; e < 2; e++)
			{
				const int columnBeg = e == 0 ? floorPixel(lo[0]) : bodyEnd;
				const int columnEnd = e == 0 ? bodyBeg : ceilPixel(hi[1]);

				if (columnBeg >= columnEnd)
					continue;

				const int64_t endMajor = bAlongX ? endX[e] : endY[e];
				const int64_t distance = e == 0 ? int64_t(bodyBeg) * 64 - endMajor : endMajor - int64_t(bodyEnd) * 64;
				const int64_t cut = (3 * (distance + line.thickness / 2) / 2 + 64) << c_lineUnitShift;

				m_vertexX.clear();
				m_vertexY.clear();

				auto vertex = [&](int64_t a, int64_t w)
				{
					int64_t x, y;
					lineToCanvas(endX[e], endY[e], endUX[e], endUY[e], a, w, x, y);
					m_vertexX.push_back(x);
					m_vertexY.push_back(y);
				};

				for (const ProfilePoint& point : m_profiles[e])
					vertex(point.a, point.w);

				vertex(cut, r);
				vertex(cut, -r);

				LinePart& part = m_lineParts[parts.nParts];
				if (_setLinePart(part, m_vertexX, m_vertexY))
				{
					part.columnBeg = columnBeg;
					part.columnEnd = columnEnd;
					parts.partBeg[parts.nParts] = columnBeg;
					parts.partEnd[parts.nParts] = columnEnd;
					parts.nParts++;
				}
			}

			return true;
		}

		// No whole columns between the ends, which are close: the band between both profiles,
		// in the frame of the first end. The second end has w the other way.

		if (std::abs(dx) >= (int64_t(1) << 27) || std::abs(dy) >= (int64_t(1) << 27))
			return false;													// Can't be, ends reach no further.

		const int64_t length = isqrt(uint64_t(dx * dx + dy * dy) << (2 * c_lineUnitShift));

		const auto& first = m_profiles[0];
		const auto& second = m_profiles[1];

		std::vector<int64_t>& ws = m_vertexX;			// Borrowed for the w of the corners.
		ws.clear();

		for (const ProfilePoint& point : first)
			ws.push_back(point.w);
		for (const ProfilePoint& point : second)
			ws.push_back(-point.w);

		std::sort(ws.begin(), ws.end());
		ws.erase(std::unique(ws.begin(), ws.end()), ws.end());

		// Where the band is long enough to reach between the profiles: g(w) <= length, where
		// g(w) is the sum of the profiles, which is convex.

		auto g = [&](int64_t w) { return profileAt(first, w) + profileAt(second, -w); };

		int iFirst = -1, iLast = -1;
		for (int i = 0; i < (int)ws.size(); i++)
		{
			if (g(ws[i]) <= length)
			{
				if (iFirst < 0)
					iFirst = i;
				iLast = i;
			}
		}

		if (iFirst < 0)
			return false;

		auto crossing = [&](int64_t wOut, int64_t wIn)
		{
			const int64_t gOut = g(wOut);
			const int64_t gIn = g(wIn);
			return wOut + (wIn - wOut) * (gOut - length) / (gOut - gIn);
		};

		std::vector<int64_t> corners;
		corners.push_back(iFirst > 0 ? crossing(ws[iFirst - 1], ws[iFirst]) : ws[iFirst]);
		for (int i = iFirst; i <= iLast; i++)
			if (ws[i] > corners.back())
				corners.push_back(ws[i]);

		const int64_t wLast = iLast < (int)ws.size() - 1 ? crossing(ws[iLast + 1], ws[iLast]) : ws[iLast];
		if (wLast > corners.back())
			corners.push_back(wLast);

		std::vector<int64_t> vx, vy;

		auto vertex = [&](int64_t a, int64_t w)
		{
			int64_t x, y;
			lineToCanvas(endX[0], endY[0], ux, uy, a, w, x, y);
			vx.push_back(x);
			vy.push_back(y);
		};

		for (size_t i = 0; i < corners.size(); i++)
			vertex(profileAt(first, corners[i]), corners[i]);

		for (size_t i = corners.size(); i-- > 0; )
			vertex(length - profileAt(second, -corners[i]), corners[i]);

		LinePart& part = m_lineParts[0];
		if (!_setLinePart(part, vx, vy))
			return false;

		part.columnBeg = floorPixel(bAlongX ? part.left : part.top);
		part.columnEnd = ceilPixel(bAlongX ? part.right : part.bottom);
		parts.partBeg[0] = part.columnBeg;
		parts.partEnd[0] = part.columnEnd;
		parts.nParts = 1;
		return true;
	}

	//____ rasterizeLinePart() ________________________________________________

	void SoftShapes::rasterizeLinePart(int partIndex, const RectI& clip, RowSink* pSink)
	{
		const LinePart& part = m_lineParts[partIndex];

		int clipBeg = std::max(clip.x, floorPixel(part.left));
		int clipEnd = std::min(clip.x + clip.w, ceilPixel(part.right));
		int rowBeg = std::max(clip.y, floorPixel(part.top));
		int rowEnd = std::min(clip.y + clip.h, ceilPixel(part.bottom));

		if (m_bLineAlongX)
		{
			clipBeg = std::max(clipBeg, part.columnBeg);
			clipEnd = std::min(clipEnd, part.columnEnd);
		}
		else
		{
			rowBeg = std::max(rowBeg, part.columnBeg);
			rowEnd = std::min(rowEnd, part.columnEnd);
		}

		PolygonShape shape;
		shape.left = { part.leftY.data(), part.leftX.data(), int(part.leftY.size()), 0 };
		shape.right = { part.rightY.data(), part.rightX.data(), int(part.rightY.size()), 0 };
		shape.splits[0] = part.top;
		shape.splits[1] = part.bottom;
		shape.pSplits = shape.splits;
		shape.nSplits = 2;
		shape.maxEdges = 2;
		shape.nIds = 2;

		_rasterize(shape, 1, rowBeg, rowEnd, clipBeg, clipEnd, pSink);
	}

}
