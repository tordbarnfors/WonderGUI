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

#include <wg_softtint.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace wg
{
	namespace
	{
		//____ Fixed point ____________________________________________________
		/*
			Geometry is converted to fixed point once per layer in set(), so that generating colors
			needs no floating point.

			Positions are in LUT entries with 32 binals. Values are limited to +-2^61 and gradients
			to c_maxSteepness LUT entries per pixel, which keeps sums like stepX * x + stepY * y + originX
			within 64 bits for coordinates up to +-2^20. Only gradients shorter than 1/128 pixel are
			affected, they are widened to that.
		*/

		const double	c_maxSteepness = 256.0;

		int64_t toFixed(double value)
		{
			const double c_limit = 2305843009213693952.0;		// 2^61

			if (value >= c_limit)
				return int64_t(1) << 61;
			if (value <= -c_limit)
				return -(int64_t(1) << 61);
			if (value != value)
				return 0;										// NaN
			return std::llround(value);
		}

		//____ Integer square roots ___________________________________________
		// floor(sqrt(n)), digit by digit. Without branches in isqrt32(), which runs once per span.

		uint32_t isqrt32(uint32_t n)
		{
			uint32_t root = 0;

			for (uint32_t bit = uint32_t(1) << 30; bit != 0; bit >>= 2)
			{
				uint32_t trial = root + bit;
				uint32_t mask = uint32_t(0) - uint32_t(n >= trial);

				n -= trial & mask;
				root = (root >> 1) + (bit & mask);
			}
			return root;
		}

		uint32_t isqrt64(uint64_t n)
		{
			uint64_t root = 0;
			uint64_t bit = uint64_t(1) << 62;

			while (bit > n)
				bit >>= 2;

			while (bit != 0)
			{
				if (n >= root + bit)
				{
					n -= root + bit;
					root = (root >> 1) + bit;
				}
				else
					root >>= 1;
				bit >>= 2;
			}
			return uint32_t(root);
		}

		//____ Lookups ________________________________________________________
		/*
			Colors for LUT indices with spread applied. The LUT has N+1 entries, N = 1 << bits,
			entry i covering positions i/N -> (i+1)/N.
		*/

		struct PadLookup
		{
			const HiColor*	pLUT;
			int				N;
			HiColor			low;			// Below position 0.0.

			inline HiColor operator()(int idx) const { return idx < 0 ? low : pLUT[idx > N ? N : idx]; }
		};

		struct RepeatLookup
		{
			const HiColor*	pLUT;
			int				mask;			// N - 1

			inline HiColor operator()(int idx) const { return pLUT[idx & mask]; }
		};

		struct ReflectLookup
		{
			const HiColor*	pLUT;
			int				bits;
			int				mask;			// 2N - 1

			// Span r >= N mirrors span 2N-1-r, which is r with all bits below 2N flipped.

			inline HiColor operator()(int idx) const { int r = idx & mask; return pLUT[r ^ (-(r >> bits) & mask)]; }
		};

		//____ radialSpan() ___________________________________________________
		/*
			Colors for a span of a radial layer, where u and v (LUT entries with 32 binals) are
			stepped with du and dv per pixel and the LUT index is floor(length(u, v)).

			Instead of a square root per pixel, the squared length D is stepped with additions
			(forward differences) and the index is kept from pixel to pixel by comparing D
			against squares of it. That is done for every other index, h = k/2, which moves at
			most a couple of steps per pixel since LUTs have two entries per pixel of radius.
			D is kept as R = D - (2h)^2 and compared against W = (2h+2)^2 - (2h)^2 = 8h + 4 to
			step h, and against M = (2h+1)^2 - (2h)^2 = 4h + 1 for the last bit of k.

			D grows or shrinks from one pixel to the next as E, its step, is positive or
			negative, and since E grows along the span only one direction needs to be checked
			per pixel.

			This is done with c_radialBinals binals, anchored to exact positions every
			c_radialChunk pixels so that rounding of the steps adds up to at most 1/2048 entry.

			The first index of a span takes a square root, unless it is close to where the last
			span started or ended (hints), which is common for rows and edgemap columns.

			Spans too far from the center for that (2^15 entries), or that step more than
			c_radialMaxFastStep entries per pixel (radius below 1/8 pixel), take a slow path
			with an integer square root per pixel.
		*/

		const int		c_radialBinals = 16;
		const int		c_radialChunk = 64;
		const int64_t	c_radialFastLimit = (int64_t(1) << 47) - (int64_t(1) << 16);	// |u| and |v| below this keeps them below 2^31 in c_radialBinals.
		const int64_t	c_radialMaxFastStep = int64_t(16) << 32;

		template<bool PAD, class Lookup>
		void radialSpan(const Lookup& lookup, int N, int64_t u, int64_t v, int64_t du, int64_t dv, int length, HiColor* pOutput, int* pHints)
		{
			auto absMax = [](int64_t a, int64_t b) { return std::max(a < 0 ? -a : a, b < 0 ? -b : b); };

			int64_t uvMax = std::max(absMax(u, u + du * (length - 1)), absMax(v, v + dv * (length - 1)));

			if (uvMax >= c_radialFastLimit || absMax(du, dv) > c_radialMaxFastStep)
			{
				// Slow path. u and v scaled down to 31 bits, so the sum of squares fits in 64.

				int shift = 0;
				while ((uvMax >> shift) >= (int64_t(1) << 31))
					shift++;

				for (int i = 0; i < length; i++)
				{
					uint64_t au = uint64_t(u < 0 ? -u : u) >> shift;
					uint64_t av = uint64_t(v < 0 ? -v : v) >> shift;
					uint64_t k = uint64_t(isqrt64(au * au + av * av)) >> (32 - shift);

					pOutput[i] = lookup(int(std::min(k, uint64_t(1) << 30)));
					u += du;
					v += dv;
				}
				return;
			}

			// Values below are kept within 32 bits where they can, so that 32-bit CPUs get 32 x 32 -> 64 bit
			// multiplications at most. k is below 2^15.5, so (2h)^2 and (2h + 10)^2 fit in 32 bits unsigned.

			const int		shift = 32 - c_radialBinals;
			const int		squareShift = 2 * c_radialBinals;
			const int64_t	half = int64_t(1) << (shift - 1);
			const int64_t	one = int64_t(1) << squareShift;					// Square of one LUT entry.

			const int32_t	dU = int32_t((du + half) >> shift);
			const int32_t	dV = int32_t((dv + half) >> shift);
			const int64_t	stepSquared = int64_t(dU) * dU + int64_t(dV) * dV;

			const int		halfN = N / 2;
			int				h = -1;

			while (length > 0)
			{
				int n = std::min(length, c_radialChunk);

				int32_t U = int32_t((u + half) >> shift);
				int32_t V = int32_t((v + half) >> shift);

				uint64_t D = uint64_t(int64_t(U) * U) + uint64_t(int64_t(V) * V);		// Below 2^63.
				int64_t E = 2 * (int64_t(U) * dU + int64_t(V) * dV) + stepSquared;		// D of next pixel - D.

				bool bFirstChunk = (h < 0);

				if (bFirstChunk)
				{
					// k^2 <= D / one < (k+1)^2, so a hint g is used if 2g - 8 <= k < 2g + 10.

					uint32_t entriesSquared = uint32_t(D >> squareShift);

					for (int i = 0; i < 2 && h < 0; i++)
					{
						uint32_t g = uint32_t(pHints[i]);
						if (pHints[i] >= 0 && entriesSquared < (2 * g + 10) * (2 * g + 10) && (g < 4 || entriesSquared >= (2 * g - 8) * (2 * g - 8)))
							h = int(g);
					}

					if (h < 0)
						h = int(isqrt32(entriesSquared)) / 2;
				}

				int64_t R = int64_t(D - (uint64_t(uint32_t(2 * h) * uint32_t(2 * h)) << squareShift));
				int64_t W = int64_t(8 * h + 4) << squareShift;
				int64_t M = int64_t(4 * h + 1) << squareShift;

				// h from hint or previous chunk can be off a few steps.

				while (R >= W) { R -= W; W += 8 * one; M += 4 * one; h++; }
				while (R < 0) { W -= 8 * one; M -= 4 * one; R += W; h--; }

				if (bFirstChunk)
					pHints[0] = h;

				for (int i = 0; i < n; i++)
				{
					if (E >= 0)
					{
						if (PAD && h >= halfN)
						{
							// Outside and moving away (D only grows from here), so the rest has the last color.

							HiColor last = lookup(N);
							for (int j = i; j < length; j++)
								pOutput[j] = last;

							pHints[1] = -1;
							return;
						}

						pOutput[i] = lookup(2 * h + int(R >= M));
						R += E;
						E += 2 * stepSquared;
						while (R >= W) { R -= W; W += 8 * one; M += 4 * one; h++; }
					}
					else
					{
						pOutput[i] = lookup(2 * h + int(R >= M));
						R += E;
						E += 2 * stepSquared;
						while (R < 0) { W -= 8 * one; M -= 4 * one; R += W; h--; }
					}
				}

				pOutput += n;
				length -= n;
				u += du * c_radialChunk;		// Only used if this chunk was full.
				v += dv * c_radialChunk;
			}

			pHints[1] = h;
		}
	}

	//____ set() ______________________________________________________________

	void SoftTint::set(const Tint* pTint, const RectSPX& rect, HiColor multiplier)
	{
		uint16_t	words[TintTools::c_maxEncodedTintWords];
		HiColor		colors[TintTools::c_maxEncodedTintColors];
		int			nColors;

		TintTools::encodeTint(pTint, rect, words, colors, nColors);

		const HiColor* pColors = colors;
		TintTools::DecodedTint decoded;
		TintTools::decodeTint(words, pColors, decoded);

		set(decoded, multiplier);
	}

	void SoftTint::set(const TintTools::DecodedTint& tint, HiColor multiplier)
	{
		m_nLayers = 0;

		bool	bOpaque = (multiplier.a == 4096);
		bool	bTransparent = true;
		bool	bAllFlat = true;
		bool	bVariesX = false;
		bool	bVariesY = false;

		int		lutEntries = 0;

		for (int l = 0; l < tint.nLayers; l++)
		{
			const TintTools::TintLayer& src = tint.layers[l];

			if (src.weight == 0 || src.nStops == 0)
				continue;

			Layer& layer = m_layers[m_nLayers++];

			layer.shape = src.shape;
			layer.spread = src.spread;
			layer.weight = src.weight;
			layer.bFlat = TintTools::isLayerFlat(src);
			layer.lutBits = 0;
			layer.lutOfs = 0;
			layer.stepX = 0;
			layer.stepY = 0;
			layer.originX = 0;
			layer.originY = 0;
			layer.stepX16 = 0;
			layer.stepY16 = 0;
			layer.radialHints[0] = -1;
			layer.radialHints[1] = -1;

			for (int i = 0; i < src.nStops; i++)
			{
				if (src.stopColors[i].a != 4096)
					bOpaque = false;
				if (src.stopColors[i].a != 0)
					bTransparent = false;
			}

			if (layer.bFlat)
			{
				layer.flatColor = src.stopColors[0].toLinear() * multiplier;
				continue;
			}

			bAllFlat = false;

			// Decide LUT size from length of gradient in pixels and convert geometry to fixed point.
			// The only floating point in SoftTint, a few operations per layer.

			int bits = TintTools::lutBitsForLayer(src);

			TintTools::CanvasGeometry geo = TintTools::layerGeometry(src);
			double scale = double(int64_t(1) << (bits + 32));

			if (geo.shape == TintShape::Linear)
			{
				double a = geo.a, b = geo.b, c = geo.c;

				// Too steep gradients are widened around their middle.

				double steepness = std::max(std::abs(a), std::abs(b)) * (1 << bits);
				if (steepness > c_maxSteepness)
				{
					double f = c_maxSteepness / steepness;
					a *= f;
					b *= f;
					c = c * f + 0.5 * (1.0 - f);
				}

				layer.stepX = toFixed(a * scale);
				layer.stepY = toFixed(b * scale);
				layer.originX = toFixed(c * scale) + (layer.stepX + layer.stepY) / 2;		// Pixel centers.
				layer.stepX16 = toFixed(a * scale / 65536.0);
				layer.stepY16 = toFixed(b * scale / 65536.0);

				if (layer.stepX != 0)
					bVariesX = true;
				if (layer.stepY != 0)
					bVariesY = true;
			}
			else
			{
				double irx = geo.invRadiusX, iry = geo.invRadiusY;

				double steepness = std::max(std::abs(irx), std::abs(iry)) * (1 << bits);
				if (steepness > c_maxSteepness)
				{
					double f = c_maxSteepness / steepness;
					irx *= f;
					iry *= f;
				}

				layer.stepX = toFixed(irx * scale);
				layer.stepY = toFixed(iry * scale);
				layer.originX = toFixed((0.5 - geo.centerX) * irx * scale);
				layer.originY = toFixed((0.5 - geo.centerY) * iry * scale);
				layer.stepX16 = toFixed(irx * scale / 65536.0);
				layer.stepY16 = toFixed(iry * scale / 65536.0);

				bVariesX = true;
				bVariesY = true;
			}

			layer.lutBits = bits;
			layer.lutOfs = lutEntries;
			lutEntries += (1 << bits) + 1;
		}

		// Build lookup tables

		if (lutEntries > 0)
		{
			if ((int)m_lut.size() < lutEntries)
				m_lut.resize(lutEntries);

			int layerIdx = 0;
			for (int l = 0; l < tint.nLayers; l++)
			{
				const TintTools::TintLayer& src = tint.layers[l];
				if (src.weight == 0 || src.nStops == 0)
					continue;

				Layer& layer = m_layers[layerIdx++];
				if (!layer.bFlat)
				{
					// LUT is built in sRGB, then converted to linear and multiplied.

					int entries = (1 << layer.lutBits) + 1;
					HiColor* pLUT = m_lut.data() + layer.lutOfs;

					TintTools::buildLUT(src, entries, pLUT);

					if (multiplier == HiColor::White)
					{
						for (int i = 0; i < entries; i++)
							pLUT[i] = pLUT[i].toLinear();
					}
					else
					{
						for (int i = 0; i < entries; i++)
							pLUT[i] = pLUT[i].toLinear() * multiplier;
					}

					layer.padLowColor = TintTools::layerColorAtFixed(src, 0).toLinear() * multiplier;
				}
			}
		}

		if (multiplier.a == 0)
			bTransparent = true;

		m_bOpaque = bOpaque;
		m_bTransparent = bTransparent && m_nLayers > 0;
		m_bVariesX = bVariesX;
		m_bVariesY = bVariesY;

		// Flat tint (all layers flat or no layers at all)

		if (m_nLayers == 0)
		{
			m_bFlat = true;
			m_flatColor = multiplier;
			m_bTransparent = (multiplier.a == 0);
		}
		else if (bAllFlat)
		{
			int r = 0, g = 0, b = 0, a = 0;
			for (int l = 0; l < m_nLayers; l++)
			{
				const HiColor& c = m_layers[l].flatColor;
				int w = m_layers[l].weight;
				r += c.r * w;
				g += c.g * w;
				b += c.b * w;
				a += c.a * w;
			}

			m_bFlat = true;
			m_flatColor = HiColor(r >> 12, g >> 12, b >> 12, a >> 12);
		}
		else
			m_bFlat = false;
	}

	//____ setFlat() __________________________________________________________

	void SoftTint::setFlat(HiColor color)
	{
		m_nLayers = 0;
		m_bFlat = true;
		m_flatColor = color;
		m_bOpaque = (color.a == 4096);
		m_bTransparent = (color.a == 0);
		m_bVariesX = false;
		m_bVariesY = false;
	}

	//____ generate() _________________________________________________________

	void SoftTint::generate(int x, int y, int dx, int dy, int length, HiColor* pOutput) const
	{
		_mixLayers(length, pOutput, [&](const Layer& layer, HiColor* pLayerOutput)
		{
			if (layer.shape == TintShape::Linear)
				_generateLinear(layer, x, y, dx, dy, length, pLayerOutput);
			else
				_generateRadial(layer, x, y, dx, dy, length, pLayerOutput);
		});
	}

	//____ generateSpan() _____________________________________________________

	void SoftTint::generateSpan(int64_t x, int64_t y, int64_t dx, int64_t dy, int length, HiColor* pOutput) const
	{
		_mixLayers(length, pOutput, [&](const Layer& layer, HiColor* pLayerOutput)
		{
			// Positions are relative to pixel centers in the layers, which are at x + 0.5.

			int64_t cx = x - 32768;
			int64_t cy = y - 32768;

			if (layer.shape == TintShape::Linear)
			{
				int64_t pos = (layer.stepX16 * cx + layer.stepY16 * cy + layer.originX) >> 16;
				int64_t step = (layer.stepX16 * dx + layer.stepY16 * dy) >> 16;

				_linearSpan(layer, pos, step, length, pLayerOutput);
			}
			else
			{
				int64_t u = layer.stepX16 * cx + layer.originX;
				int64_t v = layer.stepY16 * cy + layer.originY;

				_radialSpan(layer, u, v, layer.stepX16 * dx, layer.stepY16 * dy, length, pLayerOutput);
			}
		});
	}

	//____ _mixLayers() _______________________________________________________
	// Colors of all layers, generated by generateLayer(layer, pOutput), mixed by their weights.

	template<class LayerGenerator>
	void SoftTint::_mixLayers(int length, HiColor* pOutput, LayerGenerator&& generateLayer) const
	{
		if (m_bFlat)
		{
			for (int i = 0; i < length; i++)
				pOutput[i] = m_flatColor;
			return;
		}

		auto generate = [&](const Layer& layer, HiColor* pLayerOutput)
		{
			if (layer.bFlat)
			{
				for (int i = 0; i < length; i++)
					pLayerOutput[i] = layer.flatColor;
			}
			else
				generateLayer(layer, pLayerOutput);
		};

		if (m_nLayers == 1)
		{
			generate(m_layers[0], pOutput);
			return;
		}

		// Mix, accumulate weighted layers.

		if ((int)m_accumulator.size() < length * 4)
			m_accumulator.resize(length * 4);

		if ((int)m_layerBuffer.size() < length)
			m_layerBuffer.resize(length);

		int* pAcc = m_accumulator.data();
		std::memset(pAcc, 0, length * 4 * sizeof(int));

		HiColor* pLayerColors = m_layerBuffer.data();

		for (int l = 0; l < m_nLayers; l++)
		{
			const Layer& layer = m_layers[l];
			int w = layer.weight;

			generate(layer, pLayerColors);

			int* p = pAcc;
			for (int i = 0; i < length; i++)
			{
				const HiColor& c = pLayerColors[i];
				p[0] += c.r * w;
				p[1] += c.g * w;
				p[2] += c.b * w;
				p[3] += c.a * w;
				p += 4;
			}
		}

		int* p = pAcc;
		for (int i = 0; i < length; i++)
		{
			HiColor& c = pOutput[i];
			c.r = int16_t(p[0] >> 12);
			c.g = int16_t(p[1] >> 12);
			c.b = int16_t(p[2] >> 12);
			c.a = int16_t(p[3] >> 12);
			p += 4;
		}
	}

	//____ _generateLinear() __________________________________________________

	void SoftTint::_generateLinear(const Layer& layer, int x, int y, int dx, int dy, int length, HiColor* pOutput) const
	{
		// Position in LUT entries with 16 binals, exact at the first pixel, then stepped.

		int64_t pos = (layer.stepX * x + layer.stepY * y + layer.originX) >> 16;
		int64_t step = layer.stepX16 * dx + layer.stepY16 * dy;

		_linearSpan(layer, pos, step, length, pOutput);
	}

	//____ _linearSpan() ______________________________________________________
	// Colors of a linear layer for positions pos, pos + step ... in LUT entries with 16 binals.

	void SoftTint::_linearSpan(const Layer& layer, int64_t pos, int64_t step, int length, HiColor* pOutput) const
	{
		const HiColor* pLUT = m_lut.data() + layer.lutOfs;
		const int bits = layer.lutBits;

		switch (layer.spread)
		{
			default:
			case TintSpread::Pad:
			{
				PadLookup lookup = { pLUT, 1 << bits, layer.padLowColor };

				int64_t last = pos + step * (length - 1);

				if (std::min(pos, last) >= INT32_MIN && std::max(pos, last) <= INT32_MAX)
				{
					// Stepped in 32 bits, unsigned so that the step after the last pixel can't overflow.

					uint32_t p = uint32_t(pos);
					uint32_t s = uint32_t(step);

					for (int i = 0; i < length; i++)
					{
						pOutput[i] = lookup(int32_t(p) >> 16);
						p += s;
					}
				}
				else
				{
					// Far outside the gradient.

					for (int i = 0; i < length; i++)
					{
						int64_t idx = pos >> 16;
						pOutput[i] = lookup(idx < -1 ? -1 : idx > lookup.N ? lookup.N : int(idx));
						pos += step;
					}
				}
				break;
			}

			// Repeat and Reflect only need the bits of the position below 2N, so they are
			// stepped in 32 bits and allowed to wrap.

			case TintSpread::Repeat:
			{
				RepeatLookup lookup = { pLUT, (1 << bits) - 1 };

				uint32_t p = uint32_t(pos);
				uint32_t s = uint32_t(step);

				for (int i = 0; i < length; i++)
				{
					pOutput[i] = lookup(int(p >> 16));
					p += s;
				}
				break;
			}

			case TintSpread::Reflect:
			{
				ReflectLookup lookup = { pLUT, bits, (2 << bits) - 1 };

				uint32_t p = uint32_t(pos);
				uint32_t s = uint32_t(step);

				for (int i = 0; i < length; i++)
				{
					pOutput[i] = lookup(int(p >> 16));
					p += s;
				}
				break;
			}
		}
	}

	//____ _generateRadial() __________________________________________________

	void SoftTint::_generateRadial(const Layer& layer, int x, int y, int dx, int dy, int length, HiColor* pOutput) const
	{
		// Steps without 64-bit multiplication for dx and dy of -1, 0 and 1.

		auto stepFor = [](int64_t step, int d) { return d == 1 ? step : d == -1 ? -step : d == 0 ? 0 : step * d; };

		int64_t u = layer.stepX * x + layer.originX;
		int64_t v = layer.stepY * y + layer.originY;
		int64_t du = stepFor(layer.stepX, dx);
		int64_t dv = stepFor(layer.stepY, dy);

		_radialSpan(layer, u, v, du, dv, length, pOutput);
	}

	//____ _radialSpan() ______________________________________________________
	// Colors of a radial layer for u, v (LUT entries with 32 binals) stepped by du, dv.

	void SoftTint::_radialSpan(const Layer& layer, int64_t u, int64_t v, int64_t du, int64_t dv, int length, HiColor* pOutput) const
	{
		const HiColor* pLUT = m_lut.data() + layer.lutOfs;
		const int bits = layer.lutBits;
		const int N = 1 << bits;

		switch (layer.spread)
		{
			default:
			case TintSpread::Pad:
				radialSpan<true>(PadLookup{ pLUT, N, layer.padLowColor }, N, u, v, du, dv, length, pOutput, layer.radialHints);
				break;

			case TintSpread::Repeat:
				radialSpan<false>(RepeatLookup{ pLUT, N - 1 }, N, u, v, du, dv, length, pOutput, layer.radialHints);
				break;

			case TintSpread::Reflect:
				radialSpan<false>(ReflectLookup{ pLUT, bits, 2 * N - 1 }, N, u, v, du, dv, length, pOutput, layer.radialHints);
				break;
		}
	}

}
