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
#include <wg_streampump.h>
#include <wg_surface.h>
#include <wg_patches.h>
#include <wg_gfxbase.h>
#include <wg_compress.h>


#include <vector>
#include <cstdint>
//#include <cstring>

using namespace std;

namespace wg
{
	const TypeInfo StreamPump::TYPEINFO = { "StreamPump", &Object::TYPEINFO };



	//____ create() _______________________________________________________________

	StreamPump_p StreamPump::create()
	{
		StreamPump_p p(new StreamPump( StreamSource_p(), StreamSink_p()));
		return p;
	}

	StreamPump_p	StreamPump::create( const StreamSource_p& pInput, const StreamSink_p& pOutput)
	{
		StreamPump_p p(new StreamPump(pInput, pOutput));
		return p;
	}


	//____ constructor _____________________________________________________________

	StreamPump::StreamPump(const StreamSource_p& pInput, const StreamSink_p& pOutput)
	{
		m_pInput = pInput;
		m_pOutput = pOutput;
	}

	//____ Destructor ______________________________________________________________

	StreamPump::~StreamPump()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamPump::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setInput() _________________________________________________________

	void StreamPump::setInput(const StreamSource_p& pStream)
	{
		m_pInput = pStream;
	}

	//____ setOutput() ________________________________________________________

	void StreamPump::setOutput(const StreamSink_p& pStream)
	{
		m_pOutput = pStream;
	}

	//____ peekChunk() ________________________________________________________

	GfxStream::ChunkId StreamPump::peekChunk()
	{
		if (!m_pInput)
			return GfxStream::ChunkId::OutOfData;

		return m_pInput->peekChunk();
	}

	//____ pumpChunk() ________________________________________________________

	bool StreamPump::pumpChunk()
	{
		if (!m_pInput || !m_pOutput)
			return false;

		int	nSegments;
		const GfxStream::Data* pSegments;

		if (!m_pInput->hasChunks())
			m_pInput->fetchChunks();

		if (m_pInput->hasChunks())
		{
			std::tie(nSegments, pSegments) = m_pInput->showChunks();
			int size = GfxStream::chunkSize( pSegments[0].pBegin );

			m_pOutput->processChunks( pSegments[0].pBegin, pSegments[0].pBegin + size );
			m_pInput->discardChunks(size);

			return true;
		}

		return false;
	}

	//____ pumpUntilFrame() ________________________________________________________

	bool StreamPump::pumpUntilFrame()
	{
		if (!m_pInput || !m_pOutput)
			return false;

		return _pumpUntilChunk(GfxStream::ChunkId::BeginRender, false);
	}

	//____ pumpFrame() ________________________________________________________
	// Simple and reliable version

/*
	bool StreamPump::pumpFrame()
	{
		if (!m_pInput || !m_pOutput)
			return false;

		if ( _fetchUntilChunk(GfxStream::ChunkId::EndRender) )
			return _pumpUntilChunk(GfxStream::ChunkId::EndRender, true);

		return false;
	}
*/
	//____ pumpFrame() ________________________________________________________

	// New, faster version.

	bool StreamPump::pumpFrame()
	{
		if (!m_pInput || !m_pOutput)
			return false;

		int	nSegments;
		const GfxStream::Data* pSegments;

		if (!m_pInput->hasChunks())
			m_pInput->fetchChunks();

		while (m_pInput->hasChunks())
		{
			std::tie(nSegments, pSegments) = m_pInput->showChunks();

			int bytesProcessed = 0;
			for (int i = 0; i < nSegments; i++)
			{
				auto p = _findChunk(GfxStream::ChunkId::EndRender, pSegments[i].pBegin, pSegments[i].pEnd);
				if ( p != pSegments[i].pEnd)
				{
					// EndRender found. Process all chunks in previous segments

					for( int j = 0 ; j < i ; j++ )
					{
						const uint8_t* pBegin = pSegments[j].pBegin;
						const uint8_t* pEnd = pSegments[j].pEnd;

						m_pOutput->processChunks(pBegin, pEnd);
						bytesProcessed += pEnd - pBegin;
					}

					// Process the chunks of this segment up to and including our EndRender

					const uint8_t* pBegin = pSegments[i].pBegin;
					const uint8_t* pEnd = p + GfxStream::chunkSize(p);

					m_pOutput->processChunks(pBegin, pEnd);
					bytesProcessed += pEnd - pBegin;

					m_pInput->discardChunks(bytesProcessed);
					return true;
				}
			}

			if( !m_pInput->fetchChunks() )
				break;
		}

		return false;
	}

	//____ _pumpAllFrames() ____________________________________________________

