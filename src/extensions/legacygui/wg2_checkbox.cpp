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
#include <wg2_checkbox.h>
#include <wg2_gfxdevice.h>
#include <wg_font.h>
#include <wg2_util.h>
#include <wg2_rootpanel.h>
#include <wg2_eventhandler.h>
#include <assert.h>

static const char	c_widgetType[] = {"Checkbox"};



//____ WgCheckBox() _________________________________________________________________

WgCheckBox::WgCheckBox()
{
	m_bChecked			= false;
	m_bOver				= false;
	m_bPressed			= false;
	m_bFlipOnRelease	= false;

	m_pText				= &m_text;
	m_text.setHolder( this );
	m_text.SetAutoEllipsis(IsAutoEllipsisDefault());

	m_clickArea			= DEFAULT;
}

//____ Destructor _____________________________________________________________

WgCheckBox::~WgCheckBox()
{
}


//____ Type() _________________________________________________________________

const char * WgCheckBox::Type( void ) const
{
	return GetClass();
}

const char * WgCheckBox::GetClass( void )
{
	return c_widgetType;
}

//____ SetIcon() ______________________________________________________________

void WgCheckBox::SetIcon( wg::Skin * pIconSkin,
								const WgOrigo& origo, WgBorders borders, float scale,
								bool bPushText )
{
	m_pIconSkin			= pIconSkin;
	m_iconOrigo	= origo;
	m_iconScale			= scale;
	m_bIconPushText		= bPushText;
	m_iconBorders		= borders;
	_onRefresh();
}

//____ SetIcon() ______________________________________________________________

void WgCheckBox::SetIcon( wg::Skin * pIconSkin )
{
	m_pIconSkin			= pIconSkin;
	_onRefresh();
}

//____ SetState() _____________________________________________________________

bool WgCheckBox::SetState( bool _state )
{
	if( m_bChecked != _state )
	{
		m_bChecked = _state;
		_requestRender();
	}

	return true;

}

//____ _setState() _____________________________________________________________

bool WgCheckBox::_setState( bool _state )
{
	if( m_bChecked != _state )
	{
		m_bChecked = _state;

		WgEventHandler * pHandler = _eventHandler();
		if( pHandler )
		{
			if( _state )
				pHandler->QueueEvent( new WgEvent::CheckboxCheck( this ) );
			else
				pHandler->QueueEvent( new WgEvent::CheckboxUncheck( this ) );

			pHandler->QueueEvent( new WgEvent::CheckboxToggle(this, _state ) );
		}
		_requestRender();
	}

	return true;
}

//____ SetFlipOnRelease() _____________________________________________________

void WgCheckBox::SetFlipOnRelease( bool bFlipOnRelease )
{
	m_bFlipOnRelease = bFlipOnRelease;
}


//____ PreferredPixelSize() __________________________________________________

WgSize WgCheckBox::PreferredPixelSize() const
{
	WgSize iconPreferredSize;
	WgSize bgPreferredSize;
	WgSize textPreferredSize;

	if( m_text.nbChars() > 0 )
		textPreferredSize = m_text.unwrappedSize();

	if( m_pSkin )
	{
		bgPreferredSize = _skinPreferredSize(m_pSkin, m_scale);
		textPreferredSize += _skinContentPadding(m_pSkin, m_scale);
	}

	if( m_pIconSkin )
	{
		iconPreferredSize = _skinPreferredSize(m_pIconSkin, m_scale) + m_iconBorders.size();

		//TODO: Add magic for how icon influences textPreferredSize based on origo, iconBorders, iconScale and bgPreferredSize
	}

	WgSize preferredSize = WgSize::max( WgSize::max(iconPreferredSize,bgPreferredSize), textPreferredSize);

	return preferredSize;
}


//____ _onEnable() _________________________________________________
void WgCheckBox::_onEnable()
{
	_requestRender();
}

//____ _onDisable() _________________________________________________
void WgCheckBox::_onDisable()
{
	m_bOver = false;
	m_bPressed = false;

	_requestRender();
}

//____ _onEvent() _____________________________________________________________

void WgCheckBox::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	switch( pEvent->Type() )
	{
		case WG_EVENT_MOUSE_ENTER:
			if( !m_bOver )
			{
				m_bOver = true;
				_requestRender();
			}
			break;

		case WG_EVENT_MOUSE_LEAVE:
			if( m_bOver )
			{
				m_bOver = false;
				_requestRender();
			}
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			int button = static_cast<const WgEvent::MouseButtonPress*>(pEvent)->Button();
			if( button == 1 )
			{
				if( !m_bPressed )
				{
					if( !m_bFlipOnRelease )
						_setState( !m_bChecked );
					m_bPressed = true;
					_requestRender();
				}
			}
			else
				WgWidget::_onEvent(pEvent,pHandler);
			break;
		}

		case WG_EVENT_MOUSEBUTTON_CLICK:
		case WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK:
		case WG_EVENT_MOUSEBUTTON_REPEAT:
		case WG_EVENT_MOUSEBUTTON_DRAG:
		{
			int button = static_cast<const WgEvent::MouseButtonEvent*>(pEvent)->Button();
			if( button != 1 )
				WgWidget::_onEvent(pEvent,pHandler);
			break;
		}

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			int button = static_cast<const WgEvent::MouseButtonPress*>(pEvent)->Button();
			if( button == 1 )
			{
				if( m_bPressed )
				{
					m_bPressed = false;
					if( m_bFlipOnRelease )
						_setState( !m_bChecked );
					_requestRender();
				}
			}
			else
				WgWidget::_onEvent(pEvent,pHandler);
			break;
		}

		default:
			WgWidget::_onEvent(pEvent,pHandler);
			break;
	}
}

