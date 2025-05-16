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

#include <wg_c_streamgfx.h>
#include <wg_c_internal.h>

#include <wg_streambackend.h>
#include <wg_streamsurface.h>
#include <wg_streamsurfacefactory.h>
#include <wg_streamedgemap.h>
#include <wg_streamedgemapfactory.h>


using namespace wg;

inline StreamBackend* getPtr(wg_obj obj) {
	return static_cast<StreamBackend*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createStreamBackend(wg_obj streamEncoder, int maxEdges )
{
	auto pObj = StreamBackend::create( static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder)), maxEdges);
	pObj->retain();
	return static_cast<Object*>(pObj.rawPtr());
}

int wg_defineStreamBackendCanvasWithSurface( wg_obj streamBackend, wg_canvasRef ref, wg_obj surface )
{
	return getPtr(streamBackend)->defineCanvas( (CanvasRef) ref, static_cast<StreamSurface*>(reinterpret_cast<Object*>(surface)) );
}

int wg_defineStreamBackendCanvas( wg_obj streamBackend, wg_canvasRef ref, wg_sizeI pixelSize, wg_pixelFormat pixelFormat, int scale )
{
	return getPtr(streamBackend)->defineCanvas( (CanvasRef) ref, {pixelSize.w, pixelSize.h}, (PixelFormat) pixelFormat, scale );
}


wg_obj wg_createStreamSurface(wg_obj streamEncoder, const wg_surfaceBP* blueprint)
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = StreamSurface::create(pEncoder, bp);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}


wg_obj wg_createStreamSurfaceFromBlob(wg_obj streamEncoder, const wg_surfaceBP* blueprint, wg_obj blob, int pitch)
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = StreamSurface::create(pEncoder, bp, static_cast<Blob*>(reinterpret_cast<Object*>(blob)), pitch );
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj	wg_createStreamSurfaceFromBitmap(wg_obj streamEncoder, wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels,
									   wg_pixelFormat pixelFormat, int pitch, const wg_color8 * pPalette )
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = StreamSurface::create(pEncoder, bp, pPixels, (PixelFormat) pixelFormat, pitch, (const Color8*) pPalette);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj	wg_createStreamSurfaceFromRawData(wg_obj streamEncoder, wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels,
										const wg_pixelDescription * pPixelDescription, int pitch, const wg_color8 * pPalette )
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = StreamSurface::create(pEncoder, bp, pPixels, *(PixelDescription*) pPixelDescription, pitch, (const Color8*) pPalette);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}


wg_obj wg_createStreamSurfaceFactory(wg_obj streamEncoder)
{
	auto p = StreamSurfaceFactory::create(static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder)));
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj wg_createStreamEdgemap(wg_obj streamEncoder, const wg_edgemapBP* blueprint )
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tintmap_p	tintmapArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintmapArea);

	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemap::create(pEncoder, bp);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj wg_createStreamEdgemapFromFloats(wg_obj streamEncoder, const wg_edgemapBP* blueprint, wg_sampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tintmap_p	tintmapArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintmapArea);

	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemap::create(pEncoder, bp, (SampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj wg_createStreamEdgemapFromSpx(wg_obj streamEncoder, const wg_edgemapBP* blueprint, wg_sampleOrigo origo, const wg_spx * pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tintmap_p	tintmapArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintmapArea);

	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemap::create(pEncoder, bp, (SampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj wg_createStreamEdgemapFactory(wg_obj streamEncoder)
{
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemapFactory::create(pEncoder);
	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}
