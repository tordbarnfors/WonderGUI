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
#ifndef WG_PLUGININTERFACE_DOT_H
#define WG_PLUGININTERFACE_DOT_H
#pragma once

#include <wg_c_types.h>
#include <wax_c_geo.h>
#include <wax_c_color.h>
#include <wax_c_tint.h>


#include <wax_c_bitmapcache.h>
#include <wax_c_canvaslayers.h>
#include <wax_c_font.h>
#include <wax_c_device.h>
#include <wax_c_edgemap.h>
#include <wax_c_warpmap.h>
#include <wax_c_blurbrush.h>


#ifdef __cplusplus
extern "C" {
#endif

//____ Compatibility __________________________________________________________
//
// Hosts and plugins are updated on their own schedules, so a plugin should keep
// working with any host that has at least the calls it was built with. Each
// struct starts with its size, and a plugin refuses a host whose structs are
// smaller than its own (see PluginCalls::_init()).
//
// So new calls are only ever added at the end of their struct, never inserted
// or reordered, and existing calls keep their signatures. A call whose feature
// is removed keeps its slot, marked RETIRED, and the host fills it with a stub
// that does nothing and returns 0 or null. A change that can't be done that way
// breaks every plugin. Versioning those is up to the application hosting the
// plugins, which checks a version of its own, and they are a good time to clear
// out retired slots.

//____ wg_pluginroot_calls ____________________________________________________

typedef struct wg_pluginroot_calls_struct
{
		int			structSize;

		wax_spx			(*matchingHeight)(void* pPluginRoot, wax_spx width, int scale);
		wax_spx			(*matchingWidth)(void* pPluginRoot, wax_spx height, int scale);

		wax_sizeSPX		(*defaultSize)(void* pPluginRoot, int scale);
		wax_sizeSPX		(*minSize)(void* pPluginRoot, int scale);
		wax_sizeSPX		(*maxSize)(void* pPluginRoot, int scale);

		int				(*markTest)(void* pPluginRoot, wax_coordSPX ofs);

		void    		(*preRender)(void* pPluginRoot);
		void			(*render)(void* pPluginRoot, wax_obj pDevice, wax_rectSPX canvas, wax_rectSPX window);

		void			(*resize)(void* pPluginRoot, wax_sizeSPX size, int scale);
		void			(*setState)(void* pPluginRoot, wg_state state);
		void			(*receive)(void* pPluginRoot, wax_obj msg);

//		wg_string		(*tooltip)(void* pPluginRoot);
		wg_pointerStyle	(*pointerStyle)(void* pPluginRoot);

		void			(*setPointerPos)(void* pPluginRoot, wax_coordSPX pos, int64_t timestamp );
		void			(*setButtonState)(void* pPluginRoot, int button, int pressed, int64_t timestamp );
		void			(*setKeyState)(void* pPluginRoot, int nativeKeycode, int pressed, int64_t timestamp );
		void			(*putText)(void* pPluginRoot, const char * pUTF8String );

		void			(*onUpdate)(void* pPluginRoot, int microPassed, int64_t microsecTimestamp);

		void 			(*wheelRoll)(void* pPluginRoot, int wheel, wax_pts distance, int bInvert, int64_t timestamp );

} wg_pluginroot_calls;


//____ wg_hostbridge_calls _____________________________________________

typedef struct wg_hostbridge_calls_struct
{
	int					structSize;

	int					(*hidePointer)( void * pHostBridge );
	int					(*showPointer)( void * pHostBridge );

	int					(*getClipboardText)( void * pHostBridge, int maxChars, char * pDest );
	int					(*setClipboardText)( void * pHostBridge, const char * pText );
	int					(*requestWindowFocus)( void * pHostBridge, uintptr_t root );
	int					(*yieldWindowFocus)(void* pHostBridge, uintptr_t root);

	int					(*lockHidePointer)(void* pHostBridge);
	int					(*unlockShowPointer)(void* pHostBridge);

	int					(*setPointerStyle)(void* pHostBridge, wg_pointerStyle style );

} wg_hostbridge_calls;

//____ wg_bitmapcache_calls ___________________________________________________

typedef struct wg_bitmapcache_calls_struct
{
	int					structSize;

	wax_obj				(*createBitmapCache)( int maxSize, wax_obj surfaceFactory );
	void				(*setCacheLimit)(wax_obj bitmapCache, int maxBytes);
	int                 (*cacheLimit)(wax_obj bitmapCache);
	int                 (*cacheSize)(wax_obj bitmapCache);
	void                (*clearCache)(wax_obj bitmapCache);
	wax_cacheListener    (*addCacheListener)(wax_obj bitmapCache, wax_cacheListenerBP blueprint );
	void                (*removeCacheListener)(wax_obj bitmapCache, wax_cacheListener listener );
	wax_cacheSlot        (*getCacheSlot)(wax_obj bitmapCache, wax_sizeI size);
	int                 (*getNbCacheSurfaces)(wax_obj bitmapCache);
	int                 (*getCacheSurfaces)(wax_obj bitmapCache, int maxSurfaces, wax_obj * pArray );

} wg_bitmapcache_calls;


//____ wg_bitmapfont_calls ____________________________________________________

typedef struct wg_bitmapfont_calls_struct
{
	int					structSize;

	wax_obj				(*createBitmapFont)(wax_obj surface, char* pGlyphSpec, wax_obj backupFont);
	wax_obj				(*getBitmapFontSurface)(wax_obj bitmapFont);

} wg_bitmapfont_calls;


//____ wg_canvaslayers_calls __________________________________________________

typedef struct wg_canvaslayers_calls_struct
{
	int					structSize;

	wax_obj				(*createCanvasLayers)(const wax_canvasLayersBP* pBlueprint);
	int					(*canvasLayersSize)(wax_obj canvasLayers);
	wax_pixelFormat		(*layerFormat)(wax_obj canvasLayers, int layer);
	int					(*baseLayer)(wax_obj canvasLayers);

} wg_canvaslayers_calls;


//____ wg_font_calls __________________________________________________________

typedef struct wg_font_calls_struct
{
	int					structSize;

	int					(*setFontSize)(wax_obj font, wax_spx size);
	wax_spx				(*fontSize)(wax_obj font);
	void				(*getGlyphWithoutBitmap)(wax_obj font, uint32_t chr, wax_glyph* pGlyph);
	void				(*getGlyphWithBitmap)(wax_obj font, uint32_t chr, wax_glyph* pGlyph);
	wax_spx				(*getKerning)(wax_obj font, wax_glyph* pLeftGlyph, wax_glyph* p_rightGlyph);
	wax_spx				(*lineGap)(wax_obj font);
	wax_spx				(*whitespaceAdvance)(wax_obj font);
	wax_spx				(*maxAdvance)(wax_obj font);
	wax_spx				(*maxAscend)(wax_obj font);
	wax_spx 				(*maxDescend)(wax_obj font);
	int					(*nbGlyphs)(wax_obj font);
	int					(*hasGlyphs)(wax_obj font);
	int					(*hasGlyph)(wax_obj font, uint32_t chr);
	int					(*isMonospace)(wax_obj font);
	int					(*isMonochrome)(wax_obj font);
	wax_obj				(*getBackupFont)(wax_obj font);
	wax_obj				(*createSurfaceFromGlyph)(wax_obj font, uint32_t chr, const wax_surfaceBP* blueprint, wax_obj surfaceFactory);

} wg_font_calls;


//____ wg_gfxdevice_calls _____________________________________________________

typedef struct wg_gfxdevice_calls_struct
{
	int					structSize;

	const wax_typeInfo*	(*deviceSurfaceType)(wax_obj device);
	wax_canvasInfo		(*getCanvas)(wax_obj device);
	wax_canvasInfo		(*getCanvasRef)(wax_obj device, wax_canvasRef ref);
	wax_obj				(*canvasLayers)(wax_obj device);
	wax_obj				(*surfaceFactory)(wax_obj device);
	wax_obj				(*edgemapFactory)(wax_obj device);
	int					(*maxSegments)();
	wax_sizeSPX			(*canvasSize)(wax_obj device);
	int					(*setClipList)(wax_obj device, int nRectangles, const wax_rectSPX* pRectangles);
	void				(*resetClipList)(wax_obj device);
	int					(*pushClipList)(wax_obj device, int nRectangles, const wax_rectSPX* pRectangles);
	int					(*popClipList)(wax_obj device);
	const wax_rectSPX*	(*getClipList)(wax_obj device);
	int					(*clipListSize)(wax_obj device);
	const wax_rectSPX*	(*clipBounds)(wax_obj device);
	void				(*setTintColor)(wax_obj device, wax_color color);
	wax_color			(*getTintColor)(wax_obj device);

	// RETIRED: Tintmaps and tint gradients were replaced by Tints, see setTint() below.

	void				(*setTintmap)(wax_obj device, const wax_rectSPX* rect, const wax_obj tintmap);
	wax_obj				(*getTintmap)(wax_obj device);
	wax_rectSPX			(*getTintmapRect)(wax_obj device);
	void				(*clearTintmap)(wax_obj device);
	int					(*hasTintmap)(wax_obj device);
	void				(*setTintGradient)(wax_obj device, const wax_rectSPX* rect, const void* gradient);
	void				(*clearTintGradient)(wax_obj device);

	int					(*setBlendMode)(wax_obj device, wax_blendMode blendMode);
	wax_blendMode 		(*getBlendMode)(wax_obj device);
	int					(*setBlitSource)(wax_obj device, wax_obj surface);
	wax_obj 				(*getBlitSource)(wax_obj device);
	void				(*setMorphFactor)(wax_obj device, float factor);
	float				(*getMorphFactor)(wax_obj device);
	void				(*setRenderLayer)(wax_obj device, int layer);
	int					(*getRenderLayer)(wax_obj device);
	int					(*beginRender)(wax_obj device);
	int					(*endRender)(wax_obj device);
	int					(*isDeviceRendering)(wax_obj device);
	int					(*isDeviceIdle)(wax_obj device);
	void				(*flushDevice)(wax_obj device);
	int					(*beginCanvasUpdateWithRef)(wax_obj device, wax_canvasRef canvas, int nUpdateRects, const wax_rectSPX* pUpdateRects, wax_obj canvasLayers, int startLayer);
	int					(*beginCanvasUpdateWithSurface)(wax_obj device, wax_obj surface, int nUpdateRects, const wax_rectSPX* pUpdateRects, wax_obj canvasLayers, int startLayer);
	void				(*endCanvasUpdate)(wax_obj device);
	void				(*fill)(wax_obj device, wax_color col);
	void				(*fillRect)(wax_obj device, const wax_rectSPX* rect, wax_color col);
	void				(*plotPixels)(wax_obj device, int nCoords, const wax_coordSPX* pCoords, const wax_color* pColors);
	void				(*drawLine)(wax_obj device, wax_coordSPX begin, wax_coordSPX end, wax_color color, wax_spx thickness);
	void				(*drawStraightLine)(wax_obj device, wax_coordSPX begin, wax_direction dir, wax_spx length, wax_color color, wax_spx thickness);
	void				(*blit)(wax_obj device, wax_coordSPX dest);
	void				(*blitRect)(wax_obj device, wax_coordSPX dest, const wax_rectSPX* src);
	void				(*flipBlit)(wax_obj device, wax_coordSPX dest, wax_gfxFlip flip);
	void				(*flipBlitRect)(wax_obj device, wax_coordSPX dest, const wax_rectSPX* src, wax_gfxFlip flip);
	void				(*stretchBlit)(wax_obj device, const wax_rectSPX* dest);
	void				(*stretchBlitRect)(wax_obj device, const wax_rectSPX* dest, const wax_rectSPX* src);
	void				(*stretchFlipBlit)(wax_obj device, const wax_rectSPX* dest, wax_gfxFlip flip);
	void				(*stretchFlipBlitRect)(wax_obj device, const wax_rectSPX* dest, const wax_rectSPX* src, wax_gfxFlip flip);
	void				(*precisionBlit)(wax_obj device, const wax_rectSPX* dest, const wax_rectF* srcSPX);
	void				(*transformBlit)(wax_obj device, const wax_rectSPX* dest, wax_coordF srcSPX, const wax_transform * pTransform);
	void				(*rotScaleBlit)(wax_obj device, const wax_rectSPX* dest, float rotationDegrees, float scale, wax_coordF srcCenter, wax_coordF destCenter);
	void				(*tile)(wax_obj device, const wax_rectSPX* dest, wax_coordSPX shift);
	void				(*flipTile)(wax_obj device, const wax_rectSPX* dest, wax_gfxFlip flip, wax_coordSPX shift);
	void				(*scaleTile)(wax_obj device, const wax_rectSPX* dest, float scale, wax_coordSPX shift);
	void				(*scaleFlipTile)(wax_obj device, const wax_rectSPX* dest, float scale, wax_gfxFlip flip, wax_coordSPX shift);
	void				(*drawWave)(wax_obj device, const wax_rectSPX* dest, const wax_waveLine* pTopBorder, const wax_waveLine* pBottomBorder, wax_color fill);
	void				(*flipDrawWave)(wax_obj device, const wax_rectSPX* dest, const wax_waveLine* pTopBorder, const wax_waveLine* pBottomBorder, wax_color fill, wax_gfxFlip flip);
	void				(*drawElipse)(wax_obj device, const wax_rectSPX* canvas, wax_spx thickness, wax_color color, wax_spx outlineThickness, wax_color outlineColor);
	void				(*drawPieChart)(wax_obj device, const wax_rectSPX* canvas, float start, int nSlices, const float* pSliceSizes, const wax_color* pSliceColors, float hubSize, wax_color hubColor, wax_color backColor, int bRectangular);
	void				(*drawSegments)(wax_obj device, const wax_rectSPX* dest, int nSegments, const wax_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wax_tintMode tintMode);
	void				(*flipDrawSegments)(wax_obj device, const wax_rectSPX* dest, int nSegments, const wax_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wax_gfxFlip flip, wax_tintMode tintMode);
	void				(*drawEdgemap)(wax_obj device, wax_coordSPX dest, wax_obj edgemap);
	void				(*flipDrawEdgemap)(wax_obj device, wax_coordSPX dest, wax_obj edgemap, wax_gfxFlip flip);
	void				(*blitNinePatch)(wax_obj device, const wax_rectSPX* dstRect, const wax_borderSPX* dstFrame, const wax_ninePatch* patch, int scale);

	void				(*setBlurMatrices)(wax_obj device, wax_spx radius, const float red[9], const float green[9], const float blue[9] );
	void				(*setFixedBlendColor)(wax_obj device, wax_color color );
	wax_color			(*getFixedBlendColor)(wax_obj device);
	
	void				(*blur)(wax_obj device, wax_coordSPX dest);
	void				(*blurRect)(wax_obj device, wax_coordSPX dest, const wax_rectSPX* src);
	void				(*stretchBlur)(wax_obj device, const wax_rectSPX* dest);
	void				(*stretchBlurRect)(wax_obj device, const wax_rectSPX* dest, const wax_rectSPX* src);
	void				(*transformBlur)(wax_obj device, const wax_rectSPX* dest, wax_coordF srcSPX, const wax_transform * pTransform);
	void				(*rotScaleBlur)(wax_obj device, const wax_rectSPX* dest, float rotationDegrees, float scale, wax_coordF srcCenter, wax_coordF destCenter);

	// New since first revision to interface

	void				(*setBlurbrush)(wax_obj device, wax_obj brush );

	void				(*setTint)(wax_obj device, const wax_rectSPX* rect, const wax_obj tint);
	wax_obj				(*getTint)(wax_obj device);
	wax_rectSPX			(*getTintRect)(wax_obj device);
	void				(*clearTint)(wax_obj device);
	int					(*hasTint)(wax_obj device);
	void				(*clearTintColor)(wax_obj device);
	int					(*hasTintColor)(wax_obj device);

	void				(*tileEdgemap)(wax_obj device, const wax_rectSPX* dest, wax_obj edgemap, wax_coordSPX shift);
	void				(*flipTileEdgemap)(wax_obj device, const wax_rectSPX* dest, wax_obj edgemap, wax_gfxFlip flip, wax_coordSPX shift);

	void				(*drawRoundedBox)(wax_obj device, const wax_roundedBoxSpec* spec);
	void				(*drawRing)(wax_obj device, const wax_ringSpec* spec);

	wax_obj				(*warpmapFactory)(wax_obj device);
	void				(*warpBlit)(wax_obj device, wax_coordSPX dest, wax_obj warpmap, wax_coordSPX sourceOffset);
	void				(*warpBlitRect)(wax_obj device, wax_coordSPX dest, wax_obj warpmap, const wax_rectSPX* source, wax_coordSPX sourceOffset);
	void				(*warpDrawEdgemap)(wax_obj device, wax_coordSPX dest, wax_obj warpmap, wax_obj edgemap, wax_coordSPX sourceOffset);

	void				(*drawLineWithCaps)(wax_obj device, wax_coordSPX begin, wax_coordSPX end, wax_color color, wax_spx thickness, wax_lineCap beginCap, wax_lineCap endCap);
	void				(*drawStraightLineWithCaps)(wax_obj device, wax_coordSPX begin, wax_direction dir, wax_spx length, wax_color color, wax_spx thickness, wax_lineCap beginCap, wax_lineCap endCap);

} wg_gfxdevice_calls;

/*
//____ wg_streambuffer_calls __________________________________________________

typedef struct wg_streambuffer_calls_struct
{
	int					structSize;

	wax_obj				(*createStreamBuffer)(int initialCapacity);
	wax_component		(*getStreamBufferOutput)(wax_obj streamBuffer);
	wax_component		(*getStreamBufferInput)(wax_obj streamBuffer);
	int					(*streamBufferCapacity)(wax_obj streamBuffer);
	int					(*streamBufferHasChunk)(wax_obj streamBuffer);
	int					(*streamBufferBytes)(wax_obj streamBuffer);

} wg_streambuffer_calls;


//____ wg_streamplayer_calls __________________________________________________

typedef struct wg_streamplayer_calls_struct
{
	int					structSize;

	wax_obj				(*createStreamPlayer)(wax_obj device, wax_obj surfaceFactory, wax_obj edgemapFactory);
	wax_component		(*getStreamPlayerInput)(wax_obj streamPlayer);
	void				(*setStreamPlayerStoreDirtyRects)(wax_obj streamPlayer, int bStore);
	void				(*setStreamPlayerMaxDirtyRects)(wax_obj streamPlayer, int max);
	int					(*getStreamPlayerDirtyRects)(wax_obj streamPlayer, wax_canvasRef canvas, const wax_rectI** pRects);
	void				(*clearStreamPlayerDirtyRects)(wax_obj streamPlayer);

} wg_streamplayer_calls;


//____ wg_streampump_calls ____________________________________________________

typedef struct wg_streampump_calls_struct
{
	int					structSize;

	wax_obj				(*createStreamPump)();
	wax_obj				(*createStreamPumpWithInputOutput)(wax_component input, wax_component output);
	void				(*setStreamPumpInput)(wax_obj streamPump, wax_component input);
	void				(*setStreamPumpOutput)(wax_obj streamPump, wax_component output);
	wax_streamChunkId		(*peekChunk)(wax_obj streamPump);
	int					(*pumpChunk)(wax_obj streamPump);
	int					(*pumpUntilFrame)(wax_obj streamPump);
	int					(*pumpFrame)(wax_obj streamPump);
	int					(*pumpAll)(wax_obj streamPump);

} wg_streampump_calls;


//____ wg_streamreader_calls __________________________________________________

typedef struct wg_streamreader_calls_struct
{
	int					structSize;

	wax_obj				(*createStreamReader)(wax_readStream_func callback);
	wax_component		(*getStreamReaderOutput)(wax_obj streamReader);
	int					(*streamReaderCapacity)(wax_obj streamReader);
	int					(*streamReaderHasChunk)(wax_obj streamReader);
	int					(*streamReaderBytes)(wax_obj streamReader);

} wg_streamreader_calls;
*/


//____ wg_object_calls ________________________________________________________

typedef struct wg_object_calls_struct
{
	int					structSize;

	const wax_typeInfo*	(*getTypeInfo)(wax_obj);
	int					(*isInstanceOf)(wax_obj, const wax_typeInfo* pTypeInfo);
	void				(*setFinalizer)(wax_obj, wax_finalizer_func pFinalizer);
	wax_finalizer_func	(*finalizer)(wax_obj);
	void				(*retain)(wax_obj);
	void				(*release)(wax_obj);
	int					(*refcount)(wax_obj);

} wg_object_calls;


//____ wg_surface_calls _______________________________________________________

typedef struct wg_surface_calls_struct
{
	int					structSize;

	void				(*setSurfaceIdentity)(wax_obj surface, int id);
	int					(*getSurfaceIdentity)(wax_obj surface);
	wax_sizeI			(*surfacePixelSize)(wax_obj surface);
	int					(*surfacePixelWidth)(wax_obj surface);
	int					(*surfacePixelHeight)(wax_obj surface);
	wax_size				(*surfacePointSize)(wax_obj surface);
	wax_pts				(*surfacePointWidth)(wax_obj surface);
	wax_pts				(*surfacePointHeight)(wax_obj surface);
	int					(*surfaceScale)(wax_obj surface);
	wax_sampleMethod		(*surfaceSampleMethod)(wax_obj surface);
	int					(*surfaceIsTiling)(wax_obj surface);
	int					(*surfaceIsMipmapped)(wax_obj surface);
	int					(*surfaceAlpha)(wax_obj surface, wax_coordSPX coord);
	const wax_color8*	(*surfacePalette)(wax_obj surface);
	int					(*surfacePaletteSize)(wax_obj surface);
	int					(*surfacePaletteCapacity)(wax_obj surface);
	const wax_pixelDescription* (*surfacePixelDescription)(wax_obj surface);
	wax_pixelFormat		(*surfacePixelFormat)(wax_obj surface);
	int					(*surfacePixelBits)(wax_obj surface);
	int					(*surfaceIsOpaque)(wax_obj surface);
	int					(*surfaceCanBeCanvas)(wax_obj surface);
	wax_pixelBuffer		(*allocPixelBuffer)(wax_obj surface);
	wax_pixelBuffer		(*allocPixelBufferFromRect)(wax_obj surface, const wax_rectI* rect);
	int					(*pushPixels)(wax_obj surface, const wax_pixelBuffer* buffer);
	int					(*pushPixelsFromRect)(wax_obj surface, const wax_pixelBuffer* buffer, const wax_rectI* bufferRect);
	void				(*pullPixels)(wax_obj surface, const wax_pixelBuffer* buffer);
	void				(*pullPixelsFromRect)(wax_obj surface, const wax_pixelBuffer* buffer, const wax_rectI* bufferRect);
	void				(*freePixelBuffer)(wax_obj surface, const wax_pixelBuffer* buffer);
	int					(*fillSurface)(wax_obj surface, wax_color col);
	int					(*fillSurfaceRect)(wax_obj surface, const wax_rectI* region, wax_color col);
	int					(*copySurface)(wax_obj destSurface, wax_coordI dst, wax_obj sourceSurface);
	int					(*copySurfaceRect)(wax_obj destSurface, wax_coordI dst, wax_obj sourceSurface, const wax_rectI* srcRect);
	void				(*setSurfaceBaggage)(wax_obj surface, wax_obj baggage);
	wax_obj				(*getSurfaceBaggage)(wax_obj surface);
	int					(*addSurfaceObserver)(wax_obj surface, wax_surfaceObserver_func, void* pData, int data);
	int					(*removeSurfaceObserver)(wax_obj surface, int observerId);
	wax_surfaceBP		(*getSurfaceBlueprint)(wax_obj surface);
	wax_colorSpace		(*surfaceColorSpace)(wax_obj surface);
	int					(*surfaceIsBigEndian)(wax_obj surface);

} wg_surface_calls;


//____ wg_surfacefactory_calls ________________________________________________

typedef struct wg_surfacefactory_calls_struct
{
	int					structSize;

	wax_sizeI			(*maxSurfaceSize)(wax_obj surfaceFactory);
	wax_obj				(*createSurface)(wax_obj factory, const wax_surfaceBP* blueprint);
	wax_obj				(*createSurfaceFromBlob)(wax_obj factory, const wax_surfaceBP* blueprint, wax_obj blob, int pitch);
	wax_obj				(*createSurfaceFromBitmap)(wax_obj factory, const wax_surfaceBP* blueprint, const uint8_t* pPixels, wax_pixelFormat format,  int pitch, const wax_color8 * pPalette, int paletteSize);
	wax_obj				(*createSurfaceFromRawData)(wax_obj factory, const wax_surfaceBP* blueprint, const uint8_t* pPixels, const wax_pixelDescription* pPixelDescription, int pitch, const wax_color8* pPalette, int paletteSize);

} wg_surfacefactory_calls;


//____ wg_edgemap_calls ______________________________________________________

typedef struct wg_edgemap_calls_struct
{
	int					structSize;

	wax_sizeI			(*edgemapPixelSize)(wax_obj edgemap);
	int					(*setRenderSegments)(wax_obj edgemap, int nSegments);
	int					(*getRenderSegments)(wax_obj edgemap);

	int					(*edgemapPaletteType)(wax_obj edgemap);		// RETIRED, there are no palette types anymore.
	int					(*setEdgemapColors)(wax_obj edgemap, int begin, int end, const wax_color * pColors);
	int					(*setEdgemapColorsFromGradients)(wax_obj edgemap, int begin, int end, const void * pGradients );		// RETIRED
	int					(*setEdgemapColorsFromTintmaps)(wax_obj edgemap, int begin, int end, wax_obj * pTintmaps );			// RETIRED
	int					(*setEdgemapColorsFromStrips)(wax_obj edgemap, int begin, int end, const wax_color * pColorstripX, const wax_color * pColorstripY );	// RETIRED

	int					(*importEdgemapPaletteEntries)(wax_obj edgemap, int begin, int end, const wax_color * pColors );		// RETIRED

	const wax_color *  	(*edgemapFlatColors)(wax_obj edgemap);
	const wax_color *  	(*edgemapColorstripsX)(wax_obj edgemap);		// RETIRED
	const wax_color *  	(*edgemapColorstripsY)(wax_obj edgemap);		// RETIRED

	int					(*edgemapSegments)(wax_obj edgemap);
	int					(*edgemapSamples)(wax_obj edgemap);
	int 				(*importSpxSamples)(wax_obj edgemap, wax_sampleOrigo origo, const wax_spx* pSource, int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);
	int 				(*importFloatSamples)(wax_obj edgemap, wax_sampleOrigo origo, const float* pSource, int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);
	int 				(*exportSpxSamples)(wax_obj edgemap, wax_sampleOrigo origo, wax_spx* pDestination, int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);
	int 				(*exportFloatSamples)(wax_obj edgemap, wax_sampleOrigo origo, float* pDestination, int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);
	int					(*importPaletteEntries)(wax_obj edgemap, int begin, int end, const wax_color * pColors );		// RETIRED

	void				(*exportBounds)( wax_obj edgemap, wax_spx * pMinMaxOutput, int nSections, int sectionWidth, int topEdge, int bottomEdge, int mapOffset, int minMaxPitch );

	int					(*setEdgemapTints)(wax_obj edgemap, int begin, int end, const wax_obj * pTints );
	wax_obj				(*edgemapTint)(wax_obj edgemap, int segment);

	int					(*edgemapIsTiling)(wax_obj edgemap);
} wg_edgemap_calls;



//____ wg_warpmap_calls ______________________________________________________

typedef struct wg_warpmap_calls_struct
{
	int					structSize;

	wax_sizeI			(*warpmapPixelSize)(wax_obj warpmap);
	int					(*warpmapCellSize)(wax_obj warpmap);
	int					(*setWarpmapPeriods)(wax_obj warpmap, float uPeriod, float vPeriod);
	float				(*warpmapUPeriod)(wax_obj warpmap);
	float				(*warpmapVPeriod)(wax_obj warpmap);
	int					(*importWarpmapNodes)(wax_obj warpmap, const wax_rectI* region, const wax_coordF* pSource, int pitch);
	int					(*exportWarpmapNodes)(wax_obj warpmap, const wax_rectI* region, wax_coordF* pDestination, int pitch);

} wg_warpmap_calls;


//____ wg_warpmapfactory_calls ________________________________________________

typedef struct wg_warpmapfactory_calls_struct
{
	int					structSize;

	wax_obj				(*createWarpmap)(wax_obj factory, const wax_warpmapBP* blueprint);
	wax_obj				(*createWarpmapFromNodes)(wax_obj factory, const wax_warpmapBP* blueprint, const wax_coordF* pNodes, int pitch);

} wg_warpmapfactory_calls;


//____ wg_edgemapfactory_calls ________________________________________________

typedef struct wg_edgemapfactory_calls_struct
{
	int					structSize;

	wax_obj				(*createEdgemap)(wax_obj factory, const wax_edgemapBP* blueprint);
	wax_obj				(*createEdgemapFromFloats)(wax_obj factory, const wax_edgemapBP* blueprint, wax_sampleOrigo origo, const float* pSamples, int edges, int edgePitch, int samplePitch);
	wax_obj				(*createEdgemapFromSpx)(wax_obj factory, const wax_edgemapBP* blueprint, wax_sampleOrigo origo, const wax_spx* pSamples, int edges, int edgePitch, int samplePitch);

} wg_edgemapfactory_calls;



//____ wg_plugincapsule_calls __________________________________________________

typedef	struct wg_plugincapsule_calls_struct
{
		int			structSize;

		void		(*requestRender)(wax_obj hostCapsule, wax_rectSPX rect);
		void		(*requestResize)(wax_obj	hostCapsule);
		int			(*isVisible)(wax_obj hostCapsule);
		wax_rectSPX	(*windowSection)(wax_obj hostCapsule);
		int			(*requestFocus)(wax_obj hostCapsule, int bRaiseWindow);
		int			(*releaseFocus)(wax_obj hostCapsule);
		int			(*requestPreRenderCall)(wax_obj hostCapsule);

		void		(*requestInView)(wax_obj hostCapsule, wax_rectSPX mustHaveArea, wax_rectSPX niceToHaveArea );

		void		(*connect)(wax_obj hostCapsule, void * pPluginRoot, wg_pluginroot_calls* pInterface);
		void		(*disconnect)(wax_obj hostCapsule);

} wg_plugincapsule_calls;

//____ wg_blurbrush_calls ________________________________________________

typedef struct wg_blurbrush_calls_struct
{
	int				structSize;

	wax_obj			(*create)( wax_blurbrushBP bp );

	wax_spx			(*size)( wax_obj blurbrush );

	const float*	(*blue)( wax_obj blurbrush );
	const float*	(*green)( wax_obj blurbrush );
	const float*	(*red)( wax_obj blurbrush );

} wg_blurbrush_calls;


//____ Retired call structs ___________________________________________________
//
// Tintmap, Gradyent and StaticTintmap were replaced by Tint. The structs keep
// their place and size in wg_plugin_interface, and all calls are stubs.

typedef struct wg_tintmap_calls_struct
{
	int				structSize;

	int		(*isTintmapOpaque)( wax_obj tintmap );
	int		(*isTintmapVertical)( wax_obj tintmap );
	int		(*isTintmapHorizontal)( wax_obj tintmap );

	void	(*exportTintmapColors)( wax_obj tintmap, wax_sizeI tintmapSize, wax_color* pOutputX, wax_color* pOutputY);

} wg_tintmap_calls;

typedef struct wg_gradyent_calls_struct
{
	int				structSize;

	wax_obj	(*createGradyent)(wax_color top, wax_color bottom, wax_color left, wax_color right);

} wg_gradyent_calls;

typedef struct wg_statictintmap_calls_struct
{
	int				structSize;

	wax_obj	(*createStaticTintmap)( wax_sizeI size, const wax_color * pColorstripX, const wax_color * pColorstripY );

} wg_statictintmap_calls;

//____ wg_tint_calls_struct _____________________________________________

typedef struct wg_tint_calls_struct
{
	int				structSize;

	wax_obj		(*createTint)( const wax_tintBP* pBlueprint );
	wax_obj		(*createTintFromData)( const void* pData, int bytes );		// See wax_exportTintData().
	wax_obj		(*mixTints)( wax_obj fromTint, wax_obj toTint, float progress );

	int			(*isTintOpaque)( wax_obj tint );
	int			(*isTintFlat)( wax_obj tint );
	int			(*isTintMix)( wax_obj tint );

	wax_color	(*tintColorAt)( wax_obj tint, wax_coordSPX pos, const wax_rectSPX* pRect );
	int			(*exportTintData)( wax_obj tint, void* pDest, int maxBytes );

	wax_tintBP	(*defaultTintBP)( void );
	wax_obj		(*createTintMix)( int nbComponents, const wax_obj* pComponents, const float* pWeights );
	wax_tintBP	(*getTintBlueprint)( wax_obj tint );
	int			(*tintMixComponents)( wax_obj tint );
	wax_obj		(*tintMixComponent)( wax_obj tint, int index );
	float		(*tintMixWeight)( wax_obj tint, int index );
	int			(*tintAlphaAt)( wax_obj tint, wax_coordSPX pos, const wax_rectSPX* pRect );

} wg_tint_calls;





//____ wg_plugin_interface ____________________________________________________

typedef struct wg_plugin_interface_struct
{
	int							structSize;

	wg_bitmapcache_calls *		pBitmapCache;
	wg_bitmapfont_calls *		pBitmapFont;
	wg_canvaslayers_calls *		pCanvasLayers;
	wg_font_calls *				pFont;
	wg_object_calls *			pObject;
	wg_gfxdevice_calls *		pGfxDevice;
//	wg_streambuffer_calls *		pStreamBuffer;
//	wg_streamplayer_calls *		pStreamPlayer;
//	wg_streampump_calls *		pStreamPump;
//	wg_streamreader_calls *		pStreamReader;
	wg_surface_calls *			pSurface;
	wg_surfacefactory_calls *	pSurfaceFactory;
	wg_edgemap_calls*			pEdgemap;
	wg_edgemapfactory_calls*	pEdgemapFactory;
	wg_hostbridge_calls *		pHostBridge;
	wg_plugincapsule_calls *	pPluginCapsule;
	wg_blurbrush_calls *		pBlurbrush;
	wg_tintmap_calls *			pTintmap;			// RETIRED
	wg_gradyent_calls *			pGradyent;			// RETIRED
	wg_statictintmap_calls *	pStaticTintmap;		// RETIRED
	wg_tint_calls *				pTint;
	wg_warpmap_calls *			pWarpmap;
	wg_warpmapfactory_calls *	pWarpmapFactory;

} wg_plugin_interface;





void	wg_populatePluginInterface(wg_plugin_interface * pStruct);


#ifdef __cplusplus
}
#endif

#endif
