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



#include <wg2_button.h>
#include <wg2_gfxdevice.h>
#include <wg2_util.h>
#include <wg2_key.h>
#include <wg2_event.h>
#include <wg2_eventhandler.h>

static const char	c_widgetType[] = {"Button"};

//____ Constructor ____________________________________________________________

WgButton::WgButton()
{
	m_pText = &m_text;
	m_text.setAlignment( WgOrigo::Center );
	m_text.setLineWidth(PixelSize().w);					// We start with no textborders...
	m_text.SetAutoEllipsis(IsAutoEllipsisDefault());

 	m_mode				= WG_MODE_NORMAL;

	m_bDownOutside	 = false;
	m_bPressed 		 = false;
	m_bReturnPressed = false;
	m_bPointerInside = false;
}

//____ Destructor _____________________________________________________________

WgButton::~WgButton()
{
}

//____ Type() _________________________________________________________________

const char * WgButton::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgButton::GetClass()
{
	return c_widgetType;
}


//____ SetIcon() ______________________________________________________________

void WgButton::SetIcon( wg::Skin * pIconSkin )
{
	m_pIconSkin = pIconSkin;
	_iconModified();
}

bool WgButton::SetIcon( wg::Skin * pIconSkin, WgOrigo origo, WgBorders borders, float scale, bool bPushText )
{
	if( scale < 0 || scale > 1.f )
		return false;

	m_pIconSkin = pIconSkin;
	m_iconOrigo = origo;
	m_iconBorders = borders;
	m_iconScale = scale;
	m_bIconPushText = bPushText;

	_iconModified();
	return true;
}


//____ GetTextAreaWidth() _____________________________________________________

Uint32 WgButton::GetTextAreaWidth()
{
	WgRect	contentRect(0,0, PixelSize());

	if( m_pSkin )
		contentRect = _skinContentRect(m_pSkin, {0,0,PixelSize()}, wg::State::Default, m_scale);

	WgRect textRect = _getTextRect( contentRect, _getIconRect( contentRect, m_pIconSkin, m_scale ) );

	return textRect.w;
}

//____ MatchingPixelHeight() _______________________________________________________

int WgButton::MatchingPixelHeight( int width ) const
{
	int height = 0;

	if( m_pSkin )
		height = _skinPreferredSize( m_pSkin, m_scale).h;

	if( m_text.nbChars() != 0 )
	{
		WgSize padding;

		if (m_pSkin)
			padding = _skinContentPadding( m_pSkin, m_scale);

		int heightForText = m_text.heightForWidth(width-padding.w) + padding.h;
		if( heightForText > height )
			height = heightForText;
	}

	//TODO: Take icon into account.

	return height;
}


//____ PreferredPixelSize() _____________________________________________________________

WgSize WgButton::PreferredPixelSize() const
{
	WgSize bestSize;

	if (m_pSkin)
		bestSize = _skinPreferredSize( m_pSkin, m_scale);

	if( m_text.nbChars() != 0 )
	{
		WgSize textSize = m_text.unwrappedSize();

		if( m_pSkin )
			textSize += _skinContentPadding(m_pSkin, m_scale);

		if( textSize.w > bestSize.w )
			bestSize.w = textSize.w;

		if( textSize.h > bestSize.h )
			bestSize.h = textSize.h;
	}

	bestSize = _expandTextRect(bestSize, m_pIconSkin, m_scale);

	return bestSize;
}


//____ _onEnable() _____________________________________________________________

void WgButton::_onEnable()
{
	m_mode = WG_MODE_NORMAL;
	_requestRender();
}

//____ _onDisable() ____________________________________________________________

void WgButton::_onDisable()
{
	m_mode = WG_MODE_DISABLED;
	_requestRender();
}

//____ _onNewSize() ____________________________________________________________

void WgButton::_onNewSize( const WgSize& size )
{
	WgRect	contentRect(0,0, PixelSize());

	if (m_pSkin)
		contentRect = _skinContentRect( m_pSkin, contentRect, wg::State::Default, m_scale);

	WgRect textRect = _getTextRect( contentRect, _getIconRect( contentRect, m_pIconSkin, m_scale ) );

	m_text.setLineWidth(textRect.w);
}

//____ _setScale() _____________________________________________________________

void WgButton::_setScale( int scale )
{
	WgWidget::_setScale(scale);

	m_text.SetScale(scale);

	_requestResize();
}


//____ _onRender() _____________________________________________________________

void WgButton::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	WgBlock    block;
	WgRect contentRect = _canvas;

	WgState    state;
	state.setFocused(m_bFocused);
	state.setSelected(m_bSelected);
	state.setHovered(m_bPointerInside);
	state.setPressed(m_bPressed);
	state.setDisabled(!m_bEnabled);

	if (m_pSkin)
	{
		_renderSkin( m_pSkin, pDevice, state, _canvas, m_scale);
		contentRect = _skinContentRect( m_pSkin, _canvas, state, m_scale);
	}

	// Get icon and text rect from content rect

	WgRect iconRect = _getIconRect( contentRect, m_pIconSkin, m_scale );
	WgRect textRect = _getTextRect( contentRect, iconRect );

	// Render icon

	if( m_pIconSkin )
		_renderSkin( m_pIconSkin, pDevice, state, iconRect, m_scale );

	// Print text

 	if( !m_text.IsEmpty() )
	{
		m_text.setState(state);

		WgGfxDevice::PrintText( pDevice, &m_text, textRect );
	}
}

