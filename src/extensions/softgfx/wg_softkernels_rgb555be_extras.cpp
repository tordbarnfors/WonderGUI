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

#include <wg_softkernels_rgb555be_extras.h>
#include <wg_c_softgfx.h>

using namespace wg;

//____ _fill_rgb555bigendian_noblend_notint() ____________________________________________________________

static void _fill_rgb555bigendian_noblend_notint(uint8_t* pDst, int pitchX, int pitchY, int nLines, int lineLength, HiColor col, const SoftBackend::ColTrans& tint, CoordI patchPos)
{
	// Generate our 32-bit double-pixel.
	
	const uint8_t* pPackTab = HiColor::packLinearTab;

	int16_t b = pPackTab[col.b];
	int16_t g = pPackTab[col.g];
	int16_t r = pPackTab[col.r];
	
	uint8_t lowerByte = (b & 0xF8) | (g >> 5);
	uint8_t higherByte = ((g & 0x18) << 3) | (r >> 3);
	
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

//____ _straight_blit_rgb555bigendian_to_same_notint_noblend() ____________________________________________________________

static void _straight_blit_rgb555bigendian_to_same_notint_noblend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
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

//____ _straight_blit_index8linear_to_rgb555bigendian_notint_noblend() ____________________________________________________________

static void _straight_blit_index8linear_to_rgb555bigendian_notint_noblend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{
	auto pPalette = pSrcSurf->palette();
	
	// Copy loop
	
	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			uint32_t col = ((uint32_t*)pPalette)[*pSrc];

			uint16_t out = ((col >> 19) & 0x1F) | ((col >> 5) & 0x7C0) | ((col & 0xF8) << 8);
			out = (out >> 8 | out << 8);
			* (uint16_t*)pDst = out;
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}

//____ _straight_blit_index8linear_to_rgb555bigendian_notint_blend() ____________________________________________________________

static void _straight_blit_index8linear_to_rgb555bigendian_notint_blend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
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
	
			back = (back >> 8 | back << 8);
			int backR = back & 0x1F;
			int backG = (back >> 6) & 0x1F;
			int backB = back >> 11;
						
			int outB = (backB * invAlpha + col.b * (alpha>>3)) >> 16;
			int outG = (backG * invAlpha + col.g * (alpha>>3)) >> 16;
			int outR = (backR * invAlpha + col.r * (alpha>>3)) >> 16;
			
			uint16_t out = outR | (outG << 6) | (outB << 11 );
			out = (out >> 8 | out << 8);

			* (uint16_t*)pDst = out;
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}



//____ _straight_blit_alpha8_to_rgb555bigendian_notint_blend() ____________________________________________________________

static void _straight_blit_alpha8_to_rgb555bigendian_notint_blend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{
	// Copy loop
	
	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			int alpha = SoftBackend::s_mulTab[* pSrc];
			int invAlpha = 65536 - alpha;

			uint16_t back = * ((uint16_t*)pDst);
	
			back = (back >> 8 | back << 8);
			int backR = back & 0x1F;
			int backG = (back >> 6) & 0x1F;
			int backB = back >> 11;
						
			int outB = (backB * invAlpha + 31 * alpha) >> 16;
			int outG = (backG * invAlpha + 31 * alpha) >> 16;
			int outR = (backR * invAlpha + 31 * alpha) >> 16;
			
			uint16_t out = outR | (outG << 6) | (outB << 11 );
			out = (out >> 8 | out << 8);

			* (uint16_t*)pDst = out;
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}




//____ _straight_blit_alpha8_to_rgb555bigendian_flattint_blend() ____________________________________________________________

static void _straight_blit_alpha8_to_rgb555bigendian_flattint_blend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{

	uint8_t srcR = HiColor::packLinearTab[tint.flatTintColor.r] >> 3;
	uint8_t srcG = HiColor::packLinearTab[tint.flatTintColor.g] >> 3;
	uint8_t srcB = HiColor::packLinearTab[tint.flatTintColor.b] >> 3;
	
	// Copy loop
	
	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			int alpha = (SoftBackend::s_mulTab[* pSrc] * tint.flatTintColor.a) >> 12;
			int invAlpha = 65536 - alpha;

			uint16_t back = * ((uint16_t*)pDst);
	
			back = (back >> 8 | back << 8);
			int backR = back & 0x1F;
			int backG = (back >> 6) & 0x1F;
			int backB = back >> 11;
						
			
			int outB = (backB * invAlpha + srcB * alpha) >> 16;
			int outG = (backG * invAlpha + srcG * alpha) >> 16;
			int outR = (backR * invAlpha + srcR * alpha) >> 16;
			
			uint16_t out = outR | (outG << 6) | (outB << 11 );
			out = (out >> 8 | out << 8);

			* (uint16_t*)pDst = out;
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}

//____ _straight_blit_bgrxa8linear_to_rgb555bigendian_notint_noblend() ____________________________________________________________

