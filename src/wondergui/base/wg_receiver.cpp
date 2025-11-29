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
#include <wg_receiver.h>

namespace wg
{

	const TypeInfo Receiver::TYPEINFO = { "Receiver", &Object::TYPEINFO };



	//____ typeInfo() _________________________________________________________

	const TypeInfo& Receiver::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ receive() __________________________________________________________

	void Receiver::receive(Msg* pMsg)
	{

	}

	//____ _update() __________________________________________________________

	void Receiver::_update(int microPassed, int64_t microsecTimestamp)
	{
	}

	//____ _onRouteAdded() _________________________________________________________

	void  Receiver::_onRouteAdded()
	{
	}

	//____ _onRouteRemoved() _______________________________________________________

	void  Receiver::_onRouteRemoved()
	{
	}

} // namespace wg
