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

#ifndef	WG_STREAMDECODER_DOT_H
#define	WG_STREAMDECODER_DOT_H
#pragma once

#include <wg_gfxstream.h>
#include <wg_gfxtypes.h>
#include <wg_geo.h>
#include <wg_color.h>
#include <wg_gradient.h>
#include <wg_streamsource.h>

#include <cstring>

namespace wg
{
	class StreamDecoder;
	typedef	StrongPtr<StreamDecoder>	StreamDecoder_p;
	typedef	WeakPtr<StreamDecoder>	StreamDecoder_wp;

	//____ StreamDecoder ___________________________________________________

	class StreamDecoder : public Object
	{
	public:

		//.____ Creation __________________________________________

		static StreamDecoder_p	create() { return StreamDecoder_p(new StreamDecoder()); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _______________________________________________________

		void				setInput( const void * pBegin, const void * pEnd);

		bool				isEmpty();
		GfxStream::Header	peek();
		inline int			chunkSize();
		

		void				skip(int bytes);
		void				align();

		//.____ Operators _____________________________________________

		inline StreamDecoder& operator>> (GfxStream::Header& header);
		inline StreamDecoder& operator>> (GfxStream::DataInfo&);

		inline StreamDecoder& operator>> (uint8_t&);
		inline StreamDecoder& operator>> (int16_t&);
		inline StreamDecoder& operator>> (uint16_t&);
		inline StreamDecoder& operator>> (int&);
		inline StreamDecoder& operator>> (float&);
		inline StreamDecoder& operator>> (bool&);

		inline StreamDecoder& operator>> (GfxStream::SPX&);
		inline StreamDecoder& operator>> (CoordI&);
		inline StreamDecoder& operator>> (CoordF&);
		inline StreamDecoder& operator>> (SizeI&);
		inline StreamDecoder& operator>> (SizeF&);
		inline StreamDecoder& operator>> (RectI&);
		inline StreamDecoder& operator>> (RectF&);
		inline StreamDecoder& operator>> (BorderI&);
		inline StreamDecoder& operator>> (Border&);
		inline StreamDecoder& operator>> (Gradient&);

		inline StreamDecoder& operator>> (HiColor&);
		inline StreamDecoder& operator>> (Direction&);
		inline StreamDecoder& operator>> (BlendMode&);
		inline StreamDecoder& operator>> (TintMode&);
		inline StreamDecoder& operator>> (Axis&);
		inline StreamDecoder& operator>> (PixelFormat&);
		inline StreamDecoder& operator>> (SampleMethod&);
		inline StreamDecoder& operator>> (GfxFlip&);
		inline StreamDecoder& operator>> (XSections&);
		inline StreamDecoder& operator>> (YSections&);
		inline StreamDecoder& operator>> (CanvasRef&);
		inline StreamDecoder& operator>> (Compression&);

		inline StreamDecoder& operator>> (const GfxStream::ReadBytes&);
		inline StreamDecoder& operator>> (const GfxStream::ReadSpxField&);

		inline StreamDecoder& operator>> (int[2][2]);
		inline StreamDecoder& operator>> (float[2][2]);


	protected:
		StreamDecoder();
		~StreamDecoder();
		
		inline bool		_hasChunk();
		inline GfxStream::Header	_peekChunk();	// Is only called if _hasChunk() has returned true.

		inline char		_pullChar();
		inline short	_pullShort();
		inline int		_pullInt();
		inline float	_pullFloat();
		inline void		_pullBytes(int nBytes, void* pBytes);
		inline void		_skipBytes(int nBytes);

		GfxStream::SpxFormat	m_spxFormat;

		typedef	const uint8_t* (*SpxOp_p)(const uint8_t* pStream, spx& output);
		typedef	const uint8_t*(*CoordOp_p)(const uint8_t* pStream, CoordI& output);
		typedef	const uint8_t*(*SizeOp_p)(const uint8_t* pStream, SizeI& output);
		typedef	const uint8_t*(*RectOp_p)(const uint8_t* pStream, RectI& output);

		static const SpxOp_p	s_spxOps[4];
		static const CoordOp_p	s_coordOps[4];
		static const SizeOp_p	s_sizeOps[4];
		static const RectOp_p	s_rectOps[4];

		const uint8_t* m_pDataBegin = nullptr;
		const uint8_t* m_pDataEnd = nullptr;
		const uint8_t* m_pDataRead = nullptr;
	};

