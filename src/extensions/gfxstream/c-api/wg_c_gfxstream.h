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

#ifndef WG_C_GFXSTREAM_DOT_H
#define WG_C_GFXSTREAM_DOT_H
#pragma once


#ifdef __cplusplus
extern "C" {
#endif


//____ wg_streamData _________________________________________________________

typedef struct wg_steamData_struct
{
	uint8_t*	pBegin;
	uint8_t*	pEnd;
} wg_streamData;

//____ wg_streamChunkId ____________________________________________________

typedef enum
{
	WG_STREAMCHUNK_OUT_OF_DATA = 0,

	WG_STREAMCHUNK_PROTOCOL_VERSION = 1,
	WG_STREAMCHUNK_CANVAS_LIST = 2,
	WG_STREAMCHUNK_TIMESTAMP = 3,

	WG_STREAMCHUNK_BEGIN_RENDER = 4,
	WG_STREAMCHUNK_END_RENDER = 5,
	WG_STREAMCHUNK_BEGIN_SESSION = 6,
	WG_STREAMCHUNK_END_SESSION = 7,
	WG_STREAMCHUNK_SET_CANVAS = 8,
	WG_STREAMCHUNK_OBJECTS = 9,
	WG_STREAMCHUNK_RECTS = 10,
	WG_STREAMCHUNK_COLORS = 11,
	WG_STREAMCHUNK_TRANSFORMS = 12,
	WG_STREAMCHUNK_COMMANDS = 13,
	WG_STREAMCHUNK_UPDATE_RECTS = 14,

	WG_STREAMCHUNK_CREATE_SURFACE = 15,
	WG_STREAMCHUNK_BEGIN_SURFACE_UPDATE = 16,
	WG_STREAMCHUNK_SURFACE_PIXELS = 17,
	WG_STREAMCHUNK_END_SURFACE_UPDATE = 18,
	WG_STREAMCHUNK_FILL_SURFACE = 19,
	WG_STREAMCHUNK_COPY_SURFACE = 20,
	WG_STREAMCHUNK_DELETE_SURFACE = 21,

	WG_STREAMCHUNK_CREATE_EDGEMAP = 22,
	WG_STREAMCHUNK_SET_EDGEMAP_RENDER_SEGMENTS = 23,
	WG_STREAMCHUNK_SET_EDGEMAP_COLORS = 24,
	WG_STREAMCHUNK_BEGIN_EDGEMAP_UPDATE = 25,
	WG_STREAMCHUNK_EDGEMAP_SAMPLES = 26,
	WG_STREAMCHUNK_END_EDGEMAP_UPDATE = 27,
	WG_STREAMCHUNK_DELETE_EDGEMAP = 28,
	
} wg_streamChunkId;


#ifdef __cplusplus
}
#endif

#endif
