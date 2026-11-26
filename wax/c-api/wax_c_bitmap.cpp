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
#include <wax_c_bitmap.h>
#include <wax_bitmap.h>

#include <type_traits>


using namespace wax;


static_assert( sizeof(wax_ninePatch) == sizeof(wax::NinePatch), "wax_ninePatch differs in size from wax::NinePatch!" );
static_assert( std::is_trivially_copyable<wax::NinePatch>::value, "wax::NinePatch is not trivially copyable" );


inline Bitmap* getPtr(wax_obj obj) {
	return static_cast<Bitmap*>(reinterpret_cast<Object*>(obj));
}


void wax_setBitmapIdentity(wax_obj bitmap, int id)
{
	getPtr(bitmap)->setIdentity(id);
}


int wax_getBitmapIdentity(wax_obj bitmap)
{
	return getPtr(bitmap)->identity();
}


wax_sizeI wax_bitmapPixelSize(wax_obj bitmap)
{
    SizeI size = getPtr(bitmap)->pixelSize();
    return * (wax_sizeI*) &size;
}


int wax_bitmapPixelWidth(wax_obj bitmap)
{
	return getPtr(bitmap)->pixelWidth();
}


int wax_bitmapPixelHeight(wax_obj bitmap)
{
	return getPtr(bitmap)->pixelHeight();
}


wax_size wax_bitmapPointSize(wax_obj bitmap)
{
    Size size = getPtr(bitmap)->pointSize();
    return *(wax_size*)&size;
}


wax_pts wax_bitmapPointWidth(wax_obj bitmap)
{
	return getPtr(bitmap)->pointWidth();
}


wax_pts wax_bitmapPointHeight(wax_obj bitmap)
{
	return getPtr(bitmap)->pointHeight();
}


int wax_bitmapScale(wax_obj bitmap)
{
	return getPtr(bitmap)->scale();
}


wax_sampleMethod	wax_bitmapSampleMethod(wax_obj bitmap)
{
	return (wax_sampleMethod) getPtr(bitmap)->sampleMethod();

}


int wax_bitmapIsTiling(wax_obj bitmap)
{
	return getPtr(bitmap)->isTiling();
}


int wax_bitmapIsMipmapped(wax_obj bitmap)
{
	return getPtr(bitmap)->isMipmapped();
}


int wax_bitmapAlpha(wax_obj bitmap, wax_coordSPX coord)
{
	return getPtr(bitmap)->alpha( * (CoordSPX*) &coord );
}


const wax_color8* wax_bitmapPalette(wax_obj bitmap)
{
	return (const wax_color8 *) getPtr(bitmap)->palette();
}

int wax_bitmapPaletteSize(wax_obj bitmap)
{
	return getPtr(bitmap)->paletteSize();
}

int wax_bitmapPaletteCapacity(wax_obj bitmap)
{
	return getPtr(bitmap)->paletteCapacity();
}

const wax_pixelDescription* wax_bitmapPixelDescription(wax_obj bitmap)
{
	return (const wax_pixelDescription*)getPtr(bitmap)->pixelDescription();
}


wax_pixelFormat wax_bitmapPixelFormat(wax_obj bitmap)
{
	return (wax_pixelFormat) getPtr(bitmap)->pixelFormat();
}


int wax_bitmapPixelBits(wax_obj bitmap)
{
	return getPtr(bitmap)->pixelBits();
}

wax_colorSpace wax_bitmapColorSpace(wax_obj bitmap)
{
	return (wax_colorSpace) getPtr(bitmap)->colorSpace();
}

int wax_bitmapIsBigEndian(wax_obj bitmap)
{
	return getPtr(bitmap)->isBigEndian();
}


int wax_bitmapIsOpaque(wax_obj bitmap)
{
	return getPtr(bitmap)->isOpaque();
}


int wax_bitmapCanBeCanvas(wax_obj bitmap)
{
	return getPtr(bitmap)->canBeCanvas();
}


int wax_bitmapIsContentLost(wax_obj bitmap)
{
	return getPtr(bitmap)->isContentLost();
}


uint8_t wax_declineBitmapRestore(wax_obj bitmap, void * pData)
{
	return 0;
}


void wax_dropBitmapRestorer(wax_obj bitmap)
{
	getPtr(bitmap)->dropRestorer();
}


wax_pixelBuffer wax_allocPixelBuffer(wax_obj bitmap)
{
	auto pixbuf = getPtr(bitmap)->allocPixelBuffer();
	return { (wax_pixelFormat)pixbuf.format, pixbuf.pixels, (wax_color8*)pixbuf.palette, *(wax_rectI*)&pixbuf.rect, pixbuf.pitch, (wax_colorSpace)pixbuf.colorSpace, (uint8_t)pixbuf.bigEndian };
}


