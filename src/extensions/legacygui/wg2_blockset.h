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
#ifndef WG2_BLOCKSET_DOT_H
#define WG2_BLOCKSET_DOT_H

#ifndef WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

#include <wg_surface.h>
#include <wg_blockskin.h>


//____ WgBlockset _____________________________________________________________

class WgBlockset
{

public:
	static wg::BlockSkin_p CreateFromRow( wg::Surface * pSurf, const WgRect& rect, int nBlocks, int blockSpacing=0, int flags = 0 );
	static wg::BlockSkin_p CreateFromColumn( wg::Surface * pSurf, const WgRect& rect, int nBlocks, int blockSpacing=0, int flags = 0 );

};





#endif //WG2_BLOCKSET_DOT_H
