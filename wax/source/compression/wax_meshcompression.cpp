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

#include <wax_meshcompression.h>
#include <wax_util.h>
#include <wax_base.h>

#include <algorithm>
#include <cstring>

namespace wax
{
const TypeInfo MeshCompressor::TYPEINFO = { "MeshCompressor", &Compressor::TYPEINFO };
const TypeInfo MeshDecompressor::TYPEINFO = { "MeshDecompressor", &Decompressor::TYPEINFO };

const uint32_t MeshCompressor::ID_TOKEN = Util::makeEndianSpecificToken('M', 'S', 'H', '0');
const uint32_t MeshDecompressor::ID_TOKEN = Util::makeEndianSpecificToken('M', 'S', 'H', '0');

namespace
{
	enum Mode : uint8_t
	{
		Raw = 0,
		Predicted = 1
	};

	const int c_maxCodeBytes = 5;
	const int c_maxLag = 32;					// Lags tried by the compressor. The decompressor takes any.
	const int c_trialValues = 128;				// Values at the start of a sequence that lags are tried on.

	// Prediction of value i of a sequence from the values lag steps back. Wraps around at 32 bits.

	inline uint32_t predict(const uint32_t* pValues, int i, int lag, int order)
	{
		if (i < lag)
			return 0;
		if (order == 2 && i >= 2 * lag)
			return 2 * pValues[i - lag] - pValues[i - 2 * lag];
		return pValues[i - lag];
	}

	inline int codeSize(uint32_t value)
	{
		return value < (1u << 7) ? 1 : value < (1u << 14) ? 2 : value < (1u << 21) ? 3 : value < (1u << 28) ? 4 : 5;
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
		if (p >= pEnd)
			return nullptr;

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

	// Calls code(value) for each difference that isn't 0 and run(length) for each run of zeros, of
	// the first n values of a sequence.

	template<typename Code, typename Run>
	inline void differences(const uint32_t* pValues, int n, int lag, int order, Code code, Run run)
	{
		int zeros = 0;
		for (int i = 0; i < n; i++)
		{
			uint32_t difference = pValues[i] - predict(pValues, i, lag, order);
			if (difference == 0)
			{
				zeros++;
				continue;
			}

			if (zeros > 0)
			{
				run(zeros);
				zeros = 0;
			}
			code(zigzag(difference));
		}

		if (zeros > 0)
			run(zeros);
	}
}

//____ MeshCompressor::create() _______________________________________________

MeshCompressor_p MeshCompressor::create()
{
	return MeshCompressor_p( new MeshCompressor() );
}

//____ MeshCompressor::typeInfo() _____________________________________________

const TypeInfo& MeshCompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ MeshCompressor::idToken() ______________________________________________

uint32_t MeshCompressor::idToken() const
{
	return ID_TOKEN;
}

//____ MeshCompressor::setSequences() _________________________________________

bool MeshCompressor::setSequences( int sequences )
{
	if (sequences <= 0)
		return false;

	m_sequences = sequences;
	return true;
}

//____ MeshCompressor::maxCompressedSize() ____________________________________
/*
	A difference takes up to 5 bytes and a run of zeros up to 10 bytes for at least 2 values, plus
	mode, number and length of sequences and a code for each. Room for that much is also what keeps
	decompressing in place from overwriting data not yet read: the values written so far, 4 bytes
	each, and the data not yet read, at most 5 bytes for each value left and the codes of the
	sequences left, never take more.
*/

int MeshCompressor::maxCompressedSize( int uncompressedSize )
{
	int predicted = 1 + 2 * c_maxCodeBytes + m_sequences * c_maxCodeBytes + (uncompressedSize / 4) * c_maxCodeBytes;
	return std::max(predicted, 1 + uncompressedSize);
}

//____ MeshCompressor::compress() _____________________________________________

int MeshCompressor::compress( void * _pDest, const void * _pBegin, const void * _pEnd )
{
	auto pDest = (uint8_t*) _pDest;
	auto pBegin = (const uint8_t*) _pBegin;
	auto pEnd = (const uint8_t*) _pEnd;

	int bytes = int(pEnd - pBegin);
	int values = bytes / 4;

	if (bytes % 4 == 0 && values > 0 && values % m_sequences == 0)
	{
		const int length = values / m_sequences;

		uint8_t* wp = pDest;
		*wp++ = Mode::Predicted;
		wp = writeCode(wp, uint32_t(m_sequences));
		wp = writeCode(wp, uint32_t(length));

		for (int s = 0; s < m_sequences; s++)
		{
			auto pValues = (const uint32_t*) pBegin + s * length;

			// Lag and order that take the fewest bytes for the start of the sequence.

			int bestLag = 1, bestOrder = 1, bestSize = -1;
			const int trial = std::min(length, c_trialValues);

			for (int lag = 1; lag <= std::min(c_maxLag, std::max(1, trial - 1)); lag++)
			{
				for (int order = 1; order <= 2; order++)
				{
					int size = 0;
					differences(pValues, trial, lag, order, [&](uint32_t code) { size += codeSize(code); }, [&](int zeros) { size += 1 + codeSize(uint32_t(zeros - 1)); });

					if (bestSize < 0 || size < bestSize)
					{
						bestSize = size;
						bestLag = lag;
						bestOrder = order;
					}
				}
			}

			wp = writeCode(wp, uint32_t((bestLag - 1) * 2 + bestOrder - 1));
			differences(pValues, length, bestLag, bestOrder, [&](uint32_t code) { wp = writeCode(wp, code); }, [&](int zeros) { *wp++ = 0; wp = writeCode(wp, uint32_t(zeros - 1)); });
		}

		int size = int(wp - pDest);
		if (size <= 1 + bytes)
			return size;
	}

	*pDest = Mode::Raw;
	std::memcpy(pDest + 1, pBegin, bytes);
	return 1 + bytes;
}

//____ MeshDecompressor::create() _____________________________________________

MeshDecompressor_p MeshDecompressor::create()
{
	return MeshDecompressor_p( new MeshDecompressor() );
}

//____ MeshDecompressor::typeInfo() ___________________________________________

const TypeInfo& MeshDecompressor::typeInfo(void) const
{
	return TYPEINFO;
}

//____ MeshDecompressor::idToken() ____________________________________________

uint32_t MeshDecompressor::idToken() const
{
	return ID_TOKEN;
}

//____ MeshDecompressor::decompress() _________________________________________
/*
	Values are written as soon as they are read, so decompressing in place works as long as the
	compressed data starts far enough into the buffer, see MeshCompressor::maxCompressedSize().
	Decompressing in place, with the data within the buffer, nothing is written past its end
	whatever the data says.
*/

int MeshDecompressor::decompress( void * _pDest, const void * _pBegin, const void * _pEnd )
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

