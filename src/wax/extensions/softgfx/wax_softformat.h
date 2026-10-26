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
#ifndef	WG_SOFTFORMAT_DOT_H
#define	WG_SOFTFORMAT_DOT_H
#pragma once

#include <wg_gfxtypes.h>

namespace wg
{
	//____ SoftFormat _________________________________________________________
	/**
	 * @brief Pixel format, color space and byte order combined.
	 *
	 * Kernels of SoftBackend are made for, and looked up by, a SoftFormat, which
	 * tells everything about how pixels are read and written. Only the pixel formats
	 * up to and including BGR_565 are supported.
	 *
	 * Names without _BE are little endian. Alpha_8 and Index_8 have neither byte order
	 * nor (for Alpha_8) color space.
	 */

	enum class SoftFormat : uint8_t
	{
		Undefined,					///< Buffer of HiColor, used by kernels for the first pass of two-pass blits.

		XRGB_8_sRGB,
		XRGB_8_linear,
		XRGB_8_BE_sRGB,
		XRGB_8_BE_linear,

		ARGB_8_sRGB,
		ARGB_8_linear,
		ARGB_8_BE_sRGB,
		ARGB_8_BE_linear,

		Index_8_sRGB,
		Index_8_linear,

		Index_16_sRGB,
		Index_16_linear,
		Index_16_BE_sRGB,
		Index_16_BE_linear,

		Alpha_8,

		RGB_565_sRGB,
		RGB_565_linear,
		RGB_565_BE_sRGB,
		RGB_565_BE_linear,

		BGR_565_sRGB,
		BGR_565_linear,
		BGR_565_BE_sRGB,
		BGR_565_BE_linear,

		BGR_565_BE_linear_G5		///< Same as BGR_565_BE_linear but never sets lowest bit of green. Only used for kernels, never as key.
	};

	const static int SoftFormat_size = (int)SoftFormat::BGR_565_BE_linear_G5 + 1;

	//____ SoftFormatInfo _____________________________________________________

	namespace SoftFormatInfo
	{
		struct Entry
		{
			PixelFormat		format;
			ColorSpace		colorSpace;
			bool			bigEndian;
			const char *	name;
		};

		constexpr Entry	c_entries[SoftFormat_size] = {
			{ PixelFormat::Undefined,	ColorSpace::Linear,	false,	"Undefined" },
			{ PixelFormat::XRGB_8,		ColorSpace::sRGB,	false,	"XRGB_8_sRGB" },
			{ PixelFormat::XRGB_8,		ColorSpace::Linear,	false,	"XRGB_8_linear" },
			{ PixelFormat::XRGB_8,		ColorSpace::sRGB,	true,	"XRGB_8_BE_sRGB" },
			{ PixelFormat::XRGB_8,		ColorSpace::Linear,	true,	"XRGB_8_BE_linear" },
			{ PixelFormat::ARGB_8,		ColorSpace::sRGB,	false,	"ARGB_8_sRGB" },
			{ PixelFormat::ARGB_8,		ColorSpace::Linear,	false,	"ARGB_8_linear" },
			{ PixelFormat::ARGB_8,		ColorSpace::sRGB,	true,	"ARGB_8_BE_sRGB" },
			{ PixelFormat::ARGB_8,		ColorSpace::Linear,	true,	"ARGB_8_BE_linear" },
			{ PixelFormat::Index_8,		ColorSpace::sRGB,	false,	"Index_8_sRGB" },
			{ PixelFormat::Index_8,		ColorSpace::Linear,	false,	"Index_8_linear" },
			{ PixelFormat::Index_16,	ColorSpace::sRGB,	false,	"Index_16_sRGB" },
			{ PixelFormat::Index_16,	ColorSpace::Linear,	false,	"Index_16_linear" },
			{ PixelFormat::Index_16,	ColorSpace::sRGB,	true,	"Index_16_BE_sRGB" },
			{ PixelFormat::Index_16,	ColorSpace::Linear,	true,	"Index_16_BE_linear" },
			{ PixelFormat::Alpha_8,		ColorSpace::Linear,	false,	"Alpha_8" },
			{ PixelFormat::RGB_565,		ColorSpace::sRGB,	false,	"RGB_565_sRGB" },
			{ PixelFormat::RGB_565,		ColorSpace::Linear,	false,	"RGB_565_linear" },
			{ PixelFormat::RGB_565,		ColorSpace::sRGB,	true,	"RGB_565_BE_sRGB" },
			{ PixelFormat::RGB_565,		ColorSpace::Linear,	true,	"RGB_565_BE_linear" },
			{ PixelFormat::BGR_565,		ColorSpace::sRGB,	false,	"BGR_565_sRGB" },
			{ PixelFormat::BGR_565,		ColorSpace::Linear,	false,	"BGR_565_linear" },
			{ PixelFormat::BGR_565,		ColorSpace::sRGB,	true,	"BGR_565_BE_sRGB" },
			{ PixelFormat::BGR_565,		ColorSpace::Linear,	true,	"BGR_565_BE_linear" },
			{ PixelFormat::BGR_565,		ColorSpace::Linear,	true,	"BGR_565_BE_linear_G5" }
		};

