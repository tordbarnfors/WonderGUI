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

#include <wg_streamsplitter.h>
#include <assert.h>

namespace wg
{

	const TypeInfo StreamSplitter::TYPEINFO = { "StreamSplitter", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	StreamSplitter_p StreamSplitter::create()
	{
		return new StreamSplitter();
	}

	StreamSplitter_p StreamSplitter::create(const std::initializer_list<StreamSink_p>& outputs)
	{
		return new StreamSplitter(outputs);
	}

	//____ constructor _____________________________________________________________

	StreamSplitter::StreamSplitter() : input(this)
	{
	}

	StreamSplitter::StreamSplitter(const std::initializer_list<StreamSink_p>& outputs) : input(this)
	{
		m_outputs = outputs;
	}

	//____ Destructor _________________________________________________________

	StreamSplitter::~StreamSplitter()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamSplitter::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ clearOutputs() ________________________________________________________

	void StreamSplitter::clearOutputs()
	{
		m_outputs.clear();
	}

	//____ addOutput() ___________________________________________________________

	void StreamSplitter::addOutput( const StreamSink_p& pOutput )
	{
		m_outputs.push_back(pOutput);
	}

	//____ removeOutput() ________________________________________________________

	void StreamSplitter::removeOutput(const StreamSink_p& pOutput)
	{
		auto newEnd = std::remove( m_outputs.begin(), m_outputs.end(), pOutput );
		m_outputs.erase(newEnd, m_outputs.end());
	}

	//____ _processStreamChunks() _____________________________________________

	void StreamSplitter::_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd)
	{
		for (auto& pOutput : m_outputs)
			pOutput->processChunks(pBegin, pEnd);
	}


} // namespace wg
