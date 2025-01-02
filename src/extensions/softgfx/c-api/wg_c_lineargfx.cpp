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

#include <wg_c_lineargfx.h>
#include <wg_c_internal.h>
#include <wg_lineargfxdevice.h>
#include <wg_linearbackend.h>

using namespace wg;


wg_obj wg_createLinearGfxDevice(void*(*beginCanvasRenderFunc)(wg_canvasRef ref, int nBytes),
								 void(*endCanvasRenderFunc)(wg_canvasRef ref, int nSegments, const wg_linearGfxSegment * pSegments) )
{
	auto pDevice = LinearGfxDevice::create([beginCanvasRenderFunc](CanvasRef ref, int nBytes)
							{
								return beginCanvasRenderFunc((wg_canvasRef)ref, nBytes);
							},
							
							[endCanvasRenderFunc](CanvasRef ref, int nSegments, const LinearGfxDevice::Segment * pSegments)
							{
								return endCanvasRenderFunc((wg_canvasRef)ref, nSegments, (wg_linearGfxSegment*) pSegments);
							} );
	
	pDevice->retain();
	return (wg_obj) static_cast<Object*>(pDevice.rawPtr());
}


int wg_defineLinearGfxDeviceCanvas( wg_obj device, wg_canvasRef ref, wg_sizeSPX size, wg_pixelFormat format, int scale )
{
	auto pDevice = static_cast<LinearGfxDevice*>(reinterpret_cast<Object*>(device));

	return pDevice->defineCanvas((CanvasRef) ref, {size.w,size.h}, (PixelFormat) format, scale);
}


void wg_setLinearGfxDeviceSegmentPadding( wg_obj device, int bytes )
{
	auto pDevice = static_cast<LinearGfxDevice*>(reinterpret_cast<Object*>(device));

	pDevice->setSegmentPadding(bytes);
}


int wg_linearGfxDeviceSegmentPadding( wg_obj device)
{
	auto pDevice = static_cast<LinearGfxDevice*>(reinterpret_cast<Object*>(device));

	return pDevice->segmentPadding();
}

wg_obj wg_createLinearBackend(void*(*beginCanvasRenderFunc)(wg_canvasRef ref, int nBytes),
								 void(*endCanvasRenderFunc)(wg_canvasRef ref, int nSegments, const wg_linearGfxSegment * pSegments) )
{
	auto pBackend = LinearBackend::create([beginCanvasRenderFunc](CanvasRef ref, int nBytes)
							{
								return beginCanvasRenderFunc((wg_canvasRef)ref, nBytes);
							},

							[endCanvasRenderFunc](CanvasRef ref, int nSegments, const LinearBackend::Segment * pSegments)
							{
								return endCanvasRenderFunc((wg_canvasRef)ref, nSegments, (wg_linearGfxSegment*) pSegments);
							} );

	pBackend->retain();
	return (wg_obj) static_cast<Object*>(pBackend.rawPtr());
}


int wg_defineLinearGfxBackend( wg_obj backend, wg_canvasRef ref, wg_sizeSPX size, wg_pixelFormat format, int scale )
{
	auto pBackend = static_cast<LinearBackend*>(reinterpret_cast<Object*>(backend));

	return pBackend->defineCanvas((CanvasRef) ref, {size.w,size.h}, (PixelFormat) format, scale);
}


void wg_setLinearBackendSegmentPadding( wg_obj backend, int bytes )
{
	auto pBackend = static_cast<LinearBackend*>(reinterpret_cast<Object*>(backend));

	pBackend->setSegmentPadding(bytes);
}


int wg_linearBackendSegmentPadding( wg_obj backend)
{
	auto pBackend = static_cast<LinearBackend*>(reinterpret_cast<Object*>(backend));

	return pBackend->segmentPadding();
}

