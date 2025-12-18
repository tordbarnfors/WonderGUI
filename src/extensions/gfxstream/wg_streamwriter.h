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
#ifndef	WG_STREAMWRITER_DOT_H
#define	WG_STREAMWRITER_DOT_H
#pragma once

#include <wg_streamsink.h>

#include <functional>

namespace wg
{

	class StreamWriter;
	typedef	StrongPtr<StreamWriter>	StreamWriter_p;
	typedef	WeakPtr<StreamWriter>	StreamWriter_wp;

	class StreamWriter : public Object, protected StreamSink::Holder
	{
	public:

		//.____ Creation __________________________________________

		static StreamWriter_p	create( std::function<void(int nBytes, const void * pData)> dispatcher );

		//.____ Components _______________________________________

		StreamSink		input;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:

		StreamWriter(std::function<void(int nBytes, const void * pData)> packageDispatcher );
		~StreamWriter();

		void	_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd) override;

		std::function<void(int nBytes, const void * pData)>	m_dispatcher;
	};


} // namespace wg
#endif //WG_STREAMWRITER_DOT_H
