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
#include <wg2_popupopener.h>
#include <wg2_gfxdevice.h>
#include <wg2_util.h>
#include <wg2_key.h>
#include <wg2_event.h>
#include <wg2_eventhandler.h>
#include <wg2_container.h>
#include <wg2_popuplayer.h>

static const char	c_widgetType[] = {"PopupOpener"};

//____ Constructor ____________________________________________________________

WgPopupOpener::WgPopupOpener() : m_attachPoint(WgOrigo::SouthWest), m_bOpenOnHover(false), m_bOpen(false)
{
	m_bSelectable = false;

	m_pText = &m_text;
	m_text.setAlignment( WgOrigo::Center );
	m_text.setLineWidth(PixelSize().w);					// We start with no textborders...
	m_text.SetAutoEllipsis(IsAutoEllipsisDefault());


	m_bReturnPressed = false;
}

//____ Destructor _____________________________________________________________

WgPopupOpener::~WgPopupOpener()
{
	if (m_pPopup)
		delete m_pPopup;
}

//____ Type() _________________________________________________________________

const char * WgPopupOpener::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgPopupOpener::GetClass()
{
	return c_widgetType;
}

//____ SetIcon() ______________________________________________________________

void WgPopupOpener::SetIcon( wg::Skin * pIconSkin )
{
	m_pIconSkin = pIconSkin;
	_iconModified();
}

bool WgPopupOpener::SetIcon( wg::Skin * pIconSkin, WgOrigo origo, WgBorders borders, float scale, bool bPushText )
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


//____ SetPopup() _________________________________________________________

void WgPopupOpener::SetPopup(WgWidget * pPopup)
{
	m_pPopup = pPopup;
}

//____ SetOpenOnHover() ___________________________________________________

void WgPopupOpener::SetOpenOnHover(bool bOpen)
{
	m_bOpenOnHover = bOpen;
}

//____ SetAttachPoint() ___________________________________________________

void WgPopupOpener::SetAttachPoint(WgOrigo attachPoint)
{
	m_attachPoint = attachPoint;
}

//____ SetPopupOffset() _______________________________________________________

void WgPopupOpener::SetPopupOffset(WgCoord offset)
{
	m_popupOfs = offset;
}


//____ GetTextAreaWidth() _____________________________________________________

Uint32 WgPopupOpener::GetTextAreaWidth()
{
	WgRect	contentRect(0,0, PixelSize());

	if( m_pSkin )
		contentRect = _skinContentRect( m_pSkin, contentRect, WgStateEnum::Default, m_scale);

	WgRect textRect = _getTextRect( contentRect, _getIconRect( contentRect, m_pIconSkin, m_scale ) );

	return textRect.w;
}

//____ MatchingPixelHeight() _______________________________________________________

int WgPopupOpener::MatchingPixelHeight( int width ) const
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

WgSize WgPopupOpener::PreferredPixelSize() const
{
	WgSize bestSize;

	if (m_pSkin)
		bestSize = _skinPreferredSize( m_pSkin, m_scale);

	if( m_text.nbChars() != 0 )
	{
		WgSize textSize = m_text.unwrappedSize();

		if( m_pSkin )
			textSize += _skinContentPadding( m_pSkin, m_scale);

		if( textSize.w > bestSize.w )
			bestSize.w = textSize.w;

		if( textSize.h > bestSize.h )
			bestSize.h = textSize.h;
	}

	bestSize = _expandTextRect(bestSize, m_pIconSkin, m_scale);

	return bestSize;
}


//____ _setState() ________________________________________________________

void WgPopupOpener::_setState(WgState state)
{
	if (m_bOpen)
	{
		m_closeState = state;
		state.setPressed(true);			// Force pressed state when popup is open.
	}
//	WgWidget::_setState(state);

	m_text.setState(state);
	_requestRender(); //TODO: Only requestRender if text appearance has changed (let m_text.setState() return if rendering is needed)
}

//____ _onEnable() _____________________________________________________________

void WgPopupOpener::_onEnable()
{
	m_state.setDisabled(false);
	_requestRender();
}

//____ _onDisable() ____________________________________________________________

void WgPopupOpener::_onDisable()
{
	m_state.setDisabled(true);
	_requestRender();
}

//____ _onNewSize() ____________________________________________________________

void WgPopupOpener::_onNewSize( const WgSize& size )
{
	WgRect	contentRect(0,0, PixelSize());

	if (m_pSkin)
		contentRect = _skinContentRect( m_pSkin, contentRect, WgStateEnum::Default, m_scale);

	WgRect textRect = _getTextRect( contentRect, _getIconRect( contentRect, m_pIconSkin, m_scale ) );

	m_text.setLineWidth(textRect.w);
}


void WgPopupOpener::SetScale( int scale )
{
    if (m_scale != scale)
        _setScale(scale);
}
//____ _setScale() _____________________________________________________________

void WgPopupOpener::_setScale( int scale )
{
	WgWidget::_setScale(scale);

	m_text.SetScale(scale);

	_requestResize();
}


//____ _onRender() _____________________________________________________________

