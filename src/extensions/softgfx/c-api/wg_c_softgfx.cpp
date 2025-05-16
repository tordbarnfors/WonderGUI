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

#include <wg_c_softgfx.h>
#include <wg_c_internal.h>

#include <wg_softbackend.h>
#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softedgemap.h>
#include <wg_softedgemapfactory.h>

using namespace wg;

wg_obj wg_createSoftBackend(void)
{
	auto p = SoftBackend::create();
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

int wg_defineSoftBackendCanvas( wg_obj backend, wg_canvasRef ref, wg_obj softSurface )
{
	auto pBackend = static_cast<SoftBackend*>(reinterpret_cast<Object*>(backend));
	auto pSurface = static_cast<SoftSurface*>(reinterpret_cast<Object*>(softSurface));

	return pBackend->defineCanvas((CanvasRef) ref, pSurface);
}

wg_obj wg_createSoftSurface(const wg_surfaceBP* blueprint)
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);

	auto p = SoftSurface::create(bp);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}


wg_obj wg_createSoftSurfaceFromBlob(const wg_surfaceBP* blueprint, wg_obj blob, int pitch)
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto p = SoftSurface::create(bp, static_cast<Blob*>(reinterpret_cast<Object*>(blob)), pitch );
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj	wg_createSoftSurfaceFromBitmap(wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels,
									   wg_pixelFormat pixelFormat, int pitch, const wg_color8 * pPalette )
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto p = SoftSurface::create(bp, pPixels, (PixelFormat) pixelFormat, pitch, (const Color8*) pPalette);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj	wg_createSoftSurfaceFromRawData(wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels,
										const wg_pixelDescription * pPixelDescription, int pitch, const wg_color8 * pPalette )
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	
	auto p = SoftSurface::create(bp, pPixels, *(PixelDescription*) pPixelDescription, pitch, (const Color8*) pPalette);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}


wg_obj wg_createSoftSurfaceFactory()
{
	auto p = SoftSurfaceFactory::create();
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}


wg_obj wg_createSoftEdgemap( const wg_edgemapBP* blueprint )
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tintmap_p	tintmapArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintmapArea);

	auto p = SoftEdgemap::create(bp);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj wg_createSoftEdgemapFromFloats( const wg_edgemapBP* blueprint, wg_sampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tintmap_p	tintmapArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintmapArea);

	auto p = SoftEdgemap::create(bp, (SampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj wg_createSoftEdgemapFromSpx( const wg_edgemapBP* blueprint, wg_sampleOrigo origo, const wg_spx * pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tintmap_p	tintmapArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintmapArea);

	auto p = SoftEdgemap::create(bp, (SampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj wg_createSoftEdgemapFactory(void)
{
	auto p = SoftEdgemapFactory::create();
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

