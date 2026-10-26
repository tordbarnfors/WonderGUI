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
#include <wax_c_softgfx.h>
#include <wax_c_internal.h>

#include <wax_softbackend.h>
#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_softedgemap.h>
#include <wax_softedgemapfactory.h>
#include <wax_softwarpmapfactory.h>

using namespace wax;

wax_obj wax_createSoftBackend(void)
{
	auto p = SoftBackend::create();
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

int wax_defineSoftBackendCanvas( wax_obj backend, wax_canvasRef ref, wax_obj softSurface )
{
	auto pBackend = static_cast<SoftBackend*>(reinterpret_cast<Object*>(backend));
	auto pSurface = static_cast<SoftSurface*>(reinterpret_cast<Object*>(softSurface));

	return pBackend->defineCanvas((CanvasRef) ref, pSurface);
}

wax_obj wax_createSoftSurface(const wax_surfaceBP* blueprint)
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);

	auto p = SoftSurface::create(bp);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}


wax_obj wax_createSoftSurfaceFromBlob(const wax_surfaceBP* blueprint, wax_obj blob, int pitch)
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto p = SoftSurface::create(bp, static_cast<Blob*>(reinterpret_cast<Object*>(blob)), pitch );
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj	wax_createSoftSurfaceFromBitmap(const wax_surfaceBP* blueprint, const uint8_t* pPixels,
									   wax_pixelFormat pixelFormat, int pitch, const wax_color8 * pPalette, int paletteSize )
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto p = SoftSurface::create(bp, pPixels, (PixelFormat) pixelFormat, pitch, (const Color8*) pPalette, paletteSize);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj	wax_createSoftSurfaceFromRawData(const wax_surfaceBP* blueprint, const uint8_t* pPixels,
										const wax_pixelDescription * pPixelDescription, int pitch, const wax_color8 * pPalette, int paletteSize )
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	
	auto p = SoftSurface::create(bp, pPixels, *(PixelDescription*) pPixelDescription, pitch, (const Color8*) pPalette, paletteSize);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}


wax_obj wax_createSoftSurfaceFactory()
{
	auto p = SoftSurfaceFactory::create();
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}


wax_obj wax_createSoftEdgemap( const wax_edgemapBP* blueprint )
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tint_p	tintArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintArea);

	auto p = SoftEdgemap::create(bp);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createSoftEdgemapFromFloats( const wax_edgemapBP* blueprint, wax_sampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tint_p	tintArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintArea);

	auto p = SoftEdgemap::create(bp, (SampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createSoftEdgemapFromSpx( const wax_edgemapBP* blueprint, wax_sampleOrigo origo, const wax_spx * pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tint_p	tintArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintArea);

	auto p = SoftEdgemap::create(bp, (SampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createSoftEdgemapFactory(void)
{
	auto p = SoftEdgemapFactory::create();
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createSoftWarpmapFactory(void)
{
	auto p = SoftWarpmapFactory::create();
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

