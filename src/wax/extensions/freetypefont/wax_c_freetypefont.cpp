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
#include <wg_c_freetypefont.h>
#include <wg_freetypefont.h>

using namespace wg;

inline FreeTypeFont* getPtr(wg_obj obj) {
	return static_cast<FreeTypeFont*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createFreeTypeFont( wg_freeTypeFontBP blueprint )
{
	FreeTypeFont::Blueprint bp;

	bp.backupFont 	= blueprint.backupFont ? static_cast<Font*>(reinterpret_cast<Object*>(blueprint.backupFont)) : nullptr;
	bp.blob			= blueprint.blob ? static_cast<Blob*>(reinterpret_cast<Object*>(blueprint.blob)) : nullptr;
	bp.cache		= blueprint.cache ? static_cast<BitmapCache*>(reinterpret_cast<Object*>(blueprint.cache)) : nullptr;
	bp.faceIndex	= blueprint.faceIndex;
	bp.renderMode	= (FreeTypeFont::RenderMode) blueprint.renderMode;
	bp.stemDarkening = blueprint.stemDarkening;
	bp.xDPI			= blueprint.xDPI;
	bp.yDPI			= blueprint.yDPI;

	auto pFont = FreeTypeFont::create(bp);
	pFont->retain();
	return static_cast<Object*>(pFont);
}


wg_obj wg_getFreeTypeFontBitmapCache( wg_obj font )
{
	auto pCache = getPtr(font)->bitmapCache();
	pCache->retain();
	return static_cast<Object*>(pCache.rawPtr());
}

