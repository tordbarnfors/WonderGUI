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
#ifndef WAX_C_STREAMBUFFER_DOT_H
#define WAX_C_STREAMBUFFER_DOT_H
#pragma once

#include <wax_c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

	WAX_EXPORT wax_obj		wax_createStreamBuffer(int initialCapacity);

	WAX_EXPORT wax_component	wax_getStreamBufferOutput(wax_obj streamBuffer);
	WAX_EXPORT wax_component	wax_getStreamBufferInput(wax_obj streamBuffer);

	WAX_EXPORT int			wax_streamBufferCapacity(wax_obj streamBuffer);
	WAX_EXPORT int			wax_streamBufferHasChunk(wax_obj streamBuffer);
	WAX_EXPORT int			wax_streamBufferSize(wax_obj streamBuffer);

	WAX_EXPORT void			wax_resetStreamBuffer(wax_obj streamBuffer);
	WAX_EXPORT int			wax_setStreamBufferCapacity( wax_obj streamBuffer, int capacity );



#ifdef __cplusplus
}
#endif

#endif
