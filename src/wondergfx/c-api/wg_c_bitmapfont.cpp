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
#include <wg_c_bitmapfont.h>
#include <wg_bitmapfont.h>

using namespace wg;

inline BitmapFont* getPtr(wg_obj obj) {
	return static_cast<BitmapFont*>(reinterpret_cast<Object*>(obj));
}


wg_obj wg_createBitmapFont( wg_obj surface, char * pGlyphSpec, wg_obj backupFont )
{
	auto pSurface = static_cast<Surface*>(reinterpret_cast<Object*>(surface));
	auto pBackupFont = static_cast<Font*>(reinterpret_cast<Object*>(surface));
	
	auto p = BitmapFont::create(pSurface, pGlyphSpec, pBackupFont );
	p->retain();
	return static_cast<Object*>(p.rawPtr());
}


wg_obj wg_getBitmapFontSurface( wg_obj font )
{
	auto pSurface = getPtr(font)->surface();
	return static_cast<Object*>(pSurface.rawPtr());
}

