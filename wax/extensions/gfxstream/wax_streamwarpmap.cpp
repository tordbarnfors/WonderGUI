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
#include <wax_streamwarpmap.h>
#include <wax_streambackend.h>
#include <wax_core.h>

#include <algorithm>
#include <cstring>

namespace wax
{
	const TypeInfo StreamWarpmap::TYPEINFO = { "StreamWarpmap", &Warpmap::TYPEINFO };

	//____ create() ___________________________________________________________

	StreamWarpmap_p StreamWarpmap::create( StreamEncoder * pEncoder, const Blueprint& bp )
	{
		if (!pEncoder || !_validateBlueprint(bp))
			return nullptr;

		auto p = StreamWarpmap_p(new StreamWarpmap(pEncoder, bp));
		pEncoder->flush();
		return p;
	}

	StreamWarpmap_p StreamWarpmap::create( StreamEncoder * pEncoder, const Blueprint& bp, const CoordSPX * pNodes, int pitch )
	{
		if (!pEncoder || !_validateBlueprint(bp) || !pNodes)
			return nullptr;

		auto p = StreamWarpmap_p(new StreamWarpmap(pEncoder, bp, pNodes, pitch));
		pEncoder->flush();
		return p;
	}

	//____ constructor ________________________________________________________
	/*
		The receiving end creates its Warpmap with default nodes. We send ours even when they are
		the defaults, so that it gets exactly ours.
	*/

	StreamWarpmap::StreamWarpmap( StreamEncoder * pEncoder, const Blueprint& bp, const CoordSPX * pNodes, int pitch ) : Warpmap(bp, pNodes, pitch)
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_pCompressor = NodeCompressor::create();

		auto& encoder = * m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::CreateWarpmap, 0, 14 };
		encoder << m_inStreamId;
		encoder << m_size;
		encoder << (uint16_t) m_cellSize.w;
		encoder << (uint16_t) m_cellSize.h;

		_nodesUpdated({ 0, 0, m_nodes.w, m_nodes.h });
	}

	//____ destructor _________________________________________________________

	StreamWarpmap::~StreamWarpmap()
	{
		auto& encoder = * m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::DeleteWarpmap, 0, 2 };
		encoder << m_inStreamId;

		m_pEncoder->freeObjectId(m_inStreamId);
		m_pEncoder->flush();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamWarpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _nodesUpdated() ____________________________________________________
	/*
		Sends the region and periods in a WarpmapUpdate, followed by WarpmapNodes chunks with the
		nodes compressed. Rows narrower than the Warpmap are gathered first. Periods only change
		together with all nodes, so the receiver applies them together.
	*/

	void StreamWarpmap::_nodesUpdated( const RectI& region )
	{
		if (region.w <= 0 || region.h <= 0)
			return;

		auto& encoder = * m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::WarpmapUpdate, 0, 18 };
		encoder << m_inStreamId;
		encoder << (uint16_t) region.x;
		encoder << (uint16_t) region.y;
		encoder << (uint16_t) region.w;
		encoder << (uint16_t) region.h;
		encoder << (int32_t) m_uPeriod;
		encoder << (int32_t) m_vPeriod;

		const CoordSPX* pBeg = m_pNodes + region.y * m_nodes.w + region.x;
		int nNodes = region.w * region.h;
		int bytes = 0;

		if (region.w != m_nodes.w)
		{
			bytes = nNodes * sizeof(CoordSPX);
			auto pGathered = (CoordSPX*) Core::memStackAlloc(bytes);

			for (int row = 0; row < region.h; row++)
				std::memcpy(pGathered + row * region.w, pBeg + row * m_nodes.w, region.w * sizeof(CoordSPX));

			pBeg = pGathered;
		}

		m_pCompressor->setRowLength(region.w);
		StreamBackend::_compressSplitAndEncode(m_pEncoder, GfxStream::ChunkId::WarpmapNodes, m_inStreamId, m_pCompressor, pBeg, pBeg + nNodes);

		if (bytes > 0)
			Core::memStackFree(bytes);
	}

} // namespace wax
