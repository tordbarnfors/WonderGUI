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
#include <wg_plugininterface.h>

#include <wg_c_hostbridge.h>

#include <wax_c_bitmapcache.h>
#include <wax_c_bitmapfont.h>
#include <wax_c_canvaslayers.h>
#include <wax_c_font.h>
#include <wax_c_device.h>
/*
#include <wax_c_streambuffer.h>
#include <wax_c_streamplayer.h>
#include <wax_c_streampump.h>
#include <wax_c_streamreader.h>
*/
#include <wax_c_object.h>
#include <wax_c_surface.h>
#include <wax_c_surfacefactory.h>
#include <wax_c_edgemap.h>
#include <wax_c_edgemapfactory.h>
#include <wax_c_warpmap.h>
#include <wax_c_warpmapfactory.h>
#include <wg_c_plugincapsule.h>

#include <wax_c_tint.h>

#include <mutex>


inline std::recursive_mutex g_apiMutex;     // the "big lock", lives in the host

template<auto Func> struct Locked;

template<typename R, typename... Args, R (*Func)(Args...)>
struct Locked<Func>
{
	static R call(Args... args)
	{
		std::lock_guard lock(g_apiMutex);
		return Func(args...);
	}
};

#define LOCKED(f) (&Locked<&f>::call)


struct wg_c_calls_body
{
	wg_bitmapcache_calls		bitmapCache;
	wg_bitmapfont_calls			bitmapFont;
	wg_canvaslayers_calls		canvasLayers;
	wg_font_calls				font;
	wg_object_calls				object;
	wg_gfxdevice_calls			gfxDevice;
/*
	wg_streambuffer_calls		streamBuffer;
	wg_streamplayer_calls		streamPlayer;
	wg_streampump_calls			streamPump;
	wg_streamreader_calls		streamReader;
*/
	wg_surface_calls			surface;
	wg_surfacefactory_calls		surfaceFactory;
	wg_edgemap_calls			edgemap;
	wg_edgemapfactory_calls		edgemapFactory;
	wg_hostbridge_calls			hostBridge;
	wg_plugincapsule_calls		pluginCapsule;
	wg_blurbrush_calls			blurbrush;
	wg_tintmap_calls			tintmap;			// Retired, all stubs.
	wg_gradyent_calls			gradyent;			// Retired, all stubs.
	wg_statictintmap_calls		staticTintmap;		// Retired, all stubs.

	wg_tint_calls				tint;
	wg_warpmap_calls			warpmap;
	wg_warpmapfactory_calls		warpmapFactory;
};

static wg_c_calls_body	body;
static std::once_flag	bodyPopulated;

//____ Stubs for retired calls __________________________________________________
//
// Plugins built before a call was retired may still call it. These do nothing
// and return 0 or null, with the call's own signature so no caller is surprised
// by how a value comes back.

static void			stub_setTintmap(wax_obj, const wax_rectSPX*, const wax_obj) {}
static wax_obj		stub_getTintmap(wax_obj) { return nullptr; }
static wax_rectSPX	stub_getTintmapRect(wax_obj) { return wax_rectSPX{ 0, 0, 0, 0 }; }
static void			stub_clearTintmap(wax_obj) {}
static int			stub_hasTintmap(wax_obj) { return 0; }
static void			stub_setTintGradient(wax_obj, const wax_rectSPX*, const void*) {}
static void			stub_clearTintGradient(wax_obj) {}

static int				stub_edgemapPaletteType(wax_obj) { return 0; }		// Was WG_EDGEMAP_PALETTE_UNDEFINED.
static int				stub_setEdgemapColorsFromGradients(wax_obj, int, int, const void*) { return 0; }
static int				stub_setEdgemapColorsFromTintmaps(wax_obj, int, int, wax_obj*) { return 0; }
static int				stub_setEdgemapColorsFromStrips(wax_obj, int, int, const wax_color*, const wax_color*) { return 0; }
static int				stub_importEdgemapPaletteEntries(wax_obj, int, int, const wax_color*) { return 0; }
static const wax_color*	stub_edgemapColorstrips(wax_obj) { return nullptr; }
static int				stub_importPaletteEntries(wax_obj, int, int, const wax_color*) { return 0; }

static int		stub_isTintmapFlag(wax_obj) { return 0; }
static void		stub_exportTintmapColors(wax_obj, wax_sizeI, wax_color*, wax_color*) {}
static wax_obj	stub_createGradyent(wax_color, wax_color, wax_color, wax_color) { return nullptr; }
static wax_obj	stub_createStaticTintmap(wax_sizeI, const wax_color*, const wax_color*) { return nullptr; }



