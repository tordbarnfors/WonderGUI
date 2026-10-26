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
#include <wax_c_lineargfx.h>
#include <wax_c_internal.h>
#include <wax_linearbackend.h>

using namespace wax;


wax_obj wax_createLinearBackend(void*(*beginCanvasRenderFunc)(wax_canvasRef ref, int nBytes),
								 void(*endCanvasRenderFunc)(wax_canvasRef ref, int nSegments, const wax_linearGfxSegment * pSegments) )
{
	auto pBackend = LinearBackend::create([beginCanvasRenderFunc](CanvasRef ref, int nBytes)
							{
								return beginCanvasRenderFunc((wax_canvasRef)ref, nBytes);
							},

							[endCanvasRenderFunc](CanvasRef ref, int nSegments, const LinearBackend::Segment * pSegments)
							{
								return endCanvasRenderFunc((wax_canvasRef)ref, nSegments, (wax_linearGfxSegment*) pSegments);
							} );

	pBackend->retain();
	return (wax_obj) static_cast<Object*>(pBackend.rawPtr());
}


int wax_defineLinearBackendCanvas( wax_obj backend, wax_canvasRef ref, wax_sizeSPX size, wax_pixelFormat format, int scale,
								  wax_colorSpace colorSpace, wax_byteOrder byteOrder )
{
	auto pBackend = static_cast<LinearBackend*>(reinterpret_cast<Object*>(backend));

	bool bBigEndian = byteOrder == WAX_BYTEORDER_NATIVE ? (WAX_IS_BIG_ENDIAN == 1) : (byteOrder == WAX_BYTEORDER_BIG_ENDIAN);

	return pBackend->defineCanvas((CanvasRef) ref, {size.w,size.h}, (PixelFormat) format, scale, (ColorSpace) colorSpace, bBigEndian);
}


void wax_setLinearBackendSegmentPadding( wax_obj backend, int bytes )
{
	auto pBackend = static_cast<LinearBackend*>(reinterpret_cast<Object*>(backend));

	pBackend->setSegmentPadding(bytes);
}


int wax_linearBackendSegmentPadding( wax_obj backend)
{
	auto pBackend = static_cast<LinearBackend*>(reinterpret_cast<Object*>(backend));

	return pBackend->segmentPadding();
}

