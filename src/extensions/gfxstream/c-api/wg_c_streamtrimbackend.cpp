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


#include <wg_c_streamtrimbackend.h>
#include <wg_streamtrimbackend.h>

using namespace wg;

inline StreamTrimBackend* getPtr(wg_obj obj) {
	return static_cast<StreamTrimBackend*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createStreamTrimBackend(wg_obj backend)
{
	auto p = StreamTrimBackend::create(static_cast<GfxBackend*>(reinterpret_cast<Object*>(backend)));
	p->retain();
	return static_cast<Object*>(p.rawPtr());
}

void wg_addNonMaskingSession(wg_obj streamTrimBackend)
{
	getPtr(streamTrimBackend)->addNonMaskingSession();
}

void wg_addFullyMaskingSession(wg_obj streamTrimBackend,  wg_canvasRef canvasRef, wg_obj canvasSurface )
{
	getPtr(streamTrimBackend)->addFullyMaskingSession((CanvasRef) canvasRef, static_cast<Surface*>(reinterpret_cast<Object*>(canvasSurface)));
}

void wg_addMaskingSession(wg_obj streamTrimBackend,  wg_canvasRef canvasRef, wg_obj canvasSurface, int nMaskingRects, const wg_rectSPX * pMaskingRects )
{
	getPtr(streamTrimBackend)->addMaskingSession((CanvasRef) canvasRef, static_cast<Surface*>(reinterpret_cast<Object*>(canvasSurface)), nMaskingRects, (const RectSPX*) pMaskingRects );
}

void wg_clearSessionMasks(wg_obj streamTrimBackend)
{
	getPtr(streamTrimBackend)->clearSessionMasks();
}

void wg_setTrimLevel(wg_obj streamTrimBackend, int level)
{
	getPtr(streamTrimBackend)->setTrimLevel(level);
}
