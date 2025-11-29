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
#ifndef WG_C_STREAMSOURCE_DOT_H
#define WG_C_STREAMSOURCE_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_gfxstream.h>

#ifdef __cplusplus
extern "C" {
#endif

	WG_EXPORT int				wg_hasStreamChunks( wg_component streamSource );
	WG_EXPORT int				wg_showStreamChunks( wg_component streamSource, wg_streamData ** pSegments );
    WG_EXPORT void				wg_discardStreamChunks( wg_component streamSource, int bytes );
    WG_EXPORT int				wg_fetchStreamChunks( wg_component streamSource );
    WG_EXPORT wg_streamChunkId	wg_peekStreamChunk( wg_component streamSource );

#ifdef __cplusplus
}
#endif

#endif
