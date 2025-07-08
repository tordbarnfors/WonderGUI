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

#include <wg_softkernels_bgr565srgb_extras.h>
#include <wg_c_softgfx.h>

using namespace wg;

//____ _fill_bgr565srgb_noblend_notint() ____________________________________________________________

static void _fill_bgr565srgb_noblend_notint(uint8_t* pDst, int pitchX, int pitchY, int nLines, int lineLength, HiColor col, const SoftBackend::ColTrans& tint, CoordI patchPos)
{
	// Generate our 32-bit double-pixel.

	const uint8_t* pPackTab = HiColor::packSRGBTab;

	int16_t b = pPackTab[col.b];
	int16_t g = pPackTab[col.g];
	int16_t r = pPackTab[col.r];


	uint8_t higherByte = (r & 0xF8) | (g >> 5);
	uint8_t lowerByte = ((g & 0x1C) << 3) | (b >> 3);

	uint32_t doublePixel = lowerByte | (uint32_t(higherByte) << 8);
	doublePixel |= doublePixel << 16;

	// Calculate number of loops and if we need extra start/end writes

	int loops = lineLength;
	bool bStartSingle = false;

	if( (intptr_t(pDst) & 0x3) != 0 )
	{
		bStartSingle = true;
		loops--;
	}

	bool bEndSingle = loops % 2;
	loops /= 2;

	// Fill loop

	for (int y = 0; y < nLines; y++)
	{
		if( bStartSingle )
		{
			* ((uint16_t*)pDst) = (uint16_t) doublePixel;
			pDst += 2;
		}

		for (int x = 0; x < loops; x++)
		{
			* ((uint32_t*)pDst) = doublePixel;
			pDst += 4;
		}
		if( bEndSingle )
		{
			* ((uint16_t*)pDst) = (uint16_t) doublePixel;
			pDst += 2;
		}

		pDst += pitchY;
	}
}

//____ _straight_blit_bgr565srgb_to_same_notint_noblend() ____________________________________________________________

static void _straight_blit_bgr565srgb_to_same_notint_noblend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{
	if( pitches.srcX == 2 && pitches.dstX == 2 )
	{
		if((intptr_t(pSrc) & 0x3) == (intptr_t(pDst) & 0x3))
		{
			// Calculate number of loops and if we need extra start/end writes

			int loops = lineLength;
			bool bStartSingle = false;

			if( (intptr_t(pDst) & 0x3) != 0 )
			{
				bStartSingle = true;
				loops--;
			}

			bool bEndSingle = loops % 2;
			loops /= 2;

			for (int y = 0; y < nLines; y++)
			{
				if( bStartSingle )
				{
					* ((uint16_t*)pDst) = * ((uint16_t*)pSrc);
					pSrc += 2;
					pDst += 2;
				}

				for (int x = 0; x < loops; x++)
				{
					* ((uint32_t*)pDst) = * ((uint32_t*) pSrc);
					pSrc += 4;
					pDst += 4;
				}

				if( bEndSingle )
				{
					* ((uint16_t*)pDst) = * ((uint16_t*)pSrc);
					pSrc += 2;
					pDst += 2;
				}

				pSrc += pitches.srcY;
				pDst += pitches.dstY;
			}


		}
		else
		{
			for (int y = 0; y < nLines; y++)
			{
				for (int x = 0; x < lineLength; x++)
				{
					* ((uint16_t*)pDst) = * ((uint16_t*) pSrc);
					pSrc += 2;
					pDst += 2;
				}

				pSrc += pitches.srcY;
				pDst += pitches.dstY;
			}
		}
	}
	else
	{
		for (int y = 0; y < nLines; y++)
		{
			for (int x = 0; x < lineLength; x++)
			{
				* ((uint16_t*)pDst) = * ((uint16_t*) pSrc);
				pSrc += pitches.srcX;
				pDst += pitches.srcY;
			}

			pSrc += pitches.srcY;
			pDst += pitches.dstY;
		}
	}
}

//____ _straight_blit_index8srgb_to_bgr565srgb_notint_noblend() ____________________________________________________________

