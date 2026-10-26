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
#include <wg_streamdecoder.h>

#include <tuple>

namespace wg
{
	const TypeInfo StreamDecoder::TYPEINFO = { "StreamDecoder", &Object::TYPEINFO };

	//____ constructor ________________________________________________________

	StreamDecoder::StreamDecoder()
	{
	}

	//____ destructor ________________________________________________________

	StreamDecoder::~StreamDecoder()
	{
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamDecoder::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setVersion() ______________________________________________________

	void StreamDecoder::setVersion(uint16_t version)
	{
		m_version = version;
		m_dataInfoSize = version < 0x0300 ? 10 : 20;
	}


	//____ setInput() ________________________________________________________

	void StreamDecoder::setInput(const void * pBegin, const void * pEnd)
	{
		m_pDataBegin = (const uint8_t*) pBegin;
		m_pDataEnd = (const uint8_t*) pEnd;
		m_pDataRead = (const uint8_t*) pBegin;
	}

	//____ isEmpty() __________________________________________________________

	bool StreamDecoder::isEmpty()
	{
		return !_hasChunk();
	}

	//____ peek() _____________________________________________________________

	GfxStream::Header StreamDecoder::peek()
	{
		if (_hasChunk())
			return _peekChunk();

		return { GfxStream::ChunkId::OutOfData, 0 };
	}

	//____ skip() _____________________________________________________________

	void StreamDecoder::skip(int bytes)
	{
		_skipBytes(bytes);
	}

	//____ align() _____________________________________________________________

	void StreamDecoder::align()
	{
		if( ( std::intptr_t(m_pDataRead) & 0x1) == 1 )
			_skipBytes(1);
	}

}
