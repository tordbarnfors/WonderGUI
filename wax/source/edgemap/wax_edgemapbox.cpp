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
#include <wax_edgemapbox.h>
#include <wax_base.h>
#include <wax_util.h>

#include <algorithm>
#include <cmath>

namespace wax
{
	const TypeInfo EdgemapBox::TYPEINFO = { "EdgemapBox", &EdgemapShape::TYPEINFO };

	// Keys of regions: what part of the box and which side of its center.

	enum BoxBand : uint32_t { Nothing = 0, Outline, Fill };

	static uint32_t regionKey( uint32_t band, int side )
	{
		return band | (uint32_t(side) << 3);
	}

	static const double c_inset = 1.0 / 4096;			// How far lines at the edges of a piece are moved into it, see EdgemapShape.
	static const double c_nudge = 1.0 / 4096;			// How far within its edges a line is kept, see _regions().
	static const int	c_minDirectSize = 4 * 64;		// Smaller boxes are built the general way.

	//____ create() ______________________________________________________________

	EdgemapBox_p EdgemapBox::create( const Blueprint& blueprint, const RoundedBoxSpec& spec )
	{
		EdgemapBox_p p( new EdgemapBox(blueprint) );

		if (!p->_setSpec(spec) || !p->_applyBuilds())
			return nullptr;

		return p;
	}

	//____ constructor ___________________________________________________________

	EdgemapBox::EdgemapBox( const Blueprint& bp ) : m_pFactory(_factoryOrDefault(bp.factory))
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& EdgemapBox::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ update() ______________________________________________________________
	/**
	 * @brief Change the box.
	 *
	 * The change reaches the edgemaps when generated, see EdgemapShape. They are updated where
	 * they can, so that only what changed is imported, which is all that is streamed through a
	 * StreamEdgemapFactory.
	 *
	 * @return False if the box can't be made into edgemaps, in which case it is left as it was.
	 */

	bool EdgemapBox::update( const RoundedBoxSpec& spec )
	{
		return _setSpec(spec);
	}

	//____ Box::contains() _______________________________________________________

	bool EdgemapBox::Box::contains( double x, double y ) const
	{
		if (x < left || x >= right || y < top || y >= bottom)
			return false;

		// Points in the corner of the box beyond the center of its circle are only inside if they
		// are inside the circle.

		for (int corner = 0; corner < 4; corner++)
		{
			double cx = centerX(corner);
			double cy = centerY(corner);

			bool bBeyondX = (corner == 0 || corner == 3) ? x < cx : x > cx;
			bool bBeyondY = (corner == 0 || corner == 1) ? y < cy : y > cy;

			if (bBeyondX && bBeyondY && std::hypot(x - cx, y - cy) > radius[corner])
				return false;
		}

		return true;
	}

	//____ Box::span() ___________________________________________________________

	bool EdgemapBox::Box::span( double y, double& outLeft, double& outRight ) const
	{
		if (y < top || y >= bottom)
			return false;

		outLeft = left;
		outRight = right;

		for (int corner = 0; corner < 4; corner++)
		{
			double cy = centerY(corner);
			bool bBeyondY = (corner == 0 || corner == 1) ? y < cy : y > cy;
			double r = radius[corner];

			if (!bBeyondY || r <= 0)
				continue;

			double dy = y - cy;
			double width = std::abs(dy) < r ? std::sqrt(r * r - dy * dy) : 0.0;

			if (corner == 0 || corner == 3)
				outLeft = std::max(outLeft, centerX(corner) - width);
			else
				outRight = std::min(outRight, centerX(corner) + width);
		}

		return outLeft < outRight;
	}

	//____ _isSameGeometry() _____________________________________________________

	bool EdgemapBox::_isSameGeometry( const RoundedBoxSpec& spec ) const
	{
		if (spec.rect != m_spec.rect)
			return false;

		for (int i = 0; i < 4; i++)
			if (spec.radius[i] != m_spec.radius[i])
				return false;

		return spec.outline.top == m_spec.outline.top && spec.outline.right == m_spec.outline.right
			&& spec.outline.bottom == m_spec.outline.bottom && spec.outline.left == m_spec.outline.left;
	}

