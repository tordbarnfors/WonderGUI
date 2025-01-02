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


#include <wg_c_streampump.h>
#include <wg_streampump.h>

using namespace wg;

inline StreamPump* getPtr(wg_obj obj) {
	return static_cast<StreamPump*>(reinterpret_cast<Object*>(obj));
}


wg_obj wg_createStreamPump()
{
	auto p = StreamPump::create();
	p->retain();
	return p.rawPtr();
}

wg_obj wg_createStreamPumpWithInputOutput(wg_component input, wg_component output)
{
	StreamSource_p pInput(reinterpret_cast<Object*>(input.object), * reinterpret_cast<StreamSource*>(input.pInterface));
	StreamSink_p pOutput(reinterpret_cast<Object*>(output.object), *reinterpret_cast<StreamSink*>(output.pInterface));

	auto p = StreamPump::create(pInput,pOutput);
	p->retain();
	return p.rawPtr();
}

void wg_setStreamPumpInput(wg_obj streamPump, wg_component input)
{
	StreamSource_p pInput(reinterpret_cast<Object*>(input.object), *reinterpret_cast<StreamSource*>(input.pInterface));
	getPtr(streamPump)->setInput(pInput);
}

void wg_setStreamPumpOutput(wg_obj streamPump, wg_component output)
{
	StreamSink_p pOutput(reinterpret_cast<Object*>(output.object), *reinterpret_cast<StreamSink*>(output.pInterface));
	getPtr(streamPump)->setOutput(pOutput);
}

wg_streamChunkId wg_peekChunk(wg_obj streamPump)
{
	return (wg_streamChunkId) getPtr(streamPump)->peekChunk();
}

int wg_setSessionMasks(wg_obj streamPump, wg_obj streamTrimBackend)
{
	return getPtr(streamPump)->setSessionMasks(static_cast<StreamTrimBackend*>(reinterpret_cast<Object*>(streamTrimBackend)));
}

int wg_pumpChunk(wg_obj streamPump)
{
	return getPtr(streamPump)->pumpChunk();
}

int wg_pumpUntilFrame(wg_obj streamPump)
{
	return getPtr(streamPump)->pumpUntilFrame();
}

int wg_pumpFrame(wg_obj streamPump)
{
	return getPtr(streamPump)->pumpFrame();
}

int wg_pumpAll(wg_obj streamPump)
{
	return getPtr(streamPump)->pumpAll();
}

int wg_pumpBytes( wg_obj streamPump, int maxBytes )
{
	return getPtr(streamPump)->pumpBytes(maxBytes);
}
