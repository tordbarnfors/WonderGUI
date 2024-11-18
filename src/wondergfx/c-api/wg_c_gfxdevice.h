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

#ifndef WG_C_GFXDEVICE_DOT_H
#define WG_C_GFXDEVICE_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>
#include <wg_c_color.h>
#include <wg_c_gradient.h>
#include <wg_c_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wg_waveLine ____________________________________________________________

	typedef struct wg_waveLine_struct
	{
		int			length;
		wg_spx		thickness;
		wg_color	color;
		int* pWave;      // Pixel offset in 26.6 format.
		wg_spx		hold;      // Value for extending the line if it is too short (or completely missing).
	} wg_waveLine;

	//____ wg_canvasInfo __________________________________________________________

	typedef struct wg_canvasInfo_struct		// NOT BINARY EQUIVALENT!
	{
		wg_canvasRef	ref;
		wg_obj			surface;
		wg_sizeSPX		size;
		int				scale;
	} wg_canvasInfo;



	//____ Misc _______________________________________________________

	const wg_typeInfo*		wg_deviceSurfaceType(wg_obj device);


	wg_canvasInfo			wg_getCanvas(wg_obj device);
	wg_canvasInfo			wg_getCanvasRef(wg_obj device, wg_canvasRef ref);

	wg_obj					wg_canvasLayers(wg_obj device);

	wg_obj					wg_surfaceFactory(wg_obj device);
	wg_obj					wg_edgemapFactory(wg_obj device);


	int						wg_maxSegments(void);

	//____ Geometry _________________________________________________

	wg_sizeSPX				wg_canvasSize(wg_obj device);

	//____ State _________________________________________________

	int						wg_setClipList(wg_obj device, int nRectangles, const wg_rectSPX* pRectangles);
	void					wg_resetClipList(wg_obj device);
	int						wg_pushClipList(wg_obj device, int nRectangles, const wg_rectSPX* pRectangles);
	int						wg_popClipList(wg_obj device);

	const wg_rectSPX*		wg_getClipList(wg_obj device);
	int						wg_clipListSize(wg_obj device);
	const wg_rectSPX*		wg_clipBounds(wg_obj device);

	void					wg_setTintColor(wg_obj device, wg_color color);
	wg_color				wg_getTintColor(wg_obj device);

	void					wg_setTintGradient(wg_obj device, const wg_rectSPX* rect, const wg_gradient* gradient);
	void					wg_clearTintGradient(wg_obj device);

	int						wg_setBlendMode(wg_obj device, wg_blendMode blendMode);
	wg_blendMode 			wg_getBlendMode(wg_obj device);

	int						wg_setBlitSource(wg_obj device, wg_obj surface);
	wg_obj 					wg_getBlitSource(wg_obj device);

	void					wg_setMorphFactor(wg_obj device, float factor);
	float					wg_getMorphFactor(wg_obj device);

	void					wg_setBlurbrush(wg_obj device, wg_obj brush );

	void					wg_setFixedBlendColor(wg_obj device, wg_color color );
	wg_color				wg_getFixedBlendColor(wg_obj device);


	void					wg_setRenderLayer(wg_obj device, int layer);
	int						wg_getRenderLayer(wg_obj device);

	//.____ Rendering ________________________________________________

	int						wg_beginRender(wg_obj device);
	int						wg_endRender(wg_obj device);
	int						wg_isDeviceRendering(wg_obj device);
	int						wg_isDeviceIdle(wg_obj device);
	void					wg_flushDevice(wg_obj device);

	int						wg_beginCanvasUpdateWithRef(wg_obj device, wg_canvasRef canvas, int nUpdateRects, const wg_rectSPX* pUpdateRects, wg_obj canvasLayers, int startLayer);
	int						wg_beginCanvasUpdateWithSurface(wg_obj device, wg_obj surface, int nUpdateRects, const wg_rectSPX* pUpdateRects, wg_obj canvasLayers, int startLayer);
	void					wg_endCanvasUpdate(wg_obj device);


	// Draw methods.

	void					wg_fill(wg_obj device, wg_color col);
	void					wg_fillRect(wg_obj device, const wg_rectSPX* rect, wg_color col);

	void					wg_drawLine(wg_obj device, wg_coordSPX begin, wg_coordSPX end, wg_color color, wg_spx thickness);
	void					wg_drawStraightLine(wg_obj device, wg_coordSPX begin, wg_direction dir, wg_spx length, wg_color color, wg_spx thickness);

	// Blit methods

	void					wg_blit(wg_obj device, wg_coordSPX dest);
	void					wg_blitRect(wg_obj device, wg_coordSPX dest, const wg_rectSPX* src);

	void					wg_flipBlit(wg_obj device, wg_coordSPX dest, wg_gfxFlip flip);
	void					wg_flipBlitRect(wg_obj device, wg_coordSPX dest, const wg_rectSPX* src, wg_gfxFlip flip);

	void					wg_stretchBlit(wg_obj device, const wg_rectSPX* dest);
	void					wg_stretchBlitRect(wg_obj device, const wg_rectSPX* dest, const wg_rectSPX* src);

	void					wg_stretchFlipBlit(wg_obj device, const wg_rectSPX* dest, wg_gfxFlip flip);
	void					wg_stretchFlipBlitRect(wg_obj device, const wg_rectSPX* dest, const wg_rectSPX* src, wg_gfxFlip flip);

	void					wg_precisionBlit(wg_obj device, const wg_rectSPX* dest, const wg_rectF* srcSPX);
	void					wg_transformBlit(wg_obj device, const wg_rectSPX* dest, wg_coordF srcSPX, const wg_transform * pTransform);
	void					wg_rotScaleBlit(wg_obj device, const wg_rectSPX* dest, float rotationDegrees, float scale, wg_coordF srcCenter, wg_coordF destCenter);

	// Tile methods

	void					wg_tile(wg_obj device, const wg_rectSPX* dest, wg_coordSPX shift);
	void					wg_flipTile(wg_obj device, const wg_rectSPX* dest, wg_gfxFlip flip, wg_coordSPX shift);

	void					wg_scaleTile(wg_obj device, const wg_rectSPX* dest, float scale, wg_coordSPX shift);
	void					wg_scaleFlipTile(wg_obj device, const wg_rectSPX* dest, float scale, wg_gfxFlip flip, wg_coordSPX shift);

	// Blur methods

	void					wg_blur(wg_obj device, wg_coordSPX dest);
	void					wg_blurRect(wg_obj device, wg_coordSPX dest, const wg_rectSPX* src);
	void					wg_stretchBlur(wg_obj device, const wg_rectSPX* dest);
	void					wg_stretchBlurRect(wg_obj device, const wg_rectSPX* dest, const wg_rectSPX* src);
	void					wg_transformBlur(wg_obj device, const wg_rectSPX* dest, wg_coordF srcSPX, const wg_transform* pTransform);
	void					wg_rotScaleBlur(wg_obj device, const wg_rectSPX* dest, float rotationDegrees, float scale, wg_coordF srcCenter, wg_coordF destCenter);



	// Draw segments methods

	void					wg_drawWave(wg_obj device, const wg_rectSPX* dest, const wg_waveLine* pTopBorder, const wg_waveLine* pBottomBorder, wg_color frontFill, wg_color backFill);
	void					wg_flipDrawWave(wg_obj device, const wg_rectSPX* dest, const wg_waveLine* pTopBorder, const wg_waveLine* pBottomBorder, wg_color frontFill, wg_color backFill, wg_gfxFlip flip);

	void					wg_drawElipse(wg_obj device, const wg_rectSPX* canvas, wg_spx thickness, wg_color color, wg_spx outlineThickness, wg_color outlineColor);

	void					wg_drawPieChart(wg_obj device, const wg_rectSPX* canvas, float start, int nSlices, const float* pSliceSizes, const wg_color* pSliceColors, float hubSize, wg_color hubColor, wg_color backColor, int bRectangular);

	void					wg_drawSegments(wg_obj device, const wg_rectSPX* dest, int nSegments, const wg_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wg_tintMode tintMode);
	void					wg_flipDrawSegments(wg_obj device, const wg_rectSPX* dest, int nSegments, const wg_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wg_gfxFlip flip, wg_tintMode tintMode);

	void					wg_drawEdgemap(wg_obj device, wg_coordSPX dest, wg_obj edgemap);
	void					wg_flipDrawEdgemap(wg_obj device, wg_coordSPX dest, wg_obj edgemap, wg_gfxFlip flip);

	// Special draw/blit methods

	void					wg_blitNinePatch(wg_obj device, const wg_rectSPX* dstRect, const wg_borderSPX* dstFrame, const wg_ninePatch* patch, int scale);

#ifdef __cplusplus
}
#endif

#endif
