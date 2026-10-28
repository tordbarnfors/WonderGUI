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
#include <wax_c_device.h>
#include <algorithm>
#include <new>
#include <wax_device_gen2.h>
#include <wax_core.h>

#include <wax_c_blurbrush.h>

#include <type_traits>


using namespace wax;


static_assert( sizeof(wax_waveLine) == sizeof(wax::WaveLine), "wax_waveLine differs in size from wax::WaveLine!" );
static_assert( std::is_trivially_copyable<wax::WaveLine>::value, "wax::WaveLine is not trivially copyable" );



inline DeviceGen2* getPtr(wax_obj obj) {
	return static_cast<DeviceGen2*>(reinterpret_cast<Object*>(obj));
}

inline Edgemap* getEdgemapPtr(wax_obj obj) {
	return static_cast<Edgemap*>(reinterpret_cast<Object*>(obj));
}


wax_obj wax_createDevice( wax_obj backend )
{
	auto p = DeviceGen2::create( static_cast<Backend*>(reinterpret_cast<Object*>(backend)) );
	p->retain();
	return static_cast<Object*>(p);
}

int wax_setBackend( wax_obj device, wax_obj backend )
{
	return getPtr(device)->setBackend(static_cast<Backend*>(reinterpret_cast<Object*>(backend)));
}

wax_obj wax_getBackend( wax_obj device )
{
	return static_cast<Object*>(getPtr(device)->backend().rawPtr());
}


wax_canvasInfo wax_getCanvas(wax_obj device)
{
	auto& canvasInfo = getPtr(device)->canvas();
	return { (wax_canvasRef)canvasInfo.ref, canvasInfo.pSurface, {canvasInfo.size.w, canvasInfo.size.h}, canvasInfo.scale };
}


wax_canvasInfo wax_getCanvasRef(wax_obj device, wax_canvasRef ref)
{
	auto& canvasInfo = getPtr(device)->canvas((CanvasRef)ref);
	return { (wax_canvasRef)canvasInfo.ref, canvasInfo.pSurface, {canvasInfo.size.w, canvasInfo.size.h}, canvasInfo.scale };
}


wax_obj	wax_canvasLayers(wax_obj device)
{
	return getPtr(device)->canvasLayers().rawPtr();
}


wax_obj wax_surfaceFactory(wax_obj device)
{
	return getPtr(device)->surfaceFactory().rawPtr();
}


wax_obj wax_edgemapFactory(wax_obj device)
{
	return getPtr(device)->edgemapFactory().rawPtr();
}

wax_obj wax_warpmapFactory(wax_obj device)
{
	return getPtr(device)->warpmapFactory().rawPtr();
}


int	wax_maxSegments()
{
	return Device::maxSegments();
}


wax_sizeSPX wax_canvasSize(wax_obj device)
{
	auto canvasSize = getPtr(device)->canvasSize();
	return { canvasSize.w, canvasSize.h };
}


int wax_setClipList(wax_obj device, int nRectangles, const wax_rectSPX* pRectangles)
{
	return getPtr(device)->setClipList(nRectangles, (RectSPX*)pRectangles);
}


void wax_resetClipList(wax_obj device)
{
	getPtr(device)->resetClipList();
}


int	wax_pushClipList(wax_obj device, int nRectangles, const wax_rectSPX* pRectangles)
{
	return getPtr(device)->pushClipList(nRectangles, (RectSPX*)pRectangles);
}


int	wax_popClipList(wax_obj device)
{
	return getPtr(device)->popClipList();
}


const wax_rectSPX* wax_getClipList(wax_obj device)
{
	return (wax_rectSPX*) getPtr(device)->clipList();
}


int wax_clipListSize(wax_obj device)
{
 	return getPtr(device)->clipListSize();
}


const wax_rectSPX* wax_clipBounds(wax_obj device)
{
	return (wax_rectSPX*) &getPtr(device)->clipBounds();
}


void wax_setTintColor(wax_obj device, wax_color color)
{
	getPtr(device)->setTintColor( HiColor( color.r, color.g, color.b, color.a) );
}


