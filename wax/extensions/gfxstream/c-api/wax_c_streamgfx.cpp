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
#include <wax_c_streamgfx.h>
#include <wax_c_internal.h>

#include <wax_streambackend.h>
#include <wax_streamwarpmapfactory.h>
#include <wax_remotesurface.h>
#include <wax_remotesurfacefactory.h>
#include <wax_mirrorsurface.h>
#include <wax_mirrorsurfacefactory.h>

#include <wax_streamedgemap.h>
#include <wax_streamedgemapfactory.h>


using namespace wax;

inline StreamBackend* getPtr(wax_obj obj) {
	return static_cast<StreamBackend*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createStreamBackend(wax_obj streamEncoder, int maxEdges )
{
	auto pObj = StreamBackend::create( static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder)), maxEdges);
	pObj->retain();
	return static_cast<Object*>(pObj.rawPtr());
}

int wax_defineStreamBackendCanvasWithSurface( wax_obj streamBackend, wax_canvasRef ref, wax_obj surface )
{
	return getPtr(streamBackend)->defineCanvas( (CanvasRef) ref, static_cast<RemoteSurface*>(reinterpret_cast<Object*>(surface)) );
}

int wax_defineStreamBackendCanvas( wax_obj streamBackend, wax_canvasRef ref, wax_sizeI pixelSize, wax_pixelFormat pixelFormat, int scale,
								  wax_colorSpace colorSpace, wax_byteOrder byteOrder )
{
	bool bBigEndian = byteOrder == WAX_BYTEORDER_NATIVE ? (WAX_IS_BIG_ENDIAN == 1) : (byteOrder == WAX_BYTEORDER_BIG_ENDIAN);

	return getPtr(streamBackend)->defineCanvas( (CanvasRef) ref, {pixelSize.w, pixelSize.h}, (PixelFormat) pixelFormat, scale,
												(ColorSpace) colorSpace, bBigEndian );
}

void wax_encodeCanvasList(wax_obj streamBackend)
{
	getPtr(streamBackend)->encodeCanvasList();
}

void wax_encodeTick(wax_obj streamBackend, int32_t microsecPassed)
{
	getPtr(streamBackend)->encodeTick(microsecPassed);
}

wax_obj wax_createRemoteSurface(wax_obj streamEncoder, const wax_surfaceBP* blueprint)
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = RemoteSurface::create(pEncoder, bp);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}


wax_obj wax_createRemoteSurfaceFromBlob(wax_obj streamEncoder, const wax_surfaceBP* blueprint, wax_obj blob, int pitch)
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = RemoteSurface::create(pEncoder, bp, static_cast<Blob*>(reinterpret_cast<Object*>(blob)), pitch );
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj	wax_createRemoteSurfaceFromBitmap(wax_obj streamEncoder, wax_obj factory, const wax_surfaceBP* blueprint, const uint8_t* pPixels,
									   wax_pixelFormat pixelFormat, int pitch, const wax_color8 * pPalette )
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = RemoteSurface::create(pEncoder, bp, pPixels, (PixelFormat) pixelFormat, pitch, (const Color8*) pPalette);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj	wax_createRemoteSurfaceFromRawData(wax_obj streamEncoder, wax_obj factory, const wax_surfaceBP* blueprint, const uint8_t* pPixels,
										const wax_pixelDescription * pPixelDescription, int pitch, const wax_color8 * pPalette )
{
	Surface::Blueprint	bp;
	convertSurfaceBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = RemoteSurface::create(pEncoder, bp, pPixels, *(PixelDescription*) pPixelDescription, pitch, (const Color8*) pPalette);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}


wax_obj wax_createRemoteSurfaceFactory(wax_obj streamEncoder)
{
	auto p = RemoteSurfaceFactory::create(static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder)));
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createMirrorSurface(wax_obj streamEncoder, wax_mirrorSurfaceBP* blueprint)
{
	MirrorSurface::Blueprint	bp;
	bp.canvasRef = (CanvasRef) blueprint->canvasRef;
	bp.encoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(blueprint->encoder));
	bp.surface = static_cast<Surface*>(reinterpret_cast<Object*>(blueprint->surface));
	bp.streamOnCreate = blueprint->streamOnCreate != 0;
	auto p = MirrorSurface::create(bp);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createMirrorSurfaceFactory(wax_obj streamEncoder, wax_obj backingSurfaceFactory)
{
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto pBackingFactory = static_cast<SurfaceFactory*>(reinterpret_cast<Object*>(backingSurfaceFactory));
	auto p = MirrorSurfaceFactory::create(pEncoder, pBackingFactory);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

void wax_setRememberRemoteSurfacesCreated(wax_obj remoteSurfaceFactory, int bRemember)
{
	static_cast<RemoteSurfaceFactory*>(reinterpret_cast<Object*>(remoteSurfaceFactory))->setRememberSurfacesCreated(bRemember == 1);
}

void wax_recreateRemoteSurfaces(wax_obj remoteSurfaceFactory)
{
	static_cast<RemoteSurfaceFactory*>(reinterpret_cast<Object*>(remoteSurfaceFactory))->recreateRemoteSurfaces();
}

void wax_setRememberMirrorSurfacesCreated(wax_obj mirrorSurfaceFactory, int bRemember)
{
	static_cast<MirrorSurfaceFactory*>(reinterpret_cast<Object*>(mirrorSurfaceFactory))->setRememberSurfacesCreated(bRemember == 1);
}

void wax_streamMirrorSurfacesAsNew(wax_obj mirrorSurfaceFactory)
{
	static_cast<MirrorSurfaceFactory*>(reinterpret_cast<Object*>(mirrorSurfaceFactory))->streamAllAsNew();
}

wax_obj wax_createStreamEdgemap(wax_obj streamEncoder, const wax_edgemapBP* blueprint )
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tint_p	tintArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintArea);

	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemap::create(pEncoder, bp);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createStreamEdgemapFromFloats(wax_obj streamEncoder, const wax_edgemapBP* blueprint, wax_sampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tint_p	tintArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintArea);

	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemap::create(pEncoder, bp, (SampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createStreamEdgemapFromSpx(wax_obj streamEncoder, const wax_edgemapBP* blueprint, wax_sampleOrigo origo, const wax_spx * pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tint_p	tintArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintArea);

	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemap::create(pEncoder, bp, (SampleOrigo) origo, pSamples, edges, edgePitch, samplePitch);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createStreamEdgemapFactory(wax_obj streamEncoder)
{
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemapFactory::create(pEncoder);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createStreamWarpmapFactory(wax_obj streamEncoder)
{
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamWarpmapFactory::create(pEncoder);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}
