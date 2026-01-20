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
#ifndef	WG_Q565COMPRESSOR_DOT_H
#define	WG_Q565COMPRESSOR_DOT_H
#pragma once

#include <wg_compressor.h>


/*
*  QOI-inspired fast and easy compression for 16-bit 565 pixels.
*
*	Q565 compression format:
*
	00 0xxxxx		New pixels(1 - 32)
	00 1xxxxx		Repeat previous pixel(1 - 32)
	01 xxxxxx		Pixel from index
	1 rrgggbb		Delta rgb values -2 > +1 for r and b, -4 > +3 for g
*/


namespace wg
{

	class Q565Compressor;
	typedef StrongPtr<Q565Compressor>	Q565Compressor_p;
	typedef WeakPtr<Q565Compressor>		Q565Compressor_wp;

	class Q565Compressor : public Compressor
	{
	public:

		//.____ Creation __________________________________________________________

		static Q565Compressor_p		create();

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
		Q565Compressor();
		virtual ~Q565Compressor() {};

		uint8_t	m_pixelToIndexTable[65536];
};



}




#endif //WG_Q565COMPRESSOR_DOT_H
