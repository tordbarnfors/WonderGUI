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

namespace wg
{

//____ compressSpx() __________________________________________________________

std::tuple<Compression,int> compressSpx( const spx * pSource, int nbSpx, uint8_t * pDest )
{
	// Check sample max/min
	// and find best compression.

	int spxMask = 0;

	const int add = (1 << 21);
	const spx * p = pSource;

	for (int i = 0; i < nbSpx; i++ )
	{
		int value = *p++;
		spxMask |= value + add;
	}

	if( (spxMask & 0xFFDFC03F) == 0 )					// Check if all spx fits in uint8_t without binals.
	{
		int size = compressSpxU8I(pSource, nbSpx, pDest);
		return std::make_tuple(Compression::SpxU8I, size);
	}
	else if( (spxMask & 0xFFDF0000) == 0 )				// Check if all spx fits in int16_t with binals.
	{
		int size = compressSpx16B(pSource, nbSpx, pDest);
		return std::make_tuple(Compression::Spx16B, size);
	}
	else if( (spxMask & 0xFFC0003F) == 0 )				// Check if all spx fits in int16_t with binals.
	{
		int size = compressSpx16I(pSource, nbSpx, pDest);
		return std::make_tuple(Compression::Spx16I, size);
	}
	else
		return std::make_tuple(Compression::None, 0);
}

//____ compressSpxU8I() _______________________________________________________

int compressSpxU8I( const spx * pSource, int nb, uint8_t * pDest )
{
	for( int i = 0 ; i < nb ; i++ )
		* pDest++ = (uint8_t) ((* pSource++) >> 6);

	return nb;
}

//____ compressSpx16B() _______________________________________________________

int compressSpx16B( const spx * pSource, int nb, uint8_t * _pDest )
{
	auto pDest = (int16_t *) _pDest;

	for( int i = 0 ; i < nb ; i++ )
		* pDest++ = (int16_t) (* pSource++);

	return nb*2;
}

//____ compressSpx16I() _______________________________________________________

int compressSpx16I( const spx * pSource, int nb, uint8_t * _pDest )
{
	auto pDest = (int16_t *) _pDest;

	for( int i = 0 ; i < nb ; i++ )
		* pDest++ = (int16_t) ((* pSource++) >> 6);

	return nb*2;
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
