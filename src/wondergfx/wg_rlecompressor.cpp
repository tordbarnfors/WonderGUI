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

#include <wg_rlecompressor.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

namespace wg
{
const TypeInfo RLECompressor::TYPEINFO = { "RLECompressor", &Compressor::TYPEINFO };

const uint32_t RLECompressor::ID_TOKEN = Util::makeEndianSpecificToken('R', 'L', 'E', 'x');

//____ create() _____________________________________________________________

RLECompressor_p RLECompressor::create()
{
	return RLECompressor_p( new RLECompressor() );
}

RLECompressor_p RLECompressor::create( const Blueprint& blueprint )
{
	return RLECompressor_p( new RLECompressor(blueprint) );
}


//____ constructor ____________________________________________________________

RLECompressor::RLECompressor()
{
}

RLECompressor::RLECompressor( const Blueprint& blueprint )
{
	m_primSize = blueprint.primSize;

	if( blueprint.finalizer )
		setFinalizer(blueprint.finalizer);
}

//____ typeInfo() _________________________________________________________

const TypeInfo& RLECompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t RLECompressor::idToken() const
{
	return ID_TOKEN;
}

//____ maxCompressionSize() ___________________________________________________

int RLECompressor::maxCompressedSize( int uncompressedSize )
{
	int maxExtra;

	switch( m_primSize )
	{
		case 1:
			maxExtra = ((uncompressedSize + 127) / 128) + 1;		// Worst case scenario is one extra byte per 128 bytes + 1 extra byte for header.
			break;
		case 2:
			maxExtra = ((uncompressedSize + 65535) / 65536) + 2;	// Worst case scenario is one extra word per 32768 words + 1 extra word for header.
			break;
		default:
			assert( false );		// Not supported primSize.
	}

	return uncompressedSize + maxExtra;
}

//____ compress() _____________________________________________________________

int RLECompressor::compress( void * pDest, const void * pBegin, const void * pEnd )
{
	if( m_primSize == 1 )
	{
		int8_t * pRead = (int8_t*) pBegin;
		int8_t * pWrite = (int8_t*) pDest;

		* pWrite++ = m_primSize;

		int8_t * pSpanHead = pWrite++;
		int8_t last = * pRead++;
		* pWrite++ = last;

		int span = 1;

		while( pRead < (int8_t*)pEnd )
		{
			if( (pRead+1) < (int8_t*) pEnd && pRead[0] == last && pRead[1] == last )
			{
				* pSpanHead = span-1;

				int repeats = 2;
				while( pRead[repeats] == last && repeats < ((int8_t*) pEnd) - pRead )
					repeats++;

				while( repeats >= 2 )
				{
					int nToWrite = std::min(repeats,128);
					* pWrite++ = -nToWrite;
					repeats -= nToWrite;
					pRead += nToWrite;
				}

				if( pRead == (int8_t*) pEnd )
					return int( pWrite - (int8_t*)pDest);

				pSpanHead = pWrite++;
				span = 0;
			}
			else
			{
				if( span == 128 )
				{
					* pSpanHead = 127;
					pSpanHead = pWrite++;
					span = 0;
				}
			}

			last = * pRead++;
			* pWrite++ = last;
			span++;
		}

		* pSpanHead = span-1;

		return int( pWrite - (int8_t*)pDest);
	}
	else if( m_primSize == 2 )
	{
		int16_t * pRead = (int16_t*) pBegin;
		int16_t * pWrite = (int16_t*) pDest;

		* pWrite++ = m_primSize;

		int16_t * pSpanHead = pWrite++;
		int16_t last = * pRead++;
		* pWrite++ = last;

		int span = 1;

		while( pRead < (int16_t*)pEnd )
		{
			int byteOfs = int(pRead - (int16_t*) pBegin)*2;

			if( (pRead+1) < (int16_t*) pEnd && pRead[0] == last && pRead[1] == last )
			{
				* pSpanHead = span-1;

				int repeats = 2;
				while( pRead[repeats] == last && repeats < ((int16_t*) pEnd) - pRead )
					repeats++;

				while( repeats >= 2 )
				{
					int nToWrite = std::min(repeats,32768);
					* pWrite++ = -nToWrite;
					repeats -= nToWrite;
					pRead += nToWrite;
				}

				if( pRead == (int16_t*) pEnd )
					return int( ((int8_t*)pWrite) - (int8_t*)pDest);

				pSpanHead = pWrite++;
				span = 0;
			}
			else
			{
				if( span == 32768 )
				{
					* pSpanHead = 32767;
					pSpanHead = pWrite++;
					span = 0;
				}
			}

			last = * pRead++;
			* pWrite++ = last;
			span++;
		}

		* pSpanHead = span-1;

		return int( ((int8_t*)pWrite) - (int8_t*)pDest);

	}
}

//____ decompress() ___________________________________________________________

int RLECompressor::decompress( void * pDest, const void * pBegin, const void * pEnd )
{
	int primSize = * (int8_t*) pBegin;

	if( primSize == 1 )
	{
		int8_t * pRead = ((int8_t*) pBegin)+1;
		int8_t * pWrite = (int8_t*) pDest;

		while( pRead < pEnd )
		{
			int length = * pRead++;
			if( length >= 0 )
			{
				for( int i = 0 ; i <= length ; i++ )
					* pWrite++ = * pRead++;
			}
			else
			{
				length = -length;
				int8_t v = pWrite[-1];
				for( int i = 0 ; i < length ; i++ )
					* pWrite++ = v;
			}
		}

		return int( pWrite - ((int8_t*)pDest) );
	}
	else if( primSize == 2 )
	{
		int16_t * pRead = ((int16_t*) pBegin)+1;
		int16_t * pWrite = (int16_t*) pDest;

		while( pRead < pEnd )
		{
			int byteOfs = int(pWrite - (int16_t*) pDest)*2;

			int length = * pRead++;
			if( length >= 0 )
			{
				for( int i = 0 ; i <= length ; i++ )
					* pWrite++ = * pRead++;
			}
			else
			{
				length = -length;
				int16_t v = pWrite[-1];
				for( int i = 0 ; i < length ; i++ )
					* pWrite++ = v;
			}
		}

		return int( ((int8_t*)pWrite) - ((int8_t*)pDest) );
	}

}


}

