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

#include <wax_nodecompression.h>
#include <wax_util.h>
#include <wax_base.h>

#include <cstring>

namespace wax
{
const TypeInfo NodeCompressor::TYPEINFO = { "NodeCompressor", &Compressor::TYPEINFO };
const TypeInfo NodeDecompressor::TYPEINFO = { "NodeDecompressor", &Decompressor::TYPEINFO };

const uint32_t NodeCompressor::ID_TOKEN = Util::makeEndianSpecificToken('N', 'O', 'D', '0');
const uint32_t NodeDecompressor::ID_TOKEN = Util::makeEndianSpecificToken('N', 'O', 'D', '0');

namespace
{
	enum Mode : uint8_t
	{
		Raw = 0,
		Predicted = 1
	};

	const int c_maxCodeBytes = 5;

	// Prediction of value i of pairs in rows of rowLength pairs: left + above - above left, only
	// left on the first row, only above in the first column. Wraps around at 32 bits.

	inline uint32_t predict(const uint32_t* pValues, int i, int row, int column, int rowLength)
	{
		int above = rowLength * 2;

		if (row == 0)
			return column == 0 ? 0 : pValues[i - 2];
		if (column == 0)
			return pValues[i - above];
		return pValues[i - 2] + pValues[i - above] - pValues[i - above - 2];
	}

	inline uint8_t* writeCode(uint8_t* p, uint32_t value)
	{
		if (value < (1u << 7))
			*p++ = uint8_t(value);
		else if (value < (1u << 14))
		{
			*p++ = uint8_t(0x80 | (value >> 8));
			*p++ = uint8_t(value);
		}
		else if (value < (1u << 21))
		{
			*p++ = uint8_t(0xC0 | (value >> 16));
			*p++ = uint8_t(value >> 8);
			*p++ = uint8_t(value);
		}
		else if (value < (1u << 28))
		{
			*p++ = uint8_t(0xE0 | (value >> 24));
			*p++ = uint8_t(value >> 16);
			*p++ = uint8_t(value >> 8);
			*p++ = uint8_t(value);
		}
		else
		{
			*p++ = 0xF0;
			*p++ = uint8_t(value >> 24);
			*p++ = uint8_t(value >> 16);
			*p++ = uint8_t(value >> 8);
			*p++ = uint8_t(value);
		}
		return p;
	}

	// Reads a code, nullptr if it runs past pEnd.

	inline const uint8_t* readCode(const uint8_t* p, const uint8_t* pEnd, uint32_t& value)
	{
		uint32_t first = *p++;

		if (first < 0x80)
		{
			value = first;
			return p;
		}

		int bytes;
		if (first < 0xC0)
		{
			bytes = 1;
			value = first & 0x3F;
		}
		else if (first < 0xE0)
		{
			bytes = 2;
			value = first & 0x1F;
		}
		else if (first < 0xF0)
		{
			bytes = 3;
			value = first & 0x0F;
		}
		else
		{
			bytes = 4;
			value = 0;
		}

		if (pEnd - p < bytes)
			return nullptr;

		for (int i = 0; i < bytes; i++)
			value = (value << 8) | *p++;

		return p;
	}