wax_color wax_getTintColor(wax_obj device)
{
	auto color = getPtr(device)->tintColor();
	return { color.b, color.g, color.r, color.a };
}


void wax_clearTintColor(wax_obj device)
{
	getPtr(device)->clearTintColor();
}


int	wax_hasTintColor(wax_obj device)
{
	return getPtr(device)->hasTintColor();
}


void wax_setTint(wax_obj device, const wax_rectSPX* rect, const wax_obj tint)
{
	getPtr(device)->setTint( *(const RectSPX*)rect, static_cast<Tint*>(reinterpret_cast<Object*>(tint)) );
}


wax_obj wax_getTint(wax_obj device)
{
	return static_cast<Object*>(getPtr(device)->tint().rawPtr());
}


wax_rectSPX wax_getTintRect(wax_obj device)
{
	RectSPX rect = getPtr(device)->tintRect();
	return { rect.x, rect.y, rect.w, rect.h };
}


void wax_clearTint(wax_obj device)
{
	getPtr(device)->clearTint();
}


int wax_hasTint(wax_obj device)
{
	return getPtr(device)->hasTint();
}


int wax_setBlendMode(wax_obj device, wax_blendMode blendMode)
{
	return getPtr(device)->setBlendMode( (BlendMode) blendMode );
}


wax_blendMode wax_getBlendMode(wax_obj device)
{
	return (wax_blendMode) getPtr(device)->blendMode();
}


int wax_setBlitSource(wax_obj device, wax_obj surface)
{
	return getPtr(device)->setBlitSource( static_cast<Surface*>(reinterpret_cast<Object*>(surface)) );
}


wax_obj wax_getBlitSource(wax_obj device)
{
	return (wax_obj) getPtr(device)->blitSource();
}


void wax_setMorphFactor(wax_obj device, float factor)
{
	getPtr(device)->setMorphFactor(factor);
}


float wax_getMorphFactor(wax_obj device)
{
	return getPtr(device)->morphFactor();
}

void wax_setBlurMatrices(wax_obj device, wax_spx radius, const float red[9], const float green[9], const float blue[9] )
{
	// We need to create a Blurbrush on the fly

	auto pBlurbrush = Blurbrush::create( WAXBP(Blurbrush, _.size = radius, _.red = red, _.green = green, _.blue = blue) );

	getPtr(device)->setBlurbrush( pBlurbrush );
}

void wax_setBlurbrush(wax_obj device, wax_obj brush )
{
	Blurbrush* pBrush = brush == 0 ? nullptr : static_cast<Blurbrush*>(reinterpret_cast<Object*>(brush));

	getPtr(device)->setBlurbrush( pBrush );
}

wax_obj wax_getBlurbrush(wax_obj device)
{
	return static_cast<Object*>(getPtr(device)->blurbrush().rawPtr());
}


void wax_setFixedBlendColor(wax_obj device, wax_color color )
{
	getPtr(device)->setFixedBlendColor( HiColor( color.r, color.g, color.b, color.a) );
}


wax_color wax_getFixedBlendColor(wax_obj device)
{
	auto color = getPtr(device)->fixedBlendColor();
	return { color.b, color.g, color.r, color.a };
}


void wax_setRenderLayer(wax_obj device, int layer)
{
	getPtr(device)->setRenderLayer(layer);
}


int wax_getRenderLayer(wax_obj device)
{
	return getPtr(device)->renderLayer();
}


int wax_beginRender(wax_obj device)
{
	return getPtr(device)->beginRender();
}


int wax_endRender(wax_obj device)
{
	return getPtr(device)->endRender();
}


int wax_isDeviceRendering(wax_obj device)
{
	return getPtr(device)->isRendering();
}


int wax_isDeviceIdle(wax_obj device)
{
	return getPtr(device)->isIdle();
}


void wax_flushDevice(wax_obj device)
{
	getPtr(device)->flush();
}


