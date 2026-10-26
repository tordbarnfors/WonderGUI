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
#ifndef WAX_C_SURFACE_DOT_H
#define WAX_C_SURFACE_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>
#include <wax_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef void(*wax_surfaceObserver_func)(int nRects, const wax_rectI* pRects, void* pData, int data);


//____ wax_pixelBuffer ________________________________________________________

typedef struct	wax_pixelBuffer_struct		// NOT BINARY EQUIVALENT!
{
	wax_pixelFormat	format;
	uint8_t* pixels;
	const wax_color8* palette;
	wax_rectI		rect;
	int				pitch;
	wax_colorSpace	colorSpace;
	uint8_t			bigEndian;
} wax_pixelBuffer;


//____ wax_ninePatch __________________________________________________________

typedef struct wax_ninePatch_struct
{
	wax_rect		block;
	wax_border	frame;

	wax_pts			rigidPartXOfs;
	wax_pts			rigidPartXLength;
	uint8_t			rigidPartXSections;

	wax_pts			rigidPartYOfs;
	wax_pts			rigidPartYLength;
	uint8_t			rigidPartYSections;
} wax_ninePatch;


//____ wax_surfaceBP __________________________________________________________

typedef struct wax_surfaceBP_struct			// NOT BINARY EQUIVALENT!
{
	wax_byteOrder		byteOrder;				// Defaults to native byte order.
	uint8_t				buffered;
	uint8_t				canvas;
	wax_colorSpace		colorSpace;				// Defaults to sRGB.
	const wax_color8* 	palette;
	int					paletteSize;
	int					paletteCapacity;
	uint8_t				dynamic;
	wax_pixelFormat		format;
	int					identity;
	uint8_t				mipmap;
	wax_sampleMethod		sampleMethod;
	int					scale;
	wax_sizeI			size;					// Mandatory, except when creating from other surface.
	uint8_t				tiling;
} wax_surfaceBP;


//____ Identification __________________________________________

WAX_EXPORT void			wax_setSurfaceIdentity(wax_obj surface, int id);
WAX_EXPORT int			wax_getSurfaceIdentity(wax_obj surface);

//____ Geometry _________________________________________________

WAX_EXPORT wax_sizeI		wax_surfacePixelSize(wax_obj surface);						///< @brief Get the size in pixels of the surface.
WAX_EXPORT int			wax_surfacePixelWidth(wax_obj surface);						///< @brief Get the width of the surface.
WAX_EXPORT int			wax_surfacePixelHeight(wax_obj surface);						///< @brief Get the height of the surface.

WAX_EXPORT wax_size		wax_surfacePointSize(wax_obj surface);
WAX_EXPORT wax_pts		wax_surfacePointWidth(wax_obj surface);
WAX_EXPORT wax_pts		wax_surfacePointHeight(wax_obj surface);

WAX_EXPORT int			wax_surfaceScale(wax_obj surface);


//____ Appearance ____________________________________________________

WAX_EXPORT wax_sampleMethod	wax_surfaceSampleMethod(wax_obj surface);
WAX_EXPORT int				wax_surfaceIsTiling(wax_obj surface);
WAX_EXPORT int				wax_surfaceIsMipmapped(wax_obj surface);

//____ Content _______________________________________________________

WAX_EXPORT int				wax_surfaceAlpha(wax_obj surface, wax_coordSPX coord);	///< @brief Get Alpha value of subpixel at specified coordinate.

WAX_EXPORT const wax_color8* wax_surfacePalette(wax_obj surface);
WAX_EXPORT int 				wax_surfacePaletteSize(wax_obj surface);
WAX_EXPORT int 				wax_surfacePaletteCapacity(wax_obj surface);

WAX_EXPORT const wax_pixelDescription* wax_surfacePixelDescription(wax_obj surface); ///< @brief Get the pixel description for the surface.
WAX_EXPORT wax_pixelFormat	wax_surfacePixelFormat(wax_obj surface);
WAX_EXPORT int				wax_surfacePixelBits(wax_obj surface);
WAX_EXPORT wax_colorSpace		wax_surfaceColorSpace(wax_obj surface);
WAX_EXPORT int				wax_surfaceIsBigEndian(wax_obj surface);

WAX_EXPORT int				wax_surfaceIsOpaque(wax_obj surface);				///< @brief Check if surface is guaranteed to be entirely opaque.
WAX_EXPORT int				wax_surfaceCanBeCanvas(wax_obj surface);

//____ Control _______________________________________________________

WAX_EXPORT wax_pixelBuffer	wax_allocPixelBuffer(wax_obj surface);
WAX_EXPORT wax_pixelBuffer	wax_allocPixelBufferFromRect(wax_obj surface, const wax_rectI* rect);

WAX_EXPORT int				wax_pushPixels(wax_obj surface, const wax_pixelBuffer* buffer);
WAX_EXPORT int				wax_pushPixelsFromRect(wax_obj surface, const wax_pixelBuffer* buffer, const wax_rectI* bufferRect);

WAX_EXPORT void				wax_pullPixels(wax_obj surface, const wax_pixelBuffer* buffer);
WAX_EXPORT void				wax_pullPixelsFromRect(wax_obj surface, const wax_pixelBuffer* buffer, const wax_rectI* bufferRect);			// Needs to be overridden!
	
WAX_EXPORT void				wax_freePixelBuffer(wax_obj surface, const wax_pixelBuffer* buffer);


//____  Rendering ____________________________________________________

WAX_EXPORT int				wax_fillSurface(wax_obj surface, wax_color col);								///< @brief Fill surface with specified color.
WAX_EXPORT int				wax_fillSurfaceRect(wax_obj surface, const wax_rectI* region, wax_color col);			///< @brief Fill section of surface with specified color
WAX_EXPORT int				wax_copySurface(wax_obj destSurface, wax_coordI dst, wax_obj sourceSurface);		///< @brief Copy other surface as a block
WAX_EXPORT int				wax_copySurfaceRect(wax_obj destSurface, wax_coordI dst, wax_obj sourceSurface, const wax_rectI* srcRect);	///< @brief Copy block of graphics from other surface

//____ Misc _________________________________________________________

WAX_EXPORT void				wax_setSurfaceBaggage(wax_obj surface, wax_obj baggage);
WAX_EXPORT wax_obj			wax_getSurfaceBaggage(wax_obj surface);

WAX_EXPORT int				wax_addSurfaceObserver(wax_obj surface, wax_surfaceObserver_func, void* pData, int data);
WAX_EXPORT int				wax_removeSurfaceObserver(wax_obj surface, int observerId);

WAX_EXPORT wax_surfaceBP		wax_getSurfaceBlueprint(wax_obj surface);



#ifdef __cplusplus
}
#endif

#endif
