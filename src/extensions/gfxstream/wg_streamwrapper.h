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

#ifndef	WG_STREAMWRAPPER_DOT_H
#define	WG_STREAMWRAPPER_DOT_H
#pragma once

#include <wg_gfxtypes.h>
#include <wg_object.h>
#include <wg_pointers.h>
#include <wg_streamsource.h>
#include <wg_gfxstream.h>

#include <functional>

namespace wg
{

	class StreamWrapper;
	typedef	StrongPtr<StreamWrapper>	StreamWrapper_p;
	typedef	WeakPtr<StreamWrapper>	StreamWrapper_wp;

	class StreamWrapper : public Object, protected StreamSource::Holder
	{
	public:

		//.____ Creation __________________________________________

		static StreamWrapper_p	create( const void * pBegin, const void * pEnd );
		static StreamWrapper_p	create( GfxStream::iterator begin, GfxStream::iterator end );

		//.____ Components _______________________________________

		StreamSource		output;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _____________________________________________________
		
		inline bool	chunkInBuffer() { return _hasStreamChunks(); };
		int			bytesInBuffer() const;

		void		restart() { m_dataSegment.pBegin = m_pBegin; }

	protected:

		StreamWrapper( const void * pBegin, const void * pEnd );
		~StreamWrapper();

		bool 			_hasStreamChunks() override;
		std::tuple<int, const GfxStream::Data*> _showStreamChunks() override;
		GfxStream::ChunkId 		_peekStreamChunk() override;
		void 			_discardStreamChunks(int bytes) override;
		bool 			_fetchStreamChunks() override;

	
		uint8_t *	m_pBegin;
		uint8_t *	m_pEnd;

		GfxStream::Data	m_dataSegment;
	};



} // namespace wg
#endif //WG_STREAMWRAPPER_DOT_H
#pragma once
