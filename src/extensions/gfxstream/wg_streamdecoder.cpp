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

#include <wg_streamdecoder.h>

#include <tuple>

namespace wg
{
	const TypeInfo StreamDecoder::TYPEINFO = { "StreamDecoder", &Object::TYPEINFO };


	const uint8_t* spxOp0(const uint8_t* pDst, spx& output)
	{
		uint16_t* p = (uint16_t*)pDst;

		output = p[0] + (int(p[1]) << 16);
		return pDst + 4;
	}

	const uint8_t* spxOp1(const uint8_t* pDst, spx& output)
	{
		uint16_t* p = (uint16_t*)pDst;

		output = p[0];
		return pDst + 2;
	}

	const uint8_t* spxOp2(const uint8_t* pDst, spx& output)
	{
		int16_t* p = (int16_t*)pDst;

		output = int(p[0]) << 6;
		return pDst + 2;
	}

	const uint8_t* spxOp3(const uint8_t* p, spx& output)
	{
		output = int(p[0]) << 6;
		return p + 1;
	}

	const uint8_t* coordOp0(const uint8_t* pDst, CoordI& output)
	{
		uint16_t* p = (uint16_t*)pDst;

		output.x = p[0] + (int(p[1]) << 16);
		output.y = p[2] + (int(p[3]) << 16);
		return pDst + 8;
	}

	const uint8_t* coordOp1(const uint8_t* pDst, CoordI& output)
	{
		uint16_t* p = (uint16_t*)pDst;

		output.x = p[0];
		output.y = p[1];
		return pDst + 4;
	}

	const uint8_t* coordOp2(const uint8_t* pDst, CoordI& output)
	{
		int16_t* p = (int16_t*)pDst;

		output.x = int(p[0]) << 6;
		output.y = int(p[1]) << 6;
		return pDst + 4;
	}

	const uint8_t* coordOp3(const uint8_t* p, CoordI& output)
	{
		output.x = int(p[0]) << 6;
		output.y = int(p[1]) << 6;
		return p + 2;
	}

	const uint8_t* sizeOp0(const uint8_t* pDst, SizeI& output)
	{
		uint16_t* p = (uint16_t*)pDst;

		output.w = p[0] + (int(p[1]) << 16);
		output.h = p[2] + (int(p[3]) << 16);
		return pDst + 8;
	}

	const uint8_t* sizeOp1(const uint8_t* pDst, SizeI& output)
	{
		uint16_t* p = (uint16_t*)pDst;

		output.w = p[0];
		output.h = p[1];
		return pDst + 4;
	}

	const uint8_t* sizeOp2(const uint8_t* pDst, SizeI& output)
	{
		int16_t* p = (int16_t*)pDst;

		output.w = int(p[0]) << 6;
		output.h = int(p[1]) << 6;
		return pDst + 4;
	}

	const uint8_t* sizeOp3(const uint8_t* p, SizeI& output)
	{
		output.w = int(p[0]) << 6;
		output.h = int(p[1]) << 6;
		return p + 2;
	}

	const uint8_t* rectOp0(const uint8_t* pDst, RectI& output)
	{
		uint16_t* p = (uint16_t*)pDst;

		output.x = p[0] + (int(p[1]) << 16);
		output.y = p[2] + (int(p[3]) << 16);
		output.w = p[4] + (int(p[5]) << 16);
		output.h = p[6] + (int(p[7]) << 16);
		return pDst + 16;
	}

	const uint8_t* rectOp1(const uint8_t* pDst, RectI& output)
	{
		uint16_t* p = (uint16_t*)pDst;

		output.x = p[0];
		output.y = p[1];
		output.w = p[2];
		output.h = p[3];
		return pDst + 8;
	}

	const uint8_t* rectOp2(const uint8_t* pDst, RectI& output)
	{
		int16_t* p = (int16_t*)pDst;

		output.x = int(p[0]) << 6;
		output.y = int(p[1]) << 6;
		output.w = int(p[2]) << 6;
		output.h = int(p[3]) << 6;
		return pDst + 8;
	}

	const uint8_t* rectOp3(const uint8_t* p, RectI& output)
	{
		output.x = int(p[0]) << 6;
		output.y = int(p[1]) << 6;
		output.w = int(p[2]) << 6;
		output.h = int(p[3]) << 6;
		return p + 4;
	}

	const StreamDecoder::SpxOp_p			StreamDecoder::s_spxOps[4] = { spxOp0, spxOp1, spxOp2, spxOp3 };
	const StreamDecoder::CoordOp_p		StreamDecoder::s_coordOps[4] = { coordOp0, coordOp1, coordOp2, coordOp3 };
	const StreamDecoder::SizeOp_p		StreamDecoder::s_sizeOps[4] = { sizeOp0, sizeOp1, sizeOp2, sizeOp3 };
	const StreamDecoder::RectOp_p		StreamDecoder::s_rectOps[4]	= { rectOp0, rectOp1, rectOp2, rectOp3 };

	//____ constructor ________________________________________________________

	StreamDecoder::StreamDecoder()
	{
		m_spxFormat = GfxStream::SpxFormat::Int32_dec;
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