	int StreamPump::pumpAllFrames(StreamTrimBackend* pTrimmer)
	{
		if (!m_pInput || !m_pOutput)
			return -1;

		// Start by fetching all chunks available

		while(m_pInput->fetchChunks());

		if (!m_pInput->hasChunks())
			return 0;

		// Show all chunks

		int	nSegments;
		const GfxStream::Data* pSegments;
		std::tie(nSegments, pSegments) = m_pInput->showChunks();

		// Find last complete frame

		int nCompleteFrames = 0;
		const uint8_t* pLastCompleteFrameEnd = nullptr;
		for (int i = 0; i < nSegments; i++)
		{
			auto p = _findChunk(GfxStream::ChunkId::EndRender, pSegments[i].pBegin, pSegments[i].pEnd);
			while (p != pSegments[i].pEnd)
			{
				nCompleteFrames++;
				pLastCompleteFrameEnd = p + GfxStream::chunkSize(p);
				p = _findChunk(GfxStream::ChunkId::EndRender, pLastCompleteFrameEnd, pSegments[i].pEnd);
			}
		}

		// Early out if no complete frames found

		if (nCompleteFrames == 0)
			return 0;		// No complete frames found

		// Trim frames if we have a trimmer

		if (pTrimmer && nCompleteFrames > 1)
			_trimFrames(pTrimmer, nSegments, pSegments, pLastCompleteFrameEnd);

		// Pump all frames up to and including the last complete frame

		int bytesProcessed = 0;
		for (int i = 0; i < nSegments; i++)
		{
			auto pBegin = pSegments[i].pBegin;
			auto pEnd = pSegments[i].pEnd;

			if (pLastCompleteFrameEnd >= pBegin && pLastCompleteFrameEnd <= pEnd)
			{
				m_pOutput->processChunks(pBegin, pLastCompleteFrameEnd);
				bytesProcessed += pLastCompleteFrameEnd - pBegin;
				break;
			}

			m_pOutput->processChunks(pBegin, pEnd);
			bytesProcessed += pEnd - pBegin;
		}

		m_pInput->discardChunks(bytesProcessed);
		return nCompleteFrames;
	}

	//____ _pumpUntilChunk() ___________________________________________________

	bool StreamPump::_pumpUntilChunk(GfxStream::ChunkId id, bool bInclusive)
	{
		int	nSegments;
		const GfxStream::Data* pSegments;

		if (!m_pInput->hasChunks())
			m_pInput->fetchChunks();

		while (m_pInput->hasChunks())
		{
			std::tie(nSegments, pSegments) = m_pInput->showChunks();

			int	bytesProcessed = 0;
			for (int i = 0; i < nSegments; i++)
			{
				const uint8_t* pBegin = pSegments[i].pBegin;
				const uint8_t* pEnd = _findChunk(id, pSegments[i].pBegin, pSegments[i].pEnd);

				m_pOutput->processChunks(pBegin, pEnd);
				bytesProcessed += pEnd - pBegin;

				if (pEnd != pSegments[i].pEnd)
				{
					if (bInclusive)
					{
						bytesProcessed += GfxStream::chunkSize(pEnd);
						m_pOutput->processChunks(pEnd, pEnd + GfxStream::chunkSize(pEnd) );
					}

					m_pInput->discardChunks(bytesProcessed);
					return true;
				}
			}

			m_pInput->discardChunks(bytesProcessed);
			m_pInput->fetchChunks();
		}

		return false;
	}

	//____ _fetchUntilChunk() _________________________________________________

	bool StreamPump::_fetchUntilChunk(GfxStream::ChunkId id)
	{
		int	nSegments;
		const GfxStream::Data* pSegments;

		if (!m_pInput->hasChunks())
			m_pInput->fetchChunks();

		while (m_pInput->hasChunks())
		{
			std::tie(nSegments, pSegments) = m_pInput->showChunks();

			for (int i = 0; i < nSegments; i++)
			{
				if (_findChunk(id, pSegments[i].pBegin, pSegments[i].pEnd) != pSegments[i].pEnd)
					return true;
			}
			
			if( !m_pInput->fetchChunks() )
				break;
		}

		return false;
	}

	//____ _trimFrames() ________________________________________________

