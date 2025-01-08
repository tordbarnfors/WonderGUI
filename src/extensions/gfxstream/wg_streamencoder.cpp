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

#include <wg_streamencoder.h>

#include <algorithm>

namespace wg
{
	const TypeInfo StreamEncoder::TYPEINFO = { "StreamEncoder", &Object::TYPEINFO };

	//____ constructor ________________________________________________________

	StreamEncoder::StreamEncoder(const StreamSink_p& pStream, int bufferBytes)
	{
		m_pBuffer = new uint8_t[bufferBytes];
		m_capacity = bufferBytes;
		m_pWriteData = m_pBuffer;
		m_pStream = pStream;
	}

	//____ destructor ________________________________________________________

	StreamEncoder::~StreamEncoder()
	{
		delete[] m_pBuffer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamEncoder::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setStream() ________________________________________________________

	void StreamEncoder::setStream(const StreamSink_p& pStream)
	{
		m_pStream = pStream;
	}

	//____ setDefaultPixelFormat() ____________________________________________

	void StreamEncoder::setDefaultPixelFormat(PixelFormat pixelFormat)
	{
		m_defaultPixelFormat = pixelFormat;
	}

	//____ setDefaultSampleMethod() ___________________________________________

	void StreamEncoder::setDefaultSampleMethod(SampleMethod sampleMethod)
	{
		m_defaultSampleMethod = sampleMethod;
	}

	//____ flush() ____________________________________________________________

	void StreamEncoder::flush()
	{
		if (m_pStream)
			m_pStream->processChunks(m_pBuffer, m_pWriteData);

		m_pWriteData = m_pBuffer;
	}

	//____ allocObjectId() ____________________________________________________

	uint16_t StreamEncoder::allocObjectId()
	{
		if (m_freeIdStackSize > 0)
			return m_pFreeIdStack[--m_freeIdStackSize];

		return m_idCounter++;
	}

	//____ freeObjectId() _____________________________________________________

	void StreamEncoder::freeObjectId(uint16_t id)
	{
		if (m_freeIdStackSize == m_freeIdStackCapacity)
		{
			int capacity = std::max(16, m_freeIdStackCapacity * 2);
			uint16_t* pBuffer = new uint16_t[capacity];

			for (int i = 0; i < m_freeIdStackSize; i++)
				pBuffer[i] = m_pFreeIdStack[i];

			delete[] m_pFreeIdStack;

			m_pFreeIdStack = pBuffer;
			m_freeIdStackCapacity = capacity;
		}

		m_pFreeIdStack[m_freeIdStackSize++] = id;
	}

	//____ _beginChunk() _________________________________________________________

	void StreamEncoder::_beginChunk(GfxStream::Header header)
	{
		if (m_pWriteData + (header.size + 4) - m_pBuffer > m_capacity)
			flush();

		_pushChar((char)header.type);
		_pushChar((char)header.format);
		_pushShort(header.size);
	}
}