static void _straight_blit_index8srgb_to_bgr565srgb_notint_noblend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{
	auto pPalette = pSrcSurf->palette();

	// Copy loop

	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			uint32_t col = ((uint32_t*)pPalette)[*pSrc];

			uint16_t out = ((col >> 3) & 0x001F) | ((col >> 5) & 0x07E0) | ((col >> 8 ) & 0xF800);
			* (uint16_t*)pDst = out;
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}

//____ _straight_blit_index8srgb_to_bgr565srgb_notint_blend() ____________________________________________________________
/*
static void _straight_blit_index8srgb_to_bgr565srgb_notint_blend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{
	auto pPalette = pSrcSurf->palette();

	// Copy loop

	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			Color8 col = pPalette[*pSrc];

			int alpha = SoftBackend::s_mulTab[col.a];
			int invAlpha = 65536 - alpha;

			uint16_t back = * ((uint16_t*)pDst);

			int backR = (back >> 11);
			int backG = (back >> 5) & 0x3F;
			int backB = back & 0x1F;

			int outB = (backB * invAlpha + col.b * (alpha>>3)) >> 16;
			int outG = (backG * invAlpha + col.g * (alpha>>2)) >> 16;
			int outR = (backR * invAlpha + col.r * (alpha>>3)) >> 16;

			uint16_t out = outB | (outG << 5) | (outR << 11 );

			* (uint16_t*)pDst = out;
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}
*/


//____ _straight_blit_alpha8_to_bgr565srgb_notint_blend() ____________________________________________________________
/*
static void _straight_blit_alpha8_to_bgr565srgb_notint_blend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{
	// Copy loop

	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			int alpha = SoftBackend::s_mulTab[* pSrc];
			int invAlpha = 65536 - alpha;

			uint16_t back = * ((uint16_t*)pDst);

			int backR = (back >> 11);
			int backG = (back >> 5) & 0x3F;
			int backB = back & 0x1F;

			int outB = (backB * invAlpha + 31 * alpha) >> 16;
			int outG = (backG * invAlpha + 63 * alpha) >> 16;
			int outR = (backR * invAlpha + 31 * alpha) >> 16;

			uint16_t out = outB | (outG << 5) | (outR << 11 );

			* (uint16_t*)pDst = out;
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}
*/



//____ _straight_blit_alpha8_to_bgr565srgb_flattint_blend() ____________________________________________________________
/*
static void _straight_blit_alpha8_to_bgr565srgb_flattint_blend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{

	uint8_t srcR = HiColor::packLinearTab[tint.flatTintColor.r] >> 3;
	uint8_t srcG = HiColor::packLinearTab[tint.flatTintColor.g] >> 2;
	uint8_t srcB = HiColor::packLinearTab[tint.flatTintColor.b] >> 3;

	// Copy loop

	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			int alpha = (SoftBackend::s_mulTab[* pSrc] * tint.flatTintColor.a) >> 12;
			int invAlpha = 65536 - alpha;

			uint16_t back = * ((uint16_t*)pDst);

			int backR = (back >> 11);
			int backG = (back >> 5) & 0x3F;
			int backB = back & 0x1F;

			int outB = (backB * invAlpha + srcB * alpha) >> 16;
			int outG = (backG * invAlpha + srcG * alpha) >> 16;
			int outR = (backR * invAlpha + srcR * alpha) >> 16;

			uint16_t out = outB | (outG << 5) | (outR << 11 );

			* (uint16_t*)pDst = out;
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}
*/

//____ _straight_blit_bgrxa8srgb_to_bgr565srgb_notint_noblend() ____________________________________________________________

static void _straight_blit_bgrxa8srgb_to_bgr565srgb_notint_noblend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{

	// Copy loop

	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			uint32_t col = *((uint32_t*)pSrc);

			uint16_t out = ((col >> 3) & 0x001F) | ((col >> 5) & 0x07E0) | ((col >> 8 ) & 0xF800);
			* (uint16_t*)pDst = out;

			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}

