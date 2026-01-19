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
#ifndef	WG_COMPRESS_DOT_H
#define	WG_COMPRESS_DOT_H
#pragma once

#include <ctype.h>
#include <tuple>
#include <limits>

#include <wg_gfxtypes.h>

namespace wg
{	
	enum class PixelFilter
	{
		None,
	};

	enum class PixelCompression
	{
		None,
		Q565							// QOI-like compression. Only for 16-bit RGB565 pixel data.
	};

	enum class PaletteFilter
	{
		None
	};

	enum class PaletteCompression
	{
		None
	};

	enum class ExtrasCompression
	{
		None
	};




	std::tuple<Compression,int, const spx *> compressSpx( const spx * pBegin, const spx * pEnd, uint8_t * pDest, int maxChunkBytes = 0 );

	int 						compressSpxU8I( const spx * pSource, const spx * pEnd, uint8_t * pDest );
	int 						compressSpx16B( const spx * pSource, const spx * pEnd, uint8_t * pDest );
	int 						compressSpx16I( const spx * pSource, const spx * pEnd, uint8_t * pDest );

	void						decompressSpx( Compression type, const void * pSource, int nbBytes, void * pDest );

	void						generateTablesForQ565(uint8_t pixelToIndexTable[65536]);
	int 						compressPixelsQ565(uint8_t* pDest, const uint16_t* pBegin, const uint16_t* pEnd, const uint8_t pixelToIndexTable[65536] );
	int							decompressPixelsQ565(uint16_t* pDest, const uint8_t* pBegin, const uint8_t* pEnd, const uint8_t pixelToIndexTable[65536] );
}

#endif // WG_COMPRESS_DOT_H
