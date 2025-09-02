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

#ifndef WG_C_STREAMREADER_DOT_H
#define WG_C_STREAMREADER_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef	int(*wg_readStream_func)(int nBytes, void* pDest);


	WG_EXPORT wg_obj		wg_createStreamReader(wg_readStream_func callback);

	WG_EXPORT wg_component	wg_getStreamReaderOutput(wg_obj streamReader);

	WG_EXPORT void			wg_resetStreamReader(wg_obj streamReader);

	WG_EXPORT int			wg_streamReaderCapacity(wg_obj streamReader);
	WG_EXPORT int			wg_streamReaderHasChunk(wg_obj streamReader);
	WG_EXPORT int			wg_streamReaderBytes(wg_obj streamReader);

#ifdef __cplusplus
}
#endif

#endif
