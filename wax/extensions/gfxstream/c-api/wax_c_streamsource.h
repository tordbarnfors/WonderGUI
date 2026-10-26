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
#ifndef WAX_C_STREAMSOURCE_DOT_H
#define WAX_C_STREAMSOURCE_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_gfxstream.h>

#ifdef __cplusplus
extern "C" {
#endif

	WAX_EXPORT int				wax_hasStreamChunks( wax_component streamSource );
	WAX_EXPORT int				wax_showStreamChunks( wax_component streamSource, wax_streamData ** pSegments );
    WAX_EXPORT void				wax_discardStreamChunks( wax_component streamSource, int bytes );
    WAX_EXPORT int				wax_fetchStreamChunks( wax_component streamSource );
    WAX_EXPORT wax_streamChunkId	wax_peekStreamChunk( wax_component streamSource );

#ifdef __cplusplus
}
#endif

#endif
