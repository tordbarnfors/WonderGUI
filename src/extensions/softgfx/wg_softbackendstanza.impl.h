
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


//____ _color_tint_init() _________________________________________________

inline void _color_tint_init(TintMode tintMode, const SoftBackend::ColTrans& tint, int bits, int16_t inB, int16_t inG, int16_t inR, int16_t inA,
	int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA, HiColor*& pTintmapX, HiColor*&  pTintmapY, CoordI patchPos)
{
	if (tintMode == TintMode::None)
	{
		outR = inR;
		outG = inG;
		outB = inB;
		outA = inA;
	}

	if (tintMode == TintMode::Flat)
	{
		outR = (inR * tint.flatTintColor.r) >> 12;
		outG = (inG * tint.flatTintColor.g) >> 12;
		outB = (inB * tint.flatTintColor.b) >> 12;
		outA = (inA * tint.flatTintColor.a) >> 12;
	}

	if (tintMode == TintMode::GradientX || tintMode == TintMode::GradientXY)
	{
		pTintmapX = tint.pTintAxisX + patchPos.x - tint.tintRect.x;
	}
	
	if (tintMode == TintMode::GradientY || tintMode == TintMode::GradientXY)
	{
		pTintmapY = tint.pTintAxisY + patchPos.y - tint.tintRect.y;
	}
}

//____ _color_tint_line() _________________________________________________

inline void _color_tint_line(TintMode tintMode, const SoftBackend::ColTrans& tint, int bits, int16_t inB, int16_t inG, int16_t inR, int16_t inA,
							 int16_t& lineB, int16_t& lineG, int16_t& lineR, int16_t& lineA,
							 int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
							 HiColor*& pTintmapX, HiColor*&  pTintmapY, CoordI patchPos)
{
	if (tintMode == TintMode::GradientX || tintMode == TintMode::GradientXY )
	{
		pTintmapX = tint.pTintAxisX + patchPos.x - tint.tintRect.x;
	}

	if (tintMode == TintMode::GradientY )
	{
		outB = (inB * pTintmapY->b) >> 12;
		outG = (inG * pTintmapY->g) >> 12;
		outR = (inR * pTintmapY->r) >> 12;
		outA = (inA * pTintmapY->a) >> 12;

		pTintmapY++;
	}

	if (tintMode == TintMode::GradientXY )
	{
		lineB = (inB * pTintmapY->b) >> 12;
		lineG = (inG * pTintmapY->g) >> 12;
		lineR = (inR * pTintmapY->r) >> 12;
		lineA = (inA * pTintmapY->a) >> 12;

		pTintmapY++;
	}
}


//____ _color_tint_pixel() ______________________________________________________

inline void _color_tint_pixel(TintMode tintMode, int16_t inB, int16_t inG, int16_t inR, int16_t inA,
							  int16_t& lineB, int16_t& lineG, int16_t& lineR, int16_t& lineA,
							  int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA, HiColor*& pTintmapX)
{
	if (tintMode == TintMode::GradientX)
	{
		outB = (inB * pTintmapX->b) >> 12;
		outG = (inG * pTintmapX->g) >> 12;
		outR = (inR * pTintmapX->r) >> 12;
		outA = (inA * pTintmapX->a) >> 12;

		pTintmapX++;
	}
	
	if (tintMode == TintMode::GradientXY)
	{
		outB = (lineB * pTintmapX->b) >> 12;
		outG = (lineG * pTintmapX->g) >> 12;
		outR = (lineR * pTintmapX->r) >> 12;
		outA = (lineA * pTintmapX->a) >> 12;

		pTintmapX++;
	}
}

//____ _texel_tint_init() _________________________________________________

inline void _texel_tint_init(TintMode tintMode, const SoftBackend::ColTrans& tint, int bits,
							 int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
							 HiColor*& pTintmapX, HiColor*&  pTintmapY, CoordI patchPos)
{
	if (tintMode == TintMode::Flat)
	{
		outR = tint.flatTintColor.r;
		outG = tint.flatTintColor.g;
		outB = tint.flatTintColor.b;
		outA = tint.flatTintColor.a;
	}

	if (tintMode == TintMode::GradientX || tintMode == TintMode::GradientXY)
	{
		pTintmapX = tint.pTintAxisX + patchPos.x - tint.tintRect.x;
	}
	
	if (tintMode == TintMode::GradientY || tintMode == TintMode::GradientXY)
	{
		pTintmapY = tint.pTintAxisY + patchPos.y - tint.tintRect.y;
	}
}

//____ _texel_tint_line() _________________________________________________

