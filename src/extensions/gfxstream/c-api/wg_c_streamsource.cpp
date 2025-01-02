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


#include <wg_c_streamsource.h>
#include <wg_streamsource.h>

using namespace wg;

inline StreamSource* getPtr(wg_component sink) {
	return static_cast<StreamSource*>(reinterpret_cast<StreamSource*>(sink.pInterface));
}

int wg_hasStreamChunks( wg_component streamSource )
{
    auto p = getPtr(streamSource);
    return p->hasChunks();
}

int wg_showStreamChunks( wg_component streamSource, wg_streamData ** pSegments )
{
    auto p = getPtr(streamSource);

    int nbSegments;
	const GfxStream::Data * pSeg;

    std::tie(nbSegments, pSeg) = p->showChunks();
	* pSegments = (wg_streamData*) pSeg;
    return nbSegments;
}

void wg_discardStreamChunks( wg_component streamSource, int bytes )
{
    auto p = getPtr(streamSource);
    p->discardChunks(bytes);
}

int wg_fetchStreamChunks( wg_component streamSource )
{
    auto p = getPtr(streamSource);
    return p->fetchChunks();
}

wg_streamChunkId wg_peekStreamChunk( wg_component streamSource )
{
    auto p = getPtr(streamSource);
    return (wg_streamChunkId) p->peekChunk();
}