	//____ _isSameSpec() _________________________________________________________

	bool EdgemapBox::_isSameSpec( const RoundedBoxSpec& spec ) const
	{
		return _isSameGeometry(spec) && spec.fillColor == m_spec.fillColor && spec.outlineColor == m_spec.outlineColor;
	}

	//____ _setSpec() ____________________________________________________________

	bool EdgemapBox::_setSpec( const RoundedBoxSpec& spec )
	{
		if (m_bHasSpec && _isSameSpec(spec))
			return true;

		if (m_bHasSpec && !_isSameGeometry(spec))
			m_bCrossingsCached = false;

		const bool bHadSpec = m_bHasSpec;
		const RoundedBoxSpec oldSpec = m_spec;

		m_bHasSpec = true;
		m_spec = spec;
		m_spec.pFillTint = nullptr;					// Tints are not supported.
		m_spec.pOutlineTint = nullptr;

		// Geometry, from spx to pixels, as ReferenceBackend draws a rounded box.

		const RectSPX& rect = spec.rect;

		spx radius[4] = { spec.radius[0], spec.radius[1], spec.radius[2], spec.radius[3] };
		Util::clampCornerRadii(rect.size(), radius);

		BorderSPX outline = { std::max(spec.outline.top, 0), std::max(spec.outline.right, 0),
							  std::max(spec.outline.bottom, 0), std::max(spec.outline.left, 0) };

		spx innerRadius[4];
		Util::innerCornerRadii(radius, outline, innerRadius);

		m_outer.left = rect.x / 64.0;
		m_outer.top = rect.y / 64.0;
		m_outer.right = (int64_t(rect.x) + rect.w) / 64.0;
		m_outer.bottom = (int64_t(rect.y) + rect.h) / 64.0;

		m_inner.left = m_outer.left + outline.left / 64.0;
		m_inner.top = m_outer.top + outline.top / 64.0;
		m_inner.right = m_outer.right - outline.right / 64.0;
		m_inner.bottom = m_outer.bottom - outline.bottom / 64.0;

		for (int i = 0; i < 4; i++)
		{
			m_outer.radius[i] = radius[i] / 64.0;
			m_inner.radius[i] = innerRadius[i] / 64.0;
		}

		m_bOutline = (outline.top | outline.right | outline.bottom | outline.left) != 0;

		bool bInner = rect.w - outline.left - outline.right > 0 && rect.h - outline.top - outline.bottom > 0;

		m_centerX = (m_outer.left + m_outer.right) / 2;
		m_centerY = (m_outer.top + m_outer.bottom) / 2;

		m_colors[0] = spec.fillColor;
		m_colors[1] = spec.outlineColor;
		m_bFillDrawn = bInner && !spec.fillColor.isUndefined();
		m_bOutlineDrawn = m_bOutline && !spec.outlineColor.isUndefined();

		// Layout. Pieces running along rows, which can't have horizontal edges between them, so
		// pieces are split at the horizontal edges, moved to the nearest pixel lines. A piece for
		// the outline above and below the inside, if there is one, and one for the rest. Lines
		// through a piece are kept within the edges it is between.

		int left = int(std::floor(m_outer.left));
		int right = int(std::ceil(m_outer.right));

		double edges[4] = { m_outer.top, m_outer.bottom };
		int nEdges = 2;
		if (m_bOutline && m_inner.right > m_inner.left && m_inner.bottom > m_inner.top)
		{
			edges[1] = m_inner.top;
			edges[2] = m_inner.bottom;
			edges[3] = m_outer.bottom;
			nEdges = 4;
		}

		m_bands.clear();
		std::vector<Layout>& layout = m_layout;			// Kept, so it isn't allocated again.
		layout.clear();

		for (int i = 0; i + 1 < nEdges; i++)
		{
			int top = int(std::round(edges[i]));
			int bottom = int(std::round(edges[i + 1]));

			if (bottom > top)
			{
				m_bands.push_back({ top, bottom, edges[i], edges[i + 1] });
				layout.push_back({ RectI(left, top, right - left, bottom - top) });
			}
		}

		// Boxes of some size are built directly, the general way handles the rest.

		if (rect.w >= c_minDirectSize && rect.h >= c_minDirectSize)
		{
			_buildDirectly(layout);
			return true;
		}

		auto regions = [this](const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& out)
		{
			_regions(piece, bRow, pos, begin, end, out);
		};

		if (_buildPieces(layout, regions, m_colors, m_pFactory) != BuildResult::Ok)
		{
			// Back to the spec of the edgemaps, which are left as they were.

			if (bHadSpec)
			{
				m_bHasSpec = false;
				_setSpec(oldSpec);
			}

			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Box can't be made into edgemaps.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		return true;
	}

	//____ _regions() ____________________________________________________________
	// Regions along a line, see EdgemapShape::RegionFunc. The line is cut where it crosses an
	// edge of the box and each part classified at its middle.

	void EdgemapBox::_regions( const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& regions ) const
	{
		for (auto& band : m_bands)
		{
			if (bRow && piece.y == band.top)
				pos = std::clamp(pos, band.topEdge + c_nudge, std::max(band.bottomEdge - c_nudge, band.topEdge + c_nudge));
		}

		double cuts[40];
		int nCuts = 0;

		auto cut = [&](double v)
		{
			if (v > begin && v < end && nCuts < 40)
				cuts[nCuts++] = v;
		};

		cuts[nCuts++] = begin;
		cuts[nCuts++] = end;

		cut(bRow ? m_centerX : m_centerY);

		for (const Box* pBox : { &m_outer, &m_inner })
		{
			if (pBox == &m_inner && !m_bOutline)
				break;

			cut(bRow ? pBox->left : pBox->top);
			cut(bRow ? pBox->right : pBox->bottom);

			for (int corner = 0; corner < 4; corner++)
			{
				double r = pBox->radius[corner];
				double across = pos - (bRow ? pBox->centerY(corner) : pBox->centerX(corner));

				if (r > 0 && std::abs(across) < r)
				{
					double center = bRow ? pBox->centerX(corner) : pBox->centerY(corner);
					double half = std::sqrt(r * r - across * across);
					cut(center - half);
					cut(center + half);
				}
			}
		}

		std::sort(cuts, cuts + nCuts);

		regions.clear();

		for (int i = 0; i + 1 < nCuts; i++)
		{
			double a = cuts[i], b = cuts[i + 1];
			if (b - a < 1e-9)
				continue;

			double mid = (a + b) / 2;
			int color;
			uint32_t key = _classify(bRow ? mid : pos, bRow ? pos : mid, bRow, color);

			if (!regions.empty() && regions.back().key == key)
				regions.back().end = b - begin;
			else
				regions.push_back({ key, color, b - begin });
		}
	}

	//____ _cacheCrossings() _____________________________________________________
	/*
		Works out where the box and the inside of its outline cross the lines of each piece, in
		the order met along a line. A part of the box missing from a line is at its center, where
		the general way cuts lines, which keeps the samples the same.
	*/

	void EdgemapBox::_cacheCrossings( const std::vector<Layout>& layout )
	{
		if (m_bCrossingsCached)
			return;

		m_bCrossingsCached = true;
		m_directPieces.resize(layout.size());

		for (size_t p = 0; p < layout.size(); p++)
		{
			const RectI& rect = layout[p].rect;
			DirectPiece& piece = m_directPieces[p];

			int nLines = rect.h + 1;
			double begin = rect.x;
			spx length = rect.w * 64;

			piece.crossings.resize(nLines * 4);
			piece.present = 0;

			auto toSamples = [begin, length](double x) { return std::clamp(spx(std::lround((x - begin) * 64)), spx(0), length); };

			const Band* pBand = nullptr;
			for (auto& band : m_bands)
				if (band.top == rect.y)
					pBand = &band;

			for (int i = 0; i < nLines; i++)
			{
				double pos = rect.y + i;			// As EdgemapShape and _regions() place the line.
				if (i == 0)
					pos += c_inset;
				else if (i == nLines - 1)
					pos -= c_inset;

				if (pBand)
					pos = std::clamp(pos, pBand->topEdge + c_nudge, std::max(pBand->bottomEdge - c_nudge, pBand->topEdge + c_nudge));

				double outerLeft, outerRight, innerLeft, innerRight;

				bool bOuter = m_outer.span(pos, outerLeft, outerRight);
				bool bInner = bOuter && (m_bOutline ? m_inner.span(pos, innerLeft, innerRight) : m_outer.span(pos, innerLeft, innerRight));

				spx* pOut = &piece.crossings[i * 4];

				pOut[0] = toSamples(bOuter ? outerLeft : m_centerX);
				pOut[1] = toSamples(bInner ? innerLeft : m_centerX);
				pOut[2] = toSamples(bInner ? innerRight : m_centerX);
				pOut[3] = toSamples(bOuter ? outerRight : m_centerX);

				spx prev = 0;
				for (int c = 0; c < 4; c++)
				{
					if (pOut[c] > prev)
						piece.present |= 1 << c;
					prev = pOut[c];
				}
				if (length > prev)
					piece.present |= 1 << 4;
			}
		}
	}

	//____ _buildDirectly() ______________________________________________________
	/*
		Builds the pieces from the cached crossings. Along a line the box always has the same
		parts in the same order, outside, outline, fill, outline and outside, so only parts
		without any width in a piece are left out and neighbours of the same color joined.
	*/

	void EdgemapBox::_buildDirectly( const std::vector<Layout>& layout )
	{
		_cacheCrossings(layout);

		const int colorOfLayer[5] = { -1, m_bOutlineDrawn ? 1 : -1, m_bFillDrawn ? 0 : -1, m_bOutlineDrawn ? 1 : -1, -1 };

		if (m_directBuilds.size() < layout.size())
			m_directBuilds.resize(layout.size());

		int nBuilds = 0;

		for (size_t p = 0; p < layout.size(); p++)
		{
			const RectI& rect = layout[p].rect;
			const DirectPiece& piece = m_directPieces[p];
			const int nLines = rect.h + 1;
			const spx length = rect.w * 64;

			Build& build = m_directBuilds[nBuilds];
			build.colors.clear();
			m_edgeSources.clear();

			bool bEmpty = true;

			for (int l = 0; l < 5; l++)
			{
				if (!(piece.present & (1 << l)))
					continue;

				int color = colorOfLayer[l];
				HiColor hiColor = color >= 0 ? m_colors[color] : HiColor::Transparent;
				const spx* pEnds = l < 4 ? piece.crossings.data() + l : nullptr;

				if (build.colors.empty() || hiColor != build.colors.back())
				{
					build.colors.push_back(hiColor);
					m_edgeSources.push_back({ pEnds, 4 });
				}
				else
					m_edgeSources.back() = { pEnds, 4 };

				if (color >= 0)
					bEmpty = false;
			}

			if (bEmpty)
				continue;

			int nGroups = (int)build.colors.size();
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
				const spx* pIn = m_edgeSources[e].pSamples;

				for (int i = 0; i < nLines; i++, pOut += nEdges)
				{
					*pOut = nGroups == 1 ? length : *pIn;
					if (nGroups > 1)
						pIn += 4;
				}
			}

			if (nGroups > 1)
				_adjustSamples(layout[p], nEdges, build.samples.data());

			nBuilds++;
		}

		m_directBuilds.resize(nBuilds);
		m_builds.swap(m_directBuilds);
		m_pBuildFactory = m_pFactory;
		m_bBuilt = true;
	}

