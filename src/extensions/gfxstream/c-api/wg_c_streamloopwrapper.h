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

#ifndef WG_C_STREAMLOOPWRAPPER_DOT_H
#define WG_C_STREAMLOOPWRAPPER_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef	const void *(*wg_getWritePtr_func)(void);
	typedef	void(*wg_setReadPtr_func)(const void * pReadPos);

	WG_EXPORT wg_obj		wg_createStreamLoopWrapper(const void * pBufferBegin, const void * pBufferEnd,
											   wg_getWritePtr_func getWritePtrFunc,
											   wg_setReadPtr_func setReadPtrFunc);

	WG_EXPORT wg_component	wg_getStreamLoopWrapperOutput(wg_obj loopWrapper);


#ifdef __cplusplus
}
#endif


#endif

