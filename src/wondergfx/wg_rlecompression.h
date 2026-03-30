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
#ifndef	WG_RLECOMPRESSION_DOT_H
#define	WG_RLECOMPRESSION_DOT_H
#pragma once

#include <wg_compression.h>


/*
*  Simple and fast RLE compression of primitives of selectable size.
*
*	RLEx compression format:
*
*	First byte in stream is primitive size. If primitive size is larger
*	than 1, then a padding byte follows.
*
*	Byte value for primitive size 1:
*
*	0-127			Copy 1-128 bytes following primitives verbatim.
*	-1 - -128		Repeat previous primitive 1-128 times.
*
*	Word value for primitive size 2+:
*
*	0-32767			Copy 1-32768 uint16_t following primitives verbatim.
*	-1 - -32768		Repeat previous primitive 1-32768 times.
*
*/



namespace wg
{

	class RLECompressor;
	typedef StrongPtr<RLECompressor>	RLECompressor_p;
	typedef WeakPtr<RLECompressor>		RLECompressor_wp;

	class RLEDecompressor;
	typedef StrongPtr<RLEDecompressor>	RLEDecompressor_p;
	typedef WeakPtr<RLEDecompressor>	RLEDecompressor_wp;

	//____ RLECompressor _________________________________________________________

	class RLECompressor : public Compressor
	{
	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			int				primSize = 1;				// Size in byte of primitives to RLE encode. Must be 1 or a mutiple of 2, less than 256.
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

		void	setPrimSize( int primSize ) { m_primSize = primSize; }
		int		primSize() const { return m_primSize; }

	protected:
		RLECompressor();
		RLECompressor( const Blueprint& blueprint );
		virtual ~RLECompressor() {};

		int		m_primSize = 1;
	};

	//____ RLEDecompressor _________________________________________________________

	class RLEDecompressor : public Decompressor
	{
	public:

		//.____ Creation __________________________________________________________

		static RLEDecompressor_p		create();

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		uint32_t idToken() const override;

		const static uint32_t	ID_TOKEN;

		//.____ Misc ____________________________________________________

		int		decompress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;

	protected:
		RLEDecompressor() {};
		virtual ~RLEDecompressor() {};
	};


}


#endif //WG_RLECOMPRESSION_DOT_H
