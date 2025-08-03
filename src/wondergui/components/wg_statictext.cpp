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


namespace wg
{
	using namespace Util;

	const TypeInfo	StaticText::TYPEINFO = { "StaticText", &Component::TYPEINFO };

	//____ constructor _____________________________________________________________

	StaticText::StaticText(Widget * pWidget ) : Component( pWidget )
	{
		_layout()->addText(this);
	}

	//____ destructor ______________________________________________________________

	StaticText::~StaticText()
	{
		_layout()->removeText(this);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticText::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setStyle() ______________________________________________________________

	void StaticText::setStyle( TextStyle * pStyle )
	{
		if( pStyle == m_pStyle )
			return;
		
		TextStyle_p pOld = m_pStyle;			// Keep ref count until onStyleChanged has been called.
		m_pStyle = pStyle;
		_layout()->onStyleChanged(this, pStyle, pOld);
	}

	//____ clearStyle() ____________________________________________________________

	void StaticText::clearStyle()
	{
		if( !m_pStyle )
			return;
		
		TextStyle_p pOld = m_pStyle;			// Keep ref count until onStyleChanged has been called.
		m_pStyle = 0;
		_layout()->onStyleChanged(this, nullptr, pOld);
	}

	//____ setLayout() __________________________________________________________

	void StaticText::setLayout( TextLayout * pLayout )
	{
		if( pLayout == m_pLayout )
			return;

		_layout()->removeText(this);
		m_pLayout = pLayout;
		_layout()->addText(this);
	}

	//____ clearLayout() ________________________________________________________

	void StaticText::clearLayout()
	{
		if( !m_pLayout )
			return;

		_layout()->removeText(this);
		m_pLayout = 0;
		_layout()->addText(this);
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

	void StaticText::_receive(Msg* _pMsg)
	{
		if (!_pMsg->isMouseMsg())
			return;

		auto pMsg = static_cast<InputMsg*>(_pMsg);

		switch (pMsg->type())
		{
		case MsgType::MouseEnter:
		case MsgType::MouseMove:
		{
			TextLink_p pLink;

			// Get link from character properties

			CoordSPX localPos = _toLocal(static_cast<InputMsg*>(pMsg)->pointerSpxPos());
			int markedChar = _layout()->charAtPos(this, localPos);
			if (markedChar >= 0)
			{
				TextStyle_p pStyle = m_charBuffer.chars()[markedChar].stylePtr();
				if (pStyle)
					pLink = pStyle->link();
			}

			// Fall back to parts default style

			if (!pLink && m_pStyle)
				pLink = m_pStyle->link();


			// Post messages if there was a change in what link we are pointing at

			if (pLink != m_pMarkedLink)
			{
				MsgRouter_p	pRouter = Base::msgRouter();

				if (m_pMarkedLink)
					pRouter->post(MouseLeaveMsg::create(pMsg->inputId(), m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()));

				if (pLink)
					pRouter->post(MouseEnterMsg::create(pMsg->inputId(), pLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()));

				m_pMarkedLink = pLink;
			}

			break;
		}
		case MsgType::MouseLeave:

			if (m_pMarkedLink)
			{
				Base::msgRouter()->post(MouseLeaveMsg::create(pMsg->inputId(), m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()));
				m_pMarkedLink = 0;
			}
			break;

		case MsgType::MousePress:
			if (m_pMarkedLink)
			{
				MouseButton button = static_cast<MousePressMsg*>(pMsg)->button();
				Base::msgRouter()->post(MousePressMsg::create(pMsg->inputId(), button, m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()));

				if (button == MouseButton::Left)
					pMsg->swallow();
			}
			break;

		case MsgType::MouseDrag:

			//TODO: Add swallow under right circumstances.
			break;

		case MsgType::MouseRepeat:
			if (m_pMarkedLink)
			{
				MouseButton button = static_cast<MouseRepeatMsg*>(pMsg)->button();
				Base::msgRouter()->post(MouseRepeatMsg::create(pMsg->inputId(), button, m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()));

				if (button == MouseButton::Left)
					pMsg->swallow();
			}
			break;

		case MsgType::MouseRelease:
			//TODO: Should only post if press was inside.
			if (m_pMarkedLink)
			{
				MouseButton button = static_cast<MouseReleaseMsg*>(pMsg)->button();
				bool bReleasedInside = static_cast<MouseReleaseMsg*>(pMsg)->releaseInside();
				Base::msgRouter()->post(MouseReleaseMsg::create(pMsg->inputId(), button, m_pMarkedLink, bReleasedInside, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()));

				if (button == MouseButton::Left)
					pMsg->swallow();
			}
			break;

		case MsgType::MouseClick:
			//TODO: Doesn't check if we stay on link during whole click.
			if (m_pMarkedLink)
			{
				auto pClickMsg = static_cast<MouseClickMsg*>(pMsg);

				MouseButton button = pClickMsg->button();
				Base::msgRouter()->post(MouseClickMsg::create(pMsg->inputId(), button, m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp(), pClickMsg->duration(), pClickMsg->ordinal()));

				if (button == MouseButton::Left)
				{
					Base::msgRouter()->post(SelectMsg::create(m_pMarkedLink));
					pMsg->swallow();
				}
			}
			break;

		case MsgType::MouseDoubleClick:
			//TODO: Doesn't check if we stay on link during whole double click.
			if (m_pMarkedLink)
			{
				MouseButton button = static_cast<MouseDoubleClickMsg*>(pMsg)->button();
				Base::msgRouter()->post(MouseDoubleClickMsg::create(pMsg->inputId(), button, m_pMarkedLink, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()));

				if (button == MouseButton::Left)
					pMsg->swallow();
			}
			break;

		default:
			break;
		}
	}

	//____ _setState() ______________________________________________________________

	void StaticText::_setState( State state )
	{
		if( state == m_state )
			return;

		State old = m_state;
		m_state = state;
		_layout()->onStateChanged( this, state, old );
	}

	//____ _defaultSize() _________________________________________________________

	SizeSPX StaticText::_defaultSize(int scale) const
	{
		return _layout()->defaultSize(this, scale);
	}

	//____ _matchingWidth() _________________________________________________________

	spx StaticText::_matchingWidth( spx height, int scale ) const
	{
		return _layout()->matchingWidth(this, height, scale);
	}

	//____ _matchingHeight() ________________________________________________________

	spx StaticText::_matchingHeight( spx width, int scale ) const
	{
		return _layout()->matchingHeight(this, width, scale);
	}

	//____ _charAtPos() ___________________________________________________________

	int StaticText::_charAtPos( CoordSPX pos ) const
	{
		return _layout()->charAtPos(this,pos);
	}

	//____ _charRect() ____________________________________________________________

	RectSPX StaticText::_charRect( int charOfs ) const
	{
		return _layout()->charRect(this, charOfs);
	}

	//____ _charLine() ____________________________________________________________

	int StaticText::_charLine( int charOfs ) const
	{
		return _layout()->charLine(this, charOfs);
	}

	//____ _setSize() _____________________________________________________________

	void StaticText::_setSize( SizeSPX size, int scale )
	{
		if( size == m_size && scale && m_scale )
			return;

		SizeSPX oldSize = m_size;
		int		oldScale = m_scale;
		m_size = size;
		m_scale = scale;
		_layout()->onResized(this,size, oldSize, scale, oldScale);
	}

	//_____ _render() _____________________________________________________________

	void  StaticText::_render( GfxDevice * pDevice, const RectSPX& _canvas )
	{
		_layout()->render(this, pDevice, _canvas );
	}

	//____ _rectForRange() __________________________________________________________

	RectSPX  StaticText::_rectForRange( int ofs, int length ) const
	{
		return _layout()->rectForRange(this, ofs, length);
	}

	//____ _tooltip() _______________________________________________________________

	String StaticText::_tooltip() const
	{
		return _layout()->tooltip(this);
	}

	//____ _getString() ___________________________________________________________________

	String StaticText::_getString() const
	{
		return String(&m_charBuffer);
	}

	//____ _clear() _________________________________________________________________

	void StaticText::_clear()
	{
		int removed = m_charBuffer.length();
		m_charBuffer.clear();
		_modified(0, removed, 0);
	}

	//___ _setText() ____________________________________________________________________

	void StaticText::_setText(const CharSeq& seq)
	{
		CharSeq::CharBasket chars = seq.getChars();

		if( chars.length == m_charBuffer.length() )
		{
			auto p1 = m_charBuffer.chars();
			auto p2 = chars.ptr;
			for( int i = 0 ; i < chars.length ; i++ )
				if( (* p1++).equals(* p2++) == false )
				{
					p2--;
					break;
				}

			if( p2 == chars.ptr + chars.length )
				return;
		}

		int removed = m_charBuffer.length();
		m_charBuffer = seq;
		_modified(0, removed, m_charBuffer.length());
	}

	void StaticText::_setText(const CharBuffer * buffer)
	{
		if( m_charBuffer.compareTo(buffer) == 0 )
			return;

		int removed = m_charBuffer.length();
		m_charBuffer = *buffer;
		_modified(0, removed, m_charBuffer.length());
	}

	void StaticText::_setText(const String& str)
	{
		if( TextTool::strcmp( m_charBuffer.chars(), str.chars() ) == 0 )
			return;

		int removed = m_charBuffer.length();
		m_charBuffer = str;
		_modified(0, removed, m_charBuffer.length());
	}

	//____ _append() ________________________________________________________________

	int StaticText::_append(const CharSeq& seq)
	{
		//TODO: Check and respect boundaries. Guarantee correct parameters to _modified()

		int ofs = m_charBuffer.length();
		int len = m_charBuffer.pushBack(seq);
		_modified(ofs, 0, len);
		return len;
	}

	//____ _insert() ________________________________________________________________

	int StaticText::_insert(int ofs, const CharSeq& seq)
	{
		//TODO: Check and respect boundaries. Guarantee correct parameters to _modified()

		int len = m_charBuffer.insert(ofs, seq);
		_modified(ofs, 0, seq.length());
		return len;
	}

	//____ _replace() ___________________________________________________________

	int StaticText::_replace(int ofs, int nDelete, const CharSeq& seq)
	{
		//TODO: Check and respect boundaries. Guarantee correct parameters to _modified()

		int diff = m_charBuffer.replace(ofs, nDelete, seq);
		_modified(ofs, nDelete, seq.length());
		return diff;
	}

	//____ _erase() ____________________________________________________________

	int StaticText::_erase(int ofs, int len)
	{
		//TODO: Check and respect boundaries. Guarantee correct parameters to _modified()

		int removed = m_charBuffer.remove(ofs, len);
		_modified(ofs, len, 0);
		return removed;
	}

	//____ _setCharStyle() ______________________________________________________

	void StaticText::_setCharStyle(TextStyle * pStyle)
	{
		m_charBuffer.setStyle(pStyle);
		_layout()->onCharStyleChanged(this);
	}

	void StaticText::_setCharStyle(TextStyle * pStyle, int ofs, int len)
	{
		m_charBuffer.setStyle(pStyle, ofs, len);
		_layout()->onCharStyleChanged(this, ofs, len);
	}

	//____ _clearCharStyle() ____________________________________________________

	void StaticText::_clearCharStyle()
	{
		m_charBuffer.clearStyle();
		_layout()->onCharStyleChanged(this);
	}

	void StaticText::_clearCharStyle(int ofs, int len)
	{
		m_charBuffer.clearStyle(ofs, len);
		_layout()->onCharStyleChanged(this, ofs, len);
	}

	//____ _mapperRequestRender() _______________________________________________

	void StaticText::_mapperRequestRender(const RectSPX& rect)
	{
		_requestRender(rect);
	}

	//____ _modified() _________________________________________________________

	void StaticText::_modified( int ofs, int deleted, int inserted )
	{
		_layout()->onTextModified(this, ofs, deleted, inserted);

		NotifData notif = { ofs, deleted, inserted };
		_notify(ComponentNotif::TextModified, 0, &notif);
	}

} // namespace wg
