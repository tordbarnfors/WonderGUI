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
#include <cstring>

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

	//____ _buildLUT() ________________________________________________________

	static void _buildLUT(int nStops, const float* pStopPos, const HiColor* pStopColors, ColorSpace colorSpace, TintSpread spread,
						  int entries, HiColor* pOutput, float begin, float end, bool bSquared, HiColor multiplier)
	{
		if (entries <= 0)
			return;

		bool bMultiply = (multiplier != HiColor::White);

		// Convert stop colors to the space we interpolate in.

		bool bSRGB = (colorSpace == ColorSpace::sRGB);

		float	stopColors[Tint::c_maxStops][4];

		for (int i = 0; i < nStops; i++)
		{
			const HiColor& c = pStopColors[i];

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

		_buildLUT(nStops, pos, colors, pTint->colorSpace(), pTint->spread(), entries, pOutput, begin, end, bSquared, HiColor::White);
	}

	void buildLUT(const TintLayer& layer, int entries, HiColor* pOutput, HiColor multiplier)
	{
		// Entry i is sampled at (i + 0.5) / (entries - 1), so that a renderer that looks up floor(position * (entries - 1))
		// gets the color of the middle of the span the entry covers and hard edges end up exactly on entry boundaries.

		float halfEntry = entries > 1 ? 0.5f / (entries - 1) : 0.f;
		_buildLUT(layer.nStops, layer.stopPos, layer.stopColors, layer.colorSpace, TintSpread::Pad, entries, pOutput, halfEntry, 1.f + halfEntry, false, multiplier);
	}

	//____ layerColorAt() _____________________________________________________

	HiColor layerColorAt(const TintLayer& layer, float position, HiColor multiplier)
	{
		HiColor color;
		_buildLUT(layer.nStops, layer.stopPos, layer.stopColors, layer.colorSpace, TintSpread::Pad, 1, &color, position, position, false, multiplier);
		return color;
	}

	//____ lutBitsForLayer() __________________________________________________

	int lutBitsForLayer(const TintLayer& layer)
	{
		if (isLayerFlat(layer))
			return -1;

		CanvasGeometry geo = layerGeometry(layer);
		float length;

		if (geo.shape == TintShape::Linear)
		{
			float grad = std::sqrt(geo.a * geo.a + geo.b * geo.b);
			length = grad > 0.f ? 1.f / grad : 1.f;
		}
		else
		{
			float invRadius = std::max(geo.invRadiusX, geo.invRadiusY);		// Shortest radius gives steepest gradient.
			length = invRadius > 0.f ? 1.f / invRadius : 1.f;
		}

		// Two entries per pixel of gradient length keeps quantization well below a pixel.

		int bits = 1;
		while (bits < 11 && (1 << bits) < length * 2)
			bits++;

		return bits;
	}

	//____ gpuTintBlockSize() _________________________________________________

	int gpuTintBlockSize(const DecodedTint& tint)
	{
		int size = 1;

		for (int l = 0; l < tint.nLayers; l++)
		{
			int bits = lutBitsForLayer(tint.layers[l]);
			size += 3 + (bits < 0 ? 0 : (1 << bits) + 1);
		}
		return size;
	}

	//____ writeGpuTintBlock() ________________________________________________

	void writeGpuTintBlock(const DecodedTint& tint, float* pOutput, HiColor multiplier)
	{
		float* p = pOutput;

		auto putColor = [&](HiColor c)
		{
			p[0] = c.r / 4096.f;
			p[1] = c.g / 4096.f;
			p[2] = c.b / 4096.f;
			p[3] = c.a / 4096.f;
			p += 4;
		};

		p[0] = float(tint.nLayers); p[1] = 0.f; p[2] = 0.f; p[3] = 0.f;
		p += 4;

		const int c_maxEntries = (1 << 11) + 1;
		HiColor lut[c_maxEntries];

		for (int l = 0; l < tint.nLayers; l++)
		{
			const TintLayer& layer = tint.layers[l];
			int bits = lutBitsForLayer(layer);
			int N = bits < 0 ? 0 : (1 << bits);

			p[0] = float(int(layer.shape));
			p[1] = float(int(layer.spread));
			p[2] = float(N);
			p[3] = layer.weight / 4096.f;
			p += 4;

			for (int i = 0; i < 4; i++)
				p[i] = layer.geo[i];
			p += 4;

			if (N == 0)
			{
				HiColor col = layer.nStops > 0 ? layer.stopColors[0] * multiplier : multiplier;
				putColor(col);
			}
			else
			{
				putColor(layerColorAt(layer, 0.f, multiplier));

				buildLUT(layer, N + 1, lut, multiplier);
				for (int i = 0; i <= N; i++)
					putColor(lut[i]);
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
			*p++ = uint16_t(int(pLayer->shape()) | (int(pLayer->spread()) << 4) | (int(pLayer->colorSpace()) << 8));
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
			layer.colorSpace = ColorSpace((packed >> 8) & 0xF);

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
			putU8(int(t->shape())); putU8(int(t->spread())); putU8(int(t->colorSpace())); putU8(int(t->radiusMode()));
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
			bp.colorSpace = ColorSpace(getU8());
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
