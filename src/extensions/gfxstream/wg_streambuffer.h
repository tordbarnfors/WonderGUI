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

#ifndef	WG_STREAMBUFFER_DOT_H
#define	WG_STREAMBUFFER_DOT_H
#pragma once

#include <wg_gfxstream.h>
#include <wg_streamsource.h>
#include <wg_streamsink.h>

#include <vector>
#include <tuple>

namespace wg
{

	class StreamBuffer;
	typedef	StrongPtr<StreamBuffer>	StreamBuffer_p;
	typedef	WeakPtr<StreamBuffer>	StreamBuffer_wp;

	class StreamBuffer : public Object, protected StreamSink::Holder, protected StreamSource::Holder
	{
	public:

		//.____ Creation __________________________________________

		static StreamBuffer_p	create( int initialCapacity = GfxStream::c_maxBlockSize * 4 );

		//.____ Components _______________________________________

		StreamSink		input;
		StreamSource		output;

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _____________________________________________________
		
		void		reset( int newBufferSize = 0 );
		
		inline int	capacity() const { return m_bufferSize-2; };
		inline bool	hasChunk() { return _hasStreamChunks(); };
		int			size() const;
		
		int			setCapacity( int capacity );
		
		
		
		
	protected:

		StreamBuffer( int initialCapacity );
		~StreamBuffer();

		void			_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd) override;

		bool			_hasStreamChunks() override;
		std::tuple<int, const GfxStream::Data*> _showStreamChunks() override;
		GfxStream::ChunkId 		_peekStreamChunk() override;
		void			_discardStreamChunks(int bytes) override;
		bool			_fetchStreamChunks() override;

		void			_resizeBuffer( int newSize );

		static constexpr int c_bufferMargin = GfxStream::c_maxBlockSize;

		char*		m_pBuffer;
		int			m_bufferSize;
		int			m_readOfs;
		int			m_writeOfs;

		int			m_processedOfs;			// Offset of end of full chunks. 
		int			m_bufferOverflow;		// How much processed data overflows the buffer with in order to hold a
											// complete chunk. Overflowing data is a copy of data from beginning of buffer.

		GfxStream::Data	m_dataSegments[2];
		
	};


} // namespace wg
#endif //WG_STREAMBUFFER_DOT_H
