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
#include <wax_c_streamwrapper.h>
#include <wax_streamwrapper.h>

using namespace wax;

inline StreamWrapper* getPtr(wax_obj obj) {
	return static_cast<StreamWrapper*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createStreamWrapper( const void * pBegin, const void * pEnd )
{
	auto p = StreamWrapper::create(pBegin, pEnd);
	p->retain();
	return p.rawPtr();
}

wax_component wax_getStreamWrapperOutput(wax_obj streamWrapper)
{
	auto p = getPtr(streamWrapper);
	return { static_cast<Object*>(p), &p->output };
}

int wax_chunkInStreamWrapper(wax_obj streamWrapper)
{
	auto p = getPtr(streamWrapper);
	return p->chunkInBuffer();
}

int wax_bytesInStreamWrapper(wax_obj streamWrapper)
{
	auto p = getPtr(streamWrapper);
	return p->bytesInBuffer();
}

void	wax_restartStreamWrapper(wax_obj streamWrapper)
{
	auto p = getPtr(streamWrapper);
	p->restart();
}


