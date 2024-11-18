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

#include <wg_c_gfxdevice.h>
#include <wg_gfxdevice.h>

#include <type_traits>


using namespace wg;


static_assert( sizeof(wg_waveLine) == sizeof(wg::WaveLine), "wg_waveLine differs in size from wg::WaveLine!" );
static_assert( std::is_trivially_copyable<wg::WaveLine>::value, "wg::WaveLine is not trivially copyable" );



inline GfxDevice* getPtr(wg_obj obj) {
	return static_cast<GfxDevice*>(reinterpret_cast<Object*>(obj));
}

inline Edgemap* getEdgemapPtr(wg_obj obj) {
	return static_cast<Edgemap*>(reinterpret_cast<Object*>(obj));
}


const wg_typeInfo* wg_deviceSurfaceType(wg_obj device)
{
	auto& typeInfo = getPtr(device)->surfaceType();
	return reinterpret_cast<const wg_typeInfo*>(&typeInfo);
}


wg_canvasInfo wg_getCanvas(wg_obj device)
{
	auto& canvasInfo = getPtr(device)->canvas();
	return { (wg_canvasRef)canvasInfo.ref, canvasInfo.pSurface, {canvasInfo.size.w, canvasInfo.size.h}, canvasInfo.scale };
}


wg_canvasInfo wg_getCanvasRef(wg_obj device, wg_canvasRef ref)
{
	auto& canvasInfo = getPtr(device)->canvas((CanvasRef)ref);
	return { (wg_canvasRef)canvasInfo.ref, canvasInfo.pSurface, {canvasInfo.size.w, canvasInfo.size.h}, canvasInfo.scale };
}


wg_obj	wg_canvasLayers(wg_obj device)
{
	return getPtr(device)->canvasLayers().rawPtr();
}


wg_obj wg_surfaceFactory(wg_obj device)
{
	return getPtr(device)->surfaceFactory().rawPtr();
}


wg_obj wg_edgemapFactory(wg_obj device)
{
	return getPtr(device)->edgemapFactory().rawPtr();
}


int	wg_maxSegments()
{
	return GfxDevice::maxSegments();
}


wg_sizeSPX wg_canvasSize(wg_obj device)
{
	auto canvasSize = getPtr(device)->canvasSize();
	return { canvasSize.w, canvasSize.h };
}


int wg_setClipList(wg_obj device, int nRectangles, const wg_rectSPX* pRectangles)
{
	return getPtr(device)->setClipList(nRectangles, (RectSPX*)pRectangles);
}


void wg_resetClipList(wg_obj device)
{
	getPtr(device)->resetClipList();
}


int	wg_pushClipList(wg_obj device, int nRectangles, const wg_rectSPX* pRectangles)
{
	return getPtr(device)->pushClipList(nRectangles, (RectSPX*)pRectangles);
}


int	wg_popClipList(wg_obj device)
{
	return getPtr(device)->popClipList();
}


const wg_rectSPX* wg_getClipList(wg_obj device)
{
	return (wg_rectSPX*) getPtr(device)->clipList();
}


int wg_clipListSize(wg_obj device)
{
 	return getPtr(device)->clipListSize();
}


const wg_rectSPX* wg_clipBounds(wg_obj device)
{
	return (wg_rectSPX*) &getPtr(device)->clipBounds();
}


void wg_setTintColor(wg_obj device, wg_color color)
{
	getPtr(device)->setTintColor( HiColor( color.r, color.g, color.b, color.a) );
}


wg_color wg_getTintColor(wg_obj device)
{
	auto color = getPtr(device)->tintColor();
	return { color.b, color.g, color.r, color.a };
}


void wg_setTintGradient(wg_obj device, const wg_rectSPX* rect, const wg_gradient* gradient)
{
	getPtr(device)->setTintGradient( *(const RectSPX*)rect, *(const Gradient*)gradient );
}


void wg_clearTintGradient(wg_obj device)
{
	getPtr(device)->clearTintGradient();
}


int wg_setBlendMode(wg_obj device, wg_blendMode blendMode)
{
	return getPtr(device)->setBlendMode( (BlendMode) blendMode );
}


wg_blendMode wg_getBlendMode(wg_obj device)
{
	return (wg_blendMode) getPtr(device)->blendMode();
}


