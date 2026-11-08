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
		m_pCompressor = MeshCompressor::create();

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
		WarpmapTriangles chunks with the content compressed. Vertices are sent as sequences of all
		x, then all y, u and v, and w if any of them has a weight other than weightOne, so that
		MeshCompressor predicts each from earlier values of its own. Triangles likewise as all a,
		then all b and c.
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
			const int sequences = bWeights ? 5 : 4;
			int bytes = nVertices * sequences * sizeof(int32_t);
			auto pValues = (int32_t*) Base::memStackAlloc(bytes);

			for (int i = 0; i < nVertices; i++)
			{
				const Vertex& v = m_pVertices[firstVertex + i];
				pValues[i] = v.x;
				pValues[nVertices + i] = v.y;
				pValues[nVertices * 2 + i] = v.u;
				pValues[nVertices * 3 + i] = v.v;
				if (bWeights)
					pValues[nVertices * 4 + i] = v.w;
			}

			m_pCompressor->setSequences(sequences);
			StreamBackend::_compressSplitAndEncode(m_pEncoder, GfxStream::ChunkId::WarpmapVertices, m_inStreamId, m_pCompressor, pValues, pValues + nVertices * sequences);

			Base::memStackFree(bytes);
		}

		if (nTriangles > 0)
		{
			int bytes = nTriangles * 3 * sizeof(int32_t);
			auto pValues = (int32_t*) Base::memStackAlloc(bytes);

			for (int i = 0; i < nTriangles; i++)
			{
				const Triangle& t = m_pTriangles[firstTriangle + i];
				pValues[i] = t.a;
				pValues[nTriangles + i] = t.b;
				pValues[nTriangles * 2 + i] = t.c;
			}

			m_pCompressor->setSequences(3);
			StreamBackend::_compressSplitAndEncode(m_pEncoder, GfxStream::ChunkId::WarpmapTriangles, m_inStreamId, m_pCompressor, pValues, pValues + nTriangles * 3);

			Base::memStackFree(bytes);
		}
	}

} // namespace wax
