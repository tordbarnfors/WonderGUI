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
#include <wax_c_bitmap.h>
#include <wax_c_bitmapfactory.h>
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
	wg_bitmap_calls			bitmap;
	wg_bitmapfactory_calls		bitmapFactory;
	wg_edgemap_calls			edgemap;
	wg_edgemapfactory_calls		edgemapFactory;
	wg_hostbridge_calls			hostBridge;
	wg_plugincapsule_calls		pluginCapsule;
	wg_blurbrush_calls			blurbrush;
	wg_tint_calls				tint;
	wg_warpmap_calls			warpmap;
	wg_warpmapfactory_calls		warpmapFactory;
};

static wg_c_calls_body	body;
static std::once_flag	bodyPopulated;

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
	pBody->bitmapCache.getNbCacheBitmaps	= LOCKED(wax_getNbCacheBitmaps);
	pBody->bitmapCache.getCacheBitmaps		= LOCKED(wax_getCacheBitmaps);


	pBody->bitmapFont.structSize			= sizeof(wg_bitmapfont_calls);
	pBody->bitmapFont.createBitmapFont		= LOCKED(wax_createBitmapFont);
	pBody->bitmapFont.getBitmapFontBitmap	= LOCKED(wax_getBitmapFontBitmap);


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
	pBody->font.createBitmapFromGlyph		= LOCKED(wax_createBitmapFromGlyph);


	pBody->gfxDevice.structSize				= sizeof(wg_gfxdevice_calls);
	pBody->gfxDevice.deviceBitmapType		= nullptr;						// Hopefully this is not used anymore.
	pBody->gfxDevice.getCanvas				= LOCKED(wax_getCanvas);
	pBody->gfxDevice.getCanvasRef			= LOCKED(wax_getCanvasRef);
	pBody->gfxDevice.canvasLayers			= LOCKED(wax_canvasLayers);
	pBody->gfxDevice.bitmapFactory			= LOCKED(wax_bitmapFactory);
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
	pBody->gfxDevice.beginCanvasUpdateWithBitmap	= LOCKED(wax_beginCanvasUpdateWithBitmap);
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


	pBody->bitmap.structSize				= sizeof(wg_bitmap_calls);
	pBody->bitmap.setBitmapIdentity		= LOCKED(wax_setBitmapIdentity);
	pBody->bitmap.getBitmapIdentity		= LOCKED(wax_getBitmapIdentity);
	pBody->bitmap.bitmapPixelSize			= LOCKED(wax_bitmapPixelSize);
	pBody->bitmap.bitmapPixelWidth		= LOCKED(wax_bitmapPixelWidth);
	pBody->bitmap.bitmapPixelHeight		= LOCKED(wax_bitmapPixelHeight);
	pBody->bitmap.bitmapPointSize			= LOCKED(wax_bitmapPointSize);
	pBody->bitmap.bitmapPointWidth		= LOCKED(wax_bitmapPointWidth);
	pBody->bitmap.bitmapPointHeight		= LOCKED(wax_bitmapPointHeight);
	pBody->bitmap.bitmapScale				= LOCKED(wax_bitmapScale);
	pBody->bitmap.bitmapSampleMethod		= LOCKED(wax_bitmapSampleMethod);
	pBody->bitmap.bitmapIsTiling			= LOCKED(wax_bitmapIsTiling);
	pBody->bitmap.bitmapIsMipmapped		= LOCKED(wax_bitmapIsMipmapped);
	pBody->bitmap.bitmapAlpha				= LOCKED(wax_bitmapAlpha);
	pBody->bitmap.bitmapPalette			= LOCKED(wax_bitmapPalette);
	pBody->bitmap.bitmapPaletteSize		= LOCKED(wax_bitmapPaletteSize);
	pBody->bitmap.bitmapPaletteCapacity	= LOCKED(wax_bitmapPaletteCapacity);
	pBody->bitmap.bitmapPixelDescription	= LOCKED(wax_bitmapPixelDescription);
	pBody->bitmap.bitmapPixelFormat		= LOCKED(wax_bitmapPixelFormat);
	pBody->bitmap.bitmapPixelBits			= LOCKED(wax_bitmapPixelBits);
	pBody->bitmap.bitmapIsOpaque			= LOCKED(wax_bitmapIsOpaque);
	pBody->bitmap.bitmapCanBeCanvas		= LOCKED(wax_bitmapCanBeCanvas);
	pBody->bitmap.allocPixelBuffer			= LOCKED(wax_allocPixelBuffer);
	pBody->bitmap.allocPixelBufferFromRect	= LOCKED(wax_allocPixelBufferFromRect);
	pBody->bitmap.pushPixels				= LOCKED(wax_pushPixels);
	pBody->bitmap.pushPixelsFromRect		= LOCKED(wax_pushPixelsFromRect);
	pBody->bitmap.pullPixels				= LOCKED(wax_pullPixels);
	pBody->bitmap.pullPixelsFromRect		= LOCKED(wax_pullPixelsFromRect);
	pBody->bitmap.freePixelBuffer			= LOCKED(wax_freePixelBuffer);
	pBody->bitmap.fillBitmap				= LOCKED(wax_fillBitmap);
	pBody->bitmap.fillBitmapRect			= LOCKED(wax_fillBitmapRect);
	pBody->bitmap.copyBitmap				= LOCKED(wax_copyBitmap);
	pBody->bitmap.copyBitmapRect			= LOCKED(wax_copyBitmapRect);
	pBody->bitmap.setBitmapUserData		= LOCKED(wax_setBitmapUserData);
	pBody->bitmap.getBitmapUserData		= LOCKED(wax_getBitmapUserData);
	pBody->bitmap.addBitmapObserver		= LOCKED(wax_addBitmapObserver);
	pBody->bitmap.removeBitmapObserver	= LOCKED(wax_removeBitmapObserver);
	pBody->bitmap.getBitmapBlueprint		= LOCKED(wax_getBitmapBlueprint);
	pBody->bitmap.bitmapColorSpace		= LOCKED(wax_bitmapColorSpace);
	pBody->bitmap.bitmapIsBigEndian		= LOCKED(wax_bitmapIsBigEndian);
	pBody->bitmap.bitmapIsContentLost		= LOCKED(wax_bitmapIsContentLost);
	pBody->bitmap.dropBitmapRestorer		= LOCKED(wax_dropBitmapRestorer);


	pBody->bitmapFactory.structSize		= sizeof(wg_bitmapfactory_calls);
	pBody->bitmapFactory.maxBitmapSize	= LOCKED(wax_maxBitmapSize);
	pBody->bitmapFactory.createBitmap		= LOCKED(wax_createBitmap);
	pBody->bitmapFactory.createBitmapFromBlob = LOCKED(wax_createBitmapFromBlob);
	pBody->bitmapFactory.createBitmapFromBitmap = LOCKED(wax_createBitmapFromBitmap);
	pBody->bitmapFactory.createBitmapFromRawData = LOCKED(wax_createBitmapFromRawData);

	pBody->edgemap.structSize				= sizeof(wg_edgemap_calls);
	pBody->edgemap.edgemapPixelSize			= LOCKED(wax_edgemapPixelSize);
	pBody->edgemap.setRenderSegments		= LOCKED(wax_setRenderSegments);
	pBody->edgemap.getRenderSegments		= LOCKED(wax_getRenderSegments);

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
	pBody->warpmap.warpmapVertexCount		= LOCKED(wax_warpmapVertexCount);
	pBody->warpmap.warpmapTriangleCount		= LOCKED(wax_warpmapTriangleCount);
	pBody->warpmap.warpmapUPeriod			= LOCKED(wax_warpmapUPeriod);
	pBody->warpmap.warpmapVPeriod			= LOCKED(wax_warpmapVPeriod);
	pBody->warpmap.setWarpmapPeriods		= LOCKED(wax_setWarpmapPeriods);
	pBody->warpmap.importWarpmapVertices	= LOCKED(wax_importWarpmapVertices);
	pBody->warpmap.importWarpmapTriangles	= LOCKED(wax_importWarpmapTriangles);
	pBody->warpmap.importWarpmapMesh		= LOCKED(wax_importWarpmapMesh);
	pBody->warpmap.exportWarpmapVertices	= LOCKED(wax_exportWarpmapVertices);
	pBody->warpmap.exportWarpmapTriangles	= LOCKED(wax_exportWarpmapTriangles);

	pBody->warpmapFactory.structSize		= sizeof(wg_warpmapfactory_calls);
	pBody->warpmapFactory.createWarpmap		= LOCKED(wax_createWarpmap);
	pBody->warpmapFactory.createWarpmapFromMesh = LOCKED(wax_createWarpmapFromMesh);

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
	pHeader->pBitmap			= &pBody->bitmap;
	pHeader->pBitmapFactory	= &pBody->bitmapFactory;
	pHeader->pEdgemap			= &pBody->edgemap;
	pHeader->pEdgemapFactory	= &pBody->edgemapFactory;
	pHeader->pWarpmap			= &pBody->warpmap;
	pHeader->pWarpmapFactory	= &pBody->warpmapFactory;
	pHeader->pHostBridge		= &pBody->hostBridge;
	pHeader->pPluginCapsule		= &pBody->pluginCapsule;
	pHeader->pBlurbrush			= &pBody->blurbrush;

	pHeader->pTint				= &pBody->tint;

}