void WgPopupOpener::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	WgState useState = m_state;
	if (m_bOpen)
		useState.setPressed(true);

	if (m_pSkin)
	{
		_renderSkin( m_pSkin, pDevice, useState, _canvas, m_scale);
	}

	WgBlock    block;

	WgRect contentRect = _canvas;


	if (m_pSkin)
	{
		contentRect = _skinContentRect( m_pSkin, _canvas, useState, m_scale);
	}

	// Get icon and text rect from content rect

	WgRect iconRect = _getIconRect( contentRect, m_pIconSkin, m_scale );
	WgRect textRect = _getTextRect( contentRect, iconRect );

	// Convert WgState to WgMode


	// Render icon

	if( m_pIconSkin )
		_renderSkin(m_pIconSkin, pDevice, useState, iconRect, m_scale);

	// Print text

 	if( !m_text.IsEmpty() )
	{
		m_text.setState(useState);

		WgGfxDevice::PrintText( pDevice, &m_text, textRect );
	}
}

//____ _onEvent() ______________________________________________________________

void WgPopupOpener::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	WgState oldState = m_state;

	switch( pEvent->Type() )
	{
/*
		case	WG_EVENT_KEY_PRESS:
		{
			if( static_cast<const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
				m_bReturnPressed = true;
			else
				pHandler->ForwardEvent( pEvent );
			break;
		}

		case	WG_EVENT_KEY_REPEAT:
		{
			if( static_cast<const WgEvent::KeyPress*>(pEvent)->TranslatedKeyCode() == WG_KEY_RETURN )
			{
			}
			else
				pHandler->ForwardEvent( pEvent );
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
				pHandler->ForwardEvent( pEvent );
			break;
		}
|*/

		case	WG_EVENT_MOUSE_ENTER:
		{
			if (m_bOpenOnHover && !m_bOpen)
				_open();

			m_state.setHovered(true);
			break;
		}

		case	WG_EVENT_MOUSE_LEAVE:
			m_state.setHovered(false);
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			int button = static_cast<const WgEvent::MouseButtonPress*>(pEvent)->Button();
			if (button == 1)
				m_state.setPressed(true);
			else
				WgWidget::_onEvent(pEvent,pHandler);

			if (!m_bOpenOnHover && button == 1)
			{
				if (m_bOpen)
				{
					_close();
				}
				else
				{
					_open();
//					Base::inputHandler()->_yieldButtonEvents(MouseButton::Left, this, _parent()->_getPopupLayer());
				}
			}
			break;
		}
		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			int button = static_cast<const WgEvent::MouseButtonRelease*>(pEvent)->Button();
			if( button == 1 )
				m_state.setPressed(false);
			else
				WgWidget::_onEvent(pEvent,pHandler);
			break;
		}

		case WG_EVENT_MOUSEBUTTON_DRAG:
		{
			int button = static_cast<const WgEvent::MouseButtonEvent*>(pEvent)->Button();
			if( button != 1 )
				WgWidget::_onEvent(pEvent,pHandler);
			break;
		}

		case WG_EVENT_POPUP_CLOSED:
		{
			m_bOpen = false;
			if (!m_state.isPressed())
				_requestRender();			// We have been rendered with pressed state since popup has been open.

			break;
		}

		default:
			WgWidget::_onEvent(pEvent,pHandler);
			break;

	}

	if( oldState != m_state )
		_requestRender();
}


//____ _onRefresh() ____________________________________________________________

void WgPopupOpener::_onRefresh( void )
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

//____ _onCloneContent() _______________________________________________________

void WgPopupOpener::_onCloneContent( const WgWidget * _pOrg )
{

	WgPopupOpener * pOrg = (WgPopupOpener *) _pOrg;

	pOrg->Wg_Interface_TextHolder::_cloneInterface( this );
	WgIconHolder::_onCloneContent( pOrg );

	m_text.setText(&pOrg->m_text);
	m_pText = &m_text;
	m_text.setHolder( this );

	m_pIconSkin		= pOrg->m_pIconSkin;
//	m_state			= pOrg->m_state;
}

//____ _onAlphaTest() ___________________________________________________________

bool WgPopupOpener::_onAlphaTest( const WgCoord& ofs )
{
	if (m_pSkin)
		return WgWidget::_onAlphaTest(ofs);


	return	false;
}

//____ _onGotInputFocus() ______________________________________________________

void WgPopupOpener::_onGotInputFocus()
{
	WgWidget::_onGotInputFocus();
	_requestRender();
}

//____ _onLostInputFocus() _____________________________________________________

void WgPopupOpener::_onLostInputFocus()
{
	WgWidget::_onLostInputFocus();
	m_bReturnPressed = false;
	_requestRender();
}


//____ _textModified() __________________________________________________________

void WgPopupOpener::_textModified()
{
	_requestRender();
    _requestResize();
}

//____ _iconModified() __________________________________________________________

void WgPopupOpener::_iconModified()
{
	//TODO: Should possibly refresh size too.
	_requestRender();
    _requestResize();
}

//____ _open() ____________________________________________________________

void WgPopupOpener::_open()
{
	auto pLayer = Parent()->_getPopupLayer();
	if (pLayer && m_pPopup)
	{
		pLayer->Push(m_pPopup, this, ScreenPixelGeo(), m_attachPoint, m_popupOfs, m_bOpenOnHover, m_bOpenOnHover );
		m_bOpen = true;
		m_closeState = m_state;
	}
}

//____ _close() ___________________________________________________________

void WgPopupOpener::_close()
{
	auto pLayer = Parent()->_getPopupLayer();
	if (pLayer && m_pPopup)
	{
		pLayer->Pop(m_pPopup);
	}
}