	//____ _adjustSamples() ______________________________________________________
	/*
		A corner meets a horizontal edge on the line a band begins or ends at. An edge following
		the corner through the row after that line runs straight from where the corner starts on
		that line, which makes a point, covering pixels along the corner too little. As in
		EdgemapRing, the sample on that line is moved along it, away from the corner's center, by a
		quarter of the corner's width on the next line. That splits the difference between
		covering the right area and keeping the row from looking flat with a kink.

		The edges of a corner are found where it crosses the next line and where it starts on
		this one.
	*/

	void EdgemapBox::_adjustSamples( const Layout& layout, int nEdges, spx * pSamples ) const
	{
		static const double c_flat = 0.25;					// Moved by this much of the corner's width on the next line.

		const RectI& rect = layout.rect;

		if (!layout.bRows)
			return;

		auto place = [nEdges](spx* pLine, int e, spx sample)
		{
			pLine[e] = sample;

			for (int k = 0; k < e; k++)
				pLine[k] = std::min(pLine[k], sample);
			for (int k = e + 1; k < nEdges; k++)
				pLine[k] = std::max(pLine[k], sample);
		};

		for (auto& band : m_bands)
		{
			if (band.top != rect.y || band.bottom != rect.y + rect.h)
				continue;

			for (const Box* pBox : { &m_outer, &m_inner })
			{
				if (pBox == &m_inner && !m_bOutline)
					break;

				for (int dir = -1; dir <= 1; dir += 2)		// Top, then bottom.
				{
					double boxEdge = dir < 0 ? pBox->top : pBox->bottom;
					double bandEdge = dir < 0 ? band.topEdge : band.bottomEdge;

					if (boxEdge != bandEdge)
						continue;

					int line = dir < 0 ? band.top : band.bottom;
					int next = line - dir;

					if (next < band.top || next > band.bottom || band.bottom - band.top < 2)
						continue;

					double pos = dir < 0 ? bandEdge + c_nudge : bandEdge - c_nudge;			// Where _regions() takes the line.

					spx* pLine = pSamples + (line - rect.y) * nEdges;
					spx* pNext = pSamples + (next - rect.y) * nEdges;

					for (int corner = 0; corner < 4; corner++)
					{
						bool bTopCorner = corner == 0 || corner == 1;
						if (bTopCorner != (dir < 0))
							continue;

						double r = pBox->radius[corner];
						double cx = pBox->centerX(corner);
						double cy = pBox->centerY(corner);
						double side = (corner == 0 || corner == 3) ? -1.0 : 1.0;		// Corner is left or right of its center.

						double acrossNext = next - cy;
						double acrossLine = pos - cy;

						if (r <= 0 || std::abs(acrossNext) >= r)
							continue;

						double widthNext = std::sqrt(r * r - acrossNext * acrossNext);
						double widthLine = std::abs(acrossLine) < r ? std::sqrt(r * r - acrossLine * acrossLine) : 0.0;

						spx crossingNext = spx(std::lround((cx + side * widthNext - rect.x) * 64));
						spx crossingLine = spx(std::lround((cx + side * widthLine - rect.x) * 64));
						spx sample = spx(std::lround((cx + side * std::max(widthLine, widthNext * c_flat) - rect.x) * 64));

						for (int e = 0; e < nEdges; e++)
						{
							if (std::abs(pNext[e] - crossingNext) <= 1 && std::abs(pLine[e] - crossingLine) <= 1)
							{
								place(pLine, e, sample);
								break;
							}
						}
					}
				}
			}
		}
	}

	//____ _classify() ___________________________________________________________
	// Key and color of the region at x, y, as ReferenceBackend's RoundedBoxShape::segmentAt().

	uint32_t EdgemapBox::_classify( double x, double y, bool bRow, int& color ) const
	{
		int side = (bRow ? x > m_centerX : y > m_centerY) ? 1 : 0;

		if (!m_outer.contains(x, y))
		{
			color = -1;
			return regionKey(Nothing, side);
		}

		if (m_bOutline && !m_inner.contains(x, y))
		{
			color = m_bOutlineDrawn ? 1 : -1;
			return regionKey(Outline, side);
		}

		color = m_bFillDrawn ? 0 : -1;
		return regionKey(Fill, 0);
	}
}
