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
#include <wg_linearbackend.h>
#include <wg_softedgemap.h>
#include <wg_gfxbase.h>
#include <cstring>


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


		SoftBackend::beginSession(canvasRef, pCanvas, nUpdateRects, pUpdateRects, pInfo);
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
		for(size_t i = 0 ; i < m_canvasSegments.size() ; i++ )
		{
			auto& updateRect = m_canvasSegments[i].rect;
			m_canvasSegments[i].pitch = updateRect.w * m_canvasPixelBytes;
			m_canvasSegments[i].pBuffer = pCanvasBuffer + ofs;
			ofs += updateRect.w * updateRect.h * m_canvasPixelBytes + m_segmentPadding;
		}
	}

	//____ processCommands() _____________________________________________

	void LinearBackend::processCommands(const uint16_t* pBeg, const uint16_t* pEnd, int version)
	{
		if( m_pCanvas )
		{
			SoftBackend::processCommands(pBeg,pEnd, version);
			return;
		}

		const RectSPX *	pRects = m_pRectsPtr;
		const HiColor*	pColors = m_pColorsPtr;
		Object* const *	pObjects = m_pObjectsPtr;

		Segment *	pSegBeg = m_canvasSegments.data();
		Segment *	pSegEnd = m_canvasSegments.data() + m_canvasSegments.size();
		Segment *	pSegment = pSegBeg;

		int 		customTransformStart = version == 1 ? GfxFlip_size : NbStandardTransforms;

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
					m_tintColor = *pColors++;
					if (!(statesChanged & uint8_t(StateChange::Tint)))
						_updateTint();
				}

				if (statesChanged & uint8_t(StateChange::Tint))
					p = _setTint(p, pColors);

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
						m_colTrans.blurMtxR[i] = int(pRed[i]) * 2;
						m_colTrans.blurMtxG[i] = int(pGreen[i]) * 2;
						m_colTrans.blurMtxB[i] = int(pBlue[i]) * 2;
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

				// Optimize calls

				BlendMode blendMode = m_blendMode;
				if (blendMode == BlendMode::Blend && col.a == 4096 && m_colTrans.bTintOpaque )
				{
					blendMode = BlendMode::Replace;
				}

				auto pKernels = m_pKernels[(int)m_canvasPixelFormat];

				// Get kernel, fall back to Flat kernels drawn in runs if GradientX kernels are missing.

				auto getKernel = [&](BlendMode mode, bool& bRuns) -> FillOp_p
				{
					bRuns = false;
					if (!pKernels)
						return nullptr;

					FillOp_p pKernel = pKernels->pFillKernels[(int)m_colTrans.mode][(int)mode];
					if (!pKernel && m_colTrans.mode == TintMode::GradientX)
					{
						pKernel = pKernels->pFillKernels[(int)TintMode::Flat][(int)mode];
						bRuns = (pKernel != nullptr);
					}
					return pKernel;
				};

				auto reportMissingKernel = [&](BlendMode mode)
				{
					char errorMsg[1024];

					snprintf(errorMsg, 1024, "Failed fill operation. LinearBackend is missing fill kernel for TintMode::%s, BlendMode::%s onto surface of PixelFormat:%s.",
						toString(m_colTrans.mode),
						toString(mode),
						toString(m_canvasPixelFormat));

					GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
				};

				bool bRuns;
				FillOp_p pFunc = getKernel(blendMode, bRuns);

				if (!pFunc)
				{
					pRects += nRects;

					if (blendMode == BlendMode::Ignore)
						break;

					reportMissingKernel(blendMode);
					break;
				}

				for (int i = 0; i < nRects; i++)
				{
					RectI spxPatch = *pRects++;

					if(spxPatch.isEmpty())
						continue;

					RectI pixelPatch = spxPatch / 64;

					while( pixelPatch.x < pSegment->rect.x || pixelPatch.x >= pSegment->rect.x + pSegment->rect.w ||
						  pixelPatch.y < pSegment->rect.y || pixelPatch.y >= pSegment->rect.y + pSegment->rect.h )
					{
						pSegment++;
						if( pSegment == pSegEnd )
							pSegment = pSegBeg;
					}

					Segment& seg = * pSegment;

					// Fills a rectangle of whole pixels with given kernel, taking tint into account.

					auto fillPixels = [&](FillOp_p pOp, bool bRunsForOp, const RectI& rect, HiColor color)
					{
						_forTintSpans(rect, bRunsForOp, [&](const RectI& sub)
						{
							uint8_t* pDst = seg.pBuffer + (sub.y - seg.rect.y) * seg.pitch + (sub.x - seg.rect.x) * m_canvasPixelBytes;
							pOp(pDst, m_canvasPixelBytes, seg.pitch - sub.w * m_canvasPixelBytes, sub.h, sub.w, color, m_colTrans, sub.pos());
						});
					};

					if (((spxPatch.x | spxPatch.y | spxPatch.w | spxPatch.h) & 63) == 0)
					{
						// Pixel aligned fill

						fillPixels(pFunc, bRuns, pixelPatch, col);
					}
					else
					{
						// Subpixel fill


						// Fill all but anti-aliased edges

						int x1 = ((spxPatch.x + 63) >> 6);
						int y1 = ((spxPatch.y + 63) >> 6);
						int x2 = ((spxPatch.x + spxPatch.w) >> 6);
						int y2 = ((spxPatch.y + spxPatch.h) >> 6);

						fillPixels(pFunc, bRuns, RectI(x1, y1, x2 - x1, y2 - y1), col);

						//

						BlendMode	edgeBlendMode = (blendMode == BlendMode::Replace) ? BlendMode::Blend : blendMode; // Need to blend edges and corners even if fill is replace

						bool bEdgeRuns;
						FillOp_p pEdgeFunc = getKernel(edgeBlendMode, bEdgeRuns);

						if (pEdgeFunc == nullptr)
						{
							pRects += (nRects - i - 1);

							if (blendMode == BlendMode::Ignore)
								break;

							reportMissingKernel(edgeBlendMode);
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

						auto edge = [&](int alpha, const RectI& rect)
						{
							if (alpha != 0)
							{
								HiColor color = col;
								color.a = alpha;
								fillPixels(pEdgeFunc, bEdgeRuns, rect, color);
							}
						};

						edge(aaTop, RectI(x1, pixelPatch.y, x2 - x1, 1));
						edge(aaBottom, RectI(x1, y2, x2 - x1, 1));
						edge(aaLeft, RectI(pixelPatch.x, y1, 1, y2 - y1));
						edge(aaRight, RectI(x2, y1, 1, y2 - y1));

						// Draw corner pieces

						edge(aaTopLeft, RectI(pixelPatch.x, pixelPatch.y, 1, 1));
						edge(aaTopRight, RectI(x2, pixelPatch.y, 1, 1));
						edge(aaBottomLeft, RectI(pixelPatch.x, y2, 1, 1));
						edge(aaBottomRight, RectI(x2, y2, 1, 1));
					}
				}
				break;
			}

			case Command::Line:
			{
				int32_t nClipRects = * p++;
				int32_t nLines = *p++;
				p++;


				const RectSPX * pClipRects = pRects;
				pRects += nClipRects;

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

					snprintf(errorMsg, 1024, "Failed drawLine operation. LinearBackend is missing clipLine kernel for BlendMode::%s onto surface of PixelFormat:%s.",
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
					HiColor color = *pColors++;
					HiColor fillColor = color;

					// Lines are only tinted by the flat tint color, not by Tints.

					if( m_tintColor != HiColor::White )
						fillColor = fillColor * m_tintColor;

					CoordSPX beg, end;

					auto p32 = (const spx *) p;
					beg.x = *p32++;
					beg.y = *p32++;
					end.x = *p32++;
					end.y = *p32++;
					p = (const uint16_t*) p32;

					spx thickness = * p++;
					p++;									// padding

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

							if(clip.isEmpty())
								continue;

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
							continue;											// TODO: Should stil draw the caps!

						// Need multiplication instead of shift as operand might be negative
						slope = ((end.x - beg.x) * 65536) / length;
						width = _scaleLineThickness(thickness / 64.f, slope);

						// Loop through patches

						for (int i = 0; i < nClipRects; i++)
						{

							RectI clip = pClipRects[i] / 64;

							if(clip.isEmpty())
								continue;

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

				int32_t nSegments = pEdgemap->renderSegments();		// Segments to draw. Last one extends to the end.
				
				int nEdgeStrips = pEdgemap->m_size.w + 1;

				int edgeStripPitch = pEdgemap->m_nbSegments - 1;

				//TODO: Proper 26:6 support


				auto& mtx = s_standardTransforms[flip];

				RectI _dest = { 
					Util::roundToPixels(destX), 
					Util::roundToPixels(destY), 
					pEdgemap->m_size.w * int(abs(mtx.xx)) + pEdgemap->m_size.h * int(abs(mtx.yx)),
					pEdgemap->m_size.w * int(abs(mtx.xy)) + pEdgemap->m_size.h * int(abs(mtx.yy)),
				};

				RectI dest = _dest;

				SegmentEdge edges[c_maxSegments - 1];

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

				int colDirection = simpleTransform[0][0] + simpleTransform[0][1];

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
						if (colDirection < 0)
							dest.x += dest.w - nEdgeStrips - 1;

						dest.w = nEdgeStrips - 1;
					}
				}
				else
				{
					if (dest.h > nEdgeStrips - 1)
					{
						if (colDirection < 0)
							dest.y += dest.h - nEdgeStrips - 1;

						dest.h = nEdgeStrips - 1;
					}
				}

				// Apply tinting

				EdgemapTinting tinting;
				_beginEdgemapTinting(pEdgemap, nSegments, tinting);

				bool* transparentSegments = tinting.transparent;
				bool* opaqueSegments = tinting.opaque;

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

				//
				
				StripSource stripSource = tinting.bPerPixel ? StripSource::Tintmaps : StripSource::Colors;

				SegmentOp_p	pOp = nullptr;
				auto pKernels = m_pKernels[(int)m_canvasPixelFormat];
				if (pKernels)
					pOp = pKernels->pSegmentKernels[(int)stripSource][(int)m_blendMode];

				if (pOp == nullptr)
				{
					_endEdgemapTinting(tinting);

					if (m_blendMode == BlendMode::Ignore)
						break;

					char errorMsg[1024];

					snprintf(errorMsg, 1024, "Failed draw segments operation. LinearBackend is missing segments kernel %s tint for BlendMode::%s onto surface of PixelFormat:%s.",
						tinting.bPerPixel ? "with" : "without",
						toString(m_blendMode),
						toString(m_canvasPixelFormat));

					GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::RenderFailure, errorMsg, this, &TYPEINFO, __func__, __FILE__, __LINE__);
					break;
				}

				// Loop through patches

				for (int patchIdx = 0; patchIdx < nRects; patchIdx++)
				{
					// Clip patch

					RectI patch = RectI::overlap(dest, pMyRects[patchIdx] / 64);
					if (patch.w == 0 || patch.h == 0)
						continue;

					// Find segment this patch is within

					while( patch.x < pSegment->rect.x || patch.x >= pSegment->rect.x + pSegment->rect.w ||
						  patch.y < pSegment->rect.y || patch.y >= pSegment->rect.y + pSegment->rect.h )
					{
						pSegment++;
						if( pSegment == pSegEnd )
							pSegment = pSegBeg;
					}

					// Calculate pitches and origo

					auto seg = * pSegment;

					int xPitch = m_canvasPixelBytes;
					int yPitch = seg.pitch;

					int colPitch = simpleTransform[0][0] * xPitch + simpleTransform[0][1] * yPitch;
					int rowPitch = simpleTransform[1][0] * xPitch + simpleTransform[1][1] * yPitch;

					uint8_t* pOrigo = seg.pBuffer - seg.rect.y * yPitch - seg.rect.x * xPitch + start.y * yPitch + start.x * xPitch;


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

						// Generate colors for column if tint varies over edgemap

						_tintEdgemapColumn(pEdgemap, nSegments, tinting, columnOfs, rowOfs, rowOfs + rows, pEdgeStrips, start, simpleTransform);

						const int16_t* pColors = &tinting.colors[skippedSegments][0];

						//

						pOp(clipBeg, clipEnd, pStripStart, rowPitch, nEdges, edges, pColors, tinting.pColumns + skippedSegments * tinting.pitch, tinting.pitch, transparentSegments + skippedSegments, opaqueSegments + skippedSegments, m_colTrans);
						pEdgeStrips += edgeStripPitch;
						pStripStart += colPitch;
						columnOfs++;
					}
				}

				// Free what we have reserved on the memStack.

				_endEdgemapTinting(tinting);

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

				for (int i = 0; i < nRects; i++)
				{
					auto p32 = (const spx *) p;
					int srcX = *p32++;
					int srcY = *p32++;
					spx dstX = *p32++;
					spx dstY = *p32++;
					p = (const uint16_t*) p32;

					int32_t transform = *p++;
					p++;							// padding

					RectI	patch = (*pRects++) / 64;

					if(patch.isEmpty())
						continue;

					while (patch.x < pSegment->rect.x || patch.x >= pSegment->rect.x + pSegment->rect.w ||
						patch.y < pSegment->rect.y || patch.y >= pSegment->rect.y + pSegment->rect.h)
					{
						pSegment++;
						if (pSegment == pSegEnd)
							pSegment = pSegBeg;
					}

					Segment& seg = *pSegment;

					if (transform < customTransformStart )
					{
						const Transform& mtx = s_standardTransforms[transform];

						// Step forward _src by half a pixel, so we start from correct pixel.

						srcX += (mtx.xx + mtx.yx) * 512;
						srcY += (mtx.xy + mtx.yy) * 512;

						//

						CoordI src0 = { srcX / 1024, srcY / 1024 };
						CoordI dest = { dstX / 64, dstY / 64 };

						_forTintSpans(patch, m_bBlitTintRuns, [&](const RectI& sub)
						{
							CoordI	patchOfs = sub.pos() - dest;
							CoordI	src = src0;

							src.x += patchOfs.x * mtx.xx + patchOfs.y * mtx.yx;
							src.y += patchOfs.x * mtx.xy + patchOfs.y * mtx.yy;

							uint8_t * pDst = seg.pBuffer + (sub.y-seg.rect.y) * seg.pitch + (sub.x - seg.rect.x) * m_canvasPixelBytes;

							if (cmd == Command::Blit)
								(this->*m_pLinearStraightBlitOp)(pDst, seg.pitch, sub.w, sub.h, src, mtx, sub.pos(), m_pStraightBlitFirstPassOp);
							else if (cmd == Command::Tile)
								(this->*m_pLinearStraightTileOp)(pDst, seg.pitch, sub.w, sub.h, src, mtx, sub.pos(), m_pStraightTileFirstPassOp);
							else
								(this->*m_pLinearStraightBlurOp)(pDst, seg.pitch, sub.w, sub.h, src, mtx, sub.pos(), m_pStraightBlurFirstPassOp);
						});
					}
					else
					{
						binalInt mtx[2][2];

						const Transform* pTransform = &m_pTransformsBeg[transform - customTransformStart];

						mtx[0][0] = binalInt(pTransform->xx * BINAL_MUL);
						mtx[0][1] = binalInt(pTransform->xy * BINAL_MUL);
						mtx[1][0] = binalInt(pTransform->yx * BINAL_MUL);
						mtx[1][1] = binalInt(pTransform->yy * BINAL_MUL);

						//

						BinalCoord src0 = { srcX * (BINAL_MUL / 1024), srcY * (BINAL_MUL / 1024) };
						CoordI dest = { dstX / 64, dstY / 64 };

						_forTintSpans(patch, m_bBlitTintRuns, [&](const RectI& sub)
						{
							CoordI	patchOfs = sub.pos() - dest;
							BinalCoord src = src0;

							src.x += patchOfs.x * mtx[0][0] + patchOfs.y * mtx[1][0];
							src.y += patchOfs.x * mtx[0][1] + patchOfs.y * mtx[1][1];

							uint8_t * pDst = seg.pBuffer + (sub.y-seg.rect.y) * seg.pitch + (sub.x - seg.rect.x) * m_canvasPixelBytes;

							if( cmd == Command::Blit)
								(this->*m_pLinearTransformBlitOp)(pDst, seg.pitch, sub.w, sub.h, src, mtx, sub.pos(), m_pTransformBlitFirstPassOp);
							else if (cmd == Command::ClipBlit)
								(this->*m_pLinearTransformClipBlitOp)(pDst, seg.pitch, sub.w, sub.h, src, mtx, sub.pos(), m_pTransformClipBlitFirstPassOp);
							else if (cmd == Command::Tile)
								(this->*m_pLinearTransformTileOp)(pDst, seg.pitch, sub.w, sub.h, src, mtx, sub.pos(), m_pTransformTileFirstPassOp);
							else
								(this->*m_pLinearTransformBlurOp)(pDst, seg.pitch, sub.w, sub.h, src, mtx, sub.pos(), m_pTransformBlurFirstPassOp);
						});
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
		m_pObjectsPtr = pObjects;
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

		if( m_pTransformClipBlitOp == &SoftBackend::_onePassTransformBlit )
			m_pLinearTransformClipBlitOp = &LinearBackend::_onePassLinearTransformBlit;
		else if( m_pTransformClipBlitOp == &SoftBackend::_twoPassTransformBlit )
			m_pLinearTransformClipBlitOp = &LinearBackend::_twoPassLinearTransformBlit;
		else
			m_pLinearTransformClipBlitOp = &LinearBackend::_dummyLinearTransformBlit;


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
