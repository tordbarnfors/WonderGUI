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

#include <wg_streambuffer.h>
#include <wg_gfxbase.h>

namespace wg
{

	const TypeInfo StreamBuffer::TYPEINFO = { "StreamBuffer", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	StreamBuffer_p StreamBuffer::create( int segmentSize )
	{
		return new StreamBuffer(segmentSize);
	}

	//____ constructor _____________________________________________________________

	StreamBuffer::StreamBuffer(int initialCapacity) : input(this), output(this)
	{
		m_pBuffer = new char[initialCapacity + c_bufferMargin];
		m_bufferSize = initialCapacity;
		m_readOfs = 0;
		m_writeOfs = 0;
		m_processedOfs = 0;
		m_bufferOverflow = 0;
	}

	//____ Destructor _________________________________________________________

	StreamBuffer::~StreamBuffer()
	{
		delete [] m_pBuffer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamBuffer::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ reset() ____________________________________________________________

	void StreamBuffer::reset( int newBufferSize )
	{
		if( newBufferSize != 0 && newBufferSize != m_bufferSize )
		{
			delete [] m_pBuffer;
			m_pBuffer = new char[newBufferSize + c_bufferMargin];
		}

		m_readOfs = 0;
		m_writeOfs = 0;
		m_processedOfs = 0;
		m_bufferOverflow = 0;
	}


	//____ size() ____________________________________________________

	int StreamBuffer::size() const
	{
		if( m_readOfs <= m_writeOfs )
			return m_writeOfs - m_readOfs;
		else
			return m_writeOfs + m_bufferSize - m_readOfs;
	}

	//____ setCapacity() _________________________________________________________

	int StreamBuffer::setCapacity( int capacity )
	{
		int minCapacity = size();
				
		if( capacity < minCapacity)
			capacity = minCapacity;

		_resizeBuffer(capacity+2);			// Two byte margin since writeOf may not catch up to readOfs.
		
		return capacity;
	}

	//____ _processStreamChunks() _____________________________________________

	void StreamBuffer::_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd)
	{
		//---------------------------
		// Grow buffer if needed.
		//---------------------------

		int neededSpace = int(pEnd - pBegin);
		int currentSpace = m_bufferSize - ((m_writeOfs - m_readOfs + m_bufferSize) % m_bufferSize) - 2;		// -2 since we may not catch up to readOfs

		int bufferSizeNeeded = m_bufferSize;
		while (currentSpace < neededSpace)
		{
			currentSpace += bufferSizeNeeded;
			bufferSizeNeeded += bufferSizeNeeded;
		}

		if (bufferSizeNeeded > m_bufferSize)
		{
			char	msg[64];
			snprintf( msg, 64, "StreamBuffer reallocated to %d bytes", bufferSizeNeeded);
			GfxBase::throwError(ErrorLevel::Warning, ErrorCode::Performance, msg, this, &TYPEINFO, __func__, __FILE__, __LINE__);

			_resizeBuffer(bufferSizeNeeded);
		}
		//---------------------------
		// Copy data into buffer
		//---------------------------

		// Calcuate size of chunks to fill in the circular buffer

		int size = m_bufferSize - ((m_writeOfs - m_readOfs + m_bufferSize) % m_bufferSize) - 2;		// -2 since we may not catch up to readOfs

		int chunk1 = std::min(size, m_bufferSize - m_writeOfs);		// Until readPos or end of buffer
		int chunk2 = size - chunk1;								// From start of buffer

		// Copy data to first chunk

		int bytesToWrite = std::min(neededSpace, chunk1);
		if (bytesToWrite == 0)
			return;

		memcpy(m_pBuffer + m_writeOfs, pBegin, bytesToWrite);
		int bytesWritten = bytesToWrite;

		m_writeOfs = (m_writeOfs + bytesWritten) % m_bufferSize;

		// Fetch data to second chunk if we filled first chunk

		if (bytesWritten == chunk1 && chunk2 > 0)
		{
			bytesToWrite = std::min(neededSpace-bytesWritten, chunk2);
			memcpy(m_pBuffer, pBegin + bytesWritten, bytesToWrite);

			m_writeOfs = bytesToWrite;
		}

		// Advance m_processedOfs, handling any overflow

		int unprocessedData = ((m_writeOfs - m_processedOfs + m_bufferSize) % m_bufferSize);

		while (unprocessedData >= 2)
		{
			int chunkSize = (m_pBuffer[(m_processedOfs + 1) % m_bufferSize] & 0x1F) + 2;

			if (chunkSize >= (31+2))
			{
				if (unprocessedData < 4)
					break;

				chunkSize = (*(uint16_t*)&m_pBuffer[(m_processedOfs + 2) % m_bufferSize]) + 4;
			}

			if (chunkSize > unprocessedData)
				break;

			// Handle overflow.

			if (m_processedOfs + chunkSize >= m_bufferSize)
			{
				m_bufferOverflow = m_processedOfs + chunkSize - m_bufferSize;
				memcpy(m_pBuffer + m_bufferSize, m_pBuffer, m_bufferOverflow);
			}

			m_processedOfs = (m_processedOfs + chunkSize) % m_bufferSize;
			unprocessedData -= chunkSize;
		}

	}

	//____ _hasStreamChunks() _________________________________________________

	bool StreamBuffer::_hasStreamChunks()
	{
		return (m_processedOfs != m_readOfs);
	}

	//____ _showStreamChunks() ________________________________________________

	std::tuple<int, const GfxStream::Data*> StreamBuffer::_showStreamChunks()
	{
		if(m_processedOfs == m_readOfs )
			return std::make_tuple(int(0), nullptr);

		if (m_processedOfs < m_readOfs)
		{
			m_dataSegments[0].pBegin = (uint8_t*)m_pBuffer + m_readOfs;
			m_dataSegments[0].pEnd = (uint8_t*)m_pBuffer + m_bufferSize + m_bufferOverflow;



			if (m_processedOfs == m_bufferOverflow)
				return std::make_tuple(int(1), m_dataSegments);

			m_dataSegments[1].pBegin = (uint8_t*)m_pBuffer + m_bufferOverflow;
			m_dataSegments[1].pEnd = (uint8_t*)m_pBuffer + m_processedOfs;
			return std::make_tuple(int(2), m_dataSegments);

		}
		else
		{
			m_dataSegments[0].pBegin = (uint8_t*)m_pBuffer + m_readOfs;
			m_dataSegments[0].pEnd = (uint8_t*)m_pBuffer + m_processedOfs;
			return std::make_tuple(int(1), m_dataSegments);
		}
	}

	//____ _peekStreamChunk() __________________________________________________________

	GfxStream::ChunkId StreamBuffer:: _peekStreamChunk()
	{
		if(m_processedOfs == m_readOfs )
			return GfxStream::ChunkId::OutOfData;

		return (GfxStream::ChunkId)  m_pBuffer[m_readOfs];
	}

	//____ _discardStreamChunks() _____________________________________________

	void StreamBuffer::_discardStreamChunks(int bytes)
	{
		m_readOfs = (m_readOfs + bytes) % m_bufferSize;
		if (m_readOfs < m_processedOfs)
			m_bufferOverflow = 0;
	}

	//____ _fetchStreamChunks() _______________________________________________

	bool StreamBuffer::_fetchStreamChunks()
	{
		return false;
	}

	//____ _resizeBuffer() ____________________________________________________

	void StreamBuffer::_resizeBuffer(int newSize)
	{
		// Grow the buffer

		int oldBufferSize = m_bufferSize;
		char* pOldBuffer = m_pBuffer;

		m_bufferSize = newSize;
		m_pBuffer = new char[m_bufferSize + c_bufferMargin];

		if(m_writeOfs == m_readOfs)
		{
			m_readOfs = 0;
			m_writeOfs = 0;
			m_processedOfs = 0;
		}
		else if (m_writeOfs > m_readOfs)
		{
			memcpy(m_pBuffer, pOldBuffer + m_readOfs, m_writeOfs - m_readOfs);
			int ofsSub = m_readOfs;

			m_readOfs -= ofsSub;
			m_writeOfs -= ofsSub;
			m_processedOfs -= ofsSub;
		}
		else
		{
			int chunkSize1 = oldBufferSize - m_readOfs;
			int chunkSize2 = m_writeOfs;

			memcpy(m_pBuffer, pOldBuffer + m_readOfs, chunkSize1);
			memcpy(m_pBuffer + chunkSize1, pOldBuffer, chunkSize2);

			m_readOfs = 0;
			m_writeOfs = chunkSize1 + chunkSize2;
			m_processedOfs = (m_processedOfs - m_readOfs + oldBufferSize) % oldBufferSize;
		}

		m_bufferOverflow = 0;
		delete[] pOldBuffer;
	}


} // namespace wg
