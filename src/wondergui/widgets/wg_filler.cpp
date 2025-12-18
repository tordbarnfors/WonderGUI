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
#include <wg_filler.h>
#include <wg_gfxdevice.h>

namespace wg
{
	using namespace Util;

	const TypeInfo Filler::TYPEINFO = { "Filler", &Widget::TYPEINFO };

	//____ constructor ____________________________________________________________

	Filler::Filler()
	{
		m_defaultSize = Size(-1,-1);
	}

	//____ destructor _____________________________________________________________

	Filler::~Filler()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Filler::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setDefaultSize() _______________________________________________________

	void Filler::setDefaultSize( const Size& size, CoordTransition* pTransition )
	{
		if( size == m_defaultSize )
			return;

		if( pTransition == nullptr )
		{
			m_defaultSize = size;
			_requestResize();

			if( m_pTransition )
			{
				_stopReceiveUpdates();
				m_pTransition = nullptr;
			}
		}
		else
		{
			if( m_pTransition == nullptr )
				_startReceiveUpdates();

			m_pTransition = pTransition;
			m_transitionProgress = 0;
			m_startSize = (m_defaultSize.w >= 0 && m_defaultSize.h >= 0) ? m_defaultSize : Util::spxToPts(Widget::_defaultSize(m_scale),m_scale);
			m_endSize = (size.w >= 0 && size.h >= 0) ? size : Util::spxToPts(Widget::_defaultSize(m_scale),m_scale);
		}
	}

	//____ defaultSize() __________________________________________________________

	SizeSPX Filler::_defaultSize(int scale) const
	{
		if (m_defaultSize.w >= 0 && m_defaultSize.h >= 0)
		{
			return Util::align(SizeSPX(m_defaultSize*scale));
		}
		else
			return Widget::_defaultSize(scale);
	}

	//____ _update() _____________________________________________________________

	void Filler::_update(int microPassed, int64_t microsecTimestamp)
	{
		if( m_pTransition == nullptr )
			return;					// We assume a subclass is receiving updates.

		m_transitionProgress += microPassed;

		if( m_transitionProgress >= m_pTransition->duration())
		{
			m_transitionProgress = m_pTransition->duration();
			_stopReceiveUpdates();
			m_pTransition = nullptr;

			if( m_defaultSize != m_endSize )
			{
				m_defaultSize = m_endSize;
				_requestResize();
			}
		}
		else
		{
			Coord c = m_pTransition->snapshot(m_transitionProgress, Coord{m_startSize.w,m_startSize.h}, Coord{m_endSize.w,m_endSize.h});

			Size newSize(c.x,c.y);

			if( m_defaultSize != newSize )
			{
				m_defaultSize = newSize;
				_requestResize();
			}
		}
	}


} // namespace wg
