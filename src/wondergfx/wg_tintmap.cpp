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


#include <wg_tintmap.h>

namespace wg
{

	const TypeInfo Tintmap::TYPEINFO = { "Tintmap", &Object::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Tintmap::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ _fill() __________________________________________________

	void Tintmap::_fill(spx length, HiColor * pOutput, const HiColor& color )
	{
		int len = length / 64;

		for( int i = 0 ; i < len ; i++ )
			* pOutput ++ = color;
	}
}