int wax_beginCanvasUpdateWithRef(wax_obj device, wax_canvasRef canvas, int nUpdateRects, const wax_rectSPX* pUpdateRects, wax_obj canvasLayers, int startLayer)
{
	CanvasLayers* pLayers = canvasLayers == 0 ? nullptr : static_cast<CanvasLayers*>(reinterpret_cast<Object*>(canvasLayers));

	return getPtr(device)->beginCanvasUpdate( (CanvasRef) canvas, nUpdateRects, (const RectSPX*) pUpdateRects, 
			pLayers, startLayer );
}


int wax_beginCanvasUpdateWithSurface(wax_obj device, wax_obj surface, int nUpdateRects, const wax_rectSPX* pUpdateRects, wax_obj canvasLayers, int startLayer)
{
	CanvasLayers* pLayers = canvasLayers == 0 ? nullptr : static_cast<CanvasLayers*>(reinterpret_cast<Object*>(canvasLayers));

	return getPtr(device)->beginCanvasUpdate(static_cast<Surface*>(reinterpret_cast<Object*>(surface)), nUpdateRects, (const RectSPX*)pUpdateRects,
		pLayers, startLayer);
}


void wax_endCanvasUpdate(wax_obj device)
{
	getPtr(device)->endCanvasUpdate();
}

void wax_flattenLayers(wax_obj device)
{
	getPtr(device)->flattenLayers();
}

void wax_clearLayers(wax_obj device)
{
	getPtr(device)->clearLayers();
}

void wax_fill(wax_obj device, wax_color col)
{
	getPtr(device)->fill( HiColor(col.r, col.g, col.b, col.a) );
}

void wax_fillRect(wax_obj device, const wax_rectSPX* rect, wax_color col)
{
	getPtr(device)->fill( *(const RectSPX*) rect, HiColor(col.r, col.g, col.b, col.a) );
}


void wax_drawLine(wax_obj device, wax_coordSPX begin, wax_coordSPX end, wax_color color, wax_spx thickness)
{
	getPtr(device)->drawLine({ begin.x, begin.y }, { end.x, end.y }, * (HiColor*) &color, (spx) thickness);
}


void wax_drawStraightLine(wax_obj device, wax_coordSPX begin, wax_direction dir, wax_spx length, wax_color color, wax_spx thickness)
{
	getPtr(device)->drawLine( { begin.x, begin.y }, (Direction) dir, (spx) length, *(HiColor*)&color, (spx)thickness );
}

static LineCap toLineCap(wax_lineCap cap)
{
	return LineCap( (LineCapShape) cap.shape, (uint16_t) std::min(std::max(cap.spearLength, 0), 65535) );
}

void wax_drawLineWithCaps(wax_obj device, wax_coordSPX begin, wax_coordSPX end, wax_color color, wax_spx thickness, wax_lineCap beginCap, wax_lineCap endCap)
{
	getPtr(device)->drawLine({ begin.x, begin.y }, { end.x, end.y }, *(HiColor*)&color, (spx)thickness, { toLineCap(beginCap), toLineCap(endCap) });
}

void wax_drawStraightLineWithCaps(wax_obj device, wax_coordSPX begin, wax_direction dir, wax_spx length, wax_color color, wax_spx thickness, wax_lineCap beginCap, wax_lineCap endCap)
{
	getPtr(device)->drawLine({ begin.x, begin.y }, (Direction)dir, (spx)length, *(HiColor*)&color, (spx)thickness, { toLineCap(beginCap), toLineCap(endCap) });
}


void wax_blit(wax_obj device, wax_coordSPX dest)
{
	getPtr(device)->blit( {dest.x, dest.y} );
}


void wax_blitRect(wax_obj device, wax_coordSPX dest, const wax_rectSPX* src)
{
	getPtr(device)->blit( { dest.x, dest.y }, * (const RectSPX*) src );
}


void wax_flipBlit(wax_obj device, wax_coordSPX dest, wax_gfxFlip flip)
{
	getPtr(device)->flipBlit({ dest.x, dest.y }, (GfxFlip) flip );
}


void wax_flipBlitRect(wax_obj device, wax_coordSPX dest, const wax_rectSPX* src, wax_gfxFlip flip)
{
	getPtr(device)->flipBlit({ dest.x, dest.y }, *(const RectSPX*)src, (GfxFlip) flip );
}


