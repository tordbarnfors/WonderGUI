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
#ifndef WG_C_SURFACEREADER_DOT_H
#define WG_C_SURFACEREADER_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//____ wg_surfaceReaderBP __________________________________________________________

typedef struct wg_surfaceReaderBP_struct			// NOT BINARY EQUIVALENT!
{
	wg_obj				factory;
} wg_surfaceReaderBP;

WG_EXPORT wg_obj 	wg_createSurfaceReader(const wg_surfaceReaderBP* blueprint);
WG_EXPORT wg_obj	wg_readSurfaceFromBlob( wg_obj surfaceReader, wg_obj blob );
WG_EXPORT wg_obj	wg_readSurfaceFromMemory( wg_obj surfaceReader, const char * pData );

#ifdef __cplusplus
}
#endif

#endif


