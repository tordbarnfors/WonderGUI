
#include <wg_softbackend.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

using namespace wg;
using namespace wg::Util;


	
//____ read_pixel_fast8() _______________________________________________________

inline void _read_pixel_fast8(const uint8_t* pPixel, PixelFormat format, const Color8* pPalette, const HiColor* pPalette4096, int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA)
{
	if (format == PixelFormat::Undefined)
	{
		const int16_t* p = (const int16_t*)pPixel;

		outB = HiColor::packLinearTab[p[0]];
		outG = HiColor::packLinearTab[p[1]];
		outR = HiColor::packLinearTab[p[2]];
		outA = HiColor::packLinearTab[p[3]];
	}

	if (format == PixelFormat::BGRA_8_linear || format == PixelFormat::BGRA_8_sRGB)
	{
		outB = pPixel[0];
		outG = pPixel[1];
		outR = pPixel[2];
		outA = pPixel[3];
	}

	if (format == PixelFormat::BGR_8_linear || format == PixelFormat::BGR_8_sRGB || format == PixelFormat::BGRX_8_linear || format == PixelFormat::BGRX_8_sRGB)
	{
		outB = pPixel[0];
		outG = pPixel[1];
		outR = pPixel[2];
		outA = 255;
	}

	if (format == PixelFormat::BGR_565_linear)
	{
		uint16_t pixel = *(uint16_t*)pPixel;
#if WG_IS_BIG_ENDIAN
		pixel = Util::endianSwap(pixel);
#endif
		outB = SoftBackend::s_fast8_channel_5[pixel & 0x1F];
		outG = SoftBackend::s_fast8_channel_6[(pixel >> 5) & 0x3F];
		outR = SoftBackend::s_fast8_channel_5[pixel >> 11];
		outA = 255;
	}

	if (format == PixelFormat::RGB_565_bigendian)
	{
		uint16_t pixel = *(uint16_t*)pPixel;
#if WG_IS_LITTLE_ENDIAN
		pixel = Util::endianSwap(pixel);
#endif
		outR = SoftBackend::s_fast8_channel_5[pixel & 0x1F];
		outG = SoftBackend::s_fast8_channel_6[(pixel >> 5) & 0x3F];
		outB = SoftBackend::s_fast8_channel_5[pixel >> 11];
		outA = 255;
	}

	if (format == PixelFormat::RGB_555_bigendian)
	{
		uint16_t pixel = *(uint16_t*)pPixel;
#if WG_IS_LITTLE_ENDIAN
		pixel = Util::endianSwap(pixel);
#endif
		outR = SoftBackend::s_fast8_channel_5[pixel & 0x1F];
		outG = SoftBackend::s_fast8_channel_5[(pixel >> 6) & 0x1F];
		outB = SoftBackend::s_fast8_channel_5[pixel >> 11];
		outA = 255;
	}

	if (format == PixelFormat::BGRA_4_linear)
	{
		outB = SoftBackend::s_fast8_channel_4_1[pPixel[0]];
		outG = SoftBackend::s_fast8_channel_4_2[pPixel[0]];
		outR = SoftBackend::s_fast8_channel_4_1[pPixel[1]];
		outA = SoftBackend::s_fast8_channel_4_2[pPixel[1]];
	}

	if (format == PixelFormat::Alpha_8)
	{
		outB = 255;
		outG = 255;
		outR = 255;
		outA = *pPixel;
	}

	if (format == PixelFormat::Index_8_sRGB || format == PixelFormat::Index_8_linear)
	{
		Color8 c = pPalette[*pPixel];
		outB = c.b;
		outG = c.g;
		outR = c.r;
		outA = c.a;
	}
}


//____ read_pixel() _______________________________________________________

inline void _read_pixel(const uint8_t* pPixel, PixelFormat format, const Color8* pPalette, const HiColor* pPalette4096, int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA)
{
	if (format == PixelFormat::Undefined)
	{
		const int16_t* p = (const int16_t*)pPixel;

		outB = p[0];
		outG = p[1];
		outR = p[2];
		outA = p[3];
	}

	if (format == PixelFormat::BGRA_8_linear)
	{
		outB = HiColor::unpackLinearTab[pPixel[0]];
		outG = HiColor::unpackLinearTab[pPixel[1]];
		outR = HiColor::unpackLinearTab[pPixel[2]];
		outA = HiColor::unpackLinearTab[pPixel[3]];
	}

	if (format == PixelFormat::BGRA_8_sRGB)
	{
		outB = HiColor::unpackSRGBTab[pPixel[0]];
		outG = HiColor::unpackSRGBTab[pPixel[1]];
		outR = HiColor::unpackSRGBTab[pPixel[2]];
		outA = HiColor::unpackLinearTab[pPixel[3]];
	}

	if (format == PixelFormat::BGR_8_linear || format == PixelFormat::BGRX_8_linear )
	{
		outB = HiColor::unpackLinearTab[pPixel[0]];
		outG = HiColor::unpackLinearTab[pPixel[1]];
		outR = HiColor::unpackLinearTab[pPixel[2]];
		outA = 4096;
	}

	if (format == PixelFormat::BGR_8_sRGB || format == PixelFormat::BGRX_8_sRGB)
	{
		outB = HiColor::unpackSRGBTab[pPixel[0]];
		outG = HiColor::unpackSRGBTab[pPixel[1]];
		outR = HiColor::unpackSRGBTab[pPixel[2]];
		outA = 4096;
	}

	if (format == PixelFormat::BGR_565_linear)
	{
		uint16_t pixel = *(uint16_t*)pPixel;
#if WG_IS_BIG_ENDIAN
		pixel = Util::endianSwap(pixel);
#endif
		outB = SoftBackend::s_channel_5[pixel & 0x1F];
		outG = SoftBackend::s_channel_6[(pixel >> 5) & 0x3F];
		outR = SoftBackend::s_channel_5[pixel >> 11];
		outA = 4096;
	}

	if (format == PixelFormat::RGB_565_bigendian)
	{
		uint16_t pixel = *(uint16_t*)pPixel;
#if WG_IS_LITTLE_ENDIAN
		pixel = Util::endianSwap(pixel);
#endif
		outR = SoftBackend::s_channel_5[pixel & 0x1F];
		outG = SoftBackend::s_channel_6[(pixel >> 5) & 0x3F];
		outB = SoftBackend::s_channel_5[pixel >> 11];
		outA = 4096;
	}

	if (format == PixelFormat::RGB_555_bigendian)
	{
		uint16_t pixel = *(uint16_t*)pPixel;
#if WG_IS_LITTLE_ENDIAN
		pixel = Util::endianSwap(pixel);
#endif
		outR = SoftBackend::s_channel_5[pixel & 0x1F];
		outG = SoftBackend::s_channel_5[(pixel >> 6) & 0x1F];
		outB = SoftBackend::s_channel_5[pixel >> 11];
		outA = 4096;
	}
	
	if (format == PixelFormat::BGRA_4_linear)
	{
		outB = SoftBackend::s_channel_4_1[pPixel[0]];
		outG = SoftBackend::s_channel_4_2[pPixel[0]];
		outR = SoftBackend::s_channel_4_1[pPixel[1]];
		outA = SoftBackend::s_channel_4_2[pPixel[1]];
	}

	if (format == PixelFormat::Alpha_8)
	{
		outB = 4096;
		outG = 4096;
		outR = 4096;
		outA = HiColor::unpackLinearTab[*pPixel];
	}

	if (format == PixelFormat::Index_8_sRGB || format == PixelFormat::Index_8_linear)
	{
		const HiColor* p = &pPalette4096[(*pPixel)];
		outB = p->b;
		outG = p->g;
		outR = p->r;
		outA = p->a;
	}
}

