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
#include <wg_c_streamreader.h>
#include <wg_streamreader.h>

using namespace wg;

inline StreamReader* getPtr(wg_obj obj) {
	return static_cast<StreamReader*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createStreamReader(wg_readStream_func callback)
{
	auto p = StreamReader::create(callback);
	p->retain();
	return p.rawPtr();
}

wg_component wg_getStreamReaderOutput(wg_obj streamReader)
{
	auto p = getPtr(streamReader);
	return { static_cast<Object*>(p), &p->output };
}

void wg_resetStreamReader(wg_obj streamReader)
{
	getPtr(streamReader)->reset();
}

int wg_streamReaderCapacity(wg_obj streamReader)
{
	return getPtr(streamReader)->bufferCapacity();
}

int wg_streamReaderHasChunk(wg_obj streamReader)
{
	return getPtr(streamReader)->chunkInBuffer();
}

int wg_streamReaderBytes(wg_obj streamReader)
{
	return getPtr(streamReader)->bytesInBuffer();
}
