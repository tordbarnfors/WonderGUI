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
	namespace PixelTools
	{
		/* CURRENT LIMITATIONS
		
			Not more than 8 bits per channel.
			Bitplanes not supported.		
		*/


		bool copyPixels(int width, int height, const uint8_t* pSrc, PixelFormat srcFmt, int srcPitchAdd,
			uint8_t* pDst, PixelFormat dstFmt, int dstPitchAdd, const Color8* pSrcPalette,
			Color8* pDstPalette, int srcPaletteEntries, int& dstPaletteEntries, int maxDstPaletteEntries);

		bool copyPixels(int width, int height, const uint8_t* pSrc, const PixelDescription& srcFmt, int srcPitchAdd,
			uint8_t* pDst, PixelFormat dstFmt, int dstPitchAdd, const Color8* pSrcPalette,
			Color8* pDstPalette, int srcPaletteEntries, int& dstPaletteEntries, int maxDstPaletteEntries);

		void fillBitmap(uint8_t* pBitmap, PixelFormat type, int pitch, RectI fillRect, HiColor color, Color8* pPalette = nullptr, int paletteSize = 0);
	
		int colorToPixelBytes( HiColor color, PixelFormat type, uint8_t pixelArea[18], Color8* pPalette = nullptr, int paletteSize = 0 );
	
		int findBestMatchInPalette( HiColor color, Color8* pPalette, int paletteSize = 256, ColorSpace paletteColorSpace = ColorSpace::Undefined );
	
		bool extractAlphaChannel(PixelFormat format, const uint8_t* pSrc, int srcPitch, RectI srcRect, uint8_t* pDst, int dstPitch, const Color8* pPalette);

	}
}

#endif //WG_PIXELTOOLS_DOT_H
