/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG_C_SURFACE_DOT_H
#define WG_C_SURFACE_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>
#include <wg_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif


	typedef void(*wg_surfaceObserver_func)(int nRects, const wg_rectI* pRects, void* pData, int data);


	//____ wg_pixelBuffer ________________________________________________________

	typedef struct	wg_pixelBuffer_struct		// NOT BINARY EQUIVALENT!
	{
		wg_pixelFormat	format;
		uint8_t* pixels;
		const wg_color8* palette;
		wg_rectI		rect;
		int				pitch;
	} wg_pixelBuffer;


	//____ wg_ninePatch __________________________________________________________

	typedef struct wg_ninePatch_struct
	{
		wg_rect		block;
		wg_border	frame;

		wg_pts			rigidPartXOfs;
		wg_pts			rigidPartXLength;
		uint8_t			rigidPartXSections;

		wg_pts			rigidPartYOfs;
		wg_pts			rigidPartYLength;
		uint8_t			rigidPartYSections;
	} wg_ninePatch;


	//____ wg_surfaceBP __________________________________________________________

	typedef struct wg_surfaceBP_struct			// NOT BINARY EQUIVALENT!
	{
		uint8_t				buffered;
		uint8_t				canvas;
		const wg_color8* 	palette;
		int					paletteSize;
		int					paletteCapacity;
		uint8_t				dynamic;
		wg_pixelFormat		format;
		int					identity;
		uint8_t				mipmap;
		wg_sampleMethod		sampleMethod;
		int					scale;
		wg_sizeI			size;					// Mandatory, except when creating from other surface.
		uint8_t				tiling;
	} wg_surfaceBP;


	//____ Identification __________________________________________

	void			wg_setSurfaceIdentity(wg_obj surface, int id);
	int				wg_getSurfaceIdentity(wg_obj surface);

	//____ Geometry _________________________________________________

	wg_sizeI		wg_surfacePixelSize(wg_obj surface);						///< @brief Get the size in pixels of the surface.
	int				wg_surfacePixelWidth(wg_obj surface);						///< @brief Get the width of the surface.
	int				wg_surfacePixelHeight(wg_obj surface);						///< @brief Get the height of the surface.

	wg_size			wg_surfacePointSize(wg_obj surface);
	wg_pts			wg_surfacePointWidth(wg_obj surface);
	wg_pts			wg_surfacePointHeight(wg_obj surface);

	int				wg_surfaceScale(wg_obj surface);


	//____ Appearance ____________________________________________________

	wg_sampleMethod	wg_surfaceSampleMethod(wg_obj surface);
	int				wg_surfaceIsTiling(wg_obj surface);
	int				wg_surfaceIsMipmapped(wg_obj surface);

	//____ Content _______________________________________________________

	int				wg_surfaceAlpha(wg_obj surface, wg_coordSPX coord);	///< @brief Get Alpha value of subpixel at specified coordinate.

	const wg_color8* wg_surfacePalette(wg_obj surface);
	int 			wg_surfacePaletteSize(wg_obj surface);
	int 			wg_surfacePaletteCapacity(wg_obj surface);

	const wg_pixelDescription* wg_surfacePixelDescription(wg_obj surface); ///< @brief Get the pixel description for the surface.
	wg_pixelFormat	wg_surfacePixelFormat(wg_obj surface);
	int				wg_surfacePixelBits(wg_obj surface);

	int				wg_surfaceIsOpaque(wg_obj surface);				///< @brief Check if surface is guaranteed to be entirely opaque.
	int				wg_surfaceCanBeCanvas(wg_obj surface);

	//____ Control _______________________________________________________

	wg_pixelBuffer	wg_allocPixelBuffer(wg_obj surface);
	wg_pixelBuffer	wg_allocPixelBufferFromRect(wg_obj surface, const wg_rectI* rect);

	int				wg_pushPixels(wg_obj surface, const wg_pixelBuffer* buffer);
	int				wg_pushPixelsFromRect(wg_obj surface, const wg_pixelBuffer* buffer, const wg_rectI* bufferRect);

	void			wg_pullPixels(wg_obj surface, const wg_pixelBuffer* buffer);
	void			wg_pullPixelsFromRect(wg_obj surface, const wg_pixelBuffer* buffer, const wg_rectI* bufferRect);			// Needs to be overridden!

	void			wg_freePixelBuffer(wg_obj surface, const wg_pixelBuffer* buffer);


	//____  Rendering ____________________________________________________

	int				wg_fillSurface(wg_obj surface, wg_color col);								///< @brief Fill surface with specified color.
	int				wg_fillSurfaceRect(wg_obj surface, const wg_rectI* region, wg_color col);			///< @brief Fill section of surface with specified color
	int				wg_copySurface(wg_obj destSurface, wg_coordI dst, wg_obj sourceSurface);		///< @brief Copy other surface as a block
	int				wg_copySurfaceRect(wg_obj destSurface, wg_coordI dst, wg_obj sourceSurface, const wg_rectI* srcRect);	///< @brief Copy block of graphics from other surface

	//____ Misc _________________________________________________________

	void			wg_setSurfaceBaggage(wg_obj surface, wg_obj baggage);
	wg_obj			wg_getSurfaceBaggage(wg_obj surface);

	int				wg_addSurfaceObserver(wg_obj surface, wg_surfaceObserver_func, void* pData, int data);
	int				wg_removeSurfaceObserver(wg_obj surface, int observerId);

	wg_surfaceBP	wg_getSurfaceBlueprint(wg_obj surface);



#ifdef __cplusplus
}
#endif

#endif
