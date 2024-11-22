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

#ifndef	WG_STREAMSOURCE_DOT_H
#define	WG_STREAMSOURCE_DOT_H
#pragma once

#include <wg_gfxtypes.h>
#include <wg_pointers.h>
#include <wg_gfxstream.h>

#include <tuple>

namespace wg
{

	class StreamSource;
	typedef	ComponentPtr<StreamSource>	StreamSource_p;


	//____ StreamSource ________________________________________________________

	class StreamSource
	{
	public:

		//____ Holder ___________________________________________________

		class Holder /** @private */
		{
		public:

			virtual bool _hasStreamChunks() = 0;
			virtual std::tuple<int, const GfxStream::Data *> _showStreamChunks() = 0;
			virtual	void _discardStreamChunks(int bytes) = 0;
			virtual bool _fetchStreamChunks() = 0;
			virtual GfxStream::ChunkId _peekStreamChunk() = 0;
		};


		StreamSource(Holder * pHolder) : m_pHolder(pHolder) {};

		//.____ Control _______________________________________________________

		inline bool hasChunks() { return m_pHolder->_hasStreamChunks(); }
		inline std::tuple<int, const GfxStream::Data*> showChunks() { return m_pHolder->_showStreamChunks(); }
		inline void discardChunks(int bytes) { return m_pHolder->_discardStreamChunks(bytes); }
		inline bool fetchChunks() { return m_pHolder->_fetchStreamChunks(); }
		inline GfxStream::ChunkId peekChunk() { return m_pHolder->_peekStreamChunk(); }

	protected:

		Holder * 			m_pHolder;
	};


} // namespace wg
#endif //WG_STREAMSOURCE_DOT_H
#pragma once
