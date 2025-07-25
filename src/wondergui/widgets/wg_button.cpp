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



#include <wg_button.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>
#include <wg_key.h>
#include <wg_msg.h>
#include <wg_msgrouter.h>

namespace wg
{

	const TypeInfo Button::TYPEINFO = { "Button", &Widget::TYPEINFO };

	//____ constructor ____________________________________________________________

	Button::Button() : icon(this), label(this)
	{
	}

	//____ Destructor _____________________________________________________________

	Button::~Button()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Button::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setSelectOnPress() _________________________________________________

	void Button::setSelectOnPress(bool bSelectOnPress)
	{
		m_bSelectOnPress = bSelectOnPress;
	}

	//____ _matchingHeight() _______________________________________________________

	spx Button::_matchingHeight( spx width, int scale ) const
	{
		spx defaultHeight = m_skin.defaultSize(scale).h;

		if (label.isEmpty() && icon.isEmpty())
			return defaultHeight;

		spx iconHeight = _icon()._defaultSize(scale).h;
		spx textHeight = 0;									// includes padding/spacing 

		SizeSPX skinPadding = m_skin.contentBorderSize(scale);

		if( !label.isEmpty() )
		{
			SizeSPX iconPadding = _icon()._textPaddingSize(scale);
			textHeight = label._matchingHeight(width-skinPadding.w-iconPadding.w,scale) + iconPadding.h;
		}

		spx heightForContent = std::max(textHeight, iconHeight) + skinPadding.h;

		return std::max(heightForContent, defaultHeight);
	}


	//____ _defaultSize() _____________________________________________________________

	SizeSPX Button::_defaultSize(int scale) const
	{
		SizeSPX defaultSize;

		// Get size of text

		if (!label.isEmpty())
			defaultSize = label._defaultSize(scale);

		// Wrap text in icon if availabe

		if (!_icon().isEmpty())
			defaultSize = _icon()._defaultSize(scale, defaultSize);

		// Apply the skin

		defaultSize = m_skin.sizeForContent(defaultSize, scale);

		return defaultSize;
	}

	//____ _setState() ______________________________________________________

	void Button::_setState( State state )
	{
		if (!_icon().isEmpty())
		{
			//TODO: Remove once icon uses CSkinSlot.
			_requestRender();
		}

		label._setState(state);
		Widget::_setState(state);
	}

	//____ _resize() ____________________________________________________________

	void Button::_resize( const SizeSPX& _size, int scale )
	{
		Widget::_resize(_size, scale);

		RectSPX	contentRect = m_skin.contentRect(_size,m_scale,m_state);

		RectSPX textRect = _icon()._getTextRect( contentRect, _icon()._getIconRect( contentRect, m_scale ), m_scale );

		label._setSize( textRect, m_scale );
	}

	//____ _render() _____________________________________________________________

	void Button::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		Widget::_render(pDevice,_canvas,_window);

		RectSPX	contentRect = m_skin.contentRect(_canvas, m_scale, m_state);

		// Get icon and text rect from content rect

		RectSPX iconRect = _icon()._getIconRect( contentRect, m_scale );
		RectSPX textRect = _icon()._getTextRect( contentRect, iconRect, m_scale );

		// Render icon

		if( _icon().skin() )
			_icon().skin()->_render( pDevice, iconRect, m_scale, m_state );

		// Print text

