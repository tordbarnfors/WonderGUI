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

#include <wg_linearbackend.h>
#include <wg_softedgemap.h>
#include <wg_gfxbase.h>


using namespace std;

namespace wg
{
	const TypeInfo LinearBackend::TYPEINFO = { "LinearBackend", &GfxBackend::TYPEINFO };


	//____ create() _____________________________________________

	LinearBackend_p LinearBackend::create(std::function<void*(CanvasRef ref, int nBytes)> beginCanvasRender,
										std::function<void(CanvasRef ref, int nSegments, const Segment * pSegments)> endCanvasRender)
	{
		return LinearBackend_p(new LinearBackend(beginCanvasRender, endCanvasRender));
	}

	//____ constructor _____________________________________________

	LinearBackend::LinearBackend(std::function<void*(CanvasRef ref, int nBytes)> beginCanvasRender,
								 std::function<void(CanvasRef ref, int nSegments, const Segment * pSegments)> endCanvasRender)
	{
		m_beginCanvasRenderCallback = beginCanvasRender;
		m_endCanvasRenderCallback 	= endCanvasRender;
	}

	//____ destructor _____________________________________________

	LinearBackend::~LinearBackend()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& LinearBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ defineCanvas() _________________________________________________________

	bool LinearBackend::defineCanvas( CanvasRef ref, const SizeSPX size, PixelFormat pixelFormat, int scale )
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

	const CanvasInfo * LinearBackend::canvasInfo(CanvasRef ref) const
	{
		if( m_canvasDefinitions[int(ref)].ref == ref )
			return &m_canvasDefinitions[int(ref)];

		return nullptr;
	}

	//____ setSegmentPadding() ____________________________________________________

	void LinearBackend::setSegmentPadding( int bytes )
	{
		m_segmentPadding = bytes;
	}

	//____ beginSession() _______________________________________________________

	void LinearBackend::beginSession( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo )
	{
		SoftBackend::beginSession(canvasRef, pCanvas, nUpdateRects, pUpdateRects, pInfo);

		// We prepare our canvas segments as much as we can, i.e. we resize the vector
		// and fill in the rect. Pitch and pBuffer depends on canvas.

		m_canvasSegments.resize(nUpdateRects);

		int	nUpdatePixels = 0;

		for( int i = 0 ; i < nUpdateRects ; i++ )
		{
			RectI updateRect = pUpdateRects[i]/64;
			m_canvasSegments[i].rect = updateRect;

			nUpdatePixels += updateRect.w * updateRect.h;
		}

		m_nUpdatePixels = nUpdatePixels;
	}

	//____ endSession() _______________________________________________________

	void LinearBackend::endSession()
	{
		if( m_activeCanvas != CanvasRef::None )
		{
			m_endCanvasRenderCallback( m_activeCanvas, (int) m_canvasSegments.size(), m_canvasSegments.data() );
			m_activeCanvas = CanvasRef::None;
		}

		SoftBackend::endSession();
	}

	//____ setCanvas() ___________________________________________________________

	void LinearBackend::setCanvas( Surface * pSurface )
	{
		SoftBackend::setCanvas(pSurface);

		if( m_activeCanvas != CanvasRef::None )
		{
			m_endCanvasRenderCallback( m_activeCanvas, (int) m_canvasSegments.size(), m_canvasSegments.data() );
			m_activeCanvas = CanvasRef::None;
		}
	}

	void LinearBackend::setCanvas( CanvasRef ref )
	{
		auto pInfo = canvasInfo(ref);

		if (!pInfo)
		{
			//TODO: Error handling!

			return;
		}

		m_pCanvas = nullptr;

		m_pCanvasPixels		= nullptr;
		m_canvasPixelFormat = pInfo->format;
		m_canvasPitch		= 0;
		m_canvasPixelBytes	= Util::pixelFormatToDescription(pInfo->format).bits/8;

		_resetStates();

		if( m_activeCanvas != CanvasRef::None )
			m_endCanvasRenderCallback( m_activeCanvas, (int) m_canvasSegments.size(), m_canvasSegments.data() );

		m_activeCanvas = ref;

		int bytesNeeded = m_nUpdatePixels * m_canvasPixelBytes;
		bytesNeeded += int(m_canvasSegments.size())*m_segmentPadding;

		uint8_t * pCanvasBuffer = (uint8_t*) m_beginCanvasRenderCallback( ref, bytesNeeded );

		int ofs = 0;
		for( int i = 0 ; i < m_canvasSegments.size() ; i++ )
		{
			auto& updateRect = m_canvasSegments[i].rect;
			m_canvasSegments[i].pitch = updateRect.w * m_canvasPixelBytes;
			m_canvasSegments[i].pBuffer = pCanvasBuffer + ofs;
			ofs += updateRect.w * updateRect.h * m_canvasPixelBytes + m_segmentPadding;
		}
	}

	//____ processCommands() _____________________________________________

