
#include <wax_softbackend.h>
#include <wax_util.h>
#include <wax_core.h>

#include <algorithm>
#include <cstring>
#include <type_traits>

using namespace wax;
using namespace wax::Util;

// Macro for restrict keyword
#if (_MSC_VER > 1800)
#define WAX_RESTRICT __restrict
#else
#define WAX_RESTRICT __restrict__
#endif

// Macro for conditions that seldom hold in the kernels, so that the common path is laid out first.
#if defined(__GNUC__) || defined(__clang__)
#define WAX_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define WAX_UNLIKELY(x) (x)
#endif

// Macro for functions that only handle rare cases, which would crowd the kernels if inlined.
#if defined(__GNUC__) || defined(__clang__)
#define WAX_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define WAX_NOINLINE __declspec(noinline)
#else
#define WAX_NOINLINE
#endif

//____ Pixel layout helpers _______________________________________________________

// Loads and stores of 16-bit pixels, swapped if the byte order of the format differs from the system.

template<SoftFormat format>
static inline uint16_t _load16(const uint8_t* WAX_RESTRICT pPixel)
{
	uint16_t pixel = *(const uint16_t*)pPixel;
	if constexpr (SoftFormatInfo::needsSwap(format))
		pixel = Util::endianSwap(pixel);
	return pixel;
}

template<SoftFormat format>
static inline void _store16(uint8_t* WAX_RESTRICT pPixel, uint16_t pixel)
{
	if constexpr (SoftFormatInfo::needsSwap(format))
		pixel = Util::endianSwap(pixel);
	*(uint16_t*)pPixel = pixel;
}

// Byte offsets of the channels of 32-bit pixels.

template<SoftFormat format> constexpr int _ofsB() { return SoftFormatInfo::isBigEndian(format) ? 3 : 0; }
template<SoftFormat format> constexpr int _ofsG() { return SoftFormatInfo::isBigEndian(format) ? 2 : 1; }
template<SoftFormat format> constexpr int _ofsR() { return SoftFormatInfo::isBigEndian(format) ? 1 : 2; }
template<SoftFormat format> constexpr int _ofsA() { return SoftFormatInfo::isBigEndian(format) ? 0 : 3; }

//____ read_pixel_fast8() _______________________________________________________
//
// Reads a pixel with 8 bits per channel, without color space conversion.

template<SoftFormat format>
static inline void _read_pixel_fast8(const uint8_t* WAX_RESTRICT pPixel, const Color8* WAX_RESTRICT pPalette, const HiColor* WAX_RESTRICT pPalette4096, int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA)
{
	constexpr PixelFormat layout = SoftFormatInfo::pixelFormat(format);

	if constexpr(format == SoftFormat::Undefined)
	{
		const int16_t* p = (const int16_t*)pPixel;

		outB = HiColor::packLinearTab[p[0]];
		outG = HiColor::packLinearTab[p[1]];
		outR = HiColor::packLinearTab[p[2]];
		outA = HiColor::packLinearTab[p[3]];
	}
	else if constexpr(layout == PixelFormat::ARGB_8)
	{
		outB = pPixel[_ofsB<format>()];
		outG = pPixel[_ofsG<format>()];
		outR = pPixel[_ofsR<format>()];
		outA = pPixel[_ofsA<format>()];
	}
	else if constexpr(layout == PixelFormat::XRGB_8)
	{
		outB = pPixel[_ofsB<format>()];
		outG = pPixel[_ofsG<format>()];
		outR = pPixel[_ofsR<format>()];
		outA = 255;
	}
	else if constexpr(layout == PixelFormat::RGB_565 || layout == PixelFormat::BGR_565)
	{
		uint16_t pixel = _load16<format>(pPixel);

		int16_t high = SoftBackend::s_fast8_channel_5[pixel >> 11];
		int16_t low = SoftBackend::s_fast8_channel_5[pixel & 0x1F];

		outG = SoftBackend::s_fast8_channel_6[(pixel >> 5) & 0x3F];
		outR = layout == PixelFormat::RGB_565 ? high : low;
		outB = layout == PixelFormat::RGB_565 ? low : high;
		outA = 255;
	}
	else if constexpr(layout == PixelFormat::Alpha_8)
	{
		outB = 255;
		outG = 255;
		outR = 255;
		outA = *pPixel;
	}
	else if constexpr(layout == PixelFormat::Index_8 || layout == PixelFormat::Index_16)
	{
		Color8 c = pPalette[layout == PixelFormat::Index_8 ? *pPixel : _load16<format>(pPixel)];
		outB = c.b;
		outG = c.g;
		outR = c.r;
		outA = c.a;
	}
}


//____ read_pixel() _______________________________________________________
//
// Reads a pixel with 12 bits per channel (0 -> 4096), converted to linear.

template<SoftFormat format>
static inline void _read_pixel(const uint8_t* WAX_RESTRICT pPixel, const Color8* WAX_RESTRICT pPalette, const HiColor* WAX_RESTRICT pPalette4096, int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA)
{
	constexpr PixelFormat layout = SoftFormatInfo::pixelFormat(format);

	if constexpr(format == SoftFormat::Undefined)
	{
		const int16_t* p = (const int16_t*)pPixel;

		outB = p[0];
		outG = p[1];
		outR = p[2];
		outA = p[3];
	}
	else if constexpr(layout == PixelFormat::ARGB_8 || layout == PixelFormat::XRGB_8)
	{
		const int16_t* pUnpack = SoftFormatInfo::isLinear(format) ? HiColor::unpackLinearTab : HiColor::unpackSRGBTab;

		outB = pUnpack[pPixel[_ofsB<format>()]];
		outG = pUnpack[pPixel[_ofsG<format>()]];
		outR = pUnpack[pPixel[_ofsR<format>()]];
		outA = layout == PixelFormat::ARGB_8 ? HiColor::unpackLinearTab[pPixel[_ofsA<format>()]] : 4096;
	}
	else if constexpr(layout == PixelFormat::RGB_565 || layout == PixelFormat::BGR_565)
	{
		uint16_t pixel = _load16<format>(pPixel);

		const int16_t* pChannel5 = SoftFormatInfo::isLinear(format) ? SoftBackend::s_channel_5_linear : SoftBackend::s_channel_5_sRGB;
		const int16_t* pChannel6 = SoftFormatInfo::isLinear(format) ? SoftBackend::s_channel_6_linear : SoftBackend::s_channel_6_sRGB;

		int16_t high = pChannel5[pixel >> 11];
		int16_t low = pChannel5[pixel & 0x1F];

		outG = pChannel6[(pixel >> 5) & 0x3F];
		outR = layout == PixelFormat::RGB_565 ? high : low;
		outB = layout == PixelFormat::RGB_565 ? low : high;
		outA = 4096;
	}
	else if constexpr(layout == PixelFormat::Alpha_8)
	{
		outB = 4096;
		outG = 4096;
		outR = 4096;
		outA = HiColor::unpackLinearTab[*pPixel];
	}
	else if constexpr(layout == PixelFormat::Index_8 || layout == PixelFormat::Index_16)
	{
		const HiColor* p = &pPalette4096[layout == PixelFormat::Index_8 ? *pPixel : _load16<format>(pPixel)];
		outB = p->b;
		outG = p->g;
		outR = p->r;
		outA = p->a;
	}
}

//____ write_pixel_fast8() _______________________________________________________
//
// Writes a pixel from 8 bits per channel, without color space conversion.

template<SoftFormat format>
static inline void _write_pixel_fast8(uint8_t* pPixel, int16_t b, int16_t g, int16_t r, int16_t a)
{
	constexpr PixelFormat layout = SoftFormatInfo::pixelFormat(format);

	if constexpr(format == SoftFormat::Undefined)
	{
		int16_t* p = (int16_t*)pPixel;

		p[0] = HiColor::unpackLinearTab[b];
		p[1] = HiColor::unpackLinearTab[g];
		p[2] = HiColor::unpackLinearTab[r];
		p[3] = HiColor::unpackLinearTab[a];
	}
	else if constexpr(layout == PixelFormat::ARGB_8)
	{
		pPixel[_ofsB<format>()] = (uint8_t)b;
		pPixel[_ofsG<format>()] = (uint8_t)g;
		pPixel[_ofsR<format>()] = (uint8_t)r;
		pPixel[_ofsA<format>()] = (uint8_t)a;
	}
	else if constexpr(layout == PixelFormat::XRGB_8)
	{
		pPixel[_ofsB<format>()] = (uint8_t)b;
		pPixel[_ofsG<format>()] = (uint8_t)g;
		pPixel[_ofsR<format>()] = (uint8_t)r;
	}
	else if constexpr(layout == PixelFormat::RGB_565 || layout == PixelFormat::BGR_565)
	{
		uint16_t r5 = SoftBackend::s_round_channel_5[r] >> 3;
		uint16_t b5 = SoftBackend::s_round_channel_5[b] >> 3;

		// G5 variant rounds green to 5 bits and never sets its lowest bit.

		uint16_t g6 = format == SoftFormat::BGR_565_BE_linear_G5 ? SoftBackend::s_round_channel_5[g] >> 2 : SoftBackend::s_round_channel_6[g] >> 2;

		uint16_t high = layout == PixelFormat::RGB_565 ? r5 : b5;
		uint16_t low = layout == PixelFormat::RGB_565 ? b5 : r5;

		_store16<format>(pPixel, uint16_t((high << 11) | (g6 << 5) | low));
	}
	else if constexpr(layout == PixelFormat::Alpha_8)
	{
		pPixel[0] = (uint8_t)a;
	}
}


//____ write_pixel() _______________________________________________________
//
// Writes a pixel from 12 bits per channel (0 -> 4096) in linear color space.

template<SoftFormat format>
static inline void _write_pixel(uint8_t* pPixel, int16_t b, int16_t g, int16_t r, int16_t a)
{
	constexpr PixelFormat layout = SoftFormatInfo::pixelFormat(format);

	if constexpr(format == SoftFormat::Undefined)
	{
		int16_t* p = (int16_t*)pPixel;

		p[0] = b;
		p[1] = g;
		p[2] = r;
		p[3] = a;
	}
	else if constexpr(layout == PixelFormat::Alpha_8)
	{
		pPixel[0] = HiColor::packLinearTab[a];
	}
	else
	{
		const uint8_t* pPackTab = SoftFormatInfo::isLinear(format) ? HiColor::packLinearTab : HiColor::packSRGBTab;

		_write_pixel_fast8<format>(pPixel, pPackTab[b], pPackTab[g], pPackTab[r], HiColor::packLinearTab[a]);
	}
}

//____ _divide() _______________________________________________________________
// Division by a divisor of 0 -> 4096, by multiplying with its reciprocal from
// SoftBackend::s_reciprocalTab, which saves dividing when several values share it.
// Rounded to nearest, value plus half the divisor must be below 2^32 and the result is
// then off by at most one where it lies halfway between two values. A divisor of 0 gives 0.

static inline int16_t _divide(uint32_t value, uint32_t halfDivisor, uint32_t reciprocal)
{
	return int16_t((uint64_t(value + halfDivisor) * reciprocal) >> 32);
}

//____ _blend_over_fast8() / _blend_over() ____________________________________
// Porter-Duff over onto a pixel that isn't opaque: colors are mixed premultiplied and
// divided by the resulting alpha. Inlined, as a function call in the kernels would cost
// the common case of an opaque canvas registers.

struct BlendedPixel
{
	int16_t b, g, r, a;
};

static inline BlendedPixel _blend_over_fast8(int srcB, int srcG, int srcR, int srcA, int backB, int backG, int backR, int backA)
{
	// Alphas in 12 bits, for the table of reciprocals.

	int srcA12 = SoftBackend::s_mulTab[srcA] >> 4;
	int backWeight = ((SoftBackend::s_mulTab[backA] >> 4) * (4096 - srcA12)) >> 12;
	int outA12 = srcA12 + backWeight;

	uint32_t reciprocal = SoftBackend::s_reciprocalTab[outA12];		// 0 if outA12 is, making all 0.
	uint32_t half = outA12 / 2;

	return { _divide(srcB * srcA12 + backB * backWeight, half, reciprocal),
			 _divide(srcG * srcA12 + backG * backWeight, half, reciprocal),
			 _divide(srcR * srcA12 + backR * backWeight, half, reciprocal),
			 int16_t((outA12 * 255 + 2048) >> 12) };
}

static inline BlendedPixel _blend_over(int srcB, int srcG, int srcR, int srcA, int backB, int backG, int backR, int backA)
{
	int backWeight = (backA * (4096 - srcA)) >> 12;
	int outA = srcA + backWeight;

	uint32_t reciprocal = SoftBackend::s_reciprocalTab[outA];		// 0 if outA is, making all 0.
	uint32_t half = outA / 2;

	return { _divide(srcB * srcA + backB * backWeight, half, reciprocal),
			 _divide(srcG * srcA + backG * backWeight, half, reciprocal),
			 _divide(srcR * srcA + backR * backWeight, half, reciprocal),
			 int16_t(outA) };
}

//____ _is_effect_mode() ______________________________________________________

template<BlendMode mode>
constexpr bool _is_effect_mode()
{
	return mode == BlendMode::Add || mode == BlendMode::Subtract || mode == BlendMode::Multiply ||
		   mode == BlendMode::Invert || mode == BlendMode::Min || mode == BlendMode::Max;
}

//____ _blend_effect_on_glass() / _blend_effect() _____________________________
// Effect modes in 12 bits, see ReferenceBackend::_blendEffect() for their definitions. The source
// is given as the light it adds, its color times its alpha (s), and its alpha, which is the
// strength of the effect. Multiply and Min use the source faded towards white by it (t).
//
// On an opaque pixel the effect works on its color. On a pixel that isn't opaque it works on its
// premultiplied color, and Add, Invert and Max raise its alpha by the strongest channel of s, so
// it can hold the light they give. Multiply gives the same either way. On alpha-only canvases
// the alpha of the source is the value of the effect.
//
// _blend_effect_on_glass() takes the pixels that aren't opaque. Unlike _blend_over() for Blend it
// isn't inlined, which would slow down the common case of an opaque canvas.

