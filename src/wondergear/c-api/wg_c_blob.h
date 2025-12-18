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
#ifndef WG_C_BLOB_DOT_H
#define WG_C_BLOB_DOT_H
#pragma once

#include <wg_c_geartypes.h>

#ifdef __cplusplus
extern "C" {
#endif

	WG_EXPORT wg_obj 	wg_createBlob( int size );
	WG_EXPORT wg_obj 	wg_createBlobFromData( void * pData, int size, void(*destructor)() );

	WG_EXPORT int		wg_blobSize( wg_obj blob );
	WG_EXPORT void *	wg_blobData( wg_obj blob );

#ifdef __cplusplus
}
#endif

#endif


