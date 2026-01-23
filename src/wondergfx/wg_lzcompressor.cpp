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

#include <wg_lzcompressor.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

namespace wg
{
const TypeInfo LZCompressor::TYPEINFO = { "LZCompressor", &Compressor::TYPEINFO };

const uint32_t LZCompressor::ID_TOKEN = Util::makeEndianSpecificToken('L', 'Z', 'W', 'G');

//____ create() _____________________________________________________________

LZCompressor_p LZCompressor::create()
{
	return LZCompressor_p( new LZCompressor() );
}

//____ constructor ____________________________________________________________

LZCompressor::LZCompressor()
{
}

//____ typeInfo() _________________________________________________________

const TypeInfo& LZCompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t LZCompressor::idToken() const
{
	return ID_TOKEN;
}

//____ maxCompressionSize() ___________________________________________________

int LZCompressor::maxCompressedSize( int uncompressedSize )
{
	// Worst case scenario is one extra byte per 3 bytes.

//	int maxExtra = (uncompressedSize/3)+1;

	int maxExtra = uncompressedSize;			//TEMP!!!

	return uncompressedSize + maxExtra;
}



//____ compress() _____________________________________________________________

int LZCompressor::compress( void * _pDest, const void * _pBegin, const void * _pEnd )
{
	auto pDest = (uint8_t*) _pDest;
	auto pBegin = (uint8_t*) _pBegin;
	auto pEnd = (uint8_t*) _pEnd;


	// Generate and initialize our tables

	int hashSize = m_hashSize*sizeof(uint16_t);
	int windowSize = m_windowSize*sizeof(uint16_t);

	auto pHash = (uint16_t *) GfxBase::memStackAlloc(hashSize);
	auto pWindow = (uint16_t *) GfxBase::memStackAlloc(windowSize);


	memset( pHash, -1, hashSize );
	memset( pWindow, -1, windowSize );

	//

	auto pRead = pBegin;
	auto pWrite = pDest;

	int windowIdx = 0;
	uint8_t * 	pBestMatch = nullptr;


	while( pRead <= (pEnd - 3) )
	{
		int			bestMatchLength = 0;

		uint32_t hash = hash_bytes(pRead);		// Hash value for comming three bytes. Entry in hash table.

		int matchWindowOfs = pHash[hash];			// Entry in window for this hash.

		int matchLength = 0;

		uint8_t * pMatch = pRead - ((windowIdx + m_hashSize - matchWindowOfs)%m_hashSize);	// Pointer at latest match according to hash table.

		if( pRead == pMatch )
			pMatch -= 65536;

		if( pMatch >= pBegin )
		{
			if( pMatch[0] == pRead[0] && pMatch[1] == pRead[1] && pMatch[2] == pRead[2] )
			{
				// This is indeed a match, lets check its length.

				matchLength = 3;
				while( matchLength < 66 && pRead + matchLength < pEnd )
				{
					if( pMatch[matchLength] == pRead[matchLength] )
						matchLength++;
					else
						break;
				}

				// Right now we just take the first match

				pBestMatch = pMatch;
				bestMatchLength = matchLength;
			}
		}

		if( bestMatchLength > 0 )
		{
			int offset = int(pRead - pBestMatch) -1;

			assert(offset >= 0 && offset <= 65535);

			* pWrite++ = (bestMatchLength - 3) + 192;
			* pWrite++ = uint8_t(offset);
			* pWrite++ = uint8_t(offset>>8);

			// Insert value into hash table & update sliding window, for all skipped values

			for( int i = 0 ; i < bestMatchLength ; i++ )
			{
				pWindow[windowIdx] = pHash[hash];
				pHash[hash] = windowIdx++;
				hash = hash_bytes(++pRead);
				windowIdx = windowIdx & (m_windowSize-1);		// Loop windowIdx;
			}
		}
		else
		{
			// Insert value into hash table & update sliding window

			pWindow[windowIdx] = pHash[hash];
			pHash[hash] = windowIdx++;
//			hash = hash_bytes(++pRead);
			windowIdx = windowIdx & (m_windowSize-1);		// Loop windowIdx;

			// Right now we are storing misses one by one

			* pWrite++ = 0;
			* pWrite++ = * pRead++;
		}

	}

	// Add any remaining bytes (1 or 2)

	if( pRead < pEnd )
	{
		* pWrite++ = (pEnd - pRead - 1);	// Number of bytes
		* pWrite++ = * pRead++;			// First byte
		if( pRead < pEnd )
			* pWrite++ = * pRead++;		// Possibly a second byte
	}

	// Cleanup and return

	GfxBase::memStackFree(windowSize);
	GfxBase::memStackFree(hashSize);

	return int(pWrite - pDest);
}

//____ decompress() ___________________________________________________________

int LZCompressor::decompress( void * pDest, const void * pBegin, const void * pEnd )
{
	uint8_t* pWrite = (uint8_t*) pDest;
	uint8_t * pRead = (uint8_t*) pBegin;

	while( pRead < pEnd )
	{
		uint8_t byte1 = * pRead++;

		if( byte1 < 128)
		{
			* pWrite++ = (* pRead++);
			for( int i = 0 ; i < byte1 ; i++ )
				* pWrite++ = * pRead++;
		}
		else
		{
			if( byte1 < 192 )
			{
				int copyAmount = byte1 - 128 + 2;
				uint8_t* pCopyFrom = pWrite - 1 - (* pRead++);
				for( int i = 0 ; i  < copyAmount ; i++ )
					* pWrite++ = * pCopyFrom++;
			}
			else
			{
				int copyAmount = byte1 - 192 + 3;

				int ofs = * pRead++;
				ofs += (int(* pRead++) << 8);

				uint8_t* pCopyFrom = pWrite - 1 - ofs;
				for( int i = 0 ; i  < copyAmount ; i++ )
					* pWrite++ = * pCopyFrom++;
			}
		}
	}

	return int(pWrite - (uint8_t*) pDest);
}


}

