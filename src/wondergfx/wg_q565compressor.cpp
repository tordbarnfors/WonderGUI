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

#include <wg_q565compressor.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

namespace wg
{
const TypeInfo Q565Compressor::TYPEINFO = { "Q565Compressor", &Compressor::TYPEINFO };

const uint32_t Q565Compressor::ID_TOKEN = Util::makeEndianSpecificToken('Q', '5', '6', '5');

//____ create() _____________________________________________________________

Q565Compressor_p Q565Compressor::create()
{
	return Q565Compressor_p( new Q565Compressor() );
}

Q565Compressor_p Q565Compressor::create( const Blueprint& blueprint )
{
	return Q565Compressor_p( new Q565Compressor(blueprint) );
}


//____ constructor ____________________________________________________________

Q565Compressor::Q565Compressor()
{
	_generateTable();
}

Q565Compressor::Q565Compressor( const Blueprint& blueprint )
{
	_generateTable();						// Used also in decompress operations.

	if( blueprint.finalizer )
		setFinalizer(blueprint.finalizer);
}

//____ destructor _____________________________________________________________

Q565Compressor::~Q565Compressor()
{
	delete [] m_pPixelToIndexTable;
}


//____ typeInfo() _________________________________________________________

const TypeInfo& Q565Compressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t Q565Compressor::idToken() const
{
	return ID_TOKEN;
}

//____ maxCompressionSize() ___________________________________________________

int Q565Compressor::maxCompressedSize( int uncompressedSize )
{
	// Worst case scenario is one extra byte per 32 16-bit pixels.

	int maxExtra = (uncompressedSize + 63) / 64;

	return uncompressedSize + maxExtra;
}

//____ compress() _____________________________________________________________

int Q565Compressor::compress( void * pDest, const void * pBegin, const void * pEnd )
{
	if( m_pPixelToIndexTable == nullptr )
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "Attempting to compress with a compressor in decompress only mode.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return 0;
	}

	if( ((intptr_t(pBegin) | intptr_t(pEnd) ) &0x1) == 1 )
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Data to compress must start/stop on WORD boundary.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return 0;
	}

	uint16_t palette[64];

	for (int i = 0; i < 64; i++)
		palette[i] = 0;

	uint16_t prevValue = 0;

	const uint16_t* pRead = (const uint16_t*) pBegin;
	uint8_t* pWrite = (uint8_t *) pDest;
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
			uint8_t index = m_pPixelToIndexTable[value];

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

						uint8_t futureIndex = m_pPixelToIndexTable[futureValue];
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
	return int( pWrite - (uint8_t*)pDest);
}

//____ decompress() ___________________________________________________________

int Q565Compressor::decompress( void * _pDest, const void * pBegin, const void * pEnd )
{
	uint16_t* pDest = (uint16_t*) _pDest;

	uint16_t palette[64];

	for (int i = 0; i < 64; i++)
		palette[i] = 0;

	uint16_t	lastPixel = 0;

	auto pRead = (uint8_t*) pBegin;
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

					palette[m_pPixelToIndexTable[lastPixel]] = lastPixel;
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

				palette[m_pPixelToIndexTable[lastPixel]] = lastPixel;
			}
		}
	}

	return int( ((uint8_t*)pDest) - ((uint8_t*)_pDest) );
}

//____ _generateTable() _______________________________________________________

void Q565Compressor::_generateTable()
{
	m_pPixelToIndexTable = new uint8_t[65536];

	for (int i = 0; i < 65536; i++)
	{
		int r = i & 0x001F;
		int g = (i >> 5) & 0x003F;
		int b = (i >> 11) & 0x001F;
		m_pPixelToIndexTable[i] = (r * 3 + g * 5 + b * 7) % 64;
	}
}



}

