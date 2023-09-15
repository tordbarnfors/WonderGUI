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

#ifndef WG_C_GFXSTREAMBUFFER_DOT_H
#define WG_C_GFXSTREAMBUFFER_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

	wg_obj			wg_createStreamBuffer(int initialCapacity);

	wg_component	wg_getStreamBufferOutput(wg_obj streamBuffer);
	wg_component	wg_getStreamBufferInput(wg_obj streamBuffer);

	int				wg_streamBufferCapacity(wg_obj streamBuffer);
	int				wg_streamBufferHasChunk(wg_obj streamBuffer);
	int				wg_streamBufferSize(wg_obj streamBuffer);

	void			wg_resetStreamBuffer(wg_obj streamBuffer);
	int				wg_setStreamBufferCapacity( wg_obj streamBuffer, int capacity );



#ifdef __cplusplus
}
#endif

#endif
