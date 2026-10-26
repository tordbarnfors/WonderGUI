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
#include <wg_c_streamloopwrapper.h>
#include <wg_streamloopwrapper.h>

using namespace wg;

inline StreamLoopWrapper* getPtr(wg_obj obj) {
	return static_cast<StreamLoopWrapper*>(reinterpret_cast<Object*>(obj));
}

wg_obj	wg_createStreamLoopWrapper(const void * pBufferBegin, const void * pBufferEnd,
											wg_getWritePtr_func getWritePtrFunc,
										   wg_setReadPtr_func setReadPtrFunc)
{
	auto p = StreamLoopWrapper::create(pBufferBegin, pBufferEnd, getWritePtrFunc, setReadPtrFunc);
	p->retain();
	return p.rawPtr();

}

wg_component	wg_getStreamLoopWrapperOutput(wg_obj loopWrapper)
{
	auto p = getPtr(loopWrapper);
	return { static_cast<Object*>(p), &p->output };
}
