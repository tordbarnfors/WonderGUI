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
#ifndef	WG_SPXCOMPRESSION_DOT_H
#define	WG_SPXCOMPRESSION_DOT_H
#pragma once

#include <wg_compression.h>
#include <wg_gfxtypes.h>



namespace wg
{

	class SPXCompressor;
	typedef StrongPtr<SPXCompressor>	SPXCompressor_p;
	typedef WeakPtr<SPXCompressor>		SPXCompressor_wp;

	class SPXCompressor : public Compressor
	{
	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			bool			decompressOnly = false;
			Finalizer_p		finalizer = nullptr;
		};

		//.____ Creation __________________________________________________________

		static SPXCompressor_p		create();
		static SPXCompressor_p		create( const Blueprint& blueprint );


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
		SPXCompressor();
		virtual ~SPXCompressor() {};

		enum class Compression : uint32_t			//.autoExtras
		{
			None = 0,
			SpxU8I = 1,
			Spx16B = 2,
			Spx16I = 3
		};

		int		_compress( Compression type, void * pDest, const spx * pSrcBegin, const spx * pSrcEnd );
		int		_decompress( Compression type, spx * pDest, const void * pSrcBegin, const void * pSrcEnd );

};



}




#endif //WG_SPXCOMPRESSION_DOT_H
