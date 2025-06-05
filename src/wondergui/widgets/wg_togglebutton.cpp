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

#include <wg_togglebutton.h>
#include <wg_surface.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>
#include <wg_msgrouter.h>
#include <wg_togglegroup.h>

#include <assert.h>

namespace wg
{

	const TypeInfo ToggleButton::TYPEINFO = { "ToggleButton", &Widget::TYPEINFO };

	using namespace Util;


	//____ constructor _________________________________________________________________

	ToggleButton::ToggleButton() : label(this), icon(this)
	{
		m_bFlipOnRelease	= false;
		m_clickArea			= ClickArea::Default;
	}

	//____ destructor _____________________________________________________________

	ToggleButton::~ToggleButton()
	{
		if( m_pToggleGroup )
			m_pToggleGroup->_remove(this);
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& ToggleButton::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setFlipOnRelease() _____________________________________________________

	void ToggleButton::setFlipOnRelease( bool bFlipOnRelease )
	{
		m_bFlipOnRelease = bFlipOnRelease;
	}

	//____ _defaultSize() __________________________________________________

	SizeSPX ToggleButton::_defaultSize(int scale) const
	{
		SizeSPX defaultSize;

		// Get size of text

		if( !label.isEmpty() )
			defaultSize = label._defaultSize(scale);

		// Wrap text in icon if availabe

		if (!_icon().isEmpty())
			defaultSize = _icon()._defaultSize(scale, defaultSize);

		// Apply the skin

		defaultSize = m_skin.sizeForContent( defaultSize, scale );

		return defaultSize;
	}

	//____ _setToggleGroup() ________________________________________________________

	void ToggleButton::_setToggleGroup( ToggleGroup * pGroup )
	{
		if( m_pToggleGroup && pGroup )			// Remove us from current Toggle group if we are not just set to null (then we are being removed by Togglegroup itself).
			m_pToggleGroup->_remove(this);

		m_pToggleGroup = pGroup;
	}


	//____ _receive() _____________________________________________________________

	void ToggleButton::_receive( Msg * _pMsg )
	{
		State state = m_state;

		switch( _pMsg->type() )
		{
			case MsgType::KeyPress:
				if( static_cast<KeyMsg*>(_pMsg)->translatedKeyCode() == Key::Return )
				{
					m_bReturnPressed = true;
					_pMsg->swallow();
				}
				break;

			case MsgType::KeyRepeat:
				if( static_cast<KeyMsg*>(_pMsg)->translatedKeyCode() == Key::Return )
					_pMsg->swallow();
				break;

			case MsgType::KeyRelease:
				if( static_cast<KeyMsg*>(_pMsg)->translatedKeyCode() == Key::Return )
				{
					m_bReturnPressed = false;
					_pMsg->swallow();
				}
				break;

			case MsgType::MouseEnter:
				state.setHovered(true);
				break;
			case MsgType::MouseLeave:
				state.setHovered(false);
				break;
			case MsgType::MousePress:
				if( static_cast<MousePressMsg*>(_pMsg)->button() == MouseButton::Left )
				{
					m_bPressed = true;
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
			case MsgType::MouseDoubleClick:
			case MsgType::MouseRepeat:
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
				m_bPressed = false;
				break;
			default:
				break;
		}

		// Set pressed if return or mouse button 1 is pressed

		if( m_bReturnPressed || (m_bPressed && state.isHovered()) )
			state.setPressed(true);
		else
			state.setPressed(false);

		// Possibly flip checked

		if( state.isPressed() != m_state.isPressed() )
		{
			if( state.isPressed() != m_bFlipOnRelease )
				state.setChecked( !state.isChecked() );
		}

		//

		if( state != m_state )
			_setState(state, true);
	}

	//____ _setState() ______________________________________________________

	void ToggleButton::_setState( State state, bool bPostMsg )
	{
		// If state has changed from checked to unchecked we need to check with Togglegroup

		if( !state.isChecked() && m_state.isChecked() && m_pToggleGroup && !m_pToggleGroup->_uncheck(this) )
			state.setChecked(true);

		//

		State oldState = m_state;
		Widget::_setState(state);

		label._setState( state );

		if (!_icon().isEmpty())
		{
			//TODO: Remove once icon uses CSkinSlot.
			_requestRender();
		}
		if( state.isChecked() != oldState.isChecked() )
		{
			if( bPostMsg )
				Base::msgRouter()->post( ToggleMsg::create(this, state.isChecked() ) );

			if( m_pToggleGroup && state.isChecked() )
				m_pToggleGroup->_check(this);
		}
	}

	//____ _render() ________________________________________________________

	void ToggleButton::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		Widget::_render(pDevice,_canvas,_window);

		// Get the content rect and icon rect

		RectSPX contentRect	= m_skin.contentRect(_canvas, m_scale, m_state );

		RectSPX iconRect		= _icon()._getIconRect( contentRect, m_scale );

		// Blit icon

		if( !_icon().isEmpty() && iconRect.w > 0 && iconRect.h > 0 )
			_icon().skin()->_render( pDevice, iconRect, m_scale, m_state );

		// Print text

	 	if( !label.isEmpty() )
		{
			RectSPX	textRect = _icon()._getTextRect( contentRect, iconRect, m_scale );
			label._render( pDevice, textRect );
		}
	}

	//____ _resize() _______________________________________________________

	void ToggleButton::_resize( const SizeSPX& size, int scale )
	{
		Widget::_resize( size, scale );

		RectSPX contentRect	= m_skin.contentRect(size, m_scale, m_state );

		label._setSize( _icon()._getTextRect( contentRect, _icon()._getIconRect( contentRect, m_scale ), m_scale), m_scale );
	}

	//____ _markTestTextArea() ______________________________________________________

	bool ToggleButton::_markTestTextArea( const CoordSPX& pos )
	{
		RectSPX contentRect = m_skin.contentRect(m_size, m_scale, m_state );

		contentRect = _icon()._getTextRect( contentRect, _icon()._getIconRect( contentRect, m_scale ), m_scale );

		if( label._charAtPos( pos - contentRect.pos() ) != -1 )
			return true;

		return false;
	}

	//____ _alphaTest() ______________________________________________________

	bool ToggleButton::_alphaTest( const CoordSPX& ofs )
	{
		SizeSPX	bgSize		= m_size;

		RectSPX	contentRect = m_skin.contentRect( bgSize, m_scale, m_state );

		RectSPX	iconRect	= _icon()._getIconRect( contentRect, m_scale );

		switch( m_clickArea )
		{
			case ClickArea::Default:		// Full geometry of icon (no alpha test) + text + area between + alpha test on background.
			{
				// Extend iconRect so it connects with textArea before we compare

				RectSPX	textRect = _icon()._getTextRect( contentRect, iconRect, m_scale);

				if( iconRect.x + iconRect.w < textRect.x )
					iconRect.w = textRect.x - iconRect.x;

				if( iconRect.x > textRect.right() )
				{
					iconRect.w += iconRect.x - textRect.right();
					iconRect.x = textRect.right();
				}

				if( iconRect.y + iconRect.h < textRect.y ) 
					iconRect.h = textRect.y - iconRect.y;

				if( iconRect.y > textRect.bottom() )
				{
					iconRect.h += iconRect.y - textRect.bottom();
					iconRect.y = textRect.bottom();
				}

				//

				if( Widget::_alphaTest( ofs ) || _markTestTextArea( ofs ) || iconRect.contains( ofs ) )
					return true;

				return false;
			}
			case ClickArea::Alpha:			// Alpha test on background and icon.
			{
				if( Widget::_alphaTest( ofs ) ||
					( !_icon().isEmpty() && _icon().skin()->_markTest( ofs, iconRect, m_scale, m_state )) )
					return true;

				return false;
			}
			case ClickArea::Geo:			// Full geometry of Widget is clickable.
				return true;
			case ClickArea::Icon:			// Only the icon (alpha test) is clickable.
			{
				if( !_icon().isEmpty() && _icon().skin()->_markTest( ofs, iconRect, m_scale, m_state ) )
					return true;

				return false;
			}
			case ClickArea::Text:			// Only the text is clickable.
			{
				if( _markTestTextArea( ofs ) )
					return true;

				return false;
			}

			default:
				assert(false);			// Garbage in m_clickArea
				return false;
		};
	}

	//____ _componentPos() ______________________________________________________________

	CoordSPX ToggleButton::_componentPos(const Component* pComponent) const
	{
		RectSPX contentRect = m_skin.contentRect(contentRect, m_scale, m_state);

		// Get icon and text rect from content rect

		RectSPX iconRect = _icon()._getIconRect(contentRect, m_scale);

		if (pComponent == &icon)
			return iconRect.pos();

		RectSPX textRect = _icon()._getTextRect(contentRect, iconRect, m_scale);
		return textRect.pos();
	}

	//____ _componentSize() ______________________________________________________________

	SizeSPX ToggleButton::_componentSize(const Component* pComponent) const
	{
		SizeSPX	sz = m_size - m_skin.contentBorderSize(m_scale);

		RectSPX iconRect = _icon()._getIconRect(sz, m_scale);

		if (pComponent == &icon)
			return iconRect.size();

		RectSPX textRect = _icon()._getTextRect(sz, iconRect, m_scale);
		return textRect.size();

	}

	//____ _componentGeo() ______________________________________________________________

	RectSPX ToggleButton::_componentGeo(const Component* pComponent) const
	{
		RectSPX	contentRect = m_skin.contentRect(m_size, m_scale, m_state);

		// Get icon and text rect from content rect

		RectSPX iconRect = _icon()._getIconRect(contentRect, m_scale);

		if (pComponent == &icon)
			return iconRect;

		RectSPX textRect = _icon()._getTextRect(contentRect, iconRect, m_scale);
		return textRect;
	}



} // namespace wg
