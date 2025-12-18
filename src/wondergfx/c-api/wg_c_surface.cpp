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
#include <wg_c_surface.h>
#include <wg_surface.h>

#include <type_traits>


using namespace wg;


static_assert( sizeof(wg_ninePatch) == sizeof(wg::NinePatch), "wg_ninePatch differs in size from wg::NinePatch!" );
static_assert( std::is_trivially_copyable<wg::NinePatch>::value, "wg::NinePatch is not trivially copyable" );


inline Surface* getPtr(wg_obj obj) {
	return static_cast<Surface*>(reinterpret_cast<Object*>(obj));
}


void wg_setSurfaceIdentity(wg_obj surface, int id)
{
	getPtr(surface)->setIdentity(id);
}


int wg_getSurfaceIdentity(wg_obj surface)
{
	return getPtr(surface)->identity();
}


wg_sizeI wg_surfacePixelSize(wg_obj surface)
{
    SizeI size = getPtr(surface)->pixelSize();
    return * (wg_sizeI*) &size;
}


int wg_surfacePixelWidth(wg_obj surface)
{
	return getPtr(surface)->pixelWidth();
}


int wg_surfacePixelHeight(wg_obj surface)
{
	return getPtr(surface)->pixelHeight();
}


wg_size wg_surfacePointSize(wg_obj surface)
{
    Size size = getPtr(surface)->pointSize();
    return *(wg_size*)&size;
}


wg_pts wg_surfacePointWidth(wg_obj surface)
{
	return getPtr(surface)->pointWidth();
}


wg_pts wg_surfacePointHeight(wg_obj surface)
{
	return getPtr(surface)->pointHeight();
}


int wg_surfaceScale(wg_obj surface)
{
	return getPtr(surface)->scale();
}


wg_sampleMethod	wg_surfaceSampleMethod(wg_obj surface)
{
	return (wg_sampleMethod) getPtr(surface)->sampleMethod();

}


int wg_surfaceIsTiling(wg_obj surface)
{
	return getPtr(surface)->isTiling();
}


int wg_surfaceIsMipmapped(wg_obj surface)
{
	return getPtr(surface)->isMipmapped();
}


int wg_surfaceAlpha(wg_obj surface, wg_coordSPX coord)
{
	return getPtr(surface)->alpha( * (CoordSPX*) &coord );
}


const wg_color8* wg_surfacePalette(wg_obj surface)
{
	return (const wg_color8 *) getPtr(surface)->palette();
}

int wg_surfacePaletteSize(wg_obj surface)
{
	return getPtr(surface)->paletteSize();
}

int wg_surfacePaletteCapacity(wg_obj surface)
{
	return getPtr(surface)->paletteCapacity();
}

const wg_pixelDescription* wg_surfacePixelDescription(wg_obj surface)
{
	return (const wg_pixelDescription*)getPtr(surface)->pixelDescription();
}


wg_pixelFormat wg_surfacePixelFormat(wg_obj surface)
{
	return (wg_pixelFormat) getPtr(surface)->pixelFormat();
}


int wg_surfacePixelBits(wg_obj surface)
{
	return getPtr(surface)->pixelBits();
}


int wg_surfaceIsOpaque(wg_obj surface)
{
	return getPtr(surface)->isOpaque();
}


int wg_surfaceCanBeCanvas(wg_obj surface)
{
	return getPtr(surface)->canBeCanvas();
}


wg_pixelBuffer wg_allocPixelBuffer(wg_obj surface)
{
	auto pixbuf = getPtr(surface)->allocPixelBuffer();
	return { (wg_pixelFormat)pixbuf.format, pixbuf.pixels, (wg_color8*)pixbuf.palette, *(wg_rectI*)&pixbuf.rect, pixbuf.pitch };
}


