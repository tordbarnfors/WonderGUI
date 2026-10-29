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
#include <wax_core.h>
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

	//____ create() ______________________________________________________________

	EdgemapBox_p EdgemapBox::create( const Blueprint& blueprint, const RoundedBoxSpec& spec )
	{
		EdgemapBox_p p( new EdgemapBox(blueprint) );

		if (!p->_setSpec(spec))
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
	 * The edgemaps of the pieces are updated where they can, so that only what changed is
	 * imported, which is all that is streamed through a StreamEdgemapFactory.
	 *
	 * @return False if an edgemap couldn't be created.
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

	//____ _setSpec() ____________________________________________________________

	bool EdgemapBox::_setSpec( const RoundedBoxSpec& spec )
	{
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

		std::vector<double> edges = { m_outer.top, m_outer.bottom };
		if (m_bOutline && m_inner.right > m_inner.left && m_inner.bottom > m_inner.top)
			edges = { m_outer.top, m_inner.top, m_inner.bottom, m_outer.bottom };

		m_bands.clear();
		std::vector<Layout> layout;

		for (size_t i = 0; i + 1 < edges.size(); i++)
		{
			int top = int(std::round(edges[i]));
			int bottom = int(std::round(edges[i + 1]));

			if (bottom > top)
			{
				m_bands.push_back({ top, bottom, edges[i], edges[i + 1] });
				layout.push_back({ RectI(left, top, right - left, bottom - top) });
			}
		}

		auto regions = [this](const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& out)
		{
			_regions(piece, bRow, pos, begin, end, out);
		};

		if (_buildPieces(layout, regions, m_colors, m_pFactory) != BuildResult::Ok)
		{
			Core::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to create edgemap.",
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
		static const double c_nudge = 1.0 / 4096;			// How far within its edges a line is kept.

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
