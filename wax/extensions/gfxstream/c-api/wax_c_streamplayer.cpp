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
#include <tuple>

#include <wax_c_streamplayer.h>

#include <wax_streamplayer.h>

using namespace wax;

inline StreamPlayer* getPtr(wax_obj obj) {
	return static_cast<StreamPlayer*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createStreamPlayer(wax_obj backend, wax_obj surfaceFactory, wax_obj edgemapFactory)
{
	Backend* pBackend = static_cast<Backend*>(reinterpret_cast<Object*>(backend));
	SurfaceFactory* pSurfaceFactory = static_cast<SurfaceFactory*>(reinterpret_cast<Object*>(surfaceFactory));
	EdgemapFactory* pEdgemapFactory = static_cast<EdgemapFactory*>(reinterpret_cast<Object*>(edgemapFactory));

	auto p = StreamPlayer::create(pBackend, pSurfaceFactory, pEdgemapFactory);
	p->retain();
	return p.rawPtr();
}

wax_component wax_getStreamPlayerInput(wax_obj streamPlayer)
{
	auto p = getPtr(streamPlayer);
	return { static_cast<Object*>(p), &p->input };
}

void wax_resetStreamPlayer(wax_obj streamPlayer)
{
	auto p = getPtr(streamPlayer);
	p->reset();
}


void wax_setStreamPlayerStoreDirtyRects(wax_obj streamPlayer, int bStore)
{
	getPtr(streamPlayer)->setStoreDirtyRects(bStore);
}

void wax_setStreamPlayerMaxDirtyRects(wax_obj streamPlayer, int max)
{
	getPtr(streamPlayer)->setMaxDirtyRects(max);
}

int wax_getStreamPlayerDirtyRects(wax_obj streamPlayer, wax_canvasRef canvas, const wax_rectI** pRects)
{
	int size;
	const RectI* pDirtyRects;
	std::tie(size,pDirtyRects) = getPtr(streamPlayer)->dirtyRects( (CanvasRef) canvas);
	*pRects = (wax_rectI*) pDirtyRects;
	return size;
}

void wax_clearStreamPlayerDirtyRects(wax_obj streamPlayer)
{
	getPtr(streamPlayer)->clearDirtyRects();
}

void wax_setStreamPlayerFenceCallback(wax_obj streamPlayer, wax_streamPlayerFence_func callback)
{
	getPtr(streamPlayer)->setFenceCallback( callback );
}
