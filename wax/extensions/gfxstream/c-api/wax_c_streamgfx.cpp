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
#include <wax_remotebitmap.h>
#include <wax_remotebitmapfactory.h>
#include <wax_mirrorbitmap.h>
#include <wax_mirrorbitmapfactory.h>

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

int wax_defineStreamBackendCanvasWithBitmap( wax_obj streamBackend, wax_canvasRef ref, wax_obj bitmap )
{
	return getPtr(streamBackend)->defineCanvas( (CanvasRef) ref, static_cast<RemoteBitmap*>(reinterpret_cast<Object*>(bitmap)) );
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

wax_obj wax_createRemoteBitmap(wax_obj streamEncoder, const wax_bitmapBP* blueprint)
{
	Bitmap::Blueprint	bp;
	convertBitmapBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = RemoteBitmap::create(pEncoder, bp);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}


wax_obj wax_createRemoteBitmapFromBlob(wax_obj streamEncoder, const wax_bitmapBP* blueprint, wax_obj blob, int pitch)
{
	Bitmap::Blueprint	bp;
	convertBitmapBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = RemoteBitmap::create(pEncoder, bp, static_cast<Blob*>(reinterpret_cast<Object*>(blob)), pitch );
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj	wax_createRemoteBitmapFromBitmap(wax_obj streamEncoder, wax_obj factory, const wax_bitmapBP* blueprint, const uint8_t* pPixels,
									   wax_pixelFormat pixelFormat, int pitch, const wax_color8 * pPalette )
{
	Bitmap::Blueprint	bp;
	convertBitmapBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = RemoteBitmap::create(pEncoder, bp, pPixels, (PixelFormat) pixelFormat, pitch, (const Color8*) pPalette);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj	wax_createRemoteBitmapFromRawData(wax_obj streamEncoder, wax_obj factory, const wax_bitmapBP* blueprint, const uint8_t* pPixels,
										const wax_pixelDescription * pPixelDescription, int pitch, const wax_color8 * pPalette )
{
	Bitmap::Blueprint	bp;
	convertBitmapBlueprint(&bp, blueprint);
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));

	auto p = RemoteBitmap::create(pEncoder, bp, pPixels, *(PixelDescription*) pPixelDescription, pitch, (const Color8*) pPalette);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}


wax_obj wax_createRemoteBitmapFactory(wax_obj streamEncoder)
{
	auto p = RemoteBitmapFactory::create(static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder)));
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createMirrorBitmap(wax_obj streamEncoder, wax_mirrorBitmapBP* blueprint)
{
	MirrorBitmap::Blueprint	bp;
	bp.canvasRef = (CanvasRef) blueprint->canvasRef;
	bp.encoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(blueprint->encoder));
	bp.bitmap = static_cast<Bitmap*>(reinterpret_cast<Object*>(blueprint->bitmap));
	bp.streamOnCreate = blueprint->streamOnCreate != 0;
	auto p = MirrorBitmap::create(bp);
	if( !p )
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createMirrorBitmapFactory(wax_obj streamEncoder, wax_obj backingBitmapFactory)
{
	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto pBackingFactory = static_cast<BitmapFactory*>(reinterpret_cast<Object*>(backingBitmapFactory));
	auto p = MirrorBitmapFactory::create(pEncoder, pBackingFactory);
	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

void wax_setRememberRemoteBitmapsCreated(wax_obj remoteBitmapFactory, int bRemember)
{
	static_cast<RemoteBitmapFactory*>(reinterpret_cast<Object*>(remoteBitmapFactory))->setRememberBitmapsCreated(bRemember == 1);
}

void wax_recreateRemoteBitmaps(wax_obj remoteBitmapFactory)
{
	static_cast<RemoteBitmapFactory*>(reinterpret_cast<Object*>(remoteBitmapFactory))->recreateRemoteBitmaps();
}

void wax_setRememberMirrorBitmapsCreated(wax_obj mirrorBitmapFactory, int bRemember)
{
	static_cast<MirrorBitmapFactory*>(reinterpret_cast<Object*>(mirrorBitmapFactory))->setRememberBitmapsCreated(bRemember == 1);
}

void wax_streamMirrorBitmapsAsNew(wax_obj mirrorBitmapFactory)
{
	static_cast<MirrorBitmapFactory*>(reinterpret_cast<Object*>(mirrorBitmapFactory))->streamAllAsNew();
}

wax_obj wax_createStreamEdgemap(wax_obj streamEncoder, const wax_edgemapBP* blueprint )
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tint_p	tintArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintArea);

	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemap::create(pEncoder, bp);
	if( !p )
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createStreamEdgemapFromFloats(wax_obj streamEncoder, const wax_edgemapBP* blueprint, wax_sampleOrigin origin, const float * pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tint_p	tintArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintArea);

	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemap::create(pEncoder, bp, (SampleOrigin) origin, pSamples, edges, edgePitch, samplePitch);
	if( !p )
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createStreamEdgemapFromSpx(wax_obj streamEncoder, const wax_edgemapBP* blueprint, wax_sampleOrigin origin, const wax_spx * pSamples, int edges, int edgePitch, int samplePitch)
{
	Edgemap::Blueprint	bp;

	assert(blueprint->segments <= 32);
	Tint_p	tintArea[32];

	convertEdgemapBlueprint(&bp, blueprint, tintArea);

	auto pEncoder = static_cast<StreamEncoder*>(reinterpret_cast<Object*>(streamEncoder));
	auto p = StreamEdgemap::create(pEncoder, bp, (SampleOrigin) origin, pSamples, edges, edgePitch, samplePitch);
	if( !p )
		return nullptr;

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
