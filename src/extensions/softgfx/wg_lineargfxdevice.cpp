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

#include <wg_lineargfxdevice.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

using namespace std;

namespace wg
{
	using namespace Util;

	const TypeInfo LinearGfxDevice::TYPEINFO = { "LinearGfxDevice", &SoftGfxDevice::TYPEINFO };

//____ create() _______________________________________________________________

LinearGfxDevice_p LinearGfxDevice::create( std::function<void*(CanvasRef ref, int nBytes)> beginCanvasRender,
											std::function<void(CanvasRef ref, int nSegments, const Segment * pSegments)> endCanvasRender )
{
	return LinearGfxDevice_p(new LinearGfxDevice(beginCanvasRender, endCanvasRender));
}

//____ constructor _____________________________________________________________

LinearGfxDevice::LinearGfxDevice( std::function<void*(CanvasRef ref, int nBytes)> beginCanvasRender,
								   std::function<void(CanvasRef ref, int nSegments, const Segment * pSegments)> endCanvasRender )
	: m_beginCanvasRenderCallback(beginCanvasRender),
	m_endCanvasRenderCallback(endCanvasRender)
{
}

//____ Destructor ______________________________________________________________

LinearGfxDevice::~LinearGfxDevice()
{

}

//____ typeInfo() _________________________________________________________

const TypeInfo& LinearGfxDevice::typeInfo(void) const
{
	return TYPEINFO;
}

//____ defineCanvas() _________________________________________________________

bool LinearGfxDevice::defineCanvas( CanvasRef ref, const SizeSPX size, PixelFormat pixelFormat, int scale )
{
	auto& def = m_canvasDefinitions[int(ref)];
	def.size = size;
	def.scale = scale;
	def.ref = ref;
	def.pSurface = nullptr;
	def.format = pixelFormat;
	return true;
}

//____ canvas() _____________________________________________________

const CanvasInfo LinearGfxDevice::canvas(CanvasRef ref) const
{
	if( m_canvasDefinitions[int(ref)].ref == ref )
		return m_canvasDefinitions[int(ref)];

	return m_dummyCanvas;
}

//____ setSegmentPadding() ____________________________________________________

void LinearGfxDevice::setSegmentPadding( int bytes )
{
	m_segmentPadding = bytes;
}

//____ beginRender() __________________________________________________________

bool LinearGfxDevice::beginRender()
{
	m_nbSurfaceCanvases = 0;
	return SoftGfxDevice::beginRender();
}

//____ endRender() ____________________________________________________________

bool LinearGfxDevice::endRender()
{
	return SoftGfxDevice::endRender();
}

//____ _beginCanvasUpdate() ___________________________________________________

bool LinearGfxDevice::_beginCanvasUpdate(CanvasRef ref, Surface * pCanvas, int nUpdateRects, const RectSPX* pUpdateRects, CanvasLayers * pLayers, int startLayer)
{
	 // For now we assume that there will only be one CanvasRef in the Canvas begin/end-updateStack and that it will be the first.
	
	SoftGfxDevice::_beginCanvasUpdate(ref, pCanvas, nUpdateRects, pUpdateRects, pLayers, startLayer);

	if( pCanvas )
	{
		m_nbSurfaceCanvases++;
	}
	else
	{
		nUpdateRects = m_nClipRects;
		pUpdateRects = m_pClipRects;
		
		m_canvasSegments.resize(nUpdateRects);

		m_canvasPixelBytes = Util::pixelFormatToDescription(m_canvas.format).bits/8;

		int bytesNeeded = 0;
		for( int i = 0 ; i < nUpdateRects ; i++ )
			bytesNeeded += (pUpdateRects[i].w * pUpdateRects[i].h)/(64*64);

		bytesNeeded *= m_canvasPixelBytes;
		bytesNeeded += nUpdateRects*m_segmentPadding;
		
		uint8_t * pCanvasBuffer = (uint8_t*) m_beginCanvasRenderCallback( ref, bytesNeeded );
		
		int ofs = 0;
		for( int i = 0 ; i < nUpdateRects ; i++ )
		{
			RectI updateRect = pUpdateRects[i]/64;
			
			m_canvasSegments[i].rect = updateRect;
			m_canvasSegments[i].pitch = updateRect.w * m_canvasPixelBytes;
			m_canvasSegments[i].pBuffer = pCanvasBuffer + ofs;
			ofs += updateRect.w * updateRect.h * m_canvasPixelBytes + m_segmentPadding;
		}
		
		m_pClipSegments = m_canvasSegments.data();
		m_nClipSegments = (int) m_canvasSegments.size();
	}
	
	return true;
}

//____ endCanvasUpdate() ______________________________________________________

void LinearGfxDevice::endCanvasUpdate()
{
	if( m_nbSurfaceCanvases > 0 )
	{
		m_nbSurfaceCanvases--;
	}
	else
	{
		m_endCanvasRenderCallback( m_canvas.ref, (int) m_canvasSegments.size(), m_canvasSegments.data() );
	}

	SoftGfxDevice::endCanvasUpdate();
}

//____ _canvasWasChanged() ________________________________________________

void LinearGfxDevice::_canvasWasChanged()
{
	if(m_canvas.pSurface)
	{
		auto pSurface = m_canvas.pSurface;
		
		m_layerSurfaces[0] = pSurface;
		m_pRenderLayerSurface = pSurface;
		
		m_canvasPixelBuffer = pSurface->allocPixelBuffer();
		pSurface->pushPixels(m_canvasPixelBuffer);

		m_pCanvasPixels = m_canvasPixelBuffer.pixels;
		m_canvasPitch = m_canvasPixelBuffer.pitch;

		m_canvasPixelBits = pSurface->pixelDescription()->bits;
		m_canvasPixelFormat = pSurface->pixelFormat();
		
		_updateTintSettings();
		_updateBlitFunctions();
	}
	else if(m_canvas.ref != CanvasRef::None)
	{
		m_layerSurfaces[0] = nullptr;
		m_pCanvasPixels = nullptr;
		m_canvasPitch = 0;

		m_canvasPixelFormat 	= m_canvas.format;
		m_canvasPixelBits 		= Util::pixelFormatToDescription(m_canvas.format).bits;

		_updateTintSettings();
		_updateBlitFunctions();
	}
	else
	{
		// Clean up.

		if (m_pRenderLayerSurface)
		{
			m_pRenderLayerSurface->pullPixels(m_canvasPixelBuffer);
			m_pRenderLayerSurface->freePixelBuffer(m_canvasPixelBuffer);
		}

		m_pRenderLayerSurface = nullptr;

		m_pCanvasPixels = nullptr;
		m_canvasPixelBits = 0;
		m_canvasPitch = 0;
		m_canvasPixelFormat = PixelFormat::Undefined;

		_updateBlitFunctions();		// Good to have dummies in place when we are not allowed to blit.
	}
}

//____ _clipListWasChanged() __________________________________________________

void LinearGfxDevice::_clipListWasChanged()
{
	// We assume that clipRects comes in same order as canvas updateRects.
	
	if( m_pClipRects == m_pCanvasUpdateRects )
	{
		m_pClipSegments = m_canvasSegments.data();
		m_nClipSegments = (int) m_canvasSegments.size();
	}
	else
	{
		m_clipSegments.resize(m_nClipRects);
		
		for( int i = 0 ; i < m_nClipRects ; i++ )
		{
			RectI clipRect = m_pClipRects[i]/64;

			m_clipSegments[i].rect = clipRect;
						
			CoordI pos = clipRect.pos();

			const Segment* pCanvasSeg = m_canvasSegments.data();
			while( !pCanvasSeg->rect.contains(pos) )
				pCanvasSeg++;

			m_clipSegments[i].pBuffer = pCanvasSeg->pBuffer + (pos.y - pCanvasSeg->rect.y) * pCanvasSeg->pitch + (pos.x - pCanvasSeg->rect.x) * m_canvasPixelBytes;

			m_clipSegments[i].pitch = pCanvasSeg->pitch;
		}
		
		m_pClipSegments = m_clipSegments.data();
		m_nClipSegments = (int) m_clipSegments.size();
	}
}

//____ _updateBlitFunctions() _________________________________________________

void LinearGfxDevice::_updateBlitFunctions()
{
	SoftGfxDevice::_updateBlitFunctions();

	if( m_pStraightBlitOp == &SoftGfxDevice::_onePassStraightBlit )
		m_pLinearStraightBlitOp = &LinearGfxDevice::_onePassLinearStraightBlit;
	else if( m_pStraightBlitOp == &SoftGfxDevice::_twoPassStraightBlit )
		m_pLinearStraightBlitOp = &LinearGfxDevice::_twoPassLinearStraightBlit;
	else
		m_pLinearStraightBlitOp = &LinearGfxDevice::_dummyLinearStraightBlit;

	if( m_pStraightTileOp == &SoftGfxDevice::_onePassStraightBlit )
		m_pLinearStraightTileOp = &LinearGfxDevice::_onePassLinearStraightBlit;
	else if( m_pStraightTileOp == &SoftGfxDevice::_twoPassStraightBlit )
		m_pLinearStraightTileOp = &LinearGfxDevice::_twoPassLinearStraightBlit;
	else
		m_pLinearStraightTileOp = &LinearGfxDevice::_dummyLinearStraightBlit;

	if( m_pTransformBlitOp == &SoftGfxDevice::_onePassTransformBlit )
		m_pLinearTransformBlitOp = &LinearGfxDevice::_onePassLinearTransformBlit;
	else if( m_pTransformBlitOp == &SoftGfxDevice::_twoPassTransformBlit )
		m_pLinearTransformBlitOp = &LinearGfxDevice::_twoPassLinearTransformBlit;
	else
		m_pLinearTransformBlitOp = &LinearGfxDevice::_dummyLinearTransformBlit;

	if( m_pTransformTileOp == &SoftGfxDevice::_onePassTransformBlit )
		m_pLinearTransformTileOp = &LinearGfxDevice::_onePassLinearTransformBlit;
	else if( m_pTransformTileOp == &SoftGfxDevice::_twoPassTransformBlit )
		m_pLinearTransformTileOp = &LinearGfxDevice::_twoPassLinearTransformBlit;
	else
		m_pLinearTransformTileOp = &LinearGfxDevice::_dummyLinearTransformBlit;

	
	
}


//____ fill() _________________________________________________________________

void LinearGfxDevice::fill(const RectSPX& rect, HiColor col)
{
	if( m_canvas.pSurface )
	{
		SoftGfxDevice::fill(rect,col);
		return;
	}
	
	// Clipping

	if( !m_clipBounds.isOverlapping(rect) )
		return;


	// Skip calls that won't affect destination

	if ((col.a == 0 || m_tintColor.a == 0) && (m_blendMode == BlendMode::Blend || m_blendMode == BlendMode::Add || m_blendMode == BlendMode::Subtract ) )
		return;

	// Optimize calls

	BlendMode blendMode = m_blendMode;
	if (blendMode == BlendMode::Blend && col.a == 4096 && m_bTintOpaque)
	{
		blendMode = BlendMode::Replace;
	}


	// No subpixel precision, make it quick and easy

	RectI pixelRect = roundToPixels(rect);

	int pixelBytes = m_canvasPixelBits / 8;
	FillOp_p pFunc = nullptr;
	
	auto pKernels = m_pKernels[(int)m_canvasPixelFormat];
	if (pKernels)
		pFunc = pKernels->pFillKernels[(int)m_colTrans.mode][(int)blendMode];

	if (pFunc == nullptr)
	{
		if( m_blendMode == BlendMode::Ignore )
			return;
		
		char errorMsg[1024];
		
		snprintf(errorMsg, 1024, "Failed fill operation. SoftGfxDevice is missing fill kernel for TintMode::%s, BlendMode::%s onto surface of PixelFormat:%s.",
			toString(m_colTrans.mode),
			toString(m_blendMode),
			toString(m_canvasPixelFormat));
		
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	for (int i = 0; i < m_nClipSegments; i++)
	{
		Segment& seg = m_pClipSegments[i];
		
		RectI patch =  RectI::overlap(seg.rect, pixelRect);
		if (patch.w == 0 || patch.h == 0)
			continue;

		uint8_t * pDst = seg.pBuffer + (patch.y - seg.rect.y)*seg.pitch + (patch.x - seg.rect.x)*m_canvasPixelBytes;
		
		pFunc(pDst, m_canvasPixelBytes, seg.pitch - patch.w*m_canvasPixelBytes, patch.h, patch.w, col, m_colTrans, patch.pos());
	}
}



//____ plotPixels() _________________________________________________

void LinearGfxDevice::plotPixels(int nCoords, const CoordSPX * pCoords, const HiColor * pColors)
{
	PlotListOp_p pOp = nullptr;
	auto pKernels = m_pKernels[(int)m_canvasPixelFormat];
	if (pKernels)
		pOp = pKernels->pPlotListKernels[(int)m_blendMode];

	if (pOp == nullptr )
	{
		if( m_blendMode == BlendMode::Ignore )
			return;
		
		char errorMsg[1024];
		
		snprintf(errorMsg, 1024, "Failed plotPixels operation. SoftGfxDevice is missing plotList kernel for BlendMode::%s onto surface of PixelFormat:%s.",
			toString(m_blendMode),
			toString(m_canvasPixelFormat) );
		
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	for (int i = 0; i < m_nClipSegments; i++)
	{
		auto& seg = m_pClipSegments[i];

		uint8_t * pCanvas = seg.pBuffer - seg.rect.y * seg.pitch - seg.rect.x * m_canvasPixelBytes;
		
		pOp(seg.rect*64, nCoords, pCoords, pColors, pCanvas, m_canvasPixelBytes, seg.pitch, m_colTrans);
	}
}
//____ drawLine() ____ [from/to] __________________________________________

void LinearGfxDevice::drawLine(CoordSPX beg, CoordSPX end, HiColor color, spx thickness)
{
	if( m_canvas.pSurface )
	{
		SoftGfxDevice::drawLine(beg, end, color, thickness);
		return;
	}
	
	//TODO: Proper 26:6 support
	beg = roundToPixels(beg);
	end = roundToPixels(end);

	HiColor fillColor = color * m_tintColor;

	// Skip calls that won't affect destination

	if (fillColor.a == 0 && (m_blendMode == BlendMode::Blend || m_blendMode == BlendMode::Add || m_blendMode == BlendMode::Subtract))
		return;

	//

	ClipLineOp_p pOp = nullptr;

	auto pKernels = m_pKernels[(int)m_canvasPixelFormat];
	if (pKernels)
		pOp = pKernels->pClipLineKernels[(int)m_blendMode];

	if (pOp == nullptr )
	{
		if( m_blendMode == BlendMode::Ignore )
			return;
		
		char errorMsg[1024];
		
		snprintf(errorMsg, 1024, "Failed drawLine operation. SoftGfxDevice is missing clipLine kernel for BlendMode::%s onto surface of PixelFormat:%s.",
			toString(m_blendMode),
			toString(m_canvasPixelFormat) );
		
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	if (std::abs(beg.x - end.x) > std::abs(beg.y - end.y))
	{
		// Prepare mainly horizontal line segment

		if (beg.x > end.x)
			swap(beg, end);

		int length = end.x - beg.x;
		int slope = ((end.y - beg.y) * 65536) / length;

		int width = _scaleLineThickness(thickness/64.f, slope);



		// Loop through patches

		for( int i = 0 ; i < m_nClipSegments ; i++ )
		{
			Segment& seg = m_pClipSegments[i];

			int pos = ((beg.y-seg.rect.y) << 16) - width / 2 + 32768;

			int rowInc = m_canvasPixelBytes;
			int pixelInc = seg.pitch;

			// Do clipping

			int _length = length;
			uint8_t * pRow = seg.pBuffer + (beg.x - seg.rect.x) * rowInc;

			if (beg.x < seg.rect.x)
			{
				int cut = seg.rect.x - beg.x;
				_length -= cut;
				pRow = seg.pBuffer;
				pos += slope * cut;
			}

			if (end.x > seg.rect.x + seg.rect.w)
				_length -= end.x - (seg.rect.x + seg.rect.w);

			int clipStart = 0;
			int clipEnd = seg.rect.h << 16;

			//  Draw

			pOp(clipStart, clipEnd, pRow, rowInc, pixelInc, _length, width, pos, slope, fillColor, m_colTrans, { 0,0 });
		}
	}
	else
	{
		// Prepare mainly vertical line segment

		if (beg.y > end.y)
			swap(beg, end);

		int length = end.y - beg.y;
		if (length == 0)
			return;											// TODO: Should stil draw the caps!

		// Need multiplication instead of shift as operand might be negative
		int slope = ((end.x - beg.x) * 65536) / length;
		int width = _scaleLineThickness(thickness/64.f, slope);


		// Loop through patches

		for (int i = 0; i < m_nClipSegments; i++)
		{
			Segment& seg = m_pClipSegments[i];
			
			int pos = ((beg.x-seg.rect.x) << 16) - width / 2 + 32768;


			int rowInc = seg.pitch;
			int pixelInc = m_canvasPixelBytes;


			// Do clipping

			int _length = length;
			uint8_t * pRow = seg.pBuffer + (beg.y - seg.rect.y) * rowInc;

			if (beg.y < seg.rect.y)
			{
				int cut = seg.rect.y - beg.y;
				_length -= cut;
				pRow = seg.pBuffer;
				pos += slope * cut;
			}

			if (end.y > seg.rect.y + seg.rect.h)
				_length -= end.y - (seg.rect.y + seg.rect.h);

			int clipStart = 0;
			int clipEnd = seg.rect.w << 16;

			//  Draw

			pOp(clipStart, clipEnd, pRow, rowInc, pixelInc, _length, width, pos, slope, fillColor, m_colTrans, { 0,0 });
		}
	}
}


//____ drawLine() ____ [start/direction] ________________________________

// Coordinates for start are considered to be + 0.5 in the width dimension, so they start in the middle of a line/column.
// A one pixel thick line will only be drawn one pixel think, while a two pixels thick line will cover three pixels in thickness,
// where the outer pixels are faded.

void LinearGfxDevice::drawLine(CoordSPX _begin, Direction dir, spx _length, HiColor _col, spx _thickness)
{
	if( m_canvas.pSurface )
	{
		SoftGfxDevice::drawLine(_begin, dir, _length, _col, _thickness);
		return;
	}
	
	float thickness = _thickness / 64.f;

	//TODO: Proper 26:6 support
	_begin = roundToPixels(_begin);
	_length = roundToPixels(_length);

	//TODO: Optimize!

	if (thickness <= 0.f)
		return;

	BlendMode edgeBlendMode = m_blendMode;
	if (m_blendMode == BlendMode::Replace)
	{
		_col.a = 4096;							// Needed since we still blend the edges.
		edgeBlendMode = BlendMode::Blend;
	}

	_col = _col * m_tintColor;

	int pixelBytes = m_canvasPixelBits / 8;
	FillOp_p	pCenterOp = nullptr;
	FillOp_p	pEdgeOp = nullptr;

	auto pKernels = m_pKernels[(int)m_canvasPixelFormat];
	if (pKernels)
	{
		pCenterOp = pKernels->pFillKernels[(int)TintMode::None][(int)m_blendMode];
		pEdgeOp = pKernels->pFillKernels[(int)TintMode::None][(int)edgeBlendMode];
	}


	if (pCenterOp == nullptr || pEdgeOp == nullptr )
	{
		if( m_blendMode == BlendMode::Ignore )
			return;
		
		char errorMsg[1024];
		
		snprintf(errorMsg, 1024, "Failed drawLine operation. SoftGfxDevice is missing fill kernel for BlendMode::%s onto surface of PixelFormat:%s.",
			toString(m_blendMode),
			toString(m_canvasPixelFormat) );
		
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	
	
	for (int i = 0; i < m_nClipSegments; i++)
	{
		const Segment& clipSeg = m_pClipSegments[i];
		
		const RectI& clip = clipSeg.rect;

		CoordI begin = _begin;
		int length = _length;

		switch (dir)
		{
		case Direction::Left:
			begin.x -= length;
		case Direction::Right:
		{
			if (begin.x > clip.x + clip.w)
				continue;

			if (begin.x < clip.x)
			{
				length -= clip.x - begin.x;
				if (length <= 0)
					continue;
				begin.x = clip.x;
			}

			if (begin.x + length > clip.x + clip.w)
			{
				length = clip.x + clip.w - begin.x;
				if (length <= 0)
					continue;
			}

			if (thickness <= 1.f)
			{
				if (begin.y < clip.y || begin.y >= clip.y + clip.h)
					continue;

				HiColor col = _col;
				col.a = (int16_t)(thickness * col.a);

				uint8_t * pBegin = clipSeg.pBuffer + (begin.y - clip.y) * clipSeg.pitch + (begin.x - clip.x) * pixelBytes;
				pEdgeOp(pBegin, pixelBytes, 0, 1, length, col, m_colTrans, { 0,0 });
			}
			else
			{
				int expanse = (int)(1 + (thickness - 1) / 2);
				HiColor edgeColor(_col.r, _col.g, _col.b, (int16_t)(_col.a * ((thickness - 1) / 2 - (expanse - 1))));

				if (begin.y + expanse <= clip.y || begin.y - expanse >= clip.y + clip.h)
					continue;

				int beginY = begin.y - expanse;
				int endY = begin.y + expanse + 1;

				if (beginY < clip.y)
					beginY = clip.y - 1;
				else
				{
					uint8_t * pBegin = clipSeg.pBuffer + (beginY - clip.y) * clipSeg.pitch + (begin.x - clip.x) * pixelBytes;
					pEdgeOp(pBegin, pixelBytes, 0, 1, length, edgeColor, m_colTrans, { 0,0 });
				}

				if (endY > clip.y + clip.h)
					endY = clip.y + clip.h + 1;
				else
				{
					uint8_t * pBegin = clipSeg.pBuffer + (endY - 1 - clip.y) * clipSeg.pitch + (begin.x - clip.x) * pixelBytes;
					pEdgeOp(pBegin, pixelBytes, 0, 1, length, edgeColor, m_colTrans, { 0,0 });
				}

				int bodyThickness = endY - beginY - 2;
				uint8_t * pBegin = clipSeg.pBuffer  + (beginY + 1 -clip.y) * clipSeg.pitch + (begin.x - clip.x) * pixelBytes;
				pCenterOp(pBegin, pixelBytes, clipSeg.pitch - length * pixelBytes, bodyThickness, length, _col, m_colTrans, { 0,0 });
			}

			break;
		}
		case Direction::Up:
			begin.y -= length;
		case Direction::Down:
			if (begin.y > clip.y + clip.h)
				continue;

			if (begin.y < clip.y)
			{
				length -= clip.y - begin.y;
				if (length <= 0)
					continue;
				begin.y = clip.y;
			}

			if (begin.y + length > clip.y + clip.h)
			{
				length = clip.y + clip.h - begin.y;
				if (length <= 0)
					continue;
			}

			if (thickness <= 1.f)
			{
				if (begin.x < clip.x || begin.x >= clip.x + clip.w)
					continue;

				HiColor col = _col;
				col.a = (int16_t)(thickness * col.a);

				uint8_t * pBegin = clipSeg.pBuffer + (begin.y - clip.y) * clipSeg.pitch + (begin.x - clip.x) * pixelBytes;
				pEdgeOp(pBegin, clipSeg.pitch, 0, 1, length, col, m_colTrans, { 0,0 });
			}
			else
			{
				int expanse = (int)(1 + (thickness - 1) / 2);
				HiColor edgeColor(_col.r, _col.g, _col.b, (int16_t)(_col.a * ((thickness - 1) / 2 - (expanse - 1))));

				if (begin.x + expanse <= clip.x || begin.x - expanse >= clip.x + clip.w)
					continue;

				int beginX = begin.x - expanse;
				int endX = begin.x + expanse + 1;

				if (beginX < clip.x)
					beginX = clip.x - 1;
				else
				{
					uint8_t * pBegin = clipSeg.pBuffer + (begin.y - clip.y) * clipSeg.pitch + (beginX - clip.x) * pixelBytes;
					pEdgeOp(pBegin, clipSeg.pitch, 0, 1, length, edgeColor, m_colTrans, { 0,0 });
				}

				if (endX > clip.x + clip.w)
					endX = clip.x + clip.w + 1;
				else
				{
					uint8_t * pBegin = clipSeg.pBuffer + (begin.y - clip.y) * clipSeg.pitch + (endX - 1 - clip.x) * pixelBytes;
					pEdgeOp(pBegin, clipSeg.pitch, 0, 1, length, edgeColor, m_colTrans, { 0,0 });
				}


				int bodyThickness = endX - beginX - 2;
				uint8_t * pBegin = clipSeg.pBuffer + (begin.y - clip.y) * clipSeg.pitch + (beginX + 1 - clip.x) * pixelBytes;
				pCenterOp(pBegin, pixelBytes, clipSeg.pitch - pixelBytes * bodyThickness, length, bodyThickness, _col, m_colTrans, { 0,0 });

				
				
//				uint8_t * pBegin = clipSeg.pBuffer  + (beginY + 1 -clip.y) * clipSeg.pitch + (begin.x - clip.x) * pixelBytes;
//				pCenterOp(pBegin, pixelBytes, m_canvasPitch - length * pixelBytes, bodyThickness, length, _col, m_colTrans, { 0,0 });

//				void _fill(uint8_t* pDst, int pitchX, int pitchY, int nLines, int lineLength, HiColor col, const SoftGfxDevice::ColTrans& tint, CoordI patchPos)

				
			}

			break;
		}
	}
}

//____ _transformBlitSimple() _________________________________________________

void LinearGfxDevice::_transformBlitSimple(const RectSPX& _dest, CoordSPX _src, const int simpleTransform[2][2], OpType type)
{

	if( m_canvas.pSurface )
	{
		SoftGfxDevice::_transformBlitSimple(_dest, _src, simpleTransform, type);
		return;
	}
	
	// For this method, source and dest should be pixel aligned.

	// Clip and render the patches

	if (!_dest.isOverlapping(m_clipBounds))
		return;


	//TODO: Proper 26:6 support
	RectI dest = roundToPixels(_dest);

	const RectI& clip = dest;

	// Step forward _src by half a pixel, so we start from correct pixel.

	_src.x = _src.x + (simpleTransform[0][0] + simpleTransform[1][0])*32;
	_src.y = _src.y + (simpleTransform[0][1] + simpleTransform[1][1])*32;

	//

	for (int i = 0; i < m_nClipSegments; i++)
	{
		RectI  patch = m_pClipSegments[i].rect;

		CoordI src = _src/64;

		CoordI	patchOfs = patch.pos() - dest.pos();


		if ((clip.x > patch.x) || (clip.x + clip.w < patch.x + patch.w) ||
			(clip.y > patch.y) || (clip.y + clip.h < patch.y + patch.h))
		{

			if ((clip.x > patch.x + patch.w) || (clip.x + clip.w < patch.x) ||
				(clip.y > patch.y + patch.h) || (clip.y + clip.h < patch.y))
				continue;																					// Totally outside clip-rect.

			if (patch.x < clip.x)
			{
				int xDiff = clip.x - patch.x;
				patch.w -= xDiff;
				patch.x = clip.x;
				patchOfs.x += xDiff;
			}

			if (patch.y < clip.y)
			{
				int yDiff = clip.y - patch.y;
				patch.h -= yDiff;
				patch.y = clip.y;
				patchOfs.y += yDiff;
			}

			if (patch.x + patch.w > clip.x + clip.w)
				patch.w = (clip.x + clip.w) - patch.x;

			if (patch.y + patch.h > clip.y + clip.h)
				patch.h = (clip.y + clip.h) - patch.y;
		}

		//

		src.x += patchOfs.x * simpleTransform[0][0] + patchOfs.y * simpleTransform[1][0];
		src.y += patchOfs.x * simpleTransform[0][1] + patchOfs.y * simpleTransform[1][1];

		uint8_t * pDst = m_pClipSegments[i].pBuffer + (patch.y-m_pClipSegments[i].rect.y) * m_pClipSegments[i].pitch + (patch.x - m_pClipSegments[i].rect.x) * m_canvasPixelBytes;
		
		
		if( type == OpType::Tile )
			(this->*m_pLinearStraightTileOp)(pDst, m_pClipSegments[i].pitch, patch.w, patch.h, src, simpleTransform, patch.pos(), m_pStraightTileFirstPassOp);
		else
			(this->*m_pLinearStraightBlitOp)(pDst, m_pClipSegments[i].pitch, patch.w, patch.h, src, simpleTransform, patch.pos(), m_pStraightBlitFirstPassOp);
	}
}



//____ _transformBlitComplex() _______________________________________________

void LinearGfxDevice::_transformBlitComplex(const RectSPX& _dest, BinalCoord _src, const binalInt complexTransform[2][2], OpType type)
{
	if( m_canvas.pSurface )
	{
		SoftGfxDevice::_transformBlitComplex(_dest, _src, complexTransform, type);
		return;
	}

	// Clip and render the patches

	if (!_dest.isOverlapping(m_clipBounds))
		return;

	//TODO: Proper 26:6 support
	RectI dest = roundToPixels(_dest);

	const RectI& clip = dest;

	for (int i = 0; i < m_nClipSegments; i++)
	{
		RectI  patch = m_pClipSegments[i].rect;

		BinalCoord src = _src;

		CoordI	patchOfs = patch.pos() - dest.pos();


		if ((clip.x > patch.x) || (clip.x + clip.w < patch.x + patch.w) ||
			(clip.y > patch.y) || (clip.y + clip.h < patch.y + patch.h))
		{

			if ((clip.x > patch.x + patch.w) || (clip.x + clip.w < patch.x) ||
				(clip.y > patch.y + patch.h) || (clip.y + clip.h < patch.y))
				continue;																					// Totally outside clip-rect.

			if (patch.x < clip.x)
			{
				int xDiff = clip.x - patch.x;
				patch.w -= xDiff;
				patch.x = clip.x;
				patchOfs.x += xDiff;
			}

			if (patch.y < clip.y)
			{
				int yDiff = clip.y - patch.y;
				patch.h -= yDiff;
				patch.y = clip.y;
				patchOfs.y += yDiff;
			}

			if (patch.x + patch.w > clip.x + clip.w)
				patch.w = (clip.x + clip.w) - patch.x;

			if (patch.y + patch.h > clip.y + clip.h)
				patch.h = (clip.y + clip.h) - patch.y;
		}

		//

		src.x += patchOfs.x * complexTransform[0][0] + patchOfs.y * complexTransform[1][0];
		src.y += patchOfs.x * complexTransform[0][1] + patchOfs.y * complexTransform[1][1];

		// See if we can skip the expensive source clipping....

		bool bClipSource = false;
		if (m_bClipSource)
		{
			BinalSize clipMax = (static_cast<BinalSize>(m_pBlitSource->m_size))*BINAL_MUL;
			
			if (m_pBlitSource->m_sampleMethod == SampleMethod::Bilinear)
				clipMax -= BinalSize( BINAL_MUL, BINAL_MUL);

			
			
			BinalCoord src2 = src;		// Source pixel to read for top-right destination corner.
			BinalCoord src3 = src;		// Source pixel to read for bottom-right destination corner.
			BinalCoord src4 = src;		// Source pixel to read for bottom-left destination corner.

			src2.x += (patch.w - 1) * complexTransform[0][0];
			src2.y += (patch.w - 1) * complexTransform[0][1];

			src3.x += (patch.w - 1) * complexTransform[0][0] + (patch.h - 1) * complexTransform[1][0];
			src3.y += (patch.w - 1) * complexTransform[0][1] + (patch.h - 1) * complexTransform[1][1];

			src4.x += (patch.h - 1) * complexTransform[1][0];
			src4.y += (patch.h - 1) * complexTransform[1][1];

			if (src.x < 0 || src.y < 0 || src.x >= clipMax.w || src.y >= clipMax.h ||
				src2.x < 0 || src2.y < 0 || src2.x > clipMax.w || src2.y > clipMax.h ||
				src3.x < 0 || src3.y < 0 || src3.x > clipMax.w || src3.y > clipMax.h ||
				src4.x < 0 || src4.y < 0 || src4.x > clipMax.w || src4.y > clipMax.h )
			{
				bClipSource = true;
			}
		}

		//

		uint8_t * pDst = m_pClipSegments[i].pBuffer + (patch.y-m_pClipSegments[i].rect.y) * m_pClipSegments[i].pitch + (patch.x - m_pClipSegments[i].rect.x) * m_canvasPixelBytes;
		
		if( type == OpType::Tile )
			(this->*m_pLinearTransformTileOp)(pDst, m_pClipSegments[i].pitch, patch.w, patch.h, src, complexTransform, patch.pos(), m_pTransformTileFirstPassOp);
		else if( bClipSource )
			(this->*m_pLinearTransformClipBlitOp)(pDst, m_pClipSegments[i].pitch, patch.w, patch.h, src, complexTransform, patch.pos(), m_pTransformClipBlitFirstPassOp);
		else
			(this->*m_pLinearTransformBlitOp)(pDst, m_pClipSegments[i].pitch, patch.w, patch.h, src, complexTransform,patch.pos(), m_pTransformBlitFirstPassOp);
	}
}

//____ _onePassLinearStraightBlit() _____________________________________________

void LinearGfxDevice::_onePassLinearStraightBlit(uint8_t * pDst, int destPitch, int width, int height, CoordI src, const int simpleTransform[2][2], CoordI patchPos, StraightBlitOp_p pPassOneOp)
{
	const SoftSurface * pSource = m_pBlitSource;

	int srcPixelBytes = pSource->m_pPixelDescription->bits / 8;
	int dstPixelBytes = m_canvasPixelBytes;

	Pitches pitches;

	pitches.srcX = srcPixelBytes * simpleTransform[0][0] + pSource->m_pitch * simpleTransform[0][1];
	pitches.dstX = dstPixelBytes;
	pitches.srcY = srcPixelBytes * simpleTransform[1][0] + pSource->m_pitch * simpleTransform[1][1] - pitches.srcX*width;
	pitches.dstY = destPitch - width * dstPixelBytes;

	uint8_t * pSrc = pSource->m_pData + src.y * pSource->m_pitch + src.x * srcPixelBytes;

	pPassOneOp(pSrc, pDst, pSource, pitches, height, width, m_colTrans, patchPos, simpleTransform);
}

//____ _twoPassLinearStraightBlit() _____________________________________________

void LinearGfxDevice::_twoPassLinearStraightBlit(uint8_t * pDst, int destPitch, int width, int height, CoordI src, const int simpleTransform[2][2], CoordI patchPos, StraightBlitOp_p pPassOneOp)
{
	SoftSurface * pSource = m_pBlitSource;

	int srcPixelBytes = pSource->m_pPixelDescription->bits / 8;
	int dstPixelBytes = m_canvasPixelBytes;

	Pitches pitchesPass1, pitchesPass2;

	pitchesPass1.srcX = srcPixelBytes * simpleTransform[0][0] + pSource->m_pitch * simpleTransform[0][1];
	pitchesPass1.dstX = 8;
	pitchesPass1.srcY = srcPixelBytes * simpleTransform[1][0] + pSource->m_pitch * simpleTransform[1][1] - pitchesPass1.srcX*width;
	pitchesPass1.dstY = 0;

	pitchesPass2.srcX = 8;
	pitchesPass2.dstX = dstPixelBytes;
	pitchesPass2.srcY = 0;
	pitchesPass2.dstY = destPitch - width * dstPixelBytes;

	int chunkLines;

	if (width>= 2048)
		chunkLines = 1;
	else if (width*height <= 2048)
		chunkLines = height;
	else
		chunkLines = 2048 / width;

	int memBufferSize = chunkLines * width*8;

	uint8_t * pChunkBuffer = (uint8_t*) GfxBase::memStackAlloc(memBufferSize);

	int line = 0;

	while (line < height)
	{
		int thisChunkLines = min(height - line, chunkLines);

		uint8_t * pSrc = pSource->m_pData + src.y * pSource->m_pitch + line*(srcPixelBytes * simpleTransform[1][0] + pSource->m_pitch * simpleTransform[1][1]) + src.x * srcPixelBytes;
//			uint8_t * pSrc = pSource->m_pData + (src.y+line) * pSource->m_pitch + src.x * srcPixelBytes;

		pPassOneOp(pSrc, pChunkBuffer, pSource, pitchesPass1, thisChunkLines, width, m_colTrans, { 0,0 }, simpleTransform);
		m_pBlitSecondPassOp(pChunkBuffer, pDst, pSource, pitchesPass2, thisChunkLines, width, m_colTrans, patchPos, nullptr);

		patchPos.y += thisChunkLines;
		line += thisChunkLines;
		
		pDst += destPitch*thisChunkLines;
	}

	GfxBase::memStackFree(memBufferSize);
}

//____ _dummyLinearStraightBlit() _____________________________________________

void LinearGfxDevice::_dummyLinearStraightBlit(uint8_t * pDst, int destPitch, int width, int height, CoordI pos, const int simpleTransform[2][2], CoordI patchPos, StraightBlitOp_p pPassOneOp)
{
}


//____ _onePassLinearTransformBlit() ____________________________________________

void LinearGfxDevice::_onePassLinearTransformBlit(uint8_t * pDst, int destPitch, int destWidth, int destHeight, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp)
{
	pPassOneOp(m_pBlitSource, pos, transformMatrix, pDst, m_canvasPixelBytes, destPitch - m_canvasPixelBytes * destWidth, destHeight, destWidth, m_colTrans, patchPos);
}

//____ _twoPassLinearTransformBlit() ____________________________________________

void LinearGfxDevice::_twoPassLinearTransformBlit(	uint8_t * pDst, int destPitch, int destWidth, int destHeight, BinalCoord pos, const binalInt transformMatrix[2][2],
											CoordI patchPos, TransformBlitOp_p pPassOneOp)
{
	const SoftSurface * pSource = m_pBlitSource;

	int dstPixelBytes = m_canvasPixelBytes;

	Pitches pitchesPass2;

	pitchesPass2.srcX = 8;
	pitchesPass2.dstX = dstPixelBytes;
	pitchesPass2.srcY = 0;
	pitchesPass2.dstY = destPitch - dstPixelBytes * destWidth;

	int chunkLines;

	if (destWidth >= 2048)
		chunkLines = 1;
	else if (destWidth*destHeight <= 2048)
		chunkLines = destHeight;
	else
		chunkLines = 2048 / destHeight;

	int memBufferSize = chunkLines * destWidth * 8;

	uint8_t * pChunkBuffer = (uint8_t*)GfxBase::memStackAlloc(memBufferSize);

	int line = 0;

	while (line < destHeight)
	{
		int thisChunkLines = min(destHeight - line, chunkLines);

//		uint8_t * pDst = m_pCanvasPixels + (dest.y + line) * m_canvasPitch + dest.x * dstPixelBytes;

		pPassOneOp(pSource, pos, transformMatrix, pChunkBuffer, 8, 0, thisChunkLines, destWidth, m_colTrans, { 0,0 });
		m_pBlitSecondPassOp(pChunkBuffer, pDst, pSource, pitchesPass2, thisChunkLines, destWidth, m_colTrans, patchPos, nullptr);

		pos.x += transformMatrix[1][0] * thisChunkLines;
		pos.y += transformMatrix[1][1] * thisChunkLines;

		patchPos.y += thisChunkLines;
		line += thisChunkLines;
		
		pDst += destPitch*thisChunkLines;
	}

	GfxBase::memStackFree(memBufferSize);
}


//____ _dummyLinearTransformBlit() _____________________________________________

void LinearGfxDevice::_dummyLinearTransformBlit(uint8_t * pDst, int destPitch, int width, int height, BinalCoord pos, const binalInt matrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp)
{
}


//____ _transformDrawSegments() _________________________________________

void LinearGfxDevice::_transformDrawSegments(const RectSPX& _destIn, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * _pEdgeStrips, int edgeStripPitch, TintMode tintMode, const int _simpleTransform[2][2])
{
	if( m_canvas.pSurface )
	{
		SoftGfxDevice::_transformDrawSegments(_destIn, nSegments, pSegmentColors, nEdgeStrips, _pEdgeStrips, edgeStripPitch, tintMode, _simpleTransform);
		return;
	}
	
	//TODO: Proper 26:6 support
	RectI _dest = roundToPixels(_destIn);

	RectI dest = _dest;

	SegmentEdge edges[c_maxSegments - 1];


	// We need to modify our transform since we are moving the destination pointer, not the source pointer, according to the transform.

	int simpleTransform[2][2];

	simpleTransform[0][0] = _simpleTransform[0][0];
	simpleTransform[1][1] = _simpleTransform[1][1];

	if ((_simpleTransform[0][0] | _simpleTransform[1][1]) == 0)
	{
		simpleTransform[0][1] = _simpleTransform[1][0];
		simpleTransform[1][0] = _simpleTransform[0][1];
	}
	else
	{
		simpleTransform[0][1] = _simpleTransform[0][1];
		simpleTransform[1][0] = _simpleTransform[1][0];
	}

	// Calculate start coordinate

	CoordI start = dest.pos();

	if (simpleTransform[0][0] + simpleTransform[1][0] < 0)
		start.x += dest.w - 1;

	if (simpleTransform[0][1] + simpleTransform[1][1] < 0)
		start.y += dest.h - 1;

	// Detect if strip columns are lined horizontally or verically

	bool bHorizontalColumns = (simpleTransform[0][0] != 0);

	// Limit size of destination rect by number of edgestrips.

	if (bHorizontalColumns)
	{
		if (dest.w > nEdgeStrips - 1)
		{
			if (simpleTransform[0][0] < 0)
				dest.x += dest.w - nEdgeStrips - 1;

			dest.w = nEdgeStrips - 1;
		}
	}
	else
	{
		if (dest.h > nEdgeStrips - 1)
		{
			if (simpleTransform[0][1] < 0)
				dest.y += dest.h - nEdgeStrips - 1;

			dest.h = nEdgeStrips - 1;
		}
	}

	// Apply tinting

	int16_t	colors[c_maxSegments*4][4];				// RGBA order of elements
	bool	transparentSegments[c_maxSegments];
	bool	opaqueSegments[c_maxSegments];

	SegmentGradient* pGradientsY = nullptr;
	SegmentGradient* pGradientsX = nullptr;
	int		gradientsYBufferSize = 0;
	int		gradientsXBufferSize = 0;

	// Determine combined tint-mode

	bool	bTintFlat = false;			// Set if we tint in any way, flat, x, y, or xy.
	bool	bTintX = false;
	bool	bTintY = false;

	if (tintMode != TintMode::None)
	{
		bTintFlat = !m_tintColor.isOpaqueWhite();

		if (tintMode == TintMode::GradientXY || m_colTrans.mode == TintMode::GradientXY)
		{
			bTintX = bTintY = true;
		}
		else
		{
			if (tintMode == TintMode::GradientY || m_colTrans.mode == TintMode::GradientY)
				bTintY = true;
			if (tintMode == TintMode::GradientX || m_colTrans.mode == TintMode::GradientX)
				bTintX = true;
		}
	}

	// Unpack input colors and fill in transparentSegments

	if (!bTintX && !bTintY)
	{
		// If we just use flat tinting (or no tint at all), we tint our segment colors right away

		for (int i = 0; i < nSegments; i++)
		{
			colors[i][0] = (pSegmentColors[i].r * m_colTrans.flatTintColor.r) >> 12;
			colors[i][1] = (pSegmentColors[i].g * m_colTrans.flatTintColor.g) >> 12;
			colors[i][2] = (pSegmentColors[i].b * m_colTrans.flatTintColor.b) >> 12;
			colors[i][3] = (pSegmentColors[i].a * m_colTrans.flatTintColor.a) >> 12;

			transparentSegments[i] = (colors[i][3] == 0);
			opaqueSegments[i] = (colors[i][3] == 4096);
		}
	}
	else
	{
		int colorsPerSegment = bTintX && bTintY ? 4 : 2;

		for (int i = 0; i < nSegments*colorsPerSegment; i++)
		{
			colors[i][0] = pSegmentColors[i].r;
			colors[i][1] = pSegmentColors[i].g;
			colors[i][2] = pSegmentColors[i].b;
			colors[i][3] = pSegmentColors[i].a;
		}
	}


	// If we instead have gradients we have things to take care of...

	if (bTintX || bTintY)
	{
		// Generate the buffers that we will need

		if (bTintY)
		{
			gradientsYBufferSize = sizeof(SegmentGradient) * nSegments;
			pGradientsY = (SegmentGradient*)GfxBase::memStackAlloc(gradientsYBufferSize);
		}

		if (bTintX)
		{
			// We use two gradients per segment in X, one for top and bottom of rectangle each.

			gradientsXBufferSize = sizeof(SegmentGradient) * nSegments * 2;
			pGradientsX = (SegmentGradient*)GfxBase::memStackAlloc(gradientsXBufferSize);
		}


		// Calculate RGBA values from m_colTrans for our four corners.

		int		baseB[4], baseG[4], baseR[4], baseA[4];
		int		tempB[4], tempG[4], tempR[4], tempA[4];

		_colTransRect(tempB, tempG, tempR, tempA, _dest);

		// Rotate the colors of our four corners if we are flipped

		static const int cornerSwitchMap[2][2][2][4] =
						{ {{{0,3,2,1},				// [ 0,-1,-1, 0] = Rot90FlipY
							{1,2,3,0}},				// [ 0,-1, 1, 0] = Rot90
							{{3,0,1,2},				// [ 0, 1,-1, 0] = Rot270
							{2,1,0,3}}},			// [ 0, 1, 1, 0] = Rot90FlipX
							{{{2,3,0,1},			// [-1, 0, 0,-1] = Rot180
							{1,0,3,2}},				// [-1, 0, 0, 1] = FlipX
							{{3,2,1,0},				// [ 1, 0, 0,-1] = FlipY
							{0,1,2,3}}} };			// [ 1, 0, 0, 1] = Normal

		int i1, i2, i3;
		if (_simpleTransform[0][0] == 0)
		{
			i1 = 0;
			i2 = _simpleTransform[0][1] == 1 ? 1 : 0;
			i3 = _simpleTransform[1][0] == 1 ? 1 : 0;
		}
		else
		{
			i1 = 1;
			i2 = _simpleTransform[0][0] == 1 ? 1 : 0;
			i3 = _simpleTransform[1][1] == 1 ? 1 : 0;
		}

		const int* pSwitch = cornerSwitchMap[i1][i2][i3];

		for (int i = 0; i < 4; i++)
		{
			int n = pSwitch[i];
			baseB[i] = tempB[n];
			baseG[i] = tempG[n];
			baseR[i] = tempR[n];
			baseA[i] = tempA[n];
		}

		// Lets process each segment

		for (int seg = 0; seg < nSegments; seg++)
		{

			int		segB[4], segG[4], segR[4], segA[4];

			// Calculate RGBA values for our four corners

			if (tintMode == TintMode::Flat)
			{
				for (int i = 0; i < 4; i++)
				{
					segR[i] = baseR[i] * colors[seg][0];
					segG[i] = baseG[i] * colors[seg][1];
					segB[i] = baseB[i] * colors[seg][2];
					segA[i] = baseA[i] * colors[seg][3];
				}
			}
			else
			{
				if (tintMode == TintMode::GradientX)
				{
					segR[0] = baseR[0] * colors[seg*2][0];
					segG[0] = baseG[0] * colors[seg * 2][1];
					segB[0] = baseB[0] * colors[seg * 2][2];
					segA[0] = baseA[0] * colors[seg * 2][3];

					segR[1] = baseR[1] * colors[seg * 2+1][0];
					segG[1] = baseG[1] * colors[seg * 2+1][1];
					segB[1] = baseB[1] * colors[seg * 2+1][2];
					segA[1] = baseA[1] * colors[seg * 2+1][3];

					segR[2] = baseR[2] * colors[seg * 2 + 1][0];
					segG[2] = baseG[2] * colors[seg * 2 + 1][1];
					segB[2] = baseB[2] * colors[seg * 2 + 1][2];
					segA[2] = baseA[2] * colors[seg * 2 + 1][3];

					segR[3] = baseR[3] * colors[seg * 2][0];
					segG[3] = baseG[3] * colors[seg * 2][1];
					segB[3] = baseB[3] * colors[seg * 2][2];
					segA[3] = baseA[3] * colors[seg * 2][3];
				}
				else if (tintMode == TintMode::GradientY)
				{
					segR[0] = baseR[0] * colors[seg * 2][0];
					segG[0] = baseG[0] * colors[seg * 2][1];
					segB[0] = baseB[0] * colors[seg * 2][2];
					segA[0] = baseA[0] * colors[seg * 2][3];

					segR[1] = baseR[1] * colors[seg * 2][0];
					segG[1] = baseG[1] * colors[seg * 2][1];
					segB[1] = baseB[1] * colors[seg * 2][2];
					segA[1] = baseA[1] * colors[seg * 2][3];

					segR[2] = baseR[2] * colors[seg * 2 + 1][0];
					segG[2] = baseG[2] * colors[seg * 2 + 1][1];
					segB[2] = baseB[2] * colors[seg * 2 + 1][2];
					segA[2] = baseA[2] * colors[seg * 2 + 1][3];

					segR[3] = baseR[3] * colors[seg * 2 + 1][0];
					segG[3] = baseG[3] * colors[seg * 2 + 1][1];
					segB[3] = baseB[3] * colors[seg * 2 + 1][2];
					segA[3] = baseA[3] * colors[seg * 2 + 1][3];
				}
				else
				{
					segR[0] = baseR[0] * colors[seg * 4][0];
					segG[0] = baseG[0] * colors[seg * 4][1];
					segB[0] = baseB[0] * colors[seg * 4][2];
					segA[0] = baseA[0] * colors[seg * 4][3];

					segR[1] = baseR[1] * colors[seg * 4 + 1][0];
					segG[1] = baseG[1] * colors[seg * 4 + 1][1];
					segB[1] = baseB[1] * colors[seg * 4 + 1][2];
					segA[1] = baseA[1] * colors[seg * 4 + 1][3];

					segR[2] = baseR[2] * colors[seg * 4 + 2][0];
					segG[2] = baseG[2] * colors[seg * 4 + 2][1];
					segB[2] = baseB[2] * colors[seg * 4 + 2][2];
					segA[2] = baseA[2] * colors[seg * 4 + 2][3];

					segR[3] = baseR[3] * colors[seg * 4 + 3][0];
					segG[3] = baseG[3] * colors[seg * 4 + 3][1];
					segB[3] = baseB[3] * colors[seg * 4 + 3][2];
					segA[3] = baseA[3] * colors[seg * 4 + 3][3];
				}
			}

			// We now have the segments corner colors. Let's save
			// that information in the correct format for future
			// processing depending on tint mode.

			// Filling in the x-gradient if we have one. Two SegmentGradient structs for
			// each segment, one for gradient along top and of dest rectangle and one along the bottom.

			if (bTintX)
			{
				// Fill in top gradient

				auto p = &pGradientsX[seg * 2];

				p->begB = segB[0];
				p->begG = segG[0];
				p->begR = segR[0];
				p->begA = segA[0];

				p->incB = (segB[1] - segB[0]) / dest.w;
				p->incG = (segG[1] - segG[0]) / dest.w;
				p->incR = (segR[1] - segR[0]) / dest.w;
				p->incA = (segA[1] - segA[0]) / dest.w;

				// Fill in bottom gradient

				p++;

				p->begB = segB[3];
				p->begG = segG[3];
				p->begR = segR[3];
				p->begA = segA[3];

				p->incB = (segB[2] - segB[3]) / dest.w;
				p->incG = (segG[2] - segG[3]) / dest.w;
				p->incR = (segR[2] - segR[3]) / dest.w;
				p->incA = (segA[2] - segA[3]) / dest.w;
			}

			// If we don't have any x-gradient we can fill in y-gradient once and for all

			if (bTintY && !bTintX)
			{
				auto p = &pGradientsY[seg];

				p->begB = segB[0];
				p->begG = segG[0];
				p->begR = segR[0];
				p->begA = segA[0];

				p->incB = (segB[3] - segB[0]) / dest.h;
				p->incG = (segG[3] - segG[0]) / dest.h;
				p->incR = (segR[3] - segR[0]) / dest.h;
				p->incA = (segA[3] - segA[0]) / dest.h;
			}

			// Possibly mark this segment as transparent

			transparentSegments[seg] = (segA[0] + segA[1] + segA[2] + segA[3] == 0);
			opaqueSegments[seg] = (segA[0] + segA[1] + segA[2] + segA[3] == 4096*4096*4);
		}
	}

	// Modify opaqueSegments if our state isn't blend
	
	if( m_blendMode != BlendMode::Blend && m_blendMode != BlendMode::BlendFixedColor )
	{
		bool val = (m_blendMode == BlendMode::Replace);
		
		for (int seg = 0; seg < nSegments; seg++)
			opaqueSegments[seg] = val;
	}
	
	// Modify transparentSegments if our state is BlendFixedColor
	
	if( m_blendMode == BlendMode::BlendFixedColor )
	{
		for (int seg = 0; seg < nSegments; seg++)
			transparentSegments[seg] = false;
	}
	
	// Set start position and clip dest

	if (!dest.isOverlapping(m_clipBounds/64))
		return;

	SegmentOp_p	pOp = nullptr;
	auto pKernels = m_pKernels[(int) m_canvasPixelFormat];
	if (pKernels)
		pOp = pKernels->pSegmentKernels[(int)bTintY][(int)m_blendMode];

	if (pOp == nullptr )
	{
		if( m_blendMode == BlendMode::Ignore )
			return;
		
		char errorMsg[1024];
		
		snprintf(errorMsg, 1024, "Failed draw segments operation. SoftGfxDevice is missing segments kernel %s Y-tint for BlendMode::%s onto surface of PixelFormat:%s.",
			bTintY ? "with" : "without",
			toString(m_blendMode),
			toString(m_canvasPixelFormat ));
		
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	// Loop through patches

	for (int patchIdx = 0; patchIdx < m_nClipSegments; patchIdx++)
	{
		auto& seg = m_pClipSegments[patchIdx];
			
		int xPitch = m_canvasPixelBytes;
		int yPitch = seg.pitch;

		// Calculate pitches

		int colPitch = simpleTransform[0][0] * xPitch + simpleTransform[0][1] * yPitch;
		int rowPitch = simpleTransform[1][0] * xPitch + simpleTransform[1][1] * yPitch;

		
		uint8_t* pOrigo = seg.pBuffer - seg.rect.y * yPitch - seg.rect.x * xPitch + start.y * yPitch + start.x * xPitch;

		
		// Clip patch

		RectI patch = RectI::overlap(dest, seg.rect);
		if (patch.w == 0 || patch.h == 0)
			continue;

		// Calculate stripstart, clipBeg/clipEnd and first edge for patch

		int columnOfs;
		int rowOfs;

		int columns;
		int rows;

		if (bHorizontalColumns)
		{
			columnOfs = colPitch > 0 ? patch.x - dest.x : dest.right() - patch.right();
			rowOfs = rowPitch > 0 ? patch.y - dest.y : dest.bottom() - patch.bottom();

			columns = patch.w;
			rows = patch.h;
		}
		else
		{
			columnOfs = colPitch > 0 ? patch.y - dest.y : dest.bottom() - patch.bottom();
			rowOfs = rowPitch > 0 ? patch.x - dest.x : dest.right() - patch.right();

			columns = patch.h;
			rows = patch.w;
		}

		const int* pEdgeStrips = _pEdgeStrips + columnOfs * edgeStripPitch;
		uint8_t* pStripStart = pOrigo + columnOfs * colPitch;

		int clipBeg = rowOfs * 256;
		int clipEnd = clipBeg + (rows * 256);

		for (int x = 0; x < columns; x++)
		{
			int nEdges = 0;
			int skippedSegments = 0;

			for (int y = 0; y < nSegments - 1; y++)
			{
				int beg = pEdgeStrips[y] * 4;
				int end = pEdgeStrips[y + edgeStripPitch] * 4;

				if (beg > end)
					swap(beg, end);

				if (beg >= clipEnd)
					break;

				if (end > clipBeg)
				{
					int coverageInc = (end == beg) ? 0 : (65536 * 256) / (end - beg);
					int coverage = 0;

					if (beg < clipBeg)
					{
						int cut = clipBeg - beg;
						beg = clipBeg;
						coverage += (coverageInc * cut) >> 8;
					}

					if (end > clipEnd)
						end = clipEnd;


					edges[nEdges].begin = beg;
					edges[nEdges].end = end;
					edges[nEdges].coverage = coverage;
					edges[nEdges].coverageInc = coverageInc;
					nEdges++;
				}
				else
					skippedSegments++;
			}

			// Update tinting if we have X-gradient

			const int16_t * pColors = &colors[skippedSegments][0];

			if (bTintX)
			{
				if (bTintY)
				{
					auto pOut = pGradientsY + skippedSegments;
					auto pIn = pGradientsX + skippedSegments * 2;

					for (int i = 0; i < nEdges + 1; i++)
					{
						int beg = pIn[0].begB + pIn[0].incB * columnOfs;
						int inc = (pIn[1].begB + pIn[1].incB * columnOfs - beg) / dest.h;
						pOut[i].begB = beg;
						pOut[i].incB = inc;

						beg = pIn[0].begG + pIn[0].incG * columnOfs;
						inc = (pIn[1].begG + pIn[1].incG * columnOfs - beg) / dest.h;
						pOut[i].begG = beg;
						pOut[i].incG = inc;

						beg = pIn[0].begR + pIn[0].incR * columnOfs;
						inc = (pIn[1].begR + pIn[1].incR * columnOfs - beg) / dest.h;
						pOut[i].begR = beg;
						pOut[i].incR = inc;

						beg = pIn[0].begA + pIn[0].incA * columnOfs;
						inc = (pIn[1].begA + pIn[1].incA * columnOfs - beg) / dest.h;
						pOut[i].begA = beg;
						pOut[i].incA = inc;

						pIn += 2;
					}
				}
				else
				{
					// We just use the color array and fill in correct color values for this column.

					auto pGrad = pGradientsX + skippedSegments * 2;

					for (int i = 0; i < nEdges + 1; i++)
					{
						colors[i][0] = (pGrad->begR + pGrad->incR * columnOfs) >> 12;
						colors[i][1] = (pGrad->begG + pGrad->incG * columnOfs) >> 12;
						colors[i][2] = (pGrad->begB + pGrad->incB * columnOfs) >> 12;
						colors[i][3] = (pGrad->begA + pGrad->incA * columnOfs) >> 12;
						pGrad += 2;												// Skipping bottom gradient data since we don't need it.
					}

					pColors = &colors[0][0];
				}
			}

			//
			auto gradientPointer = pGradientsY != nullptr ? pGradientsY + skippedSegments : nullptr;
			pOp(clipBeg, clipEnd, pStripStart, rowPitch, nEdges, edges, pColors, gradientPointer, transparentSegments + skippedSegments, opaqueSegments + skippedSegments, m_colTrans);
			pEdgeStrips += edgeStripPitch;
			pStripStart += colPitch;
			columnOfs++;
		}
	}

	// Free what we have reserved on the memStack.

	if (gradientsXBufferSize > 0)
		GfxBase::memStackFree(gradientsXBufferSize);

	if (gradientsYBufferSize > 0)
		GfxBase::memStackFree(gradientsYBufferSize);

}





} //namespace wg
