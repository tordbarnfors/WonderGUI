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
};

static wg_c_calls_body	body;



void wg_populatePluginInterface(wg_plugin_interface * pHeader)
{
	auto pBody = &body;

	pBody->bitmapCache.structSize			= sizeof(wg_bitmapcache_calls);
	pBody->bitmapCache.createBitmapCache	= &wg_createBitmapCache;
	pBody->bitmapCache.setCacheLimit		= &wg_setCacheLimit;
	pBody->bitmapCache.cacheLimit			= &wg_cacheLimit;
	pBody->bitmapCache.cacheSize			= &wg_cacheSize;
	pBody->bitmapCache.clearCache			= &wg_clearCache;
	pBody->bitmapCache.addCacheListener		= &wg_addCacheListener;
	pBody->bitmapCache.removeCacheListener	= &wg_removeCacheListener;
	pBody->bitmapCache.getCacheSlot			= &wg_getCacheSlot;
	pBody->bitmapCache.getNbCacheSurfaces	= &wg_getNbCacheSurfaces;
	pBody->bitmapCache.getCacheSurfaces		= &wg_getCacheSurfaces;


	pBody->bitmapFont.structSize			= sizeof(wg_bitmapfont_calls);
	pBody->bitmapFont.createBitmapFont		= &wg_createBitmapFont;
	pBody->bitmapFont.getBitmapFontSurface	= &wg_getBitmapFontSurface;


	pBody->canvasLayers.structSize			= sizeof(wg_canvaslayers_calls);
	pBody->canvasLayers.baseLayer			= &wg_baseLayer;
	pBody->canvasLayers.canvasLayersSize	= &wg_canvasLayersSize;
	pBody->canvasLayers.createCanvasLayers	= &wg_createCanvasLayers;
	pBody->canvasLayers.layerFormat			= &wg_layerFormat;

	pBody->font.structSize					= sizeof(wg_font_calls);
	pBody->font.setFontSize					= &wg_setFontSize;
	pBody->font.fontSize					= &wg_fontSize;
	pBody->font.getGlyphWithoutBitmap		= &wg_getGlyphWithoutBitmap;
	pBody->font.getGlyphWithBitmap			= &wg_getGlyphWithBitmap;
	pBody->font.getKerning					= &wg_getKerning;
	pBody->font.lineGap						= &wg_lineGap;
	pBody->font.whitespaceAdvance			= &wg_whitespaceAdvance;
	pBody->font.maxAdvance					= &wg_maxAdvance;
	pBody->font.maxAscend					= &wg_maxAscend;
	pBody->font.maxDescend					= &wg_maxDescend;
	pBody->font.nbGlyphs					= &wg_nbGlyphs;
	pBody->font.hasGlyphs					= &wg_hasGlyphs;
	pBody->font.hasGlyph					= &wg_hasGlyph;
	pBody->font.isMonospace					= &wg_isMonospace;
	pBody->font.isMonochrome				= &wg_isMonochrome;
	pBody->font.getBackupFont				= &wg_getBackupFont;
	pBody->font.createSurfaceFromGlyph		= &wg_createSurfaceFromGlyph;


	pBody->gfxDevice.structSize				= sizeof(wg_gfxdevice_calls);
	pBody->gfxDevice.deviceSurfaceType		= &wg_deviceSurfaceType;
	pBody->gfxDevice.getCanvas				= &wg_getCanvas;
	pBody->gfxDevice.getCanvasRef			= &wg_getCanvasRef;
	pBody->gfxDevice.canvasLayers			= &wg_canvasLayers;
	pBody->gfxDevice.surfaceFactory			= &wg_surfaceFactory;
	pBody->gfxDevice.edgemapFactory			= &wg_edgemapFactory;
	pBody->gfxDevice.maxSegments			= &wg_maxSegments;
	pBody->gfxDevice.canvasSize				= &wg_canvasSize;
	pBody->gfxDevice.setClipList			= &wg_setClipList;
	pBody->gfxDevice.resetClipList			= &wg_resetClipList;
	pBody->gfxDevice.pushClipList			= &wg_pushClipList;
	pBody->gfxDevice.popClipList			= &wg_popClipList;
	pBody->gfxDevice.getClipList			= &wg_getClipList;
	pBody->gfxDevice.clipListSize			= &wg_clipListSize;
	pBody->gfxDevice.clipBounds				= &wg_clipBounds;
	pBody->gfxDevice.setTintColor			= &wg_setTintColor;
	pBody->gfxDevice.getTintColor			= &wg_getTintColor;
	pBody->gfxDevice.setTintGradient		= &wg_setTintGradient;
	pBody->gfxDevice.clearTintGradient		= &wg_clearTintGradient;
	pBody->gfxDevice.setBlendMode			= &wg_setBlendMode;
	pBody->gfxDevice.getBlendMode			= &wg_getBlendMode;
	pBody->gfxDevice.setBlitSource			= &wg_setBlitSource;
	pBody->gfxDevice.getBlitSource			= &wg_getBlitSource;
	pBody->gfxDevice.setMorphFactor			= &wg_setMorphFactor;
	pBody->gfxDevice.getMorphFactor			= &wg_getMorphFactor;
	pBody->gfxDevice.setRenderLayer			= &wg_setRenderLayer;
	pBody->gfxDevice.getRenderLayer			= &wg_getRenderLayer;
	pBody->gfxDevice.beginRender			= &wg_beginRender;
	pBody->gfxDevice.endRender				= &wg_endRender;
	pBody->gfxDevice.isDeviceRendering		= &wg_isDeviceRendering;
	pBody->gfxDevice.isDeviceIdle			= &wg_isDeviceIdle;
	pBody->gfxDevice.flushDevice			= &wg_flushDevice;
	pBody->gfxDevice.beginCanvasUpdateWithRef		= &wg_beginCanvasUpdateWithRef;
	pBody->gfxDevice.beginCanvasUpdateWithSurface	= &wg_beginCanvasUpdateWithSurface;
	pBody->gfxDevice.endCanvasUpdate		= &wg_endCanvasUpdate;
	pBody->gfxDevice.fill					= &wg_fill;
	pBody->gfxDevice.fillRect				= &wg_fillRect;
	pBody->gfxDevice.plotPixels				= nullptr;				// Plot pixels has been deprecated.
	pBody->gfxDevice.drawLine				= &wg_drawLine;
	pBody->gfxDevice.drawStraightLine		= &wg_drawStraightLine;
	pBody->gfxDevice.blit					= &wg_blit;
	pBody->gfxDevice.blitRect				= &wg_blitRect;
	pBody->gfxDevice.flipBlit				= &wg_flipBlit;
	pBody->gfxDevice.flipBlitRect			= &wg_flipBlitRect;
	pBody->gfxDevice.stretchBlit			= &wg_stretchBlit;
	pBody->gfxDevice.stretchBlitRect		= &wg_stretchBlitRect;
	pBody->gfxDevice.stretchFlipBlit		= &wg_stretchFlipBlit;
	pBody->gfxDevice.stretchFlipBlitRect	= &wg_stretchFlipBlitRect;
	pBody->gfxDevice.precisionBlit			= &wg_precisionBlit;
	pBody->gfxDevice.transformBlit			= &wg_transformBlit;
	pBody->gfxDevice.rotScaleBlit			= &wg_rotScaleBlit;
	pBody->gfxDevice.tile					= &wg_tile;
	pBody->gfxDevice.flipTile				= &wg_flipTile;
	pBody->gfxDevice.scaleTile				= &wg_scaleTile;
	pBody->gfxDevice.scaleFlipTile			= &wg_scaleFlipTile;
	pBody->gfxDevice.drawWave				= &wg_drawWave;
	pBody->gfxDevice.flipDrawWave			= &wg_flipDrawWave;
	pBody->gfxDevice.drawElipse				= &wg_drawElipse;
	pBody->gfxDevice.drawPieChart			= &wg_drawPieChart;
	pBody->gfxDevice.drawSegments			= &wg_drawSegments;
	pBody->gfxDevice.flipDrawSegments		= &wg_flipDrawSegments;
	pBody->gfxDevice.drawEdgemap			= &wg_drawEdgemap;
	pBody->gfxDevice.flipDrawEdgemap		= &wg_flipDrawEdgemap;
	pBody->gfxDevice.blitNinePatch			= &wg_blitNinePatch;

	pBody->gfxDevice.setBlurMatrices		= &wg_setBlurMatrices;
	pBody->gfxDevice.setFixedBlendColor		= &wg_setFixedBlendColor;
	pBody->gfxDevice.getFixedBlendColor		= &wg_getFixedBlendColor;
	
	pBody->gfxDevice.blur					= &wg_blur;
	pBody->gfxDevice.blurRect				= &wg_blurRect;
	pBody->gfxDevice.stretchBlur			= &wg_stretchBlur;
	pBody->gfxDevice.stretchBlurRect		= &wg_stretchBlurRect;
	pBody->gfxDevice.transformBlur			= &wg_transformBlur;
	pBody->gfxDevice.rotScaleBlur			= &wg_rotScaleBlur;

	pBody->gfxDevice.setBlurbrush			= &wg_setBlurbrush;

/*
	pBody->streamBuffer.structSize				= sizeof(wg_streambuffer_calls);
	pBody->streamBuffer.createStreamBuffer		= &wg_createStreamBuffer;
	pBody->streamBuffer.getStreamBufferOutput	= &wg_getStreamBufferOutput;
	pBody->streamBuffer.getStreamBufferInput	= &wg_getStreamBufferInput;
	pBody->streamBuffer.streamBufferCapacity	= &wg_streamBufferCapacity;
	pBody->streamBuffer.streamBufferHasChunk	= &wg_streamBufferHasChunk;
	pBody->streamBuffer.streamBufferBytes		= &wg_streamBufferBytes;


	pBody->streamPlayer.structSize			= sizeof(wg_streamplayer_calls);
	pBody->streamPlayer.createStreamPlayer	= &wg_createStreamPlayer;
	pBody->streamPlayer.getStreamPlayerInput	= &wg_getStreamPlayerInput;
	pBody->streamPlayer.setStreamPlayerStoreDirtyRects = &wg_setStreamPlayerStoreDirtyRects;
	pBody->streamPlayer.setStreamPlayerMaxDirtyRects = &wg_setStreamPlayerMaxDirtyRects;
	pBody->streamPlayer.getStreamPlayerDirtyRects = &wg_getStreamPlayerDirtyRects;
	pBody->streamPlayer.clearStreamPlayerDirtyRects = &wg_clearStreamPlayerDirtyRects;


	pBody->streamPump.structSize			= sizeof(wg_streampump_calls);
	pBody->streamPump.createStreamPump		= &wg_createStreamPump;
	pBody->streamPump.createStreamPumpWithInputOutput = &wg_createStreamPumpWithInputOutput;
	pBody->streamPump.setStreamPumpInput	= &wg_setStreamPumpInput;
	pBody->streamPump.setStreamPumpOutput	= &wg_setStreamPumpOutput;
	pBody->streamPump.peekChunk				= &wg_peekChunk;
	pBody->streamPump.pumpChunk				= &wg_pumpChunk;
	pBody->streamPump.pumpUntilFrame		= &wg_pumpUntilFrame;
	pBody->streamPump.pumpFrame				= &wg_pumpFrame;
	pBody->streamPump.pumpAll				= &wg_pumpAll;


	pBody->streamReader.structSize				= sizeof(wg_streamreader_calls);
	pBody->streamReader.createStreamReader		= &wg_createStreamReader;
	pBody->streamReader.getStreamReaderOutput	= &wg_getStreamReaderOutput;
	pBody->streamReader.streamReaderCapacity	= &wg_streamReaderCapacity;
	pBody->streamReader.streamReaderHasChunk	= &wg_streamReaderHasChunk;
	pBody->streamReader.streamReaderBytes		= &wg_streamReaderBytes;
*/


	pBody->object.structSize				= sizeof(wg_object_calls);
	pBody->object.finalizer					= &wg_finalizer;
	pBody->object.getTypeInfo				= &wg_getTypeInfo;
	pBody->object.isInstanceOf				= &wg_isInstanceOf;
	pBody->object.refcount					= &wg_refcount;
	pBody->object.release					= &wg_release;
	pBody->object.retain					= &wg_retain;
	pBody->object.setFinalizer				= &wg_setFinalizer;


	pBody->surface.structSize				= sizeof(wg_surface_calls);
	pBody->surface.setSurfaceIdentity		= &wg_setSurfaceIdentity;
	pBody->surface.getSurfaceIdentity		= &wg_getSurfaceIdentity;
	pBody->surface.surfacePixelSize			= &wg_surfacePixelSize;
	pBody->surface.surfacePixelWidth		= &wg_surfacePixelWidth;
	pBody->surface.surfacePixelHeight		= &wg_surfacePixelHeight;
	pBody->surface.surfacePointSize			= &wg_surfacePointSize;
	pBody->surface.surfacePointWidth		= &wg_surfacePointWidth;
	pBody->surface.surfacePointHeight		= &wg_surfacePointHeight;
	pBody->surface.surfaceScale				= &wg_surfaceScale;
	pBody->surface.surfaceSampleMethod		= &wg_surfaceSampleMethod;
	pBody->surface.surfaceIsTiling			= &wg_surfaceIsTiling;
	pBody->surface.surfaceIsMipmapped		= &wg_surfaceIsMipmapped;
	pBody->surface.surfaceAlpha				= &wg_surfaceAlpha;
	pBody->surface.surfacePalette			= &wg_surfacePalette;
	pBody->surface.surfacePaletteSize		= &wg_surfacePaletteSize;
	pBody->surface.surfacePaletteCapacity	= &wg_surfacePaletteCapacity;
	pBody->surface.surfacePixelDescription	= &wg_surfacePixelDescription;
	pBody->surface.surfacePixelFormat		= &wg_surfacePixelFormat;
	pBody->surface.surfacePixelBits			= &wg_surfacePixelBits;
	pBody->surface.surfaceIsOpaque			= &wg_surfaceIsOpaque;
	pBody->surface.surfaceCanBeCanvas		= &wg_surfaceCanBeCanvas;
	pBody->surface.allocPixelBuffer			= &wg_allocPixelBuffer;
	pBody->surface.allocPixelBufferFromRect	= &wg_allocPixelBufferFromRect;
	pBody->surface.pushPixels				= &wg_pushPixels;
	pBody->surface.pushPixelsFromRect		= &wg_pushPixelsFromRect;
	pBody->surface.pullPixels				= &wg_pullPixels;
	pBody->surface.pullPixelsFromRect		= &wg_pullPixelsFromRect;
	pBody->surface.freePixelBuffer			= &wg_freePixelBuffer;
	pBody->surface.fillSurface				= &wg_fillSurface;
	pBody->surface.fillSurfaceRect			= &wg_fillSurfaceRect;
	pBody->surface.copySurface				= &wg_copySurface;
	pBody->surface.copySurfaceRect			= &wg_copySurfaceRect;
	pBody->surface.setSurfaceBaggage		= &wg_setSurfaceBaggage;
	pBody->surface.getSurfaceBaggage		= &wg_getSurfaceBaggage;
	pBody->surface.addSurfaceObserver		= &wg_addSurfaceObserver;
	pBody->surface.removeSurfaceObserver	= &wg_removeSurfaceObserver;
	pBody->surface.getSurfaceBlueprint		= &wg_getSurfaceBlueprint;


	pBody->surfaceFactory.structSize		= sizeof(wg_surfacefactory_calls);
	pBody->surfaceFactory.maxSurfaceSize	= &wg_maxSurfaceSize;
	pBody->surfaceFactory.createSurface		= &wg_createSurface;
	pBody->surfaceFactory.createSurfaceFromBlob = &wg_createSurfaceFromBlob;
	pBody->surfaceFactory.createSurfaceFromBitmap = &wg_createSurfaceFromBitmap;
	pBody->surfaceFactory.createSurfaceFromRawData = &wg_createSurfaceFromRawData;

	pBody->edgemap.structSize				= sizeof(wg_edgemap_calls);
	pBody->edgemap.edgemapPixelSize			= &wg_edgemapPixelSize;
	pBody->edgemap.setRenderSegments		= &wg_setRenderSegments;
	pBody->edgemap.getRenderSegments		= &wg_getRenderSegments;

	pBody->edgemap.edgemapPaletteType		= &wg_edgemapPaletteType;
	pBody->edgemap.setEdgemapColors			= &wg_setEdgemapColors;
	pBody->edgemap.setEdgemapColorsFromGradients = &wg_setEdgemapColorsFromGradients;
	pBody->edgemap.setEdgemapColorsFromTintmaps = &wg_setEdgemapColorsFromTintmaps;
	pBody->edgemap.setEdgemapColorsFromStrips = &wg_setEdgemapColorsFromStrips;

	pBody->edgemap.importEdgemapPaletteEntries = &wg_importEdgemapPaletteEntries;

	pBody->edgemap.edgemapFlatColors		= &wg_edgemapFlatColors;
	pBody->edgemap.edgemapColorstripsX		= &wg_edgemapColorstripsX;
	pBody->edgemap.edgemapColorstripsY		= &wg_edgemapColorstripsY;

	pBody->edgemap.edgemapSegments			= &wg_edgemapSegments;
	pBody->edgemap.edgemapSamples			= &wg_edgemapSamples;
	pBody->edgemap.importSpxSamples			= &wg_importSpxSamples;
	pBody->edgemap.importFloatSamples		= &wg_importFloatSamples;
	pBody->edgemap.exportSpxSamples			= &wg_exportSpxSamples;
	pBody->edgemap.exportFloatSamples		= &wg_exportFloatSamples;
	pBody->edgemap.importPaletteEntries		= &wg_importEdgemapPaletteEntries;




	wg_edgemapPalette	(*edgemapPaletteType)(wg_obj edgemap);
	int					(*setEdgemapColors)(wg_obj edgemap, int begin, int end, const wg_color * pColors);
	int					(*setEdgemapColorsFromGradients)(wg_obj edgemap, int begin, int end, const wg_gradient * pGradients );
	int					(*setEdgemapColorsFromTintmaps)(wg_obj edgemap, int begin, int end, wg_obj * pTintmaps );
	int					(*setEdgemapColorsFromStrips)(wg_obj edgemap, int begin, int end, const wg_color * pColorstripX, const wg_color * pColorstripY );

	int					(*importEdgemapPaletteEntries)(wg_obj edgemap, int begin, int end, const wg_color * pColors );

	const wg_color *  	(*edgemapFlatColors)(wg_obj edgemap);
	const wg_color *  	(*edgemapColorstripsX)(wg_obj edgemap);
	const wg_color *  	(*edgemapColorstripsY)(wg_obj edgemap);



	pBody->edgemapFactory.structSize		= sizeof(wg_edgemapfactory_calls);
	pBody->edgemapFactory.createEdgemap		= &wg_createEdgemap;
	pBody->edgemapFactory.createEdgemapFromFloats = &wg_createEdgemapFromFloats;
	pBody->edgemapFactory.createEdgemapFromSpx = &wg_createEdgemapFromSpx;

	pBody->hostBridge.structSize			= sizeof(wg_hostbridge_calls);
	pBody->hostBridge.hidePointer			= &wg_hidePointer;
	pBody->hostBridge.showPointer			= &wg_showPointer;
	pBody->hostBridge.getClipboardText		= &wg_getClipboardText;
	pBody->hostBridge.setClipboardText		= &wg_setClipboardText;
	pBody->hostBridge.requestWindowFocus	= &wg_requestWindowFocus;
	pBody->hostBridge.yieldWindowFocus		= &wg_yieldWindowFocus;
	pBody->hostBridge.lockHidePointer		= &wg_lockHidePointer;
	pBody->hostBridge.unlockShowPointer		= &wg_unlockShowPointer;
	pBody->hostBridge.setPointerStyle		= &wg_setPointerStyle;

	pBody->pluginCapsule.structSize			= sizeof(wg_plugincapsule_calls);
	pBody->pluginCapsule.requestRender 		= &wg_pluginRequestRender;
	pBody->pluginCapsule.requestResize 		= &wg_pluginRequestResize;
	pBody->pluginCapsule.isVisible 			= &wg_isPluginVisible;
	pBody->pluginCapsule.windowSection 		= &wg_pluginWindowSection;
	pBody->pluginCapsule.requestFocus 		= &wg_pluginRequestFocus;
	pBody->pluginCapsule.releaseFocus 		= &wg_pluginReleaseFocus;
	pBody->pluginCapsule.requestPreRenderCall = &wg_pluginRequestPreRenderCall;
	pBody->pluginCapsule.requestInView		= &wg_pluginRequestInView;
	pBody->pluginCapsule.connect			= &wg_connectPlugin;
	pBody->pluginCapsule.disconnect			= &wg_disconnectPlugin;

	pBody->blurbrush.structSize				= sizeof(wg_blurbrush_calls);
	pBody->blurbrush.create					= &wg_createBlurbrush;
	pBody->blurbrush.size					= &wg_blurbrushSize;
	pBody->blurbrush.blue					= &wg_blurbrushBlue;
	pBody->blurbrush.green					= &wg_blurbrushGreen;
	pBody->blurbrush.red					= &wg_blurbrushRed;


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
}



