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

#include <wg_dummycompression.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

#include <cstring>

namespace wg
{
const TypeInfo DummyCompressor::TYPEINFO = { "DummyCompressor", &Compressor::TYPEINFO };

const uint32_t DummyCompressor::ID_TOKEN = Util::makeEndianSpecificToken('D', 'U', 'M', 'Y');

//____ create() _____________________________________________________________

DummyCompressor_p DummyCompressor::create()
{
	return DummyCompressor_p( new DummyCompressor() );
}

DummyCompressor_p DummyCompressor::create( const Blueprint& blueprint )
{
	auto p = DummyCompressor_p( new DummyCompressor() );
	if( blueprint.finalizer )
		p->setFinalizer(blueprint.finalizer);
	return p;
}

//____ constructor ____________________________________________________________

DummyCompressor::DummyCompressor()
{
}

//____ typeInfo() _________________________________________________________

const TypeInfo& DummyCompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t DummyCompressor::idToken() const
{
	return ID_TOKEN;
}

//____ maxCompressionSize() ___________________________________________________

int DummyCompressor::maxCompressedSize( int uncompressedSize )
{
	return uncompressedSize;
}

//____ compress() _____________________________________________________________

int DummyCompressor::compress( void * pDest, const void * pBegin, const void * pEnd )
{
	int size = int(((uint8_t*)pBegin) - (uint8_t*)pEnd);

	if( pDest != pBegin )
		memcpy( pDest, pBegin, size );

	return size;
}

//____ decompress() ___________________________________________________________

int DummyCompressor::decompress( void * pDest, const void * pBegin, const void * pEnd )
{
	int size = int(((uint8_t*)pBegin) - (uint8_t*)pEnd);

	if( pDest != pBegin )
		memcpy( pDest, pBegin, size );

	return size;
}


}