inline void _texel_tint_line(TintMode tintMode, const SoftBackend::ColTrans& tint, int bits,
							 int16_t& lineB, int16_t& lineG, int16_t& lineR, int16_t& lineA,
							 int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
							 HiColor*& pTintmapX, HiColor*&  pTintmapY, CoordI patchPos)
{
	if (tintMode == TintMode::GradientX || tintMode == TintMode::GradientXY )
	{
		pTintmapX = tint.pTintAxisX + patchPos.x - tint.tintRect.x;
	}

	if (tintMode == TintMode::GradientY )
	{
		outB = pTintmapY->b;
		outG = pTintmapY->g;
		outR = pTintmapY->r;
		outA = pTintmapY->a;

		pTintmapY++;
	}

	if (tintMode == TintMode::GradientXY )
	{
		lineB = pTintmapY->b;
		lineG = pTintmapY->g;
		lineR = pTintmapY->r;
		lineA = pTintmapY->a;

		pTintmapY++;
	}
}

//____ _texel_tint_pixel() ________________________________________________

inline void _texel_tint_pixel(TintMode tintMode, int bits, int16_t& pixelB, int16_t& pixelG, int16_t& pixelR, int16_t& pixelA,
							  int16_t& lineB, int16_t& lineG, int16_t& lineR, int16_t& lineA,
							  int16_t& tintB, int16_t& tintG, int16_t& tintR, int16_t& tintA, HiColor*& pTintmapX)
{
	if (tintMode == TintMode::None)
	{
	}
	else
	{
		if (tintMode == TintMode::GradientX)
		{
			tintB = pTintmapX->b;
			tintG = pTintmapX->g;
			tintR = pTintmapX->r;
			tintA = pTintmapX->a;
			
			pTintmapX++;
		}

		if (tintMode == TintMode::GradientXY)
		{
			tintB = (pTintmapX->b * lineB) >> 12;
			tintG = (pTintmapX->g * lineG) >> 12;
			tintR = (pTintmapX->r * lineR) >> 12;
			tintA = (pTintmapX->a * lineA) >> 12;
			
			pTintmapX++;
		}

		
		pixelB = (pixelB * tintB) >> 12;
		pixelG = (pixelG * tintG) >> 12;
		pixelR = (pixelR * tintR) >> 12;
		pixelA = (pixelA * tintA) >> 12;
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

//	if( TINT == TintMode::None || TINT == TintMode::Flat )
	{
		if (DSTFORMAT == PixelFormat::Alpha_8 || DSTFORMAT == PixelFormat::BGRA_4_linear ||
			DSTFORMAT == PixelFormat::BGRA_8_linear || DSTFORMAT == PixelFormat::BGRX_8_linear ||
			DSTFORMAT == PixelFormat::BGR_565_linear || DSTFORMAT == PixelFormat::RGB_565_bigendian ||
			DSTFORMAT == PixelFormat::RGB_555_bigendian || DSTFORMAT == PixelFormat::BGR_8_linear)
		{
			bFast8 = true;
			bits = 8;
		}
	}

	// Step 1: Read source pixels and prepare tint

	int16_t srcB, srcG, srcR, srcA;

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

	// Prepare Tint
	
	int16_t tintedB, tintedG, tintedR, tintedA;

	HiColor * pTintmapX;
	HiColor * pTintmapY;
	
	_color_tint_init(TINT, tint, bits, srcB, srcG, srcR, srcA, tintedB, tintedG, tintedR, tintedA,
		pTintmapX, pTintmapY, patchPos);


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

		int16_t lineB, lineG, lineR, lineA;

		_color_tint_line(TINT, tint, bits, srcB, srcG, srcR, srcA,
						 lineB, lineG, lineR, lineA,
						 tintedB, tintedG, tintedR, tintedA,
						 pTintmapX, pTintmapY, patchPos);

		for (int x = 0; x < lineLength; x++)
		{
			// Step 4: Apply any horizontal tint gradient

			_color_tint_pixel(TINT, srcB, srcG, srcR, srcA,
							 lineB, lineG, lineR, lineA,
							 tintedB, tintedG, tintedR, tintedA,
							 pTintmapX);

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
void _straight_blit(const uint8_t* pSrc, uint8_t* pDst, const SoftSurface* pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform * pMatrix)
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

	int16_t	tintB, tintG, tintR, tintA;

	HiColor * pTintmapX;
	HiColor * pTintmapY;

	_texel_tint_init(TINT, tint, bits, tintB, tintG, tintR, tintA, pTintmapX, pTintmapY, patchPos);

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

		int16_t lineB, lineG, lineR, lineA;

		_texel_tint_line(TINT, tint, bits, lineB, lineG, lineR, lineA,
						 tintB, tintG, tintR, tintA, pTintmapX, pTintmapY, patchPos);

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

			_texel_tint_pixel(TINT, bits, srcB, srcG, srcR, srcA,
							  lineB, lineG, lineR, lineA,
							  tintB, tintG, tintR, tintA, pTintmapX );

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
				srcX += pMatrix->xx;
				srcY += pMatrix->xy;
			}
			else
				pSrc += pitches.srcX;

			pDst += pitches.dstX;
		}

		if (READOP == SoftBackend::ReadOp::Tile || READOP == SoftBackend::ReadOp::Blur)
		{
			srcX += pMatrix->yx;
			srcY += pMatrix->yy;
		}
		else
			pSrc += pitches.srcY;

		pDst += pitches.dstY;
	}
}

