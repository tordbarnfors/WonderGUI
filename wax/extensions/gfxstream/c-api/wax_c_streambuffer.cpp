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
#include <wax_c_streambuffer.h>
#include <wax_streambuffer.h>

using namespace wax;

inline StreamBuffer* getPtr(wax_obj obj) {
	return static_cast<StreamBuffer*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createStreamBuffer(int initialCapacity)
{
	auto pObj = StreamBuffer::create(initialCapacity);
	pObj->retain();
	return static_cast<Object*>(pObj.rawPtr());
}

wax_component wax_getStreamBufferOutput(wax_obj streamBuffer)
{
	auto p = getPtr(streamBuffer);
	return { static_cast<Object*>(p), &p->output };
}

wax_component wax_getStreamBufferInput(wax_obj streamBuffer)
{
	auto p = getPtr(streamBuffer);
	return { static_cast<Object*>(p), &p->input };
}

int wax_streamBufferCapacity(wax_obj streamBuffer)
{
	return getPtr(streamBuffer)->capacity();
}

int wax_streamBufferHasChunk(wax_obj streamBuffer)
{
	return getPtr(streamBuffer)->hasChunk();
}

int wax_streamBufferSize(wax_obj streamBuffer)
{
	return getPtr(streamBuffer)->size();
}

void wax_resetStreamBuffer(wax_obj streamBuffer)
{
	getPtr(streamBuffer)->reset();
}

int wax_setStreamBufferCapacity( wax_obj streamBuffer, int capacity )
{
	return getPtr(streamBuffer)->setCapacity(capacity);
}