//____ _onEvent() ______________________________________________________________

void WgButton::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	switch( pEvent->Type() )
	{
		case	WG_EVENT_KEY_PRESS:
		{
			if( static_cast<const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
				m_bReturnPressed = true;
			else
				WgWidget::_onEvent(pEvent, pHandler);
			break;
		}

		case	WG_EVENT_KEY_REPEAT:
		{
			if( static_cast<const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
			{
			}
			else
				WgWidget::_onEvent(pEvent, pHandler);
			break;
		}


		case	WG_EVENT_KEY_RELEASE:
		{
			if( static_cast< const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
			{
				m_bReturnPressed = false;
				pHandler->QueueEvent( new WgEvent::ButtonPress(this) );
			}
			else
				WgWidget::_onEvent(pEvent, pHandler);
			break;
		}

		case	WG_EVENT_MOUSE_ENTER:
			m_bPointerInside = true;
			break;

		case	WG_EVENT_MOUSE_LEAVE:
			m_bPointerInside = false;
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			int button = static_cast<const WgEvent::MouseButtonPress*>(pEvent)->Button();
			if( button == 1 )
				m_bPressed = true;
			else
				WgWidget::_onEvent(pEvent, pHandler);
			break;
		}
		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			int button = static_cast<const WgEvent::MouseButtonRelease*>(pEvent)->Button();
			if( button == 1 )
				m_bPressed = false;
			else
				WgWidget::_onEvent(pEvent, pHandler);
			break;
		}

		case WG_EVENT_MOUSEBUTTON_CLICK:
		{
			int button = static_cast<const WgEvent::MouseButtonClick*>(pEvent)->Button();
			if( button == 1 )
				pHandler->QueueEvent( new WgEvent::ButtonPress(this) );
			else
				WgWidget::_onEvent(pEvent, pHandler);
			break;
		}

		case WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK:
		case WG_EVENT_MOUSEBUTTON_REPEAT:
		case WG_EVENT_MOUSEBUTTON_DRAG:
		{
			int button = static_cast<const WgEvent::MouseButtonEvent*>(pEvent)->Button();
			if( button != 1 )
				WgWidget::_onEvent(pEvent, pHandler);
			break;
		}


		default:
			WgWidget::_onEvent(pEvent, pHandler);
			break;

	}

	WgMode newMode = _getRenderMode();
	if( newMode != m_mode )
	{
		m_mode = newMode;
		_requestRender();
	}

}


//_____ _getRenderMode() ________________________________________________________

WgMode WgButton::_getRenderMode()
{
	if( !IsEnabled() )
		return WG_MODE_DISABLED;

	if( m_bReturnPressed || (m_bPressed && (m_bPointerInside || m_bDownOutside)) )
		return WG_MODE_SELECTED;

	if( m_bPointerInside )
		return WG_MODE_MARKED;

	return WG_MODE_NORMAL;
}


//____ _onRefresh() ____________________________________________________________

void WgButton::_onRefresh( void )
{
	if( m_pSkin )
	{
		if( m_pSkin->isOpaque() )
			m_bOpaque = true;
		else
			m_bOpaque = false;

		_requestRender();
	}
}

//____ SetDownWhenMouseOutside() _______________________________________________

void WgButton::SetDownWhenMouseOutside( bool bDown )
{
		m_bDownOutside		= bDown;
}

//____ _onCloneContent() _______________________________________________________

void WgButton::_onCloneContent( const WgWidget * _pOrg )
{

	WgButton * pOrg = (WgButton *) _pOrg;

	pOrg->Wg_Interface_TextHolder::_cloneInterface( this );
	WgIconHolder::_onCloneContent( pOrg );

	m_text.setText(&pOrg->m_text);
	m_pText = &m_text;
	m_text.setHolder( this );

	m_mode			= pOrg->m_mode;
}

//____ _onAlphaTest() ___________________________________________________________

bool WgButton::_onAlphaTest( const WgCoord& ofs )
{
	//TODO: Take icon into account.

	if (m_pSkin)
		return WgWidget::_onAlphaTest(ofs);

	return false;
}

//____ _onGotInputFocus() ______________________________________________________

void WgButton::_onGotInputFocus()
{
	WgWidget::_onGotInputFocus();
	_requestRender();
}

//____ _onLostInputFocus() _____________________________________________________

void WgButton::_onLostInputFocus()
{
	WgWidget::_onLostInputFocus();
	m_bReturnPressed = false;
	_requestRender();
}


//____ _textModified() __________________________________________________________

void WgButton::_textModified()
{
	_requestRender();
}

//____ _iconModified() __________________________________________________________

void WgButton::_iconModified()
{
	//TODO: Should possibly refresh size too.
	_requestRender();
}
