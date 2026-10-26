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
#include <wax_c_surfacefactory.h>
#include <wax_c_internal.h>
#include <wax_surfacefactory.h>

using namespace wax;

inline SurfaceFactory* getPtr(wax_obj obj) {
	return static_cast<SurfaceFactory*>(reinterpret_cast<Object*>(obj));
}




wax_sizeI wax_maxSurfaceSize(wax_obj surfaceFactory)
{
    SizeI size = getPtr(surfaceFactory)->maxSize();
    return *(wax_sizeI*) &size;
}


wax_obj wax_createSurface(wax_obj factory, const wax_surfaceBP* blueprint)
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	
	auto p = getPtr(factory)->createSurface(bp);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}


wax_obj wax_createSurfaceFromBlob(wax_obj factory, const wax_surfaceBP* blueprint, wax_obj blob, int pitch)
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto p = getPtr(factory)->createSurface(bp, static_cast<Blob*>(reinterpret_cast<Object*>(blob)), pitch );
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj	wax_createSurfaceFromBitmap(wax_obj factory, const wax_surfaceBP* blueprint, const uint8_t* pPixels,
								   wax_pixelFormat pixelFormat, int pitch, const wax_color8 * pPalette, int paletteSize )
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto p = getPtr(factory)->createSurface(bp, pPixels, (PixelFormat) pixelFormat, pitch, (Color8*) pPalette, paletteSize);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj	wax_createSurfaceFromRawData(wax_obj factory, const wax_surfaceBP* blueprint, const uint8_t* pPixels,
									const wax_pixelDescription * pPixelDescription, int pitch, const wax_color8 * pPalette, int paletteSize )
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto p = getPtr(factory)->createSurface(bp, pPixels, *(PixelDescription*)pPixelDescription, pitch, (Color8*) pPalette, paletteSize);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

