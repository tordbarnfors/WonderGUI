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
