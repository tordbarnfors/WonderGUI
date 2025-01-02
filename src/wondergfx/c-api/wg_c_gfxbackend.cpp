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

#include <wg_c_gfxbackend.h>
#include <wg_gfxbackend.h>

using namespace wg;

inline GfxBackend* getPtr(wg_obj obj) {
	return static_cast<GfxBackend*>(reinterpret_cast<Object*>(obj));
}


void wg_backendBeginRender( wg_obj backend )
{
	getPtr(backend)->beginRender();
}

void wg_backendEndRender( wg_obj backend )
{
	getPtr(backend)->endRender();
}

void wg_backendBeginSession( wg_obj backend, wg_canvasRef canvasRef, wg_obj canvasSurface, int nUpdateRects, const wg_rectSPX * pUpdateRects, const wg_sessionInfo * pInfo )
{
	getPtr(backend)->beginSession( (CanvasRef) canvasRef, static_cast<Surface*>(reinterpret_cast<Object*>(canvasSurface)), nUpdateRects, (const RectSPX*) pUpdateRects, (const GfxBackend::SessionInfo *) pInfo );
}

void wg_backendEndSession( wg_obj backend )
{
	getPtr(backend)->endSession();
}

void wg_backendSetCanvasWithSurface( wg_obj backend, wg_obj surface )
{
	getPtr(backend)->setCanvas(static_cast<Surface*>(reinterpret_cast<Object*>(surface)));
}

void wg_backendSetCanvasWithRef( wg_obj backend, wg_canvasRef canvas )
{
	getPtr(backend)->setCanvas( (CanvasRef) canvas );
}

void wg_backendSetObjects(wg_obj backend, wg_obj* const * pBeg, wg_obj* const * pEnd)
{
	getPtr(backend)->setObjects( reinterpret_cast<Object* const *>(pBeg), reinterpret_cast<Object* const *>(pEnd) );
}

void wg_backendSetRects(wg_obj backend, const wg_rectSPX* pBeg, const wg_rectSPX* pEnd)
{
	getPtr(backend)->setRects( (const RectSPX *) pBeg, (const RectSPX *) pEnd );
}

void wg_backendSetColors(wg_obj backend, const wg_color * pBeg, const wg_color * pEnd)
{
	getPtr(backend)->setColors( (const HiColor *) pBeg, (const HiColor *) pEnd );
}

void wg_backendSetTransforms(wg_obj backend, const wg_transform * pBeg, const wg_transform * pEnd)
{
	getPtr(backend)->setTransforms( (const Transform *) pBeg, (const Transform *) pEnd );
}

void wg_backendProcessCommands(wg_obj backend, const uint16_t* pBeg, const uint16_t * pEnd)
{
	getPtr(backend)->processCommands( pBeg, pEnd );
}

wg_canvasInfo wg_backendCanvasInfo(wg_obj backend, wg_canvasRef ref)
{
	auto canvasInfo = getPtr(backend)->canvasInfo((CanvasRef)ref);
	return { (wg_canvasRef)canvasInfo->ref, canvasInfo->pSurface, {canvasInfo->size.w, canvasInfo->size.h}, canvasInfo->scale };
}

wg_obj wg_backendSurfaceFactory(wg_obj backend)
{
	return getPtr(backend)->surfaceFactory().rawPtr();
}

wg_obj wg_backendEdgemapFactory(wg_obj backend)
{
	return getPtr(backend)->edgemapFactory().rawPtr();
}

int wg_backendMaxEdges(wg_obj backend)
{
	return getPtr(backend)->maxEdges();
}

const wg_typeInfo * wg_backendSurfaceType(wg_obj backend)
{
	auto& typeInfo = getPtr(backend)->surfaceType();
	return reinterpret_cast<const wg_typeInfo*>(&typeInfo);
}
