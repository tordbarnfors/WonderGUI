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
#ifndef	WG_STREAMMERGER_DOT_H
#define	WG_STREAMMERGER_DOT_H
#pragma once

#include <wg_streamsource.h>

#include <vector>

namespace wg
{

	class StreamMerger;
	typedef	StrongPtr<StreamMerger>	StreamMerger_p;
	typedef	WeakPtr<StreamMerger>	StreamMerger_wp;

	class StreamMerger : public Object, protected StreamSource::Holder
	{
	public:

		//.____ Creation __________________________________________

		static StreamMerger_p	create();
		static StreamMerger_p	create( const std::initializer_list<StreamSource_p>& inputs );

		//.____ Components _______________________________________

		StreamSource	output;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ________________________________________________
		
		void	clearInputs();
		void	addInput( const StreamSource_p& pInput );
		void	removeInput(const StreamSource_p& pInput);

		
	protected:

		StreamMerger();
		StreamMerger(const std::initializer_list<StreamSource_p>& inputs);
		~StreamMerger();

		bool				_hasStreamChunks() override;
		std::tuple<int, const GfxStream::Data *> _showStreamChunks() override;
		void				_discardStreamChunks(int bytes) override;
		bool				_fetchStreamChunks() override;
		GfxStream::ChunkId 	_peekStreamChunk() override;

		struct Input
		{
			StreamSource_p	pSource;
			int				bytesShown = 0;
		};

		std::vector<Input> 				m_inputs;
		std::vector<GfxStream::Data>	m_combinedChunks;

	};


} // namespace wg
#endif //WG_STREAMMERGER_DOT_H
