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
#include <wg_gpushapes.h>
#include <wg_gfxutil.h>
#include <wg_tinttools.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace wg
{
	namespace GpuShapes
	{
		namespace
		{
			inline void put(float* pEntries, int entry, float a, float b, float c, float d)
			{
				float* p = pEntries + entry * 4;
				p[0] = a;
				p[1] = b;
				p[2] = c;
				p[3] = d;
			}

			inline float pixels(int64_t spxValue)
			{
				return float(spxValue) / 64.f;
			}

			// Radius of a corner in pixels. Smaller corners are square, as in SoftShapes.

			inline float corner(spx radius)
			{
				return radius >= 4 ? radius / 64.f : 0.f;
			}

			// Steps past an encoded tint if bTinted, returns where it starts or null.

			const uint16_t* takeTint(bool bTinted, const uint16_t*& pTint, const HiColor*& pTintColors, const HiColor*& pColorsOfTint)
			{
				if (!bTinted)
				{
					pColorsOfTint = nullptr;
					return nullptr;
				}

				const uint16_t* pThis = pTint;
				pColorsOfTint = pTintColors;

				int nColors;
				pTint += TintTools::encodedTintSize(pTint, nColors);
				pTintColors += nColors;
				return pThis;
			}

			//____ writeRoundedBox() __________________________________________

			int writeRoundedBox(const uint16_t* pPayload, const HiColor* pColors, float* pEntries, Segment* pSegments)
			{
				using Flags = GfxBackend::RoundedBoxFlags;

				const GfxBackend::RoundedBoxPayload box = GfxBackend::decodeRoundedBox(pPayload);
				auto has = [&box](Flags flag) { return (box.flags & uint16_t(flag)) != 0; };

				const RectSPX& rect = box.rect;

				spx radius[4] = { box.radius[0], box.radius[1], box.radius[2], box.radius[3] };
				Util::clampCornerRadii(rect.size(), radius);

				const BorderSPX outline = { std::max(box.outline.top, 0), std::max(box.outline.right, 0),
											std::max(box.outline.bottom, 0), std::max(box.outline.left, 0) };

				const bool bOutline = (outline.top | outline.right | outline.bottom | outline.left) != 0;

				spx innerRadius[4];
				Util::innerCornerRadii(radius, outline, innerRadius);

				const int64_t right = int64_t(rect.x) + rect.w;
				const int64_t bottom = int64_t(rect.y) + rect.h;

				put(pEntries, 0, 1.f, bOutline ? float(c_boxOutline) : 0.f, 0.f, 0.f);
				put(pEntries, 1, pixels(rect.x), pixels(rect.y), pixels(right), pixels(bottom));
				put(pEntries, 2, corner(radius[0]), corner(radius[1]), corner(radius[2]), corner(radius[3]));
				put(pEntries, 3, pixels(int64_t(rect.x) + outline.left), pixels(int64_t(rect.y) + outline.top), pixels(right - outline.right), pixels(bottom - outline.bottom));
				put(pEntries, 4, corner(innerRadius[0]), corner(innerRadius[1]), corner(innerRadius[2]), corner(innerRadius[3]));

				// Colors come first, then the colors of the tints in the same order.

				const bool bFill = has(Flags::HasFill);
				const bool bOutlineDrawn = has(Flags::HasOutline);

				const uint16_t* pTint = box.pTints;
				const HiColor* pTintColors = pColors + (bFill ? 1 : 0) + (bOutlineDrawn ? 1 : 0);

				auto segment = [&](Segment& seg, int entry, bool bDrawn, bool bTinted)
				{
					seg.entry = entry;
					seg.bDrawn = bDrawn;
					seg.color = bDrawn ? *pColors++ : HiColor::Transparent;
					seg.pTint = takeTint(bTinted, pTint, pTintColors, seg.pTintColors);
					put(pEntries, entry, -1.f, -1.f, 0.f, 0.f);
				};

				segment(pSegments[0], 5, bFill, has(Flags::HasFillTint));
				segment(pSegments[1], 6, bOutlineDrawn, has(Flags::HasOutlineTint));
				return 7;
			}

			//____ writeRing() ________________________________________________

			int writeRing(const uint16_t* pPayload, const HiColor* pColors, float* pEntries, Segment* pSegments)
			{
				using Flags = GfxBackend::RingFlags;
				using SliceFlags = GfxBackend::RingSliceFlags;

				const GfxBackend::RingPayload ring = GfxBackend::decodeRing(pPayload);
				auto has = [&ring](Flags flag) { return (ring.flags & uint16_t(flag)) != 0; };
				auto sliceHas = [&ring](int slice, SliceFlags flag) { return (ring.sliceFlags(slice) & uint16_t(flag)) != 0; };

				const int nSlices = ring.nSlices;
				const RectSPX& rect = ring.rect;
				const bool bSquare = has(Flags::Square);

				// Radial geometry, as SoftShapes::rasterizeRing().

				const int64_t cx = int64_t(rect.x) + rect.w / 2;
				const int64_t cy = int64_t(rect.y) + rect.h / 2;
				const int64_t outerRadius = std::min(rect.w, rect.h) / 2;

				const bool bHole = ring.thickness > 0 && ring.thickness < outerRadius;
				const int64_t innerRadius = bHole ? outerRadius - ring.thickness : 0;

				const int64_t outline = std::min(int64_t(std::max(ring.outline, 0)), bHole ? int64_t(ring.thickness / 2) : outerRadius);
				const bool bOutline = outline > 0;

				const int64_t radius1 = bOutline ? std::max(outerRadius - outline, int64_t(0)) : outerRadius;
				const int64_t radius2 = (bOutline && bHole) ? innerRadius + outline : innerRadius;

				const int64_t right = int64_t(rect.x) + rect.w;
				const int64_t bottom = int64_t(rect.y) + rect.h;
				const int64_t inset = bOutline ? outline : 0;

				bool bSlices;
				if (bSquare)
					bSlices = right - inset > rect.x + inset && bottom - inset > rect.y + inset;
				else
					bSlices = radius1 > 0;

				if (bHole)
				{
					int64_t sliceRadius = bSquare ? std::min(cx - (rect.x + inset), cy - (rect.y + inset)) : radius1;
					if (sliceRadius <= radius2)
						bSlices = false;
				}

				int flags = 0;
				if (bSquare)
					flags |= c_ringSquare;
				if (has(Flags::HasBack) && !bSquare)
					flags |= c_ringBack;
				if (bHole)
					flags |= c_ringHole;
				if (bOutline)
					flags |= c_ringOutline;
				if (bSlices)
					flags |= c_ringSlices;

				put(pEntries, 1, pixels(cx), pixels(cy), pixels(outerRadius), pixels(radius1));
				put(pEntries, 2, pixels(radius2), pixels(innerRadius), 0.f, 0.f);
				put(pEntries, 3, pixels(rect.x), pixels(rect.y), pixels(right), pixels(bottom));
				put(pEntries, 4, pixels(rect.x + inset), pixels(rect.y + inset), pixels(right - inset), pixels(bottom - inset));

				// Segments. Colors come first, then the colors of the tints in the same order:
				// outline, hub, back and the slices.

				const int backEntry = 5;
				const int firstSliceEntry = 8;

				const HiColor* pTintColors = pColors + (has(Flags::HasOutline) ? 1 : 0) + (has(Flags::HasHub) ? 1 : 0) + (has(Flags::HasBack) ? 1 : 0);
				for (int i = 0; i < nSlices; i++)
					if (sliceHas(i, SliceFlags::Drawn))
						pTintColors++;

				const uint16_t* pTint = ring.pTints;

				auto segment = [&](Segment& seg, int entry, bool bDrawn, bool bTinted)
				{
					seg.entry = entry;
					seg.bDrawn = bDrawn;
					seg.color = bDrawn ? *pColors++ : HiColor::Transparent;
					seg.pTint = takeTint(bTinted, pTint, pTintColors, seg.pTintColors);
					put(pEntries, entry, -1.f, -1.f, 0.f, 0.f);
				};

				segment(pSegments[1], backEntry + 1, has(Flags::HasOutline), has(Flags::HasOutlineTint));
				segment(pSegments[2], backEntry + 2, has(Flags::HasHub), has(Flags::HasHubTint));
				segment(pSegments[0], backEntry, has(Flags::HasBack), has(Flags::HasBackTint));

				for (int i = 0; i < nSlices; i++)
					segment(pSegments[3 + i], firstSliceEntry + i, sliceHas(i, SliceFlags::Drawn), sliceHas(i, SliceFlags::Tinted));

				// Elements: the slices, skipping empty ones and cut at a full turn, followed by
				// a gap if they don't make a full turn. Angles at twice the scale of the payload,
				// 2^32 to a turn, so they wrap around by themselves.

				struct Element
				{
					uint32_t	angle;
					int			entry;
				};

				std::vector<Element> elements;
				elements.reserve(nSlices + 1);

				const uint64_t fullTurn = uint64_t(1) << 32;
				const uint32_t start = (ring.start % GfxBackend::c_shapeFullTurn) << 1;

				uint64_t pos = 0;
				for (int i = 0; i < nSlices && pos < fullTurn; i++)
				{
					uint64_t size = uint64_t(std::min(ring.sliceSize(i), GfxBackend::c_shapeFullTurn)) << 1;
					if (size == 0)
						continue;

					elements.push_back({ uint32_t(start + pos), firstSliceEntry + i });
					pos += size;
				}

				if (pos < fullTurn)
					elements.push_back({ uint32_t(start + pos), backEntry });

				std::sort(elements.begin(), elements.end(), [](const Element& a, const Element& b) { return a.angle < b.angle; });

				const int firstElementEntry = firstSliceEntry + nSlices;
				const int nElements = (int)elements.size();

				for (int i = 0; i < nElements; i++)
					put(pEntries, firstElementEntry + i, float(elements[i].angle / double(fullTurn)), float(elements[i].entry), 0.f, 0.f);

				put(pEntries, 0, 2.f, float(flags), float(nElements), float(firstElementEntry));
				return firstElementEntry + nElements;
			}
		}

		//____ measure() ______________________________________________________

		void measure(GfxBackend::ShapeKind kind, const uint16_t* pPayload, int& maxEntries, int& nSegments)
		{
			if (kind == GfxBackend::ShapeKind::RoundedBox)
			{
				maxEntries = 7;
				nSegments = 2;
			}
			else if (kind == GfxBackend::ShapeKind::Ring)
			{
				int nSlices = GfxBackend::decodeRing(pPayload).nSlices;
				maxEntries = 9 + nSlices * 2;
				nSegments = 3 + nSlices;
			}
			else
			{
				maxEntries = 0;
				nSegments = 0;
			}
		}

		//____ write() ________________________________________________________

		int write(GfxBackend::ShapeKind kind, const uint16_t* pPayload, const HiColor* pColors, float* pEntries, Segment* pSegments)
		{
			if (kind == GfxBackend::ShapeKind::RoundedBox)
				return writeRoundedBox(pPayload, pColors, pEntries, pSegments);

			if (kind == GfxBackend::ShapeKind::Ring)
				return writeRing(pPayload, pColors, pEntries, pSegments);

			return 0;
		}
	
		//____ writeLine() ____________________________________________________

		int writeLine(CoordSPX begin, CoordSPX end, spx thickness, LineCaps caps, float* pEntries, CoordF* pQuad)
		{
			if (thickness <= 0)
				return 0;

			const double dx = (end.x - double(begin.x)) / 64;
			const double dy = (end.y - double(begin.y)) / 64;
			const double length = std::sqrt(dx * dx + dy * dy);

			const bool bAlongX = length == 0.0 || std::abs(dx) > std::abs(dy);
			const double ux = length > 0.0 ? dx / length : 1.0;
			const double uy = length > 0.0 ? dy / length : 0.0;
			const double r = thickness / 128.0;

			// How far each end reaches beyond its endpoint, with caps resolved.

			const double c_maxReach = 65536.0;
			double reach[2];
			float codes[2];

			LineCap ends[2] = { caps.begin, caps.end };

			for (int i = 0; i < 2; i++)
			{
				LineCapShape shape = ends[i].shape;

				if (shape == LineCapShape::None)
					shape = bAlongX ? LineCapShape::Vertical : LineCapShape::Horizontal;

				if ((shape == LineCapShape::Vertical && ux == 0.0) || (shape == LineCapShape::Horizontal && uy == 0.0))
					return 0;

				switch (shape)
				{
					case LineCapShape::Square:
					case LineCapShape::Round:
						reach[i] = r;
						break;
					case LineCapShape::Vertical:
						reach[i] = std::min(r * std::abs(uy / ux), c_maxReach);
						break;
					case LineCapShape::Horizontal:
						reach[i] = std::min(r * std::abs(ux / uy), c_maxReach);
						break;
					default:
						reach[i] = 0.0;
						break;
				}

				int spear = shape == LineCapShape::Spear ? std::min(std::max((ends[i].spearLength + 12) / 25, 1), 32) : 0;
				codes[i] = float(int(shape) + 8 * spear);
			}

			pEntries[0] = float(c_lineKind);
			pEntries[1] = codes[0];
			pEntries[2] = codes[1];
			pEntries[3] = 0.f;

			pEntries[4] = float(begin.x / 64.0);
			pEntries[5] = float(begin.y / 64.0);
			pEntries[6] = float(end.x / 64.0);
			pEntries[7] = float(end.y / 64.0);

			pEntries[8] = float(ux);
			pEntries[9] = float(uy);
			pEntries[10] = float(length);
			pEntries[11] = float(r);

			// Quad around the band from reach before begin to reach after end, a pixel and a half
			// wider all around, so that it holds all pixels touched even with its corners rounded.

			const double margin = 1.5;
			const double a0 = -reach[0] - margin;
			const double a1 = length + reach[1] + margin;
			const double w = r + margin;

			const double bx = begin.x / 64.0;
			const double by = begin.y / 64.0;

			auto corner = [&](double a, double across) { return CoordF(float(bx + a * ux - across * uy), float(by + a * uy + across * ux)); };

			pQuad[0] = corner(a0, -w);
			pQuad[1] = corner(a1, -w);
			pQuad[2] = corner(a1, w);
			pQuad[3] = corner(a0, w);

			return c_lineEntries;
		}
}
}
