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

#include <wg_c_bitmapcache.h>
#include <wg_c_bitmapfont.h>
#include <wg_c_canvaslayers.h>
#include <wg_c_font.h>
#include <wg_c_gfxdevice.h>
/*
#include <wg_c_streambuffer.h>
#include <wg_c_streamplayer.h>
#include <wg_c_streampump.h>
#include <wg_c_streamreader.h>
*/
#include <wg_c_object.h>
#include <wg_c_surface.h>
#include <wg_c_surfacefactory.h>
#include <wg_c_edgemap.h>
#include <wg_c_edgemapfactory.h>
#include <wg_c_plugincapsule.h>

#include <wg_c_tintmap.h>
#include <wg_c_gradyent.h>
#include <wg_c_statictintmap.h>

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

	wg_tintmap_calls			tintmap;
	wg_gradyent_calls			gradyent;
	wg_statictintmap_calls		staticTintmap;
};

static wg_c_calls_body	body;
static std::once_flag	bodyPopulated;


// The body is shared by all headers and only ever holds the same constant function
// pointers, so it is filled once. wg_populatePluginInterface() may be called from
// several threads at once (plugins set up in parallel) while plugins read the body.

static void populateBody(wg_c_calls_body * pBody)
{
	pBody->bitmapCache.structSize			= sizeof(wg_bitmapcache_calls);
	pBody->bitmapCache.createBitmapCache	= LOCKED(wg_createBitmapCache);
	pBody->bitmapCache.setCacheLimit		= LOCKED(wg_setCacheLimit);
	pBody->bitmapCache.cacheLimit			= LOCKED(wg_cacheLimit);
	pBody->bitmapCache.cacheSize			= LOCKED(wg_cacheSize);
	pBody->bitmapCache.clearCache			= LOCKED(wg_clearCache);
	pBody->bitmapCache.addCacheListener		= LOCKED(wg_addCacheListener);
	pBody->bitmapCache.removeCacheListener	= LOCKED(wg_removeCacheListener);
	pBody->bitmapCache.getCacheSlot			= LOCKED(wg_getCacheSlot);
	pBody->bitmapCache.getNbCacheSurfaces	= LOCKED(wg_getNbCacheSurfaces);
	pBody->bitmapCache.getCacheSurfaces		= LOCKED(wg_getCacheSurfaces);


	pBody->bitmapFont.structSize			= sizeof(wg_bitmapfont_calls);
	pBody->bitmapFont.createBitmapFont		= LOCKED(wg_createBitmapFont);
	pBody->bitmapFont.getBitmapFontSurface	= LOCKED(wg_getBitmapFontSurface);


	pBody->canvasLayers.structSize			= sizeof(wg_canvaslayers_calls);
	pBody->canvasLayers.baseLayer			= LOCKED(wg_baseLayer);
	pBody->canvasLayers.canvasLayersSize	= LOCKED(wg_canvasLayersSize);
	pBody->canvasLayers.createCanvasLayers	= LOCKED(wg_createCanvasLayers);
	pBody->canvasLayers.layerFormat			= LOCKED(wg_layerFormat);

	pBody->font.structSize					= sizeof(wg_font_calls);
	pBody->font.setFontSize					= LOCKED(wg_setFontSize);
	pBody->font.fontSize					= LOCKED(wg_fontSize);
	pBody->font.getGlyphWithoutBitmap		= LOCKED(wg_getGlyphWithoutBitmap);
	pBody->font.getGlyphWithBitmap			= LOCKED(wg_getGlyphWithBitmap);
	pBody->font.getKerning					= LOCKED(wg_getKerning);
	pBody->font.lineGap						= LOCKED(wg_lineGap);
	pBody->font.whitespaceAdvance			= LOCKED(wg_whitespaceAdvance);
	pBody->font.maxAdvance					= LOCKED(wg_maxAdvance);
	pBody->font.maxAscend					= LOCKED(wg_maxAscend);
	pBody->font.maxDescend					= LOCKED(wg_maxDescend);
	pBody->font.nbGlyphs					= LOCKED(wg_nbGlyphs);
	pBody->font.hasGlyphs					= LOCKED(wg_hasGlyphs);
	pBody->font.hasGlyph					= LOCKED(wg_hasGlyph);
	pBody->font.isMonospace					= LOCKED(wg_isMonospace);
	pBody->font.isMonochrome				= LOCKED(wg_isMonochrome);
	pBody->font.getBackupFont				= LOCKED(wg_getBackupFont);
	pBody->font.createSurfaceFromGlyph		= LOCKED(wg_createSurfaceFromGlyph);


	pBody->gfxDevice.structSize				= sizeof(wg_gfxdevice_calls);
	pBody->gfxDevice.deviceSurfaceType		= nullptr;						// Hopefully this is not used anymore.
	pBody->gfxDevice.getCanvas				= LOCKED(wg_getCanvas);
	pBody->gfxDevice.getCanvasRef			= LOCKED(wg_getCanvasRef);
	pBody->gfxDevice.canvasLayers			= LOCKED(wg_canvasLayers);
	pBody->gfxDevice.surfaceFactory			= LOCKED(wg_surfaceFactory);
	pBody->gfxDevice.edgemapFactory			= LOCKED(wg_edgemapFactory);
	pBody->gfxDevice.maxSegments			= LOCKED(wg_maxSegments);
	pBody->gfxDevice.canvasSize				= LOCKED(wg_canvasSize);
	pBody->gfxDevice.setClipList			= LOCKED(wg_setClipList);
	pBody->gfxDevice.resetClipList			= LOCKED(wg_resetClipList);
	pBody->gfxDevice.pushClipList			= LOCKED(wg_pushClipList);
	pBody->gfxDevice.popClipList			= LOCKED(wg_popClipList);
	pBody->gfxDevice.getClipList			= LOCKED(wg_getClipList);
	pBody->gfxDevice.clipListSize			= LOCKED(wg_clipListSize);
	pBody->gfxDevice.clipBounds				= LOCKED(wg_clipBounds);
	pBody->gfxDevice.setTintColor			= LOCKED(wg_setTintColor);
	pBody->gfxDevice.getTintColor			= LOCKED(wg_getTintColor);

	pBody->gfxDevice.setTintmap				= LOCKED(wg_setTintmap);
	pBody->gfxDevice.getTintmap				= LOCKED(wg_getTintmap);
	pBody->gfxDevice.getTintmapRect			= LOCKED(wg_getTintmapRect);
	pBody->gfxDevice.clearTintmap			= LOCKED(wg_clearTintmap);
	pBody->gfxDevice.hasTintmap				= LOCKED(wg_hasTintmap);

	pBody->gfxDevice.setTintGradient		= LOCKED(wg_setTintGradient);
	pBody->gfxDevice.clearTintGradient		= LOCKED(wg_clearTintGradient);
	pBody->gfxDevice.setBlendMode			= LOCKED(wg_setBlendMode);
	pBody->gfxDevice.getBlendMode			= LOCKED(wg_getBlendMode);
	pBody->gfxDevice.setBlitSource			= LOCKED(wg_setBlitSource);
	pBody->gfxDevice.getBlitSource			= LOCKED(wg_getBlitSource);
	pBody->gfxDevice.setMorphFactor			= LOCKED(wg_setMorphFactor);
	pBody->gfxDevice.getMorphFactor			= LOCKED(wg_getMorphFactor);
	pBody->gfxDevice.setRenderLayer			= LOCKED(wg_setRenderLayer);
	pBody->gfxDevice.getRenderLayer			= LOCKED(wg_getRenderLayer);
	pBody->gfxDevice.beginRender			= LOCKED(wg_beginRender);
	pBody->gfxDevice.endRender				= LOCKED(wg_endRender);
	pBody->gfxDevice.isDeviceRendering		= LOCKED(wg_isDeviceRendering);
	pBody->gfxDevice.isDeviceIdle			= LOCKED(wg_isDeviceIdle);
	pBody->gfxDevice.flushDevice			= LOCKED(wg_flushDevice);
	pBody->gfxDevice.beginCanvasUpdateWithRef		= LOCKED(wg_beginCanvasUpdateWithRef);
	pBody->gfxDevice.beginCanvasUpdateWithSurface	= LOCKED(wg_beginCanvasUpdateWithSurface);
	pBody->gfxDevice.endCanvasUpdate		= LOCKED(wg_endCanvasUpdate);
	pBody->gfxDevice.fill					= LOCKED(wg_fill);
	pBody->gfxDevice.fillRect				= LOCKED(wg_fillRect);
	pBody->gfxDevice.plotPixels				= nullptr;				// Plot pixels has been deprecated.
	pBody->gfxDevice.drawLine				= LOCKED(wg_drawLine);
	pBody->gfxDevice.drawStraightLine		= LOCKED(wg_drawStraightLine);
	pBody->gfxDevice.blit					= LOCKED(wg_blit);
	pBody->gfxDevice.blitRect				= LOCKED(wg_blitRect);
	pBody->gfxDevice.flipBlit				= LOCKED(wg_flipBlit);
	pBody->gfxDevice.flipBlitRect			= LOCKED(wg_flipBlitRect);
	pBody->gfxDevice.stretchBlit			= LOCKED(wg_stretchBlit);
	pBody->gfxDevice.stretchBlitRect		= LOCKED(wg_stretchBlitRect);
	pBody->gfxDevice.stretchFlipBlit		= LOCKED(wg_stretchFlipBlit);
	pBody->gfxDevice.stretchFlipBlitRect	= LOCKED(wg_stretchFlipBlitRect);
	pBody->gfxDevice.precisionBlit			= LOCKED(wg_precisionBlit);
	pBody->gfxDevice.transformBlit			= LOCKED(wg_transformBlit);
	pBody->gfxDevice.rotScaleBlit			= LOCKED(wg_rotScaleBlit);
	pBody->gfxDevice.tile					= LOCKED(wg_tile);
	pBody->gfxDevice.flipTile				= LOCKED(wg_flipTile);
	pBody->gfxDevice.scaleTile				= LOCKED(wg_scaleTile);
	pBody->gfxDevice.scaleFlipTile			= LOCKED(wg_scaleFlipTile);
	pBody->gfxDevice.drawWave				= LOCKED(wg_drawWave);
	pBody->gfxDevice.flipDrawWave			= LOCKED(wg_flipDrawWave);
	pBody->gfxDevice.drawElipse				= LOCKED(wg_drawElipse);
	pBody->gfxDevice.drawPieChart			= LOCKED(wg_drawPieChart);
	pBody->gfxDevice.drawSegments			= LOCKED(wg_drawSegments);
	pBody->gfxDevice.flipDrawSegments		= LOCKED(wg_flipDrawSegments);
	pBody->gfxDevice.drawEdgemap			= LOCKED(wg_drawEdgemap);
	pBody->gfxDevice.flipDrawEdgemap		= LOCKED(wg_flipDrawEdgemap);
	pBody->gfxDevice.blitNinePatch			= LOCKED(wg_blitNinePatch);

	pBody->gfxDevice.setBlurMatrices		= LOCKED(wg_setBlurMatrices);
	pBody->gfxDevice.setFixedBlendColor		= LOCKED(wg_setFixedBlendColor);
	pBody->gfxDevice.getFixedBlendColor		= LOCKED(wg_getFixedBlendColor);
	
	pBody->gfxDevice.blur					= LOCKED(wg_blur);
	pBody->gfxDevice.blurRect				= LOCKED(wg_blurRect);
	pBody->gfxDevice.stretchBlur			= LOCKED(wg_stretchBlur);
	pBody->gfxDevice.stretchBlurRect		= LOCKED(wg_stretchBlurRect);
	pBody->gfxDevice.transformBlur			= LOCKED(wg_transformBlur);
	pBody->gfxDevice.rotScaleBlur			= LOCKED(wg_rotScaleBlur);

	pBody->gfxDevice.setBlurbrush			= LOCKED(wg_setBlurbrush);

/*
	pBody->streamBuffer.structSize				= sizeof(wg_streambuffer_calls);
	pBody->streamBuffer.createStreamBuffer		= LOCKED(wg_createStreamBuffer);
	pBody->streamBuffer.getStreamBufferOutput	= LOCKED(wg_getStreamBufferOutput);
	pBody->streamBuffer.getStreamBufferInput	= LOCKED(wg_getStreamBufferInput);
	pBody->streamBuffer.streamBufferCapacity	= LOCKED(wg_streamBufferCapacity);
	pBody->streamBuffer.streamBufferHasChunk	= LOCKED(wg_streamBufferHasChunk);
	pBody->streamBuffer.streamBufferBytes		= LOCKED(wg_streamBufferBytes);


	pBody->streamPlayer.structSize			= sizeof(wg_streamplayer_calls);
	pBody->streamPlayer.createStreamPlayer	= LOCKED(wg_createStreamPlayer);
	pBody->streamPlayer.getStreamPlayerInput	= LOCKED(wg_getStreamPlayerInput);
	pBody->streamPlayer.setStreamPlayerStoreDirtyRects = LOCKED(wg_setStreamPlayerStoreDirtyRects);
	pBody->streamPlayer.setStreamPlayerMaxDirtyRects = LOCKED(wg_setStreamPlayerMaxDirtyRects);
	pBody->streamPlayer.getStreamPlayerDirtyRects = LOCKED(wg_getStreamPlayerDirtyRects);
	pBody->streamPlayer.clearStreamPlayerDirtyRects = LOCKED(wg_clearStreamPlayerDirtyRects);


	pBody->streamPump.structSize			= sizeof(wg_streampump_calls);
	pBody->streamPump.createStreamPump		= LOCKED(wg_createStreamPump);
	pBody->streamPump.createStreamPumpWithInputOutput = LOCKED(wg_createStreamPumpWithInputOutput);
	pBody->streamPump.setStreamPumpInput	= LOCKED(wg_setStreamPumpInput);
	pBody->streamPump.setStreamPumpOutput	= LOCKED(wg_setStreamPumpOutput);
	pBody->streamPump.peekChunk				= LOCKED(wg_peekChunk);
	pBody->streamPump.pumpChunk				= LOCKED(wg_pumpChunk);
	pBody->streamPump.pumpUntilFrame		= LOCKED(wg_pumpUntilFrame);
	pBody->streamPump.pumpFrame				= LOCKED(wg_pumpFrame);
	pBody->streamPump.pumpAll				= LOCKED(wg_pumpAll);


	pBody->streamReader.structSize				= sizeof(wg_streamreader_calls);
	pBody->streamReader.createStreamReader		= LOCKED(wg_createStreamReader);
	pBody->streamReader.getStreamReaderOutput	= LOCKED(wg_getStreamReaderOutput);
	pBody->streamReader.streamReaderCapacity	= LOCKED(wg_streamReaderCapacity);
	pBody->streamReader.streamReaderHasChunk	= LOCKED(wg_streamReaderHasChunk);
	pBody->streamReader.streamReaderBytes		= LOCKED(wg_streamReaderBytes);
*/


	pBody->object.structSize				= sizeof(wg_object_calls);
	pBody->object.finalizer					= LOCKED(wg_finalizer);
	pBody->object.getTypeInfo				= LOCKED(wg_getTypeInfo);
	pBody->object.isInstanceOf				= LOCKED(wg_isInstanceOf);
	pBody->object.refcount					= LOCKED(wg_refcount);
	pBody->object.release					= LOCKED(wg_release);
	pBody->object.retain					= LOCKED(wg_retain);
	pBody->object.setFinalizer				= LOCKED(wg_setFinalizer);


	pBody->surface.structSize				= sizeof(wg_surface_calls);
	pBody->surface.setSurfaceIdentity		= LOCKED(wg_setSurfaceIdentity);
	pBody->surface.getSurfaceIdentity		= LOCKED(wg_getSurfaceIdentity);
	pBody->surface.surfacePixelSize			= LOCKED(wg_surfacePixelSize);
	pBody->surface.surfacePixelWidth		= LOCKED(wg_surfacePixelWidth);
	pBody->surface.surfacePixelHeight		= LOCKED(wg_surfacePixelHeight);
	pBody->surface.surfacePointSize			= LOCKED(wg_surfacePointSize);
	pBody->surface.surfacePointWidth		= LOCKED(wg_surfacePointWidth);
	pBody->surface.surfacePointHeight		= LOCKED(wg_surfacePointHeight);
	pBody->surface.surfaceScale				= LOCKED(wg_surfaceScale);
	pBody->surface.surfaceSampleMethod		= LOCKED(wg_surfaceSampleMethod);
	pBody->surface.surfaceIsTiling			= LOCKED(wg_surfaceIsTiling);
	pBody->surface.surfaceIsMipmapped		= LOCKED(wg_surfaceIsMipmapped);
	pBody->surface.surfaceAlpha				= LOCKED(wg_surfaceAlpha);
	pBody->surface.surfacePalette			= LOCKED(wg_surfacePalette);
	pBody->surface.surfacePaletteSize		= LOCKED(wg_surfacePaletteSize);
	pBody->surface.surfacePaletteCapacity	= LOCKED(wg_surfacePaletteCapacity);
	pBody->surface.surfacePixelDescription	= LOCKED(wg_surfacePixelDescription);
	pBody->surface.surfacePixelFormat		= LOCKED(wg_surfacePixelFormat);
	pBody->surface.surfacePixelBits			= LOCKED(wg_surfacePixelBits);
	pBody->surface.surfaceIsOpaque			= LOCKED(wg_surfaceIsOpaque);
	pBody->surface.surfaceCanBeCanvas		= LOCKED(wg_surfaceCanBeCanvas);
	pBody->surface.allocPixelBuffer			= LOCKED(wg_allocPixelBuffer);
	pBody->surface.allocPixelBufferFromRect	= LOCKED(wg_allocPixelBufferFromRect);
	pBody->surface.pushPixels				= LOCKED(wg_pushPixels);
	pBody->surface.pushPixelsFromRect		= LOCKED(wg_pushPixelsFromRect);
	pBody->surface.pullPixels				= LOCKED(wg_pullPixels);
	pBody->surface.pullPixelsFromRect		= LOCKED(wg_pullPixelsFromRect);
	pBody->surface.freePixelBuffer			= LOCKED(wg_freePixelBuffer);
	pBody->surface.fillSurface				= LOCKED(wg_fillSurface);
	pBody->surface.fillSurfaceRect			= LOCKED(wg_fillSurfaceRect);
	pBody->surface.copySurface				= LOCKED(wg_copySurface);
	pBody->surface.copySurfaceRect			= LOCKED(wg_copySurfaceRect);
	pBody->surface.setSurfaceBaggage		= LOCKED(wg_setSurfaceBaggage);
	pBody->surface.getSurfaceBaggage		= LOCKED(wg_getSurfaceBaggage);
	pBody->surface.addSurfaceObserver		= LOCKED(wg_addSurfaceObserver);
	pBody->surface.removeSurfaceObserver	= LOCKED(wg_removeSurfaceObserver);
	pBody->surface.getSurfaceBlueprint		= LOCKED(wg_getSurfaceBlueprint);


	pBody->surfaceFactory.structSize		= sizeof(wg_surfacefactory_calls);
	pBody->surfaceFactory.maxSurfaceSize	= LOCKED(wg_maxSurfaceSize);
	pBody->surfaceFactory.createSurface		= LOCKED(wg_createSurface);
	pBody->surfaceFactory.createSurfaceFromBlob = LOCKED(wg_createSurfaceFromBlob);
	pBody->surfaceFactory.createSurfaceFromBitmap = LOCKED(wg_createSurfaceFromBitmap);
	pBody->surfaceFactory.createSurfaceFromRawData = LOCKED(wg_createSurfaceFromRawData);

	pBody->edgemap.structSize				= sizeof(wg_edgemap_calls);
	pBody->edgemap.edgemapPixelSize			= LOCKED(wg_edgemapPixelSize);
	pBody->edgemap.setRenderSegments		= LOCKED(wg_setRenderSegments);
	pBody->edgemap.getRenderSegments		= LOCKED(wg_getRenderSegments);

	pBody->edgemap.edgemapPaletteType		= LOCKED(wg_edgemapPaletteType);
	pBody->edgemap.setEdgemapColors			= LOCKED(wg_setEdgemapColors);
	pBody->edgemap.setEdgemapColorsFromGradients = LOCKED(wg_setEdgemapColorsFromGradients);
	pBody->edgemap.setEdgemapColorsFromTintmaps = LOCKED(wg_setEdgemapColorsFromTintmaps);
	pBody->edgemap.setEdgemapColorsFromStrips = LOCKED(wg_setEdgemapColorsFromStrips);

	pBody->edgemap.importEdgemapPaletteEntries = LOCKED(wg_importEdgemapPaletteEntries);

	pBody->edgemap.edgemapFlatColors		= LOCKED(wg_edgemapFlatColors);
	pBody->edgemap.edgemapColorstripsX		= LOCKED(wg_edgemapColorstripsX);
	pBody->edgemap.edgemapColorstripsY		= LOCKED(wg_edgemapColorstripsY);

	pBody->edgemap.edgemapSegments			= LOCKED(wg_edgemapSegments);
	pBody->edgemap.edgemapSamples			= LOCKED(wg_edgemapSamples);
	pBody->edgemap.importSpxSamples			= LOCKED(wg_importSpxSamples);
	pBody->edgemap.importFloatSamples		= LOCKED(wg_importFloatSamples);
	pBody->edgemap.exportSpxSamples			= LOCKED(wg_exportSpxSamples);
	pBody->edgemap.exportFloatSamples		= LOCKED(wg_exportFloatSamples);
	pBody->edgemap.importPaletteEntries		= LOCKED(wg_importEdgemapPaletteEntries);
	pBody->edgemap.exportBounds				= LOCKED(wg_exportBounds);

	pBody->edgemapFactory.structSize		= sizeof(wg_edgemapfactory_calls);
	pBody->edgemapFactory.createEdgemap		= LOCKED(wg_createEdgemap);
	pBody->edgemapFactory.createEdgemapFromFloats = LOCKED(wg_createEdgemapFromFloats);
	pBody->edgemapFactory.createEdgemapFromSpx = LOCKED(wg_createEdgemapFromSpx);

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
	pBody->blurbrush.create					= LOCKED(wg_createBlurbrush);
	pBody->blurbrush.size					= LOCKED(wg_blurbrushSize);
	pBody->blurbrush.blue					= LOCKED(wg_blurbrushBlue);
	pBody->blurbrush.green					= LOCKED(wg_blurbrushGreen);
	pBody->blurbrush.red					= LOCKED(wg_blurbrushRed);

	pBody->tintmap.structSize				= sizeof(wg_tintmap_calls);
	pBody->tintmap.exportTintmapColors		= LOCKED(wg_exportTintmapColors);
	pBody->tintmap.isTintmapHorizontal		= LOCKED(wg_isTintmapHorizontal);
	pBody->tintmap.isTintmapVertical		= LOCKED(wg_isTintmapVertical);
	pBody->tintmap.isTintmapOpaque			= LOCKED(wg_isTintmapOpaque);

	pBody->gradyent.structSize				= sizeof(wg_gradyent_calls);
	pBody->gradyent.createGradyent			= LOCKED(wg_createGradyent);

	pBody->staticTintmap.structSize			= sizeof(wg_statictintmap_calls);
	pBody->staticTintmap.createStaticTintmap= LOCKED(wg_createStaticTintmap);
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
	pHeader->pHostBridge		= &pBody->hostBridge;
	pHeader->pPluginCapsule		= &pBody->pluginCapsule;
	pHeader->pBlurbrush			= &pBody->blurbrush;

	pHeader->pTintmap			= &pBody->tintmap;
	pHeader->pGradyent			= &pBody->gradyent;
	pHeader->pStaticTintmap		= &pBody->staticTintmap;

}



