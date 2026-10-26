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
#ifndef WAX_C_GFXSTREAM_DOT_H
#define WAX_C_GFXSTREAM_DOT_H
#pragma once


#ifdef __cplusplus
extern "C" {
#endif


//____ wax_streamData _________________________________________________________

typedef struct wax_steamData_struct
{
	uint8_t*	pBegin;
	uint8_t*	pEnd;
} wax_streamData;

//____ wax_streamChunkId ____________________________________________________

typedef enum
{
	WAX_STREAMCHUNK_OUT_OF_DATA = 0,

	WAX_STREAMCHUNK_PROTOCOL_VERSION = 1,
	WAX_STREAMCHUNK_CANVAS_LIST = 2,
	WAX_STREAMCHUNK_TIMESTAMP = 3,

	WAX_STREAMCHUNK_BEGIN_RENDER = 4,
	WAX_STREAMCHUNK_END_RENDER = 5,
	WAX_STREAMCHUNK_BEGIN_SESSION = 6,
	WAX_STREAMCHUNK_END_SESSION = 7,
	WAX_STREAMCHUNK_SET_CANVAS = 8,
	WAX_STREAMCHUNK_OBJECTS = 9,
	WAX_STREAMCHUNK_RECTS = 10,
	WAX_STREAMCHUNK_COLORS = 11,
	WAX_STREAMCHUNK_TRANSFORMS = 12,
	WAX_STREAMCHUNK_COMMANDS = 13,
	WAX_STREAMCHUNK_UPDATE_RECTS = 14,

	WAX_STREAMCHUNK_CREATE_SURFACE = 15,
	WAX_STREAMCHUNK_BEGIN_SURFACE_UPDATE = 16,
	WAX_STREAMCHUNK_SURFACE_PIXELS = 17,
	WAX_STREAMCHUNK_END_SURFACE_UPDATE = 18,
	WAX_STREAMCHUNK_FILL_SURFACE = 19,
	WAX_STREAMCHUNK_COPY_SURFACE = 20,
	WAX_STREAMCHUNK_DELETE_SURFACE = 21,

	WAX_STREAMCHUNK_CREATE_EDGEMAP = 22,
	WAX_STREAMCHUNK_SET_EDGEMAP_RENDER_SEGMENTS = 23,
	WAX_STREAMCHUNK_SET_EDGEMAP_COLORS = 24,
	WAX_STREAMCHUNK_BEGIN_EDGEMAP_UPDATE = 25,
	WAX_STREAMCHUNK_EDGEMAP_SAMPLES = 26,
	WAX_STREAMCHUNK_END_EDGEMAP_UPDATE = 27,
	WAX_STREAMCHUNK_DELETE_EDGEMAP = 28,
	
} wax_streamChunkId;


#ifdef __cplusplus
}
#endif

#endif