void wax_stretchBlit(wax_obj device, const wax_rectSPX* dest)
{
	getPtr(device)->stretchBlit( *(const RectSPX*)dest );
}


void wax_stretchBlitRect(wax_obj device, const wax_rectSPX* dest, const wax_rectSPX* src)
{
	getPtr(device)->stretchBlit( *(const RectSPX*)dest, *(const RectSPX*)src );
}


void wax_stretchFlipBlit(wax_obj device, const wax_rectSPX* dest, wax_gfxFlip flip)
{
	getPtr(device)->stretchFlipBlit( *(const RectSPX*)dest, (GfxFlip) flip );
}


void wax_stretchFlipBlitRect(wax_obj device, const wax_rectSPX* dest, const wax_rectSPX* src, wax_gfxFlip flip)
{
	getPtr(device)->stretchFlipBlit(*(const RectSPX*)dest, *(const RectSPX*)src, (GfxFlip)flip);
}


void wax_precisionBlit(wax_obj device, const wax_rectSPX* dest, const wax_rectF* srcSPX)
{
	getPtr(device)->precisionBlit(*(const RectSPX*)dest, *(const RectF*)srcSPX );

}


void wax_transformBlit(wax_obj device, const wax_rectSPX* dest, wax_coordF srcSPX, const wax_transform * pTransform)
{
	getPtr(device)->transformBlit(*(const RectSPX*)dest, {srcSPX.x, srcSPX.y}, * reinterpret_cast<const wax::Transform*>(pTransform) );
}


void wax_rotScaleBlit(wax_obj device, const wax_rectSPX* dest, float rotationDegrees, float scale, wax_coordF srcCenter, wax_coordF destCenter)
{
	getPtr(device)->rotScaleBlit(*(const RectSPX*)dest, rotationDegrees, scale, {srcCenter.x, srcCenter.y}, {destCenter.x, destCenter.y} );
}


void wax_tile(wax_obj device, const wax_rectSPX* dest, wax_coordSPX shift)
{
	getPtr(device)->tile(*(const RectSPX*)dest, {shift.x, shift.y} );
}


void wax_flipTile(wax_obj device, const wax_rectSPX* dest, wax_gfxFlip flip, wax_coordSPX shift)
{
	getPtr(device)->flipTile(*(const RectSPX*)dest, (GfxFlip) flip, { shift.x, shift.y });
}


void wax_scaleTile(wax_obj device, const wax_rectSPX* dest, float scale, wax_coordSPX shift)
{
	getPtr(device)->scaleTile(*(const RectSPX*)dest, scale, { shift.x, shift.y });
}


void wax_scaleFlipTile(wax_obj device, const wax_rectSPX* dest, float scale, wax_gfxFlip flip, wax_coordSPX shift)
{
	getPtr(device)->scaleFlipTile(*(const RectSPX*)dest, scale, (GfxFlip) flip, {shift.x, shift.y});
}


void wax_drawWave(wax_obj device, const wax_rectSPX* dest, const wax_waveLine* pTopBorder, const wax_waveLine* pBottomBorder, wax_color fill)
{
	getPtr(device)->drawWave(*(const RectSPX*)dest, (WaveLine*) pTopBorder, (WaveLine*) pBottomBorder, * (HiColor*) &fill );
}


void wax_flipDrawWave(wax_obj device, const wax_rectSPX* dest, const wax_waveLine* pTopBorder, const wax_waveLine* pBottomBorder, wax_color fill, wax_gfxFlip flip)
{
	getPtr(device)->flipDrawWave(*(const RectSPX*)dest, (WaveLine*)pTopBorder, (WaveLine*)pBottomBorder, *(HiColor*)&fill, (GfxFlip) flip );
}


void wax_drawElipse(wax_obj device, const wax_rectSPX* canvas, wax_spx thickness, wax_color color, wax_spx outlineThickness, wax_color outlineColor)
{
	getPtr(device)->drawElipse(*(const RectSPX*)canvas, thickness, *(HiColor*)&color, outlineThickness, *(HiColor*)&outlineColor );
}


