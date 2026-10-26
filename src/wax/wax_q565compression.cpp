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

#include <wg_q565compression.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

namespace wg
{
const TypeInfo Q565Compressor::TYPEINFO = { "Q565Compressor", &Compressor::TYPEINFO };
const TypeInfo Q565Decompressor::TYPEINFO = { "Q565Decompressor", &Decompressor::TYPEINFO };

const uint32_t Q565Compressor::ID_TOKEN = Util::makeEndianSpecificToken('Q', '5', '6', '5');
const uint32_t Q565Decompressor::ID_TOKEN = Util::makeEndianSpecificToken('Q', '5', '6', '5');

static const uint8_t c_endOfStream = 0xD2;		// RGB-delta with no change, which is never used for anything else.

// Value to add to the previous pixel for each delta opcode (index = opcode & 0x7F):
// dr * 2048 + dg * 32 + db, modulo 65536. Adding it changes each channel by its
// delta, as long as no channel wraps around, which the encoder never lets happen.
// Must be identical to deltaTable in the Q16 library.

static const uint16_t c_deltaTable[128] = {
	0xEF7E, 0xEF7F, 0xEF80, 0xEF81, 0xEF9E, 0xEF9F, 0xEFA0, 0xEFA1,
	0xEFBE, 0xEFBF, 0xEFC0, 0xEFC1, 0xEFDE, 0xEFDF, 0xEFE0, 0xEFE1,
	0xEFFE, 0xEFFF, 0xF000, 0xF001, 0xF01E, 0xF01F, 0xF020, 0xF021,
	0xF03E, 0xF03F, 0xF040, 0xF041, 0xF05E, 0xF05F, 0xF060, 0xF061,
	0xF77E, 0xF77F, 0xF780, 0xF781, 0xF79E, 0xF79F, 0xF7A0, 0xF7A1,
	0xF7BE, 0xF7BF, 0xF7C0, 0xF7C1, 0xF7DE, 0xF7DF, 0xF7E0, 0xF7E1,
	0xF7FE, 0xF7FF, 0xF800, 0xF801, 0xF81E, 0xF81F, 0xF820, 0xF821,
	0xF83E, 0xF83F, 0xF840, 0xF841, 0xF85E, 0xF85F, 0xF860, 0xF861,
	0xFF7E, 0xFF7F, 0xFF80, 0xFF81, 0xFF9E, 0xFF9F, 0xFFA0, 0xFFA1,
	0xFFBE, 0xFFBF, 0xFFC0, 0xFFC1, 0xFFDE, 0xFFDF, 0xFFE0, 0xFFE1,
	0xFFFE, 0xFFFF, 0x0000, 0x0001, 0x001E, 0x001F, 0x0020, 0x0021,
	0x003E, 0x003F, 0x0040, 0x0041, 0x005E, 0x005F, 0x0060, 0x0061,
	0x077E, 0x077F, 0x0780, 0x0781, 0x079E, 0x079F, 0x07A0, 0x07A1,
	0x07BE, 0x07BF, 0x07C0, 0x07C1, 0x07DE, 0x07DF, 0x07E0, 0x07E1,
	0x07FE, 0x07FF, 0x0800, 0x0801, 0x081E, 0x081F, 0x0820, 0x0821,
	0x083E, 0x083F, 0x0840, 0x0841, 0x085E, 0x085F, 0x0860, 0x0861
};

//____ _setupPixelToIndexTable() ______________________________________________

static void _setupPixelToIndexTable( uint8_t table[65536] )
{
	// Decides which of the 64 palette entries each pixel should go into.
	// Must be identical to q16_setupStaticTable() in the Q16 library.

	for (uint32_t i = 0; i < 65536; i++)
		table[i] = (uint8_t)((i + (i >> 3) + (i >> 4) + (i >> 10)) & 63);
}


//____ create() _____________________________________________________________

Q565Compressor_p Q565Compressor::create()
{
	return Q565Compressor_p( new Q565Compressor() );
}

//____ constructor ____________________________________________________________

Q565Compressor::Q565Compressor()
{
	_setupPixelToIndexTable(m_pixelToIndexTable);
}

//____ destructor _____________________________________________________________

Q565Compressor::~Q565Compressor()
{
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
	// Worst case scenario is one extra byte per 32 16-bit pixels plus end-of-stream.

	int maxExtra = (uncompressedSize + 63) / 64;

	return uncompressedSize + maxExtra + 1;
}

//____ compress() _____________________________________________________________

int Q565Compressor::compress( void * pDest, const void * pBegin, const void * pEnd )
{
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
			uint8_t index = m_pixelToIndexTable[value];

			if (palette[index] == value)
				*pWrite++ = 0x40 | index;						// Store as index lookup
			else
			{
				uint16_t prevR = (prevValue >> 11) & 0x001F;
				uint16_t prevG = (prevValue >> 5) & 0x003F;
				uint16_t prevB = prevValue & 0x001F;

				uint16_t r = (value >> 11) & 0x001F;
				uint16_t g = (value >> 5) & 0x003F;
				uint16_t b = value & 0x001F;

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

						uint8_t futureIndex = m_pixelToIndexTable[futureValue];
						if (palette[futureIndex] == futureValue)
							break;				// Next pixel can be taken from index;

						uint16_t futureR = (futureValue >> 11) & 0x001F;
						uint16_t futureG = (futureValue >> 5) & 0x003F;
						uint16_t futureB = futureValue & 0x001F;

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

	*pWrite++ = c_endOfStream;
	return int( pWrite - (uint8_t*)pDest);
}

//____ create() _____________________________________________________________

Q565Decompressor_p Q565Decompressor::create()
{
	return Q565Decompressor_p( new Q565Decompressor() );
}

//____ constructor ____________________________________________________________

Q565Decompressor::Q565Decompressor()
{
	_setupPixelToIndexTable(m_pixelToIndexTable);
}

//____ destructor _____________________________________________________________

Q565Decompressor::~Q565Decompressor()
{
}

//____ typeInfo() _________________________________________________________

const TypeInfo& Q565Decompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t Q565Decompressor::idToken() const
{
	return ID_TOKEN;
}


//____ decompress() ___________________________________________________________

int Q565Decompressor::decompress( void * _pDest, const void * pBegin, const void * pEnd )
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

					palette[m_pixelToIndexTable[lastPixel]] = lastPixel;
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
				if (v == c_endOfStream)
					break;

				lastPixel += c_deltaTable[v & 0x7F];
				*pDest++ = lastPixel;

				palette[m_pixelToIndexTable[lastPixel]] = lastPixel;
			}
		}
	}

	return int( ((uint8_t*)pDest) - ((uint8_t*)_pDest) );
}


}
