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

#include <wg_streamreader.h>
#include <assert.h>

namespace wg
{

	const TypeInfo StreamReader::TYPEINFO = { "StreamReader", &Object::TYPEINFO };

	//____ create() ___________________________________________________________

	StreamReader_p StreamReader::create(std::function<int(int nBytes, void * pDest)> dataFetcher)
	{
		return new StreamReader(dataFetcher);
	}

	//____ constructor _____________________________________________________________

	StreamReader::StreamReader(std::function<int(int nBytes, void * pDest)> dataFeeder) : output(this)
	{
		m_fetcher = dataFeeder;

		m_pBuffer = new char[c_bufferStartSize+c_bufferMargin];
		m_bufferSize = c_bufferStartSize;
		m_readOfs = 0;
		m_writeOfs = 0;
		m_processedOfs = 0;
		m_bufferOverflow = 0;
	}

	//____ Destructor _________________________________________________________

	StreamReader::~StreamReader()
	{
		delete[] m_pBuffer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamReader::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ reset() ____________________________________________________________

	void StreamReader::reset()
	{
		if (m_bufferSize != c_bufferStartSize)
		{
			delete[] m_pBuffer;
			m_pBuffer = new char[c_bufferStartSize + c_bufferMargin];
			m_bufferSize = c_bufferStartSize;
		}

		m_readOfs = 0;
		m_writeOfs = 0;
		m_processedOfs = 0;
		m_bufferOverflow = 0;
	}

	//____ bytesInBuffer() ____________________________________________________

	int StreamReader::bytesInBuffer() const
	{
		if( m_readOfs <= m_writeOfs )
			return m_writeOfs - m_readOfs;
		else
			return m_writeOfs + m_bufferSize - m_readOfs;
	}

	//____ _fetchData() _______________________________________________________

	void StreamReader::_fetchData()
	{

		// Calcuate size of chunks to fill in the circular buffer

		int size = m_bufferSize - ((m_writeOfs - m_readOfs + m_bufferSize) % m_bufferSize) -2;		// -2 since we may not catch up to readOfs

		int chunk1 = std::min(size, m_bufferSize - m_writeOfs);		// Until readPos or end of buffer
		int chunk2 = size - chunk1;								// From start of buffer

		// Fetch data to first chunk

		int written = m_fetcher(chunk1, m_pBuffer + m_writeOfs);
		if (written == 0)
			return;

		m_writeOfs = (m_writeOfs + written) % m_bufferSize;

		// Fetch data to second chunk if we filled first chunk

		if (written == chunk1 && chunk2 > 0)
		{
			written = m_fetcher(chunk2, m_pBuffer);
			m_writeOfs = written;
		}

		// Advance m_processedOfs, handling any overflow

		int unprocessedData = ((m_writeOfs - m_processedOfs + m_bufferSize) % m_bufferSize);

		while (unprocessedData >= 2)
		{
			int chunkSize = (m_pBuffer[(m_processedOfs + 1) % m_bufferSize] & 0x1F) + 2;

			if (chunkSize >= (31 + 2))
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

	bool StreamReader::_hasStreamChunks()
	{
		return (m_processedOfs != m_readOfs);
	}

	//____ _showStreamChunks() ________________________________________________

	std::tuple<int, const GfxStream::Data*> StreamReader::_showStreamChunks()
	{
		if (m_processedOfs == m_readOfs)
			return std::make_tuple(int(0), nullptr);

		if (m_processedOfs < m_readOfs)
		{
			m_dataSegments[0].pBegin = (uint8_t*)m_pBuffer + m_readOfs;
			m_dataSegments[0].pEnd = (uint8_t*)m_pBuffer + m_bufferSize + m_bufferOverflow;



			if (m_processedOfs == m_bufferOverflow )
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

	GfxStream::ChunkId StreamReader:: _peekStreamChunk()
	{
		if(m_processedOfs == m_readOfs )
			return GfxStream::ChunkId::OutOfData;

		return GfxStream::chunkType((uint8_t*)(m_pBuffer + m_readOfs));
	}

	//____ _discardStreamChunks() _____________________________________________

	void StreamReader::_discardStreamChunks(int bytes)
	{
		m_readOfs = (m_readOfs + bytes) % m_bufferSize;
		if (m_readOfs <= m_processedOfs)
			m_bufferOverflow = 0;
	}
	 
	//____ _fetchStreamChunks() _______________________________________________

	bool StreamReader::_fetchStreamChunks()
	{
		int dataInBuffer = (m_writeOfs - m_readOfs + m_bufferSize) % m_bufferSize;
		int maxFetch = m_bufferSize - dataInBuffer - 2;		// -2 since we may not catch up to readOfs

		if (maxFetch == 0)
		{
			// Grow the buffer

			int oldBufferSize = m_bufferSize;
			char* pOldBuffer = m_pBuffer;

			m_bufferSize *= 2;
			m_pBuffer = new char[m_bufferSize + c_bufferMargin];

			if (m_writeOfs > m_readOfs)
			{
				memcpy(m_pBuffer, pOldBuffer + m_readOfs, m_writeOfs - m_readOfs);
				int ofsSub = m_readOfs;

				m_readOfs -= ofsSub;
				m_writeOfs -= ofsSub;
				m_processedOfs -= ofsSub;
				m_bufferOverflow = 0;
			}
			else
			{
				int chunkSize1 = oldBufferSize - m_readOfs;
				int chunkSize2 = m_writeOfs;

				memcpy(m_pBuffer, pOldBuffer + m_readOfs, chunkSize1);
				memcpy(m_pBuffer + chunkSize1, pOldBuffer, chunkSize2);

				m_processedOfs = (m_processedOfs - m_readOfs + oldBufferSize) % oldBufferSize;
				m_readOfs = 0;
				m_writeOfs = chunkSize1 + chunkSize2;
				m_bufferOverflow = 0;
			}

			delete [] pOldBuffer;
		}

		int oldProcessedOfs = m_processedOfs;

		_fetchData();
		if (oldProcessedOfs == m_processedOfs)
		{
			// We didn't get a full chunk, either buffer is full or there is no more data atm.

			int dataInBuffer = (m_writeOfs - m_readOfs + m_bufferSize) % m_bufferSize;
			int maxFetch = m_bufferSize - dataInBuffer - 2;		// -2 since we may not catch up to readOfs

			if (maxFetch == 0)
			{
				// Call ourselves recursively to grow the buffer and refetch.

				return _fetchStreamChunks();
			}

			return false;
		}

		return true;
	}


} // namespace wg