int wg_setBlitSource(wg_obj device, wg_obj surface)
{
	return getPtr(device)->setBlitSource( static_cast<Surface*>(reinterpret_cast<Object*>(surface)) );
}


wg_obj wg_getBlitSource(wg_obj device)
{
	return (wg_obj) getPtr(device)->blitSource();
}


void wg_setMorphFactor(wg_obj device, float factor)
{
	getPtr(device)->setMorphFactor(factor);
}


float wg_getMorphFactor(wg_obj device)
{
	return getPtr(device)->morphFactor();
}

void wg_setBlurbrush(wg_obj device, wg_obj brush )
{
	Blurbrush* pBrush = brush == 0 ? nullptr : static_cast<Blurbrush*>(reinterpret_cast<Object*>(brush));

	getPtr(device)->setBlurbrush( pBrush );
}


void wg_setFixedBlendColor(wg_obj device, wg_color color )
{
	getPtr(device)->setFixedBlendColor( HiColor( color.r, color.g, color.b, color.a) );
}


wg_color wg_getFixedBlendColor(wg_obj device)
{
	auto color = getPtr(device)->fixedBlendColor();
	return { color.b, color.g, color.r, color.a };
}


void wg_setRenderLayer(wg_obj device, int layer)
{
	getPtr(device)->setRenderLayer(layer);
}


int wg_getRenderLayer(wg_obj device)
{
	return getPtr(device)->renderLayer();
}


int wg_beginRender(wg_obj device)
{
	return getPtr(device)->beginRender();
}


int wg_endRender(wg_obj device)
{
	return getPtr(device)->endRender();
}


int wg_isDeviceRendering(wg_obj device)
{
	return getPtr(device)->isRendering();
}


int wg_isDeviceIdle(wg_obj device)
{
	return getPtr(device)->isIdle();
}


void wg_flushDevice(wg_obj device)
{
	getPtr(device)->flush();
}


int wg_beginCanvasUpdateWithRef(wg_obj device, wg_canvasRef canvas, int nUpdateRects, const wg_rectSPX* pUpdateRects, wg_obj canvasLayers, int startLayer)
{
	CanvasLayers* pLayers = canvasLayers == 0 ? nullptr : static_cast<CanvasLayers*>(reinterpret_cast<Object*>(canvasLayers));

	return getPtr(device)->beginCanvasUpdate( (CanvasRef) canvas, nUpdateRects, (const RectSPX*) pUpdateRects, 
			pLayers, startLayer );
}


int wg_beginCanvasUpdateWithSurface(wg_obj device, wg_obj surface, int nUpdateRects, const wg_rectSPX* pUpdateRects, wg_obj canvasLayers, int startLayer)
{
	CanvasLayers* pLayers = canvasLayers == 0 ? nullptr : static_cast<CanvasLayers*>(reinterpret_cast<Object*>(canvasLayers));

	return getPtr(device)->beginCanvasUpdate(static_cast<Surface*>(reinterpret_cast<Object*>(surface)), nUpdateRects, (const RectSPX*)pUpdateRects,
		pLayers, startLayer);
}


void wg_endCanvasUpdate(wg_obj device)
{
	getPtr(device)->endCanvasUpdate();
}


void wg_fill(wg_obj device, wg_color col)
{
	getPtr(device)->fill( HiColor(col.r, col.g, col.b, col.a) );
}

void wg_fillRect(wg_obj device, const wg_rectSPX* rect, wg_color col)
{
	getPtr(device)->fill( *(const RectSPX*) rect, HiColor(col.r, col.g, col.b, col.a) );
}


void wg_drawLine(wg_obj device, wg_coordSPX begin, wg_coordSPX end, wg_color color, wg_spx thickness)
{
	getPtr(device)->drawLine({ begin.x, begin.y }, { end.x, end.y }, * (HiColor*) &color, (spx) thickness);
}


void wg_drawStraightLine(wg_obj device, wg_coordSPX begin, wg_direction dir, wg_spx length, wg_color color, wg_spx thickness)
{
	getPtr(device)->drawLine( { begin.x, begin.y }, (Direction) dir, (spx) length, *(HiColor*)&color, (spx)thickness );
}


