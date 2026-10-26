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
#ifndef WAX_C_STREAMREADER_DOT_H
#define WAX_C_STREAMREADER_DOT_H
#pragma once

#include <wax_c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef	int(*wax_readStream_func)(int nBytes, void* pDest);


	WAX_EXPORT wax_obj		wax_createStreamReader(wax_readStream_func callback);

	WAX_EXPORT wax_component	wax_getStreamReaderOutput(wax_obj streamReader);

	WAX_EXPORT void			wax_resetStreamReader(wax_obj streamReader);

	WAX_EXPORT int			wax_streamReaderCapacity(wax_obj streamReader);
	WAX_EXPORT int			wax_streamReaderHasChunk(wax_obj streamReader);
	WAX_EXPORT int			wax_streamReaderBytes(wax_obj streamReader);

#ifdef __cplusplus
}
#endif

#endif