template<BlendMode mode>
static WAX_NOINLINE BlendedPixel _blend_effect_on_glass(int sB, int sG, int sR, int sA, int backB, int backG, int backR, int backA)
{
	int pB = (backB * backA + 2048) >> 12;
	int pG = (backG * backA + 2048) >> 12;
	int pR = (backR * backA + 2048) >> 12;

	int strongest = std::max(sB, std::max(sG, sR));
	int outA = backA;
	int b, g, r;

	if constexpr (mode == BlendMode::Add)
	{
		b = pB + sB;
		g = pG + sG;
		r = pR + sR;
		outA = backA + strongest;
	}
	else if constexpr (mode == BlendMode::Subtract)
	{
		b = pB - sB;
		g = pG - sG;
		r = pR - sR;
	}
	else if constexpr (mode == BlendMode::Invert)
	{
		b = pB + ((sB * (4096 - 2 * pB) + 2048) >> 12);
		g = pG + ((sG * (4096 - 2 * pG) + 2048) >> 12);
		r = pR + ((sR * (4096 - 2 * pR) + 2048) >> 12);
		outA = backA + ((strongest * (4096 - backA) + 2048) >> 12);
	}
	else if constexpr (mode == BlendMode::Min)
	{
		int white = 4096 - sA;

		b = std::min(pB, white + sB);
		g = std::min(pG, white + sG);
		r = std::min(pR, white + sR);
	}
	else if constexpr (mode == BlendMode::Max)
	{
		b = std::max(pB, sB);
		g = std::max(pG, sG);
		r = std::max(pR, sR);
		outA = std::max(backA, strongest);
	}
	else
	{
		int white = 4096 - sA;

		b = (pB * (white + sB) + 2048) >> 12;
		g = (pG * (white + sG) + 2048) >> 12;
		r = (pR * (white + sR) + 2048) >> 12;
	}

	outA = std::min(outA, 4096);

	if (outA == 0)
		return { int16_t(backB), int16_t(backG), int16_t(backR), 0 };		// Still transparent, the color doesn't matter.

	uint32_t reciprocal = SoftBackend::s_reciprocalTab[outA];
	uint32_t half = outA / 2;

	auto unpremultiply = [&](int value) { return _divide(std::clamp(value, 0, outA) * 4096, half, reciprocal); };

	return { unpremultiply(b), unpremultiply(g), unpremultiply(r), int16_t(outA) };
}

template<BlendMode mode, SoftFormat destFormat>
static inline void _blend_effect(int sB, int sG, int sR, int sA, int16_t backB, int16_t backG, int16_t backR, int16_t backA,
								 int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA)
{
	if constexpr (destFormat == SoftFormat::Alpha_8)
	{
		if constexpr (mode == BlendMode::Add)
			outA = SoftBackend::s_limit4096Tab[4097 + backA + sA];
		else if constexpr (mode == BlendMode::Subtract)
			outA = SoftBackend::s_limit4096Tab[4097 + backA - sA];
		else if constexpr (mode == BlendMode::Multiply)
			outA = (backA * sA) >> 12;
		else if constexpr (mode == BlendMode::Invert)
			outA = (sA * (4096 - backA) + backA * (4096 - sA)) >> 12;
		else if constexpr (mode == BlendMode::Min)
			outA = std::min(int(backA), sA);
		else if constexpr (mode == BlendMode::Max)
			outA = std::max(int(backA), sA);
	}
	else if (mode != BlendMode::Multiply && SoftFormatInfo::hasAlpha(destFormat) && WAX_UNLIKELY(backA != 4096))
	{
		BlendedPixel out = _blend_effect_on_glass<mode>(sB, sG, sR, sA, backB, backG, backR, backA);
		outB = out.b;
		outG = out.g;
		outR = out.r;
		outA = out.a;
	}
	else
	{
		if constexpr (mode == BlendMode::Add)
		{
			outB = SoftBackend::s_limit4096Tab[4097 + backB + sB];
			outG = SoftBackend::s_limit4096Tab[4097 + backG + sG];
			outR = SoftBackend::s_limit4096Tab[4097 + backR + sR];
		}
		else if constexpr (mode == BlendMode::Subtract)
		{
			outB = SoftBackend::s_limit4096Tab[4097 + backB - sB];
			outG = SoftBackend::s_limit4096Tab[4097 + backG - sG];
			outR = SoftBackend::s_limit4096Tab[4097 + backR - sR];
		}
		else if constexpr (mode == BlendMode::Multiply)
		{
			int white = 4096 - sA;

			outB = (backB * (white + sB)) >> 12;
			outG = (backG * (white + sG)) >> 12;
			outR = (backR * (white + sR)) >> 12;
		}
		else if constexpr (mode == BlendMode::Invert)
		{
			outB = (sB * (4096 - backB) + backB * (4096 - sB)) >> 12;
			outG = (sG * (4096 - backG) + backG * (4096 - sG)) >> 12;
			outR = (sR * (4096 - backR) + backR * (4096 - sR)) >> 12;
		}
		else if constexpr (mode == BlendMode::Min)
		{
			int white = 4096 - sA;

			outB = std::min(int(backB), white + sB);
			outG = std::min(int(backG), white + sG);
			outR = std::min(int(backR), white + sR);
		}
		else if constexpr (mode == BlendMode::Max)
		{
			outB = std::max(int(backB), sB);
			outG = std::max(int(backG), sG);
			outR = std::max(int(backR), sR);
		}

		outA = backA;
	}
}

//____ _blend_effect_on_glass_fast8() _________________________________________
// _blend_effect_on_glass() for 8-bit channels, which are converted to 12 bits for it.

template<BlendMode mode>
static WAX_NOINLINE BlendedPixel _blend_effect_on_glass_fast8(int srcB, int srcG, int srcR, int srcA, int backB, int backG, int backR, int backA)
{
	auto to12 = [](int value) { return SoftBackend::s_mulTab[value] >> 4; };
	auto to8 = [](int value) { return int16_t((value * 255 + 2048) >> 12); };

	int alpha = to12(srcA);

	BlendedPixel out = _blend_effect_on_glass<mode>((to12(srcB) * alpha) >> 12, (to12(srcG) * alpha) >> 12, (to12(srcR) * alpha) >> 12, alpha,
													to12(backB), to12(backG), to12(backR), to12(backA));

	return { to8(out.b), to8(out.g), to8(out.r), to8(out.a) };
}