wg_pixelBuffer	wg_allocPixelBufferFromRect(wg_obj surface, const wg_rectI* rect)
{
	auto pixbuf = getPtr(surface)->allocPixelBuffer(* (RectI*) rect);
	return { (wg_pixelFormat)pixbuf.format, pixbuf.pixels, (wg_color8*)pixbuf.palette, *(wg_rectI*)&pixbuf.rect, pixbuf.pitch };
}


int wg_pushPixels(wg_obj surface, const wg_pixelBuffer* buffer)
{
	return getPtr(surface)->pushPixels( { (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, * (RectI*)&buffer->rect, buffer->pitch });
}


int wg_pushPixelsFromRect(wg_obj surface, const wg_pixelBuffer* buffer, const wg_rectI* bufferRect)
{
	return getPtr(surface)->pushPixels( { (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, * (RectI*)&buffer->rect, buffer->pitch }, * (const RectI*) bufferRect );
}


void wg_pullPixels(wg_obj surface, const wg_pixelBuffer* buffer)
{
	getPtr(surface)->pullPixels({ (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch });
}


void wg_pullPixelsFromRect(wg_obj surface, const wg_pixelBuffer* buffer, const wg_rectI* bufferRect)
{
	getPtr(surface)->pullPixels({ (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch }, *(const RectI*)bufferRect);
}


void wg_freePixelBuffer(wg_obj surface, const wg_pixelBuffer* buffer)
{
	getPtr(surface)->freePixelBuffer( { (PixelFormat)buffer->format, buffer->pixels, (Color8*)buffer->palette, *(RectI*)&buffer->rect, buffer->pitch } );
}


int wg_fillSurface(wg_obj surface, wg_color color)
{
	return getPtr(surface)->fill(*(HiColor*)&color);
}


int wg_fillSurfaceRect(wg_obj surface, const wg_rectI* region, wg_color color )
{
	return getPtr(surface)->fill( * (const RectI*) region, * (HiColor*)&color);
}


int wg_copySurfaceRect(wg_obj destSurface, wg_coordI dest, wg_obj sourceSurface, const wg_rectI* srcRect )
{
	return getPtr(destSurface)->copy(*(CoordI*) &dest, getPtr(sourceSurface), * (const RectI*) srcRect );

}


int wg_copySurface(wg_obj destSurface, wg_coordI dest, wg_obj sourceSurface)
{
	return getPtr(destSurface)->copy( *(CoordI*) &dest, getPtr(sourceSurface) );

}


void wg_setSurfaceBaggage(wg_obj surface, wg_obj baggage)
{
	getPtr(surface)->setBaggage(reinterpret_cast<Object*>(baggage));
}


wg_obj wg_getSurfaceBaggage(wg_obj surface)
{
	return (wg_obj)getPtr(surface)->baggage().rawPtr();
}


int wg_addSurfaceObserver(wg_obj surface, wg_surfaceObserver_func func, void* pData, int data)
{
	return getPtr(surface)->addObserver([func,pData,data](int nRects, const RectI* pRects) { func(nRects, (const wg_rectI*)pRects, pData, data); } );
}


int	wg_removeSurfaceObserver(wg_obj surface, int observerId)
{
	return getPtr(surface)->removeObserver(observerId);
}


wg_surfaceBP wg_getSurfaceBlueprint(wg_obj surface)
{
	wg_surfaceBP dest{};

	Surface::Blueprint src = getPtr(surface)->blueprint();

	dest.buffered		= src.buffered;
	dest.canvas			= src.canvas;
	dest.palette		= (wg_color8*)src.palette;
	dest.paletteSize	= src.paletteSize;
	dest.paletteCapacity= src.paletteCapacity;
	dest.dynamic		= src.dynamic;
	dest.format			= (wg_pixelFormat) src.format;
	dest.identity		= src.identity;
	dest.mipmap			= src.mipmap;
	dest.sampleMethod	= (wg_sampleMethod) src.sampleMethod;
	dest.scale			= src.scale;
	dest.size.w			= src.size.w;
	dest.size.h			= src.size.h;
	dest.tiling			= src.tiling;

	return dest;
}
