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
#ifndef	WG_COMPRESSOR_DOT_H
#define	WG_COMPRESSOR_DOT_H
#pragma once

#include <wg_object.h>


namespace wg
{

	class Compressor;
	typedef StrongPtr<Compressor>	Compressor_p;
 	typedef WeakPtr<Compressor>		Compressor_wp;

	class Compressor : public Object
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		virtual uint32_t idToken() const = 0;

		//.____ Misc ____________________________________________________

		virtual int		maxCompressedSize( int uncompressedSize ) = 0;

		virtual int		compress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) = 0;
		virtual int		decompress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) = 0;


	protected:
		Compressor() {};
		virtual ~Compressor() {};


};



}




#endif //WG_COMPRESSOR_DOT_H