	StreamDecoder& StreamDecoder::operator>> (GfxStream::Header& header)
	{
		if (_hasChunk())
		{
			header.type = (GfxStream::ChunkId)_pullChar();
			header.format = _pullChar();
			header.size = _pullShort();
		}
		else
		{
			header.type = GfxStream::ChunkId::OutOfData;
			header.format = 0;
			header.size = 0;
		}

		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (GfxStream::DataInfo& info)
	{
		info.unpackedTotalSize = _pullInt();
		info.packedTotalSize = _pullInt();
		info.chunkOffset = _pullInt();
		info.chunkSize = _pullShort();

		uint16_t flagsAndCompression = _pullShort();

		info.bFirstChunk = (flagsAndCompression >> 8) & 0x1;
		info.bLastChunk = (flagsAndCompression >> 9) & 0x1;
		info.compression = (Compression) (flagsAndCompression & 0xFF);
		return *this;
	}


	StreamDecoder& StreamDecoder::operator>> (uint8_t& i)
	{
		i = _pullChar();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (int16_t& i)
	{
		i = _pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (uint16_t& i)
	{
		i = _pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (int& i)
	{
		i = _pullInt();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (float& f)
	{
		f = _pullFloat();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (bool& b)
	{
		int16_t myBool = _pullShort();
		b = (bool)myBool;
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (GfxStream::SPX& value)
	{
		if(m_spxFormat == GfxStream::SpxFormat::Int32_dec )
			value.value = _pullInt();
		else if(m_spxFormat == GfxStream::SpxFormat::Uint16_dec )
			value.value = uint16_t(_pullShort());			// Value is unsigned.
		else if(m_spxFormat == GfxStream::SpxFormat::Int16_int )
			value.value = int(_pullShort()) << 6;
		else if(m_spxFormat == GfxStream::SpxFormat::Uint8_int )
			value.value = int(uint8_t(_pullChar())) << 6;	// Make sure value is unsigned.
		return *this;
	}


	StreamDecoder& StreamDecoder::operator>> (CoordI& coord)
	{
		m_pDataRead = s_coordOps[int(m_spxFormat)](m_pDataRead, coord);
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (CoordF& coord)
	{
		coord.x = _pullFloat();
		coord.y = _pullFloat();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (SizeI& sz)
	{
		m_pDataRead = s_sizeOps[int(m_spxFormat)](m_pDataRead, sz);
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (SizeF& sz)
	{
		sz.w = _pullFloat();
		sz.h = _pullFloat();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (RectI& rect)
	{
		m_pDataRead = s_rectOps[int(m_spxFormat)](m_pDataRead, rect);
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (RectF& rect)
	{
		rect.x = _pullFloat();
		rect.y = _pullFloat();
		rect.w = _pullFloat();
		rect.h = _pullFloat();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (BorderI& border)
	{
		border.top = _pullShort();
		border.right = _pullShort();
		border.bottom = _pullShort();
		border.left = _pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (Border& border)
	{
		border.top = _pullFloat();
		border.right = _pullFloat();
		border.bottom = _pullFloat();
		border.left = _pullFloat();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (Gradient& gradient)
	{
		*this >> gradient.topLeft;
		*this >> gradient.topRight;
		*this >> gradient.bottomRight;
		*this >> gradient.bottomLeft;
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (HiColor& color)
	{
		color.b = _pullShort();
		color.g = _pullShort();
		color.r = _pullShort();
		color.a = _pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (Direction& dir)
	{
		dir = (Direction)_pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (BlendMode& blendMode)
	{
		blendMode = (BlendMode)_pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (TintMode& tintMode)
	{
		tintMode = (TintMode)_pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (Axis& o)
	{
		o = (Axis)_pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (PixelFormat& t)
	{
		t = (PixelFormat)_pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (SampleMethod& m)
	{
		m = (SampleMethod)_pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (GfxFlip& f)
	{
		f = (GfxFlip)_pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (XSections& x)
	{
		x = (XSections)_pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (YSections& y)
	{
		y = (YSections)_pullShort();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (CanvasRef& r)
	{
		r = (CanvasRef)_pullChar();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (Compression& c)
	{
		c = (Compression)_pullShort();
		return *this;
	}


	StreamDecoder& StreamDecoder::operator>> (const GfxStream::ReadBytes& data)
	{
		_pullBytes(data.bytes, data.pBuffer);
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (const GfxStream::ReadSpxField& field)
	{
		switch( field.spxFormat )
		{
			case GfxStream::SpxFormat::Int32_dec:
				_pullBytes(field.size*4, field.pField);
				break;
			
			case GfxStream::SpxFormat::Int16_int:
			{
				auto p = field.pField;
				for( int i = 0 ; i < field.size ; i++ )
					* p++ = int(_pullShort()) << 6;
				break;
			}

			case GfxStream::SpxFormat::Uint16_dec:
			{
				auto p = field.pField;
				for( int i = 0 ; i < field.size ; i++ )
					* p++ = (unsigned short) _pullShort();
				break;
			}
				
			case GfxStream::SpxFormat::Uint8_int:
			{
				auto p = field.pField;
				for( int i = 0 ; i < field.size ; i++ )
					* p++ = int((unsigned char) _pullChar()) << 6;
				break;
			}
		}
		return *this;
	}


	StreamDecoder& StreamDecoder::operator>> (int mtx[2][2])
	{
		mtx[0][0] = _pullChar();
		mtx[0][1] = _pullChar();
		mtx[1][0] = _pullChar();
		mtx[1][1] = _pullChar();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (float mtx[2][2])
	{
		mtx[0][0] = _pullFloat();
		mtx[0][1] = _pullFloat();
		mtx[1][0] = _pullFloat();
		mtx[1][1] = _pullFloat();
		return *this;
	}


	//____ chunkSize() ______________________________________________________

	int StreamDecoder::chunkSize()
	{
		return (* (uint16_t*) &m_pDataRead[2]) + 4;
	}

	//____ _hasChunk() ______________________________________________________

	bool StreamDecoder::_hasChunk()
	{
		if (m_pDataRead != m_pDataEnd)
			return true;

		return false;
	}

	//____ _peekChunk() ______________________________________________________

	GfxStream::Header StreamDecoder::_peekChunk()
	{
		GfxStream::Header header;

		header.type = (GfxStream::ChunkId) m_pDataRead[0];
		header.format = m_pDataRead[1];
		header.size = * (uint16_t*)(&m_pDataRead[2]);
		return header;
	}


	//____ _pullChar() ______________________________________________________

	char StreamDecoder::_pullChar()
	{
		char x = *(char*)m_pDataRead;
		m_pDataRead++;
		return x;
	}

	//____ _pullShort() ______________________________________________________

	short StreamDecoder::_pullShort()
	{
		short x = *(short*)m_pDataRead;
		m_pDataRead += 2;
		return x;
	}

	//____ _pullInt() ______________________________________________________

	int StreamDecoder::_pullInt()
	{
		int x = *(uint16_t*)m_pDataRead;
		m_pDataRead += 2;
		x += (*(uint16_t*)m_pDataRead << 16);
		m_pDataRead += 2;

		return x;
	}

	//____ _pullFloat() ______________________________________________________

	float StreamDecoder::_pullFloat()
	{
		int x = *(uint16_t*)m_pDataRead;
		m_pDataRead += 2;
		x += (*(uint16_t*)m_pDataRead << 16);
		m_pDataRead += 2;

		return *((float*)&x);
	}

	//____ _pullBytes() ______________________________________________________

	void StreamDecoder::_pullBytes(int nBytes, void* pBytes)
	{
		std::memcpy(pBytes, m_pDataRead, nBytes);
		m_pDataRead += nBytes;
	}

	//____ _skipBytes() ______________________________________________________

	void StreamDecoder::_skipBytes(int nBytes)
	{
		m_pDataRead += nBytes;
	}




}



#endif // WG_STREAMDecoder_DOT_H
