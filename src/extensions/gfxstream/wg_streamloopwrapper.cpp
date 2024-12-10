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

#include <cstring>
#include <algorithm>

#include <wg_streamloopwrapper.h>
#include <assert.h>

namespace wg
{

	const TypeInfo StreamLoopWrapper::TYPEINFO = { "StreamLoopWrapper", &Object::TYPEINFO };

	//____ create() ___________________________________________________________

	StreamLoopWrapper_p StreamLoopWrapper::create(const void * pBufferBegin, const void * pBufferEnd,
														std::function<const void *()> getWritePtrFunc,
														std::function<void(const void * pReadPos)> setReadPtrFunc )
	{
		return new StreamLoopWrapper(pBufferBegin, pBufferEnd, getWritePtrFunc, setReadPtrFunc);
	}

	//____ constructor _____________________________________________________________

	StreamLoopWrapper::StreamLoopWrapper(const void * pBufferBegin, const void * pBufferEnd,
											   std::function<const void *()> getWritePtrFunc,
											   std::function<void(const void * pReadPos)> setReadPtrFunc ) : output(this)
	{
		m_pBufferBegin = (const uint8_t*) pBufferBegin;
		m_pBufferEnd = (const uint8_t*) pBufferEnd;
		
		m_pBeginChunks = (const uint8_t*) pBufferBegin;
		m_pEndChunks = (const uint8_t*) pBufferBegin;
		
		m_getWritePtrFunc = getWritePtrFunc;
		m_setReadPtrFunc = setReadPtrFunc;
	}

	//____ Destructor _________________________________________________________

	StreamLoopWrapper::~StreamLoopWrapper()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamLoopWrapper::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _hasStreamChunks() _________________________________________________

	bool StreamLoopWrapper::_hasStreamChunks()
	{
		return m_pBeginChunks != m_pEndChunks;
	}

	//____ _showStreamChunks() ________________________________________________

	std::tuple<int, const GfxStream::Data*> StreamLoopWrapper::_showStreamChunks()
	{
		if( m_pEndChunks == m_pBeginChunks )
			return std::make_tuple(int(0), nullptr);

		m_dataSegments[0].pBegin = (uint8_t *) m_pBeginChunks;

		if( m_pSplitChunkBegin )
		{
			m_dataSegments[0].pEnd = (uint8_t*) m_pSplitChunkBegin;

			m_dataSegments[1].pBegin = m_rejoinedChunk;
			m_dataSegments[1].pEnd = m_rejoinedChunk + m_rejoinedChunkSize;
			
			if( m_pSplitChunkEnd == m_pEndChunks )
				return std::make_tuple(int(2), m_dataSegments);

			m_dataSegments[2].pBegin = (uint8_t*) m_pSplitChunkEnd;
			m_dataSegments[2].pEnd = (uint8_t*) m_pEndChunks;

			return std::make_tuple(int(3), m_dataSegments);
		}
		else if( m_pEndChunks < m_pBeginChunks )
		{
			m_dataSegments[0].pEnd = (uint8_t*) m_pBufferEnd;

			m_dataSegments[1].pBegin = (uint8_t*) m_pBufferBegin;
			m_dataSegments[1].pEnd = (uint8_t*) m_pEndChunks;

			return std::make_tuple(int(2), m_dataSegments);
		}
		else
		{
			m_dataSegments[0].pEnd = (uint8_t*) m_pEndChunks;
			
			return std::make_tuple(int(1), m_dataSegments);
		}
	}

	//____ _peekStreamChunk() __________________________________________________________

	GfxStream::ChunkId StreamLoopWrapper:: _peekStreamChunk()
	{
		if( m_pBeginChunks == m_pEndChunks )
			return GfxStream::ChunkId::OutOfData;
		else
			return GfxStream::chunkType(m_pBeginChunks);
	}

	//____ _discardStreamChunks() _____________________________________________

	void StreamLoopWrapper::_discardStreamChunks(int bytes)
	{
		const uint8_t * p = m_pBeginChunks;
		p += bytes;
		if( p >= m_pBufferEnd )
		{
			p -= (m_pBufferEnd - m_pBufferBegin);
			
			m_rejoinedChunkSize = 0;
			m_pSplitChunkBegin = nullptr;
			m_pSplitChunkEnd = nullptr;
		}
		
		m_pBeginChunks = p;
		
		m_setReadPtrFunc(p);
	}

	//____ _fetchStreamChunks() _______________________________________________

	bool StreamLoopWrapper::_fetchStreamChunks()
	{
		const uint8_t *  pDataEnd = (const uint8_t*) m_getWritePtrFunc();

		const uint8_t *	pChunk = m_pEndChunks;
		
		if( pDataEnd < pChunk )
		{
			int endBufferBytes = int(m_pBufferEnd - pChunk);
			
			// Advance pChunk over chunks to the last, incomplete chunk in the buffer
			
			while( endBufferBytes > 4 )
			{
				// Get chunk size
				
				int chunkSize = ((uint16_t*) pChunk)[1] + 4;

				//
				
				if( chunkSize < endBufferBytes )
				{
					pChunk += chunkSize;
					endBufferBytes -= chunkSize;
				}
				else
					break;
			}
			
			if( pChunk < m_pBufferEnd )
			{
				// We have a broken chunk at the end

				int dataSize = int(m_pBufferEnd - pChunk) + int(pDataEnd - m_pBufferBegin);

				int chunkSize;

				if( endBufferBytes >= 4 )
					chunkSize = ((uint16_t*) pChunk)[1] + 4;
				else if( dataSize >= 4 )
					chunkSize = ((uint16_t*) m_pBufferBegin)[0] + 4;
				else
					goto out;

				// Leave if chunk is not complete
				
				if( chunkSize > dataSize )
					goto out;
				
				// Rejoin our chunk, set pointer
				
				int part1Size = int(m_pBufferEnd - pChunk);
				memcpy( m_rejoinedChunk, pChunk, part1Size );
				memcpy( m_rejoinedChunk + part1Size, m_pBufferBegin, chunkSize - part1Size );
				
				m_rejoinedChunkSize = chunkSize;
				m_pSplitChunkBegin = pChunk;
			
				pChunk = m_pBufferBegin + chunkSize - part1Size;
				m_pSplitChunkEnd = pChunk;

			}
			else
			{
				pChunk = m_pBufferBegin;
			}
		}
		
		//
		
		while( pChunk + 4 <= pDataEnd )
		{
			int chunkSize = ((uint16_t*) pChunk)[1] + 4;

			if( pChunk + chunkSize <= pDataEnd )
				pChunk += chunkSize;
			else
				break;
		}
out:
		if( pChunk != m_pEndChunks )
		{
			m_pEndChunks = pChunk;
			return true;
		}
		else
			return false;

	}


} // namespace wg