//____ _transform_blit __________________________________________

template<PixelFormat SRCFORMAT, SampleMethod SAMPLEMETHOD, TintMode TINT, BlendMode BLEND, PixelFormat DSTFORMAT, SoftBackend::ReadOp READOP >
void _transform_blit(const SoftSurface* pSrcSurf, BinalCoord pos, const binalInt matrix[2][2], uint8_t* pDst, int dstPitchX, int dstPitchY, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos)
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


	int srcPixelBytes = pSrcSurf->pixelBits()/8;
	int	srcPitch = pSrcSurf->pitch();
	binalInt srcMax_w = pSrcSurf->pixelSize().w * BINAL_MUL;
	binalInt srcMax_h = pSrcSurf->pixelSize().h * BINAL_MUL;

	binalInt pixelIncX = matrix[0][0];
	binalInt pixelIncY = matrix[0][1];

	binalInt lineIncX = matrix[1][0];
	binalInt lineIncY = matrix[1][1];

	binalInt	srcPosMaskX = pSrcSurf->tileMaskX();
	binalInt	srcPosMaskY = pSrcSurf->tileMaskY();

	binalInt	srcPosMaskX_binals = (srcPosMaskX << BINAL_SHIFT) | BINAL_MASK;
	binalInt	srcPosMaskY_binals = (srcPosMaskY << BINAL_SHIFT) | BINAL_MASK;

	const Color8 * pPalette = pSrcSurf->palette();
	const HiColor * pPalette4096 = pSrcSurf->palette4096();

	// Step 1: Setup fixed blend color
	
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
		binalInt ofsX = (binalInt)(pos.x + lineIncX * y);
		binalInt ofsY = (binalInt)(pos.y + lineIncY * y);		// We use 15 binals for all calculations


		for (int x = 0; x < lineLength; x++)
		{
			if (READOP == SoftBackend::ReadOp::Tile)
			{
				ofsX &= srcPosMaskX_binals;
				ofsY &= srcPosMaskY_binals;
			}

			// Step 2: Read source color.

			int16_t srcB, srcG, srcR, srcA;
			uint8_t* p = pSrcSurf->pixels() + (ofsY >> BINAL_SHIFT) * srcPitch + (ofsX >> BINAL_SHIFT) * srcPixelBytes;

			if (SAMPLEMETHOD == SampleMethod::Bilinear)
			{
				// Read separate source color components for our 2x2 pixel square.

				int16_t src11_b, src11_g, src11_r, src11_a;
				int16_t src12_b, src12_g, src12_r, src12_a;
				int16_t src21_b, src21_g, src21_r, src21_a;
				int16_t src22_b, src22_g, src22_r, src22_a;

				if (READOP == SoftBackend::ReadOp::Clip && ((ofsX | ofsY | (srcMax_w - (ofsX + BINAL_MUL + 1)) | (srcMax_h - (ofsY + BINAL_MUL + 1))) < 0))
				{
					if (ofsX > srcMax_w || ofsY > srcMax_h || ofsX < -BINAL_MUL || ofsY < -BINAL_MUL)
					{
						// Totally outside, no need to process pixel

						ofsX += pixelIncX;
						ofsY += pixelIncY;
						pDst += dstPitchX;
						continue;
					}
					else
					{
						// At least one of the four sample pixels is within source surface

						if (bFast8)
						{
							if (ofsX >= 0 && ofsY >= 0 && ofsX < srcMax_w && ofsY < srcMax_h)
								_read_pixel_fast8(p, SRCFORMAT, pPalette, pPalette4096, src11_b, src11_g, src11_r, src11_a);
							else
								src11_b = src11_g = src11_r = src11_a = 0;

							if ((ofsX + BINAL_MUL) >= 0 && ofsY >= 0 && (ofsX + BINAL_MUL) < srcMax_w && ofsY < srcMax_h)
								_read_pixel_fast8(p + srcPixelBytes, SRCFORMAT, pPalette, pPalette4096, src12_b, src12_g, src12_r, src12_a);
							else
								src12_b = src12_g = src12_r = src12_a = 0;

							if (ofsX >= 0 && (ofsY + BINAL_MUL) >= 0 && ofsX < srcMax_w && (ofsY + BINAL_MUL) < srcMax_h)
								_read_pixel_fast8(p + srcPitch, SRCFORMAT, pPalette, pPalette4096, src21_b, src21_g, src21_r, src21_a);
							else
								src21_b = src21_g = src21_r = src21_a = 0;

							if ((ofsX + 32768) >= 0 && (ofsY + BINAL_MUL) >= 0 && (ofsX + BINAL_MUL) < srcMax_w && (ofsY + BINAL_MUL) < srcMax_h)
								_read_pixel_fast8(p + srcPitch + srcPixelBytes, SRCFORMAT, pPalette, pPalette4096, src22_b, src22_g, src22_r, src22_a);
							else
								src22_b = src22_g = src22_r = src22_a = 0;
						}
						else
						{
							if (ofsX >= 0 && ofsY >= 0 && ofsX < srcMax_w && ofsY < srcMax_h)
								_read_pixel(p, SRCFORMAT, pPalette, pPalette4096, src11_b, src11_g, src11_r, src11_a);
							else
								src11_b = src11_g = src11_r = src11_a = 0;

							if ((ofsX + BINAL_MUL) >= 0 && ofsY >= 0 && (ofsX + BINAL_MUL) < srcMax_w && ofsY < srcMax_h)
								_read_pixel(p + srcPixelBytes, SRCFORMAT, pPalette, pPalette4096, src12_b, src12_g, src12_r, src12_a);
							else
								src12_b = src12_g = src12_r = src12_a = 0;

							if (ofsX >= 0 && (ofsY + BINAL_MUL) >= 0 && ofsX < srcMax_w && (ofsY + BINAL_MUL) < srcMax_h)
								_read_pixel(p + srcPitch, SRCFORMAT, pPalette, pPalette4096, src21_b, src21_g, src21_r, src21_a);
							else
								src21_b = src21_g = src21_r = src21_a = 0;

							if ((ofsX + BINAL_MUL) >= 0 && (ofsY + BINAL_MUL) >= 0 && (ofsX + BINAL_MUL) < srcMax_w && (ofsY + BINAL_MUL) < srcMax_h)
								_read_pixel(p + srcPitch + srcPixelBytes, SRCFORMAT, pPalette, pPalette4096, src22_b, src22_g, src22_r, src22_a);
							else
								src22_b = src22_g = src22_r = src22_a = 0;
						}
					}
				}
				else
				{
					uint8_t* p2, * p3, * p4;

					if (READOP == SoftBackend::ReadOp::Tile)
					{
						binalInt x = (ofsX >> BINAL_SHIFT), y = (ofsY >> BINAL_SHIFT);

						p2 = pSrcSurf->pixels() + y * srcPitch + ((x + 1) & srcPosMaskX) * srcPixelBytes;
						p3 = pSrcSurf->pixels() + ((y + 1) & srcPosMaskY) * srcPitch + x * srcPixelBytes;
						p4 = pSrcSurf->pixels() + ((y + 1) & srcPosMaskY) * srcPitch + ((x + 1) & srcPosMaskX) * srcPixelBytes;
					}
					else
					{
						//							assert((ofsX | ofsY | (srcMax.w - (ofsX + 32768)) | (srcMax.h - (ofsY + 32768))) >= 0);

						binalInt nextX = (ofsX & BINAL_MASK) == 0 ? 0 : srcPixelBytes;
						binalInt nextY = (ofsY & BINAL_MASK) == 0 ? 0 : srcPitch;

						p2 = p + nextX;
						p3 = p + nextY;
						p4 = p + nextX + nextY;
					}

					if (bFast8)
					{
						_read_pixel_fast8(p, SRCFORMAT, pPalette, pPalette4096, src11_b, src11_g, src11_r, src11_a);
						_read_pixel_fast8(p2, SRCFORMAT, pPalette, pPalette4096, src12_b, src12_g, src12_r, src12_a);
						_read_pixel_fast8(p3, SRCFORMAT, pPalette, pPalette4096, src21_b, src21_g, src21_r, src21_a);
						_read_pixel_fast8(p4, SRCFORMAT, pPalette, pPalette4096, src22_b, src22_g, src22_r, src22_a);
					}
					else
					{
						_read_pixel(p, SRCFORMAT, pPalette, pPalette4096, src11_b, src11_g, src11_r, src11_a);
						_read_pixel(p2, SRCFORMAT, pPalette, pPalette4096, src12_b, src12_g, src12_r, src12_a);
						_read_pixel(p3, SRCFORMAT, pPalette, pPalette4096, src21_b, src21_g, src21_r, src21_a);
						_read_pixel(p4, SRCFORMAT, pPalette, pPalette4096, src22_b, src22_g, src22_r, src22_a);
					}
				}

				// Interpolate our 2x2 source colors into one source color, srcX

				binalInt fracX2 = ofsX & BINAL_MASK;
				binalInt fracX1 = BINAL_MUL - fracX2;

				binalInt fracY2 = ofsY & BINAL_MASK;
				binalInt fracY1 = BINAL_MUL - fracY2;

				binalInt mul11 = fracX1 * fracY1 >> BINAL_SHIFT;
				binalInt mul12 = fracX2 * fracY1 >> BINAL_SHIFT;
				binalInt mul21 = fracX1 * fracY2 >> BINAL_SHIFT;
				binalInt mul22 = fracX2 * fracY2 >> BINAL_SHIFT;

				srcB = (src11_b * mul11 + src12_b * mul12 + src21_b * mul21 + src22_b * mul22) >> BINAL_SHIFT;
				srcG = (src11_g * mul11 + src12_g * mul12 + src21_g * mul21 + src22_g * mul22) >> BINAL_SHIFT;
				srcR = (src11_r * mul11 + src12_r * mul12 + src21_r * mul21 + src22_r * mul22) >> BINAL_SHIFT;
				srcA = (src11_a * mul11 + src12_a * mul12 + src21_a * mul21 + src22_a * mul22) >> BINAL_SHIFT;

			}
			else
			{
				if(READOP == SoftBackend::ReadOp::Blur)
				{
					int16_t inB[9], inG[9], inR[9], inA[9];

					binalInt x = (ofsX >> BINAL_SHIFT), y = (ofsY >> BINAL_SHIFT);

					auto surfSize = pSrcSurf->pixelSize();
					
					auto p = pSrcSurf->pixels();
					
					for( int i = 0 ; i < 9 ; i++ )
					{
						CoordI center((int)x,(int)y);
						
						CoordI ofs = center + tint.blurOfsPixel[i];
						
						if( ofs.x < 0 )
							ofs.x = 0;
						if( ofs.y < 0 )
							ofs.y = 0;
						if( ofs.x >= surfSize.w )
							ofs.x = surfSize.w -1;
						if( ofs.y >= surfSize.h )
							ofs.y = surfSize.h -1;
						
						_read_pixel(p + ofs.y * srcPitch + ofs.x * srcPixelBytes, SRCFORMAT, pPalette, pPalette4096, inB[i], inG[i], inR[i], inA[i]);
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
				else if (READOP == SoftBackend::ReadOp::Clip && ((ofsX | ofsY | (srcMax_w - 1 - ofsX) | (srcMax_h - 1 - ofsY)) < 0))
				{
					if (BLEND == BlendMode::Blend)
					{
						ofsX += pixelIncX;
						ofsY += pixelIncY;
						pDst += dstPitchX;
						continue;
					}
					else
					{
						srcB = 0;
						srcG = 0;
						srcR = 0;
						srcA = 0;
					}

				}
				else
				{
					assert((ofsX | ofsY | (srcMax_w - 1 - ofsX) | (srcMax_h - 1 - ofsY)) >= 0);
					if (bFast8)
						_read_pixel_fast8(p, SRCFORMAT, pPalette, pPalette4096, srcB, srcG, srcR, srcA);
					else
						_read_pixel(p, SRCFORMAT, pPalette, pPalette4096, srcB, srcG, srcR, srcA);
				}
			}

			// Step 3: Get color components of background pixel blending into backX
			// Step 4: Blend srcX and backX into outX
			// Step 5: Write resulting pixel to destination

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

			// Step 6: Increment source and destination pointers

			ofsX += pixelIncX;
			ofsY += pixelIncY;
			pDst += dstPitchX;
		}
		pDst += dstPitchY;
	}
}

