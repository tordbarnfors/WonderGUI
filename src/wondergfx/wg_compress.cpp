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




//____ decompressSpx() ___________________________________________________________

void decompressSpx( Compression type, const void * pSource, int nbBytes, void * pDest )
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

/*
*	WG565 compression format:
* 
	00 0xxxxx		New pixels(1 - 32)
	00 1xxxxx		Repeat previous pixel(1 - 32)
	01 xxxxxx		Pixel from index
	1 rrgggbb		Delta rgb values -2 > +1 for r and b, -4 > +3 for g
*/

//____ generateTablesForQ565() ______________________________________________

void generateTablesForQ565(uint8_t pixelToIndexTable[65536])
{
	for (int i = 0; i < 65536; i++)
	{
		int r = i & 0x001F;
		int g = (i >> 5) & 0x003F;
		int b = (i >> 11) & 0x001F;
		pixelToIndexTable[i] = (r * 3 + g * 5 + b * 7) % 64;
	}
}

//____ compressPixelsQ565() _________________________________________________

int compressPixelsQ565(uint8_t* pDest, const uint16_t* pBegin, const uint16_t* pEnd, const uint8_t pixelToIndex[65536])
{
	uint16_t palette[64];

	for (int i = 0; i < 64; i++)
		palette[i] = 0;

	uint16_t prevValue = 0;

	const uint16_t* pRead = pBegin;
	uint8_t* pWrite = pDest;
	while (pRead < pEnd)
	{
		uint16_t value = *pRead++;

		if (value == prevValue)
		{
			uint16_t count = 1;
			while (pRead < pEnd && *pRead == value && count < 32)
			{
				count++;
				pRead++;
			}

			*pWrite++ = 0x20 | (count - 1);						// Store as repeat of previous value
		}
		else
		{
			uint8_t index = pixelToIndex[value];

			if (palette[index] == value)
				*pWrite++ = 0x40 | index;						// Store as index lookup
			else
			{
				uint16_t prevR = prevValue & 0x001F;
				uint16_t prevG = (prevValue >> 5) & 0x003F;
				uint16_t prevB = (prevValue >> 11) & 0x001F;

				uint16_t r = value & 0x001F;
				uint16_t g = (value >> 5) & 0x003F;
				uint16_t b = (value >> 11) & 0x001F;

				uint16_t diffR = r - prevR + 2;
				uint16_t diffG = g - prevG + 4;
				uint16_t diffB = b - prevB + 2;


				if (diffR < 4 && diffG < 8 && diffB < 4)
				{
					*pWrite++ = 0x80 | (diffR << 5) | (diffG << 2) | diffB;		// Store as RGB-delta
					palette[index] = value;
				}
				else
				{
					auto pCounter = pWrite;

					*pWrite++ = 1;
					*pWrite++ = (uint8_t)value;
					*pWrite++ = (uint8_t)(value >> 8);

					palette[index] = value;

					int count = 1;
					while (pRead < pEnd && count < 32)
					{
						uint16_t futureValue = *pRead;

						if (futureValue == value)
							break;				// Next pixel is start of repetive section

						uint8_t futureIndex = pixelToIndex[futureValue];
						if (palette[futureIndex] == futureValue)
							break;				// Next pixel can be taken from index;

						uint16_t futureR = futureValue & 0x001F;
						uint16_t futureG = (futureValue >> 5) & 0x003F;
						uint16_t futureB = (futureValue >> 11) & 0x001F;

						uint16_t diffR = futureR - r + 2;
						uint16_t diffG = futureG - g + 4;
						uint16_t diffB = futureB - b + 2;

						if (diffR < 4 && diffG < 8 && diffB < 4)
							break;				// Next pixel can be stored as RGB-delta.

						palette[futureIndex] = futureValue;

						value = futureValue;

						r = futureR;
						g = futureG;
						b = futureB;

						*pWrite++ = (uint8_t)value;
						*pWrite++ = (uint8_t)(value >> 8);

						pRead++;
						count++;
					}

					*pCounter = count - 1;
				}

			}
		}

		prevValue = value;
	}
	return int(pWrite - pDest);
}

//____ decompressPixelsQ565() _______________________________________________

int	decompressPixelsQ565(uint16_t* pDest, const uint8_t* pBegin, const uint8_t* pEnd, const uint8_t pixelToIndex[65536])
{
	uint16_t* pOriginalDest = pDest;

	uint16_t palette[64];

	for (int i = 0; i < 64; i++)
		palette[i] = 0;

	uint16_t	lastPixel = 0;

	auto pRead = pBegin;
	while (pRead < pEnd)
	{
		uint8_t v = *pRead++;

		if (v < 0x40)
		{
			if (v < 0x20)
			{
				int nbPixels = v + 1;
				for (int i = 0; i < nbPixels; i++)
				{
					lastPixel = *pRead++;
					lastPixel |= (*pRead++) << 8;
					*pDest++ = lastPixel;

					palette[pixelToIndex[lastPixel]] = lastPixel;
				}
			}
			else
			{
				int nbPixels = (v & 0x1F) + 1;
				for (int i = 0; i < nbPixels; i++)
					*pDest++ = lastPixel;
			}
		}
		else
		{
			if (v < 0x80)
			{
				int index = v & 0x3F;
				lastPixel = palette[index];
				*pDest++ = lastPixel;
			}
			else
			{
				uint8_t r = uint8_t(lastPixel & 0x001F);
				uint8_t g = uint8_t((lastPixel >> 5) & 0x003F);
				uint8_t b = uint8_t((lastPixel >> 11) & 0x001F);

				r += ((v >> 5) & 0x3) - 2;
				g += ((v >> 2) & 0x7) - 4;
				b += (v & 0x3) - 2;

				lastPixel = (b << 11) | (g << 5) | r;
				*pDest++ = lastPixel;

				palette[pixelToIndex[lastPixel]] = lastPixel;
			}
		}
	}

	return int(pDest - pOriginalDest);
}


} // namespace wg
