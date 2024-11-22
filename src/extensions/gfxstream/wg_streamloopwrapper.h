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

#ifndef	WG_STREAMLOOPWRAPPER_DOT_H
#define	WG_STREAMLOOPWRAPPER_DOT_H
#pragma once

#include <wg_gfxtypes.h>
#include <wg_object.h>
#include <wg_pointers.h>
#include <wg_streamsource.h>
#include <wg_gfxstream.h>

#include <functional>

namespace wg
{

	class StreamLoopWrapper;
	typedef	StrongPtr<StreamLoopWrapper>	StreamLoopWrapper_p;
	typedef	WeakPtr<StreamLoopWrapper>	StreamLoopWrapper_wp;

	class StreamLoopWrapper : public Object, protected StreamSource::Holder
	{
	public:

		//.____ Creation __________________________________________

		static StreamLoopWrapper_p	create( const void * pBufferBegin, const void * pBufferEnd, 
												std::function<const void *()> getWritePtrFunc,
												std::function<void(const void * pReadPos)> setReadPtrFunc );

		//.____ Components _______________________________________

		StreamSource		output;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _____________________________________________________

	protected:

		StreamLoopWrapper(const void * pBufferBegin, const void * pBufferEnd,
							 std::function<const void *()> getWritePtrFunc,
							 std::function<void(const void * pReadPos)> setReadPtrFunc );

		~StreamLoopWrapper();

		bool 			_hasStreamChunks() override;
		std::tuple<int, const GfxStream::Data*> _showStreamChunks() override;
		GfxStream::ChunkId 		_peekStreamChunk() override;
		void 			_discardStreamChunks(int bytes) override;
		bool 			_fetchStreamChunks() override;
		
		const uint8_t * m_pBufferBegin;
		const uint8_t * m_pBufferEnd;

		std::function<const void *()> 				m_getWritePtrFunc;
		std::function<void(const void * pReadPos)> 	m_setReadPtrFunc;

		uint8_t	m_rejoinedChunk[GfxStream::c_maxBlockSize];
		int		m_rejoinedChunkSize = 0;
		
		const uint8_t * m_pBeginChunks = nullptr;
		const uint8_t * m_pSplitChunkBegin = nullptr;
		const uint8_t * m_pSplitChunkEnd = nullptr;
		const uint8_t * m_pEndChunks = nullptr;

		GfxStream::Data	m_dataSegments[3];

	};



} // namespace wg
#endif //WG_STREAMLOOPWRAPPER_DOT_H

