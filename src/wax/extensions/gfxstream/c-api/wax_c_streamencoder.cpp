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
#include <wax_c_streamencoder.h>
#include <wax_streamencoder.h>

using namespace wax;

inline StreamEncoder* getPtr(wax_obj obj) {
	return static_cast<StreamEncoder*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createStreamEncoder( const wax_streamEncoderBP* blueprint)
{
	StreamEncoder::Blueprint bp;

	bp.bufferBytes = blueprint->bufferBytes;
	bp.objectIdStart = blueprint->objectIdStart;
	bp.pixelFormat = (PixelFormat) blueprint->pixelFormat;
	bp.sampleMethod = (SampleMethod) blueprint->sampleMethod;

	bp.sink = 	StreamSink_p(reinterpret_cast<Object*>(blueprint->sink.object), *reinterpret_cast<StreamSink*>(blueprint->sink.pInterface));

	auto pObj = StreamEncoder::create(bp);
	pObj->retain();
	return static_cast<Object*>(pObj.rawPtr());
}

void wax_setEncoderStream(wax_obj encoder, wax_component sinkForStream)
{
	getPtr(encoder)->setStream(StreamSink_p(reinterpret_cast<Object*>(sinkForStream.object), *reinterpret_cast<StreamSink*>(sinkForStream.pInterface)));
}

void wax_setEncoderDefaultPixelFormat(wax_obj encoder, wax_pixelFormat pixelFormat)
{
	getPtr(encoder)->setDefaultPixelFormat((PixelFormat) pixelFormat);
}

wax_pixelFormat wax_getEncoderDefaultPixelFormat(wax_obj encoder)
{
	return (wax_pixelFormat) getPtr(encoder)->defaultPixelFormat();
}

void wax_setEncoderDefaultSampleMethod(wax_obj encoder, wax_sampleMethod sampleMethod)
{
	getPtr(encoder)->setDefaultSampleMethod((SampleMethod) sampleMethod);
}

wax_sampleMethod wax_getEncoderDefaultSampleMethod(wax_obj encoder)
{
	return (wax_sampleMethod) getPtr(encoder)->defaultSampleMethod();
}

void wax_flushEncoder(wax_obj encoder)
{
	getPtr(encoder)->flush();
}
