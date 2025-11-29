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
#ifndef WG_C_STREAMPUMP_DOT_H
#define WG_C_STREAMPUMP_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>
#include <wg_c_gfxstream.h>

#ifdef __cplusplus
extern "C" {
#endif

	WG_EXPORT wg_obj			wg_createStreamPump(void);
	WG_EXPORT wg_obj			wg_createStreamPumpWithInputOutput( wg_component input, wg_component output);

	WG_EXPORT void				wg_setStreamPumpInput(wg_obj streamPump, wg_component input);
	WG_EXPORT void				wg_setStreamPumpOutput(wg_obj streamPump, wg_component output);


	WG_EXPORT wg_streamChunkId	wg_peekChunk(wg_obj streamPump);


	WG_EXPORT int				wg_pumpChunk(wg_obj streamPump);
	WG_EXPORT int				wg_pumpUntilFrame(wg_obj streamPump);
	WG_EXPORT int				wg_pumpFrame(wg_obj streamPump);
	WG_EXPORT int				wg_pumpAllFrames(wg_obj streamPump);
	WG_EXPORT int				wg_pumpAllFramesWithTrimming(wg_obj streamPump, wg_obj trimBackend);
	WG_EXPORT int				wg_pumpAll(wg_obj streamPump);

	WG_EXPORT int				wg_pumpBytes( wg_obj streamPump, int maxBytes );

#ifdef __cplusplus
}
#endif


#endif
