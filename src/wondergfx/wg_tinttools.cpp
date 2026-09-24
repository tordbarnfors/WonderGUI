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

#include <wg_tinttools.h>

#include <algorithm>
#include <cmath>

namespace wg
{
namespace TintTools
{
	// HiColor channels are linear, 0 -> 4096. When interpolating in sRGB we work
	// with sRGB values on the same 0 -> 4096 scale.

	static float _linearToSRGB(int linear)
	{
		float v = linear / 4096.f;
		float s = v <= 0.0031308f ? v * 12.92f : 1.055f * std::pow(v, 1.f / 2.4f) - 0.055f;
		return s * 4096.f;
	}

	static float _srgbToLinearExact(float srgb)
	{
		float s = srgb / 4096.f;
		float v = s <= 0.04045f ? s / 12.92f : std::pow((s + 0.055f) / 1.055f, 2.4f);
		return v * 4096.f;
	}

	// Table for converting sRGB on 0 -> 4096 scale back to linear, used when building
	// lookup tables so we don't need a pow() per entry and channel.

	static const int16_t* _srgbToLinearTab()
	{
		static int16_t	tab[4097];
		static bool		bInitialized = [] {
			for (int i = 0; i <= 4096; i++)
				tab[i] = (int16_t) std::lround(_srgbToLinearExact((float) i));
			return true;
		}();

		(void) bInitialized;
		return tab;
	}

	//____ canvasGeometry() ___________________________________________________

	CanvasGeometry canvasGeometry(const Tint* pTint, const RectSPX& rect)
	{
		CanvasGeometry geo = {};
		geo.shape = pTint->shape();

		// Rect in canvas pixels (floats, so subpixel rects are exact).

		float rx = rect.x / 64.f;
		float ry = rect.y / 64.f;
		float rw = rect.w > 0 ? rect.w / 64.f : 1.f / 64.f;
		float rh = rect.h > 0 ? rect.h / 64.f : 1.f / 64.f;

		if (geo.shape == TintShape::Linear)
		{
			// position = A*u + B*v + C, where u,v are relative to rect (0.0 -> 1.0).

			CoordF begin = pTint->begin();
			CoordF end = pTint->end();

			float dx = end.x - begin.x;
			float dy = end.y - begin.y;
			float len2 = dx * dx + dy * dy;

			float A = 0.f, B = 0.f, C = 0.f;

			if (len2 > 0.f)
			{
				A = dx / len2;
				B = dy / len2;
				C = -(begin.x * dx + begin.y * dy) / len2;
			}

			// u = (X - rx) / rw, v = (Y - ry) / rh

			geo.a = A / rw;
			geo.b = B / rh;
			geo.c = C - A * rx / rw - B * ry / rh;
		}
		else
		{
			CoordF center = pTint->center();
			SizeF radius = pTint->radius();

			geo.centerX = rx + center.x * rw;
			geo.centerY = ry + center.y * rh;

			float radiusX, radiusY;

			if (pTint->radiusMode() == TintRadius::Circle)
				radiusX = radiusY = radius.w * std::min(rw, rh);
			else
			{
				radiusX = radius.w * rw;
				radiusY = radius.h * rh;
			}

			geo.invRadiusX = radiusX > 0.f ? 1.f / radiusX : 1e6f;
			geo.invRadiusY = radiusY > 0.f ? 1.f / radiusY : 1e6f;
		}

		return geo;
	}

	//____ positionAt() _______________________________________________________

	float positionAt(const CanvasGeometry& geo, float x, float y)
	{
		if (geo.shape == TintShape::Linear)
			return geo.a * x + geo.b * y + geo.c;

		float dx = (x - geo.centerX) * geo.invRadiusX;
		float dy = (y - geo.centerY) * geo.invRadiusY;
		return std::sqrt(dx * dx + dy * dy);
	}

	//____ mixColors() ________________________________________________________

