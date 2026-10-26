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

#include <wax_tinttools.h>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace wax
{
namespace TintTools
{
	// HiColor channels are sRGB, so Tints are interpolated directly on them. Renderers
	// convert the resulting colors to linear before blending.

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

	HiColor mixColors(HiColor from, HiColor to, float fraction)
	{
		if (fraction <= 0.f)
			return from;
		if (fraction >= 1.f)
			return to;

		auto lerp = [fraction](float a, float b) { return (int) std::lround(a + (b - a) * fraction); };

		return HiColor(lerp(from.r, to.r), lerp(from.g, to.g), lerp(from.b, to.b), lerp(from.a, to.a));
	}

	//____ _buildLUT() ________________________________________________________

	static void _buildLUT(int nStops, const float* pStopPos, const HiColor* pStopColors, TintSpread spread,
						  int entries, HiColor* pOutput, float begin, float end, bool bSquared, HiColor multiplier)
	{
		if (entries <= 0)
			return;

		bool bMultiply = (multiplier != HiColor::White);

		float	stopColors[Tint::c_maxStops][4];

		for (int i = 0; i < nStops; i++)
		{
			const HiColor& c = pStopColors[i];

			stopColors[i][0] = c.r;
			stopColors[i][1] = c.g;
			stopColors[i][2] = c.b;
			stopColors[i][3] = c.a;
		}

		float firstPos = pStopPos[0];
		float lastPos = pStopPos[nStops - 1];

		for (int i = 0; i < entries; i++)
		{
			float f = entries > 1 ? i / float(entries - 1) : 0.f;
			float position = bSquared ? end * std::sqrt(f) : begin + (end - begin) * f;
			float t = applySpread(position, spread);

			// Find stops to interpolate between (right limit at hard edges).

			float col[4];

			if (nStops == 1 || (t <= firstPos && !(t == firstPos && pStopPos[1] == firstPos)))
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
				while (pStopPos[idx] > t)
					idx--;

				float span = pStopPos[idx + 1] - pStopPos[idx];
				float frac = span > 0.f ? (t - pStopPos[idx]) / span : 1.f;

				for (int c = 0; c < 4; c++)
					col[c] = stopColors[idx][c] + (stopColors[idx + 1][c] - stopColors[idx][c]) * frac;
			}

			auto toInt = [](float v) { int i = (int) std::lround(v); return i < 0 ? 0 : i > 4096 ? 4096 : i; };

			HiColor& out = pOutput[i];

			out.r = (int16_t) toInt(col[0]);
			out.g = (int16_t) toInt(col[1]);
			out.b = (int16_t) toInt(col[2]);
			out.a = (int16_t) toInt(col[3]);

			if (bMultiply)
				out = out * multiplier;
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

		float	pos[Tint::c_maxStops];
		HiColor	colors[Tint::c_maxStops];

		for (int i = 0; i < nStops; i++)
		{
			pos[i] = pStops[i].pos;
			colors[i] = pStops[i].color;
		}

		_buildLUT(nStops, pos, colors, pTint->spread(), entries, pOutput, begin, end, bSquared, HiColor::White);
	}

	//____ _floorDiv() ________________________________________________________

	static inline int _floorDiv(int num, int den)		// den > 0
	{
		int q = num / den;
		return (num % den < 0) ? q - 1 : q;
	}

	//____ _clampedStopColor() ________________________________________________

	static inline void _clampedStopColor(const HiColor& color, int* pOut)
	{
		auto clamp = [](int v) { return v < 0 ? 0 : v > 4096 ? 4096 : v; };

		pOut[0] = clamp(color.r);
		pOut[1] = clamp(color.g);
		pOut[2] = clamp(color.b);
		pOut[3] = clamp(color.a);
	}

	//____ buildLUT() _________________________________________________________

	void buildLUT(const TintLayer& layer, int entries, HiColor* pOutput, HiColor multiplier)
	{
		// Entry i is sampled at (i + 0.5) / (entries - 1), so that a renderer that looks up floor(position * (entries - 1))
		// gets the color of the middle of the span the entry covers and hard edges end up exactly on entry boundaries.
		// Last entry is sampled at 1.0, the clamped position of (entries - 0.5) / (entries - 1).
		//
		// Positions are compared as fractions with denominator 2 * (entries - 1) * 65535, which makes entry i
		// (2i + 1) * 65535 and stop k 2 * (entries - 1) * stopPos16[k]. Odd and even, so entries never are
		// exactly on a stop. Each channel between two stops is c0 + (c1 - c0) * (pos - stopPos) / span, rounded
		// to nearest, which is stepped from entry to entry as a quotient and remainder.

		if (entries <= 0)
			return;

		int nStops = layer.nStops;

		if (entries == 1 || nStops <= 1)
		{
			HiColor color = layerColorAtFixed(layer, 0, multiplier);
			for (int i = 0; i < entries; i++)
				pOutput[i] = color;
			return;
		}

		// Up to 4097 entries keeps numerators and remainders below 2^31. More is outside the contract,
		// the extra entries get the last color.

		const int	spans = std::min(entries, 4097) - 1;
		const int	entryStep = 2 * 65535;
		const bool	bMultiply = (multiplier != HiColor::White);

		int		colors[Tint::c_maxStops][4];
		for (int s = 0; s < nStops; s++)
			_clampedStopColor(layer.stopColors[s], colors[s]);

		auto put = [&](int i, const int* pColor)
		{
			HiColor& out = pOutput[i];
			out.r = (int16_t) pColor[0];
			out.g = (int16_t) pColor[1];
			out.b = (int16_t) pColor[2];
			out.a = (int16_t) pColor[3];

			if (bMultiply)
				out = out * multiplier;
		};

		int i = 0;
		int pos = 65535;								// (2i + 1) * 65535 for entry i.

		// Before first stop

		int stopPos = 2 * spans * layer.stopPos16[0];

		while (i < spans && pos < stopPos)
		{
			put(i++, colors[0]);
			pos += entryStep;
		}

		// Between stops

		for (int s = 0; s < nStops - 1 && i < spans; s++)
		{
			int nextStopPos = 2 * spans * layer.stopPos16[s + 1];

			if (pos < nextStopPos)
			{
				// Per channel: value = c0 + floor((2 * delta * (pos - stopPos) + span) / (2 * span)).
				// Numerators stay below 2^31 since pos - stopPos is at most one entryStep here.

				int span = nextStopPos - stopPos;
				int den = 2 * span;

				int quot[4], rem[4], quotStep[4], remStep[4];

				for (int c = 0; c < 4; c++)
				{
					int delta = colors[s + 1][c] - colors[s][c];

					int num = 2 * delta * (pos - stopPos) + span;
					quot[c] = _floorDiv(num, den);
					rem[c] = num - quot[c] * den;

					int numStep = 2 * delta * entryStep;
					quotStep[c] = _floorDiv(numStep, den);
					remStep[c] = numStep - quotStep[c] * den;
				}

				while (i < spans && pos < nextStopPos)
				{
					int color[4];
					for (int c = 0; c < 4; c++)
					{
						color[c] = colors[s][c] + quot[c];

						quot[c] += quotStep[c];
						rem[c] += remStep[c];
						if (rem[c] >= den)
						{
							rem[c] -= den;
							quot[c]++;
						}
					}

					put(i++, color);
					pos += entryStep;
				}
			}

			stopPos = nextStopPos;
		}

		// After last stop, including the last entry.

		while (i < entries)
			put(i++, colors[nStops - 1]);
	}

	//____ layerColorAt() _____________________________________________________

	HiColor layerColorAt(const TintLayer& layer, float position, HiColor multiplier)
	{
		HiColor color;
		_buildLUT(layer.nStops, layer.stopPos, layer.stopColors, TintSpread::Pad, 1, &color, position, position, false, multiplier);
		return color;
	}

	//____ layerColorAtFixed() ________________________________________________

	HiColor layerColorAtFixed(const TintLayer& layer, int position, HiColor multiplier)
	{
		// Same choice of stops as _buildLUT(): right limit at hard edges, except for a hard edge
		// at the first stop, which is left of it.

		int nStops = layer.nStops;
		if (nStops <= 0)
			return multiplier;

		const uint16_t* pPos = layer.stopPos16;
		int color[4];

		if (nStops == 1 || (position <= pPos[0] && !(position == pPos[0] && pPos[1] == pPos[0])))
			_clampedStopColor(layer.stopColors[0], color);
		else if (position >= pPos[nStops - 1])
			_clampedStopColor(layer.stopColors[nStops - 1], color);
		else
		{
			int s = nStops - 1;
			while (pPos[s] > position)
				s--;

			int from[4], to[4];
			_clampedStopColor(layer.stopColors[s], from);
			_clampedStopColor(layer.stopColors[s + 1], to);

			int span = pPos[s + 1] - pPos[s];
			int ofs = position - pPos[s];

			for (int c = 0; c < 4; c++)
				color[c] = from[c] + _floorDiv(2 * (to[c] - from[c]) * ofs + span, 2 * span);
		}

		HiColor out(color[0], color[1], color[2], color[3]);
		return multiplier == HiColor::White ? out : out * multiplier;
	}

	//____ lutBitsForLayer() __________________________________________________

	int lutBitsForLayer(const TintLayer& layer)
	{
		if (isLayerFlat(layer))
			return -1;

		CanvasGeometry geo = layerGeometry(layer);

		// Two entries per pixel of gradient length keeps quantization well below a pixel. Smallest
		// number of bits where (1 << bits) >= 2 * length, compared without square root or division
		// since software renderers call this on CPUs without FPU.

		int bits = 1;

		if (geo.shape == TintShape::Linear)
		{
			// length = 1 / sqrt(a^2 + b^2), or 1 if a and b are 0.

			float grad2 = geo.a * geo.a + geo.b * geo.b;
			if (grad2 > 0.f)
			{
				while (bits < 11 && float(1 << (2 * bits)) * grad2 < 4.f)
					bits++;
			}
		}
		else
		{
			// length = 1 / invRadius for the shortest radius, which gives steepest gradient.

			float invRadius = std::max(geo.invRadiusX, geo.invRadiusY);
			if (invRadius > 0.f)
			{
				while (bits < 11 && float(1 << bits) * invRadius < 2.f)
					bits++;
			}
		}

		return bits;
	}

	//____ gpuTintBlockSize() _________________________________________________

	int gpuTintBlockSize(const DecodedTint& tint)
	{
		int size = 1;

		for (int l = 0; l < tint.nLayers; l++)
		{
			int nStops = tint.layers[l].nStops;
			size += 3 + (nStops + 3) / 4 + nStops;
		}
		return size;
	}

	//____ gpuTintBlockMaxSize() ______________________________________________

	int gpuTintBlockMaxSize(int nLayers, int nStops)
	{
		// Each layer adds 3 entries plus positions (at most one entry per started group of four stops).

		return 1 + nLayers * 4 + nStops + (nStops + 3) / 4;
	}

	//____ writeGpuTintBlock() ________________________________________________

	void writeGpuTintBlock(const DecodedTint& tint, float* pOutput, HiColor multiplier)
	{
		float* p = pOutput;

		p[0] = float(tint.nLayers); p[1] = 0.f; p[2] = 0.f; p[3] = 0.f;
		p += 4;

		for (int l = 0; l < tint.nLayers; l++)
		{
			const TintLayer& layer = tint.layers[l];
			int nStops = layer.nStops;

			p[0] = float(int(layer.shape));
			p[1] = float(int(layer.spread));
			p[2] = float(nStops);
			p[3] = layer.weight / 4096.f;
			p += 4;

			for (int i = 0; i < 4; i++)
				p[i] = layer.geo[i];
			p += 4;

			p[0] = 1.f; p[1] = 0.f; p[2] = 0.f; p[3] = 0.f;		// Stop colors are sRGB.
			p += 4;

			int posEntries = (nStops + 3) / 4;
			for (int i = 0; i < posEntries * 4; i++)
				p[i] = i < nStops ? layer.stopPos[i] : 2.f;
			p += posEntries * 4;

			for (int i = 0; i < nStops; i++)
			{
				HiColor c = layer.stopColors[i] * multiplier;

				p[0] = c.r / 4096.f;
				p[1] = c.g / 4096.f;
				p[2] = c.b / 4096.f;
				p[3] = c.a / 4096.f;
				p += 4;
			}
		}
	}

	//____ encodeTint() _______________________________________________________

	int encodeTint(const Tint* pTint, const RectSPX& rect, uint16_t* pWords, HiColor* pColors, int& nColors)
	{
		uint16_t* p = pWords;
		nColors = 0;

		if (!pTint)
		{
			*p++ = 0;
			*p++ = 0;
			return 2;
		}

		const Tint*	layers[Tint::c_maxMixComponents];
		float		weights[Tint::c_maxMixComponents];
		int			nLayers;

		if (pTint->isMix())
		{
			nLayers = pTint->nbMixComponents();
			for (int i = 0; i < nLayers; i++)
			{
				layers[i] = pTint->mixComponent(i);
				weights[i] = pTint->mixWeight(i);
			}
		}
		else
		{
			nLayers = 1;
			layers[0] = pTint;
			weights[0] = 1.f;
		}

		*p++ = uint16_t(nLayers);
		*p++ = 0;

		int weightLeft = 4096;

		for (int l = 0; l < nLayers; l++)
		{
			const Tint* pLayer = layers[l];

			int weight = (l == nLayers - 1) ? weightLeft : std::min(weightLeft, (int) std::lround(weights[l] * 4096));
			weightLeft -= weight;

			int nStops = pLayer->nbStops();

			*p++ = uint16_t(weight);
			*p++ = uint16_t(int(pLayer->shape()) | (int(pLayer->spread()) << 4));
			*p++ = uint16_t(nStops);
			*p++ = 0;

			CanvasGeometry g = canvasGeometry(pLayer, rect);

			float geo[4];
			if (g.shape == TintShape::Linear)
			{
				geo[0] = g.a;
				geo[1] = g.b;
				geo[2] = g.c;
				geo[3] = 0.f;
			}
			else
			{
				geo[0] = g.centerX;
				geo[1] = g.centerY;
				geo[2] = g.invRadiusX;
				geo[3] = g.invRadiusY;
			}

			std::memcpy(p, geo, sizeof(geo));
			p += 8;

			const ColorStop* pStops = pLayer->stops();
			for (int i = 0; i < nStops; i++)
			{
				*p++ = uint16_t(std::lround(std::clamp(pStops[i].pos, 0.f, 1.f) * 65535.f));
				pColors[nColors++] = pStops[i].color;
			}

			if (nStops & 1)
				*p++ = 0;
		}

		return int(p - pWords);
	}

	//____ decodeTint() _______________________________________________________

	const uint16_t* decodeTint(const uint16_t* pWords, const HiColor*& pColors, DecodedTint& output)
	{
		const uint16_t* p = pWords;

		int nLayers = *p++;
		p++;

		if (nLayers > Tint::c_maxMixComponents)
			nLayers = Tint::c_maxMixComponents;			// Corrupt data, shouldn't happen.

		output.nLayers = nLayers;

		for (int l = 0; l < nLayers; l++)
		{
			TintLayer& layer = output.layers[l];

			layer.weight = *p++;

			uint16_t packed = *p++;
			layer.shape = TintShape(packed & 0xF);
			layer.spread = TintSpread((packed >> 4) & 0xF);

			int nStops = *p++;
			p++;

			std::memcpy(layer.geo, p, sizeof(layer.geo));
			p += 8;

			int nStored = std::min(nStops, Tint::c_maxStops);
			layer.nStops = nStored;

			for (int i = 0; i < nStops; i++)
			{
				uint16_t pos = *p++;
				HiColor color = *pColors++;

				if (i < nStored)
				{
					layer.stopPos[i] = pos / 65535.f;
					layer.stopPos16[i] = pos;
					layer.stopColors[i] = color;
				}
			}

			if (nStops & 1)
				p++;
		}

		return p;
	}

	//____ encodedTintSize() __________________________________________________

	int encodedTintSize(const uint16_t* pWords, int& nColors)
	{
		const uint16_t* p = pWords;

		int nLayers = *p++;
		p++;

		nColors = 0;

		for (int l = 0; l < nLayers; l++)
		{
			p += 2;
			int nStops = *p++;
			p++;
			p += 8;
			p += nStops + (nStops & 1);
			nColors += nStops;
		}

		return int(p - pWords);
	}

	//____ serializeTint() ____________________________________________________

	int serializeTint(const Tint* pTint, uint8_t* pDest)
	{
		uint8_t* p = pDest;

		auto putF = [&](float f) { std::memcpy(p, &f, 4); p += 4; };
		auto putU8 = [&](int v) { *p++ = uint8_t(v); };

		if (!pTint)
		{
			putU8(0); putU8(0); putU8(0); putU8(0);
			return 4;
		}

		const Tint*	components[Tint::c_maxMixComponents];
		float		weights[Tint::c_maxMixComponents];
		int			nComponents;

		if (pTint->isMix())
		{
			nComponents = pTint->nbMixComponents();
			for (int i = 0; i < nComponents; i++)
			{
				components[i] = pTint->mixComponent(i);
				weights[i] = pTint->mixWeight(i);
			}
		}
		else
		{
			nComponents = 1;
			components[0] = pTint;
			weights[0] = 1.f;
		}

		putU8(nComponents); putU8(0); putU8(0); putU8(0);

		for (int c = 0; c < nComponents; c++)
		{
			const Tint* t = components[c];

			putF(weights[c]);
			putU8(int(t->shape())); putU8(int(t->spread())); putU8(0); putU8(int(t->radiusMode()));
			putF(t->begin().x); putF(t->begin().y);
			putF(t->end().x); putF(t->end().y);
			putF(t->center().x); putF(t->center().y);
			putF(t->radius().w); putF(t->radius().h);
			putU8(t->nbStops()); putU8(0); putU8(0); putU8(0);

			for (int i = 0; i < t->nbStops(); i++)
			{
				const ColorStop& stop = t->stops()[i];
				putF(stop.pos);
				std::memcpy(p, &stop.color, 8);
				p += 8;
			}
		}

		return int(p - pDest);
	}

	//____ deserializeTint() __________________________________________________

	Tint_p deserializeTint(const uint8_t* pSource, int& bytesRead)
	{
		const uint8_t* p = pSource;

		auto getF = [&]() { float f; std::memcpy(&f, p, 4); p += 4; return f; };
		auto getU8 = [&]() { return int(*p++); };

		int nComponents = getU8();
		p += 3;

		Tint_p	components[Tint::c_maxMixComponents];
		float	weights[Tint::c_maxMixComponents];

		bool bOk = (nComponents <= Tint::c_maxMixComponents);

		for (int c = 0; c < nComponents; c++)
		{
			Tint::Blueprint bp;

			float weight = getF();
			if (c < Tint::c_maxMixComponents)
				weights[c] = weight;

			bp.shape = TintShape(getU8());
			bp.spread = TintSpread(getU8());
			getU8();												// Reserved, used to be color space.
			bp.radiusMode = TintRadius(getU8());
			bp.begin.x = getF(); bp.begin.y = getF();
			bp.end.x = getF(); bp.end.y = getF();
			bp.center.x = getF(); bp.center.y = getF();
			bp.radius.w = getF(); bp.radius.h = getF();

			int nStops = getU8();
			p += 3;

			for (int i = 0; i < nStops; i++)
			{
				ColorStop stop;
				stop.pos = getF();
				std::memcpy(&stop.color, p, 8);
				p += 8;
				bp.stops.push_back(stop);
			}

			if (bOk && c < Tint::c_maxMixComponents)
			{
				components[c] = Tint::create(bp);
				if (!components[c])
					bOk = false;
			}
		}

		bytesRead = int(p - pSource);

		if (!bOk || nComponents == 0)
			return nullptr;

		if (nComponents == 1)
			return components[0];

		Tint* raw[Tint::c_maxMixComponents];
		for (int c = 0; c < nComponents; c++)
			raw[c] = components[c];

		return Tint::createMix(nComponents, raw, weights);
	}

	//____ layerGeometry() ____________________________________________________

	CanvasGeometry layerGeometry(const TintLayer& layer)
	{
		CanvasGeometry g = {};
		g.shape = layer.shape;

		if (layer.shape == TintShape::Linear)
		{
			g.a = layer.geo[0];
			g.b = layer.geo[1];
			g.c = layer.geo[2];
		}
		else
		{
			g.centerX = layer.geo[0];
			g.centerY = layer.geo[1];
			g.invRadiusX = layer.geo[2];
			g.invRadiusY = layer.geo[3];
		}
		return g;
	}

	//____ isLayerFlat() ______________________________________________________

	bool isLayerFlat(const TintLayer& layer)
	{
		for (int i = 1; i < layer.nStops; i++)
			if (layer.stopColors[i] != layer.stopColors[0])
				return false;

		return true;
	}

}
}