//____ write_pixel_fast8() _______________________________________________________

inline void _write_pixel_fast8(uint8_t* pPixel, PixelFormat format, int16_t b, int16_t g, int16_t r, int16_t a)
{
	//TODO: support BigEndian systems!

	if (format == PixelFormat::Undefined)
	{
		int16_t* p = (int16_t*)pPixel;

		p[0] = HiColor::unpackLinearTab[b];
		p[1] = HiColor::unpackLinearTab[g];
		p[2] = HiColor::unpackLinearTab[r];
		p[3] = HiColor::unpackLinearTab[a];
	}

	if (format == PixelFormat::BGRA_8_linear || format == PixelFormat::BGRA_8_sRGB)
	{
		pPixel[0] = (uint8_t)b;
		pPixel[1] = (uint8_t)g;
		pPixel[2] = (uint8_t)r;
		pPixel[3] = (uint8_t)a;
	}

	if (format == PixelFormat::BGR_8_linear || format == PixelFormat::BGR_8_sRGB || format == PixelFormat::BGRX_8_linear || format == PixelFormat::BGRX_8_sRGB)
	{
		pPixel[0] = (uint8_t)b;
		pPixel[1] = (uint8_t)g;
		pPixel[2] = (uint8_t)r;
	}

	if (format == PixelFormat::BGR_565_linear)
	{
		pPixel[0] = (b >> 3) | ((g & 0xFC) << 3);
		pPixel[1] = (g >> 5) | (r & 0xF8);

	}

	if (format == PixelFormat::RGB_565_bigendian)
	{
		pPixel[0] = (b & 0xF8) | (g >> 5);
		pPixel[1] = ((g & 0x1C) << 3) | (r >> 3);
	}

	if (format == PixelFormat::RGB_555_bigendian)
	{
		pPixel[0] = (b & 0xF8) | (g >> 5);
		pPixel[1] = ((g & 0x18) << 3) | (r >> 3);
	}

	if (format == PixelFormat::BGRA_4_linear)
	{
		pPixel[0] = (b >> 4) | (g & 0xF0);
		pPixel[1] = (r >> 4) | (a & 0xF0);
	}

	if (format == PixelFormat::Alpha_8)
	{
		pPixel[0] = (uint8_t)a;
	}

}


//____ write_pixel() _______________________________________________________

inline void _write_pixel(uint8_t* pPixel, PixelFormat format, int16_t b, int16_t g, int16_t r, int16_t a)
{
	//TODO: support BigEndian systems!

	if (format == PixelFormat::Undefined)
	{
		int16_t* p = (int16_t*)pPixel;

		p[0] = b;
		p[1] = g;
		p[2] = r;
		p[3] = a;
	}

	if (format == PixelFormat::BGRA_8_linear)
	{
		pPixel[0] = HiColor::packLinearTab[b];
		pPixel[1] = HiColor::packLinearTab[g];
		pPixel[2] = HiColor::packLinearTab[r];
		pPixel[3] = HiColor::packLinearTab[a];
	}

	if (format == PixelFormat::BGRA_8_sRGB)
	{
		pPixel[0] = HiColor::packSRGBTab[b];
		pPixel[1] = HiColor::packSRGBTab[g];
		pPixel[2] = HiColor::packSRGBTab[r];
		pPixel[3] = HiColor::packLinearTab[a];
	}

	if (format == PixelFormat::BGR_8_linear || format == PixelFormat::BGRX_8_linear)
	{
		pPixel[0] = HiColor::packLinearTab[b];
		pPixel[1] = HiColor::packLinearTab[g];
		pPixel[2] = HiColor::packLinearTab[r];
	}

	if (format == PixelFormat::BGR_8_sRGB || format == PixelFormat::BGRX_8_sRGB)
	{
		pPixel[0] = HiColor::packSRGBTab[b];
		pPixel[1] = HiColor::packSRGBTab[g];
		pPixel[2] = HiColor::packSRGBTab[r];
	}

	if (format == PixelFormat::BGR_565_linear)
	{
		pPixel[0] = (HiColor::packLinearTab[b] >> 3) | ((HiColor::packLinearTab[g] & 0xFC) << 3);
		pPixel[1] = (HiColor::packLinearTab[g] >> 5) | (HiColor::packLinearTab[r] & 0xF8);
	}

	if (format == PixelFormat::RGB_565_bigendian)
	{
		pPixel[0] = (HiColor::packLinearTab[b] & 0xF8) | (HiColor::packLinearTab[g] >> 5);
		pPixel[1] = ((HiColor::packLinearTab[g] & 0x1C) << 3) | (HiColor::packLinearTab[r] >> 3);
	}

	if (format == PixelFormat::RGB_555_bigendian)
	{
		pPixel[0] = (HiColor::packLinearTab[b] & 0xF8) | (HiColor::packLinearTab[g] >> 5);
		pPixel[1] = ((HiColor::packLinearTab[g] & 0x18) << 3) | (HiColor::packLinearTab[r] >> 3);
	}
	
	if (format == PixelFormat::BGRA_4_linear)
	{
		pPixel[0] = (HiColor::packLinearTab[b] >> 4) | (HiColor::packLinearTab[g] & 0xF0);
		pPixel[1] = (HiColor::packLinearTab[r] >> 4) | (HiColor::packLinearTab[a] & 0xF0);
	}

	if (format == PixelFormat::Alpha_8)
	{
		pPixel[0] = HiColor::packLinearTab[a];
	}
}

//____ _blend_pixels_fast8() ____________________________________________________

