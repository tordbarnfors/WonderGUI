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

	// A circle runs along columns from its top or bottom to where it turns this far from horizontal,
	// 26.6 degrees, where it rises two pixels per pixel along a row.

	static const double c_capCos = 0.894427190999916;		// 2 / sqrt(5)

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

		if (!p->_setSpec(spec))
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
	 * The edgemaps of the pieces are updated where they can, so that only what changed is
	 * imported, which is all that is streamed through a StreamEdgemapFactory.
	 *
	 * @return False if the spec is invalid, in which case the ring is left as it was, or if
	 * an edgemap couldn't be created.
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

		// Layout. Cells are split at the edges of rect and at the center if there are slices to
		// line up. The center, if between pixels, gets a pixel column along rows and a pixel row
		// along columns, for the lines through the center in those directions.

		int left = int(std::floor(m_left));
		int right = int(std::ceil(m_right));
		int top = int(std::floor(m_top));
		int bottom = int(std::ceil(m_bottom));

		Grid grid;
		grid.x = { left, right };
		grid.y = { top, bottom };

		bool bLines = m_bSlices && m_intervals.size() > 1;

		int centerLeft = int(std::floor(m_centerX)), centerRight = int(std::ceil(m_centerX));
		int centerTop = int(std::floor(m_centerY)), centerBottom = int(std::ceil(m_centerY));

		m_bCenterColumn = bLines && centerLeft != centerRight;
		m_bCenterRow = bLines && centerTop != centerBottom;

		if (bLines)
		{
			grid.x.insert(grid.x.end(), { centerLeft, centerRight });
			grid.y.insert(grid.y.end(), { centerTop, centerBottom });

			// Lines between slices pointing straight along a pixel line through the center.

			if (centerLeft == centerRight)
				grid.barrierColumns.push_back({ centerLeft });
			if (centerTop == centerBottom)
				grid.barrierRows.push_back({ centerTop });
		}

		// Where the pixel column or row of the center meets the top, bottom or sides of a circle,
		// the circle runs across it. Pieces in them begin and end at those pixels, since the first
		// and last pixel of a piece one pixel wide get exactly the coverage of each segment.

		for (double r : m_radius)
		{
			if (r <= 0.0)
				continue;

			// The top and left of a circle begin a piece, the bottom and right end one.

			if (m_bCenterColumn)
			{
				int row = int(std::floor(m_centerY - r));
				grid.y.push_back(row);
				grid.barrierRows.push_back({ row, centerLeft, centerRight });

				row = int(std::floor(m_centerY + r)) + 1;
				grid.y.push_back(row);
				grid.barrierRows.push_back({ row, centerLeft, centerRight });
			}

			if (m_bCenterRow)
			{
				int column = int(std::floor(m_centerX - r));
				grid.x.push_back(column);
				grid.barrierColumns.push_back({ column, centerTop, centerBottom });

				column = int(std::floor(m_centerX + r)) + 1;
				grid.x.push_back(column);
				grid.barrierColumns.push_back({ column, centerTop, centerBottom });
			}
		}

		// Caps at the top and bottom of each circle, joined where they meet. A cap is as wide as
		// its circles where they leave it, so they leave through its far edge.

		m_caps.clear();

		struct Rows { int y0, y1; double radius; };
		std::vector<Rows> topRows, bottomRows;

		for (int k = m_bSquare ? 2 : 0; k < 4; k++)
		{
			double r = m_radius[k];
			if (r < 2.0)
				continue;

			int y0 = int(std::floor(m_centerY - r));
			int y1 = std::clamp(int(std::lround(m_centerY - r * c_capCos)), y0 + 1, std::max(centerTop, y0 + 1));
			topRows.push_back({ y0, y1, r });

			y1 = int(std::ceil(m_centerY + r));
			y0 = std::clamp(int(std::lround(m_centerY + r * c_capCos)), std::min(centerBottom, y1 - 1), y1 - 1);
			bottomRows.push_back({ y0, y1, r });
		}

		auto addCaps = [&](std::vector<Rows>& rows, bool bTop)
		{
			std::sort(rows.begin(), rows.end(), [](const Rows& a, const Rows& b) { return a.y0 < b.y0; });

			for (size_t i = 0; i < rows.size(); )
			{
				int y0 = rows[i].y0, y1 = rows[i].y1;
				size_t first = i++;

				while (i < rows.size() && rows[i].y0 <= y1)
					y1 = std::max(y1, rows[i++].y1);

				double w = 0.0;
				for (size_t k = first; k < i; k++)
				{
					double dy = bTop ? m_centerY - y1 : y0 - m_centerY;
					double r = rows[k].radius;
					w = std::max(w, std::sqrt(std::max(r * r - dy * dy, 0.0)));
				}

				Cap cap = { std::max(int(std::floor(m_centerX - w)), left), std::max(y0, top),
							std::min(int(std::ceil(m_centerX + w)), right), std::min(y1, bottom) };

				if (cap.x0 < cap.x1 && cap.y0 < cap.y1)
					m_caps.push_back(cap);
			}
		};

		addCaps(topRows, true);
		size_t nTopCaps = m_caps.size();
		addCaps(bottomRows, false);

		// A bottom cap could only meet a top cap if the circle is tiny, keep them apart then.

		if (nTopCaps > 0 && m_caps.size() > nTopCaps && m_caps[nTopCaps].y0 < m_caps[nTopCaps - 1].y1)
			m_caps[nTopCaps].y0 = m_caps[nTopCaps - 1].y1;

		for (auto& cap : m_caps)
		{
			grid.x.insert(grid.x.end(), { cap.x0, cap.x1 });
			grid.y.insert(grid.y.end(), { cap.y0, cap.y1 });
		}

		// Horizontal edges of rect and of the inside of a square outline matter if anything is
		// drawn up to them. Between pixels they get a row running along columns, otherwise the
		// cells are split there. Vertical ones are fine along rows, cells are split at them if on
		// pixels.

		m_strips.clear();

		if (m_bSquare || backColor >= 0)
		{
			std::vector<double> horizontal = { m_top, m_bottom };
			std::vector<double> vertical = { m_left, m_right };

			if (m_bSquare && outline > 0)
			{
				horizontal.insert(horizontal.end(), { m_insetTop, m_insetBottom });
				vertical.insert(vertical.end(), { m_insetLeft, m_insetRight });
			}

			for (double y : horizontal)
			{
				int row = int(std::floor(y));
				if (row < top || row >= bottom)
					continue;

				grid.y.push_back(row);
				if (row != y)
				{
					grid.y.push_back(row + 1);
					m_strips.push_back({ left, row, right, row + 1 });
				}
				else
					grid.barrierRows.push_back({ row });
			}

			for (double x : vertical)
			{
				if (x == std::floor(x) && x > left && x < right)
				{
					grid.x.push_back(int(x));
					grid.barrierColumns.push_back({ int(x) });
				}
			}
		}

		for (auto* p : { &grid.x, &grid.y })
		{
			int low = p == &grid.x ? left : top;
			int high = p == &grid.x ? right : bottom;

			for (int& v : *p)
				v = std::clamp(v, low, high);

			std::sort(p->begin(), p->end());
			p->erase(std::unique(p->begin(), p->end()), p->end());
		}

		grid.orientation = [this](const RectI& cell) { return _orientation(cell); };

		auto regions = [this](const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& out)
		{
			_regions(piece, bRow, pos, begin, end, out);
		};

		if (!_buildPieces(grid, regions, m_colors.data(), m_pFactory))
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to create edgemap.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		return true;
	}

	//____ _orientation() ________________________________________________________

	int EdgemapRing::_orientation( const RectI& cell ) const
	{
		int centerLeft = int(std::floor(m_centerX));
		int centerTop = int(std::floor(m_centerY));

		bool bCenterColumn = m_bCenterColumn && cell.x == centerLeft && cell.w == 1;
		bool bCenterRow = m_bCenterRow && cell.y == centerTop && cell.h == 1;

		if (bCenterColumn)
			return 1;

		if (bCenterRow)
			return 0;

		for (auto& strip : m_strips)
			if (cell.y >= strip.y0 && cell.y + cell.h <= strip.y1)
				return 0;

		for (auto& cap : m_caps)
			if (cell.x >= cap.x0 && cell.x + cell.w <= cap.x1 && cell.y >= cap.y0 && cell.y + cell.h <= cap.y1)
				return 0;

		return 1;
	}

	//____ _regions() ____________________________________________________________
	// Regions along a line, see EdgemapShape::RegionFunc. The line is cut where it crosses an
	// edge of the ring and each part classified at its middle.

	void EdgemapRing::_regions( const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& regions ) const
	{
		// Angles in the piece are counted from the opposite direction of its center, so they don't
		// wrap around within it.

		double reference = std::atan2(piece.x + piece.w / 2.0 - m_centerX, -(piece.y + piece.h / 2.0 - m_centerY)) / (2 * c_pi) + 0.5;

		double center = bRow ? m_centerX : m_centerY;		// Along the line.
		double across = pos - (bRow ? m_centerY : m_centerX);

		double cuts[32];
		int nCuts = 0;

		auto cut = [&](double v)
		{
			if (v > begin && v < end && nCuts < 30)
				cuts[nCuts++] = v;
		};

		cuts[nCuts++] = begin;
		cuts[nCuts++] = end;

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
				if (v > begin && v < end)
					lineCuts.push_back(v);
			}
		}

		std::vector<double> all(cuts, cuts + nCuts);
		all.insert(all.end(), lineCuts.begin(), lineCuts.end());
		std::sort(all.begin(), all.end());

		regions.clear();

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
	}

	//____ _classify() ___________________________________________________________
	// Key and color of the region at x, y, as ReferenceBackend's RingShape::segmentAt().

	uint32_t EdgemapRing::_classify( double x, double y, bool bRow, double reference, int& color ) const
	{
		int side = (bRow ? x > m_centerX : y > m_centerY) ? 1 : 0;

		if (x < m_left || x >= m_right || y < m_top || y >= m_bottom)
		{
			color = -1;
			return regionKey(Nothing, side);
		}

		double dx = x - m_centerX;
		double dy = y - m_centerY;
		double distance = std::sqrt(dx * dx + dy * dy);

		bool bInsideOuterEdge = m_bSquare || distance < m_radius[0];
		bool bInsideOuterOutline = m_bSquare ? (x >= m_insetLeft && x < m_insetRight && y >= m_insetTop && y < m_insetBottom) : distance < m_radius[1];

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