static void _straight_blit_bgrxa8linear_to_rgb555bigendian_notint_noblend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
{
	
	// Copy loop
	
	for (int y = 0; y < nLines; y++)
	{
		for (int x = 0; x < lineLength; x++)
		{
			uint32_t col = *((uint32_t*)pSrc);

			uint16_t out = ((col >> 19) & 0x1F) | ((col >> 5) & 0x7C0) | ((col & 0xF8) << 8);
			out = (out >> 8 | out << 8);
			* (uint16_t*)pDst = out;

			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}

//____ _straight_blit_bgra8linear_to_rgb555bigendian_notint_blend() ____________________________________________________________

static void _straight_blit_bgra8linear_to_rgb555bigendian_notint_blend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
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
	
			back = (back >> 8 | back << 8);
			int backR = back & 0x1F;
			int backG = (back >> 6) & 0x1F;
			int backB = back >> 11;
						
			int outB = (backB * invAlpha + col.b * (alpha>>3)) >> 16;
			int outG = (backG * invAlpha + col.g * (alpha>>3)) >> 16;
			int outR = (backR * invAlpha + col.r * (alpha>>3)) >> 16;
			
			uint16_t out = outR | (outG << 6) | (outB << 11 );
			out = (out >> 8 | out << 8);

			* (uint16_t*)pDst = out;
			pSrc += pitches.srcX;
			pDst += pitches.dstX;
		}

		pSrc += pitches.srcY;
		pDst += pitches.dstY;
	}
}


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

	for( int i = 0 ; i < 256 ; i++ )
	{
		int alpha = (fgCol.a * i) / 4096;
		int invAlpha = 255 - alpha;

		int outB = (backB * invAlpha + fgCol.b * alpha) >> 15;
		int outG = (backG * invAlpha + fgCol.g * alpha) >> 15;
		int outR = (backR * invAlpha + fgCol.r * alpha) >> 15;
		
		uint16_t out = outR | (outG << 6) | (outB << 11 );
		out = (out >> 8 | out << 8);
		s_fixedBlendCache[i] = out;
	}
}



//____ _straight_blit_alpha8_to_rgb555bigendian_notint_fixedblend() ____________________________________________________________

static void _straight_blit_alpha8_to_rgb555bigendian_no_or_flat_tint_fixedblend(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
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


//____ wg_addExtraSoftKernelsForRGB555BECanvas() __________________________________

#ifdef __cplusplus
extern "C" {
#endif
int	wg_addExtraSoftKernelsForRGB555BECanvas( wg_obj backend )
{
	auto pBackend = static_cast<SoftBackend*>(reinterpret_cast<Object*>(backend));

	return addExtraSoftKernelsForRGB555BECanvas(pBackend);
}
#ifdef __cplusplus
}
#endif

//____ addExtraSoftKernelsForRGB555BECanvas() __________________________________

bool wg::addExtraSoftKernelsForRGB555BECanvas( SoftBackend * pBackend )
{
	pBackend->setFillKernel( TintMode::None, BlendMode::Replace, PixelFormat::RGB_555_bigendian, &_fill_rgb555bigendian_noblend_notint );

	
	pBackend->setStraightBlitKernel( PixelFormat::RGB_555_bigendian, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::RGB_555_bigendian, _straight_blit_rgb555bigendian_to_same_notint_noblend );

	
	pBackend->setStraightBlitKernel( PixelFormat::Index_8_linear, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::RGB_555_bigendian, _straight_blit_index8linear_to_rgb555bigendian_notint_noblend );

	pBackend->setStraightBlitKernel( PixelFormat::Index_8_linear, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Blend, PixelFormat::RGB_555_bigendian, _straight_blit_index8linear_to_rgb555bigendian_notint_blend );
	
	
	pBackend->setStraightBlitKernel( PixelFormat::Alpha_8, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Blend, PixelFormat::RGB_555_bigendian, _straight_blit_alpha8_to_rgb555bigendian_notint_blend );

	pBackend->setStraightBlitKernel( PixelFormat::Alpha_8, SoftBackend::ReadOp::Normal, TintMode::Flat, BlendMode::Blend, PixelFormat::RGB_555_bigendian, _straight_blit_alpha8_to_rgb555bigendian_flattint_blend );

	
	pBackend->setStraightBlitKernel( PixelFormat::Alpha_8, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::BlendFixedColor, PixelFormat::RGB_555_bigendian, _straight_blit_alpha8_to_rgb555bigendian_no_or_flat_tint_fixedblend );

	pBackend->setStraightBlitKernel( PixelFormat::Alpha_8, SoftBackend::ReadOp::Normal, TintMode::Flat, BlendMode::BlendFixedColor, PixelFormat::RGB_555_bigendian, _straight_blit_alpha8_to_rgb555bigendian_no_or_flat_tint_fixedblend );

	
	pBackend->setStraightBlitKernel( PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::RGB_555_bigendian, _straight_blit_bgrxa8linear_to_rgb555bigendian_notint_noblend );

	pBackend->setStraightBlitKernel( PixelFormat::BGRX_8_linear, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::RGB_555_bigendian, _straight_blit_bgrxa8linear_to_rgb555bigendian_notint_noblend );
	
	pBackend->setStraightBlitKernel( PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Blend, PixelFormat::RGB_555_bigendian, _straight_blit_bgra8linear_to_rgb555bigendian_notint_blend );
	
	return true;
};