inline void	_blend_pixels_fast8(BlendMode mode, int morphFactor, PixelFormat destFormat,
	int16_t srcB, int16_t srcG, int16_t srcR, int16_t srcA,
	int16_t backB, int16_t backG, int16_t backR, int16_t backA,
	int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
	int16_t& fixedB, int16_t& fixedG, int16_t& fixedR, int16_t& fixedA)
{
	// Note: We use if-statements instead of switch/case here since some compilers
	// won't fully eliminate the switch/case statements when this code is inlined
	// into templates where only one BlendMode is possible. With if-statements,
	// the statements are properly detected as never occuring and completely removed.

	if (destFormat == PixelFormat::Alpha_8)
	{
		// Alpha only destination. We treat alpha differently when there is no other channel.

		if (mode == BlendMode::Replace)
			outA = srcA;

		if (mode == BlendMode::Morph)
		{
			int invMorph = 4096 - morphFactor;
			outA = (backA * invMorph + srcA * morphFactor) >> 12;
		}

		if (mode == BlendMode::Blend)
		{
			int alpha = SoftBackend::s_mulTab[srcA];
			int invAlpha = 65536 - alpha;

			outA = (backA * invAlpha + 255 * alpha) >> 16;
		}

		if (mode == BlendMode::BlendFixedColor)
		{
			int alpha = SoftBackend::s_mulTab[srcA];
			int invAlpha = 65536 - alpha;

			outA = (fixedA * invAlpha + 255 * alpha) >> 16;
		}
		
		if (mode == BlendMode::Add)
			outA = limitUint8(backA + srcA);

		if (mode == BlendMode::Subtract)
			outA = limitUint8(backA - srcA);

		if (mode == BlendMode::Multiply)
			outA = (SoftBackend::s_mulTab[backA] * srcA) >> 16;

		if (mode == BlendMode::Min)
			outA = srcA - limitUint8(srcA - backA);

		if (mode == BlendMode::Max)
			outA = backA + limitUint8(srcA - backA);

		if (mode == BlendMode::Invert)
		{
			int srcA2 = SoftBackend::s_mulTab[srcA];
			outA = (srcA2 * (255 - backA) + backA * (65536 - srcA2)) >> 16;
		}
	}
	else
	{
		if (mode == BlendMode::Replace)
		{
			outB = srcB;
			outG = srcG;
			outR = srcR;
			outA = srcA;
		}

		if (mode == BlendMode::Morph)
		{
			int invMorph = 4096 - morphFactor;

			outB = (backB * invMorph + srcB * morphFactor) >> 12;
			outG = (backG * invMorph + srcG * morphFactor) >> 12;
			outR = (backR * invMorph + srcR * morphFactor) >> 12;
			outA = (backA * invMorph + srcA * morphFactor) >> 12;
		}

		if (mode == BlendMode::Blend)
		{
			int alpha = SoftBackend::s_mulTab[srcA];
			int invAlpha = 65536 - alpha;

			outB = (backB * invAlpha + srcB * alpha) >> 16;
			outG = (backG * invAlpha + srcG * alpha) >> 16;
			outR = (backR * invAlpha + srcR * alpha) >> 16;
			outA = (backA * invAlpha + 255 * alpha) >> 16;
		}

		if (mode == BlendMode::BlendFixedColor)
		{
			int alpha = SoftBackend::s_mulTab[srcA];
			int invAlpha = 65536 - alpha;

			outB = (fixedB * invAlpha + srcB * alpha) >> 16;
			outG = (fixedG * invAlpha + srcG * alpha) >> 16;
			outR = (fixedR * invAlpha + srcR * alpha) >> 16;
			outA = (fixedA * invAlpha + 255 * alpha) >> 16;
		}
		
		if (mode == BlendMode::Add)
		{
			int alpha = SoftBackend::s_mulTab[srcA];

			outB = limitUint8(backB + (srcB * alpha >> 16));
			outG = limitUint8(backG + (srcG * alpha >> 16));
			outR = limitUint8(backR + (srcR * alpha >> 16));
			outA = backA;
		}

		if (mode == BlendMode::Subtract)
		{
			int alpha = SoftBackend::s_mulTab[srcA];

			outB = limitUint8(backB - (srcB * alpha >> 16));
			outG = limitUint8(backG - (srcG * alpha >> 16));
			outR = limitUint8(backR - (srcR * alpha >> 16));
			outA = backA;
		}

		if (mode == BlendMode::Multiply)
		{
			outB = (SoftBackend::s_mulTab[backB] * srcB) >> 16;
			outG = (SoftBackend::s_mulTab[backG] * srcG) >> 16;
			outR = (SoftBackend::s_mulTab[backR] * srcR) >> 16;
			outA = backA;
		}

		if (mode == BlendMode::Min)
		{
			outB = srcB - limitUint8(srcB - backB);
			outG = srcG - limitUint8(srcG - backG);
			outR = srcR - limitUint8(srcR - backR);
			outA = backA;
		}

		if (mode == BlendMode::Max)
		{
			outB = backB + limitUint8(srcB - backB);
			outG = backG + limitUint8(srcG - backG);
			outR = backR + limitUint8(srcR - backR);
			outA = backA;
		}

		if (mode == BlendMode::Invert)
		{
			int srcB2 = SoftBackend::s_mulTab[srcB];
			int srcG2 = SoftBackend::s_mulTab[srcG];
			int srcR2 = SoftBackend::s_mulTab[srcR];

			outB = (srcB2 * (255 - backB) + backB * (65536 - srcB2)) >> 16;
			outG = (srcG2 * (255 - backG) + backG * (65536 - srcG2)) >> 16;
			outR = (srcR2 * (255 - backR) + backR * (65536 - srcR2)) >> 16;
			outA = backA;
		}
	}
}


//____ _blend_pixels() ____________________________________________________