//____ _straight_blit_bgra8srgb_to_bgr565srgb_notint_blend() ____________________________________________________________
/*
static void _straight_blit_bgra8srgb_to_bgr565srgb_notint_blend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{

	// Copy loop

	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			Color8 col = * (Color8*) pSrc;

			int alpha = SoftBackend::s_mulTab[col.a];
			int invAlpha = 65536 - alpha;

			uint16_t back = * ((uint16_t*)pDst);

			int backR = (back >> 11);
			int backG = (back >> 5) & 0x3F;
			int backB = back & 0x1F;

			int outB = (backB * invAlpha + col.b * (alpha>>3)) >> 16;
			int outG = (backG * invAlpha + col.g * (alpha>>2)) >> 16;
			int outR = (backR * invAlpha + col.r * (alpha>>3)) >> 16;

			uint16_t out = outB | (outG << 5) | (outR << 11 );

			* (uint16_t*)pDst = out;
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}
*/

static uint16_t s_fixedBlendCache[256];
static HiColor	s_fixedBlendCacheFgColor = HiColor::Undefined;
static HiColor	s_fixedBlendCacheBgColor = HiColor::Undefined;

//____ updateFixedBlendCache() ___________________________________________

static void updateFixedBlendCache( HiColor bgCol, HiColor fgCol )
{
	s_fixedBlendCacheBgColor = bgCol;
	s_fixedBlendCacheFgColor = bgCol;

	int backB = bgCol.b;
	int backG = bgCol.g;
	int backR = bgCol.r;

	int frontB = fgCol.b;
	int frontG = fgCol.g;
	int frontR = fgCol.r;

	for( int i = 0 ; i < 256 ; i++ )
	{
		int alpha = (fgCol.a * i) / 255;
		int invAlpha = 4096 - alpha;

		int outB = HiColor::packSRGBTab[(backB * invAlpha + frontB * alpha)/4096] >> 3;
		int outG = HiColor::packSRGBTab[(backG * invAlpha + frontG * alpha)/4096] >> 2;
		int outR = HiColor::packSRGBTab[(backR * invAlpha + frontR * alpha)/4096] >> 3;

		uint16_t out = outB | (outG << 5) | (outR << 11 );
		s_fixedBlendCache[i] = out;
	}
}



//____ _straight_blit_alpha8_to_bgr565srgb_no_or_flat_tint_fixedblend() ____________________________________________________________

static void _straight_blit_alpha8_to_bgr565srgb_no_or_flat_tint_fixedblend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{
	// Setup conversion table

	if( s_fixedBlendCacheBgColor != tint.fixedBlendColor || s_fixedBlendCacheFgColor != tint.flatTintColor )
		updateFixedBlendCache(tint.fixedBlendColor, tint.flatTintColor );

	// Copy loop

	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			* (uint16_t*)pDst = s_fixedBlendCache[* pSrc];
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}


//____ _draw_segment_strip() _______________________________________________

