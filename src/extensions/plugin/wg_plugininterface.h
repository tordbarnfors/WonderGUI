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


#ifndef WG_PLUGININTERFACE_DOT_H
#define WG_PLUGININTERFACE_DOT_H
#pragma once

#include <wg_c_types.h>
#include <wg_c_geo.h>
#include <wg_c_color.h>
#include <wg_c_gradient.h>


#include <wg_c_bitmapcache.h>
#include <wg_c_canvaslayers.h>
#include <wg_c_font.h>
#include <wg_c_gfxdevice.h>
#include <wg_c_edgemap.h>
#include <wg_c_blurbrush.h>


#ifdef __cplusplus
extern "C" {
#endif

//____ wg_pluginroot_calls ____________________________________________________

typedef struct wg_pluginroot_calls_struct
{
		int			structSize;

		wg_spx			(*matchingHeight)(void* pPluginRoot, wg_spx width, int scale);
		wg_spx			(*matchingWidth)(void* pPluginRoot, wg_spx height, int scale);

		wg_sizeSPX		(*defaultSize)(void* pPluginRoot, int scale);
		wg_sizeSPX		(*minSize)(void* pPluginRoot, int scale);
		wg_sizeSPX		(*maxSize)(void* pPluginRoot, int scale);

		int				(*markTest)(void* pPluginRoot, wg_coordSPX ofs);

		void    		(*preRender)(void* pPluginRoot);
		void			(*render)(void* pPluginRoot, wg_obj pDevice, wg_rectSPX canvas, wg_rectSPX window);

		void			(*resize)(void* pPluginRoot, wg_sizeSPX size, int scale);
		void			(*setState)(void* pPluginRoot, wg_state state);
		void			(*receive)(void* pPluginRoot, wg_obj msg);

//		wg_string		(*tooltip)(void* pPluginRoot);
		wg_pointerStyle	(*pointerStyle)(void* pPluginRoot);

		void			(*setPointerPos)(void* pPluginRoot, wg_coordSPX pos, int64_t timestamp );
		void			(*setButtonState)(void* pPluginRoot, int button, int pressed, int64_t timestamp );
		void			(*setKeyState)(void* pPluginRoot, int nativeKeycode, int pressed, int64_t timestamp );
		void			(*putText)(void* pPluginRoot, const char * pUTF8String );

		void			(*onUpdate)(void* pPluginRoot, int microPassed, int64_t microsecTimestamp);

		void 			(*wheelRoll)(void* pPluginRoot, int wheel, wg_pts distance, int bInvert, int64_t timestamp );

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

	wg_obj				(*createBitmapCache)( int maxSize, wg_obj surfaceFactory );
	void				(*setCacheLimit)(wg_obj bitmapCache, int maxBytes);
	int                 (*cacheLimit)(wg_obj bitmapCache);
	int                 (*cacheSize)(wg_obj bitmapCache);
	void                (*clearCache)(wg_obj bitmapCache);
	wg_cacheListener    (*addCacheListener)(wg_obj bitmapCache, wg_cacheListenerBP blueprint );
	void                (*removeCacheListener)(wg_obj bitmapCache, wg_cacheListener listener );
	wg_cacheSlot        (*getCacheSlot)(wg_obj bitmapCache, wg_sizeI size);
	int                 (*getNbCacheSurfaces)(wg_obj bitmapCache);
	int                 (*getCacheSurfaces)(wg_obj bitmapCache, int maxSurfaces, wg_obj * pArray );

} wg_bitmapcache_calls;


//____ wg_bitmapfont_calls ____________________________________________________

typedef struct wg_bitmapfont_calls_struct
{
	int					structSize;

	wg_obj				(*createBitmapFont)(wg_obj surface, char* pGlyphSpec, wg_obj backupFont);
	wg_obj				(*getBitmapFontSurface)(wg_obj bitmapFont);

} wg_bitmapfont_calls;


//____ wg_canvaslayers_calls __________________________________________________

typedef struct wg_canvaslayers_calls_struct
{
	int					structSize;

	wg_obj				(*createCanvasLayers)(const wg_canvasLayersBP* pBlueprint);
	int					(*canvasLayersSize)(wg_obj canvasLayers);
	wg_pixelFormat		(*layerFormat)(wg_obj canvasLayers, int layer);
	int					(*baseLayer)(wg_obj canvasLayers);

} wg_canvaslayers_calls;


//____ wg_font_calls __________________________________________________________

typedef struct wg_font_calls_struct
{
	int					structSize;

	int					(*setFontSize)(wg_obj font, wg_spx size);
	wg_spx				(*fontSize)(wg_obj font);
	void				(*getGlyphWithoutBitmap)(wg_obj font, uint16_t chr, wg_glyph* pGlyph);
	void				(*getGlyphWithBitmap)(wg_obj font, uint16_t chr, wg_glyph* pGlyph);
	wg_spx				(*getKerning)(wg_obj font, wg_glyph* pLeftGlyph, wg_glyph* p_rightGlyph);
	wg_spx				(*lineGap)(wg_obj font);
	wg_spx				(*whitespaceAdvance)(wg_obj font);
	wg_spx				(*maxAdvance)(wg_obj font);
	wg_spx				(*maxAscend)(wg_obj font);
	wg_spx 				(*maxDescend)(wg_obj font);
	int					(*nbGlyphs)(wg_obj font);
	int					(*hasGlyphs)(wg_obj font);
	int					(*hasGlyph)(wg_obj font, uint16_t chr);
	int					(*isMonospace)(wg_obj font);
	int					(*isMonochrome)(wg_obj font);
	wg_obj				(*getBackupFont)(wg_obj font);
	wg_obj				(*createSurfaceFromGlyph)(wg_obj font, uint16_t chr, const wg_surfaceBP* blueprint, wg_obj surfaceFactory);

} wg_font_calls;


//____ wg_gfxdevice_calls _____________________________________________________

typedef struct wg_gfxdevice_calls_struct
{
	int					structSize;

	const wg_typeInfo*	(*deviceSurfaceType)(wg_obj device);
	wg_canvasInfo		(*getCanvas)(wg_obj device);
	wg_canvasInfo		(*getCanvasRef)(wg_obj device, wg_canvasRef ref);
	wg_obj				(*canvasLayers)(wg_obj device);
	wg_obj				(*surfaceFactory)(wg_obj device);
	wg_obj				(*edgemapFactory)(wg_obj device);
	int					(*maxSegments)();
	wg_sizeSPX			(*canvasSize)(wg_obj device);
	int					(*setClipList)(wg_obj device, int nRectangles, const wg_rectSPX* pRectangles);
	void				(*resetClipList)(wg_obj device);
	int					(*pushClipList)(wg_obj device, int nRectangles, const wg_rectSPX* pRectangles);
	int					(*popClipList)(wg_obj device);
	const wg_rectSPX*	(*getClipList)(wg_obj device);
	int					(*clipListSize)(wg_obj device);
	const wg_rectSPX*	(*clipBounds)(wg_obj device);
	void				(*setTintColor)(wg_obj device, wg_color color);
	wg_color			(*getTintColor)(wg_obj device);
	void				(*setTintGradient)(wg_obj device, const wg_rectSPX* rect, const wg_gradient* gradient);
	void				(*clearTintGradient)(wg_obj device);
	int					(*setBlendMode)(wg_obj device, wg_blendMode blendMode);
	wg_blendMode 		(*getBlendMode)(wg_obj device);
	int					(*setBlitSource)(wg_obj device, wg_obj surface);
	wg_obj 				(*getBlitSource)(wg_obj device);
	void				(*setMorphFactor)(wg_obj device, float factor);
	float				(*getMorphFactor)(wg_obj device);
	void				(*setRenderLayer)(wg_obj device, int layer);
	int					(*getRenderLayer)(wg_obj device);
	int					(*beginRender)(wg_obj device);
	int					(*endRender)(wg_obj device);
	int					(*isDeviceRendering)(wg_obj device);
	int					(*isDeviceIdle)(wg_obj device);
	void				(*flushDevice)(wg_obj device);
	int					(*beginCanvasUpdateWithRef)(wg_obj device, wg_canvasRef canvas, int nUpdateRects, const wg_rectSPX* pUpdateRects, wg_obj canvasLayers, int startLayer);
	int					(*beginCanvasUpdateWithSurface)(wg_obj device, wg_obj surface, int nUpdateRects, const wg_rectSPX* pUpdateRects, wg_obj canvasLayers, int startLayer);
	void				(*endCanvasUpdate)(wg_obj device);
	void				(*fill)(wg_obj device, wg_color col);
	void				(*fillRect)(wg_obj device, const wg_rectSPX* rect, wg_color col);
	void				(*plotPixels)(wg_obj device, int nCoords, const wg_coordSPX* pCoords, const wg_color* pColors);
	void				(*drawLine)(wg_obj device, wg_coordSPX begin, wg_coordSPX end, wg_color color, wg_spx thickness);
	void				(*drawStraightLine)(wg_obj device, wg_coordSPX begin, wg_direction dir, wg_spx length, wg_color color, wg_spx thickness);
	void				(*blit)(wg_obj device, wg_coordSPX dest);
	void				(*blitRect)(wg_obj device, wg_coordSPX dest, const wg_rectSPX* src);
	void				(*flipBlit)(wg_obj device, wg_coordSPX dest, wg_gfxFlip flip);
	void				(*flipBlitRect)(wg_obj device, wg_coordSPX dest, const wg_rectSPX* src, wg_gfxFlip flip);
	void				(*stretchBlit)(wg_obj device, const wg_rectSPX* dest);
	void				(*stretchBlitRect)(wg_obj device, const wg_rectSPX* dest, const wg_rectSPX* src);
	void				(*stretchFlipBlit)(wg_obj device, const wg_rectSPX* dest, wg_gfxFlip flip);
	void				(*stretchFlipBlitRect)(wg_obj device, const wg_rectSPX* dest, const wg_rectSPX* src, wg_gfxFlip flip);
	void				(*precisionBlit)(wg_obj device, const wg_rectSPX* dest, const wg_rectF* srcSPX);
	void				(*transformBlit)(wg_obj device, const wg_rectSPX* dest, wg_coordF srcSPX, const wg_transform * pTransform);
	void				(*rotScaleBlit)(wg_obj device, const wg_rectSPX* dest, float rotationDegrees, float scale, wg_coordF srcCenter, wg_coordF destCenter);
	void				(*tile)(wg_obj device, const wg_rectSPX* dest, wg_coordSPX shift);
	void				(*flipTile)(wg_obj device, const wg_rectSPX* dest, wg_gfxFlip flip, wg_coordSPX shift);
	void				(*scaleTile)(wg_obj device, const wg_rectSPX* dest, float scale, wg_coordSPX shift);
	void				(*scaleFlipTile)(wg_obj device, const wg_rectSPX* dest, float scale, wg_gfxFlip flip, wg_coordSPX shift);
	void				(*drawWave)(wg_obj device, const wg_rectSPX* dest, const wg_waveLine* pTopBorder, const wg_waveLine* pBottomBorder, wg_color frontFill, wg_color backFill);
	void				(*flipDrawWave)(wg_obj device, const wg_rectSPX* dest, const wg_waveLine* pTopBorder, const wg_waveLine* pBottomBorder, wg_color frontFill, wg_color backFill, wg_gfxFlip flip);
	void				(*drawElipse)(wg_obj device, const wg_rectSPX* canvas, wg_spx thickness, wg_color color, wg_spx outlineThickness, wg_color outlineColor);
	void				(*drawPieChart)(wg_obj device, const wg_rectSPX* canvas, float start, int nSlices, const float* pSliceSizes, const wg_color* pSliceColors, float hubSize, wg_color hubColor, wg_color backColor, int bRectangular);
	void				(*drawSegments)(wg_obj device, const wg_rectSPX* dest, int nSegments, const wg_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wg_tintMode tintMode);
	void				(*flipDrawSegments)(wg_obj device, const wg_rectSPX* dest, int nSegments, const wg_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wg_gfxFlip flip, wg_tintMode tintMode);
	void				(*drawEdgemap)(wg_obj device, wg_coordSPX dest, wg_obj edgemap);
	void				(*flipDrawEdgemap)(wg_obj device, wg_coordSPX dest, wg_obj edgemap, wg_gfxFlip flip);
	void				(*blitNinePatch)(wg_obj device, const wg_rectSPX* dstRect, const wg_borderSPX* dstFrame, const wg_ninePatch* patch, int scale);

	void				(*setBlurbrush)(wg_obj device, wg_obj brush );
	void				(*setFixedBlendColor)(wg_obj device, wg_color color );
	wg_color			(*getFixedBlendColor)(wg_obj device);
	
	void				(*blur)(wg_obj device, wg_coordSPX dest);
	void				(*blurRect)(wg_obj device, wg_coordSPX dest, const wg_rectSPX* src);
	void				(*stretchBlur)(wg_obj device, const wg_rectSPX* dest);
	void				(*stretchBlurRect)(wg_obj device, const wg_rectSPX* dest, const wg_rectSPX* src);
	void				(*transformBlur)(wg_obj device, const wg_rectSPX* dest, wg_coordF srcSPX, const wg_transform * pTransform);
	void				(*rotScaleBlur)(wg_obj device, const wg_rectSPX* dest, float rotationDegrees, float scale, wg_coordF srcCenter, wg_coordF destCenter);

	
} wg_gfxdevice_calls;

/*
//____ wg_streambuffer_calls __________________________________________________

typedef struct wg_streambuffer_calls_struct
{
	int					structSize;

	wg_obj				(*createStreamBuffer)(int initialCapacity);
	wg_component		(*getStreamBufferOutput)(wg_obj streamBuffer);
	wg_component		(*getStreamBufferInput)(wg_obj streamBuffer);
	int					(*streamBufferCapacity)(wg_obj streamBuffer);
	int					(*streamBufferHasChunk)(wg_obj streamBuffer);
	int					(*streamBufferBytes)(wg_obj streamBuffer);

} wg_streambuffer_calls;


//____ wg_streamplayer_calls __________________________________________________

typedef struct wg_streamplayer_calls_struct
{
	int					structSize;

	wg_obj				(*createStreamPlayer)(wg_obj device, wg_obj surfaceFactory, wg_obj edgemapFactory);
	wg_component		(*getStreamPlayerInput)(wg_obj streamPlayer);
	void				(*setStreamPlayerStoreDirtyRects)(wg_obj streamPlayer, int bStore);
	void				(*setStreamPlayerMaxDirtyRects)(wg_obj streamPlayer, int max);
	int					(*getStreamPlayerDirtyRects)(wg_obj streamPlayer, wg_canvasRef canvas, const wg_rectI** pRects);
	void				(*clearStreamPlayerDirtyRects)(wg_obj streamPlayer);

} wg_streamplayer_calls;


//____ wg_streampump_calls ____________________________________________________

typedef struct wg_streampump_calls_struct
{
	int					structSize;

	wg_obj				(*createStreamPump)();
	wg_obj				(*createStreamPumpWithInputOutput)(wg_component input, wg_component output);
	void				(*setStreamPumpInput)(wg_obj streamPump, wg_component input);
	void				(*setStreamPumpOutput)(wg_obj streamPump, wg_component output);
	wg_streamChunkId		(*peekChunk)(wg_obj streamPump);
	int					(*pumpChunk)(wg_obj streamPump);
	int					(*pumpUntilFrame)(wg_obj streamPump);
	int					(*pumpFrame)(wg_obj streamPump);
	int					(*pumpAll)(wg_obj streamPump);

} wg_streampump_calls;


//____ wg_streamreader_calls __________________________________________________

typedef struct wg_streamreader_calls_struct
{
	int					structSize;

	wg_obj				(*createStreamReader)(wg_readStream_func callback);
	wg_component		(*getStreamReaderOutput)(wg_obj streamReader);
	int					(*streamReaderCapacity)(wg_obj streamReader);
	int					(*streamReaderHasChunk)(wg_obj streamReader);
	int					(*streamReaderBytes)(wg_obj streamReader);

} wg_streamreader_calls;
*/


//____ wg_object_calls ________________________________________________________

typedef struct wg_object_calls_struct
{
	int					structSize;

	const wg_typeInfo*	(*getTypeInfo)(wg_obj);
	int					(*isInstanceOf)(wg_obj, const wg_typeInfo* pTypeInfo);
	void				(*setFinalizer)(wg_obj, wg_finalizer_func pFinalizer);
	wg_finalizer_func	(*finalizer)(wg_obj);
	void				(*retain)(wg_obj);
	void				(*release)(wg_obj);
	int					(*refcount)(wg_obj);

} wg_object_calls;


//____ wg_surface_calls _______________________________________________________

typedef struct wg_surface_calls_struct
{
	int					structSize;

	void				(*setSurfaceIdentity)(wg_obj surface, int id);
	int					(*getSurfaceIdentity)(wg_obj surface);
	wg_sizeI			(*surfacePixelSize)(wg_obj surface);
	int					(*surfacePixelWidth)(wg_obj surface);
	int					(*surfacePixelHeight)(wg_obj surface);
	wg_size				(*surfacePointSize)(wg_obj surface);
	wg_pts				(*surfacePointWidth)(wg_obj surface);
	wg_pts				(*surfacePointHeight)(wg_obj surface);
	int					(*surfaceScale)(wg_obj surface);
	wg_sampleMethod		(*surfaceSampleMethod)(wg_obj surface);
	int					(*surfaceIsTiling)(wg_obj surface);
	int					(*surfaceIsMipmapped)(wg_obj surface);
	int					(*surfaceAlpha)(wg_obj surface, wg_coordSPX coord);
	const wg_color8*	(*surfacePalette)(wg_obj surface);
	int					(*surfacePaletteSize)(wg_obj surface);
	int					(*surfacePaletteCapacity)(wg_obj surface);
	const wg_pixelDescription* (*surfacePixelDescription)(wg_obj surface);
	wg_pixelFormat		(*surfacePixelFormat)(wg_obj surface);
	int					(*surfacePixelBits)(wg_obj surface);
	int					(*surfaceIsOpaque)(wg_obj surface);
	int					(*surfaceCanBeCanvas)(wg_obj surface);
	wg_pixelBuffer		(*allocPixelBuffer)(wg_obj surface);
	wg_pixelBuffer		(*allocPixelBufferFromRect)(wg_obj surface, const wg_rectI* rect);
	int					(*pushPixels)(wg_obj surface, const wg_pixelBuffer* buffer);
	int					(*pushPixelsFromRect)(wg_obj surface, const wg_pixelBuffer* buffer, const wg_rectI* bufferRect);
	void				(*pullPixels)(wg_obj surface, const wg_pixelBuffer* buffer);
	void				(*pullPixelsFromRect)(wg_obj surface, const wg_pixelBuffer* buffer, const wg_rectI* bufferRect);
	void				(*freePixelBuffer)(wg_obj surface, const wg_pixelBuffer* buffer);
	int					(*fillSurface)(wg_obj surface, wg_color col);
	int					(*fillSurfaceRect)(wg_obj surface, const wg_rectI* region, wg_color col);
	int					(*copySurface)(wg_obj destSurface, wg_coordI dst, wg_obj sourceSurface);
	int					(*copySurfaceRect)(wg_obj destSurface, wg_coordI dst, wg_obj sourceSurface, const wg_rectI* srcRect);
	void				(*setSurfaceBaggage)(wg_obj surface, wg_obj baggage);
	wg_obj				(*getSurfaceBaggage)(wg_obj surface);
	int					(*addSurfaceObserver)(wg_obj surface, wg_surfaceObserver_func, void* pData, int data);
	int					(*removeSurfaceObserver)(wg_obj surface, int observerId);
	wg_surfaceBP		(*getSurfaceBlueprint)(wg_obj surface);

} wg_surface_calls;


//____ wg_surfacefactory_calls ________________________________________________

typedef struct wg_surfacefactory_calls_struct
{
	int					structSize;

	wg_sizeI			(*maxSurfaceSize)(wg_obj surfaceFactory);
	wg_obj				(*createSurface)(wg_obj factory, const wg_surfaceBP* blueprint);
	wg_obj				(*createSurfaceFromBlob)(wg_obj factory, const wg_surfaceBP* blueprint, wg_obj blob, int pitch);
	wg_obj				(*createSurfaceFromBitmap)(wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels, wg_pixelFormat format,  int pitch, const wg_color8 * pPalette, int paletteSize);
	wg_obj				(*createSurfaceFromRawData)(wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels, const wg_pixelDescription* pPixelDescription, int pitch, const wg_color8* pPalette, int paletteSize);

} wg_surfacefactory_calls;


//____ wg_edgemap_calls ______________________________________________________

typedef struct wg_edgemap_calls_struct
{
	int					structSize;

	wg_sizeI			(*edgemapPixelSize)(wg_obj edgemap);
	int					(*setRenderSegments)(wg_obj edgemap, int nSegments);
	int					(*getRenderSegments)(wg_obj edgemap);
	wg_color			(*edgemapColor)(wg_obj edgemap, int segment);
	wg_gradient			(*edgemapGradient)(wg_obj edgemap, int segment);
	int					(*edgemapSegments)(wg_obj edgemap);
	int					(*edgemapSamples)(wg_obj edgemap);
	int 				(*importSpxSamples)(wg_obj edgemap, wg_sampleOrigo origo, const wg_spx* pSource, int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);
	int 				(*importFloatSamples)(wg_obj edgemap, wg_sampleOrigo origo, const float* pSource, int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);
	int 				(*exportSpxSamples)(wg_obj edgemap, wg_sampleOrigo origo, wg_spx* pDestination, int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);
	int 				(*exportFloatSamples)(wg_obj edgemap, wg_sampleOrigo origo, float* pDestination, int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);
	int					(*importPaletteEntries)(wg_obj edgemap, int begin, int end, const wg_color * pColors );
	

	const wg_color *	(*edgemapColors)(wg_obj edgemap);
	const wg_gradient *	(*edgemapGradients)(wg_obj edgemap);

	int					(*setEdgemapColors)( wg_obj edgemap, int begin, int end, const wg_color * pColors );
	int					(*setEdgemapGradients)( wg_obj edgemap, int begin, int end, const wg_gradient * pGradients );

} wg_edgemap_calls;


//____ wg_edgemapfactory_calls ________________________________________________

typedef struct wg_edgemapfactory_calls_struct
{
	int					structSize;

	wg_obj				(*createEdgemap)(wg_obj factory, const wg_edgemapBP* blueprint);
	wg_obj				(*createEdgemapFromFloats)(wg_obj factory, const wg_edgemapBP* blueprint, wg_sampleOrigo origo, const float* pSamples, int edges, int edgePitch, int samplePitch);
	wg_obj				(*createEdgemapFromSpx)(wg_obj factory, const wg_edgemapBP* blueprint, wg_sampleOrigo origo, const wg_spx* pSamples, int edges, int edgePitch, int samplePitch);

} wg_edgemapfactory_calls;



//____ wg_plugincapsule_calls __________________________________________________

typedef	struct wg_plugincapsule_calls_struct
{
		int			structSize;

		void		(*requestRender)(wg_obj hostCapsule, wg_rectSPX rect);
		void		(*requestResize)(wg_obj	hostCapsule);
		int			(*isVisible)(wg_obj hostCapsule);
		wg_rectSPX	(*windowSection)(wg_obj hostCapsule);
		int			(*requestFocus)(wg_obj hostCapsule, int bRaiseWindow);
		int			(*releaseFocus)(wg_obj hostCapsule);
		int			(*requestPreRenderCall)(wg_obj hostCapsule);

		void		(*requestInView)(wg_obj hostCapsule, wg_rectSPX mustHaveArea, wg_rectSPX niceToHaveArea );

		void		(*connect)(wg_obj hostCapsule, void * pPluginRoot, wg_pluginroot_calls* pInterface);
		void		(*disconnect)(wg_obj hostCapsule);

} wg_plugincapsule_calls;

//____ wg_blurbrush_calls ________________________________________________

typedef struct wg_blurbrush_calls_struct
{
	int				structSize;

	wg_obj			(*create)( wg_blurbrushBP bp );

	wg_spx			(*size)( wg_obj blurbrush );

	const float*	(*blue)( wg_obj blurbrush );
	const float*	(*green)( wg_obj blurbrush );
	const float*	(*red)( wg_obj blurbrush );

} wg_blurbrush_calls;

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

} wg_plugin_interface;





void	wg_populatePluginInterface(wg_plugin_interface * pStruct);


#ifdef __cplusplus
}
#endif

#endif
