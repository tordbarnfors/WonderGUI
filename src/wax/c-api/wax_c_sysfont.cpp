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
#include <wg_c_sysfont.h>
#include <wg_sysfont.h>

#include <type_traits>


using namespace wg;

wg_obj wg_createSysFont(wg_obj surfaceFactory, wg_obj backupFont)
{
	SurfaceFactory * pFactory = surfaceFactory == nullptr ? nullptr : static_cast<SurfaceFactory*>(reinterpret_cast<Object*>(surfaceFactory) );
	Font * pBackupFont = backupFont == nullptr ? nullptr : static_cast<Font*>(reinterpret_cast<Object*>(backupFont) );

	auto pSysFont = SysFont::create( pFactory, pBackupFont );
	pSysFont->retain();
	return static_cast<Object*>(pSysFont);
}
