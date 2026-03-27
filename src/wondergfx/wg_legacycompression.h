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
#ifndef	WG_LEGACYCOMPRESSORS_DOT_H
#define	WG_STREAM_DOT_H
#pragma once

#include <wg_spxcompression.h>

namespace wg {

class U8ICompressor;
typedef StrongPtr<U8ICompressor> U8ICompressor_p;
typedef WeakPtr<U8ICompressor> U8ICompressor_wp;


//____ U8ICompressor __________________________________________________________

class U8ICompressor : public SPXCompressor
{
public:

	//.____ Creation __________________________________________________________

	static U8ICompressor_p		create();
	static U8ICompressor_p		create( const Blueprint& blueprint );

	//.____ Identification __________________________________________

	const TypeInfo& typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;

	uint32_t idToken() const override;

	const static uint32_t	ID_TOKEN;

	//.____ Misc ____________________________________________________

	int		maxCompressedSize( int uncompressedSize ) override;

	int		compress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;
	int		decompress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;

protected:
	U8ICompressor();
	virtual ~U8ICompressor() {};
};


class S16BCompressor;
typedef StrongPtr<S16BCompressor> S16BCompressor_p;
typedef WeakPtr<S16BCompressor> S16BCompressor_wp;


//____ S16BCompressor __________________________________________________________

class S16BCompressor : public SPXCompressor
{
public:

	//.____ Creation __________________________________________________________

	static S16BCompressor_p		create();
	static S16BCompressor_p		create( const Blueprint& blueprint );

	//.____ Identification __________________________________________

	const TypeInfo& typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;

	uint32_t idToken() const override;

	const static uint32_t	ID_TOKEN;

	//.____ Misc ____________________________________________________

	int		maxCompressedSize( int uncompressedSize ) override;

	int		compress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;
	int		decompress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;

protected:
	S16BCompressor();
	virtual ~S16BCompressor() {};

};



class S16ICompressor;
typedef StrongPtr<S16ICompressor> S16ICompressor_p;
typedef WeakPtr<S16ICompressor> S16ICompressor_wp;


//____ S16ICompressor __________________________________________________________

class S16ICompressor : public SPXCompressor
{
public:

	//.____ Creation __________________________________________________________

	static S16ICompressor_p		create();
	static S16ICompressor_p		create( const Blueprint& blueprint );

	//.____ Identification __________________________________________

	const TypeInfo& typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;

	uint32_t idToken() const override;

	const static uint32_t	ID_TOKEN;

	//.____ Misc ____________________________________________________

	int		maxCompressedSize( int uncompressedSize ) override;

	int		compress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;
	int		decompress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;

protected:
	S16ICompressor();
	virtual ~S16ICompressor() {};

};




}


#endif //WG_LEGACYCOMPRESSORS_DOT_H
