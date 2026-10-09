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
#ifndef	WG_DEBUGCONTEXTSWITCH_DOT_H
#define WG_DEBUGCONTEXTSWITCH_DOT_H
#pragma once

#include <wg_base.h>

namespace wg
{
	//____ DebugContextSwitch ___________________________________________________
	//
	// Makes a GUI context current for as long as it lives, then restores the
	// previous one. Does nothing if the context is gone or already current.
	//
	// The debugger can serve windows that each have their own GUI context, so
	// its code switches to the right one before building widgets or adding
	// routes.

	class DebugContextSwitch
	{
	public:
		DebugContextSwitch( const GUIContext_p& pContext )
		{
			if( pContext && pContext != Base::context() )
				m_pPrevious = Base::setContext(pContext);
		}

		~DebugContextSwitch()
		{
			if( m_pPrevious )
				Base::setContext(m_pPrevious);
		}

	private:
		GUIContext_p	m_pPrevious;
	};

} // namespace wg
#endif //WG_DEBUGCONTEXTSWITCH_DOT_H