//____ _add_segment_color() _______________________________________________

inline void _add_segment_color(SoftBackend::StripSource SOURCE, int blendFraction, int offset, const int16_t* pSegmentColor, const HiColor * pSegmentTintmap, int& accB, int& accG, int& accR, int& accA)
{
	if (SOURCE == SoftBackend::StripSource::Colors)
	{
		accR += (blendFraction * pSegmentColor[0]) >> 4;
		accG += (blendFraction * pSegmentColor[1]) >> 4;
		accB += (blendFraction * pSegmentColor[2]) >> 4;
		accA += blendFraction << 8;
	}

	if (SOURCE == SoftBackend::StripSource::Tintmaps)
	{
		accB += (blendFraction * pSegmentTintmap[offset].b) >> 4;
		accG += (blendFraction * pSegmentTintmap[offset].g) >> 4;
		accR += (blendFraction * pSegmentTintmap[offset].r) >> 4;
		accA += blendFraction << 8;
	}

	if (SOURCE == SoftBackend::StripSource::ColorsAndTintmaps)
	{
		accB += (blendFraction >> 4) * ((pSegmentTintmap[offset].b * pSegmentColor[2]) >> 12);
		accG += (blendFraction >> 4) * ((pSegmentTintmap[offset].g * pSegmentColor[1]) >> 12);
		accR += (blendFraction >> 4) * ((pSegmentTintmap[offset].r * pSegmentColor[0]) >> 12);
		accA += blendFraction << 8;
	}

}