// The body is shared by all headers and only ever holds the same constant function
// pointers, so it is filled once. wg_populatePluginInterface() may be called from
// several threads at once (plugins set up in parallel) while plugins read the body.
//
// Every call goes through the big lock, except the stubs for retired calls, which
// touch nothing.

static void populateBody(wg_c_calls_body * pBody)
{
	pBody->bitmapCache.structSize			= sizeof(wg_bitmapcache_calls);
	pBody->bitmapCache.createBitmapCache	= LOCKED(wax_createBitmapCache);
	pBody->bitmapCache.setCacheLimit		= LOCKED(wax_setCacheLimit);
	pBody->bitmapCache.cacheLimit			= LOCKED(wax_cacheLimit);
	pBody->bitmapCache.cacheSize			= LOCKED(wax_cacheSize);
	pBody->bitmapCache.clearCache			= LOCKED(wax_clearCache);
	pBody->bitmapCache.addCacheListener		= LOCKED(wax_addCacheListener);
	pBody->bitmapCache.removeCacheListener	= LOCKED(wax_removeCacheListener);
	pBody->bitmapCache.getCacheSlot			= LOCKED(wax_getCacheSlot);
	pBody->bitmapCache.getNbCacheSurfaces	= LOCKED(wax_getNbCacheSurfaces);
	pBody->bitmapCache.getCacheSurfaces		= LOCKED(wax_getCacheSurfaces);


	pBody->bitmapFont.structSize			= sizeof(wg_bitmapfont_calls);
	pBody->bitmapFont.createBitmapFont		= LOCKED(wax_createBitmapFont);
	pBody->bitmapFont.getBitmapFontSurface	= LOCKED(wax_getBitmapFontSurface);


	pBody->canvasLayers.structSize			= sizeof(wg_canvaslayers_calls);
	pBody->canvasLayers.baseLayer			= LOCKED(wax_baseLayer);
	pBody->canvasLayers.canvasLayersSize	= LOCKED(wax_canvasLayersSize);
	pBody->canvasLayers.createCanvasLayers	= LOCKED(wax_createCanvasLayers);
	pBody->canvasLayers.layerFormat			= LOCKED(wax_layerFormat);

	pBody->font.structSize					= sizeof(wg_font_calls);
	pBody->font.setFontSize					= LOCKED(wax_setFontSize);
	pBody->font.fontSize					= LOCKED(wax_fontSize);
	pBody->font.getGlyphWithoutBitmap		= LOCKED(wax_getGlyphWithoutBitmap);
	pBody->font.getGlyphWithBitmap			= LOCKED(wax_getGlyphWithBitmap);
	pBody->font.getKerning					= LOCKED(wax_getKerning);
	pBody->font.lineGap						= LOCKED(wax_lineGap);
	pBody->font.whitespaceAdvance			= LOCKED(wax_whitespaceAdvance);
	pBody->font.maxAdvance					= LOCKED(wax_maxAdvance);
	pBody->font.maxAscend					= LOCKED(wax_maxAscend);
	pBody->font.maxDescend					= LOCKED(wax_maxDescend);
	pBody->font.nbGlyphs					= LOCKED(wax_nbGlyphs);
	pBody->font.hasGlyphs					= LOCKED(wax_hasGlyphs);
	pBody->font.hasGlyph					= LOCKED(wax_hasGlyph);
	pBody->font.isMonospace					= LOCKED(wax_isMonospace);
	pBody->font.isMonochrome				= LOCKED(wax_isMonochrome);
	pBody->font.getBackupFont				= LOCKED(wax_getBackupFont);
	pBody->font.createSurfaceFromGlyph		= LOCKED(wax_createSurfaceFromGlyph);


	pBody->gfxDevice.structSize				= sizeof(wg_gfxdevice_calls);
	pBody->gfxDevice.deviceSurfaceType		= nullptr;						// Hopefully this is not used anymore.
	pBody->gfxDevice.getCanvas				= LOCKED(wax_getCanvas);
	pBody->gfxDevice.getCanvasRef			= LOCKED(wax_getCanvasRef);
	pBody->gfxDevice.canvasLayers			= LOCKED(wax_canvasLayers);
	pBody->gfxDevice.surfaceFactory			= LOCKED(wax_surfaceFactory);
	pBody->gfxDevice.edgemapFactory			= LOCKED(wax_edgemapFactory);
	pBody->gfxDevice.maxSegments			= LOCKED(wax_maxSegments);
	pBody->gfxDevice.canvasSize				= LOCKED(wax_canvasSize);
	pBody->gfxDevice.setClipList			= LOCKED(wax_setClipList);
	pBody->gfxDevice.resetClipList			= LOCKED(wax_resetClipList);
	pBody->gfxDevice.pushClipList			= LOCKED(wax_pushClipList);
	pBody->gfxDevice.popClipList			= LOCKED(wax_popClipList);
	pBody->gfxDevice.getClipList			= LOCKED(wax_getClipList);
	pBody->gfxDevice.clipListSize			= LOCKED(wax_clipListSize);
	pBody->gfxDevice.clipBounds				= LOCKED(wax_clipBounds);
	pBody->gfxDevice.setTintColor			= LOCKED(wax_setTintColor);
	pBody->gfxDevice.clearTintColor			= LOCKED(wax_clearTintColor);
	pBody->gfxDevice.hasTintColor			= LOCKED(wax_hasTintColor);
	pBody->gfxDevice.getTintColor			= LOCKED(wax_getTintColor);

	pBody->gfxDevice.setTintmap				= &stub_setTintmap;
	pBody->gfxDevice.getTintmap				= &stub_getTintmap;
	pBody->gfxDevice.getTintmapRect			= &stub_getTintmapRect;
	pBody->gfxDevice.clearTintmap			= &stub_clearTintmap;
	pBody->gfxDevice.hasTintmap				= &stub_hasTintmap;
	pBody->gfxDevice.setTintGradient		= &stub_setTintGradient;
	pBody->gfxDevice.clearTintGradient		= &stub_clearTintGradient;
	pBody->gfxDevice.setTint				= LOCKED(wax_setTint);
	pBody->gfxDevice.getTint				= LOCKED(wax_getTint);
	pBody->gfxDevice.getTintRect			= LOCKED(wax_getTintRect);
	pBody->gfxDevice.clearTint				= LOCKED(wax_clearTint);
	pBody->gfxDevice.hasTint				= LOCKED(wax_hasTint);

	pBody->gfxDevice.setBlendMode			= LOCKED(wax_setBlendMode);
	pBody->gfxDevice.getBlendMode			= LOCKED(wax_getBlendMode);
	pBody->gfxDevice.setBlitSource			= LOCKED(wax_setBlitSource);
	pBody->gfxDevice.getBlitSource			= LOCKED(wax_getBlitSource);
	pBody->gfxDevice.setMorphFactor			= LOCKED(wax_setMorphFactor);
	pBody->gfxDevice.getMorphFactor			= LOCKED(wax_getMorphFactor);
	pBody->gfxDevice.setRenderLayer			= LOCKED(wax_setRenderLayer);
	pBody->gfxDevice.getRenderLayer			= LOCKED(wax_getRenderLayer);
	pBody->gfxDevice.beginRender			= LOCKED(wax_beginRender);
	pBody->gfxDevice.endRender				= LOCKED(wax_endRender);
	pBody->gfxDevice.isDeviceRendering		= LOCKED(wax_isDeviceRendering);
	pBody->gfxDevice.isDeviceIdle			= LOCKED(wax_isDeviceIdle);
	pBody->gfxDevice.flushDevice			= LOCKED(wax_flushDevice);
	pBody->gfxDevice.beginCanvasUpdateWithRef		= LOCKED(wax_beginCanvasUpdateWithRef);
	pBody->gfxDevice.beginCanvasUpdateWithSurface	= LOCKED(wax_beginCanvasUpdateWithSurface);
	pBody->gfxDevice.endCanvasUpdate		= LOCKED(wax_endCanvasUpdate);
	pBody->gfxDevice.fill					= LOCKED(wax_fill);
	pBody->gfxDevice.fillRect				= LOCKED(wax_fillRect);
	pBody->gfxDevice.plotPixels				= nullptr;				// Plot pixels has been deprecated.
	pBody->gfxDevice.drawLine				= LOCKED(wax_drawLine);
	pBody->gfxDevice.drawStraightLine		= LOCKED(wax_drawStraightLine);
	pBody->gfxDevice.blit					= LOCKED(wax_blit);
	pBody->gfxDevice.blitRect				= LOCKED(wax_blitRect);
	pBody->gfxDevice.flipBlit				= LOCKED(wax_flipBlit);
	pBody->gfxDevice.flipBlitRect			= LOCKED(wax_flipBlitRect);
	pBody->gfxDevice.stretchBlit			= LOCKED(wax_stretchBlit);
	pBody->gfxDevice.stretchBlitRect		= LOCKED(wax_stretchBlitRect);
	pBody->gfxDevice.stretchFlipBlit		= LOCKED(wax_stretchFlipBlit);
	pBody->gfxDevice.stretchFlipBlitRect	= LOCKED(wax_stretchFlipBlitRect);
	pBody->gfxDevice.precisionBlit			= LOCKED(wax_precisionBlit);
	pBody->gfxDevice.transformBlit			= LOCKED(wax_transformBlit);
	pBody->gfxDevice.rotScaleBlit			= LOCKED(wax_rotScaleBlit);
	pBody->gfxDevice.tile					= LOCKED(wax_tile);
	pBody->gfxDevice.flipTile				= LOCKED(wax_flipTile);
	pBody->gfxDevice.scaleTile				= LOCKED(wax_scaleTile);
	pBody->gfxDevice.scaleFlipTile			= LOCKED(wax_scaleFlipTile);
	pBody->gfxDevice.drawWave				= LOCKED(wax_drawWave);
	pBody->gfxDevice.flipDrawWave			= LOCKED(wax_flipDrawWave);
	pBody->gfxDevice.drawElipse				= LOCKED(wax_drawElipse);
	pBody->gfxDevice.drawPieChart			= LOCKED(wax_drawPieChart);
	pBody->gfxDevice.drawSegments			= LOCKED(wax_drawSegments);
	pBody->gfxDevice.flipDrawSegments		= LOCKED(wax_flipDrawSegments);
	pBody->gfxDevice.drawEdgemap			= LOCKED(wax_drawEdgemap);
	pBody->gfxDevice.flipDrawEdgemap		= LOCKED(wax_flipDrawEdgemap);
	pBody->gfxDevice.tileEdgemap			= LOCKED(wax_tileEdgemap);
	pBody->gfxDevice.flipTileEdgemap		= LOCKED(wax_flipTileEdgemap);
	pBody->gfxDevice.blitNinePatch			= LOCKED(wax_blitNinePatch);

	pBody->gfxDevice.setBlurMatrices		= LOCKED(wax_setBlurMatrices);
	pBody->gfxDevice.setFixedBlendColor		= LOCKED(wax_setFixedBlendColor);
	pBody->gfxDevice.getFixedBlendColor		= LOCKED(wax_getFixedBlendColor);
	
	pBody->gfxDevice.blur					= LOCKED(wax_blur);
	pBody->gfxDevice.blurRect				= LOCKED(wax_blurRect);
	pBody->gfxDevice.stretchBlur			= LOCKED(wax_stretchBlur);
	pBody->gfxDevice.stretchBlurRect		= LOCKED(wax_stretchBlurRect);
	pBody->gfxDevice.transformBlur			= LOCKED(wax_transformBlur);
	pBody->gfxDevice.rotScaleBlur			= LOCKED(wax_rotScaleBlur);

	pBody->gfxDevice.setBlurbrush			= LOCKED(wax_setBlurbrush);

	pBody->gfxDevice.drawRoundedBox			= LOCKED(wax_drawRoundedBox);
	pBody->gfxDevice.drawRing				= LOCKED(wax_drawRing);
	pBody->gfxDevice.warpmapFactory			= LOCKED(wax_warpmapFactory);
	pBody->gfxDevice.warpBlit				= LOCKED(wax_warpBlit);
	pBody->gfxDevice.warpBlitRect			= LOCKED(wax_warpBlitRect);
	pBody->gfxDevice.warpDrawEdgemap		= LOCKED(wax_warpDrawEdgemap);
	pBody->gfxDevice.drawLineWithCaps		= LOCKED(wax_drawLineWithCaps);
	pBody->gfxDevice.drawStraightLineWithCaps = LOCKED(wax_drawStraightLineWithCaps);

/*
	pBody->streamBuffer.structSize				= sizeof(wg_streambuffer_calls);
	pBody->streamBuffer.createStreamBuffer		= LOCKED(wax_createStreamBuffer);
	pBody->streamBuffer.getStreamBufferOutput	= LOCKED(wax_getStreamBufferOutput);
	pBody->streamBuffer.getStreamBufferInput	= LOCKED(wax_getStreamBufferInput);
	pBody->streamBuffer.streamBufferCapacity	= LOCKED(wax_streamBufferCapacity);
	pBody->streamBuffer.streamBufferHasChunk	= LOCKED(wax_streamBufferHasChunk);
	pBody->streamBuffer.streamBufferBytes		= LOCKED(wg_streamBufferBytes);


	pBody->streamPlayer.structSize			= sizeof(wg_streamplayer_calls);
	pBody->streamPlayer.createStreamPlayer	= LOCKED(wax_createStreamPlayer);
	pBody->streamPlayer.getStreamPlayerInput	= LOCKED(wax_getStreamPlayerInput);
	pBody->streamPlayer.setStreamPlayerStoreDirtyRects = LOCKED(wax_setStreamPlayerStoreDirtyRects);
	pBody->streamPlayer.setStreamPlayerMaxDirtyRects = LOCKED(wax_setStreamPlayerMaxDirtyRects);
	pBody->streamPlayer.getStreamPlayerDirtyRects = LOCKED(wax_getStreamPlayerDirtyRects);
	pBody->streamPlayer.clearStreamPlayerDirtyRects = LOCKED(wax_clearStreamPlayerDirtyRects);


	pBody->streamPump.structSize			= sizeof(wg_streampump_calls);
	pBody->streamPump.createStreamPump		= LOCKED(wax_createStreamPump);
	pBody->streamPump.createStreamPumpWithInputOutput = LOCKED(wax_createStreamPumpWithInputOutput);
	pBody->streamPump.setStreamPumpInput	= LOCKED(wax_setStreamPumpInput);
	pBody->streamPump.setStreamPumpOutput	= LOCKED(wax_setStreamPumpOutput);
	pBody->streamPump.peekChunk				= LOCKED(wax_peekChunk);
	pBody->streamPump.pumpChunk				= LOCKED(wax_pumpChunk);
	pBody->streamPump.pumpUntilFrame		= LOCKED(wax_pumpUntilFrame);
	pBody->streamPump.pumpFrame				= LOCKED(wax_pumpFrame);
	pBody->streamPump.pumpAll				= LOCKED(wax_pumpAll);


	pBody->streamReader.structSize				= sizeof(wg_streamreader_calls);
	pBody->streamReader.createStreamReader		= LOCKED(wax_createStreamReader);
	pBody->streamReader.getStreamReaderOutput	= LOCKED(wax_getStreamReaderOutput);
	pBody->streamReader.streamReaderCapacity	= LOCKED(wax_streamReaderCapacity);
	pBody->streamReader.streamReaderHasChunk	= LOCKED(wax_streamReaderHasChunk);
	pBody->streamReader.streamReaderBytes		= LOCKED(wax_streamReaderBytes);
*/


	pBody->object.structSize				= sizeof(wg_object_calls);
	pBody->object.finalizer					= LOCKED(wax_finalizer);
	pBody->object.getTypeInfo				= LOCKED(wax_getTypeInfo);
	pBody->object.isInstanceOf				= LOCKED(wax_isInstanceOf);
	pBody->object.refcount					= LOCKED(wax_refcount);
	pBody->object.release					= LOCKED(wax_release);
	pBody->object.retain					= LOCKED(wax_retain);
	pBody->object.setFinalizer				= LOCKED(wax_setFinalizer);


	pBody->surface.structSize				= sizeof(wg_surface_calls);
	pBody->surface.setSurfaceIdentity		= LOCKED(wax_setSurfaceIdentity);
	pBody->surface.getSurfaceIdentity		= LOCKED(wax_getSurfaceIdentity);
	pBody->surface.surfacePixelSize			= LOCKED(wax_surfacePixelSize);
	pBody->surface.surfacePixelWidth		= LOCKED(wax_surfacePixelWidth);
	pBody->surface.surfacePixelHeight		= LOCKED(wax_surfacePixelHeight);
	pBody->surface.surfacePointSize			= LOCKED(wax_surfacePointSize);
	pBody->surface.surfacePointWidth		= LOCKED(wax_surfacePointWidth);
	pBody->surface.surfacePointHeight		= LOCKED(wax_surfacePointHeight);
	pBody->surface.surfaceScale				= LOCKED(wax_surfaceScale);
	pBody->surface.surfaceSampleMethod		= LOCKED(wax_surfaceSampleMethod);
	pBody->surface.surfaceIsTiling			= LOCKED(wax_surfaceIsTiling);
	pBody->surface.surfaceIsMipmapped		= LOCKED(wax_surfaceIsMipmapped);
	pBody->surface.surfaceAlpha				= LOCKED(wax_surfaceAlpha);
	pBody->surface.surfacePalette			= LOCKED(wax_surfacePalette);
	pBody->surface.surfacePaletteSize		= LOCKED(wax_surfacePaletteSize);
	pBody->surface.surfacePaletteCapacity	= LOCKED(wax_surfacePaletteCapacity);
	pBody->surface.surfacePixelDescription	= LOCKED(wax_surfacePixelDescription);
	pBody->surface.surfacePixelFormat		= LOCKED(wax_surfacePixelFormat);
	pBody->surface.surfacePixelBits			= LOCKED(wax_surfacePixelBits);
	pBody->surface.surfaceIsOpaque			= LOCKED(wax_surfaceIsOpaque);
	pBody->surface.surfaceCanBeCanvas		= LOCKED(wax_surfaceCanBeCanvas);
	pBody->surface.allocPixelBuffer			= LOCKED(wax_allocPixelBuffer);
	pBody->surface.allocPixelBufferFromRect	= LOCKED(wax_allocPixelBufferFromRect);
	pBody->surface.pushPixels				= LOCKED(wax_pushPixels);
	pBody->surface.pushPixelsFromRect		= LOCKED(wax_pushPixelsFromRect);
	pBody->surface.pullPixels				= LOCKED(wax_pullPixels);
	pBody->surface.pullPixelsFromRect		= LOCKED(wax_pullPixelsFromRect);
	pBody->surface.freePixelBuffer			= LOCKED(wax_freePixelBuffer);
	pBody->surface.fillSurface				= LOCKED(wax_fillSurface);
	pBody->surface.fillSurfaceRect			= LOCKED(wax_fillSurfaceRect);
	pBody->surface.copySurface				= LOCKED(wax_copySurface);
	pBody->surface.copySurfaceRect			= LOCKED(wax_copySurfaceRect);
	pBody->surface.setSurfaceBaggage		= LOCKED(wax_setSurfaceBaggage);
	pBody->surface.getSurfaceBaggage		= LOCKED(wax_getSurfaceBaggage);
	pBody->surface.addSurfaceObserver		= LOCKED(wax_addSurfaceObserver);
	pBody->surface.removeSurfaceObserver	= LOCKED(wax_removeSurfaceObserver);
	pBody->surface.getSurfaceBlueprint		= LOCKED(wax_getSurfaceBlueprint);
	pBody->surface.surfaceColorSpace		= LOCKED(wax_surfaceColorSpace);
	pBody->surface.surfaceIsBigEndian		= LOCKED(wax_surfaceIsBigEndian);


	pBody->surfaceFactory.structSize		= sizeof(wg_surfacefactory_calls);
	pBody->surfaceFactory.maxSurfaceSize	= LOCKED(wax_maxSurfaceSize);
	pBody->surfaceFactory.createSurface		= LOCKED(wax_createSurface);
	pBody->surfaceFactory.createSurfaceFromBlob = LOCKED(wax_createSurfaceFromBlob);
	pBody->surfaceFactory.createSurfaceFromBitmap = LOCKED(wax_createSurfaceFromBitmap);
	pBody->surfaceFactory.createSurfaceFromRawData = LOCKED(wax_createSurfaceFromRawData);

	pBody->edgemap.structSize				= sizeof(wg_edgemap_calls);
	pBody->edgemap.edgemapPixelSize			= LOCKED(wax_edgemapPixelSize);
	pBody->edgemap.setRenderSegments		= LOCKED(wax_setRenderSegments);
	pBody->edgemap.getRenderSegments		= LOCKED(wax_getRenderSegments);

	pBody->edgemap.edgemapPaletteType		= &stub_edgemapPaletteType;
	pBody->edgemap.setEdgemapColorsFromGradients = &stub_setEdgemapColorsFromGradients;
	pBody->edgemap.setEdgemapColorsFromTintmaps = &stub_setEdgemapColorsFromTintmaps;
	pBody->edgemap.setEdgemapColorsFromStrips = &stub_setEdgemapColorsFromStrips;
	pBody->edgemap.importEdgemapPaletteEntries = &stub_importEdgemapPaletteEntries;
	pBody->edgemap.edgemapColorstripsX		= &stub_edgemapColorstrips;
	pBody->edgemap.edgemapColorstripsY		= &stub_edgemapColorstrips;
	pBody->edgemap.importPaletteEntries		= &stub_importPaletteEntries;
	pBody->edgemap.setEdgemapColors			= LOCKED(wax_setEdgemapColors);
	pBody->edgemap.setEdgemapTints			= LOCKED(wax_setEdgemapTints);


	pBody->edgemap.edgemapFlatColors		= LOCKED(wax_edgemapFlatColors);
	pBody->edgemap.edgemapTint				= LOCKED(wax_edgemapTint);

	pBody->edgemap.edgemapSegments			= LOCKED(wax_edgemapSegments);
	pBody->edgemap.edgemapSamples			= LOCKED(wax_edgemapSamples);
	pBody->edgemap.edgemapIsTiling			= LOCKED(wax_edgemapIsTiling);
	pBody->edgemap.importSpxSamples			= LOCKED(wax_importSpxSamples);
	pBody->edgemap.importFloatSamples		= LOCKED(wax_importFloatSamples);
	pBody->edgemap.exportSpxSamples			= LOCKED(wax_exportSpxSamples);
	pBody->edgemap.exportFloatSamples		= LOCKED(wax_exportFloatSamples);
	pBody->edgemap.exportBounds				= LOCKED(wax_exportBounds);

	pBody->edgemapFactory.structSize		= sizeof(wg_edgemapfactory_calls);
	pBody->edgemapFactory.createEdgemap		= LOCKED(wax_createEdgemap);
	pBody->edgemapFactory.createEdgemapFromFloats = LOCKED(wax_createEdgemapFromFloats);
	pBody->edgemapFactory.createEdgemapFromSpx = LOCKED(wax_createEdgemapFromSpx);

	pBody->warpmap.structSize				= sizeof(wg_warpmap_calls);
	pBody->warpmap.warpmapPixelSize			= LOCKED(wax_warpmapPixelSize);
	pBody->warpmap.warpmapCellSize			= LOCKED(wax_warpmapCellSize);
	pBody->warpmap.setWarpmapPeriods		= LOCKED(wax_setWarpmapPeriods);
	pBody->warpmap.warpmapUPeriod			= LOCKED(wax_warpmapUPeriod);
	pBody->warpmap.warpmapVPeriod			= LOCKED(wax_warpmapVPeriod);
	pBody->warpmap.importWarpmapNodes		= LOCKED(wax_importWarpmapNodes);
	pBody->warpmap.exportWarpmapNodes		= LOCKED(wax_exportWarpmapNodes);

	pBody->warpmapFactory.structSize		= sizeof(wg_warpmapfactory_calls);
	pBody->warpmapFactory.createWarpmap		= LOCKED(wax_createWarpmap);
	pBody->warpmapFactory.createWarpmapFromNodes = LOCKED(wax_createWarpmapFromNodes);

	pBody->hostBridge.structSize			= sizeof(wg_hostbridge_calls);
	pBody->hostBridge.hidePointer			= LOCKED(wg_hidePointer);
	pBody->hostBridge.showPointer			= LOCKED(wg_showPointer);
	pBody->hostBridge.getClipboardText		= LOCKED(wg_getClipboardText);
	pBody->hostBridge.setClipboardText		= LOCKED(wg_setClipboardText);
	pBody->hostBridge.requestWindowFocus	= LOCKED(wg_requestWindowFocus);
	pBody->hostBridge.yieldWindowFocus		= LOCKED(wg_yieldWindowFocus);
	pBody->hostBridge.lockHidePointer		= LOCKED(wg_lockHidePointer);
	pBody->hostBridge.unlockShowPointer		= LOCKED(wg_unlockShowPointer);
	pBody->hostBridge.setPointerStyle		= LOCKED(wg_setPointerStyle);

	pBody->pluginCapsule.structSize			= sizeof(wg_plugincapsule_calls);
	pBody->pluginCapsule.requestRender 		= LOCKED(wg_pluginRequestRender);
	pBody->pluginCapsule.requestResize 		= LOCKED(wg_pluginRequestResize);
	pBody->pluginCapsule.isVisible 			= LOCKED(wg_isPluginVisible);
	pBody->pluginCapsule.windowSection 		= LOCKED(wg_pluginWindowSection);
	pBody->pluginCapsule.requestFocus 		= LOCKED(wg_pluginRequestFocus);
	pBody->pluginCapsule.releaseFocus 		= LOCKED(wg_pluginReleaseFocus);
	pBody->pluginCapsule.requestPreRenderCall = LOCKED(wg_pluginRequestPreRenderCall);
	pBody->pluginCapsule.requestInView		= LOCKED(wg_pluginRequestInView);
	pBody->pluginCapsule.connect			= LOCKED(wg_connectPlugin);
	pBody->pluginCapsule.disconnect			= LOCKED(wg_disconnectPlugin);

	pBody->blurbrush.structSize				= sizeof(wg_blurbrush_calls);
	pBody->blurbrush.create					= LOCKED(wax_createBlurbrush);
	pBody->blurbrush.size					= LOCKED(wax_blurbrushSize);
	pBody->blurbrush.blue					= LOCKED(wax_blurbrushBlue);
	pBody->blurbrush.green					= LOCKED(wax_blurbrushGreen);
	pBody->blurbrush.red					= LOCKED(wax_blurbrushRed);

	pBody->tintmap.structSize				= sizeof(wg_tintmap_calls);
	pBody->tintmap.isTintmapOpaque			= &stub_isTintmapFlag;
	pBody->tintmap.isTintmapVertical		= &stub_isTintmapFlag;
	pBody->tintmap.isTintmapHorizontal		= &stub_isTintmapFlag;
	pBody->tintmap.exportTintmapColors		= &stub_exportTintmapColors;

	pBody->gradyent.structSize				= sizeof(wg_gradyent_calls);
	pBody->gradyent.createGradyent			= &stub_createGradyent;

	pBody->staticTintmap.structSize			= sizeof(wg_statictintmap_calls);
	pBody->staticTintmap.createStaticTintmap = &stub_createStaticTintmap;

	pBody->tint.structSize					= sizeof(wg_tint_calls);
	pBody->tint.defaultTintBP				= LOCKED(wax_defaultTintBP);
	pBody->tint.createTint					= LOCKED(wax_createTint);
	pBody->tint.createTintFromData			= LOCKED(wax_createTintFromData);
	pBody->tint.createTintMix				= LOCKED(wax_createTintMix);
	pBody->tint.mixTints					= LOCKED(wax_mixTints);
	pBody->tint.isTintOpaque				= LOCKED(wax_isTintOpaque);
	pBody->tint.isTintFlat					= LOCKED(wax_isTintFlat);
	pBody->tint.isTintMix					= LOCKED(wax_isTintMix);
	pBody->tint.getTintBlueprint			= LOCKED(wax_getTintBlueprint);
	pBody->tint.tintMixComponents			= LOCKED(wax_tintMixComponents);
	pBody->tint.tintMixComponent			= LOCKED(wax_tintMixComponent);
	pBody->tint.tintMixWeight				= LOCKED(wax_tintMixWeight);
	pBody->tint.tintColorAt					= LOCKED(wax_tintColorAt);
	pBody->tint.tintAlphaAt					= LOCKED(wax_tintAlphaAt);
	pBody->tint.exportTintData				= LOCKED(wax_exportTintData);
}