void wg_blit(wg_obj device, wg_coordSPX dest)
{
	getPtr(device)->blit( {dest.x, dest.y} );
}


void wg_blitRect(wg_obj device, wg_coordSPX dest, const wg_rectSPX* src)
{
	getPtr(device)->blit( { dest.x, dest.y }, * (const RectSPX*) src );
}


void wg_flipBlit(wg_obj device, wg_coordSPX dest, wg_gfxFlip flip)
{
	getPtr(device)->flipBlit({ dest.x, dest.y }, (GfxFlip) flip );
}


void wg_flipBlitRect(wg_obj device, wg_coordSPX dest, const wg_rectSPX* src, wg_gfxFlip flip)
{
	getPtr(device)->flipBlit({ dest.x, dest.y }, *(const RectSPX*)src, (GfxFlip) flip );
}


void wg_stretchBlit(wg_obj device, const wg_rectSPX* dest)
{
	getPtr(device)->stretchBlit( *(const RectSPX*)dest );
}


void wg_stretchBlitRect(wg_obj device, const wg_rectSPX* dest, const wg_rectSPX* src)
{
	getPtr(device)->stretchBlit( *(const RectSPX*)dest, *(const RectSPX*)src );
}


void wg_stretchFlipBlit(wg_obj device, const wg_rectSPX* dest, wg_gfxFlip flip)
{
	getPtr(device)->stretchFlipBlit( *(const RectSPX*)dest, (GfxFlip) flip );
}


void wg_stretchFlipBlitRect(wg_obj device, const wg_rectSPX* dest, const wg_rectSPX* src, wg_gfxFlip flip)
{
	getPtr(device)->stretchFlipBlit(*(const RectSPX*)dest, *(const RectSPX*)src, (GfxFlip)flip);
}


void wg_precisionBlit(wg_obj device, const wg_rectSPX* dest, const wg_rectF* srcSPX)
{
	getPtr(device)->precisionBlit(*(const RectSPX*)dest, *(const RectF*)srcSPX );

}


void wg_transformBlit(wg_obj device, const wg_rectSPX* dest, wg_coordF srcSPX, const wg_transform * pTransform)
{
	getPtr(device)->transformBlit(*(const RectSPX*)dest, {srcSPX.x, srcSPX.y}, * reinterpret_cast<const wg::Transform*>(pTransform) );
}


void wg_rotScaleBlit(wg_obj device, const wg_rectSPX* dest, float rotationDegrees, float scale, wg_coordF srcCenter, wg_coordF destCenter)
{
	getPtr(device)->rotScaleBlit(*(const RectSPX*)dest, rotationDegrees, scale, {srcCenter.x, srcCenter.y}, {destCenter.x, destCenter.y} );
}


void wg_tile(wg_obj device, const wg_rectSPX* dest, wg_coordSPX shift)
{
	getPtr(device)->tile(*(const RectSPX*)dest, {shift.x, shift.y} );
}


void wg_flipTile(wg_obj device, const wg_rectSPX* dest, wg_gfxFlip flip, wg_coordSPX shift)
{
	getPtr(device)->flipTile(*(const RectSPX*)dest, (GfxFlip) flip, { shift.x, shift.y });
}


void wg_scaleTile(wg_obj device, const wg_rectSPX* dest, float scale, wg_coordSPX shift)
{
	getPtr(device)->scaleTile(*(const RectSPX*)dest, scale, { shift.x, shift.y });
}


void wg_scaleFlipTile(wg_obj device, const wg_rectSPX* dest, float scale, wg_gfxFlip flip, wg_coordSPX shift)
{
	getPtr(device)->scaleFlipTile(*(const RectSPX*)dest, scale, (GfxFlip) flip, {shift.x, shift.y});
}


void wg_drawWave(wg_obj device, const wg_rectSPX* dest, const wg_waveLine* pTopBorder, const wg_waveLine* pBottomBorder, wg_color frontFill, wg_color backFill)
{
	getPtr(device)->drawWave(*(const RectSPX*)dest, (WaveLine*) pTopBorder, (WaveLine*) pBottomBorder, * (HiColor*) &frontFill, *(HiColor*)&backFill );
}


