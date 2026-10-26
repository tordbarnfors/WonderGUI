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
#ifndef	WG_PIXELTOOLS_DOT_H
#define	WG_PIXELTOOLS_DOT_H
#pragma once

#include <wg_gfxtypes.h>
#include <wg_color.h>
#include <wg_geo.h>

namespace wg
{
	/**
	 * Tools for converting, filling and inspecting raw pixel data.
	 *
	 * Pixels are described by a PixelFormat (or PixelDescription), a ColorSpace and a byte order.
	 * Conversion between any of them is supported. ColorSpace::Undefined is treated as sRGB.
	 *
	 * Colors (HiColor) given to these functions are sRGB, as everywhere else in WonderGUI.
	 *
	 * Palettes are in the color space of the pixels they belong to.
	 *
	 * Lines of bitplanes need to start at the first pixel of a 16-pixel word, except for
	 * fillBitmap() and extractAlphaChannel(), which take a rectangle within the bitmap.
	 */

	namespace PixelTools
	{
		bool copyPixels(int width, int height,
						const uint8_t* pSrc, PixelFormat srcFormat, ColorSpace srcColorSpace, bool srcBigEndian, int srcPitchAdd,
						const Color8* pSrcPalette, int srcPaletteEntries,
						uint8_t* pDst, PixelFormat dstFormat, ColorSpace dstColorSpace, bool dstBigEndian, int dstPitchAdd,
						Color8* pDstPalette, int& dstPaletteEntries, int maxDstPaletteEntries);

		bool copyPixels(int width, int height,
						const uint8_t* pSrc, const PixelDescription& srcDescription, ColorSpace srcColorSpace, int srcPitchAdd,
						const Color8* pSrcPalette, int srcPaletteEntries,
						uint8_t* pDst, PixelFormat dstFormat, ColorSpace dstColorSpace, bool dstBigEndian, int dstPitchAdd,
						Color8* pDstPalette, int& dstPaletteEntries, int maxDstPaletteEntries);

		void	fillBitmap(uint8_t* pBitmap, PixelFormat format, ColorSpace colorSpace, bool bigEndian, int pitch, const RectI& fillRect,
						   HiColor color, const Color8* pPalette = nullptr, int paletteSize = 0);

		int		colorToPixelBytes(HiColor color, PixelFormat format, ColorSpace colorSpace, bool bigEndian, uint8_t pixelArea[18],
								  const Color8* pPalette = nullptr, int paletteSize = 0);		// Bitplanes get one word per plane.

		int		findBestMatchInPalette(HiColor color, ColorSpace paletteColorSpace, const Color8* pPalette, int paletteSize);

		bool	extractAlphaChannel(PixelFormat format, bool bigEndian, const uint8_t* pSrc, int srcPitch, const RectI& srcRect,
									uint8_t* pDst, int dstPitch, const Color8* pPalette);

		int		bytesPerLine(const PixelDescription& description, int width);

		void	_initTables();
		void	_releaseTables();
	}
}

#endif //WG_PIXELTOOLS_DOT_H