	inline uint32_t zigzag(uint32_t difference) { return (difference << 1) ^ (0 - (difference >> 31)); }
	inline uint32_t unzigzag(uint32_t code) { return (code >> 1) ^ (0 - (code & 1)); }
}

//____ NodeCompressor::create() _______________________________________________

NodeCompressor_p NodeCompressor::create()
{
	return NodeCompressor_p( new NodeCompressor() );
}

//____ NodeCompressor::typeInfo() _____________________________________________

const TypeInfo& NodeCompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ NodeCompressor::idToken() ______________________________________________

uint32_t NodeCompressor::idToken() const
{
	return ID_TOKEN;
}

//____ NodeCompressor::setRowLength() _________________________________________

bool NodeCompressor::setRowLength( int pairs )
{
	if (pairs <= 0)
		return false;

	m_rowLength = pairs;
	return true;
}

//____ NodeCompressor::maxCompressedSize() ____________________________________
/*
	A value takes up to 5 bytes, 10 bytes for a pair of 8, plus mode and row length. Room for
	that much is also what keeps decompressing in place from overwriting data not yet read.
*/

int NodeCompressor::maxCompressedSize( int uncompressedSize )
{
	return 1 + c_maxCodeBytes + (uncompressedSize / 8) * 2 * c_maxCodeBytes + (uncompressedSize % 8);
}

//____ NodeCompressor::compress() _____________________________________________

int NodeCompressor::compress( void * _pDest, const void * _pBegin, const void * _pEnd )
{
	auto pDest = (uint8_t*) _pDest;
	auto pBegin = (const uint8_t*) _pBegin;
	auto pEnd = (const uint8_t*) _pEnd;

	int bytes = int(pEnd - pBegin);
	int pairs = bytes / 8;

	if (bytes % 8 == 0 && pairs % m_rowLength == 0 && pairs > 0)
	{
		auto pValues = (const uint32_t*) pBegin;
		int rows = pairs / m_rowLength;

		uint8_t* wp = pDest;
		*wp++ = Mode::Predicted;
		wp = writeCode(wp, uint32_t(m_rowLength));

		int i = 0;
		for (int row = 0; row < rows; row++)
			for (int column = 0; column < m_rowLength; column++)
				for (int value = 0; value < 2; value++, i++)
					wp = writeCode(wp, zigzag(pValues[i] - predict(pValues, i, row, column, m_rowLength)));

		int size = int(wp - pDest);
		if (size <= 1 + bytes)
			return size;
	}

	*pDest = Mode::Raw;
	std::memcpy(pDest + 1, pBegin, bytes);
	return 1 + bytes;
}

//____ NodeDecompressor::create() _____________________________________________

NodeDecompressor_p NodeDecompressor::create()
{
	return NodeDecompressor_p( new NodeDecompressor() );
}

//____ NodeDecompressor::typeInfo() ___________________________________________

const TypeInfo& NodeDecompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ NodeDecompressor::idToken() ____________________________________________

uint32_t NodeDecompressor::idToken() const
{
	return ID_TOKEN;
}

//____ NodeDecompressor::decompress() _________________________________________
/*
	Values are written as soon as they are read, so decompressing in place works as long as the
	compressed data starts far enough into the buffer, see NodeCompressor::maxCompressedSize().
*/

int NodeDecompressor::decompress( void * _pDest, const void * _pBegin, const void * _pEnd )
{
	auto pDest = (uint8_t*) _pDest;
	auto p = (const uint8_t*) _pBegin;
	auto pEnd = (const uint8_t*) _pEnd;

	if (p >= pEnd)
		return 0;

	uint8_t mode = *p++;

	if (mode == Mode::Raw)
	{
		int bytes = int(pEnd - p);
		std::memmove(pDest, p, bytes);
		return bytes;
	}

	if (mode != Mode::Predicted)
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Unknown mode of compressed nodes.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return 0;
	}

	uint32_t rowLength;
	p = readCode(p, pEnd, rowLength);
	if (!p || rowLength == 0 || rowLength > (1u << 24))
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Invalid row length of compressed nodes.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return 0;
	}

	auto pValues = (uint32_t*) pDest;
	int row = 0, column = 0, value = 0;
	int i = 0;

	while (p < pEnd)
	{
		uint32_t code;
		p = readCode(p, pEnd, code);
		if (!p)
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Compressed nodes end in the middle of a value.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			break;
		}

		pValues[i] = predict(pValues, i, row, column, int(rowLength)) + unzigzag(code);
		i++;

		if (++value == 2)
		{
			value = 0;
			if (++column == int(rowLength))
			{
				column = 0;
				row++;
			}
		}
	}

	return i * 4;
}

} // namespace wax
