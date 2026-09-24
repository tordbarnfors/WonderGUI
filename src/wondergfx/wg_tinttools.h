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
#ifndef	WG_TINTTOOLS_DOT_H
#define	WG_TINTTOOLS_DOT_H
#pragma once

#include <wg_tint.h>

#include <cmath>

namespace wg
{
	/**
	 * @brief Helpers shared by everything that renders Tints.
	 *
	 * All backends use these so that stop interpolation, spread and geometry
	 * behave identically everywhere.
	 */

	namespace TintTools
	{
		//____ CanvasGeometry _________________________________________________
		/**
		 * Geometry of a simple Tint converted to canvas pixel space, for the
		 * position of pixel centers, e.g. (x + 0.5, y + 0.5) for pixel (x,y).
		 *
		 * Linear:	position = a * X + b * Y + c
		 * Radial:	position = length( (X - centerX) * invRadiusX, (Y - centerY) * invRadiusY )
		 */

		struct CanvasGeometry
		{
			TintShape	shape;

			float		a, b, c;				// Linear.

			float		centerX, centerY;		// Radial.
			float		invRadiusX, invRadiusY;
		};

		CanvasGeometry	canvasGeometry(const Tint* pTint, const RectSPX& rect);

		float			positionAt(const CanvasGeometry& geo, float x, float y);		// Position at canvas pixel coordinate, no spread applied.

		//____ Spread _________________________________________________________

		inline float	applySpread(float position, TintSpread spread);

		//____ Colors _________________________________________________________

		HiColor			mixColors(HiColor from, HiColor to, float fraction, ColorSpace colorSpace);

		/**
		 * Fill a lookup table with the colors of a simple Tint.
		 *
		 * Entry i covers position:
		 *   bSquared == false:	begin + (end - begin) * i / (entries - 1)
		 *   bSquared == true:	end * sqrt( i / (entries - 1) )		(begin is ignored and taken as 0.0)
		 *
		 * The squared form is for radial Tints, where a renderer can step the
		 * squared distance with additions and use it as index directly.
		 *
		 * Spread is applied, so a table covering more than 0.0 -> 1.0 repeats
		 * or reflects as the Tint specifies.
		 */

		void			buildLUT(const Tint* pTint, int entries, HiColor* pOutput, float begin = 0.f, float end = 1.f, bool bSquared = false);

		//____ Encoding _______________________________________________________
		/**
		 * Tints are passed to backends (and through streams) in encoded form:
		 * a sequence of 16-bit words plus colors, one color per stop.
		 *
		 * Layout, all counts in 16-bit words, always a multiple of two so that
		 * 32-bit alignment is kept:
		 *
		 *   uint16	nLayers				0 = no tint, 1 = simple tint, 2-4 = mix.
		 *   uint16	padding
		 *   per layer:
		 *     uint16	weight				0 -> 4096, weights of all layers sum to 4096.
		 *     uint16	shape | spread << 4 | colorSpace << 8
		 *     uint16	nStops				1 -> Tint::c_maxStops
		 *     uint16	padding
		 *     float	geometry[4]			CanvasGeometry. Linear: a, b, c, 0. Radial: centerX, centerY, invRadiusX, invRadiusY.
		 *     uint16	stopPos[nStops]		0 -> 65535 for 0.0 -> 1.0.
		 *     uint16	padding				Only if nStops is odd.
		 *
		 * Colors: nStops colors per layer, in layer order.
		 */

		const int c_maxEncodedTintWords = 2 + Tint::c_maxMixComponents * (4 + 8 + Tint::c_maxStops);
		const int c_maxEncodedTintColors = Tint::c_maxMixComponents * Tint::c_maxStops;

		struct TintLayer
		{
			int			weight;					// 0 -> 4096
			TintShape	shape;
			TintSpread	spread;
			ColorSpace	colorSpace;
			int			nStops;
			float		geo[4];					// See CanvasGeometry.
			float		stopPos[Tint::c_maxStops];
			HiColor		stopColors[Tint::c_maxStops];
		};

		struct DecodedTint
		{
			int			nLayers;				// 0 = no tint.
			TintLayer	layers[Tint::c_maxMixComponents];
		};

