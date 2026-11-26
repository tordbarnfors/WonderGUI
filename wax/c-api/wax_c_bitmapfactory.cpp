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
#include <wax_c_bitmapfactory.h>
#include <wax_c_internal.h>
#include <wax_bitmapfactory.h>

using namespace wax;

inline BitmapFactory* getPtr(wax_obj obj) {
	return static_cast<BitmapFactory*>(reinterpret_cast<Object*>(obj));
}




wax_sizeI wax_maxBitmapSize(wax_obj bitmapFactory)
{
    SizeI size = getPtr(bitmapFactory)->maxSize();
    return *(wax_sizeI*) &size;
}


wax_obj wax_createBitmap(wax_obj factory, const wax_bitmapBP* blueprint)
{
	Bitmap::Blueprint	bp;
	convertBitmapBlueprint(&bp, blueprint);
	
	auto p = getPtr(factory)->createBitmap(bp);
	if( !p )
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}


wax_obj wax_createBitmapFromBlob(wax_obj factory, const wax_bitmapBP* blueprint, wax_obj blob, int pitch)
{
	Bitmap::Blueprint	bp;
	convertBitmapBlueprint(&bp, blueprint);
	auto p = getPtr(factory)->createBitmap(bp, static_cast<Blob*>(reinterpret_cast<Object*>(blob)), pitch );
	if( !p )
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj	wax_createBitmapFromBitmap(wax_obj factory, const wax_bitmapBP* blueprint, const uint8_t* pPixels,
								   wax_pixelFormat pixelFormat, int pitch, const wax_color8 * pPalette, int paletteSize )
{
	Bitmap::Blueprint	bp;
	convertBitmapBlueprint(&bp, blueprint);
	auto p = getPtr(factory)->createBitmap(bp, pPixels, (PixelFormat) pixelFormat, pitch, (Color8*) pPalette, paletteSize);
	if( !p )
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj	wax_createBitmapFromRawData(wax_obj factory, const wax_bitmapBP* blueprint, const uint8_t* pPixels,
									const wax_pixelDescription * pPixelDescription, int pitch, const wax_color8 * pPalette, int paletteSize )
{
	Bitmap::Blueprint	bp;
	convertBitmapBlueprint(&bp, blueprint);
	auto p = getPtr(factory)->createBitmap(bp, pPixels, *(PixelDescription*)pPixelDescription, pitch, (Color8*) pPalette, paletteSize);
	if( !p )
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

