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

#include <wg_streamwriter.h>
#include <assert.h>

namespace wg
{

	const TypeInfo StreamWriter::TYPEINFO = { "StreamWriter", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	StreamWriter_p StreamWriter::create(std::function<void(int nBytes, const void * pData)> dispatcher )
	{
		return new StreamWriter(dispatcher);
	}

	//____ constructor _____________________________________________________________

	StreamWriter::StreamWriter(std::function<void(int nBytes, const void * pData)> dispatcher) : input(this)
	{
		m_dispatcher = dispatcher;
	}

	//____ Destructor _________________________________________________________

	StreamWriter::~StreamWriter()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StreamWriter::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _processStreamChunks() _____________________________________________

	void StreamWriter::_processStreamChunks(const uint8_t* pBegin, const uint8_t* pEnd)
	{
		m_dispatcher(int(pEnd - pBegin), pBegin);
	}


} // namespace wg