void _draw_segment_strip_blend_to_bgr565srgb(int colBeg, int colEnd, uint8_t* pStripStart, int pixelPitch, int nEdges, 
	SoftBackend::SegmentEdge* pEdges, const int16_t* pSegmentColors, const HiColor* pSegmentTintmap, 
	int segmentTintMapPitch, const bool* pTransparentSegments, const bool* pOpaqueSegments, const SoftBackend::ColTrans& tint)
{
	// Render the column

	int offset = colBeg;				// 24.8 format, but binals cleared (always pointing at beginning of full pixel).
	uint8_t* pDst = pStripStart + (offset >> 8) * pixelPitch;

	while (offset < colEnd)
	{
		if (nEdges == 0 || offset + 255 < pEdges[0].begin)
		{
			// We are fully inside a segment, no need to take any edge into account.

			int end = nEdges == 0 ? colEnd : pEdges[0].begin;

			if (*pTransparentSegments)									// This test is still valid in GRADIENT mode.
			{
				pDst = pStripStart + (end >> 8) * pixelPitch;
				offset = end & 0xFFFFFF00;												// Just skip segment since it is transparent
			}
			else
			{
				int16_t inB, inG, inR, inA;

				inR = pSegmentColors[0];
				inG = pSegmentColors[1];
				inB = pSegmentColors[2];
				inA = pSegmentColors[3];

				if (*pOpaqueSegments)
				{
					uint16_t	out = (HiColor::packSRGBTab[inB] >> 3) | ((HiColor::packSRGBTab[inG] & 0xFC) << 3) |
									  (((HiColor::packSRGBTab[inG] >> 5) | (HiColor::packSRGBTab[inR] & 0xF8)) << 8) ;

					while (offset + 255 < end)
					{
						*((uint16_t*)pDst) = out;
						pDst += pixelPitch;
						offset += 256;
					}
				}
				else
				{
					int alpha = inA;
					int invAlpha = 4096 - alpha;

					int modB = inB * alpha;
					int modG = inG * alpha;
					int modR = inR * alpha;

					int			in = -1;
					uint16_t	out;


					while (offset + 255 < end)
					{
						int16_t backB, backG, backR;
						int16_t outB, outG, outR;

						int pixel = *(uint16_t*)pDst;
						if (pixel != in)
						{
							in = pixel;
#if WG_IS_BIG_ENDIAN
							pixel = Util::endianSwap(pixel);
#endif
							backB = SoftBackend::s_channel_5_sRGB[pixel & 0x1F];
							backG = SoftBackend::s_channel_6_sRGB[(pixel >> 5) & 0x3F];
							backR = SoftBackend::s_channel_5_sRGB[pixel >> 11];

							outB = (backB * invAlpha + modB) >> 12;
							outG = (backG * invAlpha + modG) >> 12;
							outR = (backR * invAlpha + modR) >> 12;

							out = (HiColor::packSRGBTab[outB] >> 3) | ((HiColor::packSRGBTab[outG] & 0xFC) << 3) |
								(((HiColor::packSRGBTab[outG] >> 5) | (HiColor::packSRGBTab[outR] & 0xF8)) << 8);
						}

						*((uint16_t*)pDst) = out;
						pDst += pixelPitch;
						offset += 256;
					}
				}
			}
		}
		else
		{
			{
				int edge = 0;

				int	segmentFractions[SoftBackend::c_maxSegments];
				int remainingFractions = 65536;

				while (edge < nEdges && offset + 255 >= pEdges[edge].begin)
				{
					int frac;				// Fractions of pixel below edge.

					if (offset + 255 < pEdges[edge].end)
					{
						int beginHeight = 256 - (pEdges[edge].begin & 0xFF);
						int coverageInc = (pEdges[edge].coverageInc * beginHeight) >> 8;

						frac = ((pEdges[edge].coverage + coverageInc / 2) * beginHeight) >> 8;

						pEdges[edge].coverage += coverageInc;
						pEdges[edge].begin = offset + 256;
					}
					else
					{
						frac = ((((pEdges[edge].coverage + 65536) / 2) * (pEdges[edge].end - pEdges[edge].begin)) >> 8)
							+ (256 - (pEdges[edge].end & 0xFF)) * (65536 / 256);
					}

					frac = std::min(frac, remainingFractions);

					segmentFractions[edge] = remainingFractions - frac;

					int oldRemaining = remainingFractions;

					remainingFractions = frac;
					edge++;
				}

				segmentFractions[edge] = remainingFractions;

				int16_t backB, backG, backR;

				uint16_t pixel = *(uint16_t*)pDst;
#if WG_IS_BIG_ENDIAN
				pixel = Util::endianSwap(pixel);
#endif
				backB = SoftBackend::s_channel_5_sRGB[pixel & 0x1F];
				backG = SoftBackend::s_channel_6_sRGB[(pixel >> 5) & 0x3F];
				backR = SoftBackend::s_channel_5_sRGB[pixel >> 11];

				int16_t outB = 0, outG = 0, outR = 0;

				int accB = 0;
				int accG = 0;
				int accR = 0;
				int accA = 0;

				int backFraction = 65536;

				for (int i = 0; i <= edge; i++)
				{
					int alpha = pSegmentColors[i * 4 + 3];
					int blendFraction = ((segmentFractions[i] * alpha) / 4096);
					backFraction -= blendFraction;

					accR += blendFraction * pSegmentColors[i*4+0];
					accG += blendFraction * pSegmentColors[i*4+1];
					accB += blendFraction * pSegmentColors[i*4+2];
				}

				outB = (accB + (backB * backFraction)) >> 16;
				outG = (accG + (backG * backFraction)) >> 16;
				outR = (accR + (backR * backFraction)) >> 16;

				pDst[0] = (HiColor::packSRGBTab[outB] >> 3) | ((HiColor::packSRGBTab[outG] & 0xFC) << 3);
				pDst[1] = (HiColor::packSRGBTab[outG] >> 5) | (HiColor::packSRGBTab[outR] & 0xF8);
			}
			pDst += pixelPitch;
			offset += 256;
		}

		while (nEdges > 0 && offset >= pEdges[0].end)
		{
			pEdges++;
			nEdges--;

			pTransparentSegments++;
			pOpaqueSegments++;
			pSegmentColors += 4;

		}
	}
}