//____ _draw_segment_strip() _______________________________________________

template<SoftBackend::StripSource SOURCE, BlendMode BLEND, PixelFormat DSTFORMAT>
void _draw_segment_strip(int colBeg, int colEnd, uint8_t* pStripStart, int pixelPitch, int nEdges, SoftBackend::SegmentEdge* pEdges, const int16_t* pSegmentColors, const HiColor* pSegmentTintmap, int segmentTintmapPitch, const bool* pTransparentSegments, const bool* pOpaqueSegments, const SoftBackend::ColTrans& tint)
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


	// Setup fixed blend color

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

				if (SOURCE == SoftBackend::StripSource::Colors)
				{
					if (bFast8)
					{
						inR = HiColor::packLinearTab[pSegmentColors[0]];
						inG = HiColor::packLinearTab[pSegmentColors[1]];
						inB = HiColor::packLinearTab[pSegmentColors[2]];
						inA = HiColor::packLinearTab[pSegmentColors[3]];
					}
					else
					{
						inR = pSegmentColors[0];
						inG = pSegmentColors[1];
						inB = pSegmentColors[2];
						inA = pSegmentColors[3];
					}
				}

				if (*pOpaqueSegments)
				{
					while (offset + 255 < end)
					{
						if (SOURCE == SoftBackend::StripSource::Tintmaps)
						{
							if (bFast8)
							{
								inB = HiColor::packLinearTab[pSegmentTintmap[offset >> 8].b];
								inG = HiColor::packLinearTab[pSegmentTintmap[offset >> 8].g];
								inR = HiColor::packLinearTab[pSegmentTintmap[offset >> 8].r];
								inA = HiColor::packLinearTab[pSegmentTintmap[offset >> 8].a];

							}
							else
							{
								inB = pSegmentTintmap[offset >> 8].b;
								inG = pSegmentTintmap[offset >> 8].g;
								inR = pSegmentTintmap[offset >> 8].r;
								inA = pSegmentTintmap[offset >> 8].a;
							}
						}
						
						if (SOURCE == SoftBackend::StripSource::ColorsAndTintmaps)
						{
							if (bFast8)
							{
								inB = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].b * pSegmentColors[2]) >> 12];
								inG = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].g * pSegmentColors[1]) >> 12];
								inR = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].r * pSegmentColors[0]) >> 12];
								inA = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].a * pSegmentColors[3]) >> 12];
							}
							else
							{
								inB = (pSegmentTintmap[offset >> 8].b * pSegmentColors[2]) >> 12;
								inG = (pSegmentTintmap[offset >> 8].g * pSegmentColors[1]) >> 12;
								inR = (pSegmentTintmap[offset >> 8].r * pSegmentColors[0]) >> 12;
								inA = (pSegmentTintmap[offset >> 8].a * pSegmentColors[3]) >> 12;
							}
						}

						if (bFast8)
						{
							_write_pixel_fast8(pDst, DSTFORMAT, inB, inG, inR, inA);
						}
						else
						{
							_write_pixel(pDst, DSTFORMAT, inB, inG, inR, inA);
						}

						pDst += pixelPitch;
						offset += 256;
					}
				}
				else
				{
					while (offset + 255 < end)
					{
						if (SOURCE == SoftBackend::StripSource::Tintmaps)
						{
							if (bFast8)
							{
								inB = HiColor::packLinearTab[pSegmentTintmap[offset >> 8].b];
								inG = HiColor::packLinearTab[pSegmentTintmap[offset >> 8].g];
								inR = HiColor::packLinearTab[pSegmentTintmap[offset >> 8].r];
								inA = HiColor::packLinearTab[pSegmentTintmap[offset >> 8].a];
							}
							else
							{
								inB = pSegmentTintmap[offset >> 8].b;
								inG = pSegmentTintmap[offset >> 8].g;
								inR = pSegmentTintmap[offset >> 8].r;
								inA = pSegmentTintmap[offset >> 8].a;
							}
						}

						if (SOURCE == SoftBackend::StripSource::ColorsAndTintmaps)
						{
							if (bFast8)
							{
								inB = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].b * pSegmentColors[2]) >> 12];
								inG = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].g * pSegmentColors[1]) >> 12];
								inR = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].r * pSegmentColors[0]) >> 12];
								inA = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].a * pSegmentColors[3]) >> 12];
							}
							else
							{
								inB = (pSegmentTintmap[offset >> 8].b * pSegmentColors[2]) >> 12;
								inG = (pSegmentTintmap[offset >> 8].g * pSegmentColors[1]) >> 12;
								inR = (pSegmentTintmap[offset >> 8].r * pSegmentColors[0]) >> 12;
								inA = (pSegmentTintmap[offset >> 8].a * pSegmentColors[3]) >> 12;
							}
						}

						int16_t backB, backG, backR, backA;
						int16_t outB, outG, outR, outA;

						if (bFast8)
						{
							_read_pixel_fast8(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
							_blend_pixels_fast8(BLEND, tint.morphFactor, DSTFORMAT, inB, inG, inR, inA, backB, backG, backR, backA,
								outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
							_write_pixel_fast8(pDst, DSTFORMAT, outB, outG, outR, outA);
						}
						else
						{
							_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);
							_blend_pixels(BLEND, tint.morphFactor, DSTFORMAT, inB, inG, inR, inA, backB, backG, backR, backA,
								outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
							_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
						}

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
							+ (256 - (pEdges[edge].end & 0xFF)) * 65536 / 256;
					}

					segmentFractions[edge] = remainingFractions - frac;

					//						if (segmentFractions[edge] < 0 || segmentFractions[edge] > 65536)
					//							int err = 1;

					remainingFractions = frac;
					edge++;
				}

				segmentFractions[edge] = remainingFractions;

				int16_t backB, backG, backR, backA;

				_read_pixel(pDst, DSTFORMAT, nullptr, nullptr, backB, backG, backR, backA);

				int16_t outB = 0, outG = 0, outR = 0, outA = 0;

				int accB = 0;
				int accG = 0;
				int accR = 0;
				int accA = 0;

				if (BLEND == BlendMode::Replace)
				{
					for (int i = 0; i <= edge; i++)
					{
						int blendFraction = segmentFractions[i];
						_add_segment_color(SOURCE, blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					outB = accB >> 12;
					outG = accG >> 12;
					outR = accR >> 12;
					outA = accA >> 12;
				}

				if (BLEND == BlendMode::Blend)
				{
					int backFraction = 65536;

					for (int i = 0; i <= edge; i++)
					{
						int alpha;
						
						if( SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = pSegmentTintmap[offset >> 8].a;
						if( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * pSegmentTintmap[offset >> 8].a / 4096;

						
						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						backFraction -= blendFraction;
						_add_segment_color(SOURCE, blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					outB = (accB >> 12) + ((backB * backFraction) >> 16);
					outG = (accG >> 12) + ((backG * backFraction) >> 16);
					outR = (accR >> 12) + ((backR * backFraction) >> 16);
					outA = (accA >> 12) + ((backA * backFraction) >> 16);	// This should be correct... 							
				}

				if (BLEND == BlendMode::BlendFixedColor)
				{
					int backFraction = 65536;

					for (int i = 0; i <= edge; i++)
					{
						int alpha;
						
						if( SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = pSegmentTintmap[offset >> 8].a;
						if( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * pSegmentTintmap[offset >> 8].a / 4096;

						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						backFraction -= blendFraction;
						_add_segment_color(SOURCE, blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					outB = (accB >> 12) + ((fixedB * backFraction) >> 16);
					outG = (accG >> 12) + ((fixedG * backFraction) >> 16);
					outR = (accR >> 12) + ((fixedR * backFraction) >> 16);
					outA = (accA >> 12) + ((fixedA * backFraction) >> 16);	// This should be correct...
				}

				if (BLEND == BlendMode::Add)
				{
					for (int i = 0; i <= edge; i++)
					{
						int alpha;
						
						if( SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = pSegmentTintmap[offset >> 8].a;
						if( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * pSegmentTintmap[offset >> 8].a / 4096;

						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						_add_segment_color(SOURCE, blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					outB = SoftBackend::s_limit4096Tab[4097 + backB + (accB >> 12)];
					outG = SoftBackend::s_limit4096Tab[4097 + backG + (accG >> 12)];
					outR = SoftBackend::s_limit4096Tab[4097 + backR + (accR >> 12)];
					outA = backA;
				}

				if (BLEND == BlendMode::Subtract)
				{
					for (int i = 0; i <= edge; i++)
					{
						int alpha;
						
						if( SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = pSegmentTintmap[offset >> 8].a;
						if( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * pSegmentTintmap[offset >> 8].a / 4096;

						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						_add_segment_color(SOURCE, blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					outB = SoftBackend::s_limit4096Tab[4097 + backB - (accB >> 12)];
					outG = SoftBackend::s_limit4096Tab[4097 + backG - (accG >> 12)];
					outR = SoftBackend::s_limit4096Tab[4097 + backR - (accR >> 12)];
					outA = backA;
				}

				if (BLEND == BlendMode::Multiply)
				{
					for (int i = 0; i <= edge; i++)
					{
						int blendFraction = segmentFractions[i];
						_add_segment_color(SOURCE, blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					outB = (backB * (accB >> 12)) >> 12;
					outG = (backG * (accG >> 12)) >> 12;
					outR = (backR * (accR >> 12)) >> 12;
					outA = backA;
				}

				if (BLEND == BlendMode::Invert)
				{
					for (int i = 0; i <= edge; i++)
					{
						int blendFraction = segmentFractions[i];
						_add_segment_color(SOURCE, blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					int srcB2 = accB >> 12;
					int srcG2 = accG >> 12;
					int srcR2 = accR >> 12;

					outB = (srcB2 * (4096 - backB) + backB * (4096 - srcB2)) >> 12;
					outG = (srcG2 * (4096 - backG) + backG * (4096 - srcG2)) >> 12;
					outR = (srcR2 * (4096 - backR) + backR * (4096 - srcR2)) >> 12;
					outA = backA;
				}

				if (BLEND == BlendMode::Min)
				{
					int backFraction = 65536;

					for (int i = 0; i <= edge; i++)
					{
						int alpha;
						
						if( SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = pSegmentTintmap[offset >> 8].a;
						if( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * pSegmentTintmap[offset >> 8].a / 4096;

						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						backFraction -= blendFraction;
						_add_segment_color(SOURCE, blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					int16_t srcB = (accB >> 12) + ((backB * backFraction) >> 16);
					int16_t srcG = (accG >> 12) + ((backG * backFraction) >> 16);
					int16_t srcR = (accR >> 12) + ((backR * backFraction) >> 16);

					outB = std::min(backB, srcB);
					outG = std::min(backG, srcG);
					outR = std::min(backR, srcR);
				}

				if (BLEND == BlendMode::Max)
				{
					int backFraction = 65536;

					for (int i = 0; i <= edge; i++)
					{
						int alpha;
						
						if( SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = pSegmentTintmap[offset >> 8].a;
						if( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * pSegmentTintmap[offset >> 8].a / 4096;

						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						backFraction -= blendFraction;
						_add_segment_color(SOURCE, blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					int16_t srcB = (accB >> 12) + ((backB * backFraction) >> 16);
					int16_t srcG = (accG >> 12) + ((backG * backFraction) >> 16);
					int16_t srcR = (accR >> 12) + ((backR * backFraction) >> 16);

					outB = std::max(backB, srcB);
					outG = std::max(backG, srcG);
					outR = std::max(backR, srcR);
				}

				if (BLEND == BlendMode::Morph)
				{
					int backFraction = 65536;

					for (int i = 0; i <= edge; i++)
					{
						int alpha;
						
						if( SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = pSegmentTintmap[offset >> 8].a;
						if( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * pSegmentTintmap[offset >> 8].a / 4096;

						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						backFraction -= blendFraction;
						_add_segment_color(SOURCE, blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					int invMorph = 4096 - tint.morphFactor;

					outB = (backB * invMorph + (accB >> 12) * tint.morphFactor) >> 12;
					outG = (backG * invMorph + (accG >> 12) * tint.morphFactor) >> 12;
					outR = (backR * invMorph + (accR >> 12) * tint.morphFactor) >> 12;
					outA = (backA * invMorph + (backFraction >> 4) * tint.morphFactor) >> 12;
				}

				_write_pixel(pDst, DSTFORMAT, outB, outG, outR, outA);
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
			if (SOURCE == SoftBackend::StripSource::Tintmaps || SOURCE == SoftBackend::StripSource::ColorsAndTintmaps)
				pSegmentTintmap += segmentTintmapPitch;
			else
				pSegmentColors += 4;

		}
	}
}