	 	if( !label.isEmpty() )
			label._render( pDevice, textRect );
	}

	//____ _receive() ______________________________________________________________

	void Button::_receive( Msg * _pMsg )
	{
		State state = m_state;
		MsgRouter_p	pRouter = Base::msgRouter();

		switch( _pMsg->type() )
		{
			case MsgType::KeyPress:
				if( static_cast<KeyPressMsg*>(_pMsg)->translatedKeyCode() == Key::Return )
				{
					if( m_bSelectOnPress )
						pRouter->post(SelectMsg::create(this));

					m_bReturnPressed = true;
					_pMsg->swallow();
				}
				break;

			case MsgType::KeyRepeat:
				if( static_cast<KeyRepeatMsg*>(_pMsg)->translatedKeyCode() == Key::Return )
					if (m_bSelectOnPress)
						pRouter->post(SelectMsg::create(this));

					_pMsg->swallow();
				break;

			case MsgType::KeyRelease:
				if( static_cast<KeyReleaseMsg*>(_pMsg)->translatedKeyCode() == Key::Return )
				{
					m_bReturnPressed = false;
					if( !m_bSelectOnPress )
						pRouter->post( SelectMsg::create(this) );
					_pMsg->swallow();
				}
				break;

			case MsgType::MouseEnter:
				m_bHovered = true;
				break;
			case MsgType::MouseLeave:
				m_bHovered = false;
				break;
			case MsgType::MousePress:
				if( static_cast<MousePressMsg*>(_pMsg)->button() == MouseButton::Left )
				{
					if (m_bSelectOnPress)
						pRouter->post(SelectMsg::create(this));

					m_bPressed = true;
					_pMsg->swallow();
				}
				break;

			case MsgType::MouseRepeat:
				if (static_cast<MouseRepeatMsg*>(_pMsg)->button() == MouseButton::Left)
				{
					if (m_bSelectOnPress)
						pRouter->post(SelectMsg::create(this));

					_pMsg->swallow();
				}
				break;

			case MsgType::MouseRelease:
				if( static_cast<MouseReleaseMsg*>(_pMsg)->button() == MouseButton::Left )
				{
					m_bPressed = false;
					_pMsg->swallow();
				}
				break;

			case MsgType::MouseClick:
				if( static_cast<MouseClickMsg*>(_pMsg)->button() == MouseButton::Left )
				{
					if( !m_bSelectOnPress )
						pRouter->post( SelectMsg::create(this) );
					_pMsg->swallow();
				}
				break;

			case MsgType::MouseDoubleClick:		// We handle double-click as single-click, allowing fast repeated clicking.
				if (static_cast<MouseClickMsg*>(_pMsg)->button() == MouseButton::Left)
				{
					if (!m_bSelectOnPress)
						pRouter->post(SelectMsg::create(this));
					_pMsg->swallow();
				}
				break;

			case MsgType::MouseDrag:
				if( static_cast<MouseButtonMsg*>(_pMsg)->button() == MouseButton::Left )
					_pMsg->swallow();
				break;

			case MsgType::FocusGained:
				state.setFocused(true);
				break;
			case MsgType::FocusLost:
				state.setFocused(false);
				m_bReturnPressed = false;
				m_bPressed = false;				// Is this correct behaviour?
				break;

			default:
				break;
		}

		state.setPressed(m_bReturnPressed || m_bPressed);
		state.setHovered(m_bHovered);						// Needs to be after setPressed() due to their interdependence.

		if( state != m_state )
			_setState(state);
	}

	//____ _alphaTest() ___________________________________________________________

	bool Button::_alphaTest( const CoordSPX& ofs )
	{
		if( _icon().skin() )
		{
			//TODO: Test against icon.
		}

		return Widget::_alphaTest(ofs);
	}

	//____ _componentPos() ______________________________________________________________

	CoordSPX Button::_componentPos( const Component * pComponent ) const
	{
		RectSPX contentRect = m_skin.contentRect(contentRect, m_scale, m_state);

		// Get icon and text rect from content rect

		RectSPX iconRect = _icon()._getIconRect( contentRect, m_scale );

		if( pComponent == &icon )
			return iconRect.pos();

		RectSPX textRect = _icon()._getTextRect( contentRect, iconRect, m_scale );
		return textRect.pos();
	}

	//____ _componentSize() ______________________________________________________________

	SizeSPX Button::_componentSize( const Component * pComponent ) const
	{
		SizeSPX	sz = m_size - m_skin.contentBorderSize(m_scale);

		RectSPX iconRect = _icon()._getIconRect( sz, m_scale );

		if( pComponent == &icon )
			return iconRect.size();

		RectSPX textRect = _icon()._getTextRect( sz, iconRect, m_scale );
		return textRect.size();

	}

	//____ _componentGeo() ______________________________________________________________

	RectSPX Button::_componentGeo( const Component * pComponent ) const
	{
		RectSPX	contentRect = m_skin.contentRect(m_size, m_scale, m_state);

		// Get icon and text rect from content rect

		RectSPX iconRect = _icon()._getIconRect( contentRect, m_scale );

		if( pComponent == &icon )
			return iconRect;

		RectSPX textRect = _icon()._getTextRect( contentRect, iconRect, m_scale );
		return textRect;
	}

} // namespace wg
