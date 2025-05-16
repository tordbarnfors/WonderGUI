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


#include <wg_c_streamencoder.h>
#include <wg_streamencoder.h>

using namespace wg;

inline StreamEncoder* getPtr(wg_obj obj) {
	return static_cast<StreamEncoder*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createStreamEncoder( const wg_streamEncoderBP* blueprint)
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

void wg_setEncoderStream(wg_obj encoder, wg_component sinkForStream)
{
	getPtr(encoder)->setStream(StreamSink_p(reinterpret_cast<Object*>(sinkForStream.object), *reinterpret_cast<StreamSink*>(sinkForStream.pInterface)));
}

void wg_setEncoderDefaultPixelFormat(wg_obj encoder, wg_pixelFormat pixelFormat)
{
	getPtr(encoder)->setDefaultPixelFormat((PixelFormat) pixelFormat);
}

wg_pixelFormat wg_getEncoderDefaultPixelFormat(wg_obj encoder)
{
	return (wg_pixelFormat) getPtr(encoder)->defaultPixelFormat();
}

void wg_setEncoderDefaultSampleMethod(wg_obj encoder, wg_sampleMethod sampleMethod)
{
	getPtr(encoder)->setDefaultSampleMethod((SampleMethod) sampleMethod);
}

wg_sampleMethod wg_getEncoderDefaultSampleMethod(wg_obj encoder)
{
	return (wg_sampleMethod) getPtr(encoder)->defaultSampleMethod();
}
