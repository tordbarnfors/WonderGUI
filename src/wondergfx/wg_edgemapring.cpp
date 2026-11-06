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
#include <wg_edgemapring.h>
#include <wg_gfxbase.h>
#include <wg_gfxbackend.h>

#include <algorithm>
#include <cmath>

namespace wg
{
	const TypeInfo EdgemapRing::TYPEINFO = { "EdgemapRing", &EdgemapShape::TYPEINFO };

	static const double c_pi = 3.14159265358979323846;

	// How far into a row a circle must reach at its top or bottom for its edges there to start on the
	// line of the row, see EdgemapRing::_adjustSamples().

	static const double c_apexFlatFromRow = 0.875;
	static const double c_apexFlatMax = 0.25;			// How far from the center they start at most, relative to the width on the next line.

	static const double c_inset = 1.0 / 4096;			// How far lines at the edges of a piece are moved into it, see EdgemapShape.
	static const double c_nudge = 1.0 / 4096;			// How far from the center a quarter ends.
	static const int	c_minDirectSize = 4 * 64;		// Smaller than this and pieces may not reach the center, which only the general way handles.
	static const int	c_maxOrder = 64;				// Slices met in a quarter, more than an edgemap can hold.

	// Keys of regions: what part of the ring, which side of the center and for slices which slice
	// or gap in what turn around the center.

	enum RegionBand : uint32_t { Nothing = 0, Back, OuterOutline, Slices, InnerOutline, Hub };

	static uint32_t regionKey( uint32_t band, int side, int turn = 0, int interval = 0 )
	{
		return band | (uint32_t(side) << 3) | (uint32_t(turn + 8) << 4) | (uint32_t(interval) << 8);
	}

	//____ create() ______________________________________________________________

	EdgemapRing_p EdgemapRing::create( const Blueprint& blueprint, const RingSpec& spec )
	{
		EdgemapRing_p p( new EdgemapRing(blueprint) );

		if (!p->_setSpec(spec) || !p->_applyBuilds())
			return nullptr;

		return p;
	}

	//____ constructor ___________________________________________________________

	EdgemapRing::EdgemapRing( const Blueprint& bp ) : m_pFactory(_factoryOrDefault(bp.factory))
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& EdgemapRing::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ update() ______________________________________________________________
	/**
	 * @brief Change the ring.
	 *
	 * The change reaches the edgemaps when generated, see EdgemapShape. They are updated where
	 * they can, so that only what changed is imported, which is all that is streamed through a
	 * StreamEdgemapFactory.
	 *
	 * @return False if the spec is invalid or has too many slices in a quarter, in which case
	 * the ring is left as it was.
	 */

	bool EdgemapRing::update( const RingSpec& spec )
	{
		return _setSpec(spec);
	}

	//____ _setSpec() ____________________________________________________________

