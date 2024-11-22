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

#ifndef	WG_STREAMREADER_DOT_H
#define	WG_STREAMREADER_DOT_H
#pragma once

#include <wg_gfxtypes.h>
#include <wg_object.h>
#include <wg_pointers.h>
#include <wg_streamsource.h>
#include <wg_gfxstream.h>

#include <functional>

namespace wg
{

	class StreamReader;
	typedef	StrongPtr<StreamReader>	StreamReader_p;
	typedef	WeakPtr<StreamReader>	StreamReader_wp;

	class StreamReader : public Object, protected StreamSource::Holder
	{
	public:

		//.____ Creation __________________________________________

		static StreamReader_p	create( std::function<int(int nBytes, void * pDest)> dataFetcher );

		//.____ Components _______________________________________

		StreamSource		output;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _____________________________________________________
	
		void		reset();

		inline int	bufferCapacity() const { return m_bufferSize; };
		inline bool	chunkInBuffer() { return _hasStreamChunks(); };
		int			bytesInBuffer() const;

	protected:

		StreamReader(std::function<int(int nBytes, void * pDest)> dataFeeder );
		~StreamReader();

		bool 			_hasStreamChunks() override;
		std::tuple<int, const GfxStream::Data*> _showStreamChunks() override;
		GfxStream::ChunkId 		_peekStreamChunk() override;
		void 			_discardStreamChunks(int bytes) override;
		bool 			_fetchStreamChunks() override;

		void			_fetchData();

		std::function<int(int nBytes, void * pDest)>	m_fetcher;

		static constexpr int c_bufferStartSize = 8192;
		static constexpr int c_bufferMargin = GfxStream::c_maxBlockSize;

		char *		m_pBuffer;
		int			m_bufferSize;
		int			m_readOfs;
		int			m_writeOfs;

		int			m_processedOfs;			// Offset of end of full chunks. 
		int			m_bufferOverflow;		// How much processed data overflows the buffer with in order to hold a
											// complete chunk. Overflowing data is a copy of data from beginning of buffer.

		GfxStream::Data	m_dataSegments[2];
	};



} // namespace wg
#endif //WG_STREAMREADER_DOT_H
#pragma once
