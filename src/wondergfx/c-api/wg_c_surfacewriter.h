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

#ifndef WG_C_BLOB_DOT_H
#define WG_C_BLOB_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>

#ifdef __cplusplus
extern "C" {
#endif


enum
{
	WG_SAVE_IDENTITY = 1,
	WG_SAVE_SAMPLE_METHOD = 2,
	WG_SAVE_SCALE = 4,
	WG_SAVE_BUFFERED_FLAG = 8,
	WG_SAVE_CANVAS_FLAG = 16,
	WG_SAVE_DYNAMIC_FLAG = 32,
	WG_SAVE_TILING_FLAG = 64,
	WG_SAVE_MIPMAP_FLAG = 128

} wg_surfaceWriteSaveInfo;

//____ wg_surfaceWriterBP __________________________________________________________

typedef struct wg_surfaceWriterBP_struct			// NOT BINARY EQUIVALENT!
{
	int			saveInfo;
	
} wg_surfaceWriterBP;

WG_EXPORT wg_obj 	wg_createSurfaceWriter(const wg_surfaceWriterBP* blueprint);
WG_EXPORT wg_obj	wg_writeSurfaceToBlob( wg_obj surfaceWriter, wg_obj surface, int extraDataSize, char * pExtraData );

#ifdef __cplusplus
}
#endif

#endif


