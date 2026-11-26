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
#include <wax_c_bitmapreader.h>
#include <wax_bitmapreader.h>

using namespace wax;

inline BitmapReader* getPtr(wax_obj obj) {
	return static_cast<BitmapReader*>(reinterpret_cast<Object*>(obj));
}


wax_obj wax_createBitmapReader(const wax_bitmapReaderBP* blueprint)
{
	BitmapReader::Blueprint bp;
	bp.factory = blueprint->factory == nullptr ? nullptr : static_cast<BitmapFactory*>(reinterpret_cast<Object*>(blueprint->factory));
	
	auto pReader = BitmapReader::create(bp);
	pReader->retain();
	return static_cast<Object*>(pReader);
}

wax_obj wax_readBitmapFromBlob( wax_obj bitmapReader, wax_obj blob )
{
	auto pBitmap = getPtr(bitmapReader)->readBitmapFromBlob(static_cast<Blob*>(reinterpret_cast<Object*>(blob)));
	if( !pBitmap )
		return nullptr;

	pBitmap->retain();
	return static_cast<Object*>(pBitmap);
}

wax_obj wax_readBitmapFromMemory( wax_obj bitmapReader, const char * pData )
{
	auto pBitmap = getPtr(bitmapReader)->readBitmapFromMemory(pData);
	if( !pBitmap )
		return nullptr;

	pBitmap->retain();
	return static_cast<Object*>(pBitmap);
}
