
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

#include <wg_c_streamwrapper.h>
#include <wg_streamwrapper.h>

using namespace wg;

inline StreamWrapper* getPtr(wg_obj obj) {
	return static_cast<StreamWrapper*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createStreamWrapper( const void * pBegin, const void * pEnd )
{
	auto p = StreamWrapper::create(pBegin, pEnd);
	p->retain();
	return p.rawPtr();
}

wg_component wg_getStreamWrapperOutput(wg_obj streamWrapper)
{
	auto p = getPtr(streamWrapper);
	return { static_cast<Object*>(p), &p->output };
}

int wg_chunkInStreamWrapper(wg_obj streamWrapper)
{
	auto p = getPtr(streamWrapper);
	return p->chunkInBuffer();
}

int wg_bytesInStreamWrapper(wg_obj streamWrapper)
{
	auto p = getPtr(streamWrapper);
	return p->bytesInBuffer();
}

void	wg_restartStreamWrapper(wg_obj streamWrapper)
{
	auto p = getPtr(streamWrapper);
	p->restart();
}


