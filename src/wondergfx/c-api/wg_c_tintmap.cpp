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

void wg_exportColors( wg_obj tintmap, wg_sizeI tintmapSize, wg_color* pOutputX, wg_color* pOutputY)
{
	getPtr(tintmap)->exportColors({tintmapSize.w, tintmapSize.h}, (HiColor*) pOutputX, (HiColor*) pOutputY);
}



