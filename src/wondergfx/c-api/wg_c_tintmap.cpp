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

#include <wg_c_tintmap.h>
#include <wg_tintmap.h>

using namespace wg;

inline Tintmap* getPtr(wg_obj obj) {
	return static_cast<Tintmap*>(reinterpret_cast<Object*>(obj));
}

int wg_isTintmapOpaque( wg_obj tintmap )
{
	return getPtr(tintmap)->isOpaque();
}

int wg_isTintmapVertical( wg_obj tintmap )
{
	return getPtr(tintmap)->isVertical();
}

int wg_isTintmapHorizontal( wg_obj tintmap )
{
	return getPtr(tintmap)->isHorizontal();
}

void wg_exportHorizontalColors( wg_obj tintmap, wg_spx length, wg_color* pOutput)
{
	getPtr(tintmap)->exportHorizontalColors(length, (HiColor*) pOutput);
}

void wg_exportVerticalColors( wg_obj tintmap, wg_spx length, wg_color* pOutput)
{
	getPtr(tintmap)->exportVerticalColors(length, (HiColor*) pOutput);
}



