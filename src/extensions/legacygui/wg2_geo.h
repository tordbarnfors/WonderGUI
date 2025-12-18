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
#ifndef	WG2_GEO_DOT_H
#define	WG2_GEO_DOT_H
//=============================================================================

#ifndef	WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#include <wg_geo.h>

typedef wg::CoordT<int8_t>		WgCoord8;
typedef wg::CoordT<int16_t>		WgCoord16;
typedef wg::CoordT<uint8_t>		WgUCoord8;

typedef wg::CoordI		WgCoord;
typedef wg::CoordF		WgCoordF;

typedef wg::BorderI		WgBorders;
typedef wg::SizeI		WgSize;
typedef wg::RectI		WgRect;
typedef wg::RectF		WgRectF;


//=======================================================================================
#endif	//WG2_GEO_DOT_H