void wg_flipDrawWave(wg_obj device, const wg_rectSPX* dest, const wg_waveLine* pTopBorder, const wg_waveLine* pBottomBorder, wg_color frontFill, wg_color backFill, wg_gfxFlip flip)
{
	getPtr(device)->flipDrawWave(*(const RectSPX*)dest, (WaveLine*)pTopBorder, (WaveLine*)pBottomBorder, *(HiColor*)&frontFill, *(HiColor*)&backFill, (GfxFlip) flip );
}


void wg_drawElipse(wg_obj device, const wg_rectSPX* canvas, wg_spx thickness, wg_color color, wg_spx outlineThickness, wg_color outlineColor)
{
	getPtr(device)->drawElipse(*(const RectSPX*)canvas, thickness, *(HiColor*)&color, outlineThickness, *(HiColor*)&outlineColor );
}


void wg_drawPieChart(wg_obj device, const wg_rectSPX* canvas, float start, int nSlices, const float* pSliceSizes, const wg_color* pSliceColors, float hubSize, wg_color hubColor, wg_color backColor, int bRectangular)
{
	getPtr(device)->drawPieChart(*(const RectSPX*)canvas, start, nSlices, pSliceSizes, (HiColor*)pSliceColors, hubSize, *(HiColor*)&hubColor, *(HiColor*)&backColor, bRectangular );
}


void wg_drawSegments(wg_obj device, const wg_rectSPX* dest, int nSegments, const wg_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wg_tintMode tintMode)
{
	getPtr(device)->drawSegments(*(const RectSPX*)dest, nSegments, (HiColor*)pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, (TintMode) tintMode );
}


void wg_flipDrawSegments(wg_obj device, const wg_rectSPX* dest, int nSegments, const wg_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wg_gfxFlip flip, wg_tintMode tintMode)
{
	getPtr(device)->flipDrawSegments(*(const RectSPX*)dest, nSegments, (HiColor*)pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, (GfxFlip) flip, (TintMode)tintMode);
}


void wg_drawEdgemap(wg_obj device, wg_coordSPX dest, wg_obj edgemap)
{
	getPtr(device)->drawEdgemap({ dest.x, dest.y }, getEdgemapPtr(edgemap));
}


void wg_flipDrawEdgemap(wg_obj device, wg_coordSPX dest, wg_obj edgemap, wg_gfxFlip flip)
{
	getPtr(device)->flipDrawEdgemap({ dest.x, dest.y }, getEdgemapPtr(edgemap), (GfxFlip) flip );
}


void wg_blitNinePatch(wg_obj device, const wg_rectSPX* dstRect, const wg_borderSPX* dstFrame, const wg_ninePatch* patch, int scale)
{
	getPtr(device)->blitNinePatch(*(const RectSPX*)dstRect, *(BorderSPX*) dstFrame, * (NinePatch*) patch, scale);
}

void wg_blur(wg_obj device, wg_coordSPX dest)
{
	getPtr(device)->blur( {dest.x, dest.y} );
}

void wg_blurRect(wg_obj device, wg_coordSPX dest, const wg_rectSPX* src)
{
	getPtr(device)->blur( { dest.x, dest.y }, * (const RectSPX*) src );
}

void wg_stretchBlur(wg_obj device, const wg_rectSPX* dest)
{
	getPtr(device)->stretchBlur( *(const RectSPX*)dest );
}

void wg_stretchBlurRect(wg_obj device, const wg_rectSPX* dest, const wg_rectSPX* src)
{
	getPtr(device)->stretchBlur( *(const RectSPX*)dest, *(const RectSPX*)src );
}

void wg_transformBlur(wg_obj device, const wg_rectSPX* dest, wg_coordF srcSPX, const wg_transform* pTransform)
{
	getPtr(device)->transformBlur(*(const RectSPX*)dest, {srcSPX.x, srcSPX.y}, *reinterpret_cast<const wg::Transform*>(pTransform) );
}

void wg_rotScaleBlur(wg_obj device, const wg_rectSPX* dest, float rotationDegrees, float scale, wg_coordF srcCenter, wg_coordF destCenter)
{
	getPtr(device)->rotScaleBlur(*(const RectSPX*)dest, rotationDegrees, scale, {srcCenter.x, srcCenter.y}, {destCenter.x, destCenter.y} );
}
