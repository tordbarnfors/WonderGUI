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
#include <wax_c_gfxbackend.h>
#include <cstddef>
#include <wax_gfxbackend.h>

using namespace wax;

static_assert(sizeof(wax_sessionInfo) == sizeof(GfxBackend::SessionInfo), "wax_sessionInfo differs in size from GfxBackend::SessionInfo!");
static_assert(offsetof(wax_sessionInfo, nWarpDraws) == offsetof(GfxBackend::SessionInfo, nWarpDraws), "wax_sessionInfo isn't binary equivalent to GfxBackend::SessionInfo!");
static_assert(offsetof(wax_sessionInfo, nShapeTints) == offsetof(GfxBackend::SessionInfo, nShapeTints), "wax_sessionInfo isn't binary equivalent to GfxBackend::SessionInfo!");

inline GfxBackend* getPtr(wax_obj obj) {
	return static_cast<GfxBackend*>(reinterpret_cast<Object*>(obj));
}


void wax_backendBeginRender( wax_obj backend )
{
	getPtr(backend)->beginRender();
}

void wax_backendEndRender( wax_obj backend )
{
	getPtr(backend)->endRender();
}

void wax_backendBeginSession( wax_obj backend, wax_canvasRef canvasRef, wax_obj canvasSurface, int nUpdateRects, const wax_rectSPX * pUpdateRects, const wax_sessionInfo * pInfo )
{
	getPtr(backend)->beginSession( (CanvasRef) canvasRef, static_cast<Surface*>(reinterpret_cast<Object*>(canvasSurface)), nUpdateRects, (const RectSPX*) pUpdateRects, (const GfxBackend::SessionInfo *) pInfo );
}

void wax_backendEndSession( wax_obj backend )
{
	getPtr(backend)->endSession();
}

void wax_backendSetCanvasWithSurface( wax_obj backend, wax_obj surface )
{
	getPtr(backend)->setCanvas(static_cast<Surface*>(reinterpret_cast<Object*>(surface)));
}

void wax_backendSetCanvasWithRef( wax_obj backend, wax_canvasRef canvas )
{
	getPtr(backend)->setCanvas( (CanvasRef) canvas );
}

void wax_backendSetObjects(wax_obj backend, wax_obj* const * pBeg, wax_obj* const * pEnd)
{
	getPtr(backend)->setObjects( reinterpret_cast<Object* const *>(pBeg), reinterpret_cast<Object* const *>(pEnd) );
}

void wax_backendSetRects(wax_obj backend, const wax_rectSPX* pBeg, const wax_rectSPX* pEnd)
{
	getPtr(backend)->setRects( (const RectSPX *) pBeg, (const RectSPX *) pEnd );
}

void wax_backendSetColors(wax_obj backend, const wax_color * pBeg, const wax_color * pEnd)
{
	getPtr(backend)->setColors( (const HiColor *) pBeg, (const HiColor *) pEnd );
}

void wax_backendSetTransforms(wax_obj backend, const wax_transform * pBeg, const wax_transform * pEnd)
{
	getPtr(backend)->setTransforms( (const Transform *) pBeg, (const Transform *) pEnd );
}

void wax_backendProcessCommands(wax_obj backend, const uint16_t* pBeg, const uint16_t * pEnd, int version)
{
	getPtr(backend)->processCommands( pBeg, pEnd, version );
}

wax_canvasInfo wax_backendCanvasInfo(wax_obj backend, wax_canvasRef ref)
{
	auto canvasInfo = getPtr(backend)->canvasInfo((CanvasRef)ref);
	return { (wax_canvasRef)canvasInfo->ref, canvasInfo->pSurface, {canvasInfo->size.w, canvasInfo->size.h}, canvasInfo->scale };
}

wax_obj wax_backendSurfaceFactory(wax_obj backend)
{
	return getPtr(backend)->surfaceFactory().rawPtr();
}

wax_obj wax_backendEdgemapFactory(wax_obj backend)
{
	return getPtr(backend)->edgemapFactory().rawPtr();
}

wax_obj wax_backendWarpmapFactory(wax_obj backend)
{
	return getPtr(backend)->warpmapFactory().rawPtr();
}

int wax_backendMaxEdges(wax_obj backend)
{
	return getPtr(backend)->maxEdges();
}

