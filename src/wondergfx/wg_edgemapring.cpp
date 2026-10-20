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
	 * @return False if the spec is invalid or has too many slices in a quarter, in which case
	 * the ring is left as it was, or if an edgemap couldn't be created.
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

		RingSpec oldSpec = m_spec;
		std::vector<RingSlice> oldSlices = m_slices;
		oldSpec.pSlices = oldSlices.data();

		_prepare(spec);

		auto regions = [this](const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& out)
		{
			_regions(piece, bRow, pos, begin, end, out);
		};

		switch (_buildPieces(m_layout, regions, m_colors.data(), m_pFactory))
		{
			case BuildResult::Ok:
				return true;

			case BuildResult::TooManySegments:
				_prepare(oldSpec);
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Too many slices in a quarter of the ring.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				return false;

			default:
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to create edgemap.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				return false;
		}
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
		else
			m_layout.push_back({ RectI(left, m_middleTop, right - left, m_middleBottom - m_middleTop) });

		m_layout.push_back({ RectI(left, m_middleBottom, right - left, bottom - m_middleBottom) });
	}

	//____ _regions() ____________________________________________________________
	// Regions along a line, see EdgemapShape::RegionFunc. The line is cut where it crosses an
	// edge of the ring and each part classified at its middle.

	void EdgemapRing::_regions( const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& regions ) const
	{
		static const double c_nudge = 1.0 / 4096;			// How far from the center a quarter ends.

		double center = bRow ? m_centerX : m_centerY;		// Along the line.

		// A quarter only holds what is on its side of the center. Where it reaches beyond the
		// center, up to the pixel line it is split at, what is next to the center continues.

		double from = begin, to = end;						// Part of the line classified.

		if (m_bQuarters && piece.y >= m_middleTop && piece.y < m_middleBottom)
		{
			bool bLeft = piece.x < m_splitX;
			bool bTop = piece.y < m_splitY;

			double centerAcross = bRow ? m_centerY : m_centerX;
			bool bLowAcross = bRow ? bTop : bLeft;
			pos = bLowAcross ? std::min(pos, centerAcross - c_nudge) : std::max(pos, centerAcross + c_nudge);

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
