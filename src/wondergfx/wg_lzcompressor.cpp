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

#include <algorithm>

namespace wg
{
const TypeInfo LZCompressor::TYPEINFO = { "LZCompressor", &Compressor::TYPEINFO };

const uint32_t LZCompressor::ID_TOKEN = Util::makeEndianSpecificToken('L', 'Z', 'W', 'G');

//____ create() _____________________________________________________________

LZCompressor_p LZCompressor::create()
{
	return LZCompressor_p( new LZCompressor() );
}

LZCompressor_p LZCompressor::create( const Blueprint& blueprint )
{
	return LZCompressor_p( new LZCompressor(blueprint) );
}

//____ constructor ____________________________________________________________

LZCompressor::LZCompressor()
{
	_generateTable();
}

LZCompressor::LZCompressor( const Blueprint& blueprint )
{
	if( !blueprint.decompressOnly )
		_generateTable();

	if( blueprint.finalizer )
		setFinalizer(blueprint.finalizer);
}


//____ destructor ____________________________________________________________

LZCompressor::~LZCompressor()
{
	delete [] m_pHashTable;
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
	// Worst case scenario is one extra byte for every 4 bytes.

	int maxExtra = (uncompressedSize/4)+1;

	return uncompressedSize + maxExtra;
}


//____ compress() _____________________________________________________________

int LZCompressor::compress( void * _pDest, const void * _pBegin, const void * _pEnd )
{
	if( m_pHashTable == nullptr )
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "Attempting to compress with a compressor in decompress only mode.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return 0;
	}

	auto pDest = (uint8_t*) _pDest;
	auto pBegin = (uint8_t*) _pBegin;
	auto pEnd = (uint8_t*) _pEnd;


	// Generate and initialize our tables

	int windowSize = m_windowSize*sizeof(uint16_t);

	auto pWindow = (uint16_t *) GfxBase::memStackAlloc(windowSize);

	memset( pWindow, 0, windowSize );

	//

	auto pRead = pBegin;
	auto pWrite = pDest;

	int windowIdx = 0;
	uint8_t * 	pBestMatch = nullptr;

	int		nbNewInserts = 0;
	uint8_t * pNewInserts = nullptr;

	while( pRead <= (pEnd - 3) )
	{
		uint32_t hash = hash_bytes(pRead);		// Hash value for comming three bytes. Entry in hash table.

		int matchWindowOfs = m_pHashTable[hash];			// Entry in window for this hash.

		uint8_t * pMatch = pRead - ((windowIdx + m_windowSize - matchWindowOfs)%m_windowSize);	// Pointer at latest match given matchWindowOfs.

		assert( pMatch <= pRead );

		int	bestMatchLength = 0;
		int steps = 0;

		int maxLength = std::min( 66, int(pEnd - pRead) );

		while( steps < m_maxSteps && pMatch < pRead && pMatch >= pBegin )
		{
			// This is indeed a match, lets check its length.

			if( pMatch[bestMatchLength] == pRead[bestMatchLength] && pMatch[0] == pRead[0] && pMatch[1] == pRead[1] && pMatch[2] == pRead[2] )
			{
				int matchLength = 3;

				while( matchLength < maxLength )
				{
					if( pMatch[matchLength] == pRead[matchLength] )
						matchLength++;
					else
						break;
				}

				// Check if we found a better match than before

				if( matchLength > bestMatchLength )
				{
					pBestMatch = pMatch;
					bestMatchLength = matchLength;

					if( matchLength == maxLength )
						break;
				}
			}



			matchWindowOfs = pWindow[matchWindowOfs];
			pMatch = pRead - ((windowIdx + m_windowSize - matchWindowOfs)%m_windowSize);

			steps++;
		}

		if( bestMatchLength > 0 )
		{
			// End new inserts chunk if we have one

			if( nbNewInserts != 0 )
			{
				* pNewInserts = nbNewInserts-1;
				nbNewInserts = 0;
			}

			// Save encoded data

			int offset = int(pRead - pBestMatch) -1;

			assert(offset >= 0 && offset <= 65535);

			if( offset < 256 )
			{
				* pWrite++ = (bestMatchLength - 3) + 128;
				* pWrite++ = uint8_t(offset);
			}
			else
			{
				* pWrite++ = (bestMatchLength - 3) + 192;
				* pWrite++ = uint8_t(offset);
				* pWrite++ = uint8_t(offset>>8);
			}

			// Insert value into hash table & update sliding window, for all skipped values

			if( pRead + bestMatchLength > (pEnd - 3) )
				pRead += bestMatchLength;
			else
			{
				for( int i = 0 ; i < bestMatchLength ; i++ )
				{
					hash = hash_bytes(pRead++);
					pWindow[windowIdx] = m_pHashTable[hash];
					m_pHashTable[hash] = windowIdx++;
					windowIdx = windowIdx & (m_windowSize-1);		// Loop windowIdx;
				}
			}
		}
		else
		{
			// Insert value into hash table & update sliding window

			pWindow[windowIdx] = m_pHashTable[hash];
			m_pHashTable[hash] = windowIdx++;
			windowIdx = windowIdx & (m_windowSize-1);		// Loop windowIdx;

			// Begin new inserts chunk if we don't have one

			if( nbNewInserts == 0 )
			{
				pNewInserts = pWrite;
				* pWrite++ = 0;
			}

			//Insert unencoded data

			* pWrite++ = * pRead++;
			nbNewInserts++;

			// End new inserts chunk if we have reach its limit

			if( nbNewInserts == 128 )
			{
				* pNewInserts = 128-1;
				nbNewInserts = 0;
			}
		}

	}

	// Make sure to finish up last chunk of new inserts

	if( nbNewInserts > 0 )
		* pNewInserts = nbNewInserts-1;


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
				int copyAmount = byte1 - 128 + 3;
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

//____ _generateTable() _______________________________________________________

void LZCompressor::_generateTable()
{
	m_pHashTable = new uint16_t[m_hashSize];

	memset( m_pHashTable, 0, m_hashSize*sizeof(uint16_t) );
}


}

