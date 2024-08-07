/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/


#include <wg_gradient.h>

namespace wg
{

	const Gradient Gradient::Undefined = Gradient();


	//____ constructor ________________________________________________________

	Gradient::Gradient(Placement start, HiColor startColor, HiColor endColor)
	{
		switch (start)
		{
		case Placement::Center:
			topLeft = topRight = bottomRight = bottomLeft = startColor;
			break;

		case Placement::NorthWest:
			topLeft = startColor;
			bottomRight = endColor;
			topRight = bottomLeft = HiColor::mix(startColor, endColor, 2048);
			break;

		case Placement::North:
			topLeft = topRight = startColor;
			bottomLeft = bottomRight = endColor;
			break;

		case Placement::NorthEast:
			topRight = startColor;
			bottomLeft = endColor;
			topLeft = bottomRight = HiColor::mix(startColor, endColor, 2048);
			break;

		case Placement::East:
			topRight = bottomRight = startColor;
			topLeft = bottomLeft = endColor;
			break;

		case Placement::SouthEast:
			bottomRight = startColor;
			topLeft = endColor;
			bottomLeft = topRight = HiColor::mix(startColor, endColor, 2048);
			break;

		case Placement::South:
			bottomLeft = bottomRight = startColor;
			topLeft = topRight = endColor;
			break;

		case Placement::SouthWest:
			bottomLeft = startColor;
			topRight = endColor;
			bottomRight = topLeft = HiColor::mix(startColor, endColor, 2048);
			break;

		case Placement::West:
			topLeft = bottomLeft = startColor;
			topRight = bottomRight = endColor;
			break;
				
		default:
			return;
		}
	}

}



