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
#ifndef	WG_STREAMDECODER_DOT_H
#define	WG_STREAMDECODER_DOT_H
#pragma once

#include <wg_gfxstream.h>
#include <wg_gfxtypes.h>
#include <wg_geo.h>
#include <wg_color.h>
#include <wg_streamsource.h>
#include <wg_gfxutil.h>

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

		void				setVersion(uint16_t version);
		void				setInput( const void * pBegin, const void * pEnd);

		//.____ Misc ________________________________________________________________

		bool				isEmpty();
		GfxStream::Header	peek();
		inline int			chunkSize();

		void				skip(int bytes);
		void				align();

		const uint8_t *		readPtr() const { return m_pDataRead; }

//		inline int			dataInfoSize() const { return m_dataInfoSize; }


		//.____ Operators _____________________________________________

		inline StreamDecoder& operator>> (GfxStream::Header& header);
		inline StreamDecoder& operator>> (GfxStream::DataInfo&);

		inline StreamDecoder& operator>> (uint8_t&);
		inline StreamDecoder& operator>> (int16_t&);
		inline StreamDecoder& operator>> (uint16_t&);
		inline StreamDecoder& operator>> (int&);
		inline StreamDecoder& operator>> (uint32_t&);
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

		uint16_t		m_version;
		int				m_dataInfoSize = 20;

		const uint8_t* m_pDataBegin = nullptr;
		const uint8_t* m_pDataEnd = nullptr;
		const uint8_t* m_pDataRead = nullptr;
	};

	StreamDecoder& StreamDecoder::operator>> (GfxStream::Header& header)
	{
		if (_hasChunk())
		{
			header.type = (GfxStream::ChunkId)_pullChar();
			header.dummy = _pullChar();
			header.size = _pullShort();
		}
		else
		{
			header.type = GfxStream::ChunkId::OutOfData;
			header.dummy = 0;
			header.size = 0;
		}

		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (GfxStream::DataInfo& info)
	{
		info = GfxStream::readDataInfo(m_pDataRead);
		m_pDataRead += info.encodedSize;
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

	StreamDecoder& StreamDecoder::operator>> (uint32_t& i)
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
		value.value = _pullInt();
		return *this;
	}


	StreamDecoder& StreamDecoder::operator>> (CoordI& coord)
	{
		uint16_t* p = (uint16_t*)m_pDataRead;

		coord.x = p[0] + (int(p[1]) << 16);
		coord.y = p[2] + (int(p[3]) << 16);

		m_pDataRead += 8;
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (CoordF& coord)
	{
		coord.x = _pullFloat();
		coord.y = _pullFloat();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (SizeI& size)
	{
		uint16_t* p = (uint16_t*)m_pDataRead;

		size.w = p[0] + (int(p[1]) << 16);
		size.h = p[2] + (int(p[3]) << 16);

		m_pDataRead += 8;
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (SizeF& size)
	{
		size.w = _pullFloat();
		size.h = _pullFloat();
		return *this;
	}

	StreamDecoder& StreamDecoder::operator>> (RectI& rect)
	{
		uint16_t* p = (uint16_t*)m_pDataRead;

		rect.x = p[0] + (int(p[1]) << 16);
		rect.y = p[2] + (int(p[3]) << 16);
		rect.w = p[4] + (int(p[5]) << 16);
		rect.h = p[6] + (int(p[7]) << 16);
		m_pDataRead += 16;
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
		header.dummy = m_pDataRead[1];
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
