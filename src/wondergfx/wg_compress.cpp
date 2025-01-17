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

#include <wg_compress.h>

#include <cstring>
#include <algorithm>

namespace wg
{

//____ compressSpx() __________________________________________________________

std::tuple<Compression,int, const spx * > compressSpx( const spx * pBeg, const spx * pEnd, uint8_t * pDest, int maxChunkBytes )
{
	// Check sample max/min
	// and find best compression.

	int spxMask = 0;

	const int add = (1 << 21);

	for( auto p = pBeg ; p != pEnd ; p++ )
		spxMask |= *p + add;

	if( (spxMask & 0xFFDFC03F) == 0 )					// Check if all spx fits in uint8_t without binals.
	{
		auto pEnd2 = maxChunkBytes == 0 ? pEnd : std::min( pEnd, pBeg + (maxChunkBytes ));
		int size = compressSpxU8I(pBeg, pEnd2, pDest);
		return std::make_tuple(Compression::SpxU8I, size, pEnd2);
	}
	else if( (spxMask & 0xFFDF0000) == 0 )				// Check if all spx fits in int16_t with binals.
	{
		auto pEnd2 = maxChunkBytes == 0 ? pEnd : std::min( pEnd, pBeg + (maxChunkBytes/2 ));
		int size = compressSpx16B(pBeg, pEnd2, pDest);
		return std::make_tuple(Compression::Spx16B, size, pEnd2);
	}
	else if( (spxMask & 0xFFC0003F) == 0 )				// Check if all spx fits in int16_t with binals.
	{
		auto pEnd2 = maxChunkBytes == 0 ? pEnd : std::min( pEnd, pBeg + (maxChunkBytes/2 ));
		int size = compressSpx16I(pBeg, pEnd2, pDest);
		return std::make_tuple(Compression::Spx16I, size, pEnd2);
	}
	else
	{
		return std::make_tuple(Compression::None, 0, pEnd);
	}
}

//____ compressSpxU8I() _______________________________________________________

int compressSpxU8I( const spx * pBeg, const spx * pEnd, uint8_t * pDest )
{
	auto wp = pDest;

	while( pBeg < pEnd )
		* wp++ = (uint8_t) ((* pBeg++) >> 6);

	return int(wp - pDest);
}

//____ compressSpx16B() _______________________________________________________

int compressSpx16B( const spx * pBeg, const spx * pEnd, uint8_t * pDest )
{
	auto wp = (int16_t *) pDest;

	while( pBeg < pEnd )
		* wp++ = (int16_t) (* pBeg++);

	return int(((uint8_t*)wp) - pDest);
}

//____ compressSpx16I() _______________________________________________________

int compressSpx16I( const spx * pBeg, const spx * pEnd, uint8_t * pDest )
{
	auto wp = (int16_t *) pDest;

	while( pBeg < pEnd )
		* wp++ = (int16_t) ((* pBeg++) >> 6);

	return int(((uint8_t*)wp) - pDest);
}

//____ decompress() ___________________________________________________________

void decompress( Compression type, const void * pSource, int nbBytes, void * pDest )
{
	switch( type )
	{
		case Compression::None:
			memcpy(pDest,pSource,nbBytes);
			break;

		case Compression::SpxU8I:
		{
			auto rp = (uint8_t *) pSource;
			auto wp = (spx *) pDest;
			for( int i = 0 ; i < nbBytes ; i++ )
				* wp++ = ((spx)(* rp++)) << 6;
			break;
		}

		case Compression::Spx16I:
		{
			auto rp = (uint16_t *) pSource;
			auto wp = (spx *) pDest;
			for( int i = 0 ; i < nbBytes/2 ; i++ )
				* wp++ = ((spx)(* rp++)) << 6;
			break;
		}

		case Compression::Spx16B:
		{
			auto rp = (uint16_t *) pSource;
			auto wp = (spx *) pDest;
			for( int i = 0 ; i < nbBytes/2 ; i++ )
				* wp++ = (spx)(* rp++);
			break;
		}
	}
}



} // namespace wg
