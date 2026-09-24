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
#include <cstring>

namespace wg
{

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

			layer.geo = TintTools::layerGeometry(src);
			layer.spread = src.spread;
			layer.weight = src.weight;
			layer.bFlat = TintTools::isLayerFlat(src);
			layer.lutBits = 0;
			layer.lutOfs = 0;

			for (int i = 0; i < src.nStops; i++)
			{
				if (src.stopColors[i].a != 4096)
					bOpaque = false;
				if (src.stopColors[i].a != 0)
					bTransparent = false;
			}

			if (layer.bFlat)
			{
				layer.flatColor = src.stopColors[0] * multiplier;
				continue;
			}

			bAllFlat = false;

			// Classify and decide LUT size from length of gradient in pixels.

			if (layer.geo.shape == TintShape::Linear)
			{
				if (layer.geo.a != 0.f)
					bVariesX = true;
				if (layer.geo.b != 0.f)
					bVariesY = true;
			}
			else
			{
				bVariesX = true;
				bVariesY = true;
			}

			int bits = TintTools::lutBitsForLayer(src);

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
					TintTools::buildLUT(src, (1 << layer.lutBits) + 1, m_lut.data() + layer.lutOfs, multiplier);
					layer.padLowColor = TintTools::layerColorAt(src, 0.f, multiplier);
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
		if (m_bFlat)
		{
			for (int i = 0; i < length; i++)
				pOutput[i] = m_flatColor;
			return;
		}

		if (m_nLayers == 1)
		{
			_generateLayer(m_layers[0], x, y, dx, dy, length, pOutput);
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

			_generateLayer(layer, x, y, dx, dy, length, pLayerColors);

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

	//____ _generateLayer() ___________________________________________________

	void SoftTint::_generateLayer(const Layer& layer, int x, int y, int dx, int dy, int length, HiColor* pOutput) const
	{
		if (layer.bFlat)
		{
			for (int i = 0; i < length; i++)
				pOutput[i] = layer.flatColor;
			return;
		}

		const HiColor* pLUT = m_lut.data() + layer.lutOfs;
		const int64_t N = int64_t(1) << layer.lutBits;

		// Index into LUT with spread applied. LUT has N+1 entries, entry i covers positions i/N -> (i+1)/N.

		auto lookup = [&](int64_t idx) -> const HiColor&
		{
			switch (layer.spread)
			{
				default:
				case TintSpread::Pad:
					return idx < 0 ? layer.padLowColor : pLUT[idx > N ? N : idx];
				case TintSpread::Repeat:
					return pLUT[idx & (N - 1)];
				case TintSpread::Reflect:
				{
					int64_t r = idx & (2 * N - 1);
					return pLUT[r >= N ? 2 * N - 1 - r : r];		// Span r >= N mirrors span 2N-1-r.
				}
			}
		};

		const TintTools::CanvasGeometry& geo = layer.geo;

		if (geo.shape == TintShape::Linear)
		{
			// Step position in LUT index space, 16 binals. 64-bit to handle positions far outside the gradient.

			double scale = double(N) * 65536.0;
			double t = double(geo.a) * (x + 0.5) + double(geo.b) * (y + 0.5) + double(geo.c);

			int64_t idx = int64_t(std::floor(t * scale));
			int64_t step = int64_t(std::llround((double(geo.a) * dx + double(geo.b) * dy) * scale));

			for (int i = 0; i < length; i++)
			{
				pOutput[i] = lookup(idx >> 16);
				idx += step;
			}
		}
		else
		{
			float u = (x + 0.5f - geo.centerX) * geo.invRadiusX;
			float v = (y + 0.5f - geo.centerY) * geo.invRadiusY;
			float du = dx * geo.invRadiusX;
			float dv = dy * geo.invRadiusY;
			float n = float(N);

			for (int i = 0; i < length; i++)
			{
				float t = std::sqrt(u * u + v * v);
				if (t > 1e9f)
					t = 1e9f;

				pOutput[i] = lookup(int64_t(t * n));
				u += du;
				v += dv;
			}
		}
	}

}