void wg_populatePluginInterface(wg_plugin_interface * pHeader)
{
	std::call_once(bodyPopulated, populateBody, &body);

	auto pBody = &body;

	pHeader->structSize			= sizeof(wg_plugin_interface);
	pHeader->pBitmapCache		= &pBody->bitmapCache;
	pHeader->pBitmapFont		= &pBody->bitmapFont;
	pHeader->pCanvasLayers		= &pBody->canvasLayers;
	pHeader->pFont				= &pBody->font;
	pHeader->pObject			= &pBody->object;
	pHeader->pGfxDevice			= &pBody->gfxDevice;
/*
	pHeader->pStreamBuffer		= &pBody->streamBuffer;
	pHeader->pStreamPlayer		= &pBody->streamPlayer;
	pHeader->pStreamPump		= &pBody->streamPump;
	pHeader->pStreamReader		= &pBody->streamReader;
*/
	pHeader->pSurface			= &pBody->surface;
	pHeader->pSurfaceFactory	= &pBody->surfaceFactory;
	pHeader->pEdgemap			= &pBody->edgemap;
	pHeader->pEdgemapFactory	= &pBody->edgemapFactory;
	pHeader->pWarpmap			= &pBody->warpmap;
	pHeader->pWarpmapFactory	= &pBody->warpmapFactory;
	pHeader->pHostBridge		= &pBody->hostBridge;
	pHeader->pPluginCapsule		= &pBody->pluginCapsule;
	pHeader->pBlurbrush			= &pBody->blurbrush;
	pHeader->pTintmap			= &pBody->tintmap;
	pHeader->pGradyent			= &pBody->gradyent;
	pHeader->pStaticTintmap		= &pBody->staticTintmap;

	pHeader->pTint				= &pBody->tint;

}