wax_pixelBuffer	wax_allocPixelBufferFromRect(wax_obj bitmap, const wax_rectI* rect)
{
	auto pixbuf = getPtr(bitmap)->allocPixelBuffer(* (RectI*) rect);
	return { (wax_pixelFormat)pixbuf.format, pixbuf.pixels, (wax_color8*)pixbuf.palette, *(wax_rectI*)&pixbuf.rect, pixbuf.pitch, (wax_colorSpace)pixbuf.colorSpace, (uint8_t)pixbuf.bigEndian };
}


int wax_pushPixels(wax_obj bitmap, const wax_pixelBuffer* buffer)
{
	return getPtr(bitmap)->pushPixels( { (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch, (ColorSpace)buffer->colorSpace, buffer->bigEndian != 0 });
}


int wax_pushPixelsFromRect(wax_obj bitmap, const wax_pixelBuffer* buffer, const wax_rectI* bufferRect)
{
	return getPtr(bitmap)->pushPixels( { (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch, (ColorSpace)buffer->colorSpace, buffer->bigEndian != 0 }, * (const RectI*) bufferRect );
}


void wax_pullPixels(wax_obj bitmap, const wax_pixelBuffer* buffer)
{
	getPtr(bitmap)->pullPixels({ (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch, (ColorSpace)buffer->colorSpace, buffer->bigEndian != 0 });
}


void wax_pullPixelsFromRect(wax_obj bitmap, const wax_pixelBuffer* buffer, const wax_rectI* bufferRect)
{
	getPtr(bitmap)->pullPixels({ (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch, (ColorSpace)buffer->colorSpace, buffer->bigEndian != 0 }, *(const RectI*)bufferRect);
}


void wax_freePixelBuffer(wax_obj bitmap, const wax_pixelBuffer* buffer)
{
	getPtr(bitmap)->freePixelBuffer( { (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch, (ColorSpace)buffer->colorSpace, buffer->bigEndian != 0 } );
}


int wax_fillBitmap(wax_obj bitmap, wax_color color)
{
	return getPtr(bitmap)->fill(*(Color*)&color);
}


int wax_fillBitmapRect(wax_obj bitmap, const wax_rectI* region, wax_color color )
{
	return getPtr(bitmap)->fill( * (const RectI*) region, * (Color*)&color);
}


int wax_copyBitmapRect(wax_obj destBitmap, wax_coordI dest, wax_obj sourceBitmap, const wax_rectI* srcRect )
{
	return getPtr(destBitmap)->copy(*(CoordI*) &dest, getPtr(sourceBitmap), * (const RectI*) srcRect );

}


int wax_copyBitmap(wax_obj destBitmap, wax_coordI dest, wax_obj sourceBitmap)
{
	return getPtr(destBitmap)->copy( *(CoordI*) &dest, getPtr(sourceBitmap) );

}


void wax_setBitmapUserData(wax_obj bitmap, wax_obj userData)
{
	getPtr(bitmap)->setUserData(reinterpret_cast<Object*>(userData));
}


wax_obj wax_getBitmapUserData(wax_obj bitmap)
{
	return (wax_obj)getPtr(bitmap)->userData().rawPtr();
}


int wax_addBitmapObserver(wax_obj bitmap, wax_bitmapObserver_func func, void* pData, int data)
{
	return getPtr(bitmap)->addObserver([func,pData,data](int nRects, const RectI* pRects) { func(nRects, (const wax_rectI*)pRects, pData, data); } );
}


int	wax_removeBitmapObserver(wax_obj bitmap, int observerId)
{
	return getPtr(bitmap)->removeObserver(observerId);
}


wax_bitmapBP wax_getBitmapBlueprint(wax_obj bitmap)
{
	wax_bitmapBP dest{};

	Bitmap::Blueprint src = getPtr(bitmap)->blueprint();

	dest.byteOrder		= src.bigEndian ? WAX_BYTEORDER_BIG_ENDIAN : WAX_BYTEORDER_LITTLE_ENDIAN;
	dest.canvas			= src.canvas;
	dest.colorSpace		= (wax_colorSpace) src.colorSpace;
	dest.palette		= (wax_color8*)src.palette;
	dest.paletteSize	= src.paletteSize;
	dest.paletteCapacity= src.paletteCapacity;
	dest.dynamic		= src.dynamic;
	dest.format			= (wax_pixelFormat) src.format;
	dest.identity		= src.identity;
	dest.mipmap			= src.mipmap;
	dest.restorer		= nullptr;			// A std::function, which can't be given as a function of C.
	dest.restorerData	= nullptr;
	dest.sampleMethod	= (wax_sampleMethod) src.sampleMethod;
	dest.scale			= src.scale;
	dest.size.w			= src.size.w;
	dest.size.h			= src.size.h;
	dest.tiling			= src.tiling;

	return dest;
}
