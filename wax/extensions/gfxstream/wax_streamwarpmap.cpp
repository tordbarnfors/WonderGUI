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
#include <wax_base.h>

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

	StreamWarpmap_p StreamWarpmap::create( StreamEncoder * pEncoder, const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles )
	{
		if (!pEncoder || !pVertices || !pTriangles || !_validateBlueprint(bp, pTriangles))
			return nullptr;

		auto p = StreamWarpmap_p(new StreamWarpmap(pEncoder, bp, pVertices, pTriangles));
		pEncoder->flush();
		return p;
	}

	//____ constructor ________________________________________________________
	/*
		The receiving end creates its Warpmap with default content. We send ours even when it is
		the default, so that it gets exactly ours.
	*/

	StreamWarpmap::StreamWarpmap( StreamEncoder * pEncoder, const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles ) : Warpmap(bp, pVertices, pTriangles)
	{
		m_pEncoder = pEncoder;
		m_inStreamId = pEncoder->allocObjectId();
		m_pCompressor = NodeCompressor::create();

		auto& encoder = * m_pEncoder;

		encoder << GfxStream::Header{ GfxStream::ChunkId::CreateWarpmap, 0, 18 };
		encoder << m_inStreamId;
		encoder << m_size;
		encoder << (int32_t) m_nVertices;
		encoder << (int32_t) m_nTriangles;

		_updated(0, m_nVertices, 0, m_nTriangles);
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

	//____ _updated() _________________________________________________________
	/*
		Sends the ranges and periods in a WarpmapUpdate, followed by WarpmapVertices and
		WarpmapTriangles chunks with the content compressed. Each vertex is two pairs, (x, y) and
		(u, v), so that NodeCompressor predicts each value from the vertex before and the values to
		the left, and a third, (w, 0), if any of them has a weight other than weightOne. Triangles
		are packed two in three pairs, (a, b), (c, a), (b, c), without padding.
	*/

	void StreamWarpmap::_updated( int firstVertex, int nVertices, int firstTriangle, int nTriangles )
	{
		auto& encoder = * m_pEncoder;

		bool bWeights = false;
		for (int i = 0; i < nVertices; i++)
		{
			if (m_pVertices[firstVertex + i].w != weightOne)
			{
				bWeights = true;
				break;
			}
		}

		encoder << GfxStream::Header{ GfxStream::ChunkId::WarpmapUpdate, 0, 28 };
		encoder << m_inStreamId;
		encoder << (uint16_t) (bWeights ? GfxStream::WarpmapFlags::Weights : 0);
		encoder << (int32_t) firstVertex;
		encoder << (int32_t) nVertices;
		encoder << (int32_t) firstTriangle;
		encoder << (int32_t) nTriangles;
		encoder << (int32_t) m_uPeriod;
		encoder << (int32_t) m_vPeriod;

		if (nVertices > 0)
		{
			const int perVertex = bWeights ? 3 : 2;
			int bytes = nVertices * perVertex * sizeof(CoordSPX);
			auto pPairs = (CoordSPX*) Base::memStackAlloc(bytes);

			for (int i = 0; i < nVertices; i++)
			{
				const Vertex& v = m_pVertices[firstVertex + i];
				CoordSPX* pOut = pPairs + i * perVertex;
				pOut[0] = { v.x, v.y };
				pOut[1] = { v.u, v.v };
				if (bWeights)
					pOut[2] = { v.w, 0 };
			}

			m_pCompressor->setRowLength(perVertex);
			StreamBackend::_compressSplitAndEncode(m_pEncoder, GfxStream::ChunkId::WarpmapVertices, m_inStreamId, m_pCompressor, pPairs, pPairs + nVertices * perVertex);

			Base::memStackFree(bytes);
		}

		if (nTriangles > 0)
		{
			const int nRows = (nTriangles + 1) / 2;
			int bytes = nRows * 3 * sizeof(CoordSPX);
			auto pPairs = (CoordSPX*) Base::memStackAlloc(bytes);

			for (int i = 0; i < nRows; i++)
			{
				const Triangle& t0 = m_pTriangles[firstTriangle + i * 2];
				const Triangle& t1 = m_pTriangles[firstTriangle + std::min(i * 2 + 1, nTriangles - 1)];
				pPairs[i * 3] = { t0.a, t0.b };
				pPairs[i * 3 + 1] = { t0.c, t1.a };
				pPairs[i * 3 + 2] = { t1.b, t1.c };
			}

			m_pCompressor->setRowLength(3);
			StreamBackend::_compressSplitAndEncode(m_pEncoder, GfxStream::ChunkId::WarpmapTriangles, m_inStreamId, m_pCompressor, pPairs, pPairs + nRows * 3);

			Base::memStackFree(bytes);
		}
	}

} // namespace wax