	auto fail = [&](const char* pMessage, int written)
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, pMessage, this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return written;
	};

	if (mode != Mode::Predicted)
		return fail("Unknown mode of compressed mesh.", 0);

	uint32_t sequences, length;
	p = readCode(p, pEnd, sequences);
	if (p)
		p = readCode(p, pEnd, length);

	if (!p || sequences == 0 || length == 0 || uint64_t(sequences) * length > (uint64_t(1) << 28))
		return fail("Invalid number or length of sequences of compressed mesh.", 0);

	if (pDest <= p && uint64_t(sequences) * length * 4 > uint64_t(pEnd - pDest))
		return fail("Compressed mesh is larger than its buffer.", 0);

	auto pOut = (uint32_t*) pDest;

	for (uint32_t s = 0; s < sequences; s++)
	{
		uint32_t* pValues = pOut + s * length;
		int written = int(s * length * 4);

		uint32_t header;
		p = readCode(p, pEnd, header);
		if (!p || header / 2 >= length)
			return fail("Invalid lag of compressed mesh.", written);

		const int lag = int(header / 2) + 1;
		const int order = int(header % 2) + 1;

		uint32_t i = 0;
		while (i < length)
		{
			uint32_t code;
			p = readCode(p, pEnd, code);
			if (!p)
				return fail("Compressed mesh ends in the middle of a sequence.", written + int(i * 4));

			if (code == 0)
			{
				uint32_t zeros;
				p = readCode(p, pEnd, zeros);
				if (!p || zeros >= length - i)
					return fail("Invalid run of compressed mesh.", written + int(i * 4));

				for (uint32_t end = i + zeros + 1; i < end; i++)
					pValues[i] = predict(pValues, int(i), lag, order);
			}
			else
			{
				pValues[i] = predict(pValues, int(i), lag, order) + unzigzag(code);
				i++;
			}
		}
	}

	return int(sequences * length * 4);
}

} // namespace wax