inline void	_blend_pixels(BlendMode mode, int morphFactor, PixelFormat destFormat,
	int16_t srcB, int16_t srcG, int16_t srcR, int16_t srcA,
	int16_t backB, int16_t backG, int16_t backR, int16_t backA,
	int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
	int16_t& fixedB, int16_t& fixedG, int16_t& fixedR, int16_t& fixedA )
{
	// Note: We use if-statements instead of switch/case here since some compilers
	// won't fully eliminate the switch/case statements when this code is inlined
	// into templates where only one BlendMode is possible. With if-statements,
	// the statements are properly detected as never occuring and completely removed.

	if (destFormat == PixelFormat::Alpha_8)
	{
		// Alpha only destination. We treat alpha differently when there is no other channel.

		if (mode == BlendMode::Replace)
			outA = srcA;

		if (mode == BlendMode::Morph)
		{
			int invMorph = 4096 - morphFactor;
			outA = (backA * invMorph + srcA * morphFactor) >> 12;
		}

		if (mode == BlendMode::Blend)
		{
			int alpha = srcA;
			int invAlpha = 4096 - alpha;

			outA = (backA * invAlpha + 4096 * alpha) >> 12;
		}

		if (mode == BlendMode::BlendFixedColor)
		{
			int alpha = srcA;
			int invAlpha = 4096 - alpha;

			outA = (fixedA * invAlpha + 4096 * alpha) >> 12;
		}
		
		if (mode == BlendMode::Add)
			outA = SoftBackend::s_limit4096Tab[4097 + backA + srcA];

		if (mode == BlendMode::Subtract)
			outA = SoftBackend::s_limit4096Tab[4097 + backA - srcA];

		if (mode == BlendMode::Multiply)
			outA = (backA * srcA) >> 12;

		if (mode == BlendMode::Min)
			outA = std::min(backA, srcA);

		if (mode == BlendMode::Max)
			outA = std::max(backA, srcA);

		if (mode == BlendMode::Invert)
			outA = (srcA * (4096 - backA) + backA * (4096 - srcA)) >> 12;

	}
	else
	{
		if (mode == BlendMode::Replace)
		{
			outB = srcB;
			outG = srcG;
			outR = srcR;
			outA = srcA;
		}

		if (mode == BlendMode::Morph)
		{
			int invMorph = 4096 - morphFactor;

			outB = (backB * invMorph + srcB * morphFactor) >> 12;
			outG = (backG * invMorph + srcG * morphFactor) >> 12;
			outR = (backR * invMorph + srcR * morphFactor) >> 12;
			outA = (backA * invMorph + srcA * morphFactor) >> 12;
		}

		if (mode == BlendMode::Blend)
		{
			int alpha = srcA;
			int invAlpha = 4096 - alpha;

			outB = (backB * invAlpha + srcB * alpha) >> 12;
			outG = (backG * invAlpha + srcG * alpha) >> 12;
			outR = (backR * invAlpha + srcR * alpha) >> 12;
			outA = (backA * invAlpha + 4096 * alpha) >> 12;
		}

		if (mode == BlendMode::BlendFixedColor)
		{
			int alpha = srcA;
			int invAlpha = 4096 - alpha;

			outB = (fixedB * invAlpha + srcB * alpha) >> 12;
			outG = (fixedG * invAlpha + srcG * alpha) >> 12;
			outR = (fixedR * invAlpha + srcR * alpha) >> 12;
			outA = (fixedA * invAlpha + 4096 * alpha) >> 12;
		}
		
		if (mode == BlendMode::Add)
		{
			int alpha = srcA;

			outB = SoftBackend::s_limit4096Tab[4097 + backB + (srcB * alpha >> 12)];
			outG = SoftBackend::s_limit4096Tab[4097 + backG + (srcG * alpha >> 12)];
			outR = SoftBackend::s_limit4096Tab[4097 + backR + (srcR * alpha >> 12)];
			outA = backA;
		}

		if (mode == BlendMode::Subtract)
		{
			int alpha = srcA;

			outB = SoftBackend::s_limit4096Tab[4097 + backB - (srcB * alpha >> 12)];
			outG = SoftBackend::s_limit4096Tab[4097 + backG - (srcG * alpha >> 12)];
			outR = SoftBackend::s_limit4096Tab[4097 + backR - (srcR * alpha >> 12)];
			outA = backA;
		}

		if (mode == BlendMode::Multiply)
		{
			outB = (backB * srcB) >> 12;
			outG = (backG * srcG) >> 12;
			outR = (backR * srcR) >> 12;
			outA = backA;
		}

		if (mode == BlendMode::Min)
		{
			outB = std::min(backB, srcB);
			outG = std::min(backG, srcG);
			outR = std::min(backR, srcR);
			outA = backA;
		}

		if (mode == BlendMode::Max)
		{
			outB = std::max(backB, srcB);
			outG = std::max(backG, srcG);
			outR = std::max(backR, srcR);
			outA = backA;
		}

		if (mode == BlendMode::Invert)
		{
			outB = (srcB * (4096 - backB) + backB * (4096 - srcB)) >> 12;
			outG = (srcG * (4096 - backG) + backG * (4096 - srcG)) >> 12;
			outR = (srcR * (4096 - backR) + backR * (4096 - srcR)) >> 12;
			outA = backA;
		}
	}
}



//____ _color_tint_pixel() ______________________________________________________

inline void _color_tint_pixel(TintMode tintMode, int bits,
	int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
	uint32_t& xIncB, uint32_t& xIncG, uint32_t& xIncR, uint32_t& xIncA,
	uint32_t& pixelB, uint32_t& pixelG, uint32_t& pixelR, uint32_t& pixelA)
{
	if (tintMode == TintMode::GradientX || tintMode == TintMode::GradientXY)
	{
		outB = pixelB >> (18 - bits);
		outG = pixelG >> (18 - bits);
		outR = pixelR >> (18 - bits);
		outA = pixelA >> (18 - bits);

		pixelB += xIncB;
		pixelG += xIncG;
		pixelR += xIncR;
		pixelA += xIncA;

	}
}


//____ _texel_tint_pixel() ________________________________________________

inline void _texel_tint_pixel(TintMode tintMode, int bits, int16_t& pixelB, int16_t& pixelG, int16_t& pixelR, int16_t& pixelA,
	uint32_t& xIncB, uint32_t& xIncG, uint32_t& xIncR, uint32_t& xIncA,
	uint32_t& tintB, uint32_t& tintG, uint32_t& tintR, uint32_t& tintA)
{
	if (tintMode == TintMode::None)
	{
	}
	else
	{
		pixelB = (pixelB * tintB) >> 18;
		pixelG = (pixelG * tintG) >> 18;
		pixelR = (pixelR * tintR) >> 18;
		pixelA = (pixelA * tintA) >> 18;

		if (tintMode == TintMode::GradientX || tintMode == TintMode::GradientXY)
		{
			tintB += xIncB;
			tintG += xIncG;
			tintR += xIncR;
			tintA += xIncA;
		}
	}

}

//____ _plot() ____________________________________________________________

