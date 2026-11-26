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
#ifndef WAX_C_BITMAP_DOT_H
#define WAX_C_BITMAP_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>
#include <wax_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef void(*wax_bitmapObserver_func)(int nRects, const wax_rectI* pRects, void* pData, int data);


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


//____ wax_bitmapBP __________________________________________________________

typedef struct wax_bitmapBP_struct			// NOT BINARY EQUIVALENT!
{
	wax_byteOrder		byteOrder;				// Defaults to native byte order.
	uint8_t				canvas;
	wax_colorSpace		colorSpace;				// Defaults to sRGB.
	uint8_t				useFillColor;			// Set to have a bitmap created without pixels filled with fillColor. Its content is unspecified otherwise.
	wax_color			fillColor;
	const wax_color8* 	palette;
	int					paletteSize;
	int					paletteCapacity;
	uint8_t				dynamic;
	wax_pixelFormat		format;
	int					identity;
	uint8_t				mipmap;
	wax_restorer_func	restorer;				// Called to put the content back when the texture is lost, see Bitmap::Blueprint. Null to have the bitmap keep its pixels.
	void *				restorerData;			// Passed to restorer. Has to be valid for as long as the bitmap is, or until wax_dropBitmapRestorer().
	wax_sampleMethod		sampleMethod;
	int					scale;
	wax_sizeI			size;					// Mandatory, except when creating from other bitmap.
	uint8_t				tiling;
} wax_bitmapBP;


//____ Identification __________________________________________

WAX_EXPORT void			wax_setBitmapIdentity(wax_obj bitmap, int id);
WAX_EXPORT int			wax_getBitmapIdentity(wax_obj bitmap);

//____ Geometry _________________________________________________

WAX_EXPORT wax_sizeI		wax_bitmapPixelSize(wax_obj bitmap);						///< @brief Get the size in pixels of the bitmap.
WAX_EXPORT int			wax_bitmapPixelWidth(wax_obj bitmap);						///< @brief Get the width of the bitmap.
WAX_EXPORT int			wax_bitmapPixelHeight(wax_obj bitmap);						///< @brief Get the height of the bitmap.

WAX_EXPORT wax_size		wax_bitmapPointSize(wax_obj bitmap);
WAX_EXPORT wax_pts		wax_bitmapPointWidth(wax_obj bitmap);
WAX_EXPORT wax_pts		wax_bitmapPointHeight(wax_obj bitmap);

WAX_EXPORT int			wax_bitmapScale(wax_obj bitmap);


//____ Appearance ____________________________________________________

WAX_EXPORT wax_sampleMethod	wax_bitmapSampleMethod(wax_obj bitmap);
WAX_EXPORT int				wax_bitmapIsTiling(wax_obj bitmap);
WAX_EXPORT int				wax_bitmapIsMipmapped(wax_obj bitmap);

//____ Content _______________________________________________________

WAX_EXPORT int				wax_bitmapAlpha(wax_obj bitmap, wax_coordSPX coord);	///< @brief Get Alpha value of subpixel at specified coordinate.

WAX_EXPORT const wax_color8* wax_bitmapPalette(wax_obj bitmap);
WAX_EXPORT int 				wax_bitmapPaletteSize(wax_obj bitmap);
WAX_EXPORT int 				wax_bitmapPaletteCapacity(wax_obj bitmap);

WAX_EXPORT const wax_pixelDescription* wax_bitmapPixelDescription(wax_obj bitmap); ///< @brief Get the pixel description for the bitmap.
WAX_EXPORT wax_pixelFormat	wax_bitmapPixelFormat(wax_obj bitmap);
WAX_EXPORT int				wax_bitmapPixelBits(wax_obj bitmap);
WAX_EXPORT wax_colorSpace		wax_bitmapColorSpace(wax_obj bitmap);
WAX_EXPORT int				wax_bitmapIsBigEndian(wax_obj bitmap);

WAX_EXPORT int				wax_bitmapIsOpaque(wax_obj bitmap);				///< @brief Check if bitmap is guaranteed to be entirely opaque.
WAX_EXPORT int				wax_bitmapCanBeCanvas(wax_obj bitmap);

WAX_EXPORT int				wax_bitmapIsContentLost(wax_obj bitmap);			///< @brief Check if the bitmap lost its content with the device of a GPU backend.
WAX_EXPORT uint8_t			wax_declineBitmapRestore(wax_obj bitmap, void * pData);	///< @brief A restorer that does nothing, for wax_bitmapBP.
WAX_EXPORT void				wax_dropBitmapRestorer(wax_obj bitmap);			///< @brief Have a bitmap with a restorer decline from now on, for when the restorer or its data is going away.

//____ Control _______________________________________________________

WAX_EXPORT wax_pixelBuffer	wax_allocPixelBuffer(wax_obj bitmap);
WAX_EXPORT wax_pixelBuffer	wax_allocPixelBufferFromRect(wax_obj bitmap, const wax_rectI* rect);

WAX_EXPORT int				wax_pushPixels(wax_obj bitmap, const wax_pixelBuffer* buffer);
WAX_EXPORT int				wax_pushPixelsFromRect(wax_obj bitmap, const wax_pixelBuffer* buffer, const wax_rectI* bufferRect);

WAX_EXPORT void				wax_pullPixels(wax_obj bitmap, const wax_pixelBuffer* buffer);
WAX_EXPORT void				wax_pullPixelsFromRect(wax_obj bitmap, const wax_pixelBuffer* buffer, const wax_rectI* bufferRect);			// Needs to be overridden!
	
WAX_EXPORT void				wax_freePixelBuffer(wax_obj bitmap, const wax_pixelBuffer* buffer);


//____  Rendering ____________________________________________________

WAX_EXPORT int				wax_fillBitmap(wax_obj bitmap, wax_color col);								///< @brief Fill bitmap with specified color.
WAX_EXPORT int				wax_fillBitmapRect(wax_obj bitmap, const wax_rectI* region, wax_color col);			///< @brief Fill section of bitmap with specified color
WAX_EXPORT int				wax_copyBitmap(wax_obj destBitmap, wax_coordI dst, wax_obj sourceBitmap);		///< @brief Copy other bitmap as a block
WAX_EXPORT int				wax_copyBitmapRect(wax_obj destBitmap, wax_coordI dst, wax_obj sourceBitmap, const wax_rectI* srcRect);	///< @brief Copy block of graphics from other bitmap

//____ Misc _________________________________________________________

WAX_EXPORT void				wax_setBitmapUserData(wax_obj bitmap, wax_obj userData);
WAX_EXPORT wax_obj			wax_getBitmapUserData(wax_obj bitmap);

WAX_EXPORT int				wax_addBitmapObserver(wax_obj bitmap, wax_bitmapObserver_func, void* pData, int data);
WAX_EXPORT int				wax_removeBitmapObserver(wax_obj bitmap, int observerId);

WAX_EXPORT wax_bitmapBP		wax_getBitmapBlueprint(wax_obj bitmap);			///< @brief Get a blueprint of the bitmap. It is without the restorer, which can't be given as a function of C.



#ifdef __cplusplus
}
#endif

#endif
