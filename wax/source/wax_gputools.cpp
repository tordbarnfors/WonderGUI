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
#include <wax_gputools.h>
#include <wax_util.h>

#include <cmath>
#include <cstdlib>
#include <utility>

namespace wax
{
	namespace GpuTools
	{
		namespace
		{
			// Widths a line of a given slope needs to keep an even thickness, indexed by slope * 16.

			struct LineThicknessTable
			{
				float	scale[17];

				LineThicknessTable()
				{
					for (int i = 0; i < 17; i++)
					{
						double b = i / 16.0;
						scale[i] = (float)Util::squareRoot(1.0 + b * b);
					}
				}
			};

			const LineThicknessTable	s_lineThickness;
		}

		//____ decodeStateChange() ____________________________________________

		const uint16_t* decodeStateChange(const uint16_t* p, const HiColor*& pColors, StateChangeParams& params)
		{
			// Every payload has to be stepped past even when a backend ignores it, or it loses
			// track of where it is in the command stream. The order is fixed by DeviceGen2.

			uint8_t changed = uint8_t(*p++);
			params.changed = changed;

			if (changed & uint8_t(Backend::StateChange::BlitSource))
				params.blitSource = Backend::readObjectIndex(p);

			if (changed & uint8_t(Backend::StateChange::TintColor))
				params.tintColor = *pColors++;

			if (changed & uint8_t(Backend::StateChange::Tint))
				p = TintTools::decodeTint(p, pColors, params.tint);

			if (changed & uint8_t(Backend::StateChange::BlendMode))
				params.blendMode = BlendMode(*p++);

			if (changed & uint8_t(Backend::StateChange::MorphFactor))
				params.morphFactor = *p++;

			if (changed & uint8_t(Backend::StateChange::FixedBlendColor))
				params.fixedBlendColor = *pColors++;

			if (changed & uint8_t(Backend::StateChange::Blur))
			{
				params.blurRadius = *p++;
				params.pBlurWeights = p;
				p += 27;
			}

			// Commands start at 32-bit boundaries.

			if ((uintptr_t(p) & 0x2) == 2)
				p++;

			return p;
		}

		//____ blurOffsets() __________________________________________________

		void blurOffsets(spx radius, SizeI sourceSize, float offsets[9][2])
		{
			float w = float(sourceSize.w > 0 ? sourceSize.w : 1);
			float h = float(sourceSize.h > 0 ? sourceSize.h : 1);

			int corner = radius * 7 / 10;

			float posX = float((radius + 32) >> 6) / w, negX = float((-radius + 32) >> 6) / w;
			float posY = float((radius + 32) >> 6) / h, negY = float((-radius + 32) >> 6) / h;
			float cPosX = float((corner + 32) >> 6) / w, cNegX = float((-corner + 32) >> 6) / w;
			float cPosY = float((corner + 32) >> 6) / h, cNegY = float((-corner + 32) >> 6) / h;

			const float taps[9][2] = {
				{ cNegX, cNegY },	{ 0.f, negY },	{ cPosX, cNegY },
				{ negX, 0.f },		{ 0.f, 0.f },	{ posX, 0.f },
				{ cNegX, cPosY },	{ 0.f, posY },	{ cPosX, cPosY } };

			for (int i = 0; i < 9; i++)
			{
				offsets[i][0] = taps[i][0];
				offsets[i][1] = taps[i][1];
			}
		}

		//____ scaleLineThickness() ___________________________________________

		float scaleLineThickness(float thickness, float slope)
		{
			slope = std::abs(slope);

			float scale = s_lineThickness.scale[(int)(slope * 16)];

			if (slope < 1.f)
			{
				float scale2 = s_lineThickness.scale[(int)(slope * 16) + 1];
				scale += (scale2 - scale) * ((slope * 16) - ((int)(slope * 16)));
			}

			return thickness * scale;
		}

		//____ lineQuad() _____________________________________________________

		bool lineQuad(CoordSPX beginSpx, CoordSPX endSpx, spx thicknessSpx, LineQuad& quad)
		{
			float thickness = thicknessSpx / 64.f;

			CoordI begin = Util::roundToPixels(beginSpx);
			CoordI end = Util::roundToPixels(endSpx);

			if (std::abs(begin.x - end.x) > std::abs(begin.y - end.y))
			{
				// Mainly horizontal, so it is measured along x.

				if (begin.x > end.x)
					std::swap(begin, end);

				int length = end.x - begin.x;
				if (length == 0)
					return false;						//TODO: Should still draw the caps!

				float slope = ((float)(end.y - begin.y)) / length;
				float width = scaleLineThickness(thickness, slope);
				float w = width / 2 + 0.5f;

				// The line passes through the centers of the pixels at begin and end.

				float y1 = begin.y + 0.5f - slope / 2;			// Center of the line at the left side of the quad.
				float y2 = y1 + slope * length;					// ...and at the right side.

				quad.corners[0] = { begin.x, int(std::floor(y1 - w)) };
				quad.corners[1] = { end.x, int(std::floor(y2 - w)) };
				quad.corners[2] = { end.x, int(std::ceil(y2 + w)) };
				quad.corners[3] = { begin.x, int(std::ceil(y1 + w)) };

				quad.offset = (begin.y + 0.5f) - (begin.x + 0.5f) * slope;
				quad.w = w;
				quad.slope = slope;
				quad.bSteep = false;
			}
			else
			{
				// Mainly vertical, so x and y swap roles.

				if (begin.y > end.y)
					std::swap(begin, end);

				int length = end.y - begin.y;
				if (length == 0)
					return false;						//TODO: Should still draw the caps!

				float slope = ((float)(end.x - begin.x)) / length;
				float width = scaleLineThickness(thickness, slope);
				float w = width / 2 + 0.5f;

				float x1 = begin.x + 0.5f - slope / 2;			// Center of the line at the top of the quad.
				float x2 = x1 + slope * length;					// ...and at the bottom.

				quad.corners[0] = { int(std::floor(x1 - w)), begin.y };
				quad.corners[1] = { int(std::ceil(x1 + w)), begin.y };
				quad.corners[2] = { int(std::ceil(x2 + w)), end.y };
				quad.corners[3] = { int(std::floor(x2 - w)), end.y };

				quad.offset = (begin.x + 0.5f) - (begin.y + 0.5f) * slope;
				quad.w = w;
				quad.slope = slope;
				quad.bSteep = true;
			}

			quad.begin = begin;
			return true;
		}
	}
}
