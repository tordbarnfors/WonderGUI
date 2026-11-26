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
#ifndef WAX_C_BITMAPREADER_DOT_H
#define WAX_C_BITMAPREADER_DOT_H
#pragma once

#include <wax_c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

//____ wax_bitmapReaderBP __________________________________________________________

typedef struct wax_bitmapReaderBP_struct			// NOT BINARY EQUIVALENT!
{
	wax_obj				factory;
} wax_bitmapReaderBP;

WAX_EXPORT wax_obj 	wax_createBitmapReader(const wax_bitmapReaderBP* blueprint);
WAX_EXPORT wax_obj	wax_readBitmapFromBlob( wax_obj bitmapReader, wax_obj blob );
WAX_EXPORT wax_obj	wax_readBitmapFromMemory( wax_obj bitmapReader, const char * pData );

#ifdef __cplusplus
}
#endif

#endif


