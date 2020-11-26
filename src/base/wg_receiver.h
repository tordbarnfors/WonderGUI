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

#ifndef	WG_RECEIVER_DOT_H
#define	WG_RECEIVER_DOT_H
#pragma once

#include <wg_pointers.h>

namespace wg
{

	class Msg;
	typedef	StrongPtr<Msg>			Msg_p;

	class Receiver;
	typedef	StrongPtr<Receiver>	Receiver_p;
	typedef	WeakPtr<Receiver>		Receiver_wp;

	class Receiver : public Object
	{
		friend class MsgRouter;
		friend class Base;
	public:

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc ____________________________________________________________

		virtual void 	receive( Msg * pMsg );


	protected:
		Receiver() {};

		virtual void	_update(int microPassed, int64_t microsecTimestamp);

		virtual void	_onRouteAdded();
		virtual void	_onRouteRemoved();
	};



} // namespace wg
#endif //WG_RECEIVER_DOT_H
