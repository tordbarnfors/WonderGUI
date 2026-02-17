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
#include <cstring>
#include <algorithm>

#include <wg_streammerger.h>
#include <assert.h>

namespace wg
{

	const TypeInfo StreamMerger::TYPEINFO = { "StreamMerger", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	StreamMerger_p StreamMerger::create()
	{
		return new StreamMerger();
	}

	StreamMerger_p StreamMerger::create(const std::initializer_list<StreamSource_p>& inputs)
	{
		return new StreamMerger(inputs);
	}

	//____ constructor _____________________________________________________________

	StreamMerger::StreamMerger() : output(this)
	{
	}

	StreamMerger::StreamMerger(const std::initializer_list<StreamSource_p>& inputs) : output(this)
	{
		m_inputs.resize(inputs.size());

		int i = 0;
		for( auto& input : inputs )
			m_inputs[i].pSource = input;
	}

	//____ Destructor _________________________________________________________

	StreamMerger::~StreamMerger()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamMerger::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ clearInputs() ________________________________________________________

	void StreamMerger::clearInputs()
	{
		m_inputs.clear();
	}

	//____ addInput() ___________________________________________________________

	void StreamMerger::addInput( const StreamSource_p& pInput )
	{
		m_inputs.push_back( {pInput,0} );
	}

	//____ removeInput() ________________________________________________________

	void StreamMerger::removeInput(const StreamSource_p& pInput)
	{
		auto newEnd = std::remove_if( m_inputs.begin(), m_inputs.end(), [&pInput](Input& input) { return input.pSource == pInput; } );
		m_inputs.erase(newEnd, m_inputs.end());
	}

	//____ _hasStreamChunks() ____________________________________________________

	bool StreamMerger::_hasStreamChunks()
	{
		for( auto& input : m_inputs )
		{
			if( input.pSource->hasChunks() )
				return true;
		}

		return false;
	}

	//____ _showStreamChunks() ___________________________________________________

	std::tuple<int, const GfxStream::Data *> StreamMerger::_showStreamChunks()
	{
		m_combinedChunks.clear();

		for( auto& input : m_inputs )
		{
			int nbEntries;
			const GfxStream::Data * pEntries;
			std::tie( nbEntries, pEntries ) = input.pSource->showChunks();

			int nBytes = 0;
			for( int i = 0 ; i < nbEntries ; i++ )
			{
				nBytes += pEntries[i].pEnd - pEntries[i].pBegin;
				m_combinedChunks.push_back(pEntries[i]);
			}
			input.bytesShown = nBytes;
		}

		return std::make_tuple(m_combinedChunks.size(), &m_combinedChunks.front() );
	}

	//____ _discardStreamChunks() ________________________________________________

	void StreamMerger::_discardStreamChunks(int bytes)
	{
		for( auto& input : m_inputs )
		{
			int toDiscard = std::min(bytes,input.bytesShown);
			input.pSource->discardChunks(toDiscard);
			bytes -= toDiscard;
			if( bytes == 0 )
				break;
		}
	}

	//____ _fetchStreamChunks() __________________________________________________

	bool StreamMerger::_fetchStreamChunks()
	{
		bool result = false;

		for( auto& input : m_inputs )
			result = input.pSource->fetchChunks() ? true : result;

		return result;
	}

	//____ _peekStreamChunks() ___________________________________________________

	GfxStream::ChunkId StreamMerger::_peekStreamChunk()
	{
		for( auto& input : m_inputs )
		{
			auto id = input.pSource->peekChunk();
			if( id != GfxStream::ChunkId::OutOfData )
				return id;
		}

		return GfxStream::ChunkId::OutOfData;
	}

} // namespace wg
