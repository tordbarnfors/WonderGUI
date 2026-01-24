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
#ifndef	WG_LZCOMPRESSOR_DOT_H
#define	WG_LZCOMPRESSOR_DOT_H
#pragma once

#include <wg_compressor.h>


/*
*  Lempel-Ziw based compression for any data with repetitive patterns.
*
*	Encoding:
*
*	0xxxxxxx						New pixels(1 - 128)
*	10xxxxxx yyyyyyyy				Replay X (3-66) bytes, starting from Y (1-255) bytes back.
*	11xxxxxx yyyyyyyy yyyyyyyy		Replay X (3-66) bytes, starting from Y (1-65535) bytes back. Low byte first (little-endian).
*/


namespace wg
{

	class LZCompressor;
	typedef StrongPtr<LZCompressor>	LZCompressor_p;
	typedef WeakPtr<LZCompressor>		LZCompressor_wp;

	class LZCompressor : public Compressor
	{
	public:

		//.____ Creation __________________________________________________________

		static LZCompressor_p		create();

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
		LZCompressor();
		virtual ~LZCompressor() {};

		inline uint32_t hash_bytes(uint8_t* data)
		{
			uint32_t h = data[0];
			h = ((h << 5) ^ data[1]) & (m_hashSize - 1);
			h = ((h << 5) ^ data[2]) & (m_hashSize - 1);
			return h;
		}


		int		m_hashSize = 65536;			// Minimum 16384, maximum 65536. Must be modulo 2.
		int		m_windowSize = 32768;		// Minimum 4096, maximum 32768. Must be modulo 2.
		int		m_maxSteps = 6;				// Minimum 1. Affects compression and speed a lot.
};



}




#endif //WG_LZCOMPRESSOR_DOT_H
