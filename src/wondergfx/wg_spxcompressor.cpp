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

#include <wg_spxcompressor.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

namespace wg
{
const TypeInfo SPXCompressor::TYPEINFO = { "SPXCompressor", &Compressor::TYPEINFO };

const uint32_t SPXCompressor::ID_TOKEN = Util::makeEndianSpecificToken('S', 'P', 'X', '0');

//____ create() _____________________________________________________________

SPXCompressor_p SPXCompressor::create()
{
	return SPXCompressor_p( new SPXCompressor() );
}

//____ constructor ____________________________________________________________

SPXCompressor::SPXCompressor()
{
}

//____ typeInfo() _________________________________________________________

const TypeInfo& SPXCompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t SPXCompressor::idToken() const
{
	return ID_TOKEN;
}

//____ maxCompressionSize() ___________________________________________________

int SPXCompressor::maxCompressedSize( int uncompressedSize )
{
	// Worst case scenario is uncompressed with an spx-sized header.

	return uncompressedSize+sizeof(Compression);
}

//____ compress() _____________________________________________________________

int SPXCompressor::compress( void * _pDest, const void * _pBegin, const void * _pEnd )
{
	if( ((intptr_t(_pBegin) | intptr_t(_pEnd) ) &0x01) == 1 )
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Data to compress must start/stop on WORD boundary.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return 0;
	}

	const spx* pBeg = (const spx*) _pBegin;
	const spx* pEnd= (const spx*)_pEnd;

	auto pDest = (Compression *) _pDest;

	const int add = (1 << 21);

	int spxMask = 0;

	for (auto p = pBeg; p != pEnd; p++)
		spxMask |= *p + add;


	if ((spxMask & 0xFFDFC03F) == 0)					// Check if all spx fits in uint8_t without binals.
	{
		* pDest++ = Compression::SpxU8I;
		auto wp = (uint8_t*) pDest;

		while (pBeg < pEnd)
			*wp++ = (uint8_t)((*pBeg++) >> 6);

		return int(wp - (uint8_t*)_pDest);
	}
	else if ((spxMask & 0xFFDF0000) == 0)				// Check if all spx fits in int16_t with binals.
	{
		*pDest++ = Compression::Spx16B;
		auto wp = (int16_t*)pDest;

		while (pBeg < pEnd)
			*wp++ = (int16_t)(*pBeg++);

		return int(((uint8_t*)wp) - (uint8_t*)_pDest);
	}
	else if ((spxMask & 0xFFC0003F) == 0)				// Check if all spx fits in int16_t without binals.
	{
		*pDest++ = Compression::Spx16I;
		auto wp = (int16_t*)pDest;

		while (pBeg < pEnd)
			*wp++ = (int16_t)((*pBeg++) >> 6);

		return int(((uint8_t*)wp) - (uint8_t*)_pDest);
	}
	else
	{
		*pDest++ = Compression::None;

		memcpy(pDest, pBeg, ((uint8_t*)pEnd) - ((uint8_t*)pBeg));
		return int(((uint8_t*)pEnd) - (uint8_t*)_pDest);
	}
}

//____ decompress() ___________________________________________________________

int SPXCompressor::decompress( void * _pDest, const void * pBegin, const void * pEnd )
{
	Compression* pSource = (Compression*)pBegin;
	Compression type = * pSource++;

	int nbBytes = int(((uint8_t*)pEnd) - ((uint8_t*)pSource));

	spx* pDest = (spx*)_pDest;
	auto wp = pDest;

	switch (type)
	{
		case Compression::None:
			memcpy(pDest, pSource, nbBytes);
			return nbBytes;
	
		case Compression::SpxU8I:
		{
			auto rp = (uint8_t*)pSource;
			for (int i = 0; i < nbBytes; i++)
				*wp++ = ((spx)(*rp++)) << 6;
			return nbBytes * 4;
		}

		case Compression::Spx16I:
		{
			auto rp = (uint16_t*)pSource;
			for (int i = 0; i < nbBytes / 2; i++)
				*wp++ = ((spx)(*rp++)) << 6;
			return nbBytes * 2;
		}

		case Compression::Spx16B:
		{
			auto rp = (uint16_t*)pSource;
			for (int i = 0; i < nbBytes / 2; i++)
				*wp++ = (spx)(*rp++);
			return nbBytes * 2;
		}
	}
}




}