		int				encodeTint(const Tint* pTint, const RectSPX& rect, uint16_t* pWords, HiColor* pColors, int& nColors);	// Returns number of words. pTint may be null.
		const uint16_t*	decodeTint(const uint16_t* pWords, const HiColor*& pColors, DecodedTint& output);						// Returns pointer to word after the tint.
		int				encodedTintSize(const uint16_t* pWords, int& nColors);													// Number of words, for skipping.

		//____ Serialization ______________________________________________________
		/**
		 * Tint objects (not placed in any rect) serialized to bytes, e.g. for streams
		 * and plugins. Native byte order.
		 *
		 *   uint8	nComponents			0 = no tint (nullptr), 1 = simple tint, 2-4 = mix.
		 *   uint8	padding[3]
		 *   per component:
		 *     float	weight
		 *     uint8	shape, spread, colorSpace, radiusMode
		 *     float	begin.x, begin.y, end.x, end.y, center.x, center.y, radius.w, radius.h
		 *     uint8	nStops
		 *     uint8	padding[3]
		 *     per stop:
		 *       float		pos
		 *       int16		b, g, r, a
		 */

		const int c_maxSerializedTintBytes = 4 + Tint::c_maxMixComponents * (44 + Tint::c_maxStops * 12);

		int				serializeTint(const Tint* pTint, uint8_t* pDest);			// Returns bytes written. pTint may be nullptr.
		Tint_p			deserializeTint(const uint8_t* pSource, int& bytesRead);	// Returns nullptr if no tint was serialized (or on error).

		CanvasGeometry	layerGeometry(const TintLayer& layer);
		bool			isLayerFlat(const TintLayer& layer);

		// Build LUT for a decoded layer, entry i covering positions i/(entries-1) -> (i+1)/(entries-1) and sampled
		// in the middle of that span, so it should be indexed with floor(position * (entries - 1)).
		// Spread is not applied, entries - 1 is expected to be a power of two so the renderer can apply it.

		void			buildLUT(const TintLayer& layer, int entries, HiColor* pOutput, HiColor multiplier = HiColor::White);

		HiColor			layerColorAt(const TintLayer& layer, float position, HiColor multiplier = HiColor::White);		// No spread applied (e.g. Pad).

		// Size of lookup table for a layer, as (1 << bits) + 1 entries. Based on the length of the gradient
		// in pixels, two entries per pixel, 2 to 2048+1 entries. 0 for a flat layer.

		int				lutBitsForLayer(const TintLayer& layer);

		//____ GPU tint blocks ______________________________________________________
		/**
		 * Decoded tints prepared for evaluation in shaders, as an array of float4
		 * (r,g,b,a order for colors). Same lookup tables and indexing as the software
		 * renderer, so results match.
		 *
		 *   [0]					nLayers, 0, 0, 0
		 *   per layer:
		 *     [+0]				shape, spread, N (LUT has N+1 entries, N is 0 for a flat layer), weight (0.0 -> 1.0)
		 *     [+1]				geometry (see CanvasGeometry)
		 *     [+2]				color for positions below 0.0 with Pad spread, or the flat color
		 *     [+3 -> +3+N]		LUT. Entry i covers positions i/N -> (i+1)/N.
		 *
		 * Evaluation of a layer at pixel center p:
		 *   t = shape == 0 ? dot(geo.xy, p) + geo.z : length((p - geo.xy) * geo.zw)
		 *   i = floor(t * N)
		 *   Pad:		i < 0 ? padLow : lut[min(i, N)]
		 *   Repeat:	lut[i mod N]
		 *   Reflect:	r = i mod 2N, lut[r >= N ? 2N-1-r : r]
		 * Result is the weighted sum of all layers. Colors are multiplied with multiplier.
		 */

		int				gpuTintBlockSize(const DecodedTint& tint);		// In float4 units.
		void			writeGpuTintBlock(const DecodedTint& tint, float* pOutput, HiColor multiplier = HiColor::White);


		//____ applySpread() __________________________________________________

		inline float applySpread(float position, TintSpread spread)
		{
			switch (spread)
			{
				default:
				case TintSpread::Pad:
					return position < 0.f ? 0.f : position > 1.f ? 1.f : position;

				case TintSpread::Repeat:
				{
					float t = position - std::floor(position);
					return t;
				}

				case TintSpread::Reflect:
				{
					float t = position - 2.f * std::floor(position * 0.5f);		// 0.0 -> 2.0
					return t > 1.f ? 2.f - t : t;
				}
			}
		}
	}

}

#endif //WG_TINTTOOLS_DOT_H
