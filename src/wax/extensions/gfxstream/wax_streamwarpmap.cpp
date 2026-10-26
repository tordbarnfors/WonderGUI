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
#include <wg_streamwarpmap.h>

#include <algorithm>

namespace wg
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

	StreamWarpmap_p StreamWarpmap::create( StreamEncoder * pEncoder, const Blueprint& bp, const CoordF * pNodes, int pitch )
	{
		if (!pEncoder || !_validateBlueprint(bp) || !pNodes)
			return nullptr;

		auto p = StreamWarpmap_p(new StreamWarpmap(pEncoder, bp));
		SizeI nodes = p->nodes();
		p->importNodes({ 0,0,nodes.w,nodes.h }, pNodes, pitch);
		pEncoder->flush();
		return p;
	}

	//____ constructor ________________________________________________________
	/*
		The receiving end creates its Warpmap with the same nodes, but we send them anyway, so
		that it gets exactly ours.
	*/

	StreamWarpmap::StreamWarpmap( StreamEncoder * pEncoder, const Blueprint& bp ) : Warpmap(bp)
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();

		auto& encoder = * m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::CreateWarpmap, 0, 20 };
		encoder << m_inStreamId;
		encoder << m_size;
		encoder << (uint16_t) m_cellSize;
		encoder << m_uPeriod;
		encoder << m_vPeriod;

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

	//____ setPeriods() _______________________________________________________
	/*
		The periods are sent before the nodes that Warpmap::setPeriods() normalizes into them,
		so that the receiving end doesn't normalize those into the old periods.
	*/

	bool StreamWarpmap::setPeriods( float uPeriod, float vPeriod )
	{
		if (uPeriod < 0.f || vPeriod < 0.f)
			return false;

		auto& encoder = * m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::SetWarpmapPeriods, 0, 10 };
		encoder << m_inStreamId;
		encoder << uPeriod;
		encoder << vPeriod;

		return Warpmap::setPeriods(uPeriod, vPeriod);
	}

	//____ _nodesUpdated() ____________________________________________________
	// Sends the nodes of region, row by row, in chunks of at most c_maxNodesPerChunk nodes.

	void StreamWarpmap::_nodesUpdated( const RectI& region )
	{
		auto& encoder = * m_pEncoder;

		for (int row = region.y; row < region.y + region.h; row++)
		{
			int column = region.x;
			int end = region.x + region.w;

			while (column < end)
			{
				int n = std::min(end - column, c_maxNodesPerChunk);

				encoder << GfxStream::Header{ GfxStream::ChunkId::WarpmapNodes, 0, 8 + n * 8 };
				encoder << m_inStreamId;
				encoder << (uint16_t) column;
				encoder << (uint16_t) row;
				encoder << (uint16_t) n;

				const CoordF* p = m_pNodes + row * m_nodes.w + column;
				for (int i = 0; i < n; i++)
				{
					encoder << p[i].x;
					encoder << p[i].y;
				}

				column += n;
			}
		}
	}

} // namespace wg
