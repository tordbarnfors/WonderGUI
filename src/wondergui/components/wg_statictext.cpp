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

#include <wg_statictext.h>
#include <wg_msg.h>
#include <wg_msgrouter.h>
#include <wg_util.h>

namespace wg
{
	using namespace Util;

	const TypeInfo	StaticText::TYPEINFO = { "StaticText", &TextBase::TYPEINFO };

	//____ constructor _____________________________________________________________

	StaticText::StaticText(Widget * pWidget ) : TextBase( pWidget )
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticText::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ markedLink() _____________________________________________________

	TextLink_p StaticText::markedLink() const
	{
		//TODO: Implement!

		return TextLink_p();
	}

	//____ _initFromBlueprint() ________________________________________________

	void StaticText::_initFromBlueprint(const Blueprint& bp)
	{
		if (bp.style)
			setStyle(bp.style);

		if (bp.layout)
			setLayout(bp.layout);
	}

	//____ _receive() ___________________________________________________________

	void StaticText::_receive( Msg * _pMsg )
	{
		if( !_pMsg->isMouseMsg() )
			return;

		auto pMsg = static_cast<InputMsg*>( _pMsg );

		switch( pMsg->type() )
		{
			case MsgType::MouseEnter:
			case MsgType::MouseMove:
			{
				TextLink_p pLink;

				// Get link from character properties

				CoordSPX localPos = _toLocal(static_cast<InputMsg*>( pMsg )->pointerSpxPos());
				int markedChar = _layout()->charAtPos(this, localPos );
				if( markedChar >= 0 )
				{
					TextStyle_p pStyle = m_charBuffer.chars()[markedChar].stylePtr();
					if( pStyle )
						pLink = pStyle->link();
				}

				// Fall back to parts default style

				if( !pLink && m_pStyle )
					pLink = m_pStyle->link();


				// Post messages if there was a change in what link we are pointing at

				if( pLink != m_pMarkedLink )
				{
					MsgRouter_p	pRouter = Base::msgRouter();

					if( m_pMarkedLink )
						pRouter->post( MouseLeaveMsg::create( pMsg->inputId(), m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp() ) );

					if( pLink )
						pRouter->post( MouseEnterMsg::create( pMsg->inputId(), pLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp() ) );

					m_pMarkedLink = pLink;
				}

				break;
			}
			case MsgType::MouseLeave:

				if( m_pMarkedLink )
				{
					Base::msgRouter()->post( MouseLeaveMsg::create( pMsg->inputId(), m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp() ) );
					m_pMarkedLink = 0;
				}
				break;

			case MsgType::MousePress:
				if( m_pMarkedLink )
				{
					MouseButton button = static_cast<MousePressMsg*>(pMsg)->button();
					Base::msgRouter()->post( MousePressMsg::create( pMsg->inputId(), button, m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp() ) );

					if( button == MouseButton::Left )
						pMsg->swallow();
				}
				break;

			case MsgType::MouseDrag:

				//TODO: Add swallow under right circumstances.
				break;

			case MsgType::MouseRepeat:
				if( m_pMarkedLink )
				{
					MouseButton button = static_cast<MouseRepeatMsg*>(pMsg)->button();
					Base::msgRouter()->post( MouseRepeatMsg::create( pMsg->inputId(), button, m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp() ) );

					if( button == MouseButton::Left )
						pMsg->swallow();
				}
				break;

			case MsgType::MouseRelease:
				//TODO: Should only post if press was inside.
				if( m_pMarkedLink )
				{
					MouseButton button = static_cast<MouseReleaseMsg*>(pMsg)->button();
					bool bReleasedInside = static_cast<MouseReleaseMsg*>(pMsg)->releaseInside();
					Base::msgRouter()->post( MouseReleaseMsg::create( pMsg->inputId(), button, m_pMarkedLink, bReleasedInside, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp() ) );

					if( button == MouseButton::Left )
						pMsg->swallow();
				}
				break;

			case MsgType::MouseClick:
				//TODO: Doesn't check if we stay on link during whole click.
				if( m_pMarkedLink )
				{
					auto pClickMsg = static_cast<MouseClickMsg*>(pMsg);

					MouseButton button = pClickMsg->button();
					Base::msgRouter()->post( MouseClickMsg::create(pMsg->inputId(), button, m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp(), pClickMsg->duration(), pClickMsg->ordinal() ) );

					if( button == MouseButton::Left )
					{
						Base::msgRouter()->post( SelectMsg::create( m_pMarkedLink ) );
						pMsg->swallow();
					}
				}
				break;

			case MsgType::MouseDoubleClick:
				//TODO: Doesn't check if we stay on link during whole double click.
				if( m_pMarkedLink )
				{
					MouseButton button = static_cast<MouseDoubleClickMsg*>(pMsg)->button();
					Base::msgRouter()->post( MouseDoubleClickMsg::create(pMsg->inputId(), button, m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()) );

					if( button == MouseButton::Left )
						pMsg->swallow();
				}
				break;

			default:
				break;
		}
	}



} // namespace wg
