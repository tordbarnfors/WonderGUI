/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef	WG_COMPRESS_DOT_H
#define	WG_COMPRESS_DOT_H
#pragma once

#include <ctype.h>
#include <tuple>

#include <wg_gfxtypes.h>

namespace wg
{	
	std::tuple<Compression,int> compressSpx( const spx * pSource, int nbSpx, uint8_t * pDest );

	int 						compressSpxU8I( const spx * pSource, int nbSpx, uint8_t * pDest );
	int 						compressSpx16B( const spx * pSource, int nbSpx, uint8_t * pDest );
	int 						compressSpx16I( const spx * pSource, int nbSpx, uint8_t * pDest );

	void						decompress( Compression type, const void * pSource, int nbBytes, void * pDest );
}

#endif // WG_COMPRESS_DOT_H
