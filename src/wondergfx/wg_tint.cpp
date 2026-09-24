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

#include <wg_tint.h>
#include <wg_tinttools.h>
#include <wg_gfxbase.h>

#include <algorithm>
#include <cmath>

namespace wg
{

	const TypeInfo Tint::TYPEINFO = { "Tint", &Object::TYPEINFO };

	//____ create() ___________________________________________________________

	Tint_p Tint::create(const Blueprint& bp)
	{
		if (bp.stops.empty() || bp.stops.size() > c_maxStops)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "A Tint needs between 1 and 16 color stops.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		for (auto& stop : bp.stops)
		{
			if (!stop.color.isValid() || !std::isfinite(stop.pos))
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Invalid color stop.",
					nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
				return nullptr;
			}
		}

		if (bp.colorSpace == ColorSpace::Undefined)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "ColorSpace can not be Undefined.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		if (!std::isfinite(bp.begin.x) || !std::isfinite(bp.begin.y) || !std::isfinite(bp.end.x) || !std::isfinite(bp.end.y) ||
			!std::isfinite(bp.center.x) || !std::isfinite(bp.center.y) || !std::isfinite(bp.radius.w) || !std::isfinite(bp.radius.h))
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Invalid geometry.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		auto p = Tint_p(new Tint(bp));
		if (bp.finalizer)
			p->setFinalizer(bp.finalizer);
		return p;
	}

	Tint_p Tint::create(HiColor color)
	{
		Blueprint bp;
		bp.stops = { {0.f, color} };
		return create(bp);
	}

	Tint_p Tint::create(HiColor from, HiColor to, CoordF begin, CoordF end, ColorSpace colorSpace)
	{
		Blueprint bp;
		bp.begin = begin;
		bp.end = end;
		bp.colorSpace = colorSpace;
		bp.stops = { {0.f, from}, {1.f, to} };
		return create(bp);
	}

	Tint_p Tint::create(std::initializer_list<ColorStop> stops, CoordF begin, CoordF end, ColorSpace colorSpace)
	{
		Blueprint bp;
		bp.begin = begin;
		bp.end = end;
		bp.colorSpace = colorSpace;
		bp.stops = stops;
		return create(bp);
	}

	//____ blend() ____________________________________________________________
	/**
	 * @brief Get a Tint that is a blend between two Tints.
	 *
	 * If the Tints are morphable into each other (same shape, spread, color space
	 * and radius mode) the result is a simple Tint with interpolated geometry
	 * and colors. Otherwise the result is a mix of both, crossfaded per pixel.
	 *
	 * Blending from or to a mix flattens the mix instead of nesting it. A mix
	 * holds at most c_maxMixComponents Tints, the ones with the smallest weights
	 * are dropped if needed.
	 *
	 * @param pFrom		Tint at progress 0.0. Nullptr is treated as no tint (white).
	 * @param pTo		Tint at progress 1.0. Nullptr is treated as no tint (white).
	 * @param progress	0.0 -> 1.0.
	 */

	Tint_p Tint::blend(Tint* pFrom, Tint* pTo, float progress)
	{
		if (pFrom == pTo)
			return pFrom;

		if (!(progress > 0.f))						// Also catches NaN.
			return pFrom;
		if (progress >= 1.f)
			return pTo;

		// No tint is the same as a white tint.

		Tint_p pWhite;

		if (!pFrom || !pTo)
		{
			pWhite = create(HiColor::White);
			if (!pFrom)
				pFrom = pWhite;
			else
				pTo = pWhite;
		}

		// Morph if possible

		if (pFrom->isMorphableTo(pTo))
		{
			ColorStop	stops[c_maxStops];
			int			nStops;

			if (_morphStops(pFrom, pTo, progress, stops, nStops))
			{
				auto lerp = [progress](float a, float b) { return a + (b - a) * progress; };

				Tint_p p = new Tint();

				p->m_shape = pFrom->m_shape;
				p->m_spread = pFrom->m_spread;
				p->m_colorSpace = pFrom->m_colorSpace;
				p->m_radiusMode = pFrom->m_radiusMode;

				p->m_begin = { lerp(pFrom->m_begin.x, pTo->m_begin.x), lerp(pFrom->m_begin.y, pTo->m_begin.y) };
				p->m_end = { lerp(pFrom->m_end.x, pTo->m_end.x), lerp(pFrom->m_end.y, pTo->m_end.y) };
				p->m_center = { lerp(pFrom->m_center.x, pTo->m_center.x), lerp(pFrom->m_center.y, pTo->m_center.y) };
				p->m_radius = { lerp(pFrom->m_radius.w, pTo->m_radius.w), lerp(pFrom->m_radius.h, pTo->m_radius.h) };

				p->m_nStops = nStops;
				for (int i = 0; i < nStops; i++)
					p->m_stops[i] = stops[i];

				p->_updateFlags();
				return p;
			}
		}

		// Crossfade. Collect weighted components, flattening any mixes.

		struct Entry { Tint* pTint; float weight; };

		Entry	entries[c_maxMixComponents * 2];
		int		nEntries = 0;

		auto add = [&](Tint* pTint, float weight)
		{
			if (weight <= 0.f)
				return;

			for (int i = 0; i < nEntries; i++)
			{
				if (entries[i].pTint == pTint)
				{
					entries[i].weight += weight;
					return;
				}
			}
			entries[nEntries++] = { pTint, weight };
		};

		auto addTint = [&](Tint* pTint, float weight)
		{
			if (pTint->isMix())
			{
				for (int i = 0; i < pTint->m_nMixComponents; i++)
					add(pTint->m_mixComponents[i], pTint->m_mixWeights[i] * weight);
			}
			else
				add(pTint, weight);
		};

		addTint(pFrom, 1.f - progress);
		addTint(pTo, progress);

		std::stable_sort(entries, entries + nEntries, [](const Entry& a, const Entry& b) { return a.weight > b.weight; });

		if (nEntries > c_maxMixComponents)
			nEntries = c_maxMixComponents;

		if (nEntries == 1)
			return entries[0].pTint;

		float totalWeight = 0.f;
		for (int i = 0; i < nEntries; i++)
			totalWeight += entries[i].weight;

		Tint_p p = new Tint();

		p->m_nMixComponents = nEntries;
		for (int i = 0; i < nEntries; i++)
		{
			p->m_mixComponents[i] = entries[i].pTint;
			p->m_mixWeights[i] = entries[i].weight / totalWeight;
		}

		p->_updateFlags();
		return p;
	}

	//____ createMix() ______________________________________________________
	/**
	 * @brief Create a mix of simple Tints directly.
	 *
	 * Normally mixes are created by blend(). This is mainly for recreating a
	 * serialized mix. Mixes among the components are flattened, zero weights
	 * dropped and weights normalized. At most c_maxMixComponents components.
	 */

	Tint_p Tint::createMix(int nComponents, Tint* const* pComponents, const float* pWeights)
	{
		if (nComponents < 1 || nComponents > c_maxMixComponents || !pComponents || !pWeights)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Invalid number of mix components.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		float	total = 0.f;
		int		nValid = 0;

		for (int i = 0; i < nComponents; i++)
		{
			if (!pComponents[i] || pComponents[i]->isMix() || !(pWeights[i] >= 0.f))
			{
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Mix components must be simple Tints with non-negative weights.",
					nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
				return nullptr;
			}

			if (pWeights[i] > 0.f)
			{
				total += pWeights[i];
				nValid++;
			}
		}

		if (nValid == 0)
			return pComponents[0];

		if (nValid == 1)
		{
			for (int i = 0; i < nComponents; i++)
				if (pWeights[i] > 0.f)
					return pComponents[i];
		}

		Tint_p p = new Tint();

		for (int i = 0; i < nComponents; i++)
		{
			if (pWeights[i] > 0.f)
			{
				p->m_mixComponents[p->m_nMixComponents] = pComponents[i];
				p->m_mixWeights[p->m_nMixComponents] = pWeights[i] / total;
				p->m_nMixComponents++;
			}
		}

		p->_updateFlags();
		return p;
	}

	//____ constructor ________________________________________________________

	Tint::Tint()
	{
	}

	Tint::Tint(const Blueprint& bp)
	{
		m_shape = bp.shape;
		m_spread = bp.spread;
		m_colorSpace = bp.colorSpace;
		m_radiusMode = bp.radiusMode;

		m_begin = bp.begin;
		m_end = bp.end;
		m_center = bp.center;
		m_radius = bp.radius;

		m_nStops = (int) bp.stops.size();
		for (int i = 0; i < m_nStops; i++)
		{
			m_stops[i] = bp.stops[i];
			m_stops[i].pos = std::clamp(m_stops[i].pos, 0.f, 1.f);
		}

		// Stable sort so that stops with same position (hard edges) keep their order.

		std::stable_sort(m_stops, m_stops + m_nStops, [](const ColorStop& a, const ColorStop& b) { return a.pos < b.pos; });

		_updateFlags();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Tint::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ position() _________________________________________________________
	/**
	 * @brief Position along the gradient for a point, before spread is applied.
	 *
	 * @param pos	Point, in same unit as rect.
	 * @param rect	Rectangle the Tint is placed in.
	 *
	 * @return Position along the gradient, 0.0 at begin/center and 1.0 at end/radius.
	 * Always 0.0 for a mix.
	 */

	float Tint::position(CoordF pos, const RectF& rect) const
	{
		if (isMix())
			return 0.f;

		float w = rect.w > 0.f ? rect.w : 1.f;
		float h = rect.h > 0.f ? rect.h : 1.f;

		if (m_shape == TintShape::Linear)
		{
			// Calculated in relative space, so that e.g. a gradient from top-left to
			// bottom-right corner has its 0.5 line going through the other two corners.

			float u = (pos.x - rect.x) / w;
			float v = (pos.y - rect.y) / h;

			float dx = m_end.x - m_begin.x;
			float dy = m_end.y - m_begin.y;
			float len2 = dx * dx + dy * dy;

			if (len2 <= 0.f)
				return 0.f;

			return ((u - m_begin.x) * dx + (v - m_begin.y) * dy) / len2;
		}
		else
		{
			float radiusX, radiusY;

			if (m_radiusMode == TintRadius::Circle)
			{
				float shortest = std::min(w, h);
				radiusX = radiusY = m_radius.w * shortest;
			}
			else
			{
				radiusX = m_radius.w * w;
				radiusY = m_radius.h * h;
			}

			float dx = pos.x - (rect.x + m_center.x * w);
			float dy = pos.y - (rect.y + m_center.y * h);

			if (radiusX <= 0.f || radiusY <= 0.f)
				return (dx == 0.f && dy == 0.f) ? 0.f : 1e9f;

			dx /= radiusX;
			dy /= radiusY;

			return std::sqrt(dx * dx + dy * dy);
		}
	}

	//____ colorAtPosition() __________________________________________________
	/**
	 * @brief Color at a position along the gradient, with spread applied.
	 *
	 * Returns White for a mix, which has no single position.
	 */

	HiColor Tint::colorAtPosition(float position) const
	{
		if (isMix())
			return HiColor::White;

		return _colorAtStops(TintTools::applySpread(position, m_spread), false);
	}

	//____ colorAt() __________________________________________________________
	/**
	 * @brief Color of a pixel.
	 *
	 * @param pos	Top-left corner of the pixel, in spx. Color is sampled at the pixel center.
	 * @param rect	Rectangle the Tint is placed in, in spx.
	 */

	HiColor Tint::colorAt(CoordSPX pos, const RectSPX& rect) const
	{
		if (isMix())
		{
			float r = 0.f, g = 0.f, b = 0.f, a = 0.f;

			for (int i = 0; i < m_nMixComponents; i++)
			{
				HiColor c = m_mixComponents[i]->colorAt(pos, rect);
				float w = m_mixWeights[i];

				r += c.r * w;
				g += c.g * w;
				b += c.b * w;
				a += c.a * w;
			}

			return HiColor( (int) std::lround(r), (int) std::lround(g), (int) std::lround(b), (int) std::lround(a) );
		}

		CoordF	center = { float(pos.x + 32), float(pos.y + 32) };
		RectF	area = { float(rect.x), float(rect.y), float(rect.w), float(rect.h) };

		return colorAtPosition(position(center, area));
	}

	//____ alpha() ____________________________________________________________
	/**
	 * @brief Alpha of a pixel, 0 -> 4096.
	 *
	 * @param pos	Top-left corner of the pixel, in spx.
	 * @param rect	Rectangle the Tint is placed in, in spx.
	 */

	int Tint::alpha(CoordSPX pos, const RectSPX& rect) const
	{
		return colorAt(pos, rect).a;
	}

	//____ isMorphableTo() ____________________________________________________
	/**
	 * @brief Check if blend() between this and another Tint would morph rather than crossfade.
	 *
	 * Note that a morph can still fall back to crossfade if the Tints together have too many
	 * distinct stop positions.
	 */

	bool Tint::isMorphableTo(const Tint* pOther) const
	{
		if (!pOther || isMix() || pOther->isMix())
			return false;

		if (m_shape != pOther->m_shape || m_spread != pOther->m_spread || m_colorSpace != pOther->m_colorSpace)
			return false;

		if (m_shape == TintShape::Radial && m_radiusMode != pOther->m_radiusMode)
			return false;

		return true;
	}

	//____ _morphStops() ______________________________________________________
	/**
	 * Resample two lists of stops on the union of their positions and blend them.
	 * Hard edges in either list give a hard edge in the result.
	 * Returns false if the result would need more than c_maxStops stops.
	 */

	bool Tint::_morphStops(const Tint* pFrom, const Tint* pTo, float progress, ColorStop* pOutput, int& nOutput)
	{
		float	positions[c_maxStops * 2];
		int		nPositions = 0;

		for (int i = 0; i < pFrom->m_nStops; i++)
			positions[nPositions++] = pFrom->m_stops[i].pos;
		for (int i = 0; i < pTo->m_nStops; i++)
			positions[nPositions++] = pTo->m_stops[i].pos;

		std::sort(positions, positions + nPositions);
		nPositions = int(std::unique(positions, positions + nPositions) - positions);

		ColorSpace colorSpace = pFrom->m_colorSpace;
		nOutput = 0;

		for (int i = 0; i < nPositions; i++)
		{
			float pos = positions[i];

			HiColor fromLeft = pFrom->_colorAtStops(pos, true);
			HiColor fromRight = pFrom->_colorAtStops(pos, false);
			HiColor toLeft = pTo->_colorAtStops(pos, true);
			HiColor toRight = pTo->_colorAtStops(pos, false);

			HiColor right = _mixColors(fromRight, toRight, progress, colorSpace);

			if (fromLeft != fromRight || toLeft != toRight)
			{
				if (nOutput + 2 > c_maxStops)
					return false;

				pOutput[nOutput++] = { pos, _mixColors(fromLeft, toLeft, progress, colorSpace) };
			}
			else if (nOutput + 1 > c_maxStops)
				return false;

			pOutput[nOutput++] = { pos, right };
		}

		return true;
	}

	//____ _mixColors() _______________________________________________________

	HiColor Tint::_mixColors(HiColor from, HiColor to, float fraction, ColorSpace colorSpace)
	{
		return TintTools::mixColors(from, to, fraction, colorSpace);
	}

	//____ _colorAtStops() ____________________________________________________
	/**
	 * Color at position (0.0 -> 1.0, spread already applied).
	 *
	 * At a hard edge (two stops with same position) the right limit is the second
	 * stop and the left limit is the first.
	 */

	HiColor Tint::_colorAtStops(float position, bool bLeftLimit) const
	{
		if (m_nStops == 1)
			return m_stops[0].color;

		const ColorStop* pFirst = m_stops;
		const ColorStop* pLast = m_stops + m_nStops - 1;

		if (position < pFirst->pos || (bLeftLimit && position == pFirst->pos))
			return pFirst->color;

		if (position > pLast->pos || (!bLeftLimit && position == pLast->pos))
			return pLast->color;

		int idx;		// Stop before position.

		if (bLeftLimit)
		{
			// First stop with pos >= position, interpolate from the one before.

			idx = 0;
			while (m_stops[idx].pos < position)
				idx++;
			idx--;
		}
		else
		{
			// Last stop with pos <= position.

			idx = m_nStops - 1;
			while (m_stops[idx].pos > position)
				idx--;
		}

		const ColorStop& a = m_stops[idx];
		const ColorStop& b = m_stops[idx + 1];

		float span = b.pos - a.pos;
		float fraction = span > 0.f ? (position - a.pos) / span : 1.f;

		return TintTools::mixColors(a.color, b.color, fraction, m_colorSpace);
	}

	//____ _updateFlags() _____________________________________________________

	void Tint::_updateFlags()
	{
		if (isMix())
		{
			m_bOpaque = true;
			for (int i = 0; i < m_nMixComponents; i++)
				m_bOpaque = m_bOpaque && m_mixComponents[i]->m_bOpaque;

			m_bFlat = false;
			return;
		}

		m_bOpaque = true;
		m_bFlat = true;

		for (int i = 0; i < m_nStops; i++)
		{
			if (m_stops[i].color.a != 4096)
				m_bOpaque = false;

			if (m_stops[i].color != m_stops[0].color)
				m_bFlat = false;
		}
	}

}
