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
#include <wax_c_streamreader.h>
#include <wax_streamreader.h>

using namespace wax;

inline StreamReader* getPtr(wax_obj obj) {
	return static_cast<StreamReader*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createStreamReader(wax_readStream_func callback)
{
	auto p = StreamReader::create(callback);
	p->retain();
	return p.rawPtr();
}

wax_component wax_getStreamReaderOutput(wax_obj streamReader)
{
	auto p = getPtr(streamReader);
	return { static_cast<Object*>(p), &p->output };
}

void wax_resetStreamReader(wax_obj streamReader)
{
	getPtr(streamReader)->reset();
}

int wax_streamReaderCapacity(wax_obj streamReader)
{
	return getPtr(streamReader)->bufferCapacity();
}

int wax_streamReaderHasChunk(wax_obj streamReader)
{
	return getPtr(streamReader)->chunkInBuffer();
}

int wax_streamReaderBytes(wax_obj streamReader)
{
	return getPtr(streamReader)->bytesInBuffer();
}