	void StreamPump::_trimFrames(StreamTrimBackend* pTrimBackend, int nSegments, const GfxStream::Data * pSegments, const uint8_t* pEndPoint)
	{
		pTrimBackend->clearSessionMasks();

		//

		CanvasRef		canvasRef;
		uint16_t		nUpdateRects;
		int				nSessions = 0;

		std::vector<RectSPX>	updateRects;

		for (int seg = 0; seg <= nSegments; seg++)
		{
			const uint8_t* pEnd = (pEndPoint >= pSegments[seg].pBegin && pEndPoint <= pSegments[seg].pEnd) ? pEndPoint : pSegments[seg].pEnd;

			auto p = pSegments[seg].pBegin;

			while (p != pEnd)
			{
				GfxStream::ChunkId chunkId = GfxStream::chunkType(p);

				if (chunkId == GfxStream::ChunkId::BeginSession)
				{
					const uint16_t* pChunkData = (uint16_t*)(p + GfxStream::headerSize(p));

					canvasRef = (CanvasRef)pChunkData[1];
					nUpdateRects = pChunkData[2];

					nSessions++;

					if (nSessions > 1)
					{
						if (canvasRef == CanvasRef::None)
						{
							// We can't mask writes to surfaces, they might be used as source for blits before last frame.

							pTrimBackend->addNonMaskingSession();
						}
						else
						{
							if (nUpdateRects == 0)
								pTrimBackend->addFullyMaskingSession(canvasRef, nullptr);
							else
								updateRects.resize(nUpdateRects);
						}
					}
				}

				if (chunkId == GfxStream::ChunkId::UpdateRects && canvasRef != CanvasRef::None && nSessions > 1)
				{
					GfxStream::DataInfo info = GfxStream::decodeDataInfo(p);

					uint8_t* pSrc = p + GfxStream::HeaderSize + GfxStream::DataInfoSize;
					uint8_t* pDest = ((uint8_t*)updateRects.data()) + info.chunkOffset;
					int dataSize = (GfxStream::dataSize(p) - GfxStream::DataInfoSize);

					decompressSpx(info.compression, pSrc, dataSize, pDest);

					if (info.bLastChunk)
						pTrimBackend->addMaskingSession(canvasRef, nullptr, nUpdateRects, updateRects.data());
				}

				p = p + GfxStream::chunkSize(p);
			}

			if(pEnd == pEndPoint)
				break;
		}
	}


	//____ pumpAll() __________________________________________________________

	bool StreamPump::pumpAll()
	{
		if (!m_pInput || !m_pOutput)
			return false;

		int	nSegments;
		const GfxStream::Data* pSegments;

		if (!m_pInput->hasChunks())
			m_pInput->fetchChunks();

		while (m_pInput->hasChunks())
		{
			std::tie(nSegments, pSegments) = m_pInput->showChunks();

			int	bytesProcessed = 0;
			for (int i = 0; i < nSegments; i++)
			{
				m_pOutput->processChunks(pSegments[i].pBegin, pSegments[i].pEnd);
				bytesProcessed += pSegments[i].pEnd - pSegments[i].pBegin;
			}

			m_pInput->discardChunks(bytesProcessed);

			m_pInput->fetchChunks();
		}

		return true;
	}

	//____ pumpBytes() __________________________________________________________

	int StreamPump::pumpBytes( int maxBytes )
	{
		if (!m_pInput || !m_pOutput)
			return false;

		int bytesLeft = maxBytes;
		bool done = false;
		
		while (!done)
		{
			if (!m_pInput->hasChunks())
			{
				m_pInput->fetchChunks();
				if( !m_pInput->hasChunks())
					return maxBytes - bytesLeft;				// Out of data!
			}

			int	nSegments;
			const GfxStream::Data* pSegments;
			std::tie(nSegments, pSegments) = m_pInput->showChunks();

			int	bytesProcessed = 0;
			for (int i = 0; i < nSegments && !done; i++)
			{
				const uint8_t* pBegin = pSegments[i].pBegin;
				const uint8_t* pEnd = pSegments[i].pEnd;
				const uint8_t* p = pBegin;

				if( bytesLeft >= (pEnd - pBegin) )
					p = pEnd;
				else
				{
					int chunkSize = GfxStream::chunkSize(p);

					while( p + chunkSize - pBegin <= bytesLeft )
					{
						p += chunkSize;
						chunkSize = GfxStream::chunkSize(p);
					}

					done = true;
				}

				if( p > pBegin )
				{
					m_pOutput->processChunks(pBegin, p);
					bytesProcessed += p - pBegin;
					bytesLeft -= p - pBegin;
				}
				
			}

			m_pInput->discardChunks(bytesProcessed);
		}

		return maxBytes - bytesLeft;
	}


	//____ _findChunk() _______________________________________________________

	const uint8_t* StreamPump::_findChunk(GfxStream::ChunkId id, const uint8_t * pBegin, const uint8_t * pEnd)
	{
		const uint8_t* p = pBegin;

		while (p != pEnd)
		{
			GfxStream::ChunkId chunkId = GfxStream::chunkType(p);
			if (chunkId == id)
				break;
			p += GfxStream::chunkSize(p);
		}

		return p;
	}

} // namespace
