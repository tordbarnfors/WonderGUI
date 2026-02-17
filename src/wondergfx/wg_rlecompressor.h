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
#ifndef	WG_RLECOMPRESSOR_DOT_H
#define	WG_RLECOMPRESSOR_DOT_H
#pragma once

#include <wg_compressor.h>


/*
*  Simple and fast RLE compression of primitives of selectable size.
*
*	RLEx compression format:
*

	Byte value:

	0-127			Copy 1-128 following primitives verbatim.
	-1 - -128		Repeat previous primitive 1-128 times.

*/



namespace wg
{

	class RLECompressor;
	typedef StrongPtr<RLECompressor>	RLECompressor_p;
	typedef WeakPtr<RLECompressor>		RLECompressor_wp;

	class RLECompressor : public Compressor
	{
	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			int				primSize = 1;				// Size in byte of primitive to RLE encode.
			bool			decompressOnly = false;
			Finalizer_p		finalizer = nullptr;
		};

		//.____ Creation __________________________________________________________

		static RLECompressor_p		create();
		static RLECompressor_p		create( const Blueprint& blueprint );

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
		RLECompressor();
		RLECompressor( const Blueprint& blueprint );
		virtual ~RLECompressor() {};

		int		m_primSize = 1;
};

}


#endif //WG_RLECOMPRESSOR_DOT_H
