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
#ifndef	WG_SOFTTINT_DOT_H
#define	WG_SOFTTINT_DOT_H
#pragma once

#include <wg_tinttools.h>

#include <vector>

namespace wg
{
	//____ SoftTint ___________________________________________________________
	/**
	 * @brief A Tint prepared for software rendering.
	 *
	 * Holds lookup tables and geometry for a decoded Tint and generates lines
	 * of tint colors from them. Used by SoftBackend for the device tint and by
	 * SoftEdgemap for segment tints.
	 *
	 * Generating colors and building the lookup tables is integer math only, since
	 * SoftBackend runs on CPUs without FPU. Floating point is limited to converting
	 * the geometry of each layer to fixed point in set().
	 */

	class SoftTint
	{
	public:

		// Colors of Tints are sRGB and interpolated in sRGB, while everything SoftTint
		// gives out is linear, ready for blending. Multipliers are linear.

		// Set from a decoded tint. All colors are multiplied with multiplier.

		void		set(const TintTools::DecodedTint& tint, HiColor multiplier = HiColor::White);

		// Set from a Tint placed in given rect (in spx of the coordinate system lines are later generated in).

		void		set(const Tint* pTint, const RectSPX& rect, HiColor multiplier = HiColor::White);

		// Set to a flat color, which is linear.

		void		setFlat(HiColor color);

		inline bool		isFlat() const { return m_bFlat; }
		inline HiColor	flatColor() const { return m_flatColor; }		// Only valid if isFlat().
		inline bool		isOpaque() const { return m_bOpaque; }
		inline bool		isTransparent() const { return m_bTransparent; }

		inline bool		variesAlongX() const { return m_bVariesX; }
		inline bool		variesAlongY() const { return m_bVariesY; }

		// Generate colors for pixels (x,y), (x+dx, y+dy) ... length pixels.
		// Colors are sampled at pixel centers. dx and dy should be -1, 0 or 1.
		// Coordinates are expected to stay within +-2^20.

		void		generate(int x, int y, int dx, int dy, int length, HiColor* pOutput) const;

		// Generate colors at positions (x,y), (x+dx, y+dy) ... length positions, in pixels with
		// 16 binals. Unlike generate() these are positions, not pixels, so pixel (x,y) is sampled
		// at (x + 0.5, y + 0.5). Used where pixels are mapped to arbitrary positions, as by Warpmaps.
		// Positions are expected to stay within +-2^20.

		void		generateSpan(int64_t x, int64_t y, int64_t dx, int64_t dy, int length, HiColor* pOutput) const;

	protected:

		struct Layer
		{
			TintShape		shape;
			TintSpread		spread;
			int				weight;			// 0 -> 4096.
			bool			bFlat;
			HiColor			flatColor;		// Only if bFlat, multiplier included.
			HiColor			padLowColor;	// Color for positions below 0.0 with Pad spread (LUT entry 0 is sampled a bit inside).
			int				lutBits;		// LUT has (1 << lutBits) + 1 entries.
			int				lutOfs;			// Offset into m_lut.

			// Geometry in fixed point, positions in LUT entries with 32 binals, for the center
			// of pixel (x,y):
			//
			//   Linear:	position = stepX * x + stepY * y + originX
			//   Radial:	position = length(u, v), where u = stepX * x + originX, v = stepY * y + originY

			int64_t			stepX;
			int64_t			stepY;
			int64_t			originX;
			int64_t			originY;		// Radial only.

			int64_t			stepX16;		// stepX and stepY with 16 binals, each rounded once from
			int64_t			stepY16;		// floating point.

			mutable int		radialHints[2];	// Radial only, LUT index / 2 at start and end of last span, -1 if none.
		};

		template<class LayerGenerator>
		void		_mixLayers(int length, HiColor* pOutput, LayerGenerator&& generateLayer) const;

		void		_generateLinear(const Layer& layer, int x, int y, int dx, int dy, int length, HiColor* pOutput) const;
		void		_generateRadial(const Layer& layer, int x, int y, int dx, int dy, int length, HiColor* pOutput) const;
		void		_linearSpan(const Layer& layer, int64_t pos, int64_t step, int length, HiColor* pOutput) const;
		void		_radialSpan(const Layer& layer, int64_t u, int64_t v, int64_t du, int64_t dv, int length, HiColor* pOutput) const;

		int			m_nLayers = 0;
		Layer		m_layers[Tint::c_maxMixComponents];

		std::vector<HiColor>	m_lut;
		mutable std::vector<int>	m_accumulator;
		mutable std::vector<HiColor>	m_layerBuffer;

		bool		m_bFlat = true;
		HiColor		m_flatColor = HiColor::White;
		bool		m_bOpaque = true;
		bool		m_bTransparent = false;
		bool		m_bVariesX = false;
		bool		m_bVariesY = false;
	};

}

#endif //WG_SOFTTINT_DOT_H
