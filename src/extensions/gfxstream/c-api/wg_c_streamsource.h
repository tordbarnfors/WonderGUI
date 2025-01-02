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

#ifndef WG_C_STREAMSOURCE_DOT_H
#define WG_C_STREAMSOURCE_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_gfxstream.h>

#ifdef __cplusplus
extern "C" {
#endif

    int             wg_hasStreamChunks( wg_component streamSource );
    int             wg_showStreamChunks( wg_component streamSource, wg_streamData ** pSegments );
    void            wg_discardStreamChunks( wg_component streamSource, int bytes );
    int             wg_fetchStreamChunks( wg_component streamSource );
    wg_streamChunkId   wg_peekStreamChunk( wg_component streamSource );

#ifdef __cplusplus
}
#endif

#endif