	HiColor mixColors(HiColor from, HiColor to, float fraction, ColorSpace colorSpace)
	{
		if (fraction <= 0.f)
			return from;
		if (fraction >= 1.f)
			return to;

		auto lerp = [fraction](float a, float b) { return a + (b - a) * fraction; };

		int a = (int) std::lround(lerp(from.a, to.a));

		if (colorSpace == ColorSpace::sRGB)
		{
			float r = lerp(_linearToSRGB(from.r), _linearToSRGB(to.r));
			float g = lerp(_linearToSRGB(from.g), _linearToSRGB(to.g));
			float b = lerp(_linearToSRGB(from.b), _linearToSRGB(to.b));

			return HiColor((int) std::lround(_srgbToLinearExact(r)), (int) std::lround(_srgbToLinearExact(g)),
							(int) std::lround(_srgbToLinearExact(b)), a);
		}
		else
		{
			return HiColor((int) std::lround(lerp(from.r, to.r)), (int) std::lround(lerp(from.g, to.g)),
							(int) std::lround(lerp(from.b, to.b)), a);
		}
	}

	//____ buildLUT() _________________________________________________________

	void buildLUT(const Tint* pTint, int entries, HiColor* pOutput, float begin, float end, bool bSquared)
	{
		if (entries <= 0)
			return;

		if (pTint->isMix())
		{
			for (int i = 0; i < entries; i++)
				pOutput[i] = HiColor::White;
			return;
		}

		int nStops = pTint->nbStops();
		const ColorStop* pStops = pTint->stops();

		if (pTint->isFlat() || nStops == 1)
		{
			for (int i = 0; i < entries; i++)
				pOutput[i] = pStops[0].color;
			return;
		}

		// Convert stop colors to the space we interpolate in.

		bool bSRGB = (pTint->colorSpace() == ColorSpace::sRGB);

		float	stopColors[Tint::c_maxStops][4];

		for (int i = 0; i < nStops; i++)
		{
			const HiColor& c = pStops[i].color;

			if (bSRGB)
			{
				stopColors[i][0] = _linearToSRGB(c.r);
				stopColors[i][1] = _linearToSRGB(c.g);
				stopColors[i][2] = _linearToSRGB(c.b);
			}
			else
			{
				stopColors[i][0] = c.r;
				stopColors[i][1] = c.g;
				stopColors[i][2] = c.b;
			}
			stopColors[i][3] = c.a;
		}

		const int16_t* pToLinear = bSRGB ? _srgbToLinearTab() : nullptr;

		TintSpread spread = pTint->spread();
		float firstPos = pStops[0].pos;
		float lastPos = pStops[nStops - 1].pos;

		for (int i = 0; i < entries; i++)
		{
			float f = entries > 1 ? i / float(entries - 1) : 0.f;
			float position = bSquared ? end * std::sqrt(f) : begin + (end - begin) * f;
			float t = applySpread(position, spread);

			// Find stops to interpolate between (right limit at hard edges).

			float col[4];

			if (t <= firstPos && !(t == firstPos && nStops > 1 && pStops[1].pos == firstPos))
			{
				for (int c = 0; c < 4; c++)
					col[c] = stopColors[0][c];
			}
			else if (t >= lastPos)
			{
				for (int c = 0; c < 4; c++)
					col[c] = stopColors[nStops - 1][c];
			}
			else
			{
				int idx = nStops - 1;
				while (pStops[idx].pos > t)
					idx--;

				float span = pStops[idx + 1].pos - pStops[idx].pos;
				float frac = span > 0.f ? (t - pStops[idx].pos) / span : 1.f;

				for (int c = 0; c < 4; c++)
					col[c] = stopColors[idx][c] + (stopColors[idx + 1][c] - stopColors[idx][c]) * frac;
			}

			auto toInt = [](float v) { int i = (int) std::lround(v); return i < 0 ? 0 : i > 4096 ? 4096 : i; };

			HiColor& out = pOutput[i];

			if (bSRGB)
			{
				out.r = pToLinear[toInt(col[0])];
				out.g = pToLinear[toInt(col[1])];
				out.b = pToLinear[toInt(col[2])];
			}
			else
			{
				out.r = (int16_t) toInt(col[0]);
				out.g = (int16_t) toInt(col[1]);
				out.b = (int16_t) toInt(col[2]);
			}
			out.a = (int16_t) toInt(col[3]);
		}
	}

}
}
