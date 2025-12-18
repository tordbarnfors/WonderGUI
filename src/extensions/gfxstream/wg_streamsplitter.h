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
#ifndef	WG_STREAMSPLITTER_DOT_H
#define	WG_STREAMSPLITTER_DOT_H
#pragma once

#include <wg_streamsink.h>

#include <vector>

namespace wg
{

	class StreamSplitter;
	typedef	StrongPtr<StreamSplitter>	StreamSplitter_p;
	typedef	WeakPtr<StreamSplitter>	StreamSplitter_wp;

	class StreamSplitter : public Object, protected StreamSink::Holder
	{
	public:

		//.____ Creation __________________________________________

		static StreamSplitter_p	create();
		static StreamSplitter_p	create( const std::initializer_list<StreamSink_p>& outputs );

		//.____ Components _______________________________________

		StreamSink		input;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ________________________________________________
		
		void	clearOutputs();
		void	addOutput( const StreamSink_p& pOutput );
		void	removeOutput(const StreamSink_p& pOutput);
		
		
	protected:

		StreamSplitter();
		StreamSplitter(const std::initializer_list<StreamSink_p>& outputs);
		~StreamSplitter();

		void	_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd) override;

		std::vector<StreamSink_p> m_outputs;
	};


} // namespace wg
#endif //WG_STREAMSPLITTER_DOT_H
