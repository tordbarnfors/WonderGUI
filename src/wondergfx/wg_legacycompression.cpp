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

#include <wg_legacycompression.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

#include <cstring>

namespace wg
{
const TypeInfo U8ICompressor::TYPEINFO = { "U8ICompressor", &SPXCompressor::TYPEINFO };
const TypeInfo S16ICompressor::TYPEINFO = { "S16ICompressor", &SPXCompressor::TYPEINFO };
const TypeInfo S16BCompressor::TYPEINFO = { "S16BCompressor", &SPXCompressor::TYPEINFO };

const uint32_t U8ICompressor::ID_TOKEN = Util::makeEndianSpecificToken('U', '8', 'I', ' ');
const uint32_t S16ICompressor::ID_TOKEN = Util::makeEndianSpecificToken('S', '1', '6', 'I');
const uint32_t S16BCompressor::ID_TOKEN = Util::makeEndianSpecificToken('S', '1', '6', 'B');


//____ create() _____________________________________________________________

U8ICompressor_p U8ICompressor::create()
{
	return U8ICompressor_p( new U8ICompressor() );
}

U8ICompressor_p U8ICompressor::create( const Blueprint& blueprint )
{
	auto p = U8ICompressor_p( new U8ICompressor() );
	if( blueprint.finalizer )
		p->setFinalizer(blueprint.finalizer);
	return p;
}

//____ constructor ____________________________________________________________

U8ICompressor::U8ICompressor()
{
}

//____ typeInfo() _________________________________________________________

const TypeInfo& U8ICompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t U8ICompressor::idToken() const
{
	return ID_TOKEN;
}

//____ maxCompressionSize() ___________________________________________________

int U8ICompressor::maxCompressedSize( int uncompressedSize )
{
	return uncompressedSize / 4;
}

//____ compress() _____________________________________________________________

int U8ICompressor::compress( void * _pDest, const void * _pBegin, const void * _pEnd )
{
	return _compress( Compression::SpxU8I, _pDest, (const spx*) _pBegin, (const spx*) _pEnd );
}

//____ decompress() ___________________________________________________________

int U8ICompressor::decompress( void * pDest, const void * pBegin, const void * pEnd )
{
	return _decompress( Compression::SpxU8I, (spx*) pDest, pBegin, pEnd );
}



//____ create() _____________________________________________________________

S16ICompressor_p S16ICompressor::create()
{
	return S16ICompressor_p( new S16ICompressor() );
}

S16ICompressor_p S16ICompressor::create( const Blueprint& blueprint )
{
	auto p = S16ICompressor_p( new S16ICompressor() );
	if( blueprint.finalizer )
		p->setFinalizer(blueprint.finalizer);
	return p;
}

//____ constructor ____________________________________________________________

S16ICompressor::S16ICompressor()
{
}

//____ typeInfo() _________________________________________________________

const TypeInfo& S16ICompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t S16ICompressor::idToken() const
{
	return ID_TOKEN;
}

//____ maxCompressionSize() ___________________________________________________

int S16ICompressor::maxCompressedSize( int uncompressedSize )
{
	return uncompressedSize / 2;
}

//____ compress() _____________________________________________________________

int S16ICompressor::compress( void * _pDest, const void * _pBegin, const void * _pEnd )
{
	return _compress( Compression::Spx16I, _pDest, (const spx*) _pBegin, (const spx*) _pEnd );
}

//____ decompress() ___________________________________________________________

int S16ICompressor::decompress( void * pDest, const void * pBegin, const void * pEnd )
{
	return _decompress( Compression::Spx16I, (spx *) pDest, pBegin, pEnd );
}



//____ create() _____________________________________________________________

S16BCompressor_p S16BCompressor::create()
{
	return S16BCompressor_p( new S16BCompressor() );
}

S16BCompressor_p S16BCompressor::create( const Blueprint& blueprint )
{
	auto p = S16BCompressor_p( new S16BCompressor() );
	if( blueprint.finalizer )
		p->setFinalizer(blueprint.finalizer);
	return p;
}

//____ constructor ____________________________________________________________

S16BCompressor::S16BCompressor()
{
}

//____ typeInfo() _________________________________________________________

const TypeInfo& S16BCompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ idToken() ___________________________________________________

uint32_t S16BCompressor::idToken() const
{
	return ID_TOKEN;
}

//____ maxCompressionSize() ___________________________________________________

int S16BCompressor::maxCompressedSize( int uncompressedSize )
{
	return uncompressedSize / 2;
}

//____ compress() _____________________________________________________________

int S16BCompressor::compress( void * _pDest, const void * _pBegin, const void * _pEnd )
{
	return _compress( Compression::Spx16B, _pDest, (const spx*) _pBegin, (const spx*) _pEnd );
}

//____ decompress() ___________________________________________________________

int S16BCompressor::decompress( void * pDest, const void * pBegin, const void * pEnd )
{
	return _decompress( Compression::Spx16B, (spx*) pDest, pBegin, pEnd );
}



}