		constexpr PixelFormat	pixelFormat(SoftFormat f) { return c_entries[int(f)].format; }
		constexpr ColorSpace	colorSpace(SoftFormat f) { return c_entries[int(f)].colorSpace; }
		constexpr bool			isBigEndian(SoftFormat f) { return c_entries[int(f)].bigEndian; }

		// Byte order differs from the one of the system, so 16 and 32-bit loads and stores need to be swapped.

		constexpr bool			needsSwap(SoftFormat f) { return c_entries[int(f)].bigEndian != (WG_IS_BIG_ENDIAN == 1); }

		// Channels are kept linear in 8 bits. Blending them directly is blending in linear space.
		// True for alpha only, since alpha always is linear. Not true for the HiColor buffer (Undefined).

		constexpr bool			isLinear(SoftFormat f) { return f != SoftFormat::Undefined && c_entries[int(f)].colorSpace == ColorSpace::Linear; }

		constexpr bool			isIndexed(SoftFormat f) { return pixelFormat(f) == PixelFormat::Index_8 || pixelFormat(f) == PixelFormat::Index_16; }

		constexpr bool			hasAlpha(SoftFormat f) { return pixelFormat(f) != PixelFormat::XRGB_8 && pixelFormat(f) != PixelFormat::RGB_565 && pixelFormat(f) != PixelFormat::BGR_565; }

		constexpr int			bytesPerPixel(SoftFormat f)
		{
			return f == SoftFormat::Undefined ? 8 : pixelFormat(f) == PixelFormat::XRGB_8 || pixelFormat(f) == PixelFormat::ARGB_8 ? 4 :
					pixelFormat(f) == PixelFormat::Index_8 || pixelFormat(f) == PixelFormat::Alpha_8 ? 1 : 2;
		}

		// Can be a canvas, i.e. has kernels drawing to it.

		constexpr bool			isDestination(SoftFormat f) { return f != SoftFormat::Undefined && !isIndexed(f) && f != SoftFormat::BGR_565_BE_linear_G5; }

		inline const char *		toString(SoftFormat f) { return int(f) < SoftFormat_size ? c_entries[int(f)].name : "Invalid"; }

		// SoftFormat for pixels of given format, color space and byte order. Undefined if not supported.

		inline SoftFormat		softFormat(PixelFormat format, ColorSpace colorSpace, bool bBigEndian)
		{
			if (colorSpace == ColorSpace::Undefined)
				colorSpace = ColorSpace::sRGB;

			switch (format)
			{
				case PixelFormat::Alpha_8:
					return SoftFormat::Alpha_8;

				case PixelFormat::Index_8:
					return colorSpace == ColorSpace::Linear ? SoftFormat::Index_8_linear : SoftFormat::Index_8_sRGB;

				default:
					for (int i = 1; i < SoftFormat_size - 1; i++)
					{
						auto& e = c_entries[i];
						if (e.format == format && e.colorSpace == colorSpace && e.bigEndian == bBigEndian)
							return SoftFormat(i);
					}
					return SoftFormat::Undefined;
			}
		}
	}

	inline const char * toString(SoftFormat f) { return SoftFormatInfo::toString(f); }
}

#endif //WG_SOFTFORMAT_DOT_H
