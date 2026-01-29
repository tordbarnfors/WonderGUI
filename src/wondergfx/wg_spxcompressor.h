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
#ifndef	WG_SPXCOMPRESSOR_DOT_H
#define	WG_SPXCOMPRESSOR_DOT_H
#pragma once

#include <wg_compressor.h>




namespace wg
{

	class SPXCompressor;
	typedef StrongPtr<SPXCompressor>	SPXCompressor_p;
	typedef WeakPtr<SPXCompressor>		SPXCompressor_wp;

	class SPXCompressor : public Compressor
	{
	public:

		//.____ Creation __________________________________________________________

		static SPXCompressor_p		create();

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


};



}




#endif //WG_Q565COMPRESSOR_DOT_H
