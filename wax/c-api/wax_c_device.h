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
#ifndef WAX_C_DEVICE_DOT_H
#define WAX_C_DEVICE_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>
#include <wax_c_color.h>
#include <wax_c_surface.h>
#include <wax_c_backend.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wax_waveLine ____________________________________________________________

	typedef struct wax_waveLine_struct
	{
		int			length;
		wax_spx		thickness;
		wax_color	color;
		int* pWave;      // Pixel offset in 26.6 format.
		wax_spx		hold;      // Value for extending the line if it is too short (or completely missing).
	} wax_waveLine;

	//____ wax_roundedBoxSpec ______________________________________________________
	// Colors with negative alpha (undefined) are not drawn.

	typedef struct wax_roundedBoxSpec_struct
	{
		wax_rectSPX		rect;				// Outer edge of the box.
		wax_spx			radius[4];			// Corner radii: top-left, top-right, bottom-right, bottom-left.
		wax_borderSPX	outline;			// Thickness of the outline along each side.
		wax_color		fillColor;
		wax_color		outlineColor;
		wax_obj			fillTint;			// Tint or null.
		wax_obj			outlineTint;		// Tint or null.
	} wax_roundedBoxSpec;

	//____ wax_ringSlice ___________________________________________________________

	typedef struct wax_ringSlice_struct
	{
		float			size;				// Fraction of a full turn.
		wax_color		color;
		wax_obj			tint;				// Tint or null.
	} wax_ringSlice;

	//____ wax_ringSpec ____________________________________________________________
	// Colors with negative alpha (undefined) are not drawn.

	typedef struct wax_ringSpec_struct
	{
		wax_rectSPX			rect;
		wax_spx				thickness;		// Width of ring, outline included. 0 or at least the radius for a full pie or disc.
		wax_spx				outline;
		float				start;			// Fraction of a turn clockwise from 12 o'clock.
		int					nSlices;
		const wax_ringSlice*	pSlices;
		int					bSquare;
		wax_color			outlineColor;
		wax_obj				outlineTint;
		wax_color			hubColor;
		wax_obj				hubTint;
		wax_color			backColor;
		wax_obj				backTint;
	} wax_ringSpec;

	//____ wax_lineCap ____________________________________________________________
	// See LineCap.

	typedef struct wax_lineCap_struct
	{
		wax_lineCapShape		shape;
		int					spearLength;	// Percent of thickness from tip to full width, WAX_LINECAP_SPEAR only. 25 to 800.
	} wax_lineCap;

	//____ Creation ___________________________________________________

	WAX_EXPORT wax_obj		wax_createDevice( wax_obj backend );

	//____ Misc _______________________________________________________

	WAX_EXPORT int			wax_setBackend( wax_obj device, wax_obj backend );
	WAX_EXPORT wax_obj		wax_getBackend( wax_obj device );

	WAX_EXPORT wax_canvasInfo	wax_getCanvas(wax_obj device);
	WAX_EXPORT wax_canvasInfo	wax_getCanvasRef(wax_obj device, wax_canvasRef ref);

	WAX_EXPORT wax_obj		wax_canvasLayers(wax_obj device);

	WAX_EXPORT wax_obj		wax_surfaceFactory(wax_obj device);
	WAX_EXPORT wax_obj		wax_edgemapFactory(wax_obj device);
	WAX_EXPORT wax_obj		wax_warpmapFactory(wax_obj device);


	WAX_EXPORT int			wax_maxSegments(void);

	//____ Geometry _________________________________________________

	WAX_EXPORT wax_sizeSPX	wax_canvasSize(wax_obj device);

	//____ State _________________________________________________

	WAX_EXPORT int			wax_setClipList(wax_obj device, int nRectangles, const wax_rectSPX* pRectangles);
	WAX_EXPORT void			wax_resetClipList(wax_obj device);
	WAX_EXPORT int			wax_pushClipList(wax_obj device, int nRectangles, const wax_rectSPX* pRectangles);
	WAX_EXPORT int			wax_popClipList(wax_obj device);

	WAX_EXPORT const wax_rectSPX*	wax_getClipList(wax_obj device);
	WAX_EXPORT int			wax_clipListSize(wax_obj device);
	WAX_EXPORT const wax_rectSPX*	wax_clipBounds(wax_obj device);

	WAX_EXPORT void			wax_setTintColor(wax_obj device, wax_color color);
	WAX_EXPORT wax_color		wax_getTintColor(wax_obj device);
	WAX_EXPORT void			wax_clearTintColor(wax_obj device);
	WAX_EXPORT int			wax_hasTintColor(wax_obj device);

	WAX_EXPORT void			wax_setTint(wax_obj device, const wax_rectSPX* rect, const wax_obj tint);
	WAX_EXPORT wax_obj		wax_getTint(wax_obj device);
	WAX_EXPORT wax_rectSPX	wax_getTintRect(wax_obj device);
	WAX_EXPORT void			wax_clearTint(wax_obj device);
	WAX_EXPORT int			wax_hasTint(wax_obj device);

	WAX_EXPORT int			wax_setBlendMode(wax_obj device, wax_blendMode blendMode);
	WAX_EXPORT wax_blendMode 	wax_getBlendMode(wax_obj device);

	WAX_EXPORT int			wax_setBlitSource(wax_obj device, wax_obj surface);
	WAX_EXPORT wax_obj 		wax_getBlitSource(wax_obj device);

	WAX_EXPORT void			wax_setMorphFactor(wax_obj device, float factor);
	WAX_EXPORT float			wax_getMorphFactor(wax_obj device);

	WAX_EXPORT void			wax_setBlurbrush(wax_obj device, wax_obj brush );
	WAX_EXPORT wax_obj		wax_getBlurbrush(wax_obj device);

	WAX_EXPORT void			wax_setFixedBlendColor(wax_obj device, wax_color color );
	WAX_EXPORT wax_color		wax_getFixedBlendColor(wax_obj device);


	WAX_EXPORT void			wax_setRenderLayer(wax_obj device, int layer);
	WAX_EXPORT int			wax_getRenderLayer(wax_obj device);

	//.____ Rendering ________________________________________________

	WAX_EXPORT int			wax_beginRender(wax_obj device);
	WAX_EXPORT int			wax_endRender(wax_obj device);
	WAX_EXPORT int			wax_isDeviceRendering(wax_obj device);
	WAX_EXPORT int			wax_isDeviceIdle(wax_obj device);
	WAX_EXPORT void			wax_flushDevice(wax_obj device);

	WAX_EXPORT int			wax_beginCanvasUpdateWithRef(wax_obj device, wax_canvasRef canvas, int nUpdateRects, const wax_rectSPX* pUpdateRects, wax_obj canvasLayers, int startLayer);
	WAX_EXPORT int			wax_beginCanvasUpdateWithSurface(wax_obj device, wax_obj surface, int nUpdateRects, const wax_rectSPX* pUpdateRects, wax_obj canvasLayers, int startLayer);
	WAX_EXPORT void			wax_endCanvasUpdate(wax_obj device);

	WAX_EXPORT void			wax_flattenLayers(wax_obj device);
	WAX_EXPORT void			wax_clearLayers(wax_obj device);

	// Draw methods.

	WAX_EXPORT void			wax_fill(wax_obj device, wax_color col);
	WAX_EXPORT void			wax_fillRect(wax_obj device, const wax_rectSPX* rect, wax_color col);

	WAX_EXPORT void			wax_drawLine(wax_obj device, wax_coordSPX begin, wax_coordSPX end, wax_color color, wax_spx thickness);
	WAX_EXPORT void			wax_drawStraightLine(wax_obj device, wax_coordSPX begin, wax_direction dir, wax_spx length, wax_color color, wax_spx thickness);
	WAX_EXPORT void			wax_drawLineWithCaps(wax_obj device, wax_coordSPX begin, wax_coordSPX end, wax_color color, wax_spx thickness, wax_lineCap beginCap, wax_lineCap endCap);
	WAX_EXPORT void			wax_drawStraightLineWithCaps(wax_obj device, wax_coordSPX begin, wax_direction dir, wax_spx length, wax_color color, wax_spx thickness, wax_lineCap beginCap, wax_lineCap endCap);

	// Blit methods

	WAX_EXPORT void			wax_blit(wax_obj device, wax_coordSPX dest);
	WAX_EXPORT void			wax_blitRect(wax_obj device, wax_coordSPX dest, const wax_rectSPX* src);

	WAX_EXPORT void			wax_flipBlit(wax_obj device, wax_coordSPX dest, wax_gfxFlip flip);
	WAX_EXPORT void			wax_flipBlitRect(wax_obj device, wax_coordSPX dest, const wax_rectSPX* src, wax_gfxFlip flip);

	WAX_EXPORT void			wax_stretchBlit(wax_obj device, const wax_rectSPX* dest);
	WAX_EXPORT void			wax_stretchBlitRect(wax_obj device, const wax_rectSPX* dest, const wax_rectSPX* src);

	WAX_EXPORT void			wax_stretchFlipBlit(wax_obj device, const wax_rectSPX* dest, wax_gfxFlip flip);
	WAX_EXPORT void			wax_stretchFlipBlitRect(wax_obj device, const wax_rectSPX* dest, const wax_rectSPX* src, wax_gfxFlip flip);

	WAX_EXPORT void			wax_precisionBlit(wax_obj device, const wax_rectSPX* dest, const wax_rectF* srcSPX);
	WAX_EXPORT void			wax_transformBlit(wax_obj device, const wax_rectSPX* dest, wax_coordF srcSPX, const wax_transform* pTransform);
	WAX_EXPORT void			wax_rotScaleBlit(wax_obj device, const wax_rectSPX* dest, float rotationDegrees, float scale, wax_coordF srcCenter, wax_coordF destCenter);

	// Tile methods

	WAX_EXPORT void			wax_tile(wax_obj device, const wax_rectSPX* dest, wax_coordSPX shift);
	WAX_EXPORT void			wax_flipTile(wax_obj device, const wax_rectSPX* dest, wax_gfxFlip flip, wax_coordSPX shift);

	WAX_EXPORT void			wax_scaleTile(wax_obj device, const wax_rectSPX* dest, float scale, wax_coordSPX shift);
	WAX_EXPORT void			wax_scaleFlipTile(wax_obj device, const wax_rectSPX* dest, float scale, wax_gfxFlip flip, wax_coordSPX shift);

	// Blur methods

	WAX_EXPORT void			wax_blur(wax_obj device, wax_coordSPX dest);
	WAX_EXPORT void			wax_blurRect(wax_obj device, wax_coordSPX dest, const wax_rectSPX* src);
	WAX_EXPORT void			wax_stretchBlur(wax_obj device, const wax_rectSPX* dest);
	WAX_EXPORT void			wax_stretchBlurRect(wax_obj device, const wax_rectSPX* dest, const wax_rectSPX* src);
	WAX_EXPORT void			wax_transformBlur(wax_obj device, const wax_rectSPX* dest, wax_coordF srcSPX, const wax_transform* pTransform);
	WAX_EXPORT void			wax_rotScaleBlur(wax_obj device, const wax_rectSPX* dest, float rotationDegrees, float scale, wax_coordF srcCenter, wax_coordF destCenter);

	// Shapes

	WAX_EXPORT void			wax_drawRoundedBox(wax_obj device, const wax_roundedBoxSpec* spec);
	WAX_EXPORT void			wax_drawRing(wax_obj device, const wax_ringSpec* spec);

	// Draw edgemap methods

	WAX_EXPORT void			wax_drawEdgemap(wax_obj device, wax_coordSPX dest, wax_obj edgemap);
	WAX_EXPORT void			wax_flipDrawEdgemap(wax_obj device, wax_coordSPX dest, wax_obj edgemap, wax_gfxFlip flip);
	WAX_EXPORT void			wax_tileEdgemap(wax_obj device, const wax_rectSPX* dest, wax_obj edgemap, wax_coordSPX shift);
	WAX_EXPORT void			wax_flipTileEdgemap(wax_obj device, const wax_rectSPX* dest, wax_obj edgemap, wax_gfxFlip flip, wax_coordSPX shift);

	// Warp methods, see Warpmap. Source offset is in spx of the source.

	WAX_EXPORT void			wax_warpBlit(wax_obj device, wax_coordSPX dest, wax_obj warpmap, wax_coordSPX sourceOffset);
	WAX_EXPORT void			wax_warpBlitRect(wax_obj device, wax_coordSPX dest, wax_obj warpmap, const wax_rectSPX* source, wax_coordSPX sourceOffset);
	WAX_EXPORT void			wax_warpDrawEdgemap(wax_obj device, wax_coordSPX dest, wax_obj warpmap, wax_obj edgemap, wax_coordSPX sourceOffset);

	// Special draw/blit methods

	WAX_EXPORT void			wax_blitNinePatch(wax_obj device, const wax_rectSPX* dstRect, const wax_borderSPX* dstFrame, const wax_ninePatch* patch, int scale);

	// Deprecated

	WAX_DEPRECATED void		wax_drawWave(wax_obj device, const wax_rectSPX* dest, const wax_waveLine* pTopBorder, const wax_waveLine* pBottomBorder, wax_color fill);
	WAX_DEPRECATED void		wax_flipDrawWave(wax_obj device, const wax_rectSPX* dest, const wax_waveLine* pTopBorder, const wax_waveLine* pBottomBorder, wax_color fill, wax_gfxFlip flip);

	WAX_DEPRECATED void		wax_drawElipse(wax_obj device, const wax_rectSPX* canvas, wax_spx thickness, wax_color color, wax_spx outlineThickness, wax_color outlineColor);

	WAX_DEPRECATED void		wax_drawPieChart(wax_obj device, const wax_rectSPX* canvas, float start, int nSlices, const float* pSliceSizes, const wax_color* pSliceColors, float hubSize, wax_color hubColor, wax_color backColor, int bRectangular);

	WAX_DEPRECATED void		wax_drawSegments(wax_obj device, const wax_rectSPX* dest, int nSegments, const wax_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wax_tintMode tintMode);
	WAX_DEPRECATED void		wax_flipDrawSegments(wax_obj device, const wax_rectSPX* dest, int nSegments, const wax_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wax_gfxFlip flip, wax_tintMode tintMode);


	WAX_DEPRECATED void		wax_setBlurMatrices(wax_obj device, wax_spx radius, const float red[9], const float green[9], const float blue[9] );


#ifdef __cplusplus
}
#endif

#endif