void wax_drawPieChart(wax_obj device, const wax_rectSPX* canvas, float start, int nSlices, const float* pSliceSizes, const wax_color* pSliceColors, float hubSize, wax_color hubColor, wax_color backColor, int bRectangular)
{
	getPtr(device)->drawPieChart(*(const RectSPX*)canvas, start, nSlices, pSliceSizes, (HiColor*)pSliceColors, hubSize, *(HiColor*)&hubColor, *(HiColor*)&backColor, bRectangular );
}


void wax_drawSegments(wax_obj device, const wax_rectSPX* dest, int nSegments, const wax_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wax_tintMode tintMode)
{
	getPtr(device)->drawSegments(*(const RectSPX*)dest, nSegments, (HiColor*)pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, (TintMode) tintMode );
}


void wax_flipDrawSegments(wax_obj device, const wax_rectSPX* dest, int nSegments, const wax_color* pSegmentColors, int nEdgeStrips, const int* pEdgeStrips, int edgeStripPitch, wax_gfxFlip flip, wax_tintMode tintMode)
{
	getPtr(device)->flipDrawSegments(*(const RectSPX*)dest, nSegments, (HiColor*)pSegmentColors, nEdgeStrips, pEdgeStrips, edgeStripPitch, (GfxFlip) flip, (TintMode)tintMode);
}


void wax_drawEdgemap(wax_obj device, wax_coordSPX dest, wax_obj edgemap)
{
	getPtr(device)->drawEdgemap({ dest.x, dest.y }, getEdgemapPtr(edgemap));
}


void wax_flipDrawEdgemap(wax_obj device, wax_coordSPX dest, wax_obj edgemap, wax_gfxFlip flip)
{
	getPtr(device)->flipDrawEdgemap({ dest.x, dest.y }, getEdgemapPtr(edgemap), (GfxFlip) flip );
}

void wax_warpBlit(wax_obj device, wax_coordSPX dest, wax_obj warpmap, wax_coordSPX sourceOffset)
{
	getPtr(device)->warpBlit({ dest.x, dest.y }, static_cast<Warpmap*>(reinterpret_cast<Object*>(warpmap)), { sourceOffset.x, sourceOffset.y });
}

void wax_warpBlitRect(wax_obj device, wax_coordSPX dest, wax_obj warpmap, const wax_rectSPX* source, wax_coordSPX sourceOffset)
{
	getPtr(device)->warpBlit({ dest.x, dest.y }, static_cast<Warpmap*>(reinterpret_cast<Object*>(warpmap)), *(const RectSPX*)source, { sourceOffset.x, sourceOffset.y });
}

void wax_warpDrawEdgemap(wax_obj device, wax_coordSPX dest, wax_obj warpmap, wax_obj edgemap, wax_coordSPX sourceOffset)
{
	getPtr(device)->warpDrawEdgemap({ dest.x, dest.y }, static_cast<Warpmap*>(reinterpret_cast<Object*>(warpmap)), getEdgemapPtr(edgemap), { sourceOffset.x, sourceOffset.y });
}

void wax_tileEdgemap(wax_obj device, const wax_rectSPX* dest, wax_obj edgemap, wax_coordSPX shift)
{
	getPtr(device)->tileEdgemap(*(const RectSPX*)dest, getEdgemapPtr(edgemap), { shift.x, shift.y });
}

void wax_flipTileEdgemap(wax_obj device, const wax_rectSPX* dest, wax_obj edgemap, wax_gfxFlip flip, wax_coordSPX shift)
{
	getPtr(device)->flipTileEdgemap(*(const RectSPX*)dest, getEdgemapPtr(edgemap), (GfxFlip) flip, { shift.x, shift.y });
}


