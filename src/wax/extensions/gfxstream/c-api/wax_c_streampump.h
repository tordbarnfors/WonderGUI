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
#ifndef WAX_C_STREAMPUMP_DOT_H
#define WAX_C_STREAMPUMP_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>
#include <wax_c_gfxstream.h>

#ifdef __cplusplus
extern "C" {
#endif

	WAX_EXPORT wax_obj			wax_createStreamPump(void);
	WAX_EXPORT wax_obj			wax_createStreamPumpWithInputOutput( wax_component input, wax_component output);

	WAX_EXPORT void				wax_setStreamPumpInput(wax_obj streamPump, wax_component input);
	WAX_EXPORT void				wax_setStreamPumpOutput(wax_obj streamPump, wax_component output);

	WAX_EXPORT void				wax_setFlowControl(wax_obj streamPump, uint16_t fenceId, int credits, int bytesPerCredit );
	WAX_EXPORT void				wax_restartFlowControl(wax_obj streamPump, int credits);
	WAX_EXPORT void				wax_AddFlowControlCredits(wax_obj streamPump, int credits);



	WAX_EXPORT wax_streamChunkId	wax_peekChunk(wax_obj streamPump);


	WAX_EXPORT int				wax_pumpChunk(wax_obj streamPump);
	WAX_EXPORT int				wax_pumpUntilFrame(wax_obj streamPump);
	WAX_EXPORT int				wax_pumpFrame(wax_obj streamPump);
	WAX_EXPORT int				wax_pumpAllFrames(wax_obj streamPump);
	WAX_EXPORT int				wax_pumpAllFramesWithTrimming(wax_obj streamPump, wax_obj trimBackend);
	WAX_EXPORT int				wax_pumpAll(wax_obj streamPump);

	WAX_EXPORT int				wax_pumpBytes( wax_obj streamPump, int maxBytes );

#ifdef __cplusplus
}
#endif


#endif