	bool EdgemapRing::_setSpec( const RingSpec& spec )
	{
		if (spec.nSlices < 0 || (spec.nSlices > 0 && !spec.pSlices))
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Invalid slices.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		if (m_bHasSpec && _isSameSpec(spec))
			return true;

		RingSpec oldSpec = m_spec;
		m_previousSlices = m_slices;
		oldSpec.pSlices = m_previousSlices.data();

		_prepare(spec);

		bool bOk;

		if (!m_bSquare && spec.rect.w >= c_minDirectSize && spec.rect.h >= c_minDirectSize)
			bOk = _buildDirectly();
		else
		{
			auto regions = [this](const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& out)
			{
				_regions(piece, bRow, pos, begin, end, out);
			};

			bOk = _buildPieces(m_layout, regions, m_colors.data(), m_pFactory) == BuildResult::Ok;
		}

		if (!bOk)
		{
			_prepare(oldSpec);
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Too many slices in a quarter of the ring.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		m_bHasSpec = true;
		return true;
	}

	//____ _isSameSpec() _________________________________________________________

	bool EdgemapRing::_isSameSpec( const RingSpec& spec ) const
	{
		if (spec.rect != m_spec.rect || spec.thickness != m_spec.thickness || spec.outline != m_spec.outline || spec.start != m_spec.start
			|| spec.nSlices != m_spec.nSlices || spec.bSquare != m_spec.bSquare || spec.outlineColor != m_spec.outlineColor
			|| spec.hubColor != m_spec.hubColor || spec.backColor != m_spec.backColor)
			return false;

		for (int i = 0; i < spec.nSlices; i++)
			if (spec.pSlices[i].size != m_slices[i].size || spec.pSlices[i].color != m_slices[i].color)
				return false;

		return true;
	}

	//____ _cacheCircles() _______________________________________________________
	/*
		Works out where circles and the rect cross the lines of the pieces. Only needs doing when
		they change, not when slices do.
	*/

	void EdgemapRing::_cacheCircles()
	{
		const RingSpec& spec = m_spec;

		if (m_bCirclesCached && m_cachedRect == spec.rect && m_cachedThickness == spec.thickness && m_cachedOutline == spec.outline
			&& m_bCachedQuarters == m_bQuarters && m_bCachedHalves == m_bHalves)
			return;

		m_bCirclesCached = true;
		m_cachedRect = spec.rect;
		m_cachedThickness = spec.thickness;
		m_cachedOutline = spec.outline;
		m_bCachedQuarters = m_bQuarters;
		m_bCachedHalves = m_bHalves;

		bool bSplit = m_bQuarters || m_bHalves;

		m_directPieces.clear();

		for (int l = 0; l < (int)m_layout.size(); l++)
		{
			const RectI& rect = m_layout[l].rect;

			if (rect.w <= 0 || rect.h <= 0)
				continue;

			m_directPieces.emplace_back();
			DirectPiece& piece = m_directPieces.back();

			int nLines = rect.h + 1;

			piece.layout = l;
			piece.side = !bSplit ? 0 : rect.x < m_splitX ? -1 : 1;
			piece.bTop = rect.y < m_splitY;
			piece.nCrossings = piece.side == 0 ? 10 : 5;
			piece.begin = rect.x;
			piece.across.resize(nLines);
			piece.crossings.resize(nLines * piece.nCrossings);

			double begin = rect.x;
			double end = rect.x + rect.w;
			spx length = rect.w * 64;

			auto toSamples = [begin, length](double x) { return std::clamp(spx(std::lround((x - begin) * 64)), spx(0), length); };

			for (int i = 0; i < nLines; i++)
			{
				double pos = rect.y + i;
				if (i == 0)
					pos += c_inset;
				else if (i == nLines - 1)
					pos -= c_inset;

				if (m_bQuarters)
					pos = piece.bTop ? std::min(pos, m_centerY - c_nudge) : std::max(pos, m_centerY + c_nudge);

				double across = pos - m_centerY;
				piece.across[i] = across;

				bool bInRect = pos >= m_rowTop && pos < m_rowBottom;

				double width[4];								// From center to where each circle crosses the line, negative if it doesn't.

				for (int r = 0; r < 4; r++)
				{
					width[r] = bInRect && m_radius[r] > 0 && std::abs(across) < m_radius[r] ? std::sqrt(m_radius[r] * m_radius[r] - across * across) : -1.0;

					if (bSplit && width[r] <= c_nudge)
						width[r] = -1.0;
				}

				spx* pOut = &piece.crossings[i * piece.nCrossings];

				if (piece.side <= 0)
				{
					double missing = piece.side == 0 ? m_centerX : end;			// Where a layer missing from the line is.

					*pOut++ = toSamples(bInRect ? m_left : missing);
					for (int r = 0; r < 4; r++)
						*pOut++ = toSamples(width[r] >= 0.0 ? m_centerX - width[r] : missing);
				}

				if (piece.side >= 0)
				{
					double missing = piece.side == 0 ? m_centerX : begin;

					for (int r = 3; r >= 0; r--)
						*pOut++ = toSamples(width[r] >= 0.0 ? m_centerX + width[r] : missing);
					*pOut++ = toSamples(bInRect ? m_right : missing);
				}

			}

			_slopeApexes(piece, rect);

			// Layers with width anywhere in the piece.

			for (int i = 0; i < nLines; i++)
			{
				const spx* pCrossings = &piece.crossings[i * piece.nCrossings];

				spx prev = 0;
				for (int c = 0; c < piece.nCrossings; c++)
				{
					if (pCrossings[c] > prev)
						piece.present |= 1 << c;
					prev = std::max(prev, pCrossings[c]);
				}
				if (length > prev)
					piece.present |= 1 << piece.nCrossings;
			}
		}
	}

	//____ _slopeApexes() ________________________________________________________
	/*
		Gives the circles of a piece built directly the slope that _adjustSamples() gives edges
		built the general way, see there. Here it is done to where the circles cross the lines,
		before lines between slices are placed between them and layers are joined, so it is known
		which circle is which and slices in the row follow along.
	*/

	void EdgemapRing::_slopeApexes( DirectPiece& piece, const RectI& rect )
	{
		static const double c_maxOvershoot = 256.0;		// Pixels, for a circle barely entering the row.

		const int nCrossings = piece.nCrossings;

		auto place = [nCrossings](spx* pLine, int c, spx sample)
		{
			pLine[c] = sample;

			for (int k = 0; k < c; k++)
				pLine[k] = std::min(pLine[k], sample);
			for (int k = c + 1; k < nCrossings; k++)
				pLine[k] = std::max(pLine[k], sample);
		};

		for (int i = 0; i < 4; i++)
		{
			double r = m_radius[i];

			if (r <= 0)
				continue;

			for (int dir = -1; dir <= 1; dir += 2)		// Top, then bottom.
			{
				double apex = m_centerY + dir * r;
				int outside = dir < 0 ? int(std::floor(apex)) : int(std::ceil(apex));
				int inside = outside - dir;

				if (outside < rect.y || outside > rect.y + rect.h || inside < rect.y || inside > rect.y + rect.h)
					continue;

				double across = inside - m_centerY;
				if (std::abs(across) >= r)
					continue;

				double reach = std::abs(inside - apex);
				double width = std::sqrt(r * r - across * across);

				double start = reach >= c_apexFlatFromRow ? width * c_apexFlatMax * (reach - c_apexFlatFromRow) / (1 - c_apexFlatFromRow)
														  : std::max(width * (1 - c_apexFlatFromRow / reach), -c_maxOvershoot);

				int beyondLine = outside + dir;
				bool bBeyond = start > 0.0 && beyondLine >= rect.y && beyondLine <= rect.y + rect.h;

				spx* pOutside = &piece.crossings[(outside - rect.y) * nCrossings];
				spx* pBeyond = bBeyond ? &piece.crossings[(beyondLine - rect.y) * nCrossings] : nullptr;

				for (int side = -1; side <= 1; side += 2)
				{
					if ((piece.side < 0 && side > 0) || (piece.side > 0 && side < 0))
						continue;

					int c = side < 0 ? i + 1 : (piece.side == 0 ? 5 : 0) + 3 - i;

					place(pOutside, c, spx(std::lround((m_centerX + side * start - piece.begin) * 64)));

					if (pBeyond)
						place(pBeyond, c, spx(std::lround((m_centerX - side * c_maxOvershoot - piece.begin) * 64)));
				}
			}
		}
	}

	//____ _buildDirectly() ______________________________________________________
	/*
		Builds the pieces of a round ring from what is known about it, instead of from the regions
		along each line. Along a line the layers are always in the same order and the slices of a
		quarter in the order of their angles, with the lines between them only depending on the
		angle. Layers and slices without any width in a piece are left out.

		Returns false if a piece needs more segments than an edgemap has, with nothing changed.
	*/

	bool EdgemapRing::_buildDirectly()
	{
		enum Layer { Outside, Back, OuterOutline, Slices, InnerOutline, Hub };

		static const Layer c_layersIn[6] = { Outside, Back, OuterOutline, Slices, InnerOutline, Hub };
		static const Layer c_layersOut[6] = { Hub, InnerOutline, Slices, OuterOutline, Back, Outside };
		static const Layer c_layersWhole[11] = { Outside, Back, OuterOutline, Slices, InnerOutline, Hub, InnerOutline, Slices, OuterOutline, Back, Outside };

		auto drawn = [this](int index) { return m_colors[index].isUndefined() ? -1 : index; };

		_cacheCircles();

		const int nIntervals = (int)m_intervals.size();

		if ((int)m_directBuilds.size() < (int)m_directPieces.size())
			m_directBuilds.resize(m_directPieces.size());

		int nBuilds = 0;

		for (const DirectPiece& piece : m_directPieces)
		{
			const RectI& rect = m_layout[piece.layout].rect;
			const int nLines = rect.h + 1;
			const spx length = rect.w * 64;

			const Layer* pLayers = piece.side < 0 ? c_layersIn : piece.side > 0 ? c_layersOut : c_layersWhole;
			const int nLayers = piece.nCrossings + 1;

			Build& build = m_directBuilds[nBuilds];
			build.colors.clear();
			m_edgeSources.clear();

			bool bEmpty = true;

			auto addRegion = [&](int color, const spx* pEnds, int stride)
			{
				HiColor hiColor = color >= 0 ? m_colors[color] : HiColor::Transparent;

				if (build.colors.empty() || hiColor != build.colors.back())
				{
					build.colors.push_back(hiColor);
					m_edgeSources.push_back({ pEnds, stride });
				}
				else
					m_edgeSources.back() = { pEnds, stride };

				if (color >= 0)
					bEmpty = false;
			};

			for (int l = 0; l < nLayers; l++)
			{
				if (!(piece.present & (1 << l)))
					continue;

				const spx* pEnds = l < piece.nCrossings ? piece.crossings.data() + l : nullptr;

				switch (pLayers[l])
				{
					case Outside:
						addRegion(-1, pEnds, piece.nCrossings);
						break;
					case Back:
						addRegion(drawn(0), pEnds, piece.nCrossings);
						break;
					case OuterOutline:
					case InnerOutline:
						addRegion(drawn(1), pEnds, piece.nCrossings);
						break;
					case Hub:
						addRegion(drawn(2), pEnds, piece.nCrossings);
						break;

					case Slices:
					{
						if (!m_bSlices)
						{
							addRegion(drawn(1), pEnds, piece.nCrossings);
							break;
						}

						if (nIntervals == 1)
						{
							addRegion(m_intervals[0].color, pEnds, piece.nCrossings);
							break;
						}

						// Slices of this quarter in the order of their angles, from where it begins.

						const double quarterStart = piece.side > 0 ? (piece.bTop ? 0.0 : 0.25) : (piece.bTop ? 0.75 : 0.5);

						double position = quarterStart - m_start;
						position -= std::floor(position);

						int interval = 0;
						while (interval < nIntervals - 1 && position >= m_intervals[interval].end)
							interval++;

						int order[c_maxOrder];
						double angles[c_maxOrder];			// Of the line after each slice in order.
						int nOrder = 0;

						double angle = quarterStart;
						double left = 0.25;

						while (true)
						{
							if (nOrder == c_maxOrder)
								return false;

							order[nOrder] = interval;

							double span = m_intervals[interval].end - position;
							if (span >= left)
							{
								nOrder++;
								break;
							}

							angle += span;
							left -= span;
							angles[nOrder++] = angle;

							position = m_intervals[interval].end;
							if (++interval == nIntervals)
							{
								interval = 0;
								position = 0.0;
							}
						}

						const int nLinesBetween = nOrder - 1;

						const spx* pBandBegin = l > 0 ? piece.crossings.data() + l - 1 : nullptr;
						const spx* pBandEnd = pEnds;

						const double* pAcross = piece.across.data();

						if ((int)m_lineSamples.size() < nLines * nLinesBetween)
						{
							// All pointers into this buffer are from this piece, and none are made yet if it grows here.
							m_lineSamples.resize(nLines * std::max(nLinesBetween, 8));
						}

						// Along a line, angles grow in the top quarters and shrink in the bottom ones.

						const bool bReverse = !piece.bTop;
						const double centerInSamples = (m_centerX - piece.begin) * 64;

						for (int k = 0; k < nLinesBetween; k++)
						{
							double a = angles[bReverse ? nLinesBetween - 1 - k : k] * 2 * c_pi;
							double slope = std::sin(a) / -std::cos(a) * 64;

							spx* pOut = m_lineSamples.data() + k * nLines;
							const spx* pBegin = pBandBegin;
							const spx* pEnd = pBandEnd;

							for (int i = 0; i < nLines; i++)
							{
								double low = *pBegin, high = *pEnd;
								double x = centerInSamples + pAcross[i] * slope;
								x = x < low ? low : x > high ? high : x;
								pOut[i] = spx(x + 0.5);

								pBegin += piece.nCrossings;
								pEnd += piece.nCrossings;
							}
						}

						// Those having width on some line make regions.

						for (int k = 0; k < nOrder; k++)
						{
							int index = bReverse ? nOrder - 1 - k : k;

							const spx* pFrom = k == 0 ? pBandBegin : m_lineSamples.data() + (k - 1) * nLines;
							const spx* pTo = k == nLinesBetween ? pBandEnd : m_lineSamples.data() + k * nLines;
							int fromStride = k == 0 ? piece.nCrossings : 1;
							int toStride = k == nLinesBetween ? piece.nCrossings : 1;

							bool bPresent = false;
							for (int i = 0; i < nLines && !bPresent; i++)
							{
								if (pTo[i * toStride] > pFrom[i * fromStride])
									bPresent = true;
							}

							if (bPresent)
								addRegion(m_intervals[order[index]].color, pTo, toStride);
						}
						break;
					}
				}
			}

			int nGroups = (int)build.colors.size();

			if (nGroups > Edgemap::maxSegments)
				return false;

			if (bEmpty)
				continue;

			if (nGroups == 1)
				build.colors.push_back(build.colors.back());

			int nEdges = (int)build.colors.size() - 1;

			build.rect = rect;
			build.bRows = true;
			build.bEmpty = false;
			build.samples.resize(nLines * nEdges);

			for (int e = 0; e < nEdges; e++)
			{
				spx* pOut = build.samples.data() + e;

				if (nGroups == 1)
				{
					for (int i = 0; i < nLines; i++, pOut += nEdges)
						*pOut = length;
				}
				else
				{
					const spx* pIn = m_edgeSources[e].pSamples;
					const int stride = m_edgeSources[e].stride;

					for (int i = 0; i < nLines; i++, pOut += nEdges, pIn += stride)
						*pOut = *pIn;
				}
			}

			nBuilds++;
		}

		m_directBuilds.resize(nBuilds);
		m_builds.swap(m_directBuilds);
		m_pBuildFactory = m_pFactory;
		m_bBuilt = true;
		return true;
	}

	//____ _prepare() ____________________________________________________________
	// Takes the spec and works out the geometry of the ring.

	void EdgemapRing::_prepare( const RingSpec& spec )
	{
		// Keep a copy, tints left out since they are not supported.

		m_slices.assign(spec.pSlices, spec.pSlices + spec.nSlices);
		for (auto& slice : m_slices)
			slice.pTint = nullptr;

		m_spec = spec;
		m_spec.pSlices = m_slices.data();
		m_spec.pOutlineTint = nullptr;
		m_spec.pHubTint = nullptr;
		m_spec.pBackTint = nullptr;

		// Geometry, from spx to pixels, as ReferenceBackend draws a ring.

		const RectSPX& rect = spec.rect;

		m_left = rect.x / 64.0;
		m_top = rect.y / 64.0;
		m_right = (int64_t(rect.x) + rect.w) / 64.0;
		m_bottom = (int64_t(rect.y) + rect.h) / 64.0;

		m_centerX = (m_left + m_right) / 2;
		m_centerY = (m_top + m_bottom) / 2;

		double outerRadius = std::max(std::min(m_right - m_left, m_bottom - m_top) / 2, 0.0);
		double thickness = spec.thickness / 64.0;

		bool bHole = thickness > 0 && thickness < outerRadius;
		double holeRadius = bHole ? outerRadius - thickness : 0.0;
		double outline = std::clamp(std::max(spec.outline, 0) / 64.0, 0.0, bHole ? thickness / 2 : outerRadius);

		m_radius[0] = outerRadius;
		m_radius[1] = outerRadius - outline;
		m_radius[2] = bHole ? holeRadius + outline : 0.0;
		m_radius[3] = holeRadius;

		m_bSquare = spec.bSquare;
		m_insetLeft = m_left + outline;
		m_insetTop = m_top + outline;
		m_insetRight = m_right - outline;
		m_insetBottom = m_bottom - outline;

		double sliceRadius = m_bSquare ? std::min(m_centerX - m_insetLeft, m_centerY - m_insetTop) : m_radius[1];
		m_bSlices = sliceRadius > m_radius[2];

		// Angles quantized as GfxDevice passes them on, start and sizes in turns.

		const double fullTurn = double(GfxBackend::c_shapeFullTurn);

		float start = std::isfinite(spec.start) ? spec.start - std::floor(spec.start) : 0.f;
		m_start = (uint32_t(double(start) * fullTurn) % GfxBackend::c_shapeFullTurn) / fullTurn;

		// Colors: back, outline, hub and slices. Undefined ones are not drawn.

		auto isDrawn = [](HiColor color) { return !color.isUndefined(); };

		m_colors.clear();
		m_colors.push_back(spec.backColor);
		m_colors.push_back(spec.outlineColor);
		m_colors.push_back(spec.hubColor);

		int backColor = isDrawn(spec.backColor) ? 0 : -1;

		// Slices and the gap after them, as positions in turns from start.

		m_intervals.clear();
		m_boundaries.clear();
		m_boundaries.push_back(m_start);

		double end = 0.0;
		for (int i = 0; i < spec.nSlices; i++)
		{
			const RingSlice& slice = spec.pSlices[i];
			m_colors.push_back(slice.color);

			float size = slice.size > 0.f ? std::min(slice.size, 1.f) : 0.f;		// Also for NaN.
			double turns = uint32_t(double(size) * fullTurn) / fullTurn;

			if (turns <= 0.0 || end >= 1.0)
				continue;

			end = std::min(end + turns, 1.0);
			m_intervals.push_back({ end, isDrawn(slice.color) ? 3 + i : -1 });

			if (end < 1.0)
				m_boundaries.push_back(m_start + end);
		}

		if (end < 1.0)
			m_intervals.push_back({ 1.0, backColor });

		// Layout. Rows can't have a horizontal edge between them, only pieces can begin or end
		// at one, so horizontal edges of rect, if a back color or square is drawn up to them,
		// and of the inside of a square outline are moved to the nearest pixel line and pieces
		// split there. A square outline gets a piece above and below the rest.

		m_rowTop = std::round(m_top);
		m_rowBottom = std::round(m_bottom);
		m_rowInsetTop = std::round(m_insetTop);
		m_rowInsetBottom = std::round(m_insetBottom);

		bool bRectEdges = m_bSquare || backColor >= 0;

		// Without them, nothing is cut at the rows of the rect. The circle is inside it anyway, and
		// rounding the rows would cut off where it reaches into a row past them.

		if (!bRectEdges)
		{
			m_rowTop = std::floor(m_top);
			m_rowBottom = std::ceil(m_bottom);
		}

		int left = int(std::floor(m_left));
		int right = int(std::ceil(m_right));
		int top = bRectEdges ? int(m_rowTop) : int(std::floor(m_top));
		int bottom = bRectEdges ? int(m_rowBottom) : int(std::ceil(m_bottom));

		m_middleTop = top;
		m_middleBottom = bottom;

		if (m_bSquare && outline > 0)
		{
			m_middleTop = std::clamp(int(m_rowInsetTop), top, bottom);
			m_middleBottom = std::clamp(int(m_rowInsetBottom), m_middleTop, bottom);
		}

		// The middle is made of a piece for each quarter if there are slices to line up, split at
		// the pixel lines nearest the center.

		m_bQuarters = m_bSlices && m_intervals.size() > 1;

		// Without slices it is enough with a left and a right piece, needed if a sample of a circle at
		// its top or bottom is moved outside the piece, see _adjustSamples(). The edges of the left
		// and right side would cross in a single piece.

		m_bHalves = false;

		for (int i = m_bSquare ? 2 : 0; i < 4 && !m_bQuarters && !m_bHalves; i++)
		{
			if (m_radius[i] > 0)
			{
				double apexTop = m_centerY - m_radius[i];
				double apexBottom = m_centerY + m_radius[i];

				m_bHalves = (std::floor(apexTop) + 1 - apexTop) < c_apexFlatFromRow || int(std::floor(apexTop)) > top
							|| (apexBottom - std::ceil(apexBottom) + 1) < c_apexFlatFromRow || int(std::ceil(apexBottom)) < bottom;
			}
		}

		m_splitX = std::clamp(int(std::lround(m_centerX)), left, right);
		m_splitY = std::clamp(int(std::lround(m_centerY)), m_middleTop, m_middleBottom);

		m_layout.clear();
		m_layout.push_back({ RectI(left, top, right - left, m_middleTop - top) });

		if (m_bQuarters)
		{
			int x = m_splitX, y = m_splitY;

			m_layout.push_back({ RectI(left, m_middleTop, x - left, y - m_middleTop) });
			m_layout.push_back({ RectI(x, m_middleTop, right - x, y - m_middleTop) });
			m_layout.push_back({ RectI(left, y, x - left, m_middleBottom - y) });
			m_layout.push_back({ RectI(x, y, right - x, m_middleBottom - y) });
		}
		else if (m_bHalves)
		{
			m_layout.push_back({ RectI(left, m_middleTop, m_splitX - left, m_middleBottom - m_middleTop) });
			m_layout.push_back({ RectI(m_splitX, m_middleTop, right - m_splitX, m_middleBottom - m_middleTop) });
		}
		else
			m_layout.push_back({ RectI(left, m_middleTop, right - left, m_middleBottom - m_middleTop) });

		m_layout.push_back({ RectI(left, m_middleBottom, right - left, bottom - m_middleBottom) });
	}

	//____ _regions() ____________________________________________________________
	// Regions along a line, see EdgemapShape::RegionFunc. The line is cut where it crosses an
	// edge of the ring and each part classified at its middle.

	void EdgemapRing::_regions( const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& regions ) const
	{
		double center = bRow ? m_centerX : m_centerY;		// Along the line.

		// A quarter or half only holds what is on its side of the center. Where it reaches beyond
		// the center, up to the pixel line it is split at, what is next to the center continues.

		double from = begin, to = end;						// Part of the line classified.

		if ((m_bQuarters || m_bHalves) && piece.y >= m_middleTop && piece.y < m_middleBottom)
		{
			bool bLeft = piece.x < m_splitX;
			bool bTop = piece.y < m_splitY;

			if (m_bQuarters)
			{
				double centerAcross = bRow ? m_centerY : m_centerX;
				bool bLowAcross = bRow ? bTop : bLeft;
				pos = bLowAcross ? std::min(pos, centerAcross - c_nudge) : std::max(pos, centerAcross + c_nudge);
			}

			if (bRow ? bLeft : bTop)
				to = std::max(std::min(to, center - c_nudge), from);
			else
				from = std::min(std::max(from, center + c_nudge), to);
		}

		// Angles in the piece are counted from the opposite direction of its center, so they don't
		// wrap around within it.

		double reference = std::atan2(piece.x + piece.w / 2.0 - m_centerX, -(piece.y + piece.h / 2.0 - m_centerY)) / (2 * c_pi) + 0.5;

		double across = pos - (bRow ? m_centerY : m_centerX);

		regions.clear();

		if (to - from < 1e-9)
		{
			int color;
			uint32_t key = _classify(bRow ? from : pos, bRow ? pos : from, bRow, reference, color);
			regions.push_back({ key, color, end - begin });
			return;
		}

		double cuts[32];
		int nCuts = 0;

		auto cut = [&](double v)
		{
			if (v > from && v < to && nCuts < 30)
				cuts[nCuts++] = v;
		};

		cuts[nCuts++] = from;
		cuts[nCuts++] = to;

		cut(center);

		for (double r : m_radius)
		{
			if (r > 0 && std::abs(across) < r)
			{
				double half = std::sqrt(r * r - across * across);
				cut(center - half);
				cut(center + half);
			}
		}

		cut(bRow ? m_left : m_top);
		cut(bRow ? m_right : m_bottom);

		if (m_bSquare)
		{
			cut(bRow ? m_insetLeft : m_insetTop);
			cut(bRow ? m_insetRight : m_insetBottom);
		}

		std::sort(cuts, cuts + nCuts);

		// Lines between slices cross at most once each, but there can be many.

		std::vector<double> lineCuts;

		for (double angle : m_boundaries)
		{
			double dirX = std::sin(angle * 2 * c_pi);
			double dirY = -std::cos(angle * 2 * c_pi);

			double along = bRow ? dirX : dirY;
			double acrossDir = bRow ? dirY : dirX;

			if (std::abs(acrossDir) > 1e-12)
			{
				double v = center + across * along / acrossDir;
				if (v > from && v < to)
					lineCuts.push_back(v);
			}
		}

		std::vector<double> all(cuts, cuts + nCuts);
		all.insert(all.end(), lineCuts.begin(), lineCuts.end());
		std::sort(all.begin(), all.end());

		for (size_t i = 0; i + 1 < all.size(); i++)
		{
			double a = all[i], b = all[i + 1];
			if (b - a < 1e-9)
				continue;

			double mid = (a + b) / 2;
			int color;
			uint32_t key = _classify(bRow ? mid : pos, bRow ? pos : mid, bRow, reference, color);

			if (!regions.empty() && regions.back().key == key)
				regions.back().end = b - begin;
			else
				regions.push_back({ key, color, b - begin });
		}

		if (!regions.empty())
			regions.back().end = end - begin;
	}

	//____ _adjustSamples() ______________________________________________________
	/*
		An edge is straight between two lines, so where a circle reaches its top or bottom within a
		row, an edge following the circle would run from the center on the line outside to where the
		circle crosses the line inside. That makes a point, reaching the line outside no matter how
		far into the row the circle really starts: pixels at the center get covered far too much
		or, for a circle starting right on the line, too little further out.

		So the sample on the line outside is moved along that line, giving the edge a slope closer to
		the coverage of the circle in the row. If the circle reaches far into the row the edge starts
		away from the center on the line outside. Otherwise it starts outside the piece, on the other
		side of the center, so that it passes the center within the row, at about where the circle
		starts. That is no problem since edges are clipped against the edgemap. An edge starting away
		from the center would reach into the row beyond the line outside, where the circle isn't,
		so the sample on the line beyond is moved far outside the piece.

		Matching the area exactly would leave the row flat too far out from the center, with a visible
		kink where the slope starts, so the edge is made to split the difference. At the center
		pixels are covered about 1.14 times what they should, at most fully.
	*/

	void EdgemapRing::_adjustSamples( const Layout& layout, int nEdges, spx * pSamples ) const
	{
		static const double c_maxOvershoot = 256.0;		// Pixels, for a circle barely entering the row.

		const RectI& rect = layout.rect;

		if (!layout.bRows)
			return;

		for (int i = m_bSquare ? 2 : 0; i < 4; i++)
		{
			double r = m_radius[i];

			if (r <= 0 || (i == 1 && r == m_radius[0]) || (i == 2 && r == m_radius[3]))
				continue;

			for (int dir = -1; dir <= 1; dir += 2)		// Top, then bottom.
			{
				double apex = m_centerY + dir * r;
				int outside = dir < 0 ? int(std::floor(apex)) : int(std::ceil(apex));
				int inside = outside - dir;

				if (outside < rect.y || outside > rect.y + rect.h || inside < rect.y || inside > rect.y + rect.h)
					continue;

				double across = inside - m_centerY;
				if (std::abs(across) >= r)
					continue;

				double reach = std::abs(inside - apex);		// Of the row the circle is in.
				double width = std::sqrt(r * r - across * across);

				double start = reach >= c_apexFlatFromRow ? width * c_apexFlatMax * (reach - c_apexFlatFromRow) / (1 - c_apexFlatFromRow)
														  : std::max(width * (1 - c_apexFlatFromRow / reach), -c_maxOvershoot);

				spx* pInside = pSamples + (inside - rect.y) * nEdges;
				spx* pOutside = pSamples + (outside - rect.y) * nEdges;
				spx* pBeyond = start > 0.0 && outside + dir >= rect.y && outside + dir <= rect.y + rect.h ? pSamples + (outside + dir - rect.y) * nEdges : nullptr;

				auto place = [nEdges](spx* pLine, int e, spx sample)
				{
					pLine[e] = sample;

					for (int k = 0; k < e; k++)
						pLine[k] = std::min(pLine[k], sample);
					for (int k = e + 1; k < nEdges; k++)
						pLine[k] = std::max(pLine[k], sample);
				};

				for (int side = -1; side <= 1; side += 2)
				{
					spx crossing = spx(std::lround((m_centerX + side * width - rect.x) * 64));
					spx sample = spx(std::lround((m_centerX + side * start - rect.x) * 64));
					spx beyond = spx(std::lround((m_centerX - side * c_maxOvershoot - rect.x) * 64));

					for (int e = 0; e < nEdges; e++)
					{
						if (std::abs(pInside[e] - crossing) <= 1)
						{
							place(pOutside, e, sample);

							if (pBeyond)
								place(pBeyond, e, beyond);
						}
					}
				}
			}
		}
	}

	//____ _classify() ___________________________________________________________
	// Key and color of the region at x, y, as ReferenceBackend's RingShape::segmentAt().

	uint32_t EdgemapRing::_classify( double x, double y, bool bRow, double reference, int& color ) const
	{
		int side = (bRow ? x > m_centerX : y > m_centerY) ? 1 : 0;

		if (x < m_left || x >= m_right || y < m_rowTop || y >= m_rowBottom)
		{
			color = -1;
			return regionKey(Nothing, side);
		}

		double dx = x - m_centerX;
		double dy = y - m_centerY;
		double distance = std::sqrt(dx * dx + dy * dy);

		bool bInsideOuterEdge = m_bSquare || distance < m_radius[0];
		bool bInsideOuterOutline = m_bSquare ? (x >= m_insetLeft && x < m_insetRight && y >= m_rowInsetTop && y < m_rowInsetBottom) : distance < m_radius[1];

		auto drawn = [this](int index) { return m_colors[index].isUndefined() ? -1 : index; };

		if (!bInsideOuterEdge)
		{
			color = drawn(0);
			return regionKey(Back, side);
		}

		if (!bInsideOuterOutline)
		{
			color = drawn(1);
			return regionKey(OuterOutline, side);
		}

		if (distance < m_radius[3])
		{
			color = drawn(2);
			return regionKey(Hub, 0);
		}

		if (distance < m_radius[2] || !m_bSlices)
		{
			color = drawn(1);
			return regionKey(InnerOutline, side);
		}

		if (m_intervals.size() == 1)
		{
			color = m_intervals[0].color;
			return regionKey(Slices, side);
		}

		// Slice or gap. The angle is unrolled from the reference, so the same slice gets the same
		// key along all lines of a piece, and a slice met twice gets different turns.

		double angle = (dx == 0.0 && dy == 0.0) ? 0.0 : std::atan2(dx, -dy) / (2 * c_pi);

		double unrolled = angle - reference;
		unrolled -= std::floor(unrolled);
		unrolled += reference - m_start;

		double turn = std::floor(unrolled);
		double position = unrolled - turn;

		int interval = 0;
		while (interval < (int)m_intervals.size() - 1 && position >= m_intervals[interval].end)
			interval++;

		color = m_intervals[interval].color;
		return regionKey(Slices, side, int(turn), interval);
	}
}
