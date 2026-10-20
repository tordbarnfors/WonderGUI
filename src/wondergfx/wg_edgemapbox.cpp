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
#include <wg_edgemapbox.h>
#include <wg_gfxbase.h>
#include <wg_gfxutil.h>

#include <algorithm>
#include <cmath>

namespace wg
{
	const TypeInfo EdgemapBox::TYPEINFO = { "EdgemapBox", &EdgemapShape::TYPEINFO };

	// A corner runs along columns from its top or bottom to where it turns this far from horizontal,
	// 26.6 degrees, where it rises two pixels per pixel along a row.

	static const double c_capCos = 0.894427190999916;		// 2 / sqrt(5)

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

		// Layout. Straight edges between pixels get a pixel row along columns or a pixel column
		// along rows, those on pixels a barrier. Corners get caps along columns at top and bottom.

		int left = int(std::floor(m_outer.left));
		int right = int(std::ceil(m_outer.right));
		int top = int(std::floor(m_outer.top));
		int bottom = int(std::ceil(m_outer.bottom));

		Grid grid;
		grid.x = { left, right };
		grid.y = { top, bottom };

		m_areas.clear();

		std::vector<const Box*> boxes = { &m_outer };
		if (m_bOutline && m_inner.right > m_inner.left && m_inner.bottom > m_inner.top)
			boxes.push_back(&m_inner);

		// Straight edges only run between the corners.

		for (const Box* pBox : boxes)
		{
			for (int side : { 0, 1 })
			{
				double y = side == 0 ? pBox->top : pBox->bottom;
				int row = int(std::floor(y));
				if (row < top || row >= bottom)
					continue;

				int x0 = int(std::floor(side == 0 ? pBox->centerX(0) : pBox->centerX(3)));
				int x1 = int(std::ceil(side == 0 ? pBox->centerX(1) : pBox->centerX(2)));

				grid.x.insert(grid.x.end(), { x0, x1 });
				grid.y.push_back(row);

				if (row != y)
				{
					grid.y.push_back(row + 1);
					m_areas.push_back({ x0, row, x1, row + 1, false });
				}
				else
					grid.barrierRows.push_back({ row, x0, x1 });
			}
		}

		for (const Box* pBox : boxes)
		{
			for (int side : { 0, 1 })
			{
				double x = side == 0 ? pBox->left : pBox->right;
				int column = int(std::floor(x));
				if (column < left || column >= right)
					continue;

				int y0 = int(std::floor(side == 0 ? pBox->centerY(0) : pBox->centerY(1)));
				int y1 = int(std::ceil(side == 0 ? pBox->centerY(3) : pBox->centerY(2)));

				grid.y.insert(grid.y.end(), { y0, y1 });
				grid.x.push_back(column);

				if (column != x)
				{
					grid.x.push_back(column + 1);
					m_areas.push_back({ column, y0, column + 1, y1, true });
				}
				else
					grid.barrierColumns.push_back({ column, y0, y1 });
			}
		}

		for (const Box* pBox : boxes)
		{
			for (int corner = 0; corner < 4; corner++)
			{
				double r = pBox->radius[corner];
				if (r < 2.0)
					continue;

				double cx = pBox->centerX(corner);
				double cy = pBox->centerY(corner);
				bool bTop = corner < 2;
				bool bLeft = corner == 0 || corner == 3;

				int y0, y1;
				if (bTop)
				{
					y0 = int(std::floor(cy - r));
					y1 = std::max(int(std::lround(cy - r * c_capCos)), y0 + 1);
				}
				else
				{
					y1 = int(std::ceil(cy + r));
					y0 = std::min(int(std::lround(cy + r * c_capCos)), y1 - 1);
				}

				double dy = bTop ? cy - y1 : y0 - cy;
				double w = std::sqrt(std::max(r * r - dy * dy, 0.0));

				int x0 = bLeft ? int(std::floor(cx - w)) : int(std::floor(cx));
				int x1 = bLeft ? int(std::ceil(cx)) : int(std::ceil(cx + w));

				m_areas.push_back({ x0, y0, x1, y1, false });
				grid.x.insert(grid.x.end(), { x0, x1 });
				grid.y.insert(grid.y.end(), { y0, y1 });
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

		if (!_buildPieces(grid, regions, m_colors, m_pFactory))
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to create edgemap.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		return true;
	}

	//____ _orientation() ________________________________________________________

	int EdgemapBox::_orientation( const RectI& cell ) const
	{
		for (auto& area : m_areas)
		{
			if (cell.x >= area.x0 && cell.x + cell.w <= area.x1 && cell.y >= area.y0 && cell.y + cell.h <= area.y1)
				return area.bRows ? 1 : 0;
		}

		return 1;
	}

	//____ _regions() ____________________________________________________________
	// Regions along a line, see EdgemapShape::RegionFunc. The line is cut where it crosses an
	// edge of the box and each part classified at its middle.

	void EdgemapBox::_regions( const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& regions ) const
	{
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
