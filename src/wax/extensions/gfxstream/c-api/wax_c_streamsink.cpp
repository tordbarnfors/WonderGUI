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
#include <wg_c_streamsink.h>
#include <wg_streamsink.h>

using namespace wg;

inline StreamSink* getPtr(wg_component sink) {
	return static_cast<StreamSink*>(reinterpret_cast<StreamSink*>(sink.pInterface));
}

void wg_processStreamChunks( wg_component streamSink, const void * pBegin, const void * pEnd )
{
    auto p = getPtr(streamSink);
    p->processChunks(pBegin, pEnd);
}