//____ GetTextAreaWidth() _____________________________________________________

Uint32 WgCheckBox::GetTextAreaWidth()
{
	WgSize widgetSize = PixelSize();

	WgSize	iconSize;

	if( m_pIconSkin )
		iconSize = _skinPreferredSize(m_pIconSkin, m_scale);

	return _getTextRect( widgetSize, _getIconRect( WgRect(0,0,widgetSize), iconSize, m_scale ) ).w;
}


//____ _onRender() ________________________________________________________

void WgCheckBox::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	// Get correct state

	WgState state = m_state;

	if( m_bOver )
	{
		state.setHovered(true);
		if( m_bPressed )
			state.setPressed(true);
	}

	if( m_bChecked )
	{
		state.setSelekted(true);
		state.setChecked(true);
	}

	// Blit background

	if( m_pSkin )
		_renderSkin(m_pSkin, pDevice, state, _canvas, m_scale );

	// Get the content rect and icon rect

	WgRect contentRect	= m_pSkin ? _skinContentRect(m_pSkin, _canvas, state, m_scale) : _canvas;
	WgRect iconRect		= _getIconRect( contentRect, m_pIconSkin, m_scale );

	// Blit icon

	if( iconRect.w > 0 && iconRect.h > 0 )
		_renderSkin( m_pIconSkin, pDevice, state, iconRect, m_scale );

	// Print text

 	if( m_text.nbLines()!= 0 )
	{
		WgRect	textRect = _getTextRect( contentRect, iconRect );
		m_pText->setState( state );

		WgGfxDevice::PrintText( pDevice, m_pText, textRect );
	}
}

//____ _onRefresh() _______________________________________________________

void WgCheckBox::_onRefresh( void )
{
	if( m_pSkin && m_pSkin->isOpaque() )
		m_bOpaque = true;
	else
		m_bOpaque = false;

	_requestRender();
}

//____ _onNewSize() _______________________________________________________

void WgCheckBox::_onNewSize( const WgSize& size )
{
	m_text.setLineWidth( GetTextAreaWidth() );
}


//____ _setScale() _____________________________________________________________

void WgCheckBox::_setScale( int scale )
{
	WgWidget::_setScale(scale);

	m_text.SetScale(scale);

		_requestResize();
}



//____ _onCloneContent() _______________________________________________________

void WgCheckBox::_onCloneContent( const WgWidget * _pOrg )
{
	WgCheckBox * pOrg = (WgCheckBox *) _pOrg;

	m_bChecked			= pOrg->m_bChecked;
	m_bOver				= false;
	m_bPressed			= false;

	m_text				= pOrg->m_text;
	m_clickArea			= pOrg->m_clickArea;

	Wg_Interface_TextHolder::_onCloneContent( pOrg );
	WgIconHolder::_onCloneContent( pOrg );
}

//____ _textModified() _________________________________________________________

void WgCheckBox::_textModified()
{
	//TODO: Need to do more here, like possibly request resize.

	_requestRender();
}

//____ _iconModified() ________________________________________________________

void WgCheckBox::_iconModified()
{
	//TODO: Need to do more here, like possibly request resize.

	_requestRender();
}



//____ _markTestTextArea() ______________________________________________________

bool WgCheckBox::_markTestTextArea( int _x, int _y )
{
	WgRect	contentRect = _getTextRect( PixelSize(), _getIconRect( PixelSize(), m_pIconSkin, m_scale ) );

	if( m_text.CoordToOfs( WgCoord(_x,_y), contentRect ) != -1 )
		return true;

	return false;
}

//____ _onAlphaTest() ______________________________________________________

bool WgCheckBox::_onAlphaTest( const WgCoord& ofs )
{
	WgMode mode = WG_MODE_NORMAL;

	if( !m_bEnabled )
		mode = WG_MODE_DISABLED;
	else if( m_bOver )
	{
		mode = WG_MODE_MARKED;
		if( m_bPressed )
			mode = WG_MODE_SELECTED;
	}

	// Get blocksets for background and icon

	WgSize	bgSize		= PixelSize();
	WgRect	iconRect	= _getIconRect( bgSize, m_pIconSkin, m_scale );


	switch( m_clickArea )
	{
		case DEFAULT:		// Full geometry of icon (no alpha test) + text + area between + alpha test on background.
		{
			// Extend iconRect so it connects with textArea before we compare

			WgRect	textRect = _getTextRect( bgSize, iconRect);

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

			if( (m_pSkin && _markTestSkin(m_pSkin, ofs, WgRect(0,0,bgSize), wg::State::Default, m_markOpacity, m_scale)) ||
				_markTestTextArea( ofs.x, ofs.y ) ||
				iconRect.contains( ofs ) )
				return true;

			return false;
		}
		case ALPHA:			// Alpha test on background and icon.
		{
			if( (m_pSkin && _markTestSkin(m_pSkin, ofs, WgRect(0,0,bgSize), wg::State::Default, m_markOpacity, m_scale)) ||
			   (m_pIconSkin && _markTestSkin( m_pIconSkin, ofs, iconRect, wg::State::Default, m_markOpacity, m_scale )) )
				return true;

			return false;
		}
		case GEO:			// Full geometry of Widget is clickable.
			return true;
		case ICON:			// Only the icon (alpha test) is clickable.
		{
			if( m_pIconSkin && _markTestSkin( m_pIconSkin, ofs, iconRect, wg::State::Default, m_markOpacity, m_scale ) )
				return true;

			return false;
		}
		case TEXT:			// Only the text is clickable.
		{
			if( _markTestTextArea( ofs.x, ofs.y ) )
				return true;

			return false;
		}

		default:
			assert(false);			// Garbage in m_clickArea
			return false;
	};
}
