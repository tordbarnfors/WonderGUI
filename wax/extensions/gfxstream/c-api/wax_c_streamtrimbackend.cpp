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
#include <wax_c_streamtrimbackend.h>
#include <wax_streamtrimbackend.h>

using namespace wax;

inline StreamTrimBackend* getPtr(wax_obj obj) {
	return static_cast<StreamTrimBackend*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createStreamTrimBackend(wax_obj backend)
{
	auto p = StreamTrimBackend::create(static_cast<Backend*>(reinterpret_cast<Object*>(backend)));
	p->retain();
	return static_cast<Object*>(p.rawPtr());
}

void wax_addNonMaskingSession(wax_obj streamTrimBackend)
{
	getPtr(streamTrimBackend)->addNonMaskingSession();
}

void wax_addFullyMaskingSession(wax_obj streamTrimBackend,  wax_canvasRef canvasRef, wax_obj canvasSurface )
{
	getPtr(streamTrimBackend)->addFullyMaskingSession((CanvasRef) canvasRef, static_cast<Surface*>(reinterpret_cast<Object*>(canvasSurface)));
}

void wax_addMaskingSession(wax_obj streamTrimBackend,  wax_canvasRef canvasRef, wax_obj canvasSurface, int nMaskingRects, const wax_rectSPX * pMaskingRects )
{
	getPtr(streamTrimBackend)->addMaskingSession((CanvasRef) canvasRef, static_cast<Surface*>(reinterpret_cast<Object*>(canvasSurface)), nMaskingRects, (const RectSPX*) pMaskingRects );
}

void wax_clearSessionMasks(wax_obj streamTrimBackend)
{
	getPtr(streamTrimBackend)->clearSessionMasks();
}

void wax_setTrimLevel(wax_obj streamTrimBackend, int level)
{
	getPtr(streamTrimBackend)->setTrimLevel(level);
}
