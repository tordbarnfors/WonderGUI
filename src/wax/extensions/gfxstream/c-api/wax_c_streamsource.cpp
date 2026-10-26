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
#include <wax_c_streamsource.h>
#include <wax_streamsource.h>

using namespace wax;

inline StreamSource* getPtr(wax_component sink) {
	return static_cast<StreamSource*>(reinterpret_cast<StreamSource*>(sink.pInterface));
}

int wax_hasStreamChunks( wax_component streamSource )
{
    auto p = getPtr(streamSource);
    return p->hasChunks();
}

int wax_showStreamChunks( wax_component streamSource, wax_streamData ** pSegments )
{
    auto p = getPtr(streamSource);

    int nbSegments;
	const GfxStream::Data * pSeg;

    std::tie(nbSegments, pSeg) = p->showChunks();
	* pSegments = (wax_streamData*) pSeg;
    return nbSegments;
}

void wax_discardStreamChunks( wax_component streamSource, int bytes )
{
    auto p = getPtr(streamSource);
    p->discardChunks(bytes);
}

int wax_fetchStreamChunks( wax_component streamSource )
{
    auto p = getPtr(streamSource);
    return p->fetchChunks();
}

wax_streamChunkId wax_peekStreamChunk( wax_component streamSource )
{
    auto p = getPtr(streamSource);
    return (wax_streamChunkId) p->peekChunk();
}

