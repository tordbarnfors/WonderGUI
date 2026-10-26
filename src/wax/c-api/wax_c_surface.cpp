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
#include <wax_c_surface.h>
#include <wax_surface.h>

#include <type_traits>


using namespace wax;


static_assert( sizeof(wax_ninePatch) == sizeof(wax::NinePatch), "wax_ninePatch differs in size from wax::NinePatch!" );
static_assert( std::is_trivially_copyable<wax::NinePatch>::value, "wax::NinePatch is not trivially copyable" );


inline Surface* getPtr(wax_obj obj) {
	return static_cast<Surface*>(reinterpret_cast<Object*>(obj));
}


void wax_setSurfaceIdentity(wax_obj surface, int id)
{
	getPtr(surface)->setIdentity(id);
}


int wax_getSurfaceIdentity(wax_obj surface)
{
	return getPtr(surface)->identity();
}


wax_sizeI wax_surfacePixelSize(wax_obj surface)
{
    SizeI size = getPtr(surface)->pixelSize();
    return * (wax_sizeI*) &size;
}


int wax_surfacePixelWidth(wax_obj surface)
{
	return getPtr(surface)->pixelWidth();
}


int wax_surfacePixelHeight(wax_obj surface)
{
	return getPtr(surface)->pixelHeight();
}


wax_size wax_surfacePointSize(wax_obj surface)
{
    Size size = getPtr(surface)->pointSize();
    return *(wax_size*)&size;
}


wax_pts wax_surfacePointWidth(wax_obj surface)
{
	return getPtr(surface)->pointWidth();
}


wax_pts wax_surfacePointHeight(wax_obj surface)
{
	return getPtr(surface)->pointHeight();
}


int wax_surfaceScale(wax_obj surface)
{
	return getPtr(surface)->scale();
}


wax_sampleMethod	wax_surfaceSampleMethod(wax_obj surface)
{
	return (wax_sampleMethod) getPtr(surface)->sampleMethod();

}


int wax_surfaceIsTiling(wax_obj surface)
{
	return getPtr(surface)->isTiling();
}


int wax_surfaceIsMipmapped(wax_obj surface)
{
	return getPtr(surface)->isMipmapped();
}


int wax_surfaceAlpha(wax_obj surface, wax_coordSPX coord)
{
	return getPtr(surface)->alpha( * (CoordSPX*) &coord );
}


const wax_color8* wax_surfacePalette(wax_obj surface)
{
	return (const wax_color8 *) getPtr(surface)->palette();
}

int wax_surfacePaletteSize(wax_obj surface)
{
	return getPtr(surface)->paletteSize();
}

int wax_surfacePaletteCapacity(wax_obj surface)
{
	return getPtr(surface)->paletteCapacity();
}

const wax_pixelDescription* wax_surfacePixelDescription(wax_obj surface)
{
	return (const wax_pixelDescription*)getPtr(surface)->pixelDescription();
}


wax_pixelFormat wax_surfacePixelFormat(wax_obj surface)
{
	return (wax_pixelFormat) getPtr(surface)->pixelFormat();
}


int wax_surfacePixelBits(wax_obj surface)
{
	return getPtr(surface)->pixelBits();
}

wax_colorSpace wax_surfaceColorSpace(wax_obj surface)
{
	return (wax_colorSpace) getPtr(surface)->colorSpace();
}

int wax_surfaceIsBigEndian(wax_obj surface)
{
	return getPtr(surface)->isBigEndian();
}


int wax_surfaceIsOpaque(wax_obj surface)
{
	return getPtr(surface)->isOpaque();
}


int wax_surfaceCanBeCanvas(wax_obj surface)
{
	return getPtr(surface)->canBeCanvas();
}


wax_pixelBuffer wax_allocPixelBuffer(wax_obj surface)
{
	auto pixbuf = getPtr(surface)->allocPixelBuffer();
	return { (wax_pixelFormat)pixbuf.format, pixbuf.pixels, (wax_color8*)pixbuf.palette, *(wax_rectI*)&pixbuf.rect, pixbuf.pitch, (wax_colorSpace)pixbuf.colorSpace, (uint8_t)pixbuf.bigEndian };
}


