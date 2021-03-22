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

#ifndef WG_SCROLLBARTARGET_DOT_H
#define WG_SCROLLBARTARGET_DOT_H
#pragma once

#include <wg_pointers.h>
#include <wg_widget.h>

namespace wg
{


	class ScrollbarTarget
	{
		friend class Scrollbar;

	protected:

		// Calls for Scrollbar

		virtual float	_stepFwd() = 0;
		virtual float	_stepBwd() = 0;
		virtual float	_jumpFwd() = 0;
		virtual float	_jumpBwd() = 0;

		virtual float	_wheelRolled( int distance ) = 0;

		virtual float	_setPosition( float fraction ) = 0;

		virtual Widget*_getWidget() = 0;
		virtual float	_getHandlePosition() = 0;
		virtual float	_getHandleSize() = 0;

		// Calls for Target

		void			_updateScrollbar( float pos, float size );

		//

		Widget_wp	m_pScrollbar;
	};




} // namespace wg
#endif

