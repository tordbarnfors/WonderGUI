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
#ifndef WAX_C_BLOB_DOT_H
#define WAX_C_BLOB_DOT_H
#pragma once

#include <wax_c_types.h>

#ifdef __cplusplus
extern "C" {
#endif


enum
{
	WAX_SAVE_IDENTITY = 1,
	WAX_SAVE_SAMPLE_METHOD = 2,
	WAX_SAVE_SCALE = 4,
	WAX_SAVE_BUFFERED_FLAG = 8,
	WAX_SAVE_CANVAS_FLAG = 16,
	WAX_SAVE_DYNAMIC_FLAG = 32,
	WAX_SAVE_TILING_FLAG = 64,
	WAX_SAVE_MIPMAP_FLAG = 128

} wax_surfaceWriteSaveInfo;

//____ wax_surfaceWriterBP __________________________________________________________

typedef struct wax_surfaceWriterBP_struct			// NOT BINARY EQUIVALENT!
{
	int			saveInfo;
	
} wax_surfaceWriterBP;

WAX_EXPORT wax_obj 	wax_createSurfaceWriter(const wax_surfaceWriterBP* blueprint);
WAX_EXPORT wax_obj	wax_writeSurfaceToBlob( wax_obj surfaceWriter, wax_obj surface, int extraDataSize, char * pExtraData );

#ifdef __cplusplus
}
#endif

#endif