void wax_drawRoundedBox(wax_obj device, const wax_roundedBoxSpec* spec)
{
	RoundedBoxSpec box;

	box.rect = *(const RectSPX*)&spec->rect;

	for (int i = 0; i < 4; i++)
		box.radius[i] = spec->radius[i];

	box.outline = *(const BorderSPX*)&spec->outline;
	box.fillColor = *(const HiColor*)&spec->fillColor;
	box.outlineColor = *(const HiColor*)&spec->outlineColor;
	box.pFillTint = static_cast<Tint*>(reinterpret_cast<Object*>(spec->fillTint));
	box.pOutlineTint = static_cast<Tint*>(reinterpret_cast<Object*>(spec->outlineTint));

	getPtr(device)->drawRoundedBox(box);
}

void wax_drawRing(wax_obj device, const wax_ringSpec* spec)
{
	// Invalid slices are passed on for Device to report.

	int nSlices = std::min(spec->nSlices, Device::maxRingSlices());
	int nCopied = spec->pSlices ? std::max(nSlices, 0) : 0;

	int bufferSize = sizeof(RingSlice) * std::max(nCopied, 1);
	auto slices = (RingSlice*) Core::memStackAlloc(bufferSize);

	for (int i = 0; i < nCopied; i++)
	{
		new (&slices[i]) RingSlice{ spec->pSlices[i].size, *(const HiColor*)&spec->pSlices[i].color,
									static_cast<Tint*>(reinterpret_cast<Object*>(spec->pSlices[i].tint)) };
	}

	RingSpec ring;

	ring.rect = *(const RectSPX*)&spec->rect;
	ring.thickness = spec->thickness;
	ring.outline = spec->outline;
	ring.start = spec->start;
	ring.nSlices = nSlices;
	ring.pSlices = spec->pSlices ? slices : nullptr;
	ring.bSquare = spec->bSquare != 0;
	ring.outlineColor = *(const HiColor*)&spec->outlineColor;
	ring.pOutlineTint = static_cast<Tint*>(reinterpret_cast<Object*>(spec->outlineTint));
	ring.hubColor = *(const HiColor*)&spec->hubColor;
	ring.pHubTint = static_cast<Tint*>(reinterpret_cast<Object*>(spec->hubTint));
	ring.backColor = *(const HiColor*)&spec->backColor;
	ring.pBackTint = static_cast<Tint*>(reinterpret_cast<Object*>(spec->backTint));

	getPtr(device)->drawRing(ring);

	Core::memStackFree(bufferSize);
}

void wax_blitNinePatch(wax_obj device, const wax_rectSPX* dstRect, const wax_borderSPX* dstFrame, const wax_ninePatch* patch, int scale)
{
	getPtr(device)->blitNinePatch(*(const RectSPX*)dstRect, *(BorderSPX*) dstFrame, * (NinePatch*) patch, scale);
}

void wax_blur(wax_obj device, wax_coordSPX dest)
{
	getPtr(device)->blur( {dest.x, dest.y} );
}

void wax_blurRect(wax_obj device, wax_coordSPX dest, const wax_rectSPX* src)
{
	getPtr(device)->blur( { dest.x, dest.y }, * (const RectSPX*) src );
}

void wax_stretchBlur(wax_obj device, const wax_rectSPX* dest)
{
	getPtr(device)->stretchBlur( *(const RectSPX*)dest );
}

void wax_stretchBlurRect(wax_obj device, const wax_rectSPX* dest, const wax_rectSPX* src)
{
	getPtr(device)->stretchBlur( *(const RectSPX*)dest, *(const RectSPX*)src );
}

void wax_transformBlur(wax_obj device, const wax_rectSPX* dest, wax_coordF srcSPX, const wax_transform* pTransform)
{
	getPtr(device)->transformBlur(*(const RectSPX*)dest, {srcSPX.x, srcSPX.y}, *reinterpret_cast<const wax::Transform*>(pTransform) );
}

void wax_rotScaleBlur(wax_obj device, const wax_rectSPX* dest, float rotationDegrees, float scale, wax_coordF srcCenter, wax_coordF destCenter)
{
	getPtr(device)->rotScaleBlur(*(const RectSPX*)dest, rotationDegrees, scale, {srcCenter.x, srcCenter.y}, {destCenter.x, destCenter.y} );
}