	void LinearBackend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd)
	{
		if( m_pCanvas )
		{
			SoftBackend::processCommands(pBeg,pEnd);
			return;
		}

		const RectSPX *	pRects = m_pRectsPtr;
		const HiColor*	pColors = m_pColorsPtr;
		Object* const *	pObjects = m_pObjectsPtr;

		Segment *	pSegBeg = m_canvasSegments.data();
		Segment *	pSegEnd = m_canvasSegments.data() + m_canvasSegments.size();
		Segment *	pSegment = pSegBeg;

		auto p = pBeg;
		while (p < pEnd)
		{
			auto cmd = Command(*p++);
			switch (cmd)
			{
			case Command::None:
				break;

			case Command::StateChange:
			{
				int32_t statesChanged = *p++;

				if (statesChanged & uint8_t(StateChange::BlitSource))
				{
					auto pBlitSource = static_cast<SoftSurface*>(* pObjects++);

					if (!pBlitSource || !m_pBlitSource || pBlitSource->pixelFormat() != m_pBlitSource->pixelFormat() ||
						pBlitSource->sampleMethod() != m_pBlitSource->sampleMethod())
						m_bBlitFunctionNeedsUpdate = true;

					m_pBlitSource = pBlitSource;
				}

				if (statesChanged & uint8_t(StateChange::TintColor))
				{
					m_colTrans.flatTintColor = *pColors++;
					m_colTrans.pTintAxisX = nullptr;
					m_colTrans.pTintAxisY = nullptr;

					m_colTrans.bTintOpaque = (m_colTrans.flatTintColor.a == 4096);
					_updateTintMode();
				}

				if (statesChanged & uint8_t(StateChange::TintMap))
				{
					auto p32 = (const spx *) p;
					int32_t	x = *p32++ / 64;
					int32_t	y = *p32++ / 64;
					int32_t	w = *p32++ / 64;
					int32_t	h = *p32++ / 64;

					m_colTrans.tintRect = RectI(x, y, w, h);

					int32_t nHorrColors = *p32++;
					int32_t nVertColors = *p32++;
					p = (const uint16_t*) p32;

					auto pOurColors = pColors;
					
					if( nHorrColors > 0 )
					{
						m_colTrans.pTintAxisX = pColors;
						pColors += nHorrColors;
					}
					else
						m_colTrans.pTintAxisX = nullptr;

					if( nVertColors > 0 )
					{
						m_colTrans.pTintAxisY = pColors;
						pColors += nVertColors;
					}
					else
						m_colTrans.pTintAxisY = nullptr;

					// Check if whole tint is opaque.
					
					int totalAlpha = 0;
					while( pOurColors < pColors )
					{
						totalAlpha += pOurColors->a;
						pOurColors++;
					}
					
					m_colTrans.bTintOpaque = (totalAlpha == 4096 * (nHorrColors + nVertColors));

					_updateTintMode();
				}

				if (statesChanged & uint8_t(StateChange::BlendMode))
				{
					m_blendMode = (BlendMode)*p++;

					m_bBlitFunctionNeedsUpdate = true;
				}

				if (statesChanged & uint8_t(StateChange::MorphFactor))
				{
					m_colTrans.morphFactor = *p++;
				}

				if (statesChanged & uint8_t(StateChange::FixedBlendColor))
				{
					m_colTrans.fixedBlendColor = *pColors++;
				}

				if (statesChanged & uint8_t(StateChange::Blur))
				{
					spx		radius = *p++;

					const uint16_t* pRed = p;
					const uint16_t* pGreen = p + 9;
					const uint16_t* pBlue = p + 18;
					p += 27;

					_updateBlurRadius(radius);

					for (int i = 0; i < 9; i++)
					{
						m_colTrans.blurMtxR[i] = int(pRed[i]) * 32768;
						m_colTrans.blurMtxG[i] = int(pGreen[i]) * 32768;
						m_colTrans.blurMtxB[i] = int(pBlue[i]) * 32768;
					}
				}

				// Take care of alignment

				if( (uintptr_t(p) & 0x2) == 2 )
					p++;
				
				break;
			}

			case Command::Fill:
			{
				int32_t nRects = *p++;

				const HiColor&  col = * pColors++;

				FillOp_p pFunc = nullptr;

				// Optimize calls

				BlendMode blendMode = m_blendMode;
				if (blendMode == BlendMode::Blend && col.a == 4096 && (m_colTrans.mode == TintMode::None || (m_colTrans.mode == TintMode::Flat && m_colTrans.flatTintColor.a == 4096)) )
				{
					blendMode = BlendMode::Replace;
				}

				auto pKernels = m_pKernels[(int)m_canvasPixelFormat];
				if (pKernels)
					pFunc = pKernels->pFillKernels[(int)m_colTrans.mode][(int)blendMode];

				if (!pFunc)
				{
					pRects += nRects;

					if (blendMode == BlendMode::Ignore)
						break;

					char errorMsg[1024];

					snprintf(errorMsg, 1024, "Failed fill operation. SoftGfxDevice is missing fill kernel for TintMode::%s, BlendMode::%s onto surface of PixelFormat:%s.",
						toString(m_colTrans.mode),
						toString(blendMode),
						toString(m_canvasPixelFormat));

					GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
					break;
				}

				for (int i = 0; i < nRects; i++)
				{
					RectI spxPatch = *pRects++;

					RectI pixelPatch = spxPatch / 64;

					while( pixelPatch.x < pSegment->rect.x || pixelPatch.x >= pSegment->rect.x + pSegment->rect.w ||
						  pixelPatch.y < pSegment->rect.y || pixelPatch.y >= pSegment->rect.y + pSegment->rect.h )
					{
						pSegment++;
						if( pSegment == pSegEnd )
							pSegment = pSegBeg;
					}

					Segment& seg = * pSegment;


					if (((spxPatch.x | spxPatch.y | spxPatch.w | spxPatch.h) & 63) == 0)
					{
						// Pixel aligned fill

						uint8_t * pDst = seg.pBuffer + (pixelPatch.y - seg.rect.y)*seg.pitch + (pixelPatch.x - seg.rect.x)*m_canvasPixelBytes;

						pFunc(pDst, m_canvasPixelBytes, seg.pitch - pixelPatch.w*m_canvasPixelBytes, pixelPatch.h, pixelPatch.w, col, m_colTrans, pixelPatch.pos());
					}
					else
					{
						// Subpixel fill


						// Fill all but anti-aliased edges

						int x1 = ((spxPatch.x + 63) >> 6);
						int y1 = ((spxPatch.y + 63) >> 6);
						int x2 = ((spxPatch.x + spxPatch.w) >> 6);
						int y2 = ((spxPatch.y + spxPatch.h) >> 6);

						uint8_t * pDst = seg.pBuffer + (y1 - seg.rect.y)*seg.pitch + (x1 - seg.rect.x)*m_canvasPixelBytes;

						pFunc(pDst, m_canvasPixelBytes, seg.pitch - (x2 - x1) * m_canvasPixelBytes, y2 - y1, x2 - x1, col, m_colTrans, {x1, y1} );

						//

						BlendMode	edgeBlendMode = (blendMode == BlendMode::Replace) ? BlendMode::Blend : blendMode; // Need to blend edges and corners even if fill is replace

						FillOp_p pEdgeFunc = pKernels->pFillKernels[(int)m_colTrans.mode][(int)edgeBlendMode];

						if (pEdgeFunc == nullptr)
						{
							pRects += (nRects - i - 1);

							if (blendMode == BlendMode::Ignore)
								break;

							char errorMsg[1024];

							snprintf(errorMsg, 1024, "Failed fill operation. SoftGfxDevice is missing fill kernel for TintMode::%s, BlendMode::%s onto surface of PixelFormat:%s.",
								toString(m_colTrans.mode),
								toString(blendMode),
								toString(m_canvasPixelFormat));

							GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
							break;
						}

						// Draw the sides

						int aaLeft = (4096 - (spxPatch.x & 0x3F) * 64) & 4095;
						int aaTop = (4096 - (spxPatch.y & 0x3F) * 64) & 4095;
						int aaRight = ((spxPatch.x + spxPatch.w) & 0x3F) * 64;
						int aaBottom = ((spxPatch.y + spxPatch.h) & 0x3F) * 64;

						int aaTopLeft = aaTop * aaLeft / 4096;
						int aaTopRight = aaTop * aaRight / 4096;
						int aaBottomLeft = aaBottom * aaLeft / 4096;
						int aaBottomRight = aaBottom * aaRight / 4096;


						if (blendMode != BlendMode::Replace)
						{
							int alpha = col.a;

							aaLeft = aaLeft * alpha >> 12;
							aaTop = aaTop * alpha >> 12;
							aaRight = aaRight * alpha >> 12;
							aaBottom = aaBottom * alpha >> 12;

							aaTopLeft = aaTopLeft * alpha >> 12;
							aaTopRight = aaTopRight * alpha >> 12;
							aaBottomLeft = aaBottomLeft * alpha >> 12;
							aaBottomRight = aaBottomRight * alpha >> 12;
						}

						HiColor color = col;

						if (aaTop != 0)
						{
							uint8_t * pDst = seg.pBuffer + (pixelPatch.y - seg.rect.y)*seg.pitch + (x1 - seg.rect.x)*m_canvasPixelBytes;
							int length = x2 - x1;
							color.a = aaTop;
							pEdgeFunc(pDst, m_canvasPixelBytes, 0, 1, length, color, m_colTrans, { x1,pixelPatch.y });
						}

						if (aaBottom != 0)
						{
							uint8_t * pDst = seg.pBuffer + (y2 - seg.rect.y)*seg.pitch + (x1 - seg.rect.x)*m_canvasPixelBytes;
							int length = x2 - x1;
							color.a = aaBottom;
							pEdgeFunc(pDst, m_canvasPixelBytes, 0, 1, length, color, m_colTrans, { x1,y2 });
						}

						if (aaLeft != 0)
						{
							uint8_t * pDst = seg.pBuffer + (y1 - seg.rect.y)*seg.pitch + (pixelPatch.x - seg.rect.x)*m_canvasPixelBytes;
							int length = y2 - y1;
							color.a = aaLeft;
							pEdgeFunc(pDst, seg.pitch, 0, 1, length, color, m_colTrans, { pixelPatch.x, y1 });
						}

						if (aaRight != 0)
						{
							uint8_t * pDst = seg.pBuffer + (y1 - seg.rect.y)*seg.pitch + (x2 - seg.rect.x)*m_canvasPixelBytes;
							int length = y2 - y1;
							color.a = aaRight;
							pEdgeFunc(pDst, seg.pitch, 0, 1, length, color, m_colTrans, { x2, y1 });
						}

						// Draw corner pieces


						if (aaTopLeft != 0)
						{
							uint8_t * pDst = seg.pBuffer + (pixelPatch.y - seg.rect.y)*seg.pitch + (pixelPatch.x - seg.rect.x)*m_canvasPixelBytes;
							color.a = aaTopLeft;
							pEdgeFunc(pDst, 0, 0, 1, 1, color, m_colTrans, { pixelPatch.x, pixelPatch.y });
						}

						if (aaTopRight != 0)
						{
							uint8_t * pDst = seg.pBuffer + (pixelPatch.y - seg.rect.y)*seg.pitch + (x2 - seg.rect.x)*m_canvasPixelBytes;
							color.a = aaTopRight;
							pEdgeFunc(pDst, 0, 0, 1, 1, color, m_colTrans, { x2, pixelPatch.y });
						}

						if (aaBottomLeft != 0)
						{
							uint8_t * pDst = seg.pBuffer + (y2 - seg.rect.y)*seg.pitch + (pixelPatch.x - seg.rect.x)*m_canvasPixelBytes;
							color.a = aaBottomLeft;
							pEdgeFunc(pDst, 0, 0, 1, 1, color, m_colTrans, { pixelPatch.x, y2 });
						}

						if (aaBottomRight != 0)
						{
							uint8_t * pDst = seg.pBuffer + (y2 - seg.rect.y)*seg.pitch + (x2 - seg.rect.x)*m_canvasPixelBytes;
							color.a = aaBottomRight;
							pEdgeFunc(pDst, 0, 0, 1, 1, color, m_colTrans, { x2, y2 });
						}

					}
				}
				break;
			}

			case Command::Line:
			{
				int32_t nClipRects = * p++;
				spx thickness = * p++;
				int32_t nLines = *p++;

				HiColor color = *pColors++;

				const RectSPX * pClipRects = pRects;
				pRects += nClipRects;

				HiColor fillColor = color;
				
				if( m_colTrans.mode == TintMode::Flat )
				fillColor = fillColor * m_colTrans.flatTintColor;

				//

				ClipLineOp_p pOp = nullptr;

				auto pKernels = m_pKernels[(int)m_canvasPixelFormat];
				if (pKernels)
					pOp = pKernels->pClipLineKernels[(int)m_blendMode];

				if (pOp == nullptr)
				{
					if (m_blendMode == BlendMode::Ignore)
						return;

					char errorMsg[1024];

					snprintf(errorMsg, 1024, "Failed drawLine operation. SoftGfxDevice is missing clipLine kernel for BlendMode::%s onto surface of PixelFormat:%s.",
						toString(m_blendMode),
						toString(m_canvasPixelFormat));

					GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
					return;
				}


				uint8_t* pRow;
				int		rowInc, pixelInc;
				int 	length, width;
				int		pos, slope;
				int		clipStart, clipEnd;

				for (int line = 0; line < nLines; line++)
				{
					auto p32 = (const spx *) p;
					CoordSPX beg = { *p32++, *p32++ };
					CoordSPX end = { *p32++, *p32++ };
					p = (const uint16_t*) p32;

					//TODO: Proper 26:6 support
					beg = Util::roundToPixels(beg);
					end = Util::roundToPixels(end);


					if (std::abs(beg.x - end.x) > std::abs(beg.y - end.y))
					{
						// Prepare mainly horizontal line segment

						if (beg.x > end.x)
							swap(beg, end);

						length = end.x - beg.x;
						slope = ((end.y - beg.y) * 65536) / length;

						width = _scaleLineThickness(thickness / 64.f, slope);

						// Loop through patches

						for( int i = 0 ; i < nClipRects ; i++ )
						{
							RectI clip = pClipRects[i] / 64;

							while( clip.x < pSegment->rect.x || clip.x >= pSegment->rect.x + pSegment->rect.w ||
								  clip.y < pSegment->rect.y || clip.y >= pSegment->rect.y + pSegment->rect.h )
							{
								pSegment++;
								if( pSegment == pSegEnd )
									pSegment = pSegBeg;
							}

							Segment& seg = * pSegment;


							int pos = ((beg.y-clip.y) << 16) - width / 2 + 32768;

							int rowInc = m_canvasPixelBytes;
							int pixelInc = seg.pitch;

							// Do clipping

							int _length = length;
							uint8_t * pRow = seg.pBuffer + (beg.x - seg.rect.x) * rowInc + (clip.y - seg.rect.y) * seg.pitch;

							if (beg.x < clip.x)
							{
								int cut = clip.x - beg.x;
								_length -= cut;
								pRow = seg.pBuffer + (clip.x - seg.rect.x) * rowInc + (clip.y - seg.rect.y) * seg.pitch;
								pos += slope * cut;
							}

							if (end.x > clip.x + clip.w)
								_length -= end.x - (clip.x + clip.w);

							int clipStart = 0;
							int clipEnd = (clip.h) << 16;


							//  Draw

							pOp(clipStart, clipEnd, pRow, rowInc, pixelInc, _length, width, pos, slope, fillColor, m_colTrans, { 0,0 });
						}

					}
					else
					{
						// Prepare mainly vertical line segment

						if (beg.y > end.y)
							swap(beg, end);

						length = end.y - beg.y;
						if (length == 0)
							return;											// TODO: Should stil draw the caps!

						// Need multiplication instead of shift as operand might be negative
						slope = ((end.x - beg.x) * 65536) / length;
						width = _scaleLineThickness(thickness / 64.f, slope);

						// Loop through patches

						for (int i = 0; i < nClipRects; i++)
						{

							RectI clip = pClipRects[i] / 64;

							while( clip.x < pSegment->rect.x || clip.x >= pSegment->rect.x + pSegment->rect.w ||
								  clip.y < pSegment->rect.y || clip.y >= pSegment->rect.y + pSegment->rect.h )
							{
								pSegment++;
								if( pSegment == pSegEnd )
									pSegment = pSegBeg;
							}

							Segment& seg = * pSegment;

							int pos = ((beg.x-clip.x) << 16) - width / 2 + 32768;

							int rowInc = seg.pitch;
							int pixelInc = m_canvasPixelBytes;

							// Do clipping

							int _length = length;
							uint8_t * pRow = seg.pBuffer + (beg.y - seg.rect.y) * rowInc + (clip.x - seg.rect.x) * m_canvasPixelBytes;

							if (beg.y < clip.y)
							{
								int cut = clip.y - beg.y;
								_length -= cut;
								pRow += rowInc * cut;
								pos += slope * cut;
							}

							if (end.y > clip.y + clip.h)
								_length -= end.y - (clip.y + clip.h);

							clipStart = 0;
							clipEnd = clip.w << 16;

							//  Draw

							pOp(clipStart, clipEnd, pRow, rowInc, pixelInc, _length, width, pos, slope, fillColor, m_colTrans, { 0,0 });
						}
					}
				}

				break;
			}


			case Command::DrawEdgemap:
			{
				auto pEdgemap = static_cast<SoftEdgemap*>(*pObjects++);

				int32_t nRects = *p++;
				int32_t	flip = *p++;
				p++;						// padding

				auto p32 = (const spx *) p;
				int32_t	destX = *p32++;
				int32_t	destY = *p32++;
				p = (const uint16_t*) p32;

				const RectSPX * pMyRects = pRects;

				pRects += nRects;

				int32_t nSegments = pEdgemap->segments();
				const HiColor * pSegmentColors = pEdgemap->m_pFlatColors;
				
				int nEdgeStrips = pEdgemap->m_size.w + 1;

				int edgeStripPitch = pEdgemap->m_nbSegments - 1;

				//TODO: Proper 26:6 support


				auto& mtx = s_blitFlipTransforms[flip];

				RectI _dest = { 
					Util::roundToPixels(destX), 
					Util::roundToPixels(destY), 
					pEdgemap->m_size.w * int(abs(mtx.xx)) + pEdgemap->m_size.h * int(abs(mtx.yx)),
					pEdgemap->m_size.w * int(abs(mtx.xy)) + pEdgemap->m_size.h * int(abs(mtx.yy)),
				};

				RectI dest = _dest;

				SegmentEdge edges[c_maxSegments - 1];


				while( dest.x < pSegment->rect.x || dest.x >= pSegment->rect.x + pSegment->rect.w ||
					  dest.y < pSegment->rect.y || dest.y >= pSegment->rect.y + pSegment->rect.h )
				{
					pSegment++;
					if( pSegment == pSegEnd )
						pSegment = pSegBeg;
				}

				auto& seg = * pSegment;

				int xPitch = m_canvasPixelBytes;
				int yPitch = seg.pitch;

				// We need to modify our transform since we are moving the destination pointer, not the source pointer, according to the transform.

				int simpleTransform[2][2];

				simpleTransform[0][0] = mtx.xx;
				simpleTransform[1][1] = mtx.yy;

				if (mtx.xx == 0 || mtx.yy == 0)
				{
					simpleTransform[0][1] = mtx.yx;
					simpleTransform[1][0] = mtx.xy;
				}
				else
				{
					simpleTransform[0][1] = mtx.xy;
					simpleTransform[1][0] = mtx.yx;
				}

				// Calculate pitches

				int colPitch = simpleTransform[0][0] * xPitch + simpleTransform[0][1] * yPitch;
				int rowPitch = simpleTransform[1][0] * xPitch + simpleTransform[1][1] * yPitch;

				// Calculate start coordinate

				CoordI start = dest.pos();

				if (simpleTransform[0][0] + simpleTransform[1][0] < 0)
					start.x += dest.w - 1;

				if (simpleTransform[0][1] + simpleTransform[1][1] < 0)
					start.y += dest.h - 1;

				// Detect if strip columns are lined horizontally or verically

				bool bHorizontalColumns = (abs(colPitch) == xPitch);

				// Limit size of destination rect by number of edgestrips.

				if (bHorizontalColumns)
				{
					if (dest.w > nEdgeStrips - 1)
					{
						if (colPitch < 0)
							dest.x += dest.w - nEdgeStrips - 1;

						dest.w = nEdgeStrips - 1;
					}
				}
				else
				{
					if (dest.h > nEdgeStrips - 1)
					{
						if (colPitch < 0)
							dest.y += dest.h - nEdgeStrips - 1;

						dest.h = nEdgeStrips - 1;
					}
				}

				// Apply tinting

				int16_t	colors[c_maxSegments * 4][4];				// RGBA order of elements
				bool	transparentSegments[c_maxSegments];
				bool	opaqueSegments[c_maxSegments];


				// Determine combined tint-mode, start with global m_colTrans...

				bool	bTintX = false;
				bool	bTintY = false;

				if (m_colTrans.mode != TintMode::None)
				{
					if (m_colTrans.mode == TintMode::GradientXY)
					{
						bTintX = bTintY = true;
					}
					else
					{
						if (m_colTrans.mode == TintMode::GradientY)
						{
							if (mtx.xy == 0 && mtx.yx == 0)
								bTintY = true;
							else
								bTintX = true;
						}
						if (m_colTrans.mode == TintMode::GradientX)
						{
							if (mtx.xy == 0 && mtx.yx == 0)
								bTintX = true;
							else
								bTintY = true;
						}

					}
				}

				// ... add in tintmaps for segments
								
				if( pEdgemap->colorstripsX() )
					bTintX = true;
				
				if( pEdgemap->colorstripsY() )
					bTintY = true;				

				// Unpack input colors and fill in transparentSegments

				if (!bTintX && !bTintY)
				{
					// If we just use flat tinting (or no tint at all), we tint our segment colors right away

					auto pColor = pSegmentColors;
					
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


				HiColor * pTintColorsX = nullptr;
				HiColor * pTintColorsY = nullptr;

				int		tintBufferSizeX = 0;
				int		tintBufferSizeY = 0;
				
				int		segmentPitchTintmapY = 0;
				int		segmentPitchTintmapX = 0;

				
				// If we instead have gradients we have things to take care of...

				if (bTintX || bTintY)
				{
					if (bTintX)
					{
						int length = pEdgemap->m_size.w;

						segmentPitchTintmapX = length;

						// Generate the buffer that we will need

						tintBufferSizeX = sizeof(HiColor) * nSegments * length;
						pTintColorsX = (HiColor*)GfxBase::memStackAlloc(tintBufferSizeX);
						
						// export segment colorstrip or flat colors into our buffer
						
						if (pEdgemap->m_pColorstripsX)
							memcpy(pTintColorsX, pEdgemap->m_pColorstripsX, tintBufferSizeX);
						else if (pEdgemap->m_pFlatColors)
						{
							HiColor* pOutput = pTintColorsX;
							HiColor* pInput = pEdgemap->m_pFlatColors;

							for (int seg = 0; seg < nSegments; seg++)
							{
								for (int i = 0; i < length; i++)
									*pOutput++ = *pInput;

								pInput++;
							}
						}
					}
					
					if (bTintY)
					{
						int length = pEdgemap->m_size.h;

						segmentPitchTintmapY = length;
						
						// Generate the buffer that we will need

						tintBufferSizeY = sizeof(HiColor) * nSegments * length;
						pTintColorsY = (HiColor*)GfxBase::memStackAlloc(tintBufferSizeY);
						
						// Export segment colorstrip into our buffer if we have them.
						// Otherwise, if we haven't already exported our flat colors we do it now.
						// If flat colors already have been exported, we just fill with white.

						if (pEdgemap->m_pColorstripsY)
							memcpy(pTintColorsY, pEdgemap->m_pColorstripsY, tintBufferSizeY);
						else if (pEdgemap->m_pFlatColors && !bTintX)
						{
							HiColor* pOutput = pTintColorsX;
							HiColor* pInput = pEdgemap->m_pFlatColors;

							for (int seg = 0; seg < nSegments; seg++)
							{
								for (int i = 0; i < length; i++)
									*pOutput++ = *pInput;

								pInput++;
							}
						}
						else
						{
							for (int i = 0; i < tintBufferSizeY; i++)
								pTintColorsY[i] = HiColor::White;
						}
					}

					// Possibly add in global tint, which might need to be rotated, offset and reversed

					if (m_colTrans.mode == TintMode::Flat)
					{
						if (m_colTrans.flatTintColor != HiColor::White)
						{
							// We only apply tintColor once, so we only modify one of the color lists.

							if (pTintColorsX)
							{
								for (int i = 0; i < nSegments * pEdgemap->m_size.w; i++)
									pTintColorsX[i] *= m_colTrans.flatTintColor;
							}
							else if (pTintColorsY)
							{
								for (int i = 0; i < nSegments * pEdgemap->m_size.h; i++)
									pTintColorsY[i] *= m_colTrans.flatTintColor;
							}
						}
					}
					else if (m_colTrans.mode == TintMode::GradientX || m_colTrans.mode == TintMode::GradientY || m_colTrans.mode == TintMode::GradientXY)
					{
						const HiColor* pGlobalsX = m_colTrans.pTintAxisX ? m_colTrans.pTintAxisX + _dest.x - m_colTrans.tintRect.x : nullptr;
						const HiColor* pGlobalsY = m_colTrans.pTintAxisY ? m_colTrans.pTintAxisY + _dest.y - m_colTrans.tintRect.y : nullptr;

						int width = _dest.w;
						int height = _dest.h;

						int pitchX = mtx.xx + mtx.xy;
						int pitchY = mtx.yx + mtx.yy;

						if (mtx.xy != 0 || mtx.yx != 0)
						{
							std::swap(pGlobalsX, pGlobalsY);
							std::swap(width, height);
							std::swap(pitchX, pitchY);
						}


						if (pitchX < 0 && pGlobalsX)
							pGlobalsX += width - 1;

						if (pitchY < 0 && pGlobalsY)
							pGlobalsY += height - 1;


						if (pGlobalsX)
						{
							HiColor* pDest = pTintColorsX;
							for (int seg = 0; seg < nSegments; seg++)
							{
								const HiColor* pSrc = pGlobalsX;

								for (int i = 0; i < width; i++)
								{
									*pDest++ *= *pSrc;
									pSrc += pitchX;
								}
							}
						}

						if (pGlobalsY)
						{
							HiColor* pDest = pTintColorsY;
							for (int seg = 0; seg < nSegments; seg++)
							{
								const HiColor* pSrc = pGlobalsY;

								for (int i = 0; i < height; i++)
								{
									*pDest++ *= *pSrc;
									pSrc += pitchY;
								}
							}
						}
					}


					// Mark transparent and opaque segments
					
					if (m_colTrans.bTintOpaque)
					{
						for (int i = 0; i < nSegments; i++)
						{
							opaqueSegments[i] = pEdgemap->m_opaqueSegments.test(i);
							transparentSegments[i] = pEdgemap->m_transparentSegments.test(i);
						}

					}
					else
					{
						for (int i = 0; i < nSegments; i++)
						{
							opaqueSegments[i] = false;
							transparentSegments[i] = pEdgemap->m_transparentSegments.test(i);
						}
					}
				}


				// Modify opaqueSegments if our state isn't blend

				if (m_blendMode != BlendMode::Blend && m_blendMode != BlendMode::BlendFixedColor)
				{
					bool val = (m_blendMode == BlendMode::Replace);

					for (int seg = 0; seg < nSegments; seg++)
						opaqueSegments[seg] = val;
				}

				// Modify transparentSegments if our state is BlendFixedColor

				if (m_blendMode == BlendMode::BlendFixedColor)
				{
					for (int seg = 0; seg < nSegments; seg++)
						transparentSegments[seg] = false;
				}

				// Set start position and clip dest

				uint8_t* pOrigo = seg.pBuffer - seg.rect.y * yPitch - seg.rect.x * xPitch + start.y * yPitch + start.x * xPitch;
//				uint8_t* pOrigo = m_pCanvasPixels + start.y * yPitch + start.x * xPitch;

				
				StripSource stripSource = StripSource::Colors;
				if( bTintY )
				{
					if(bTintX)
						stripSource = StripSource::ColorsAndTintmaps;
					else
						stripSource = StripSource::Tintmaps;
				}
				
				SegmentOp_p	pOp = nullptr;
				auto pKernels = m_pKernels[(int)m_canvasPixelFormat];
				if (pKernels)
					pOp = pKernels->pSegmentKernels[(int)stripSource][(int)m_blendMode];

				if (pOp == nullptr)
				{
					if (m_blendMode == BlendMode::Ignore)
						return;

					char errorMsg[1024];

					snprintf(errorMsg, 1024, "Failed draw segments operation. SoftGfxDevice is missing segments kernel %s Y-tint for BlendMode::%s onto surface of PixelFormat:%s.",
						bTintY ? "with" : "without",
						toString(m_blendMode),
						toString(m_canvasPixelFormat));

					GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
					return;
				}

				// Loop through patches

				for (int patchIdx = 0; patchIdx < nRects; patchIdx++)
				{
					// Clip patch

					RectI patch = RectI::overlap(dest, pMyRects[patchIdx] / 64);
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

					
					pEdgemap->samples();

					const int* pEdgeStrips = pEdgemap->m_pSamples + columnOfs * edgeStripPitch;
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

						const int16_t* pColors = &colors[skippedSegments][0];

						if (bTintX)
						{
							for( int i = 0 ; i < nSegments ; i++ )
							{
								HiColor& col = pTintColorsX[i*segmentPitchTintmapX+columnOfs];

								colors[i][0] = col.r;
								colors[i][1] = col.g;
								colors[i][2] = col.b;
								colors[i][3] = col.a;
							}
							
							pColors = &colors[0][0];
						}

						//

 						pOp(clipBeg, clipEnd, pStripStart, rowPitch, nEdges, edges, pColors, pTintColorsY + skippedSegments * segmentPitchTintmapY, segmentPitchTintmapY , transparentSegments + skippedSegments, opaqueSegments + skippedSegments, m_colTrans);
						pEdgeStrips += edgeStripPitch;
						pStripStart += colPitch;
						columnOfs++;
					}
				}

				// Free what we have reserved on the memStack.

				if (tintBufferSizeY > 0)
					GfxBase::memStackFree(tintBufferSizeY);

				if (tintBufferSizeX > 0)
					GfxBase::memStackFree(tintBufferSizeX);

				break;
			}

			case Command::Blur:
			case Command::Blit:
			case Command::ClipBlit:
			case Command::Tile:
			{
				if (m_bBlitFunctionNeedsUpdate)
				{
					_updateBlitFunctions();
					m_bBlitFunctionNeedsUpdate = false;
				}

				int32_t nRects = *p++;
				int32_t transform = *p++;
				p++;							// padding

				auto p32 = (const spx *) p;
				int srcX = *p32++;
				int srcY = *p32++;
				spx dstX = *p32++;
				spx dstY = *p32++;
				p = (const uint16_t*) p32;

				if (transform <= int(GfxFlip_max) )
				{
					const Transform& mtx = s_blitFlipTransforms[transform];

					// Step forward _src by half a pixel, so we start from correct pixel.

					srcX += (mtx.xx + mtx.yx) * 512;
					srcY += (mtx.xy + mtx.yy) * 512;

					//

					for (int i = 0; i < nRects; i++)
					{
						RectI	patch = (*pRects++) / 64;

						CoordI src = { srcX / 1024, srcY / 1024 };
						CoordI dest = { dstX / 64, dstY / 64 };

						CoordI	patchOfs = patch.pos() - dest;


						//

						src.x += patchOfs.x * mtx.xx + patchOfs.y * mtx.yx;
						src.y += patchOfs.x * mtx.xy + patchOfs.y * mtx.yy;


						while( patch.x < pSegment->rect.x || patch.x >= pSegment->rect.x + pSegment->rect.w ||
							  patch.y < pSegment->rect.y || patch.y >= pSegment->rect.y + pSegment->rect.h )
						{
							pSegment++;
							if( pSegment == pSegEnd )
								pSegment = pSegBeg;
						}

						Segment& seg = * pSegment;


						uint8_t * pDst = seg.pBuffer + (patch.y-seg.rect.y) * seg.pitch + (patch.x - seg.rect.x) * m_canvasPixelBytes;


						if (cmd == Command::Blit)
							(this->*m_pLinearStraightBlitOp)(pDst, seg.pitch, patch.w, patch.h, src, mtx, patch.pos(), m_pStraightBlitFirstPassOp);
						else if (cmd == Command::Tile)
							(this->*m_pLinearStraightTileOp)(pDst, seg.pitch, patch.w, patch.h, src, mtx, patch.pos(), m_pStraightTileFirstPassOp);
						else
							(this->*m_pLinearStraightBlurOp)(pDst, seg.pitch, patch.w, patch.h, src, mtx, patch.pos(), m_pStraightBlurFirstPassOp);
					}
				}
				else
				{
					binalInt mtx[2][2];

					const Transform* pTransform = &m_pTransformsBeg[transform - GfxFlip_size];

					mtx[0][0] = binalInt(pTransform->xx * BINAL_MUL);
					mtx[0][1] = binalInt(pTransform->xy * BINAL_MUL);
					mtx[1][0] = binalInt(pTransform->yx * BINAL_MUL);
					mtx[1][1] = binalInt(pTransform->yy * BINAL_MUL);


					//

					for (int i = 0; i < nRects; i++)
					{
						RectI	patch = (*pRects++) / 64;


						BinalCoord src = { srcX * (BINAL_MUL / 1024), srcY * (BINAL_MUL / 1024) };
						CoordI dest = { dstX / 64, dstY / 64 };

						CoordI	patchOfs = patch.pos() - dest;

						//

						src.x += patchOfs.x * mtx[0][0] + patchOfs.y * mtx[1][0];
						src.y += patchOfs.x * mtx[0][1] + patchOfs.y * mtx[1][1];

						//

						Segment& seg = * pSegment;

						uint8_t * pDst = seg.pBuffer + (patch.y-seg.rect.y) * seg.pitch + (patch.x - seg.rect.x) * m_canvasPixelBytes;


						if( cmd == Command::Blit)
							(this->*m_pLinearTransformBlitOp)(pDst, seg.pitch, patch.w, patch.h, src, mtx,patch.pos(), m_pTransformBlitFirstPassOp);
						else if (cmd == Command::ClipBlit)
							(this->*m_pLinearTransformClipBlitOp)(pDst, seg.pitch, patch.w, patch.h, src, mtx, patch.pos(), m_pTransformClipBlitFirstPassOp);
						else if (cmd == Command::Tile)
							(this->*m_pLinearTransformTileOp)(pDst, seg.pitch, patch.w, patch.h, src, mtx, patch.pos(), m_pTransformTileFirstPassOp);
						else
							(this->*m_pLinearTransformBlurOp)(pDst, seg.pitch, patch.w, patch.h, src, mtx, patch.pos(), m_pTransformBlurFirstPassOp);

					}
				}

				break;
			}

			default:
				break;
			}
		}

		// Save progress.

		m_pRectsPtr = pRects;
		m_pColorsPtr = pColors;
	}

	//____ _updateBlitFunctions() _____________________________________________

	void LinearBackend::_updateBlitFunctions()
	{
		SoftBackend::_updateBlitFunctions();

		if( m_pStraightBlitOp == &SoftBackend::_onePassStraightBlit )
			m_pLinearStraightBlitOp = &LinearBackend::_onePassLinearStraightBlit;
		else if( m_pStraightBlitOp == &SoftBackend::_twoPassStraightBlit )
			m_pLinearStraightBlitOp = &LinearBackend::_twoPassLinearStraightBlit;
		else
			m_pLinearStraightBlitOp = &LinearBackend::_dummyLinearStraightBlit;

		if( m_pStraightTileOp == &SoftBackend::_onePassStraightBlit )
			m_pLinearStraightTileOp = &LinearBackend::_onePassLinearStraightBlit;
		else if( m_pStraightTileOp == &SoftBackend::_twoPassStraightBlit )
			m_pLinearStraightTileOp = &LinearBackend::_twoPassLinearStraightBlit;
		else
			m_pLinearStraightTileOp = &LinearBackend::_dummyLinearStraightBlit;

		if( m_pStraightBlurOp == &SoftBackend::_onePassStraightBlit )
			m_pLinearStraightBlurOp = &LinearBackend::_onePassLinearStraightBlit;
		else if( m_pStraightBlurOp == &SoftBackend::_twoPassStraightBlit )
			m_pLinearStraightBlurOp = &LinearBackend::_twoPassLinearStraightBlit;
		else
			m_pLinearStraightBlurOp = &LinearBackend::_dummyLinearStraightBlit;

		if( m_pTransformBlitOp == &SoftBackend::_onePassTransformBlit )
			m_pLinearTransformBlitOp = &LinearBackend::_onePassLinearTransformBlit;
		else if( m_pTransformBlitOp == &SoftBackend::_twoPassTransformBlit )
			m_pLinearTransformBlitOp = &LinearBackend::_twoPassLinearTransformBlit;
		else
			m_pLinearTransformBlitOp = &LinearBackend::_dummyLinearTransformBlit;

		if( m_pTransformTileOp == &SoftBackend::_onePassTransformBlit )
			m_pLinearTransformTileOp = &LinearBackend::_onePassLinearTransformBlit;
		else if( m_pTransformTileOp == &SoftBackend::_twoPassTransformBlit )
			m_pLinearTransformTileOp = &LinearBackend::_twoPassLinearTransformBlit;
		else
			m_pLinearTransformTileOp = &LinearBackend::_dummyLinearTransformBlit;

		if( m_pTransformBlurOp == &SoftBackend::_onePassTransformBlit )
			m_pLinearTransformBlurOp = &LinearBackend::_onePassLinearTransformBlit;
		else if( m_pTransformBlurOp == &SoftBackend::_twoPassTransformBlit )
			m_pLinearTransformBlurOp = &LinearBackend::_twoPassLinearTransformBlit;
		else
			m_pLinearTransformBlurOp = &LinearBackend::_dummyLinearTransformBlit;

	}

	//____ _onePassLinearStraightBlit() _____________________________________________

	void LinearBackend::_onePassLinearStraightBlit(uint8_t * pDst, int destPitch, int width, int height, CoordI src, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp)
	{
		const SoftSurface * pSource = m_pBlitSource;

		int srcPixelBytes = pSource->m_pPixelDescription->bits / 8;
		int dstPixelBytes = m_canvasPixelBytes;

		Pitches pitches;

		pitches.srcX = srcPixelBytes * matrix.xx + pSource->m_pitch * matrix.xy;
		pitches.dstX = dstPixelBytes;
		pitches.srcY = srcPixelBytes * matrix.yx + pSource->m_pitch * matrix.yy - pitches.srcX*width;
		pitches.dstY = destPitch - width * dstPixelBytes;

		uint8_t * pSrc = pSource->m_pData + src.y * pSource->m_pitch + src.x * srcPixelBytes;

		pPassOneOp(pSrc, pDst, pSource, pitches, height, width, m_colTrans, patchPos, &matrix);
	}

	//____ _twoPassLinearStraightBlit() _____________________________________________

	void LinearBackend::_twoPassLinearStraightBlit(uint8_t * pDst, int destPitch, int width, int height, CoordI src, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp)
	{
		SoftSurface * pSource = m_pBlitSource;

		int srcPixelBytes = pSource->m_pPixelDescription->bits / 8;
		int dstPixelBytes = m_canvasPixelBytes;

		Pitches pitchesPass1, pitchesPass2;

		pitchesPass1.srcX = srcPixelBytes * matrix.xx + pSource->m_pitch * matrix.xy;
		pitchesPass1.dstX = 8;
		pitchesPass1.srcY = srcPixelBytes * matrix.yx + pSource->m_pitch * matrix.yy - pitchesPass1.srcX*width;
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

			uint8_t * pSrc = pSource->m_pData + src.y * pSource->m_pitch + line*int(srcPixelBytes * matrix.yx + pSource->m_pitch * matrix.yy) + src.x * srcPixelBytes;
	//			uint8_t * pSrc = pSource->m_pData + (src.y+line) * pSource->m_pitch + src.x * srcPixelBytes;

			pPassOneOp(pSrc, pChunkBuffer, pSource, pitchesPass1, thisChunkLines, width, m_colTrans, { 0,0 }, &matrix);
			m_pBlitSecondPassOp(pChunkBuffer, pDst, pSource, pitchesPass2, thisChunkLines, width, m_colTrans, patchPos, nullptr);

			patchPos.y += thisChunkLines;
			line += thisChunkLines;

			pDst += destPitch*thisChunkLines;
		}

		GfxBase::memStackFree(memBufferSize);
	}

	//____ _dummyLinearStraightBlit() _____________________________________________

	void LinearBackend::_dummyLinearStraightBlit(uint8_t * pDst, int destPitch, int width, int height, CoordI pos, const Transform& matrix, CoordI patchPos, StraightBlitOp_p pPassOneOp)
	{
	}


	//____ _onePassLinearTransformBlit() ____________________________________________

	void LinearBackend::_onePassLinearTransformBlit(uint8_t * pDst, int destPitch, int destWidth, int destHeight, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp)
	{
		pPassOneOp(m_pBlitSource, pos, transformMatrix, pDst, m_canvasPixelBytes, destPitch - m_canvasPixelBytes * destWidth, destHeight, destWidth, m_colTrans, patchPos);
	}

	//____ _twoPassLinearTransformBlit() ____________________________________________

	void LinearBackend::_twoPassLinearTransformBlit(	uint8_t * pDst, int destPitch, int destWidth, int destHeight, BinalCoord pos, const binalInt transformMatrix[2][2],	CoordI patchPos, TransformBlitOp_p pPassOneOp)
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

	void LinearBackend::_dummyLinearTransformBlit(uint8_t * pDst, int destPitch, int width, int height, BinalCoord pos, const binalInt transformMatrix[2][2], CoordI patchPos, TransformBlitOp_p pPassOneOp)
	{
	}



} // namespace wg