//____ _blend_pixels_fast8() ____________________________________________________
template<BlendMode mode, SoftFormat destFormat>
static inline void _blend_pixels_fast8(int morphFactor,
	int16_t srcB, int16_t srcG, int16_t srcR, int16_t srcA,
	int16_t backB, int16_t backG, int16_t backR, int16_t backA,
	int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
	int16_t& fixedB, int16_t& fixedG, int16_t& fixedR, int16_t& fixedA)
{
	// Note: We use if-statements instead of switch/case here since some compilers
	// won't fully eliminate the switch/case statements when this code is inlined
	// into templates where only one BlendMode is possible. With if-statements,
	// the statements are properly detected as never occuring and completely removed.

	if constexpr(destFormat == SoftFormat::Alpha_8)
	{
		// Alpha only destination. We treat alpha differently when there is no other channel.

		if constexpr(mode == BlendMode::Replace) {
			outA = srcA;
		}
		else if constexpr(mode == BlendMode::Morph)
		{
			int invMorph = 4096 - morphFactor;
			outA = (backA * invMorph + srcA * morphFactor) >> 12;
		}
		else if constexpr(mode == BlendMode::Blend)
		{
			int alpha = SoftBackend::s_mulTab[srcA];
			int invAlpha = 65536 - alpha;

			outA = (backA * invAlpha + 255 * alpha + 0x8000) >> 16;
		}
		else if constexpr(mode == BlendMode::BlendFixedColor)
		{
			int alpha = SoftBackend::s_mulTab[srcA];
			int invAlpha = 65536 - alpha;

			outA = (fixedA * invAlpha + 255 * alpha + 0x8000) >> 16;
		}
		else if constexpr(mode == BlendMode::Add)
		{
			outA = limitUint8(backA + srcA);
		}
		else if constexpr(mode == BlendMode::Subtract)
		{
			outA = limitUint8(backA - srcA);
		}
		else if constexpr(mode == BlendMode::Multiply)
		{
			outA = (SoftBackend::s_mulTab[backA] * srcA + 0x8000) >> 16;
		}
		else if constexpr(mode == BlendMode::Min)
		{
			outA = srcA - limitUint8(srcA - backA);
		}
		else if constexpr(mode == BlendMode::Max)
		{
			outA = backA + limitUint8(srcA - backA);
		}
		else if constexpr(mode == BlendMode::Invert)
		{
			int srcA2 = SoftBackend::s_mulTab[srcA];
			outA = (srcA2 * (255 - backA) + backA * (65536 - srcA2) + 0x8000) >> 16;
		}
	}
	else
	{
		if constexpr(mode == BlendMode::Replace)
		{
			outB = srcB;
			outG = srcG;
			outR = srcR;
			outA = srcA;
		}
		else if constexpr(mode == BlendMode::Morph)
		{
			// Cross-fade of premultiplied colors, so that a transparent side just fades out.

			// Weights in 12 bits, for the table of reciprocals.

			int backWeight = ((SoftBackend::s_mulTab[backA] >> 4) * (4096 - morphFactor)) >> 12;
			int srcWeight = ((SoftBackend::s_mulTab[srcA] >> 4) * morphFactor) >> 12;
			int weight = backWeight + srcWeight;

			outA = (weight * 255 + 2048) >> 12;

			if (weight == 4096)
			{
				outB = (backB * backWeight + srcB * srcWeight) >> 12;
				outG = (backG * backWeight + srcG * srcWeight) >> 12;
				outR = (backR * backWeight + srcR * srcWeight) >> 12;
			}
			else
			{
				uint32_t reciprocal = SoftBackend::s_reciprocalTab[weight];		// 0 if weight is, making all 0.
				uint32_t half = weight / 2;

				outB = _divide(backB * backWeight + srcB * srcWeight, half, reciprocal);
				outG = _divide(backG * backWeight + srcG * srcWeight, half, reciprocal);
				outR = _divide(backR * backWeight + srcR * srcWeight, half, reciprocal);
			}
		}
		else if constexpr(mode == BlendMode::Blend)
		{
			// Porter-Duff over only differs from mixing by the alpha of the source where the
			// canvas isn't opaque.

			if (SoftFormatInfo::hasAlpha(destFormat) && WAX_UNLIKELY(backA != 255))
			{
				BlendedPixel out = _blend_over_fast8(srcB, srcG, srcR, srcA, backB, backG, backR, backA);
				outB = out.b;
				outG = out.g;
				outR = out.r;
				outA = out.a;
			}
			else
			{
				int alpha = SoftBackend::s_mulTab[srcA];
				int invAlpha = 65536 - alpha;

				outB = (backB * invAlpha + srcB * alpha + 0x8000) >> 16;
				outG = (backG * invAlpha + srcG * alpha + 0x8000) >> 16;
				outR = (backR * invAlpha + srcR * alpha + 0x8000) >> 16;
				outA = (backA * invAlpha + 255 * alpha + 0x8000) >> 16;
			}
		}
		else if constexpr(mode == BlendMode::BlendFixedColor)
		{
			int alpha = SoftBackend::s_mulTab[srcA];
			int invAlpha = 65536 - alpha;

			outB = (fixedB * invAlpha + srcB * alpha + 0x8000) >> 16;
			outG = (fixedG * invAlpha + srcG * alpha + 0x8000) >> 16;
			outR = (fixedR * invAlpha + srcR * alpha + 0x8000) >> 16;
			outA = (fixedA * invAlpha + 255 * alpha + 0x8000) >> 16;
		}
		else if constexpr(_is_effect_mode<mode>() && mode != BlendMode::Multiply && SoftFormatInfo::hasAlpha(destFormat))
		{
			// Effect modes onto a pixel that isn't opaque work on its premultiplied color, see
			// _blend_effect(). Multiply gives the same either way.

			if (WAX_UNLIKELY(backA != 255))
			{
				BlendedPixel out = _blend_effect_on_glass_fast8<mode>(srcB, srcG, srcR, srcA, backB, backG, backR, backA);
				outB = out.b;
				outG = out.g;
				outR = out.r;
				outA = out.a;
			}
			else
			{
				// Opaque, as on a canvas without alpha.

				_blend_pixels_fast8<mode, SoftFormat::XRGB_8_linear>(morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
																	 outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
			}
		}
		else if constexpr(mode == BlendMode::Add)
		{
			int alpha = SoftBackend::s_mulTab[srcA];

			outB = limitUint8(backB + ((srcB * alpha + 0x8000) >> 16));
			outG = limitUint8(backG + ((srcG * alpha + 0x8000) >> 16));
			outR = limitUint8(backR + ((srcR * alpha + 0x8000) >> 16));
			outA = backA;
		}
		else if constexpr(mode == BlendMode::Subtract)
		{
			int alpha = SoftBackend::s_mulTab[srcA];

			outB = limitUint8(backB - ((srcB * alpha + 0x8000) >> 16));
			outG = limitUint8(backG - ((srcG * alpha + 0x8000) >> 16));
			outR = limitUint8(backR - ((srcR * alpha + 0x8000) >> 16));
			outA = backA;
		}
		else if constexpr(mode == BlendMode::Multiply)
		{
			// The alpha of the source is the strength of the effect: it fades the source
			// towards white, which has no effect.

			int alpha = SoftBackend::s_mulTab[srcA];
			int white = 255 - srcA;

			outB = (SoftBackend::s_mulTab[backB] * (white + ((srcB * alpha + 0x8000) >> 16)) + 0x8000) >> 16;
			outG = (SoftBackend::s_mulTab[backG] * (white + ((srcG * alpha + 0x8000) >> 16)) + 0x8000) >> 16;
			outR = (SoftBackend::s_mulTab[backR] * (white + ((srcR * alpha + 0x8000) >> 16)) + 0x8000) >> 16;
			outA = backA;
		}
		else if constexpr(mode == BlendMode::Min)
		{
			// Source faded towards white by its alpha.

			int alpha = SoftBackend::s_mulTab[srcA];
			int white = 255 - srcA;

			int b = white + ((srcB * alpha + 0x8000) >> 16);
			int g = white + ((srcG * alpha + 0x8000) >> 16);
			int r = white + ((srcR * alpha + 0x8000) >> 16);

			outB = b - limitUint8(b - backB);
			outG = g - limitUint8(g - backG);
			outR = r - limitUint8(r - backR);
			outA = backA;
		}
		else if constexpr(mode == BlendMode::Max)
		{
			// Source faded towards black by its alpha.

			int alpha = SoftBackend::s_mulTab[srcA];

			outB = backB + limitUint8(((srcB * alpha + 0x8000) >> 16) - backB);
			outG = backG + limitUint8(((srcG * alpha + 0x8000) >> 16) - backG);
			outR = backR + limitUint8(((srcR * alpha + 0x8000) >> 16) - backR);
			outA = backA;
		}
		else if constexpr(mode == BlendMode::Invert)
		{
			// Source faded towards black by its alpha.

			int alpha = SoftBackend::s_mulTab[srcA];

			int srcB2 = SoftBackend::s_mulTab[(srcB * alpha + 0x8000) >> 16];
			int srcG2 = SoftBackend::s_mulTab[(srcG * alpha + 0x8000) >> 16];
			int srcR2 = SoftBackend::s_mulTab[(srcR * alpha + 0x8000) >> 16];

			outB = (srcB2 * (255 - backB) + backB * (65536 - srcB2) + 0x8000) >> 16;
			outG = (srcG2 * (255 - backG) + backG * (65536 - srcG2) + 0x8000) >> 16;
			outR = (srcR2 * (255 - backR) + backR * (65536 - srcR2) + 0x8000) >> 16;
			outA = backA;
		}
	}
}


//____ _blend_pixels() ____________________________________________________
template<BlendMode mode, SoftFormat destFormat>
static inline void	_blend_pixels(int morphFactor,
	int16_t srcB, int16_t srcG, int16_t srcR, int16_t srcA,
	int16_t backB, int16_t backG, int16_t backR, int16_t backA,
	int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
	int16_t& fixedB, int16_t& fixedG, int16_t& fixedR, int16_t& fixedA )
{
	// Note: We use if-statements instead of switch/case here since some compilers
	// won't fully eliminate the switch/case statements when this code is inlined
	// into templates where only one BlendMode is possible. With if-statements,
	// the statements are properly detected as never occuring and completely removed.

	if constexpr(destFormat == SoftFormat::Alpha_8)
	{
		// Alpha only destination. We treat alpha differently when there is no other channel.
		if constexpr(mode == BlendMode::Replace)
		{
			outA = srcA;
		}
		else if constexpr(mode == BlendMode::Morph)
		{
			int invMorph = 4096 - morphFactor;
			outA = (backA * invMorph + srcA * morphFactor) >> 12;
		}
		else if constexpr(mode == BlendMode::Blend)
		{
			int alpha = srcA;
			int invAlpha = 4096 - alpha;

			outA = (backA * invAlpha + 4096 * alpha) >> 12;
		}
		else if constexpr(mode == BlendMode::BlendFixedColor)
		{
			int alpha = srcA;
			int invAlpha = 4096 - alpha;

			outA = (fixedA * invAlpha + 4096 * alpha) >> 12;
		}
		else if constexpr(mode == BlendMode::Add)
		{
			outA = SoftBackend::s_limit4096Tab[4097 + backA + srcA];
		}
		else if constexpr(mode == BlendMode::Subtract)
		{
			outA = SoftBackend::s_limit4096Tab[4097 + backA - srcA];
		}
		else if constexpr(mode == BlendMode::Multiply)
		{
			outA = (backA * srcA) >> 12;
		}
		else if constexpr(mode == BlendMode::Min)
		{
			outA = std::min(backA, srcA);
		}
		else if constexpr(mode == BlendMode::Max)
		{
			outA = std::max(backA, srcA);
		}
		else if constexpr(mode == BlendMode::Invert)
		{
			outA = (srcA * (4096 - backA) + backA * (4096 - srcA)) >> 12;
		}
	}
	else
	{
		if constexpr (mode == BlendMode::Replace)
		{
			outB = srcB;
			outG = srcG;
			outR = srcR;
			outA = srcA;
		}
		else if constexpr(mode == BlendMode::Morph)
		{
			// Cross-fade of premultiplied colors, so that a transparent side just fades out.

			int invMorph = 4096 - morphFactor;
			int backWeight = (backA * invMorph) >> 12;
			int srcWeight = (srcA * morphFactor) >> 12;

			outA = backWeight + srcWeight;

			if (outA == 4096)
			{
				outB = (backB * backWeight + srcB * srcWeight) >> 12;
				outG = (backG * backWeight + srcG * srcWeight) >> 12;
				outR = (backR * backWeight + srcR * srcWeight) >> 12;
			}
			else
			{
				uint32_t reciprocal = SoftBackend::s_reciprocalTab[outA];		// 0 if outA is, making all 0.
				uint32_t half = outA / 2;

				outB = _divide(backB * backWeight + srcB * srcWeight, half, reciprocal);
				outG = _divide(backG * backWeight + srcG * srcWeight, half, reciprocal);
				outR = _divide(backR * backWeight + srcR * srcWeight, half, reciprocal);
			}
		}
		else if constexpr(mode == BlendMode::Blend)
		{
			// Porter-Duff over only differs from mixing by the alpha of the source where the
			// canvas isn't opaque.

			if (SoftFormatInfo::hasAlpha(destFormat) && WAX_UNLIKELY(backA != 4096))
			{
				BlendedPixel out = _blend_over(srcB, srcG, srcR, srcA, backB, backG, backR, backA);
				outB = out.b;
				outG = out.g;
				outR = out.r;
				outA = out.a;
			}
			else
			{
				int alpha = srcA;
				int invAlpha = 4096 - alpha;

				outB = (backB * invAlpha + srcB * alpha) >> 12;
				outG = (backG * invAlpha + srcG * alpha) >> 12;
				outR = (backR * invAlpha + srcR * alpha) >> 12;
				outA = (backA * invAlpha + 4096 * alpha) >> 12;
			}
		}
		else if constexpr(mode == BlendMode::BlendFixedColor)
		{
			int alpha = srcA;
			int invAlpha = 4096 - alpha;

			outB = (fixedB * invAlpha + srcB * alpha) >> 12;
			outG = (fixedG * invAlpha + srcG * alpha) >> 12;
			outR = (fixedR * invAlpha + srcR * alpha) >> 12;
			outA = (fixedA * invAlpha + 4096 * alpha) >> 12;
		}
		else if constexpr(_is_effect_mode<mode>())
		{
			// The source faded towards black by its alpha, which is the strength of the effect.

			_blend_effect<mode, destFormat>((srcB * srcA) >> 12, (srcG * srcA) >> 12, (srcR * srcA) >> 12, srcA,
											backB, backG, backR, backA, outB, outG, outR, outA);
		}
	}
}


//____ _transparent_op() __________________________________________________
//
// What blending a transparent black source pixel (which a tint can't change) does
// to the destination in each blend mode. Used for pixels outside the source of a
// ClipBlit, where the result can often be had without reading the destination.

enum class TransparentOp
{
	Skip,		// Destination is left as is.
	Write,		// Destination gets a constant, see _write_transparent().
	Process		// Result depends on the destination, pixel must be blended as usual.
};

template<SoftFormat format>
constexpr bool _has_alpha_channel()
{
	return SoftFormatInfo::hasAlpha(format);
}

template<BlendMode mode, SoftFormat destFormat>
constexpr TransparentOp _transparent_op()
{
	if constexpr (mode == BlendMode::Blend || mode == BlendMode::Add || mode == BlendMode::Subtract ||
				  mode == BlendMode::Max || mode == BlendMode::Invert)
		return TransparentOp::Skip;
	else if constexpr (mode == BlendMode::Replace || mode == BlendMode::BlendFixedColor)
		return TransparentOp::Write;
	else if constexpr (mode == BlendMode::Multiply || mode == BlendMode::Min)
	{
		// A transparent source has no effect, except on alpha-only destinations where the
		// alpha of the source is the value: Multiply and Min with 0 gives 0.

		if constexpr (destFormat == SoftFormat::Alpha_8)
			return TransparentOp::Write;
		else
			return TransparentOp::Skip;
	}
	else
		return TransparentOp::Process;			// Morph
}

//____ _write_transparent() _______________________________________________
//
// Writes the result of blending a transparent black source pixel, for blend modes
// where _transparent_op() returns Write.

template<BlendMode mode, SoftFormat destFormat, bool bFast8>
static inline void _write_transparent(uint8_t* pDst, int16_t fixedB, int16_t fixedG, int16_t fixedR, int16_t fixedA)
{
	int16_t b = 0, g = 0, r = 0, a = 0;

	if constexpr (mode == BlendMode::BlendFixedColor)
	{
		b = fixedB;
		g = fixedG;
		r = fixedR;
		a = fixedA;
	}

	if constexpr (bFast8)
		_write_pixel_fast8<destFormat>(pDst, b, g, r, a);
	else
		_write_pixel<destFormat>(pDst, b, g, r, a);
}

//____ _color_tint_init() _________________________________________________
template<TintMode tintMode>
static inline void _color_tint_init(const SoftBackend::ColTrans& tint, int bits, int16_t inB, int16_t inG, int16_t inR, int16_t inA,
							 int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA, const HiColor*& WAX_RESTRICT pTintmapX, const HiColor*& WAX_RESTRICT pTintmapY, CoordI patchPos)
{
	if constexpr(tintMode == TintMode::None)
	{
		outR = inR;
		outG = inG;
		outB = inB;
		outA = inA;
	}
	else if constexpr(tintMode == TintMode::Flat)
	{
		outR = (inR * tint.flatTintColor.r) >> 12;
		outG = (inG * tint.flatTintColor.g) >> 12;
		outB = (inB * tint.flatTintColor.b) >> 12;
		outA = (inA * tint.flatTintColor.a) >> 12;
	}

	if constexpr(tintMode == TintMode::GradientX || tintMode == TintMode::GradientXY)
	{
		pTintmapX = tint.pTintAxisX + patchPos.x - tint.tintRect.x;
	}

	if constexpr(tintMode == TintMode::GradientY || tintMode == TintMode::GradientXY)
	{
		pTintmapY = tint.pTintAxisY + patchPos.y - tint.tintRect.y;
	}
}

//____ _color_tint_line() _________________________________________________
template<TintMode tintMode>
static inline void _color_tint_line(const SoftBackend::ColTrans& tint, int bits, int16_t inB, int16_t inG, int16_t inR, int16_t inA,
														int16_t& lineB, int16_t& lineG, int16_t& lineR, int16_t& lineA,
														int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
														const HiColor*& pTintmapX, const HiColor*& pTintmapY, CoordI patchPos)
{
	if constexpr(tintMode == TintMode::GradientX || tintMode == TintMode::GradientXY )
	{
		pTintmapX = tint.pTintAxisX + patchPos.x - tint.tintRect.x;
	}
	else if constexpr(tintMode == TintMode::GradientY)
	{
		outB = (inB * pTintmapY->b) >> 12;
		outG = (inG * pTintmapY->g) >> 12;
		outR = (inR * pTintmapY->r) >> 12;
		outA = (inA * pTintmapY->a) >> 12;

		pTintmapY++;
	}
	else if constexpr(tintMode == TintMode::GradientXY)
	{
		lineB = (inB * pTintmapY->b) >> 12;
		lineG = (inG * pTintmapY->g) >> 12;
		lineR = (inR * pTintmapY->r) >> 12;
		lineA = (inA * pTintmapY->a) >> 12;

		pTintmapY++;
	}
}


//____ _color_tint_pixel() ______________________________________________________
template<TintMode tintMode>
static inline void _color_tint_pixel(int16_t inB, int16_t inG, int16_t inR, int16_t inA,
							  int16_t& lineB, int16_t& lineG, int16_t& lineR, int16_t& lineA,
							  int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA, const HiColor*& pTintmapX)
{
	if constexpr(tintMode == TintMode::GradientX)
	{
		outB = (inB * pTintmapX->b) >> 12;
		outG = (inG * pTintmapX->g) >> 12;
		outR = (inR * pTintmapX->r) >> 12;
		outA = (inA * pTintmapX->a) >> 12;

		pTintmapX++;
	}
	else if constexpr(tintMode == TintMode::GradientXY)
	{
		outB = (lineB * pTintmapX->b) >> 12;
		outG = (lineG * pTintmapX->g) >> 12;
		outR = (lineR * pTintmapX->r) >> 12;
		outA = (lineA * pTintmapX->a) >> 12;

		pTintmapX++;
	}
}

//____ _texel_tint_init() _________________________________________________
template<TintMode tintMode>
inline void _texel_tint_init(const SoftBackend::ColTrans& tint, int bits,
							 int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
							 const HiColor*& pTintmapX, const HiColor*& pTintmapY, CoordI patchPos)
{
	if constexpr(tintMode == TintMode::Flat)
	{
		outR = tint.flatTintColor.r;
		outG = tint.flatTintColor.g;
		outB = tint.flatTintColor.b;
		outA = tint.flatTintColor.a;
	}

	if constexpr(tintMode == TintMode::GradientX || tintMode == TintMode::GradientXY)
	{
		pTintmapX = tint.pTintAxisX + patchPos.x - tint.tintRect.x;
	}

	if constexpr(tintMode == TintMode::GradientY || tintMode == TintMode::GradientXY)
	{
		pTintmapY = tint.pTintAxisY + patchPos.y - tint.tintRect.y;
	}
}

//____ _texel_tint_line() _________________________________________________
template <TintMode tintMode>
inline void _texel_tint_line(const SoftBackend::ColTrans& tint, int bits,
							 int16_t& lineB, int16_t& lineG, int16_t& lineR, int16_t& lineA,
							 int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA,
							 const HiColor*& pTintmapX, const HiColor*&  pTintmapY, CoordI patchPos)
{
	if constexpr(tintMode == TintMode::GradientX || tintMode == TintMode::GradientXY )
	{
		pTintmapX = tint.pTintAxisX + patchPos.x - tint.tintRect.x;
	}
	else if constexpr(tintMode == TintMode::GradientY)
	{
		outB = pTintmapY->b;
		outG = pTintmapY->g;
		outR = pTintmapY->r;
		outA = pTintmapY->a;

		pTintmapY++;
	}

	if constexpr(tintMode == TintMode::GradientXY )
	{
		lineB = pTintmapY->b;
		lineG = pTintmapY->g;
		lineR = pTintmapY->r;
		lineA = pTintmapY->a;

		pTintmapY++;
	}
}

//____ _texel_tint_pixel() ________________________________________________
template<TintMode tintMode>
inline void _texel_tint_pixel(int bits, int16_t& pixelB, int16_t& pixelG, int16_t& pixelR, int16_t& pixelA,
							  int16_t& lineB, int16_t& lineG, int16_t& lineR, int16_t& lineA,
							  int16_t& tintB, int16_t& tintG, int16_t& tintR, int16_t& tintA, const HiColor*& pTintmapX)
{
	if constexpr(tintMode == TintMode::None)
	{
	}
	else
	{
		if constexpr(tintMode == TintMode::GradientX)
		{
			tintB = pTintmapX->b;
			tintG = pTintmapX->g;
			tintR = pTintmapX->r;
			tintA = pTintmapX->a;
			
			pTintmapX++;
		}
		else if constexpr(tintMode == TintMode::GradientXY)
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

//____ _blend_line_edge() ____________________________________________________
//
// Blends a pixel of a line that is partly covered, coverage 0 -> 65536, see "Blending and
// transparency" in the guide. Paint modes give it the average of what they give the covered part
// and the original pixel for the rest:
//
// - Blend and BlendFixedColor fade the source by the coverage, which is the same.
// - Replace and Morph cross-fade by the coverage, times the morph factor for Morph.
// - Replace and Morph on canvases without alpha, where their result is opaque, are applied in full
//   and then mixed with the original pixel by the coverage.
//
// Effect modes fade the source by the coverage, as its alpha does. On alpha-only canvases, where
// the alpha of the source is the value, Multiply and Min fade it towards 1, which has no effect.

template<BlendMode BLEND, SoftFormat DSTFORMAT, bool bFast8>
static inline void _blend_line_edge(uint8_t* WAX_RESTRICT pDst, int coverage, int morphFactor,
									int16_t srcB, int16_t srcG, int16_t srcR, int16_t srcA,
									int16_t fixedB, int16_t fixedG, int16_t fixedR, int16_t fixedA)
{
	int16_t backB, backG, backR, backA;
	int16_t outB, outG, outR, outA;

	if constexpr (bFast8)
		_read_pixel_fast8<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
	else
		_read_pixel<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);

	auto blend = [&](auto mode, int morph, int alpha)
	{
		if constexpr (bFast8)
			_blend_pixels_fast8<decltype(mode)::value, DSTFORMAT>(morph, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
																  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
		else
			_blend_pixels<decltype(mode)::value, DSTFORMAT>(morph, srcB, srcG, srcR, alpha, backB, backG, backR, backA,
															outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
	};

	if constexpr ((BLEND == BlendMode::Replace || BLEND == BlendMode::Morph) && SoftFormatInfo::hasAlpha(DSTFORMAT))
	{
		int morph = BLEND == BlendMode::Replace ? coverage >> 4 : (morphFactor * coverage) >> 16;
		blend(std::integral_constant<BlendMode, BlendMode::Morph>(), morph, srcA);
	}
	else if constexpr (DSTFORMAT == SoftFormat::Alpha_8 && (BLEND == BlendMode::Multiply || BLEND == BlendMode::Min))
	{
		constexpr int full = bFast8 ? 255 : 4096;
		blend(std::integral_constant<BlendMode, BLEND>(), morphFactor, full - (((full - srcA) * coverage) >> 16));
	}
	else if constexpr (BLEND == BlendMode::Blend || BLEND == BlendMode::BlendFixedColor || _is_effect_mode<BLEND>())
	{
		blend(std::integral_constant<BlendMode, BLEND>(), morphFactor, (srcA * coverage) >> 16);
	}
	else
	{
		blend(std::integral_constant<BlendMode, BLEND>(), morphFactor, srcA);

		outB = backB + (((outB - backB) * coverage) >> 16);
		outG = backG + (((outG - backG) * coverage) >> 16);
		outR = backR + (((outR - backR) * coverage) >> 16);
		outA = backA + (((outA - backA) * coverage) >> 16);
	}

	if constexpr (bFast8)
		_write_pixel_fast8<DSTFORMAT>(pDst, outB, outG, outR, outA);
	else
		_write_pixel<DSTFORMAT>(pDst, outB, outG, outR, outA);
}

//____ _draw_line() _______________________________________________________

template<BlendMode BLEND, TintMode TINT, SoftFormat DSTFORMAT>
void _draw_line(uint8_t* WAX_RESTRICT pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const SoftBackend::ColTrans& tint, CoordI patchPos)
{
	constexpr bool bFast8 = SoftFormatInfo::isLinear(DSTFORMAT);


	// Step 1: Read source pixels

	int16_t srcB, srcG, srcR, srcA;

	if constexpr(bFast8)
	{
		const uint8_t* WAX_RESTRICT pPackTab = HiColor::packLinearTab;

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

	if constexpr(TINT != TintMode::None)
	{
		srcR = (srcR * tint.flatTintColor.r) >> 12;
		srcG = (srcG * tint.flatTintColor.g) >> 12;
		srcB = (srcB * tint.flatTintColor.b) >> 12;
		srcA = (srcA * tint.flatTintColor.a) >> 12;
	}

	// Step 2: Setup fixed blend color
	
	int16_t fixedB, fixedG, fixedR, fixedA;

	if constexpr(bFast8)
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

		uint8_t* WAX_RESTRICT pDst = pRow + ofs * pixelInc;

		if (beg == end)
		{
			// Special case, one pixel wide row

			_blend_line_edge<BLEND, DSTFORMAT, bFast8>(pDst, width, tint.morphFactor, srcB, srcG, srcR, srcA, fixedB, fixedG, fixedR, fixedA);
		}
		else
		{
			// First anti-aliased pixel of column

			_blend_line_edge<BLEND, DSTFORMAT, bFast8>(pDst, 65536 - (pos & 0xFFFF), tint.morphFactor, srcB, srcG, srcR, srcA, fixedB, fixedG, fixedR, fixedA);

			ofs++;
			pDst += pixelInc;

			// All non-antialiased middle pixels of column

			if (ofs < end)
			{
				do
				{
					int16_t backB, backG, backR, backA;
					int16_t outB, outG, outR, outA;

					if constexpr(bFast8)
					{
						_read_pixel_fast8<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
						_blend_pixels_fast8<BLEND, DSTFORMAT>(tint.morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
																	  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
						_write_pixel_fast8<DSTFORMAT>(pDst, outB, outG, outR, outA);
					}
					else
					{
						_read_pixel<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
						_blend_pixels<BLEND, DSTFORMAT>(tint.morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
																outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
						_write_pixel<DSTFORMAT>(pDst, outB, outG, outR, outA);
					}
					ofs++;
					pDst += pixelInc;

				} while (ofs < end);
			}

			// Last anti-aliased pixel of column

			int overflow = (pos + width) & 0xFFFF;
			if (overflow > 0)
			{
				_blend_line_edge<BLEND, DSTFORMAT, bFast8>(pDst, overflow, tint.morphFactor, srcB, srcG, srcR, srcA, fixedB, fixedG, fixedR, fixedA);
			}
		}

		pRow += rowInc;
		pos += slope;
	}
}

//____ _clip_draw_line() __________________________________________________

template<BlendMode BLEND, TintMode TINT, SoftFormat DSTFORMAT>
void _clip_draw_line(int clipStart, int clipEnd, uint8_t* WAX_RESTRICT pRow, int rowInc, int pixelInc, int length, int width, int pos, int slope, HiColor color, const SoftBackend::ColTrans& tint, CoordI patchPos)
{
	constexpr bool bFast8 = false;


	// Step 1: Read source pixels

	int16_t srcB, srcG, srcR, srcA;

	if constexpr(bFast8)
	{
		const uint8_t* WAX_RESTRICT pPackTab = HiColor::packLinearTab;

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

	if constexpr(TINT != TintMode::None)
	{
		srcR = (srcR * tint.flatTintColor.r) >> 12;
		srcG = (srcG * tint.flatTintColor.g) >> 12;
		srcB = (srcB * tint.flatTintColor.b) >> 12;
		srcA = (srcA * tint.flatTintColor.a) >> 12;
	}

	// Step 2: Setup fixed blend color
	
	int16_t fixedB, fixedG, fixedR, fixedA;

	if constexpr(bFast8)
	{
		const uint8_t* WAX_RESTRICT pPackTab = HiColor::packLinearTab;

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

		uint8_t* WAX_RESTRICT pDst = pRow + ofs * pixelInc;

		if (beg == end)
		{
			// Special case, one pixel wide row, covered by the clipped width.

			_blend_line_edge<BLEND, DSTFORMAT, bFast8>(pDst, clippedWidth, tint.morphFactor, srcB, srcG, srcR, srcA, fixedB, fixedG, fixedR, fixedA);
		}
		else
		{
			// First anti-aliased pixel of column

			_blend_line_edge<BLEND, DSTFORMAT, bFast8>(pDst, 65536 - (clippedPos & 0xFFFF), tint.morphFactor, srcB, srcG, srcR, srcA, fixedB, fixedG, fixedR, fixedA);

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

					if constexpr(bFast8)
					{
						_read_pixel_fast8<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
						_blend_pixels_fast8<BLEND, DSTFORMAT>(tint.morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
																	  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
						_write_pixel_fast8<DSTFORMAT>(pDst, outB, outG, outR, outA);
					}
					else
					{
						_read_pixel<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
						_blend_pixels<BLEND, DSTFORMAT>(tint.morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
																outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
						_write_pixel<DSTFORMAT>(pDst, outB, outG, outR, outA);
					}

					ofs++;
					pDst += pixelInc;

				} while (ofs < end);
			}

			// Last anti-aliased pixel of column

			int overflow = (clippedPos + clippedWidth) & 0xFFFF;
			if (overflow > 0)
			{
				_blend_line_edge<BLEND, DSTFORMAT, bFast8>(pDst, overflow, tint.morphFactor, srcB, srcG, srcR, srcA, fixedB, fixedG, fixedR, fixedA);
			}
		}

		pRow += rowInc;
		pos += slope;
	}
}


//____ _fill() ____________________________________________________________

template<TintMode TINT, BlendMode BLEND, SoftFormat DSTFORMAT>
void _fill(uint8_t* WAX_RESTRICT pDst, int pitchX, int pitchY, int nLines, int lineLength, HiColor col, const SoftBackend::ColTrans& tint, CoordI patchPos)
{
//	if( TINT == TintMode::None || TINT == TintMode::Flat )

	constexpr bool bFast8 = SoftFormatInfo::isLinear(DSTFORMAT);

	constexpr int bits = bFast8 ? 8 : 12;

	// Step 1: Read source pixels and prepare tint

	int16_t srcB, srcG, srcR, srcA;

	if constexpr(bFast8)
	{
		const uint8_t* WAX_RESTRICT pPackTab = HiColor::packLinearTab;

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

	const HiColor* pTintmapX;
	const HiColor* pTintmapY;

	_color_tint_init<TINT>(tint, bits, srcB, srcG, srcR, srcA, tintedB, tintedG, tintedR, tintedA,
							pTintmapX, pTintmapY, patchPos);

	// Step 2: Setup fixed blend color
	
	int16_t fixedB, fixedG, fixedR, fixedA;

	if constexpr(bFast8)
	{
		const uint8_t* WAX_RESTRICT pPackTab = HiColor::packLinearTab;

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

	// Replacing with a color that is the same along each line (no tint, flat or vertical tint)
	// writes the same pixel throughout the line. It is packed once per line and copied, keeping
	// any bytes the format doesn't write, like the X of XRGB.

	constexpr int pixelBytes = SoftFormatInfo::bytesPerPixel(DSTFORMAT);

	if constexpr (BLEND == BlendMode::Replace && DSTFORMAT != SoftFormat::Undefined && !SoftFormatInfo::isIndexed(DSTFORMAT) &&
				  (TINT == TintMode::None || TINT == TintMode::Flat || TINT == TintMode::GradientY) &&
				  (pixelBytes == 1 || pixelBytes == 2 || pixelBytes == 4))
	{
		using Pixel = std::conditional_t<pixelBytes == 4, uint32_t, std::conditional_t<pixelBytes == 2, uint16_t, uint8_t>>;

		for (int y = 0; y < nLines; y++)
		{
			int16_t lineB, lineG, lineR, lineA;

			_color_tint_line<TINT>(tint, bits, srcB, srcG, srcR, srcA,
								 lineB, lineG, lineR, lineA,
								 tintedB, tintedG, tintedR, tintedA,
								 pTintmapX, pTintmapY, patchPos);

			int16_t outB = 0, outG = 0, outR = 0, outA = 0;
			int16_t backB = 0, backG = 0, backR = 0, backA = 0;

			// Written into a cleared and a filled pixel, bytes that differ aren't written.

			Pixel cleared = 0, filled = Pixel(~Pixel(0));

			if constexpr(bFast8)
			{
				_blend_pixels_fast8<BLEND, DSTFORMAT>(tint.morphFactor, tintedB, tintedG, tintedR, tintedA, backB, backG, backR, backA,
															  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel_fast8<DSTFORMAT>((uint8_t*)&cleared, outB, outG, outR, outA);
				_write_pixel_fast8<DSTFORMAT>((uint8_t*)&filled, outB, outG, outR, outA);
			}
			else
			{
				_blend_pixels<BLEND, DSTFORMAT>(tint.morphFactor, tintedB, tintedG, tintedR, tintedA, backB, backG, backR, backA,
														outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel<DSTFORMAT>((uint8_t*)&cleared, outB, outG, outR, outA);
				_write_pixel<DSTFORMAT>((uint8_t*)&filled, outB, outG, outR, outA);
			}

			const Pixel keep = Pixel(cleared ^ filled);

			if (keep == 0 && pitchX == pixelBytes)
			{
				// Pixels next to each other, the common case, are written 16 bytes at a time.

				uint8_t pattern[16];
				for (int i = 0; i < 16; i += pixelBytes)
					std::memcpy(pattern + i, &cleared, pixelBytes);

				uint8_t* p = pDst;
				int bytes = lineLength * pixelBytes;

				for (; bytes >= 16; bytes -= 16, p += 16)
					std::memcpy(p, pattern, 16);

				for (; bytes > 0; bytes -= pixelBytes, p += pixelBytes)
					std::memcpy(p, &cleared, pixelBytes);
			}
			else
			{
				uint8_t* p = pDst;

				for (int x = 0; x < lineLength; x++, p += pitchX)
				{
					Pixel pixel;
					std::memcpy(&pixel, p, pixelBytes);
					pixel = Pixel((pixel & keep) | cleared);
					std::memcpy(p, &pixel, pixelBytes);
				}
			}

			pDst += pitchX * lineLength + pitchY;
		}
		return;
	}

	for (int y = 0; y < nLines; y++)
	{

		// Step 3: Apply any vertical tint gradient

		int16_t lineB, lineG, lineR, lineA;

		_color_tint_line<TINT>(tint, bits, srcB, srcG, srcR, srcA,
							 lineB, lineG, lineR, lineA,
							 tintedB, tintedG, tintedR, tintedA,
							 pTintmapX, pTintmapY, patchPos);

		for (int x = 0; x < lineLength; x++)
		{
			// Step 4: Apply any horizontal tint gradient

			_color_tint_pixel<TINT>(srcB, srcG, srcR, srcA,
									lineB, lineG, lineR, lineA,
									tintedB, tintedG, tintedR, tintedA,
									pTintmapX);

			// Step 5: Get color components of background pixel blending into backX
			// Step 6: Blend srcX and backX into outX
			// Step 7: Write resulting pixel to destination

			int16_t backB, backG, backR, backA;
			int16_t outB, outG, outR, outA;

			if constexpr(bFast8)
			{
				_read_pixel_fast8<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels_fast8<BLEND, DSTFORMAT>(tint.morphFactor, tintedB, tintedG, tintedR, tintedA, backB, backG, backR, backA,
															  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel_fast8<DSTFORMAT>(pDst, outB, outG, outR, outA);
			}
			else
			{
				_read_pixel<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels<BLEND, DSTFORMAT>(tint.morphFactor, tintedB, tintedG, tintedR, tintedA, backB, backG, backR, backA,
														outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel<DSTFORMAT>(pDst, outB, outG, outR, outA);
			}

			// Step 7: Increment destination pointers

			pDst += pitchX;
		}
		pDst += pitchY;
	}
}

//_____ _straight_blit() ____________________________________________________________

template<SoftFormat SRCFORMAT, TintMode TINT, BlendMode BLEND, SoftFormat DSTFORMAT, SoftBackend::ReadOp READOP>
void _straight_blit(const uint8_t* WAX_RESTRICT pSrc, uint8_t* WAX_RESTRICT pDst, const SoftSurface* WAX_RESTRICT pSrcSurf, const SoftBackend::Pitches& pitches, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos, const Transform* WAX_RESTRICT pMatrix)
{
	constexpr bool srcIsLinear = SoftFormatInfo::isLinear(SRCFORMAT);

	constexpr bool dstIsLinear = SoftFormatInfo::isLinear(DSTFORMAT);

	constexpr bool bFast8 = (SRCFORMAT != SoftFormat::Undefined &&
							 DSTFORMAT != SoftFormat::Undefined &&
							 READOP != SoftBackend::ReadOp::Blur) &&
							((srcIsLinear && dstIsLinear) ||
							 (!srcIsLinear && !dstIsLinear &&
							  TINT == TintMode::None &&
							  BLEND == BlendMode::Replace));

	constexpr int bits = bFast8 ? 8 : 12;

	// Fast track for transparent black pixels in the HiColor buffers of two-pass blits.
	// Not done for other source formats (like the ARGB_8_linear buffers), since those
	// kernels are also used for blits from ordinary surfaces, which should stay fast.

	constexpr TransparentOp transparentOp = _transparent_op<BLEND, DSTFORMAT>();
	constexpr bool bFastTransparent = SRCFORMAT == SoftFormat::Undefined &&
										READOP == SoftBackend::ReadOp::Normal && transparentOp != TransparentOp::Process;

	// Fast track for opaque and transparent pixels blended onto a canvas of the same format.
	// 8-bit channels come back unchanged when unpacked and packed again, so an opaque pixel
	// blends into a copy of itself and a transparent one leaves the canvas as it is. Copying
	// and skipping them saves the conversions, which matters for blits of cached content.

	constexpr bool bCopyOpaque = BLEND == BlendMode::Blend && TINT == TintMode::None && READOP == SoftBackend::ReadOp::Normal &&
								 SoftFormatInfo::pixelFormat(SRCFORMAT) == PixelFormat::ARGB_8 &&
								 (SoftFormatInfo::pixelFormat(DSTFORMAT) == PixelFormat::ARGB_8 || SoftFormatInfo::pixelFormat(DSTFORMAT) == PixelFormat::XRGB_8) &&
								 SoftFormatInfo::isLinear(SRCFORMAT) == SoftFormatInfo::isLinear(DSTFORMAT) &&
								 SoftFormatInfo::isBigEndian(SRCFORMAT) == SoftFormatInfo::isBigEndian(DSTFORMAT);

	// Preapare tiling and blurring

	int srcX, srcY;
	int maskX, maskY;
	int xPitch, yPitch;
	
	int	srcXstart;
	
	if constexpr (READOP == SoftBackend::ReadOp::Tile || READOP == SoftBackend::ReadOp::Blur)
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

	if constexpr (READOP == SoftBackend::ReadOp::Blur)
	{
		surfSize = pSrcSurf->pixelSize();
	}

	const Color8* WAX_RESTRICT pPalette = pSrcSurf->palette();
	const HiColor* WAX_RESTRICT pPalette4096 = pSrcSurf->palette4096();

	// Step 1: Prepare any tint gradient

	int16_t	tintB, tintG, tintR, tintA;

	const HiColor* pTintmapX;
	const HiColor* pTintmapY;

	_texel_tint_init<TINT>(tint, bits, tintB, tintG, tintR, tintA, pTintmapX, pTintmapY, patchPos);

	// Step 2: Setup fixed blend color
	
	int16_t fixedB, fixedG, fixedR, fixedA;
	
	if constexpr (bFast8)
	{
		const uint8_t* WAX_RESTRICT pPackTab = HiColor::packLinearTab;

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
		if constexpr(READOP == SoftBackend::ReadOp::Blur)
		{
			srcX = srcXstart;
		}
		
		// Step 3: Prepare tint for any vertical gradient

		int16_t lineB, lineG, lineR, lineA;

		_texel_tint_line<TINT>(tint, bits, lineB, lineG, lineR, lineA,
							   tintB, tintG, tintR, tintA, pTintmapX, pTintmapY, patchPos);

		for (int x = 0; x < lineLength; x++)
		{
			// Step 4: Read source pixels

			if constexpr(READOP == SoftBackend::ReadOp::Tile)
			{
				srcX &= maskX;
				srcY &= maskY;
				pSrc = pSrcSurf->pixels() + srcY * yPitch + srcX * xPitch;
			}

			if constexpr(bCopyOpaque)
			{
				uint8_t alpha = pSrc[_ofsA<SRCFORMAT>()];

				if (alpha == 255 || alpha == 0)
				{
					if (alpha == 255)
					{
						if constexpr(SoftFormatInfo::pixelFormat(DSTFORMAT) == PixelFormat::ARGB_8)
						{
							// Copy all opaque pixels in a row at once, unless flipped.

							if (pitches.srcX == 4 && pitches.dstX == 4)
							{
								int run = 1;
								while (x + run < lineLength && pSrc[run * 4 + _ofsA<SRCFORMAT>()] == 255)
									run++;

								std::memcpy(pDst, pSrc, run * 4);

								pSrc += run * 4;
								pDst += run * 4;
								x += run - 1;
								continue;
							}

							std::memcpy(pDst, pSrc, 4);
						}
						else
						{
							pDst[_ofsB<DSTFORMAT>()] = pSrc[_ofsB<SRCFORMAT>()];
							pDst[_ofsG<DSTFORMAT>()] = pSrc[_ofsG<SRCFORMAT>()];
							pDst[_ofsR<DSTFORMAT>()] = pSrc[_ofsR<SRCFORMAT>()];
						}
					}
					else if constexpr(SoftFormatInfo::pixelFormat(DSTFORMAT) == PixelFormat::ARGB_8)
					{
						// Blending onto a fully transparent pixel clears it.

						if (pDst[_ofsA<DSTFORMAT>()] == 0)
							std::memset(pDst, 0, 4);
					}

					pSrc += pitches.srcX;
					pDst += pitches.dstX;
					continue;
				}
			}

			int16_t srcB, srcG, srcR, srcA;

			if constexpr(READOP == SoftBackend::ReadOp::Blur)
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

					_read_pixel<SRCFORMAT>(pSrc + ofs.y * yPitch + ofs.x * xPitch, pPalette, pPalette4096, inB[i], inG[i], inR[i], inA[i]);
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
				if constexpr(bFast8)
				{
					_read_pixel_fast8<SRCFORMAT>(pSrc, pPalette, pPalette4096, srcB, srcG, srcR, srcA);
				}
				else
					_read_pixel<SRCFORMAT>(pSrc, pPalette, pPalette4096, srcB, srcG, srcR, srcA);
			}


			// Step 5: Apply any tint

			_texel_tint_pixel<TINT>(bits, srcB, srcG, srcR, srcA,
									lineB, lineG, lineR, lineA,
									tintB, tintG, tintR, tintA, pTintmapX);

			// Transparent black pixels are skipped or written directly when the blend mode
			// allows it. They are common in the buffers blitted by the second pass of a
			// two-pass ClipBlit, where everything outside the source is transparent black.
			// The raw source pixel is tested, which is cheaper than testing the channels.

			if constexpr(bFastTransparent)
			{
				if (*(const uint64_t*)pSrc == 0)
				{
					if constexpr(transparentOp == TransparentOp::Write)
						_write_transparent<BLEND, DSTFORMAT, bFast8>(pDst, fixedB, fixedG, fixedR, fixedA);

					pSrc += pitches.srcX;
					pDst += pitches.dstX;
					continue;
				}
			}

			// Step 6: Get color components of background pixel blending into backX
			// Step 7: Blend srcX and backX into outX
			// Step 8: Write resulting pixel to destination

			int16_t backB, backG, backR, backA;
			int16_t outB, outG, outR, outA;

			if constexpr(bFast8)
			{
				_read_pixel_fast8<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels_fast8<BLEND, DSTFORMAT>(tint.morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
															  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel_fast8<DSTFORMAT>(pDst, outB, outG, outR, outA);
			}
			else
			{
				_read_pixel<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels<BLEND, DSTFORMAT>(tint.morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
														outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel<DSTFORMAT>(pDst, outB, outG, outR, outA);
			}

			// Step 9: Increment source and destination pointers

			if constexpr(READOP == SoftBackend::ReadOp::Tile || READOP == SoftBackend::ReadOp::Blur)
			{
				srcX += pMatrix->xx;
				srcY += pMatrix->xy;
			}
			else
				pSrc += pitches.srcX;

			pDst += pitches.dstX;
		}

		if constexpr(READOP == SoftBackend::ReadOp::Tile || READOP == SoftBackend::ReadOp::Blur)
		{
			srcX += pMatrix->yx;
			srcY += pMatrix->yy;
		}
		else
			pSrc += pitches.srcY;

		pDst += pitches.dstY;
	}
}

//____ _bilinear_mix() ____________________________________________________
//
// Interpolates a 2x2 square of samples into one color. Weights of the samples are in
// 12 bits, together exactly 4096. Shared by _transform_blit() and _scale_blit_bilinear(),
// which must give the same result.

struct Texel
{
	int16_t b, g, r, a;
};

template<bool bOpaqueSamples, bool bFast8>
static inline void _bilinear_mix(int fracX, int fracY, const Texel& s11, const Texel& s12, const Texel& s21, const Texel& s22,
								 int16_t& outB, int16_t& outG, int16_t& outR, int16_t& outA)
{
	int mul22 = (fracX * fracY) >> 12;
	int mul12 = fracX - mul22;
	int mul21 = fracY - mul22;
	int mul11 = 4096 - fracX - fracY + mul22;

	if constexpr (bOpaqueSamples)
	{
		// All samples are opaque.

		outB = (s11.b * mul11 + s12.b * mul12 + s21.b * mul21 + s22.b * mul22 + 2048) >> 12;
		outG = (s11.g * mul11 + s12.g * mul12 + s21.g * mul21 + s22.g * mul22 + 2048) >> 12;
		outR = (s11.r * mul11 + s12.r * mul12 + s21.r * mul21 + s22.r * mul22 + 2048) >> 12;
		outA = s11.a;
	}
	else
	{
		// Colors are interpolated as premultiplied colors, so that transparent samples, like
		// those outside a ClipBlit, don't bleed their color into their neighbours. Their
		// weights are also in 12 bits, together at most 4096, for the table of reciprocals.

		int alpha11 = mul11 * s11.a;
		int alpha12 = mul12 * s12.a;
		int alpha21 = mul21 * s21.a;
		int alpha22 = mul22 * s22.a;

		outA = (alpha11 + alpha12 + alpha21 + alpha22 + 2048) >> 12;

		constexpr int alphaBits = bFast8 ? 8 : 12;

		int w11 = alpha11 >> alphaBits;
		int w12 = alpha12 >> alphaBits;
		int w21 = alpha21 >> alphaBits;
		int w22 = alpha22 >> alphaBits;
		int weight = w11 + w12 + w21 + w22;

		uint32_t reciprocal = SoftBackend::s_reciprocalTab[weight];		// 0 if weight is, making all 0.
		uint32_t half = weight / 2;

		outB = _divide(s11.b * w11 + s12.b * w12 + s21.b * w21 + s22.b * w22, half, reciprocal);
		outG = _divide(s11.g * w11 + s12.g * w12 + s21.g * w21 + s22.g * w22, half, reciprocal);
		outR = _divide(s11.r * w11 + s12.r * w12 + s21.r * w21 + s22.r * w22, half, reciprocal);
	}
}

//____ _scale_blit_bilinear() _____________________________________________
//
// Fast track of _transform_blit() for bilinear scaling without rotation and flipping
// that doesn't shrink horizontally, as when stretching an image to a larger size. The
// sampled rows of the source are unpacked once into a buffer, instead of each pixel
// unpacking its four samples, and pixels are written without reading the canvas when
// blending can't change the result. Gives exactly the same result as _transform_blit().

template<SoftFormat SRCFORMAT, BlendMode BLEND, SoftFormat DSTFORMAT, bool bFast8>
void _scale_blit_bilinear(const SoftSurface* WAX_RESTRICT pSrcSurf, BinalCoord pos, binalInt pixelIncX, binalInt lineIncY,
						  uint8_t* WAX_RESTRICT pDst, int dstPitchX, int dstPitchY, int nLines, int lineLength, const SoftBackend::ColTrans& tint,
						  int16_t fixedB, int16_t fixedG, int16_t fixedR, int16_t fixedA)
{
	if (nLines <= 0 || lineLength <= 0)
		return;

	int srcPixelBytes = pSrcSurf->pixelBits() / 8;
	int srcPitch = pSrcSurf->pitch();

	const Color8* WAX_RESTRICT pPalette = pSrcSurf->palette();
	const HiColor* WAX_RESTRICT pPalette4096 = pSrcSurf->palette4096();

	// Columns of the source that are sampled, same as by _transform_blit(): the right
	// neighbour of a sample is only read when it has a weight.

	binalInt lastX = pos.x + pixelIncX * (lineLength - 1);
	int firstColumn = int(pos.x >> BINAL_SHIFT);
	int nColumns = int(lastX >> BINAL_SHIFT) + ((lastX & BINAL_MASK) != 0 ? 1 : 0) - firstColumn + 1;

	// Buffer for two unpacked rows, which are kept as long as they are sampled.

	int bufferBytes = nColumns * 4 * int(sizeof(int16_t)) * 2;
	int16_t* pBuffer = (int16_t*)Core::memStackAlloc(bufferBytes);

	int16_t* pRows[2] = { pBuffer, pBuffer + nColumns * 4 };
	int rowIndex[2] = { -1, -1 };

	auto unpackRow = [&](int row, int keepRow) -> const int16_t*
	{
		if (rowIndex[0] == row)
			return pRows[0];
		if (rowIndex[1] == row)
			return pRows[1];

		int slot = rowIndex[0] == keepRow ? 1 : 0;

		int16_t* p = pRows[slot];
		const uint8_t* pPixel = pSrcSurf->pixels() + row * srcPitch + firstColumn * srcPixelBytes;

		for (int i = 0; i < nColumns; i++)
		{
			if constexpr (bFast8)
				_read_pixel_fast8<SRCFORMAT>(pPixel, pPalette, pPalette4096, p[0], p[1], p[2], p[3]);
			else
				_read_pixel<SRCFORMAT>(pPixel, pPalette, pPalette4096, p[0], p[1], p[2], p[3]);

			pPixel += srcPixelBytes;
			p += 4;
		}

		rowIndex[slot] = row;
		return pRows[slot];
	};

	constexpr bool bOpaqueSamples = !SoftFormatInfo::hasAlpha(SRCFORMAT);
	constexpr int16_t fullAlpha = bFast8 ? 255 : 4096;

	for (int y = 0; y < nLines; y++)
	{
		binalInt ofsY = pos.y + lineIncY * y;

		int row = int(ofsY >> BINAL_SHIFT);
		int nextRow = (ofsY & BINAL_MASK) == 0 ? row : row + 1;
		int fracY = int((ofsY & BINAL_MASK) >> (BINAL_SHIFT - 12));

		const int16_t* pRow1 = unpackRow(row, nextRow);
		const int16_t* pRow2 = nextRow == row ? pRow1 : unpackRow(nextRow, row);

		binalInt ofsX = pos.x;

		for (int x = 0; x < lineLength; x++)
		{
			int column = int(ofsX >> BINAL_SHIFT) - firstColumn;
			int next = (ofsX & BINAL_MASK) == 0 ? 0 : 4;
			int fracX = int((ofsX & BINAL_MASK) >> (BINAL_SHIFT - 12));

			const int16_t* p1 = pRow1 + column * 4;
			const int16_t* p2 = pRow2 + column * 4;

			int16_t srcB, srcG, srcR, srcA;

			// Opaque samples in 12 bits weigh as much as samples of an opaque source, which saves
			// weighting them by alpha. (In 8 bits the weights are rounded, so they still need to.)

			if (!bOpaqueSamples && !bFast8 && (p1[3] & p1[next + 3] & p2[3] & p2[next + 3]) == 4096)
			{
				int mul22 = (fracX * fracY) >> 12;
				int mul12 = fracX - mul22;
				int mul21 = fracY - mul22;
				int mul11 = 4096 - fracX - fracY + mul22;

				constexpr uint32_t reciprocal = 0xFFFFFFFFu / 4096;		// Total weight is 4096, as in _bilinear_mix().

				srcB = _divide(p1[0] * mul11 + p1[next] * mul12 + p2[0] * mul21 + p2[next] * mul22, 2048, reciprocal);
				srcG = _divide(p1[1] * mul11 + p1[next + 1] * mul12 + p2[1] * mul21 + p2[next + 1] * mul22, 2048, reciprocal);
				srcR = _divide(p1[2] * mul11 + p1[next + 2] * mul12 + p2[2] * mul21 + p2[next + 2] * mul22, 2048, reciprocal);
				srcA = 4096;
			}
			else
				_bilinear_mix<bOpaqueSamples, bFast8>(fracX, fracY,
					{ p1[0], p1[1], p1[2], p1[3] }, { p1[next], p1[next + 1], p1[next + 2], p1[next + 3] },
					{ p2[0], p2[1], p2[2], p2[3] }, { p2[next], p2[next + 1], p2[next + 2], p2[next + 3] },
					srcB, srcG, srcR, srcA);

			// Replacing, or blending an opaque color, gives the color whatever the canvas holds.

			if (BLEND == BlendMode::Replace || (BLEND == BlendMode::Blend && srcA == fullAlpha))
			{
				if constexpr (bFast8)
					_write_pixel_fast8<DSTFORMAT>(pDst, srcB, srcG, srcR, srcA);
				else
					_write_pixel<DSTFORMAT>(pDst, srcB, srcG, srcR, srcA);
			}
			else
			{
				int16_t backB, backG, backR, backA;
				int16_t outB, outG, outR, outA;

				if constexpr (bFast8)
				{
					_read_pixel_fast8<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
					_blend_pixels_fast8<BLEND, DSTFORMAT>(tint.morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
														  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
					_write_pixel_fast8<DSTFORMAT>(pDst, outB, outG, outR, outA);
				}
				else
				{
					_read_pixel<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
					_blend_pixels<BLEND, DSTFORMAT>(tint.morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
													outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
					_write_pixel<DSTFORMAT>(pDst, outB, outG, outR, outA);
				}
			}

			ofsX += pixelIncX;
			pDst += dstPitchX;
		}
		pDst += dstPitchY;
	}

	Core::memStackFree(bufferBytes);
}

//____ _transform_blit __________________________________________

template<SoftFormat SRCFORMAT, SampleMethod SAMPLEMETHOD, TintMode TINT, BlendMode BLEND, SoftFormat DSTFORMAT, SoftBackend::ReadOp READOP>
void _transform_blit(const SoftSurface* WAX_RESTRICT pSrcSurf, BinalCoord pos, const binalInt matrix[2][2], uint8_t* WAX_RESTRICT pDst, int dstPitchX, int dstPitchY, int nLines, int lineLength, const SoftBackend::ColTrans& tint, CoordI patchPos)
{
	constexpr bool srcIsLinear = SoftFormatInfo::isLinear(SRCFORMAT);

	constexpr bool dstIsLinear = SoftFormatInfo::isLinear(DSTFORMAT);

	// sRGB pixels can be copied as they are, but not interpolated, since that must be
	// done on linear values.

	constexpr bool bFast8 = ((srcIsLinear && dstIsLinear) || (!srcIsLinear && !dstIsLinear && TINT == TintMode::None && BLEND == BlendMode::Replace && SAMPLEMETHOD == SampleMethod::Nearest)) &&
							(SRCFORMAT != SoftFormat::Undefined && DSTFORMAT != SoftFormat::Undefined && READOP != SoftBackend::ReadOp::Blur);

	constexpr int bits = bFast8 ? 8 : 12;

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

	const Color8* WAX_RESTRICT pPalette = pSrcSurf->palette();
	const HiColor* WAX_RESTRICT pPalette4096 = pSrcSurf->palette4096();

	// Step 1: Setup fixed blend color
	
	int16_t fixedB, fixedG, fixedR, fixedA;

	if constexpr(bFast8)
	{
		const uint8_t* WAX_RESTRICT pPackTab = HiColor::packLinearTab;

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

	// Bilinear scaling without rotation and flipping that doesn't shrink horizontally
	// has a fast track.

	if constexpr(SAMPLEMETHOD == SampleMethod::Bilinear && READOP == SoftBackend::ReadOp::Normal && SRCFORMAT != SoftFormat::Undefined)
	{
		if (pixelIncY == 0 && lineIncX == 0 && pixelIncX > 0 && pixelIncX <= BINAL_MUL)
		{
			_scale_blit_bilinear<SRCFORMAT, BLEND, DSTFORMAT, bFast8>(pSrcSurf, pos, pixelIncX, lineIncY, pDst, dstPitchX, dstPitchY,
																	  nLines, lineLength, tint, fixedB, fixedG, fixedR, fixedA);
			return;
		}
	}

	for (int y = 0; y < nLines; y++)
	{
		binalInt ofsX = (binalInt)(pos.x + lineIncX * y);
		binalInt ofsY = (binalInt)(pos.y + lineIncY * y);		// We use 15 binals for all calculations


		for (int x = 0; x < lineLength; x++)
		{
			if constexpr(READOP == SoftBackend::ReadOp::Tile)
			{
				ofsX &= srcPosMaskX_binals;
				ofsY &= srcPosMaskY_binals;
			}

			// Step 2: Read source color.

			int16_t srcB, srcG, srcR, srcA;
			uint8_t* WAX_RESTRICT p = pSrcSurf->pixels() + (ofsY >> BINAL_SHIFT) * srcPitch + (ofsX >> BINAL_SHIFT) * srcPixelBytes;

			if constexpr(SAMPLEMETHOD == SampleMethod::Bilinear)
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
						// Totally outside, so the source is transparent black. Skip or write the
						// pixel when the blend mode allows it, otherwise blend it as usual.

						constexpr TransparentOp op = _transparent_op<BLEND, DSTFORMAT>();

						if constexpr(op != TransparentOp::Process)
						{
							if constexpr(op == TransparentOp::Write)
								_write_transparent<BLEND, DSTFORMAT, bFast8>(pDst, fixedB, fixedG, fixedR, fixedA);

							ofsX += pixelIncX;
							ofsY += pixelIncY;
							pDst += dstPitchX;
							continue;
						}
						else
						{
							src11_b = src11_g = src11_r = src11_a = 0;
							src12_b = src12_g = src12_r = src12_a = 0;
							src21_b = src21_g = src21_r = src21_a = 0;
							src22_b = src22_g = src22_r = src22_a = 0;
						}
					}
					else
					{
						// At least one of the four sample pixels is within source surface

						if constexpr(bFast8)
						{
							if (ofsX >= 0 && ofsY >= 0 && ofsX < srcMax_w && ofsY < srcMax_h)
								_read_pixel_fast8<SRCFORMAT>(p, pPalette, pPalette4096, src11_b, src11_g, src11_r, src11_a);
							else
								src11_b = src11_g = src11_r = src11_a = 0;

							if ((ofsX + BINAL_MUL) >= 0 && ofsY >= 0 && (ofsX + BINAL_MUL) < srcMax_w && ofsY < srcMax_h)
								_read_pixel_fast8<SRCFORMAT>(p + srcPixelBytes, pPalette, pPalette4096, src12_b, src12_g, src12_r, src12_a);
							else
								src12_b = src12_g = src12_r = src12_a = 0;

							if (ofsX >= 0 && (ofsY + BINAL_MUL) >= 0 && ofsX < srcMax_w && (ofsY + BINAL_MUL) < srcMax_h)
								_read_pixel_fast8<SRCFORMAT>(p + srcPitch, pPalette, pPalette4096, src21_b, src21_g, src21_r, src21_a);
							else
								src21_b = src21_g = src21_r = src21_a = 0;

							if ((ofsX + BINAL_MUL) >= 0 && (ofsY + BINAL_MUL) >= 0 && (ofsX + BINAL_MUL) < srcMax_w && (ofsY + BINAL_MUL) < srcMax_h)
								_read_pixel_fast8<SRCFORMAT>(p + srcPitch + srcPixelBytes, pPalette, pPalette4096, src22_b, src22_g, src22_r, src22_a);
							else
								src22_b = src22_g = src22_r = src22_a = 0;
						}
						else
						{
							if (ofsX >= 0 && ofsY >= 0 && ofsX < srcMax_w && ofsY < srcMax_h)
								_read_pixel<SRCFORMAT>(p, pPalette, pPalette4096, src11_b, src11_g, src11_r, src11_a);
							else
								src11_b = src11_g = src11_r = src11_a = 0;

							if ((ofsX + BINAL_MUL) >= 0 && ofsY >= 0 && (ofsX + BINAL_MUL) < srcMax_w && ofsY < srcMax_h)
								_read_pixel<SRCFORMAT>(p + srcPixelBytes, pPalette, pPalette4096, src12_b, src12_g, src12_r, src12_a);
							else
								src12_b = src12_g = src12_r = src12_a = 0;

							if (ofsX >= 0 && (ofsY + BINAL_MUL) >= 0 && ofsX < srcMax_w && (ofsY + BINAL_MUL) < srcMax_h)
								_read_pixel<SRCFORMAT>(p + srcPitch, pPalette, pPalette4096, src21_b, src21_g, src21_r, src21_a);
							else
								src21_b = src21_g = src21_r = src21_a = 0;

							if ((ofsX + BINAL_MUL) >= 0 && (ofsY + BINAL_MUL) >= 0 && (ofsX + BINAL_MUL) < srcMax_w && (ofsY + BINAL_MUL) < srcMax_h)
								_read_pixel<SRCFORMAT>(p + srcPitch + srcPixelBytes, pPalette, pPalette4096, src22_b, src22_g, src22_r, src22_a);
							else
								src22_b = src22_g = src22_r = src22_a = 0;
						}
					}
				}
				else
				{
					uint8_t* p2, * p3, * p4;

					if constexpr(READOP == SoftBackend::ReadOp::Tile)
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

					if constexpr(bFast8)
					{
						_read_pixel_fast8<SRCFORMAT>(p, pPalette, pPalette4096, src11_b, src11_g, src11_r, src11_a);
						_read_pixel_fast8<SRCFORMAT>(p2, pPalette, pPalette4096, src12_b, src12_g, src12_r, src12_a);
						_read_pixel_fast8<SRCFORMAT>(p3, pPalette, pPalette4096, src21_b, src21_g, src21_r, src21_a);
						_read_pixel_fast8<SRCFORMAT>(p4, pPalette, pPalette4096, src22_b, src22_g, src22_r, src22_a);
					}
					else
					{
						_read_pixel<SRCFORMAT>(p, pPalette, pPalette4096, src11_b, src11_g, src11_r, src11_a);
						_read_pixel<SRCFORMAT>(p2, pPalette, pPalette4096, src12_b, src12_g, src12_r, src12_a);
						_read_pixel<SRCFORMAT>(p3, pPalette, pPalette4096, src21_b, src21_g, src21_r, src21_a);
						_read_pixel<SRCFORMAT>(p4, pPalette, pPalette4096, src22_b, src22_g, src22_r, src22_a);
					}
				}

				// Interpolate our 2x2 source colors into one source color, srcX.

				int fracX = int((ofsX & BINAL_MASK) >> (BINAL_SHIFT - 12));
				int fracY = int((ofsY & BINAL_MASK) >> (BINAL_SHIFT - 12));

				constexpr bool bOpaqueSamples = !SoftFormatInfo::hasAlpha(SRCFORMAT) && READOP != SoftBackend::ReadOp::Clip;

				_bilinear_mix<bOpaqueSamples, bFast8>(fracX, fracY,
					{ src11_b, src11_g, src11_r, src11_a }, { src12_b, src12_g, src12_r, src12_a },
					{ src21_b, src21_g, src21_r, src21_a }, { src22_b, src22_g, src22_r, src22_a },
					srcB, srcG, srcR, srcA);
			}
			else
			{
				if constexpr(READOP == SoftBackend::ReadOp::Blur)
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

						_read_pixel<SRCFORMAT>(p + ofs.y * srcPitch + ofs.x * srcPixelBytes, pPalette, pPalette4096, inB[i], inG[i], inR[i], inA[i]);
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
					// Outside, so the source is transparent black. Skip or write the pixel
					// when the blend mode allows it, otherwise blend it as usual.

					constexpr TransparentOp op = _transparent_op<BLEND, DSTFORMAT>();

					if constexpr(op != TransparentOp::Process)
					{
						if constexpr(op == TransparentOp::Write)
							_write_transparent<BLEND, DSTFORMAT, bFast8>(pDst, fixedB, fixedG, fixedR, fixedA);

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
					if constexpr(bFast8)
						_read_pixel_fast8<SRCFORMAT>(p, pPalette, pPalette4096, srcB, srcG, srcR, srcA);
					else
						_read_pixel<SRCFORMAT>(p, pPalette, pPalette4096, srcB, srcG, srcR, srcA);
				}
			}

			// Step 3: Get color components of background pixel blending into backX
			// Step 4: Blend srcX and backX into outX
			// Step 5: Write resulting pixel to destination

			int16_t backB, backG, backR, backA;
			int16_t outB, outG, outR, outA;

			if constexpr(bFast8)
			{
				_read_pixel_fast8<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels_fast8<BLEND, DSTFORMAT>(tint.morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
															  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel_fast8<DSTFORMAT>(pDst, outB, outG, outR, outA);
			}
			else
			{
				_read_pixel<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
				_blend_pixels<BLEND, DSTFORMAT>(tint.morphFactor, srcB, srcG, srcR, srcA, backB, backG, backR, backA,
														outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
				_write_pixel<DSTFORMAT>(pDst, outB, outG, outR, outA);
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

template<SoftBackend::StripSource SOURCE>
static inline void _add_segment_color(int blendFraction, int offset, const int16_t* WAX_RESTRICT pSegmentColor, const HiColor* WAX_RESTRICT pSegmentTintmap, int& accB, int& accG, int& accR, int& accA)
{
	if constexpr(SOURCE == SoftBackend::StripSource::Colors)
	{
		accB += (blendFraction * pSegmentColor[0]) >> 4;
		accG += (blendFraction * pSegmentColor[1]) >> 4;
		accR += (blendFraction * pSegmentColor[2]) >> 4;
		accA += blendFraction << 8;
	}
	else if constexpr(SOURCE == SoftBackend::StripSource::Tintmaps)
	{
		accB += (blendFraction * pSegmentTintmap[offset].b) >> 4;
		accG += (blendFraction * pSegmentTintmap[offset].g) >> 4;
		accR += (blendFraction * pSegmentTintmap[offset].r) >> 4;
		accA += blendFraction << 8;
	}
	else if constexpr(SOURCE == SoftBackend::StripSource::ColorsAndTintmaps)
	{
		accB += (blendFraction >> 4) * ((pSegmentTintmap[offset].b * pSegmentColor[0]) >> 12);
		accG += (blendFraction >> 4) * ((pSegmentTintmap[offset].g * pSegmentColor[1]) >> 12);
		accR += (blendFraction >> 4) * ((pSegmentTintmap[offset].r * pSegmentColor[2]) >> 12);
		accA += blendFraction << 8;
	}
}

//____ _draw_segment_strip() _______________________________________________
template<SoftBackend::StripSource SOURCE, BlendMode BLEND, SoftFormat DSTFORMAT>
void _draw_segment_strip(int colBeg, int colEnd, uint8_t* WAX_RESTRICT pStripStart, int pixelPitch, int nEdges, SoftBackend::SegmentEdge* WAX_RESTRICT pEdges, const int16_t* WAX_RESTRICT pSegmentColors, const HiColor* WAX_RESTRICT pSegmentTintmap, int segmentTintmapPitch, const bool* WAX_RESTRICT pTransparentSegments, const bool* WAX_RESTRICT pOpaqueSegments, const SoftBackend::ColTrans& tint)
{
	constexpr bool bFast8 = SoftFormatInfo::isLinear(DSTFORMAT);

	constexpr int bits = bFast8 ? 8 : 12;

	// Setup fixed blend color

	int16_t fixedB, fixedG, fixedR, fixedA;

	if constexpr(bFast8)
	{
		const uint8_t* WAX_RESTRICT pPackTab = HiColor::packLinearTab;

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
	uint8_t* WAX_RESTRICT pDst = pStripStart + (offset >> 8) * pixelPitch;

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

				if constexpr(SOURCE == SoftBackend::StripSource::Colors)
				{
					if constexpr(bFast8)
					{
						inB = HiColor::packLinearTab[pSegmentColors[0]];
						inG = HiColor::packLinearTab[pSegmentColors[1]];
						inR = HiColor::packLinearTab[pSegmentColors[2]];
						inA = HiColor::packLinearTab[pSegmentColors[3]];
					}
					else
					{
						inB = pSegmentColors[0];
						inG = pSegmentColors[1];
						inR = pSegmentColors[2];
						inA = pSegmentColors[3];
					}
				}

				if (*pOpaqueSegments)
				{
					while (offset + 255 < end)
					{
						if constexpr(SOURCE == SoftBackend::StripSource::Tintmaps)
						{
							if constexpr(bFast8)
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

						if constexpr(SOURCE == SoftBackend::StripSource::ColorsAndTintmaps)
						{
							if constexpr(bFast8)
							{
								inB = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].b * pSegmentColors[0]) >> 12];
								inG = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].g * pSegmentColors[1]) >> 12];
								inR = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].r * pSegmentColors[2]) >> 12];
								inA = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].a * pSegmentColors[3]) >> 12];
							}
							else
							{
								inB = (pSegmentTintmap[offset >> 8].b * pSegmentColors[0]) >> 12;
								inG = (pSegmentTintmap[offset >> 8].g * pSegmentColors[1]) >> 12;
								inR = (pSegmentTintmap[offset >> 8].r * pSegmentColors[2]) >> 12;
								inA = (pSegmentTintmap[offset >> 8].a * pSegmentColors[3]) >> 12;
							}
						}

						if constexpr(bFast8)
						{
							_write_pixel_fast8<DSTFORMAT>(pDst, inB, inG, inR, inA);
						}
						else
						{
							_write_pixel<DSTFORMAT>(pDst, inB, inG, inR, inA);
						}

						pDst += pixelPitch;
						offset += 256;
					}
				}
				else
				{
					while (offset + 255 < end)
					{
						if constexpr(SOURCE == SoftBackend::StripSource::Tintmaps)
						{
							if constexpr(bFast8)
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
						else if constexpr(SOURCE == SoftBackend::StripSource::ColorsAndTintmaps)
						{
							if constexpr(bFast8)
							{
								inB = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].b * pSegmentColors[0]) >> 12];
								inG = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].g * pSegmentColors[1]) >> 12];
								inR = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].r * pSegmentColors[2]) >> 12];
								inA = HiColor::packLinearTab[(pSegmentTintmap[offset >> 8].a * pSegmentColors[3]) >> 12];
							}
							else
							{
								inB = (pSegmentTintmap[offset >> 8].b * pSegmentColors[0]) >> 12;
								inG = (pSegmentTintmap[offset >> 8].g * pSegmentColors[1]) >> 12;
								inR = (pSegmentTintmap[offset >> 8].r * pSegmentColors[2]) >> 12;
								inA = (pSegmentTintmap[offset >> 8].a * pSegmentColors[3]) >> 12;
							}
						}

						int16_t backB, backG, backR, backA;
						int16_t outB, outG, outR, outA;

						if constexpr(bFast8)
						{
							_read_pixel_fast8<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
							_blend_pixels_fast8<BLEND, DSTFORMAT>(tint.morphFactor, inB, inG, inR, inA, backB, backG, backR, backA,
																		  outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
							_write_pixel_fast8<DSTFORMAT>(pDst, outB, outG, outR, outA);
						}
						else
						{
							_read_pixel<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);
							_blend_pixels<BLEND, DSTFORMAT>(tint.morphFactor, inB, inG, inR, inA, backB, backG, backR, backA,
															outB, outG, outR, outA, fixedB, fixedG, fixedR, fixedA);
							_write_pixel<DSTFORMAT>(pDst, outB, outG, outR, outA);
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
							+ (256 - (pEdges[edge].end & 0xFF)) * (65536 / 256);
					}

					frac = std::min( frac, remainingFractions );

					segmentFractions[edge] = remainingFractions - frac;

					remainingFractions = frac;
					edge++;
				}

				segmentFractions[edge] = remainingFractions;

				int16_t backB, backG, backR, backA;

				_read_pixel<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);

				int16_t outB = 0, outG = 0, outR = 0, outA = 0;

				int accB = 0;
				int accG = 0;
				int accR = 0;
				int accA = 0;

				if constexpr(BLEND == BlendMode::Replace)
				{
					// Segments are weighted by both coverage and alpha, giving us a premultiplied
					// color which we then convert back to straight alpha.

					for (int i = 0; i <= edge; i++)
					{
						int alpha;

						if constexpr( SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if constexpr( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = (pSegmentTintmap + i * segmentTintmapPitch)[offset >> 8].a;
						if constexpr( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * (pSegmentTintmap + i * segmentTintmapPitch)[offset >> 8].a / 4096;

						// On canvases without alpha the result of each segment is opaque, so they are weighted
						// by coverage only.

						int blendFraction = SoftFormatInfo::hasAlpha(DSTFORMAT) ? ((segmentFractions[i] * alpha) / 4096) : segmentFractions[i];
						_add_segment_color<SOURCE>(blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					outA = accA >> 12;

					if (accA == 65536 << 8)
					{
						outB = accB >> 12;
						outG = accG >> 12;
						outR = accR >> 12;
					}
					else if (accA > 0)
					{
						outB = std::min(4096, int((int64_t(accB) << 12) / accA));
						outG = std::min(4096, int((int64_t(accG) << 12) / accA));
						outR = std::min(4096, int((int64_t(accR) << 12) / accA));
					}
				}
				else if constexpr(BLEND == BlendMode::Blend)
				{
					int backFraction = 65536;

					for (int i = 0; i <= edge; i++)
					{
						int alpha;
						
						if constexpr( SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if constexpr( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = (pSegmentTintmap + i * segmentTintmapPitch)[offset >> 8].a;
						if constexpr( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * (pSegmentTintmap + i * segmentTintmapPitch)[offset >> 8].a / 4096;

						
						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						backFraction -= blendFraction;
						_add_segment_color<SOURCE>(blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					if (!SoftFormatInfo::hasAlpha(DSTFORMAT) || backA == 4096)
					{
						outB = (accB >> 12) + ((backB * backFraction) >> 16);
						outG = (accG >> 12) + ((backG * backFraction) >> 16);
						outR = (accR >> 12) + ((backR * backFraction) >> 16);
						outA = (accA >> 12) + ((backA * backFraction) >> 16);
					}
					else
					{
						// Porter-Duff over onto a canvas that isn't opaque: colors are mixed premultiplied
						// and divided by the resulting alpha.

						int backWeight = (backA * backFraction) >> 16;
						outA = (accA >> 12) + backWeight;

						if (outA > 0)
						{
							outB = std::min(4096, ((accB >> 12) * 4096 + backB * backWeight) / outA);
							outG = std::min(4096, ((accG >> 12) * 4096 + backG * backWeight) / outA);
							outR = std::min(4096, ((accR >> 12) * 4096 + backR * backWeight) / outA);
						}
					}
				}
				else if constexpr(BLEND == BlendMode::BlendFixedColor)
				{
					int backFraction = 65536;

					for (int i = 0; i <= edge; i++)
					{
						int alpha;

						if constexpr(SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if constexpr( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = (pSegmentTintmap + i * segmentTintmapPitch)[offset >> 8].a;
						if constexpr( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * (pSegmentTintmap + i * segmentTintmapPitch)[offset >> 8].a / 4096;

						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						backFraction -= blendFraction;
						_add_segment_color<SOURCE>(blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					outB = (accB >> 12) + ((fixedB * backFraction) >> 16);
					outG = (accG >> 12) + ((fixedG * backFraction) >> 16);
					outR = (accR >> 12) + ((fixedR * backFraction) >> 16);
					outA = (accA >> 12) + ((fixedA * backFraction) >> 16);	// This should be correct...
				}
				else if constexpr(_is_effect_mode<BLEND>())
				{
					// Effect modes apply the segments together, as one source that is their premultiplied
					// average weighted by coverage, with the alpha of them all together. The segments cover
					// the whole pixel, so on alpha-only canvases no part of it is left for Multiply and Min
					// to fade towards 1.

					for (int i = 0; i <= edge; i++)
					{
						int alpha;

						if constexpr(SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if constexpr( SOURCE == SoftBackend::StripSource::Tintmaps )
							alpha = (pSegmentTintmap + i * segmentTintmapPitch)[offset >> 8].a;
						if constexpr( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * (pSegmentTintmap + i * segmentTintmapPitch)[offset >> 8].a / 4096;

						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						_add_segment_color<SOURCE>(blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					_blend_effect<BLEND, DSTFORMAT>(accB >> 12, accG >> 12, accR >> 12, accA >> 12, backB, backG, backR, backA,
													outB, outG, outR, outA);
				}
				else if constexpr(BLEND == BlendMode::Morph)
				{
					int backFraction = 65536;

					for (int i = 0; i <= edge; i++)
					{
						int alpha;
						
						if constexpr( SOURCE == SoftBackend::StripSource::Colors)
							alpha = pSegmentColors[i * 4 + 3];
						if constexpr(SOURCE == SoftBackend::StripSource::Tintmaps)
							alpha = (pSegmentTintmap + i * segmentTintmapPitch)[offset >> 8].a;
						if constexpr( SOURCE == SoftBackend::StripSource::ColorsAndTintmaps )
							alpha = pSegmentColors[i * 4 + 3] * (pSegmentTintmap + i * segmentTintmapPitch)[offset >> 8].a / 4096;

						int blendFraction = ((segmentFractions[i] * alpha) / 4096);
						backFraction -= blendFraction;
						_add_segment_color<SOURCE>(blendFraction, offset >> 8, &pSegmentColors[i * 4], pSegmentTintmap + i * segmentTintmapPitch, accB, accG, accR, accA);
					}

					// Cross-fade of premultiplied colors, so that a transparent side just fades out.

					int backWeight = (backA * (4096 - tint.morphFactor)) >> 12;
					int morph = tint.morphFactor;

					outA = backWeight + (((accA >> 12) * morph) >> 12);

					if (outA > 0)
					{
						outB = std::min(4096, ((accB >> 12) * morph + backB * backWeight) / outA);
						outG = std::min(4096, ((accG >> 12) * morph + backG * backWeight) / outA);
						outR = std::min(4096, ((accR >> 12) * morph + backR * backWeight) / outA);
					}
					else
						outB = outG = outR = 0;
				}

				_write_pixel<DSTFORMAT>(pDst, outB, outG, outR, outA);
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
			if constexpr(SOURCE == SoftBackend::StripSource::Tintmaps || SOURCE == SoftBackend::StripSource::ColorsAndTintmaps)
				pSegmentTintmap += segmentTintmapPitch;
			else
				pSegmentColors += 4;

		}
	}
}

//____ _limit4096() _______________________________________________________
// Clamps to 0 -> 4096, like SoftBackend::s_limit4096Tab but for any value.

static inline int _limit4096(int value)
{
	return value < 0 ? 0 : value > 4096 ? 4096 : value;
}

//____ _draw_coverage_span() _______________________________________________
//
// Blends a span of partly covered pixels, pixelPitch bytes apart, along a row or down
// a column, see SoftBackend::CoveragePixel. Colors are premultiplied: weighted by both
// coverage and alpha.
//
// With a paint mode a pixel gets the average of what the blend mode gives the covered part and
// the original pixel for the rest, weighted by their areas and averaged as premultiplied colors.
// Effect modes are applied with a strength scaled by the coverage. See "Blending and transparency"
// in the guide.
//
// Pixels can be shared by several segments, so the source is a sum over them. For effect modes
// that makes them one source, applied once, as they should be.
//
// Alpha-only destinations treat alpha as the color, like the fill kernels.

template<BlendMode BLEND, SoftFormat DSTFORMAT>
void _draw_coverage_span(uint8_t* WAX_RESTRICT pDst, int pixelPitch, int length, const SoftBackend::CoveragePixel* WAX_RESTRICT pPixels, const SoftBackend::ColTrans& tint)
{
	constexpr bool bAlphaOnly = (DSTFORMAT == SoftFormat::Alpha_8);
	constexpr bool bAlpha = SoftFormatInfo::hasAlpha(DSTFORMAT);

	const int fixedB = tint.fixedBlendColor.b;
	const int fixedG = tint.fixedBlendColor.g;
	const int fixedR = tint.fixedBlendColor.r;
	const int fixedA = tint.fixedBlendColor.a;

	const int morph = tint.morphFactor;

	// Paint modes: the source (premultiplied, times srcScale / 4096) plus what is left of the
	// canvas, backWeight of it, divided by the alpha of the result.

	auto paint = [](int backB, int backG, int backR, int backA, int srcB, int srcG, int srcR, int srcA, int srcScale, int keep,
					int& outB, int& outG, int& outR, int& outA)
	{
		int backWeight = (backA * keep + 2048) >> 12;
		outA = ((srcA * srcScale + 2048) >> 12) + backWeight;

		if constexpr (DSTFORMAT != SoftFormat::Alpha_8)		// Not bAlphaOnly, which MSVC can't use in a lambda.
		{
			if (outA == 4096)
			{
				outB = (srcB * srcScale + backB * backWeight + 2048) >> 12;
				outG = (srcG * srcScale + backG * backWeight + 2048) >> 12;
				outR = (srcR * srcScale + backR * backWeight + 2048) >> 12;
			}
			else
			{
				uint32_t reciprocal = SoftBackend::s_reciprocalTab[std::min(outA, 4096)];	// 0 if outA is, making all 0.
				uint32_t half = outA / 2;

				outB = _divide(srcB * srcScale + backB * backWeight, half, reciprocal);
				outG = _divide(srcG * srcScale + backG * backWeight, half, reciprocal);
				outR = _divide(srcR * srcScale + backR * backWeight, half, reciprocal);
			}
		}
	};

	for (int i = 0; i < length; i++)
	{
		const SoftBackend::CoveragePixel& px = pPixels[i];
		const int cov = px.coverage;

		if (cov > 0)
		{
			int16_t backB, backG, backR, backA;
			_read_pixel<DSTFORMAT>(pDst, nullptr, nullptr, backB, backG, backR, backA);

			int outB = backB, outG = backG, outR = backR, outA = backA;

			if constexpr (BLEND == BlendMode::Replace && !bAlpha)
			{
				// The canvas doesn't store alpha, so the covered part gets the color of the source as if
				// it was opaque and is mixed with the rest of the canvas by coverage.

				int inv = 4096 - cov;

				uint32_t reciprocal = SoftBackend::s_reciprocalTab[px.a];		// 0 if px.a is, adding nothing.
				uint32_t half = px.a / 2;

				outB = ((backB * inv + 2048) >> 12) + _divide(px.b * cov, half, reciprocal);
				outG = ((backG * inv + 2048) >> 12) + _divide(px.g * cov, half, reciprocal);
				outR = ((backR * inv + 2048) >> 12) + _divide(px.r * cov, half, reciprocal);
			}
			else if constexpr (BLEND == BlendMode::Replace)
			{
				// The covered part is replaced, the rest of the canvas is kept.

				paint(backB, backG, backR, backA, px.b, px.g, px.r, px.a, 4096, 4096 - cov, outB, outG, outR, outA);
			}
			else if constexpr (BLEND == BlendMode::Blend)
			{
				int inv = 4096 - px.a;

				if (!bAlpha || backA == 4096)
				{
					outB = ((backB * inv + 2048) >> 12) + px.b;
					outG = ((backG * inv + 2048) >> 12) + px.g;
					outR = ((backR * inv + 2048) >> 12) + px.r;
					outA = ((backA * inv + 2048) >> 12) + px.a;
				}
				else
					paint(backB, backG, backR, backA, px.b, px.g, px.r, px.a, 4096, inv, outB, outG, outR, outA);
			}
			else if constexpr (BLEND == BlendMode::BlendFixedColor)
			{
				int inv = 4096 - cov;
				int fixed = cov - px.a;					// Covered but transparent part shows the fixed color.

				outB = ((backB * inv + fixedB * fixed + 2048) >> 12) + px.b;
				outG = ((backG * inv + fixedG * fixed + 2048) >> 12) + px.g;
				outR = ((backR * inv + fixedR * fixed + 2048) >> 12) + px.r;
				outA = ((backA * inv + fixedA * fixed + 2048) >> 12) + px.a;
			}
			else if constexpr (_is_effect_mode<BLEND>())
			{
				// The source is applied with a strength scaled by coverage, as which it already is premultiplied.
				// On alpha-only canvases its alpha is the value, which for Multiply and Min is faded towards 1
				// where the pixel isn't covered.

				int value = px.a;

				if constexpr (bAlphaOnly && (BLEND == BlendMode::Multiply || BLEND == BlendMode::Min))
					value += 4096 - cov;

				int16_t effB, effG, effR, effA;
				_blend_effect<BLEND, DSTFORMAT>(px.b, px.g, px.r, value, backB, backG, backR, backA, effB, effG, effR, effA);

				outB = effB;
				outG = effG;
				outR = effR;
				outA = effA;
			}
			else if constexpr (BLEND == BlendMode::Morph && !bAlpha)
			{
				// The canvas doesn't store alpha, so the cross-fade of the covered part is made opaque
				// and mixed with the rest of the canvas by coverage.

				int backWeight = ((4096 - morph) * cov + 2048) >> 12;
				int weight = backWeight + ((px.a * morph + 2048) >> 12);
				int inv = 4096 - cov;

				uint32_t reciprocal = SoftBackend::s_reciprocalTab[std::min(weight, 4096)];	// 0 if weight is, leaving the canvas.
				uint32_t half = weight / 2;

				if (weight > 0)
				{
					outB = ((backB * inv + 2048) >> 12) + ((_divide(backB * backWeight + px.b * morph, half, reciprocal) * cov) >> 12);
					outG = ((backG * inv + 2048) >> 12) + ((_divide(backG * backWeight + px.g * morph, half, reciprocal) * cov) >> 12);
					outR = ((backR * inv + 2048) >> 12) + ((_divide(backR * backWeight + px.r * morph, half, reciprocal) * cov) >> 12);
				}
			}
			else if constexpr (BLEND == BlendMode::Morph)
			{
				// The covered part cross-fades to the source by the morph factor.

				paint(backB, backG, backR, backA, px.b, px.g, px.r, px.a, morph, 4096 - ((morph * cov + 2048) >> 12), outB, outG, outR, outA);
			}

			_write_pixel<DSTFORMAT>(pDst, _limit4096(outB), _limit4096(outG), _limit4096(outR), _limit4096(outA));
		}

		pDst += pixelPitch;
	}
}
