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
#ifndef	WG_GRADIENT_DOT_H
#define	WG_GRADIENT_DOT_H
#pragma once

#include <wg_gfxtypes.h>
#include <wg_color.h>

namespace wg
{
	class Gradient
	{
	public:
		const static Gradient	Undefined;

		Gradient() :
			topLeft(HiColor::Undefined),
			topRight(HiColor::Undefined),
			bottomRight(HiColor::Undefined),
			bottomLeft(HiColor::Undefined)
		{
		}

		Gradient(HiColor fixedColor) :
			topLeft(fixedColor),
			topRight(fixedColor),
			bottomRight(fixedColor),
			bottomLeft(fixedColor)
		{
		}
		
		Gradient(HiColor topLeft, HiColor topRight, HiColor bottomRight, HiColor bottomLeft) :
			topLeft(topLeft),
			topRight(topRight),
			bottomRight(bottomRight),
			bottomLeft(bottomLeft)
		{
		}

		Gradient(Placement start, HiColor startColor, HiColor endColor);

		inline void clear() { topLeft = topRight = bottomRight = bottomLeft = HiColor::White; }
		inline bool isOpaque() const { return (topLeft.a + topRight.a + bottomRight.a + bottomLeft.a) == 4096 * 4; }
		inline bool isUndefined() const { return topLeft.isUndefined(); }
		inline bool isValid() const { return topLeft.isValid() && topRight.isValid() && bottomLeft.isValid() && bottomRight.isValid(); }
		inline bool isVertical() const { return ((topLeft != bottomLeft) || (topRight != bottomRight)); }
		inline bool isHorizontal() const { return ((topLeft != topRight) || (bottomLeft != bottomRight)); }

		inline bool operator==(const Gradient& other) const
		{
			return	((topLeft.argb ^ other.topLeft.argb) | (topRight.argb ^ other.topRight.argb) |
					 (bottomRight.argb ^ other.bottomRight.argb) | (bottomLeft.argb ^ other.bottomLeft.argb)) == 0;
		}

		inline bool operator!=(const Gradient& other) const
		{
			return	((topLeft.argb ^ other.topLeft.argb) | (topRight.argb ^ other.topRight.argb) |
				(bottomRight.argb ^ other.bottomRight.argb) | (bottomLeft.argb ^ other.bottomLeft.argb)) != 0;
		}


		HiColor		topLeft;
		HiColor		topRight;
		HiColor		bottomRight;
		HiColor		bottomLeft;
	};

}



#endif //WG_GRADIENT_DOT_H