//____ wg_addExtraSoftKernelsForBGR565sRGBCanvas() __________________________________

#ifdef __cplusplus
extern "C" {
#endif
int	wg_addExtraSoftKernelsForBGR565sRGBCanvas( wg_obj backend )
{
	auto pBackend = static_cast<SoftBackend*>(reinterpret_cast<Object*>(backend));

	return addExtraSoftKernelsForBGR565sRGBCanvas(pBackend);
}
#ifdef __cplusplus
}
#endif

//____ addExtraSoftKernelsForBGR565sRGBCanvas() __________________________________

bool wg::addExtraSoftKernelsForBGR565sRGBCanvas( SoftBackend * pBackend )
{
	pBackend->setFillKernel( TintMode::None, BlendMode::Replace, PixelFormat::BGR_565_sRGB, &_fill_bgr565srgb_noblend_notint );

	pBackend->setStraightBlitKernel( PixelFormat::BGR_565_sRGB, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::BGR_565_sRGB, _straight_blit_bgr565srgb_to_same_notint_noblend );


	pBackend->setStraightBlitKernel( PixelFormat::Index_8_sRGB, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::BGR_565_sRGB, _straight_blit_index8srgb_to_bgr565srgb_notint_noblend );
/*
	pBackend->setStraightBlitKernel( PixelFormat::Index_8_sRGB, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Blend, PixelFormat::BGR_565_sRGB, _straight_blit_index8sRGB_to_bgr565srgb_notint_blend );


	pBackend->setStraightBlitKernel( PixelFormat::Alpha_8, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Blend, PixelFormat::BGR_565_sRGB, _straight_blit_alpha8_to_bgr565srgb_notint_blend );

	pBackend->setStraightBlitKernel( PixelFormat::Alpha_8, SoftBackend::ReadOp::Normal, TintMode::Flat, BlendMode::Blend, PixelFormat::BGR_565_sRGB, _straight_blit_alpha8_to_bgr565srgb_flattint_blend );
 */

	pBackend->setStraightBlitKernel( PixelFormat::Alpha_8, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::BlendFixedColor, PixelFormat::BGR_565_sRGB, _straight_blit_alpha8_to_bgr565srgb_no_or_flat_tint_fixedblend );

	pBackend->setStraightBlitKernel( PixelFormat::Alpha_8, SoftBackend::ReadOp::Normal, TintMode::Flat, BlendMode::BlendFixedColor, PixelFormat::BGR_565_sRGB, _straight_blit_alpha8_to_bgr565srgb_no_or_flat_tint_fixedblend );


	pBackend->setStraightBlitKernel( PixelFormat::BGRA_8_sRGB, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::BGR_565_sRGB, _straight_blit_bgrxa8srgb_to_bgr565srgb_notint_noblend );
/*
	pBackend->setStraightBlitKernel( PixelFormat::BGRX_8_sRGB, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::BGR_565_sRGB, _straight_blit_bgrxa8srgb_to_bgr565srgb_notint_noblend );

	pBackend->setStraightBlitKernel( PixelFormat::BGRA_8_sRGB, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Blend, PixelFormat::BGR_565_sRGB, _straight_blit_bgra8srgb_to_bgr565srgb_notint_blend );
*/

	pBackend->setSegmentStripKernel(SoftBackend::StripSource::Colors, BlendMode::Blend, PixelFormat::BGR_565_sRGB, _draw_segment_strip_blend_to_bgr565srgb);

	return true;
};

