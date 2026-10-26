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
#ifndef	WAX_STREAMSINK_DOT_H
#define	WAX_STREAMSINK_DOT_H
#pragma once

#include <wax_pointers.h>
#include <wax_gfxstream.h>



namespace wax
{

	class StreamSink;
	typedef	ComponentPtr<StreamSink>	StreamSink_p;


	//____ StreamSink __________________________________________________________

	class StreamSink : public GfxStream
	{
	public:

		//____ Holder ___________________________________________________

		class Holder /** @private */
		{
		public:

			virtual void	_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd) = 0;
		};

		StreamSink(Holder * pHolder) : m_pHolder(pHolder) {}

		//.____ Control _______________________________________________________

		inline void	processChunks(const void* pBegin, const void* pEnd) { m_pHolder->_processStreamChunks( (uint8_t*) pBegin, (uint8_t*) pEnd); }

	protected:

		Holder * 	m_pHolder;
	};


} // namespace wax
#endif //WAX_STREAMSINK_DOT_H