wax_pixelBuffer	wax_allocPixelBufferFromRect(wax_obj surface, const wax_rectI* rect)
{
	auto pixbuf = getPtr(surface)->allocPixelBuffer(* (RectI*) rect);
	return { (wax_pixelFormat)pixbuf.format, pixbuf.pixels, (wax_color8*)pixbuf.palette, *(wax_rectI*)&pixbuf.rect, pixbuf.pitch, (wax_colorSpace)pixbuf.colorSpace, (uint8_t)pixbuf.bigEndian };
}


int wax_pushPixels(wax_obj surface, const wax_pixelBuffer* buffer)
{
	return getPtr(surface)->pushPixels( { (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch, (ColorSpace)buffer->colorSpace, buffer->bigEndian != 0 });
}


int wax_pushPixelsFromRect(wax_obj surface, const wax_pixelBuffer* buffer, const wax_rectI* bufferRect)
{
	return getPtr(surface)->pushPixels( { (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch, (ColorSpace)buffer->colorSpace, buffer->bigEndian != 0 }, * (const RectI*) bufferRect );
}


void wax_pullPixels(wax_obj surface, const wax_pixelBuffer* buffer)
{
	getPtr(surface)->pullPixels({ (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch, (ColorSpace)buffer->colorSpace, buffer->bigEndian != 0 });
}


void wax_pullPixelsFromRect(wax_obj surface, const wax_pixelBuffer* buffer, const wax_rectI* bufferRect)
{
	getPtr(surface)->pullPixels({ (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch, (ColorSpace)buffer->colorSpace, buffer->bigEndian != 0 }, *(const RectI*)bufferRect);
}


void wax_freePixelBuffer(wax_obj surface, const wax_pixelBuffer* buffer)
{
	getPtr(surface)->freePixelBuffer( { (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch, (ColorSpace)buffer->colorSpace, buffer->bigEndian != 0 } );
}


int wax_fillSurface(wax_obj surface, wax_color color)
{
	return getPtr(surface)->fill(*(HiColor*)&color);
}


int wax_fillSurfaceRect(wax_obj surface, const wax_rectI* region, wax_color color )
{
	return getPtr(surface)->fill( * (const RectI*) region, * (HiColor*)&color);
}


int wax_copySurfaceRect(wax_obj destSurface, wax_coordI dest, wax_obj sourceSurface, const wax_rectI* srcRect )
{
	return getPtr(destSurface)->copy(*(CoordI*) &dest, getPtr(sourceSurface), * (const RectI*) srcRect );

}


int wax_copySurface(wax_obj destSurface, wax_coordI dest, wax_obj sourceSurface)
{
	return getPtr(destSurface)->copy( *(CoordI*) &dest, getPtr(sourceSurface) );

}


void wax_setSurfaceBaggage(wax_obj surface, wax_obj baggage)
{
	getPtr(surface)->setBaggage(reinterpret_cast<Object*>(baggage));
}


wax_obj wax_getSurfaceBaggage(wax_obj surface)
{
	return (wax_obj)getPtr(surface)->baggage().rawPtr();
}


int wax_addSurfaceObserver(wax_obj surface, wax_surfaceObserver_func func, void* pData, int data)
{
	return getPtr(surface)->addObserver([func,pData,data](int nRects, const RectI* pRects) { func(nRects, (const wax_rectI*)pRects, pData, data); } );
}


int	wax_removeSurfaceObserver(wax_obj surface, int observerId)
{
	return getPtr(surface)->removeObserver(observerId);
}


wax_surfaceBP wax_getSurfaceBlueprint(wax_obj surface)
{
	wax_surfaceBP dest{};

	Surface::Blueprint src = getPtr(surface)->blueprint();

	dest.byteOrder		= src.bigEndian ? WAX_BYTEORDER_BIG_ENDIAN : WAX_BYTEORDER_LITTLE_ENDIAN;
	dest.buffered		= src.buffered;
	dest.canvas			= src.canvas;
	dest.colorSpace		= (wax_colorSpace) src.colorSpace;
	dest.palette		= (wax_color8*)src.palette;
	dest.paletteSize	= src.paletteSize;
	dest.paletteCapacity= src.paletteCapacity;
	dest.dynamic		= src.dynamic;
	dest.format			= (wax_pixelFormat) src.format;
	dest.identity		= src.identity;
	dest.mipmap			= src.mipmap;
	dest.sampleMethod	= (wax_sampleMethod) src.sampleMethod;
	dest.scale			= src.scale;
	dest.size.w			= src.size.w;
	dest.size.h			= src.size.h;
	dest.tiling			= src.tiling;

	return dest;
}
