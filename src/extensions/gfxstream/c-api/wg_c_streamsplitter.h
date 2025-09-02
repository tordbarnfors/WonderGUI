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

#ifndef WG_C_STREAMSPLITTER_DOT_H
#define WG_C_STREAMSPLITTER_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	WG_EXPORT wg_obj	wg_createStreamSplitter();
	WG_EXPORT wg_obj	wg_createStreamSplitterWithOutputs( wg_component output1, wg_component output2 );

	WG_EXPORT wg_component	wg_getStreamSplitterInput(wg_obj streamSplitter);

	WG_EXPORT void		wg_clearStreamSplitterOutputs(wg_obj streamSplitter);
	WG_EXPORT void		wg_addStreamSplitterOutput(wg_obj streamSplitter, wg_component output);
	WG_EXPORT void		wg_removeStreamSplitterOutput(wg_obj streamSplitter, wg_component output);


#ifdef __cplusplus
}
#endif


#endif