template<BlendMode BLEND, TintMode TINT, PixelFormat DSTFORMAT>
void _plot(uint8_t* pDst, HiColor color, const SoftBackend::ColTrans& tint)
{
	bool bFast8 = false;

	if (DSTFORMAT == PixelFormat::Alpha_8 || DSTFORMAT == PixelFormat::BGRA_4_linear ||
		DSTFORMAT == PixelFormat::BGRA_8_linear || DSTFORMAT == PixelFormat::BGRX_8_linear ||
		DSTFORMAT == PixelFormat::BGR_565_linear || DSTFORMAT == PixelFormat::RGB_565_bigendian ||
		DSTFORMAT == PixelFormat::RGB_555_bigendian ||DSTFORMAT == PixelFormat::BGR_8_linear)
	{
		bFast8 = true;
	}

	// Step 1: Read source pixels

	int16_t srcB, srcG, srcR, srcA;

	if (bFast8)
	{
		const uint8_t* pPackTab = HiColor::packLinearTab;

		srcB = pPackTab[color.b];
		srcG = pPackTab[color.g];
		srcR = pPackTab[color.r];
		srcA = pPackTab[color.a];
	}
	else
	{
		srcB = color.b;
		srcG = color.g;
		srcR = color.r;
		srcA = color.a;
	}

	// Step 1.5: Apply any tint to source

	if (TINT != TintMode::None)
	{
		srcR = (srcR * tint.flatTintColor.r) >> 12;
		srcG = (srcG * tint.flatTintColor.g) >> 12;
		srcB = (srcB * tint.flatTintColor.b) >> 12;
		srcA = (srcA * tint.flatTintColor.a) >> 12;
	}

	// Step 2: Setup fixed blend color
	
	int16_t fixedB, fixedG, fixedR, fixedA;

	if (bFast8)
	{
		const uint8_t* pPackTab = HiColor::packLinearTab;

		fixedB = pPackTab[tint.fixedBlendColor.b];
		fixedG = pPackTab[tint.fixedBlendColor.g];
		fixedR = pPackTab[tint.fixedBlendColor.r];
		fixedA = pPackTab[tint.fixedBlendColor.a];
	}
	else
	{
		fixedB = tint.fixedBlendColor.b;
		fixedG = tint.fixedBlendColor.g;
		fixedR = tint.fixedBlendColor.r;
		fixedA = tint.fixedBlendColor.a;
	}
	
	// Step 3: Get color components of background pixel blending into backX
	// Step 4: Blend srcX and backX into outX
	// Step 5: Write resulting pixel to destination

	int16_t backB, backG, backR, backA;
	int16_t outB, outG, outR, outA;

	if (bFast8)
	{
		_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
		_blend_pixels_fast8(BLEND, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, srcA, backB, backG, backR, backA, outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
		_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
	}
	else
	{
		_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
		_blend_pixels(BLEND, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, srcA, backB, backG, backR, backA, outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
		_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
	}
}

//____ plot_list() ________________________________________________________

template<BlendMode BLEND, TintMode TINT, PixelFormat DSTFORMAT>
void _plot_list(const RectI& clip, int nCoords, const CoordI* pCoords, const HiColor* pColors, uint8_t* pCanvas, int pitchX, int pitchY, const SoftBackend::ColTrans& tint)
{
	bool bFast8 = false;

	if (DSTFORMAT == PixelFormat::Alpha_8 || DSTFORMAT == PixelFormat::BGRA_4_linear ||
		DSTFORMAT == PixelFormat::BGRA_8_linear || DSTFORMAT == PixelFormat::BGRX_8_linear ||
		DSTFORMAT == PixelFormat::BGR_565_linear || DSTFORMAT == PixelFormat::RGB_565_bigendian ||
		DSTFORMAT == PixelFormat::RGB_555_bigendian || DSTFORMAT == PixelFormat::BGR_8_linear)
	{
		bFast8 = true;
	}

	int tintB, tintG, tintR, tintA;

	if (TINT != TintMode::None)
	{
		tintR = tint.flatTintColor.r;
		tintG = tint.flatTintColor.g;
		tintB = tint.flatTintColor.b;
		tintA = tint.flatTintColor.a;
	}

	const uint8_t* pPackTab = HiColor::packLinearTab;

	RectI pixelClip = clip / 64;

	for (int i = 0; i < nCoords; i++)
	{
		const int x = (pCoords[i].x + 32) >> 6;
		const int y = (pCoords[i].y + 32) >> 6;

		if (y >= pixelClip.y && y <= pixelClip.y + pixelClip.h - 1 && x >= pixelClip.x && x <= pixelClip.x + pixelClip.w - 1)
		{
			uint8_t* pDst = pCanvas + y * pitchY + x * pitchX;

			// Step 1: Read source pixels

			int16_t srcB, srcG, srcR, srcA;

			if (bFast8)
			{

				srcB = pPackTab[pColors[i].b];
				srcG = pPackTab[pColors[i].g];
				srcR = pPackTab[pColors[i].r];
				srcA = HiColor::packLinearTab[pColors[i].a];
			}
			else
			{
				srcB = pColors[i].b;
				srcG = pColors[i].g;
				srcR = pColors[i].r;
				srcA = pColors[i].a;
			}

			// Step 1.5: Apply any tint to source

			if (TINT != TintMode::None)
			{
				srcB = (srcB * tintB) >> 12;
				srcG = (srcG * tintG) >> 12;
				srcR = (srcR * tintR) >> 12;
				srcA = (srcA * tintA) >> 12;
			}

			// Step 2: Setup fixed blend color
			
			int16_t fixedB, fixedG, fixedR, fixedA;

			if (bFast8)
			{
				const uint8_t* pPackTab = HiColor::packLinearTab;

				fixedB = pPackTab[tint.fixedBlendColor.b];
				fixedG = pPackTab[tint.fixedBlendColor.g];
				fixedR = pPackTab[tint.fixedBlendColor.r];
				fixedA = HiColor::packLinearTab[tint.fixedBlendColor.a];
			}
			else
			{
				fixedB = tint.fixedBlendColor.b;
				fixedG = tint.fixedBlendColor.g;
				fixedR = tint.fixedBlendColor.r;
				fixedA = tint.fixedBlendColor.a;
			}
			
			// Step 3: Get color components of background pixel blending into backX

			int16_t backB, backG, backR, backA;
			int16_t outB, outG, outR, outA;

			if (bFast8)
			{
				_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels_fast8(BLEND, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
									outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA );
				_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
			}
			else
			{
				_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels(BLEND, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
							  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
			}
		}
	}
}

//____ _draw_line() _______________________________________________________

template<BlendMode BLEND, TintMode TINT, PixelFormat DSTFORMAT>
void _draw_line(uint8_t* pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const SoftBackend::ColTrans& tint, CoordI patchPos)
{
	bool bFast8 = false;

	if (DSTFORMAT == PixelFormat::Alpha_8 || DSTFORMAT == PixelFormat::BGRA_4_linear ||
		DSTFORMAT == PixelFormat::BGRA_8_linear || DSTFORMAT == PixelFormat::BGRX_8_linear ||
		DSTFORMAT == PixelFormat::BGR_565_linear || DSTFORMAT == PixelFormat::RGB_565_bigendian ||
		DSTFORMAT == PixelFormat::RGB_555_bigendian || DSTFORMAT == PixelFormat::BGR_8_linear)
	{
		bFast8 = true;
	}

	const BlendMode EdgeBlendMode = BLEND == BlendMode::Replace ? BlendMode::Blend : BLEND;

	if (BLEND == BlendMode::Replace)
		color.a = 4096;						// Needed since we still blend the edges.

	// Step 1: Read source pixels

	int16_t srcB, srcG, srcR, srcA;

	if (bFast8)
	{
		const uint8_t* pPackTab = HiColor::packLinearTab;

		srcB = pPackTab[color.b];
		srcG = pPackTab[color.g];
		srcR = pPackTab[color.r];
		srcA = HiColor::packLinearTab[color.a];
	}
	else
	{
		srcB = color.b;
		srcG = color.g;
		srcR = color.r;
		srcA = color.a;
	}


	// Step 1.5: Apply any flatTintColor

	if (TINT != TintMode::None)
	{
		srcR = (srcR * tint.flatTintColor.r) >> 12;
		srcG = (srcG * tint.flatTintColor.g) >> 12;
		srcB = (srcB * tint.flatTintColor.b) >> 12;
		srcA = (srcA * tint.flatTintColor.a) >> 12;
	}

	// Step 2: Setup fixed blend color
	
	int16_t fixedB, fixedG, fixedR, fixedA;

	if (bFast8)
	{
		const uint8_t* pPackTab = HiColor::packLinearTab;

		fixedB = pPackTab[tint.fixedBlendColor.b];
		fixedG = pPackTab[tint.fixedBlendColor.g];
		fixedR = pPackTab[tint.fixedBlendColor.r];
		fixedA = HiColor::packLinearTab[tint.fixedBlendColor.a];
	}
	else
	{
		fixedB = tint.fixedBlendColor.b;
		fixedG = tint.fixedBlendColor.g;
		fixedR = tint.fixedBlendColor.r;
		fixedA = tint.fixedBlendColor.a;
	}
	
	//
	
	for (int i = 0; i < length; i++)
	{
		int beg = pos >> 16;
		int end = (pos + width) >> 16;
		int ofs = beg;

		uint8_t* pDst = pRow + ofs * pixelInc;

		if (beg == end)
		{
			// Special case, one pixel wide row

			int alpha = (color.a * width) >> 16;

			int16_t backB, backG, backR, backA;
			int16_t outB, outG, outR, outA;

			if (bFast8)
			{
				_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels_fast8(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
									outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA );
				_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
			}
			else
			{
				_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
							  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
			}
		}
		else
		{
			// First anti-aliased pixel of column

			int alpha = (srcA * (65536 - (pos & 0xFFFF))) >> 16;		// Special AA

			int16_t backB, backG, backR, backA;
			int16_t outB, outG, outR, outA;

			if (bFast8)
			{
				_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels_fast8(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
									outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
			}
			else
			{
				_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
							  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
			}

			ofs++;
			pDst += pixelInc;

			// All non-antialiased middle pixels of column

			if (ofs < end)
			{
				do
				{
					int16_t backB, backG, backR, backA;
					int16_t outB, outG, outR, outA;

					if (bFast8)
					{
						_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
						_blend_pixels_fast8(BLEND, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
											outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
						_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
					}
					else
					{
						_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
						_blend_pixels(BLEND, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
									  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
						_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
					}
					ofs++;
					pDst += pixelInc;

				} while (ofs < end);
			}

			// Last anti-aliased pixel of column

			int overflow = (pos + width) & 0xFFFF;
			if (overflow > 0)
			{
				int alpha = srcA * overflow >> 16;		// Special AA

				int16_t backB, backG, backR, backA;
				int16_t outB, outG, outR, outA;

				if (bFast8)
				{
					_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
					_blend_pixels_fast8(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
										outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
					_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
				}
				else
				{
					_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
					_blend_pixels(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
								  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
					_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
				}

			}
		}

		pRow += rowInc;
		pos += slope;
	}
}

//____ _clip_draw_line() __________________________________________________

template<BlendMode BLEND, TintMode TINT, PixelFormat DSTFORMAT>
void _clip_draw_line(int clipStart, int clipEnd, uint8_t* pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const SoftBackend::ColTrans& tint, CoordI patchPos)
{
	bool bFast8 = false;

	const BlendMode EdgeBlendMode = (BLEND == BlendMode::Replace) ? BlendMode::Blend : BLEND;

	if (BLEND == BlendMode::Replace)
		color.a = 4096;						// Needed since we still blend the edges.

	// Step 1: Read source pixels

	int16_t srcB, srcG, srcR, srcA;

	if (bFast8)
	{
		const uint8_t* pPackTab = HiColor::packLinearTab;

		srcB = pPackTab[color.b];
		srcG = pPackTab[color.g];
		srcR = pPackTab[color.r];
		srcA = HiColor::packLinearTab[color.a];
	}
	else
	{
		srcB = color.b;
		srcG = color.g;
		srcR = color.r;
		srcA = color.a;
	}

	// Step 1.5: Apply any flatTintColor

	if (TINT != TintMode::None)
	{
		srcR = (srcR * tint.flatTintColor.r) >> 12;
		srcG = (srcG * tint.flatTintColor.g) >> 12;
		srcB = (srcB * tint.flatTintColor.b) >> 12;
		srcA = (srcA * tint.flatTintColor.a) >> 12;
	}

	// Step 2: Setup fixed blend color
	
	int16_t fixedB, fixedG, fixedR, fixedA;

	if (bFast8)
	{
		const uint8_t* pPackTab = HiColor::packLinearTab;

		fixedB = pPackTab[tint.fixedBlendColor.b];
		fixedG = pPackTab[tint.fixedBlendColor.g];
		fixedR = pPackTab[tint.fixedBlendColor.r];
		fixedA = HiColor::packLinearTab[tint.fixedBlendColor.a];
	}
	else
	{
		fixedB = tint.fixedBlendColor.b;
		fixedG = tint.fixedBlendColor.g;
		fixedR = tint.fixedBlendColor.r;
		fixedA = tint.fixedBlendColor.a;
	}
	
	//
	
	for (int i = 0; i < length; i++)
	{
		if (pos >= clipEnd || pos + width <= clipStart)
		{
			pRow += rowInc;
			pos += slope;
			continue;
		}

		int clippedPos = pos;
		int clippedWidth = width;

		if (clippedPos < clipStart)
		{
			clippedWidth -= clipStart - clippedPos;
			clippedPos = clipStart;
		}

		if (clippedPos + clippedWidth > clipEnd)
			clippedWidth = clipEnd - clippedPos;


		int beg = clippedPos >> 16;
		int end = (clippedPos + clippedWidth) >> 16;
		int ofs = beg;

		uint8_t* pDst = pRow + ofs * pixelInc;

		if (beg == end)
		{
			// Special case, one pixel wide row


			//TODO: Width might be more than a pixel, needs to be clipped as well!

			int16_t alpha = ((int32_t)color.a * (std::min(int32_t(1 << 16), (int32_t)width))) >> 16;

			int16_t backB, backG, backR, backA;
			int16_t outB, outG, outR, outA;

			if (bFast8)
			{
				_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels_fast8(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
									outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
			}
			else
			{
				_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
							  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
			}
		}
		else
		{
			// First anti-aliased pixel of column

			int alpha = (srcA * (65536 - (clippedPos & 0xFFFF))) >> 16;		// Special AA

			int16_t backB, backG, backR, backA;
			int16_t outB, outG, outR, outA;

			if (bFast8)
			{
				_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels_fast8(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
									outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
			}
			else
			{
				_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
							  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
			}

			ofs++;
			pDst += pixelInc;

			// All non-antialiased middle pixels of column

			if (ofs < end)
			{
				do
				{
					// Step 2: Get color components of background pixel blending into backX
					// Step 3: Blend srcX and backX into outX
					// Step 4: Write resulting pixel to destination

					int16_t backB, backG, backR, backA;
					int16_t outB, outG, outR, outA;

					if (bFast8)
					{
						_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
						_blend_pixels_fast8(BLEND, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
											outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
						_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
					}
					else
					{
						_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
						_blend_pixels(BLEND, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
									  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
						_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
					}

					ofs++;
					pDst += pixelInc;

				} while (ofs < end);
			}

			// Last anti-aliased pixel of column

			int overflow = (clippedPos + clippedWidth) & 0xFFFF;
			if (overflow > 0)
			{
				int alpha = srcA * overflow >> 16;		// Special AA

				int16_t backB, backG, backR, backA;
				int16_t outB, outG, outR, outA;

				if (bFast8)
				{
					_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
					_blend_pixels_fast8(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
										outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
					_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
				}
				else
				{
					_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
					_blend_pixels(EdgeBlendMode, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
								  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
					_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
				}
			}
		}

		pRow += rowInc;
		pos += slope;
	}
}


//____ _fill() ____________________________________________________________

template<TintMode TINT, BlendMode BLEND, PixelFormat DSTFORMAT>
void _fill(uint8_t* pDst, int pitchX, int pitchY, int nLines, int lineLength, HiColor col, const SoftBackend::ColTrans& tint, CoordI patchPos)
{
	bool bFast8 = false;
	int		bits = 12;

	if (DSTFORMAT == PixelFormat::Alpha_8 || DSTFORMAT == PixelFormat::BGRA_4_linear ||
		DSTFORMAT == PixelFormat::BGRA_8_linear || DSTFORMAT == PixelFormat::BGRX_8_linear ||
		DSTFORMAT == PixelFormat::BGR_565_linear || DSTFORMAT == PixelFormat::RGB_565_bigendian ||
		DSTFORMAT == PixelFormat::RGB_555_bigendian || DSTFORMAT == PixelFormat::BGR_8_linear)
	{
		bFast8 = true;
		bits = 8;
	}

	// Step 1: Read source pixels and prepare tint

	uint32_t	leftB, leftG, leftR, leftA;						// Left side colors when tinting Y
	uint32_t	rightB, rightG, rightR, rightA;					// Right side colors when tinting X
	uint32_t	leftIncB, leftIncG, leftIncR, leftIncA;
	uint32_t	rightIncB, rightIncG, rightIncR, rightIncA;
	uint32_t	xIncB, xIncG, xIncR, xIncA;
	uint32_t	pixelB, pixelG, pixelR, pixelA;

	int16_t srcB, srcG, srcR, srcA;
	int16_t tintedB, tintedG, tintedR, tintedA;

	if (bFast8)
	{
		const uint8_t* pPackTab = HiColor::packLinearTab;

		srcB = pPackTab[col.b];
		srcG = pPackTab[col.g];
		srcR = pPackTab[col.r];
		srcA = pPackTab[col.a];
	}
	else
	{
		srcB = col.b;
		srcG = col.g;
		srcR = col.r;
		srcA = col.a;
	}

/*
	_color_tint_init(TINT, tint, bits, srcB, srcG, srcR, srcA, tintedB, tintedG, tintedR, tintedA,
		leftB, leftG, leftR, leftA, rightB, rightG, rightR, rightA,
		leftIncB, leftIncG, leftIncR, leftIncA, rightIncB, rightIncG, rightIncR, rightIncA,
		xIncB, xIncG, xIncR, xIncA, patchPos);
*/

	// Step 2: Setup fixed blend color
	
	int16_t fixedB, fixedG, fixedR, fixedA;

	if (bFast8)
	{
		const uint8_t* pPackTab = HiColor::packLinearTab;

		fixedB = pPackTab[tint.fixedBlendColor.b];
		fixedG = pPackTab[tint.fixedBlendColor.g];
		fixedR = pPackTab[tint.fixedBlendColor.r];
		fixedA = pPackTab[tint.fixedBlendColor.a];
	}
	else
	{
		fixedB = tint.fixedBlendColor.b;
		fixedG = tint.fixedBlendColor.g;
		fixedR = tint.fixedBlendColor.r;
		fixedA = tint.fixedBlendColor.a;
	}
	
	for (int y = 0; y < nLines; y++)
	{

		// Step 3: Apply any vertical tint gradient
/*
		_color_tint_line(TINT, tint, bits, srcB, srcG, srcR, srcA, tintedB, tintedG, tintedR, tintedA,
			leftB, leftG, leftR, leftA, rightB, rightG, rightR, rightA,
			leftIncB, leftIncG, leftIncR, leftIncA, rightIncB, rightIncG, rightIncR, rightIncA,
			xIncB, xIncG, xIncR, xIncA, pixelB, pixelG, pixelR, pixelA, patchPos);
*/

		for (int x = 0; x < lineLength; x++)
		{
			// Step 4: Apply any horizontal tint gradient
/*
			_color_tint_pixel(TINT, bits, tintedB, tintedG, tintedR, tintedA, xIncB, xIncG, xIncR, xIncA,
				pixelB, pixelG, pixelR, pixelA);
*/

			tintedB = srcB;
			tintedG = srcG;
			tintedR = srcR;
			tintedA = srcA;



			// Step 5: Get color components of background pixel blending into backX
			// Step 6: Blend srcX and backX into outX
			// Step 7: Write resulting pixel to destination

			int16_t backB, backG, backR, backA;
			int16_t outB, outG, outR, outA;

			if (bFast8)
			{
				_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels_fast8(BLEND, tint.morphFactor, DSTFORMAT, tintedB, tintedG, tintedR, tintedA, backB, backG, backR, backA,
									outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
			}
			else
			{
				_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels(BLEND, tint.morphFactor, DSTFORMAT, tintedB, tintedG, tintedR, tintedA, backB, backG, backR, backA,
							  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
			}

			// Step 7: Increment destination pointers

			pDst += pitchX;
		}
		pDst += pitchY;
	}
}

//_____ _straight_blit() ____________________________________________________________

template<PixelFormat SRCFORMAT, TintMode TINT, BlendMode BLEND, PixelFormat DSTFORMAT, SoftBackend::ReadOp READOP>
void _straight_blit(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const int simpleTransform[2][2])
{
	bool	bFast8 = false;
	int		bits = 12;

	bool srcIsLinear = (SRCFORMAT == PixelFormat::Alpha_8 || SRCFORMAT == PixelFormat::BGRA_4_linear ||
		SRCFORMAT == PixelFormat::BGRA_8_linear || SRCFORMAT == PixelFormat::BGRX_8_linear ||
		SRCFORMAT == PixelFormat::BGR_565_linear || SRCFORMAT == PixelFormat::RGB_565_bigendian ||
		SRCFORMAT == PixelFormat::RGB_555_bigendian || SRCFORMAT == PixelFormat::BGR_8_linear);

	bool dstIsLinear = (DSTFORMAT == PixelFormat::Alpha_8 || DSTFORMAT == PixelFormat::BGRA_4_linear ||
		DSTFORMAT == PixelFormat::BGRA_8_linear || DSTFORMAT == PixelFormat::BGRX_8_linear ||
		DSTFORMAT == PixelFormat::BGR_565_linear || DSTFORMAT == PixelFormat::RGB_565_bigendian ||
		DSTFORMAT == PixelFormat::RGB_555_bigendian || DSTFORMAT == PixelFormat::BGR_8_linear);

	if ((srcIsLinear && dstIsLinear) || (!srcIsLinear && !dstIsLinear && TINT == TintMode::None && BLEND == BlendMode::Replace))
	{
		if (SRCFORMAT != PixelFormat::Undefined && DSTFORMAT != PixelFormat::Undefined && READOP != SoftBackend::ReadOp::Blur )
		{
			bFast8 = true;
			bits = 8;
		}
	}

	
	// Preapare tiling and blurring

	int srcX, srcY;
	int maskX, maskY;
	int xPitch, yPitch;
	
	int	srcXstart;
	
	if (READOP == SoftBackend::ReadOp::Tile || READOP == SoftBackend::ReadOp::Blur)
	{
		xPitch = pSrcSurf->pixelBits()/8;
		yPitch = pSrcSurf->pitch();

		int ofs = int(pSrc - pSrcSurf->pixels());
		srcY = ofs / yPitch;
		srcX = (ofs % yPitch) / xPitch;
		pSrc = pSrcSurf->pixels();
		maskX = pSrcSurf->tileMaskX();
		maskY = pSrcSurf->tileMaskY();
		
		srcXstart = srcX;
	}

	SizeI surfSize;

	if (READOP == SoftBackend::ReadOp::Blur)
	{
		surfSize = pSrcSurf->pixelSize();
	}
	
	const Color8 * pPalette = pSrcSurf->palette();
	const HiColor * pPalette4096 = pSrcSurf->palette4096();


	// Step 1: Prepare any tint gradient

	uint32_t	tintB, tintG, tintR, tintA;

	uint32_t	leftB, leftG, leftR, leftA;				// Left side colors when tinting Y
	uint32_t	rightB, rightG, rightR, rightA;			// Right side colors when tinting X
	uint32_t	xIncB, xIncG, xIncR, xIncA;
/*
	_texel_tint_init(TINT, tint, bits, leftB, leftG, leftR, leftA, rightB, rightG, rightR, rightA,
		xIncB, xIncG, xIncR, xIncA, tintB, tintG, tintR, tintA, patchPos);
*/

	// Step 2: Setup fixed blend color
	
	int16_t fixedB, fixedG, fixedR, fixedA;

	if (bFast8)
	{
		const uint8_t* pPackTab = HiColor::packLinearTab;

		fixedB = pPackTab[tint.fixedBlendColor.b];
		fixedG = pPackTab[tint.fixedBlendColor.g];
		fixedR = pPackTab[tint.fixedBlendColor.r];
		fixedA = pPackTab[tint.fixedBlendColor.a];
	}
	else
	{
		fixedB = tint.fixedBlendColor.b;
		fixedG = tint.fixedBlendColor.g;
		fixedR = tint.fixedBlendColor.r;
		fixedA = tint.fixedBlendColor.a;
	}
	
	
	
	
	for (int y = 0; y < nLines; y++)
	{
		if( READOP == SoftBackend::ReadOp::Blur )
		{
			srcX = srcXstart;
		}
		
		// Step 3: Prepare tint for any vertical gradient
/*
		_texel_tint_line(TINT, tint, bits, leftB, leftG, leftR, leftA, rightB, rightG, rightR, rightA,
			xIncB, xIncG, xIncR, xIncA, tintB, tintG, tintR, tintA, patchPos);
*/

		for (int x = 0; x < lineLength; x++)
		{
			// Step 4: Read source pixels

			if (READOP == SoftBackend::ReadOp::Tile)
			{
				srcX &= maskX;
				srcY &= maskY;
				pSrc = pSrcSurf->pixels() + srcY * yPitch + srcX * xPitch;
			}

			int16_t srcB, srcG, srcR, srcA;

			if(READOP == SoftBackend::ReadOp::Blur)
			{
				int16_t inB[9], inG[9], inR[9], inA[9];

				for( int i = 0 ; i < 9 ; i++ )
				{
					CoordI center(srcX,srcY);
					
					CoordI ofs = center + tint.blurOfsPixel[i];
					
					if( ofs.x < 0 )
						ofs.x = 0;
					if( ofs.y < 0 )
						ofs.y = 0;
					if( ofs.x >= surfSize.w )
						ofs.x = surfSize.w -1;
					if( ofs.y >= surfSize.h )
						ofs.y = surfSize.h -1;
					
					_read_pixel(pSrc + ofs.y * yPitch + ofs.x * xPitch, SRCFORMAT, pPalette, pPalette4096, inB[i], inG[i], inR[i], inA[i]);
				}
				
				int tSrcB = ( inB[0] * tint.blurMtxB[0] + inB[1] * tint.blurMtxB[1] + inB[2] * tint.blurMtxB[2]
							+ inB[3] * tint.blurMtxB[3] + inB[4] * tint.blurMtxB[4] + inB[5] * tint.blurMtxB[5]
							+ inB[6] * tint.blurMtxB[6] + inB[7] * tint.blurMtxB[7] + inB[8] * tint.blurMtxB[8]) / 65536;

				int tSrcG = ( inG[0] * tint.blurMtxG[0] + inG[1] * tint.blurMtxG[1] + inG[2] * tint.blurMtxG[2]
							+ inG[3] * tint.blurMtxG[3] + inG[4] * tint.blurMtxG[4] + inG[5] * tint.blurMtxG[5]
							+ inG[6] * tint.blurMtxG[6] + inG[7] * tint.blurMtxG[7] + inG[8] * tint.blurMtxG[8]) / 65536;

				int tSrcR = ( inR[0] * tint.blurMtxR[0] + inR[1] * tint.blurMtxR[1] + inR[2] * tint.blurMtxR[2]
							+ inR[3] * tint.blurMtxR[3] + inR[4] * tint.blurMtxR[4] + inR[5] * tint.blurMtxR[5]
							+ inR[6] * tint.blurMtxR[6] + inR[7] * tint.blurMtxR[7] + inR[8] * tint.blurMtxR[8]) / 65536;

				srcA = inA[4];

				if (tSrcB > 4096)
					tSrcB = 4096;

				if (tSrcG > 4096)
					tSrcG = 4096;

				if (tSrcR > 4096)
					tSrcR = 4096;

				if (tSrcB < 0)
					tSrcB = 0;

				if (tSrcG < 0)
					tSrcG = 0;

				if (tSrcR < 0)
					tSrcR = 0;

				srcB = (int16_t)tSrcB;
				srcG = (int16_t)tSrcG;
				srcR = (int16_t)tSrcR;
			}
			else
			{
				if (bFast8)
					_read_pixel_fast8(pSrc, SRCFORMAT, pPalette, pPalette4096, srcB, srcG, srcR, srcA);
				else
					_read_pixel(pSrc, SRCFORMAT, pPalette, pPalette4096, srcB, srcG, srcR, srcA);
			}


			// Step 5: Apply any tint
/*
			_texel_tint_pixel(TINT, bits, srcB, srcG, srcR, srcA, xIncB, xIncG, xIncR, xIncA,
				tintB, tintG, tintR, tintA);
*/

			// Step 6: Get color components of background pixel blending into backX
			// Step 7: Blend srcX and backX into outX
			// Step 8: Write resulting pixel to destination

			int16_t backB, backG, backR, backA;
			int16_t outB, outG, outR, outA;

			if (bFast8)
			{
				_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels_fast8(BLEND, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
									outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
			}
			else
			{
				_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels(BLEND, tint.morphFactor, DSTFORMAT, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
							  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
			}

			// Step 9: Increment source and destination pointers

			if (READOP == SoftBackend::ReadOp::Tile || READOP == SoftBackend::ReadOp::Blur)
			{
				srcX += simpleTransform[0][0];
				srcY += simpleTransform[0][1];
			}
			else
				pSrc += pitches.srcX;

			pDst += pitches.dstX;
		}

		if (READOP == SoftBackend::ReadOp::Tile || READOP == SoftBackend::ReadOp::Blur)
		{
			srcX += simpleTransform[1][0];
			srcY += simpleTransform[1][1];
		}
		else
			pSrc += pitches.srcY;

		pDst += pitches.dstY;
	}
}

