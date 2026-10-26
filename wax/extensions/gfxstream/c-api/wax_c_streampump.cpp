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
#include <wax_c_streampump.h>
#include <wax_streampump.h>

using namespace wax;

inline StreamPump* getPtr(wax_obj obj) {
	return static_cast<StreamPump*>(reinterpret_cast<Object*>(obj));
}


wax_obj wax_createStreamPump()
{
	auto p = StreamPump::create();
	p->retain();
	return p.rawPtr();
}

wax_obj wax_createStreamPumpWithInputOutput(wax_component input, wax_component output)
{
	StreamSource_p pInput(reinterpret_cast<Object*>(input.object), * reinterpret_cast<StreamSource*>(input.pInterface));
	StreamSink_p pOutput(reinterpret_cast<Object*>(output.object), *reinterpret_cast<StreamSink*>(output.pInterface));

	auto p = StreamPump::create(pInput,pOutput);
	p->retain();
	return p.rawPtr();
}

void wax_setStreamPumpInput(wax_obj streamPump, wax_component input)
{
	StreamSource_p pInput(reinterpret_cast<Object*>(input.object), *reinterpret_cast<StreamSource*>(input.pInterface));
	getPtr(streamPump)->setInput(pInput);
}

void wax_setStreamPumpOutput(wax_obj streamPump, wax_component output)
{
	StreamSink_p pOutput(reinterpret_cast<Object*>(output.object), *reinterpret_cast<StreamSink*>(output.pInterface));
	getPtr(streamPump)->setOutput(pOutput);
}

void wax_setFlowControl(wax_obj streamPump, uint16_t fenceId, int credits, int bytesPerCredit )
{
	getPtr(streamPump)->setFlowControl( fenceId, credits, bytesPerCredit );
}

void wax_restartFlowContol(wax_obj streamPump, int credits)
{
	getPtr(streamPump)->restartFlowControl(credits);
}

void wax_addFlowControlCredits(wax_obj streamPump, int credits)
{
	getPtr(streamPump)->addCredits(credits);
}

wax_streamChunkId wax_peekChunk(wax_obj streamPump)
{
	return (wax_streamChunkId) getPtr(streamPump)->peekChunk();
}

int wax_pumpChunk(wax_obj streamPump)
{
	return getPtr(streamPump)->pumpChunk();
}

int wax_pumpUntilFrame(wax_obj streamPump)
{
	return getPtr(streamPump)->pumpUntilFrame();
}

int wax_pumpFrame(wax_obj streamPump)
{
	return getPtr(streamPump)->pumpFrame();
}

int wax_pumpAllFrames(wax_obj streamPump)
{
	return getPtr(streamPump)->pumpAllFrames();
}

int wax_pumpAllFramesWithTrimming(wax_obj streamPump, wax_obj trimBackend)
{
	return getPtr(streamPump)->pumpAllFrames(static_cast<StreamTrimBackend*>(reinterpret_cast<Object*>(trimBackend)));
}

int wax_pumpAll(wax_obj streamPump)
{
	return getPtr(streamPump)->pumpAll();
}

int wax_pumpBytes( wax_obj streamPump, int maxBytes )
{
	return getPtr(streamPump)->pumpBytes(maxBytes);
}
