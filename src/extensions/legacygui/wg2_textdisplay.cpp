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
#include <wg2_textdisplay.h>
#include <wg2_key.h>
#include <wg_font.h>
#include <wg2_gfxdevice.h>
#include <wg2_eventhandler.h>
#include <wg2_util.h>

static const char	c_widgetType[] = {"TextDisplay"};


//____ WgTextDisplay() _________________________________________________________________

WgTextDisplay::WgTextDisplay()
{
	m_pText			= &m_text;
	m_maxLines		= 0;
	m_text.setHolder( this );

	m_text.setLineWidth( PixelSize().w );
	m_text.SetAutoEllipsis(IsAutoEllipsisDefault());
	m_text.SetEditMode( WgTextEditMode::Static );
	m_bResetCursorOnFocus = true;
}


//____ Destructor _____________________________________________________________

WgTextDisplay::~WgTextDisplay()
{
}


//____ Type() _________________________________________________________________

const char * WgTextDisplay::Type() const
{
	return WgTextDisplay::GetClass();
}

const char * WgTextDisplay::GetClass()
{
	return c_widgetType;
}

//____ GoBOL() ________________________________________________________________
void WgTextDisplay::GoBOL()
{
	if( IsEditable() && m_bFocused )
		m_pText->goBOL();
}

//____ GoEOL() ________________________________________________________________
void WgTextDisplay::GoEOL()
{
	if( IsEditable() && m_bFocused )
		m_pText->goEOL();
}

//____ GoBOF() ________________________________________________________________
void WgTextDisplay::GoBOF()
{
	if( IsEditable() && m_bFocused )
		m_pText->goBOF();
}

//____ GoEOF() ________________________________________________________________
void WgTextDisplay::GoEOF()
{
	if( IsEditable() && m_bFocused )
		m_pText->goEOF();
}

//_______________________________________________________________
void WgTextDisplay::SetEditMode(WgTextEditMode mode)
{
	m_text.SetEditMode(mode);
}

//_______________________________________________________________
void WgTextDisplay::SetFixedBlendColor( wg::HiColor color )
{
	m_fixedBlendColor = color;
	_requestRender();
}

//____ MatchingPixelHeight() _______________________________________________________

int WgTextDisplay::MatchingPixelHeight( int width ) const
{
	int height = 0;

	if (m_pSkin)
		height = _skinPreferredSize( m_pSkin, m_scale).h;

	if (m_text.nbChars() != 0 || m_text.GetEditMode() == WgTextEditMode::Editable )
	{
		WgSize padding;

		if (m_pSkin)
			padding = _skinContentPadding( m_pSkin, m_scale);

		int heightForText = m_text.heightForWidth(width - padding.w) + padding.h;
		if (heightForText > height)
			height = heightForText;
	}

	return height;
}

//____ PreferredPixelSize() _____________________________________________________________

WgSize WgTextDisplay::PreferredPixelSize() const
{
	WgSize bestSize;

	if (m_pSkin)
		bestSize = _skinPreferredSize( m_pSkin, m_scale);


	if (m_text.nbChars() != 0 || m_text.GetEditMode() == WgTextEditMode::Editable )
	{
		WgSize textSize = m_text.unwrappedSize();

		if (m_pSkin)
			textSize += _skinContentPadding( m_pSkin, m_scale);

		if (textSize.w > bestSize.w)
			bestSize.w = textSize.w;

		if (textSize.h > bestSize.h)
			bestSize.h = textSize.h;
	}

	return bestSize;
}

//____ ChangeText() _____________________________________________________________

void WgTextDisplay::ChangeText(const wg::CharSeq& kStr)
{
    wg::String kStrTmp = wg::String(kStr);

    if(kStrTmp != m_kStr)
    {
        SetText(kStrTmp);
        m_kStr = kStrTmp;
    }
}

//____ ChangeText() _____________________________________________________________

void WgTextDisplay::ChangeText(std::string kStr)
{
    if(m_kStr != kStr)
    {
        wg::String kStrTmp = wg::String(kStr);
        SetText(kStrTmp);
        m_kStr = kStrTmp;
    }
}

//____ ChangeText() _____________________________________________________________

void WgTextDisplay::ChangeText(const char* kStr)
{
    if(m_kStr != kStr)
    {
        wg::String kStrTmp = wg::String(kStr);
        SetText(kStrTmp);
        m_kStr = kStrTmp;
    }
}

//____ GetPointerStyle() ________________________________________

WgPointerStyle WgTextDisplay::GetPointerStyle() const
{
	if( m_text.GetMarkedLink() )
		return WgPointerStyle::Hand;

	return m_pointerStyle;
}

//____ GetTooltipString() _____________________________________________________

//wg::String WgTextDisplay::GetTooltipString() const
//{
//	if( !m_tooltip.isEmpty() )
//		return m_tooltip;
//	else
//	{
//		WgSize sz = PixelSize();
//		if( sz.w < m_text.width() || sz.h < m_text.height() )
//			return m_text.getBuffer();
//	}
//
//	return 0;
//}

//____ _onRender() ________________________________________________________

void WgTextDisplay::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{

	WgWidget::_onRender(pDevice,_canvas,_window);

	WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, _canvas, m_state, m_scale) : _canvas;

	auto popData = ::wg::Util::limitClipList(pDevice, canvas*64);

    m_text.setState(m_state);       // TODO: _setState not called from SetSelected/SetNormal

	WgText * pText = &m_text;

	if( m_bFocused && IsEditable() )
		m_text.showCursor();
	else
		m_text.hideCursor();

	auto kChronoDiff = std::chrono::steady_clock::now() - m_kAnimTimeStart;
	auto kMillisecDiff = std::chrono::duration_cast<std::chrono::milliseconds>(kChronoDiff);

	if( m_fixedBlendColor != wg::HiColor::Undefined )
	{
		pDevice->setFixedBlendColor(m_fixedBlendColor);
		pDevice->setBlendMode(wg::BlendMode::BlendFixedColor);
		WgGfxDevice::PrintText( pDevice, pText, canvas );
		pDevice->setBlendMode(wg::BlendMode::Blend);
	}
	else if(m_bWrapTextAnim && kMillisecDiff.count() > 1000)
    {
        // Only animate if the text is longer than the canvas
        const int textCanvasDiff = (m_text.unwrappedWidth() - canvas.w);
        if(textCanvasDiff < 0)
        {
            WgGfxDevice::PrintText( pDevice, pText, canvas );
        }
		else
		{
	        WgRect canvas2 = {canvas.x, canvas.y, m_text.unwrappedWidth(), canvas.h};
			if(m_textAnimPos == -1)
			{
				m_textAnimPos = textCanvasDiff;
			}
			
			canvas2.x = canvas.x + (m_textAnimPos - textCanvasDiff);


            if(m_textAnimPos <= 0 && !m_reachedEnd)
            {
                m_kAnimTimeStopDelay = std::chrono::steady_clock::now();
				m_reachedEnd = true;
            }
            
            auto stopDelay = std::chrono::steady_clock::now() - m_kAnimTimeStopDelay;
            auto stopDelayDiff = std::chrono::duration_cast<std::chrono::milliseconds>(stopDelay);

			if(stopDelayDiff.count() > 1000 && m_textAnimPos <= 0)
			{
				// it's a speed among others
                m_textAnimPos = -1;
                m_kAnimTimeStart = std::chrono::steady_clock::now();
				m_reachedEnd = false;
			}
			else if( m_textAnimPos > 0)
			{
                m_textAnimPos = m_textAnimPos - m_textAnimSpeed;
			}

            WgGfxDevice::PrintText( pDevice, pText, canvas2 );
		}
    }
    else
    {
        WgGfxDevice::PrintText( pDevice, pText, canvas );
    }

	if(!m_bWrapTextAnim)
	{
		m_textAnimPos = -1;
	}

	if( pText != &m_text )
		delete pText;

	wg::Util::popClipList(pDevice, popData);

}

//____ _onRefresh() _______________________________________________________

void WgTextDisplay::_onRefresh( void )
{
	//TODO: Implement more I believe...

	_requestRender();
}

//____ _onEvent() ______________________________________________________________

void WgTextDisplay::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	bool bSwallowed = false;

	WgState	oldState = m_state;

	int type 				= pEvent->Type();
	WgModifierKeys modKeys 	= pEvent->ModKeys();

	if( type == WG_EVENT_TICK )
	{


		if( IsSelectable() && m_bFocused )
		{
			m_pText->incTime( ((const WgEvent::Tick*)(pEvent))->Millisec() );
			_requestRender();					//TODO: Should only render the cursor and selection!
		}

        if(m_bWrapTextAnim)
        {
            _requestRender();
        }

		return;
	}


	if (type == WG_EVENT_MOUSE_ENTER)
	{
        m_state.setHovered(true);

	}

	if (type == WG_EVENT_MOUSE_LEAVE)
	{
		m_state.setHovered(false);

	}

	if(type == WG_EVENT_MOUSEBUTTON_PRESS)
	{
		m_state.setPressed(true);
	}

	if (type == WG_EVENT_MOUSEBUTTON_RELEASE)
	{
		m_state.setPressed(false);
	}


	if( m_bFocused && (type == WG_EVENT_MOUSEBUTTON_PRESS || type == WG_EVENT_MOUSEBUTTON_DRAG) && ((const WgEvent::MouseButtonEvent*)(pEvent))->Button() == 1 )
	{

		if( IsSelectable() && (modKeys & WG_MODKEY_SHIFT) )
		{
			m_pText->setSelectionMode(true);
		}

		WgRect textCanvas = PixelSize();
		if( m_pSkin )
			textCanvas = _skinContentRect( m_pSkin, textCanvas, WgStateEnum::Default, m_scale );

		m_pText->CursorGotoCoord( pEvent->PointerPixelPos(), textCanvas );
		_bringCursorInView();

		if(IsSelectable() && type == WG_EVENT_MOUSEBUTTON_PRESS && !(modKeys & WG_MODKEY_SHIFT))
		{
			m_pText->clearSelection();
			m_pText->setSelectionMode(true);
		}

		bSwallowed = true;
	}
	else if( type == WG_EVENT_MOUSEBUTTON_RELEASE  )
	{
		if (m_bFocused && ((const WgEvent::MouseButtonEvent*)(pEvent))->Button() == 1)
		{
			m_pText->setSelectionMode(false);
			bSwallowed = true;
		}
	}
	else if( !m_bFocused && IsEditable() && type == WG_EVENT_MOUSEBUTTON_PRESS && ((const WgEvent::MouseButtonEvent*)(pEvent))->Button() == 1 )
	{
		GrabFocus();
		_bringCursorInView();
		bSwallowed = true;
	}


	if( type == WG_EVENT_CHARACTER )
	{
		if( IsEditable() && m_bFocused )
		{
			int  chr = static_cast<const WgEvent::Character*>(pEvent)->Char();

			if( chr >= 32 && chr != 127)
			{
				_insertCharAtCursor(chr);
			}
			else if( chr == 13 )
			{
				_insertCharAtCursor('\n');
			}
			else if( chr == '\t' && m_bTabLock )
			{
				_insertCharAtCursor( '\t' );
			}

			_bringCursorInView();
			bSwallowed = true;
		}
	}

	if( type == WG_EVENT_KEY_RELEASE && m_bFocused )
	{
		switch( static_cast<const WgEvent::KeyEvent*>(pEvent)->TranslatedKeyCode() )
		{
			case WG_KEY_SHIFT:
				if(!pHandler->IsMouseButtonPressed(1))
					m_pText->setSelectionMode(false);
				bSwallowed = true;
				break;
		}
	}

	if( (type == WG_EVENT_KEY_PRESS || type == WG_EVENT_KEY_REPEAT) && IsEditable() && m_bFocused )
	{
		const WgModKeyMap& modKeyMap = pHandler->GetModKeyMap();

		switch( static_cast<const WgEvent::KeyEvent*>(pEvent)->TranslatedKeyCode() )
		{
			case WG_KEY_LEFT:
				if( modKeys & WG_MODKEY_SHIFT )
					m_pText->setSelectionMode(true);

				if( modKeys & modKeyMap.stepWord )
					m_pText->gotoPrevWord();
				else if( modKeys & modKeyMap.beginEndLine )
					m_pText->goBOL();
				else
					m_pText->goLeft();
				bSwallowed = true;
				_bringCursorInView();
				break;
			case WG_KEY_RIGHT:
				if( modKeys & WG_MODKEY_SHIFT )
					m_pText->setSelectionMode(true);

				if( modKeys & modKeyMap.stepWord )
					m_pText->gotoNextWord();
				else if( modKeys & modKeyMap.beginEndLine )
					m_pText->goEOL();
				else
					m_pText->goRight();
				bSwallowed = true;
				_bringCursorInView();
				break;

			case WG_KEY_UP:
				if( modKeys & WG_MODKEY_SHIFT )
					m_pText->setSelectionMode(true);

				if( modKeys & modKeyMap.beginEndText )
					m_pText->goBOF();
				else
					m_pText->CursorGoUp( 1, ScreenPixelGeo() );
				bSwallowed = true;
				_bringCursorInView();
				break;

			case WG_KEY_DOWN:
				if( modKeys & WG_MODKEY_SHIFT )
					m_pText->setSelectionMode(true);

				if( modKeys & modKeyMap.beginEndText )
					m_pText->goEOF();
				else
					m_pText->CursorGoDown( 1, ScreenPixelGeo() );
				bSwallowed = true;
				_bringCursorInView();
				break;

			case WG_KEY_BACKSPACE:
				if(m_pText->hasSelection())
					m_pText->delSelection();
				else if( modKeys & modKeyMap.stepWord )
					m_pText->delPrevWord();
				else if( modKeys & modKeyMap.beginEndLine )
					m_pText->delToBOL();
				else
					m_pText->delPrevChar();
				bSwallowed = true;
				_bringCursorInView();
				break;

			case WG_KEY_DELETE:
				if(m_pText->hasSelection())
					m_pText->delSelection();
				else if( modKeys & modKeyMap.stepWord )
					m_pText->delNextWord();
				else if( modKeys & modKeyMap.beginEndLine )
					m_pText->delToEOL();
				else
					m_pText->delNextChar();
				bSwallowed = true;
				_bringCursorInView();
				break;

			case WG_KEY_HOME:
				if( modKeys & WG_MODKEY_SHIFT )
					m_pText->setSelectionMode(true);

				if( modKeys & WG_MODKEY_STD_CTRL )
					m_pText->goBOF();
				else
					m_pText->goBOL();
				bSwallowed = true;
				_bringCursorInView();
				break;

			case WG_KEY_END:
				if( modKeys & WG_MODKEY_SHIFT )
					m_pText->setSelectionMode(true);

				if( modKeys & WG_MODKEY_STD_CTRL )
					m_pText->goEOF();
				else
					m_pText->goEOL();
				bSwallowed = true;
				_bringCursorInView();
				break;

			default:
				break;
		}
	}

	// Let text object handle its actions.
/*
	bool bChanged = m_text.OnAction( action, button_key, ScreenPixelGeo(), WgCoord(info.x, info.y) );
	if( bChanged )
		RequestRender();
*/

	//

	if (oldState != m_state)
	{
        // If the state has changed we need to update since the
        // font might have states without having a skin
        //if( m_pSkin )
        //{
        //    _requestRender();
        //}
        
		m_text.setState(m_state);
        _requestRender();
    }

    // Forward event depending on rules.

	if (!bSwallowed)
	{
		WgWidget::_onEvent( pEvent, pHandler );
	}
}

//____ _bringCursorInView() _______________________________________________________

void WgTextDisplay::_bringCursorInView()
{
	WgCursorInstance * pCursor = m_pText->GetCursor();

	WgCoord cursorPos = { pCursor->ofsX(), pCursor->ofsY() };

	wg::TextAttr attr;
	m_pText->GetBaseAttr(attr);

	int size = int(attr.size) * m_scale >> WG_SCALE_BINALS;

    attr.pFont->setSize(size);
    int glyphHeight = attr.pFont->maxAdvance()/64 + attr.pFont->maxDescend()/64;
    int glyphWidth = attr.pFont->whitespaceAdvance()/64;

	if(m_pSkin)
		cursorPos += _skinContentOfs( m_pSkin, m_state, m_scale);

	WgRect mustHaveArea = { cursorPos.x - glyphWidth, cursorPos.y, glyphWidth*2, glyphHeight };
	WgRect niceToHaveArea = { mustHaveArea.x, mustHaveArea.y - glyphHeight/2, mustHaveArea.w, mustHaveArea.h + glyphHeight };

	WgRect canvas = WgRect( 0,0, PixelSize() );
	_requestInView( WgRect::overlap(mustHaveArea,canvas), WgRect::overlap(niceToHaveArea, canvas) );
}

//____ _onCloneContent() _______________________________________________________

void WgTextDisplay::_onCloneContent( const WgWidget * _pOrg )
{
	//TODO: Implement!
}

//____ _onAlphaTest() ______________________________________________________

bool WgTextDisplay::_onAlphaTest( const WgCoord& ofs )
{
	return true;																				// Accept all at least for now...
}

//____ _setState() ________________________________________________________

void WgTextDisplay::_setState(WgState state)
{
    //    WgWidget::_setState(state);

    m_text.setState(m_state);
    _requestRender(); //TODO: Only requestRender if text appearance has changed (let m_text.setState() return if rendering is needed)
}

//____ _onEnable() ___________________________________________________

void WgTextDisplay::_onEnable( void )
{
	m_text.setState(wg::State::Default);
	_requestRender();
}

//____ _onDisable() ___________________________________________________

void WgTextDisplay::_onDisable( void )
{
	m_text.setState(wg::State::Disabled);
	_requestRender();
}

//____ _onNewSize() ________________________________________________

void WgTextDisplay::_onNewSize( const WgSize& size )
{
	WgRect	contentRect(0, 0, PixelSize());

	if (m_pSkin)
		contentRect = _skinContentRect( m_pSkin, contentRect, WgStateEnum::Default, m_scale);

	m_text.setLineWidth( contentRect.w );
}


//____ _onGotInputFocus() ______________________________________________

void WgTextDisplay::_onGotInputFocus()
{
	WgWidget::_onGotInputFocus();

	if( IsEditable() ) // render with cursor on
	{
		_startReceiveTicks();
		if(	m_bResetCursorOnFocus )
			m_pText->goEOF();
		_requestRender();
	}
}

//____ _onLostInputFocus() ______________________________________________

void WgTextDisplay::_onLostInputFocus()
{
	WgWidget::_onLostInputFocus();

	m_bResetCursorOnFocus = false;
	if( IsEditable() )
	{
		_queueEvent(new WgEvent::TextSet(this, m_pText));
		_stopReceiveTicks();
		_requestRender();
		_queueEvent(new WgEvent::TextSet(this, m_pText));
	}
}

//____ _setScale() _____________________________________________________________

void WgTextDisplay::_setScale( int scale )
{
	WgWidget::_setScale(scale);

	m_text.SetScale(scale);

	WgRect    contentRect(0, 0, PixelSize());

	if (m_pSkin)
		contentRect = _skinContentRect(m_pSkin, contentRect, WgStateEnum::Default, m_scale);

	m_text.setLineWidth( contentRect.w );

	_requestResize();
}



//____ _textModified() _________________________________________________________

void WgTextDisplay::_textModified()
{
	m_bResetCursorOnFocus = true;
	_requestResize();
	_requestRender();
	_queueEvent(new WgEvent::TextModify(this, m_pText));
}

//____ InsertTextAtCursor() ___________________________________________________

Uint32 WgTextDisplay::InsertTextAtCursor( const wg::CharSeq& str )
{
	if( !IsEditable() )
		return 0;

	if( m_bFocused )
		if( !GrabFocus() )
			return 0;				// Couldn't get input focus...

	Uint32 nChars = m_pText->putText( str );

	if( m_maxLines != 0 && m_maxLines < (int) m_pText->nbSoftLines() )
	{
		m_pText->unputText( nChars );
		nChars = 0;
	}

	return nChars;
}

//____ InsertCharAtCursor() ___________________________________________________

bool WgTextDisplay::InsertCharAtCursor( Uint16 c )
{
	if( !IsEditable() )
		return 0;

	if( m_bFocused )
		if( !GrabFocus() )
			return false;				// Couldn't get input focus...

	return _insertCharAtCursor(c);
}

//____ _insertCharAtCursor() ___________________________________________

bool WgTextDisplay::_insertCharAtCursor( Uint16 c )
{
	if(m_pText->hasSelection())
		m_pText->delSelection();
	m_pText->setSelectionMode(false);

	if( m_pText->putChar( c ) == false )
		return false;

	if( m_maxLines != 0 && m_maxLines < (int) m_pText->nbSoftLines() )
	{
		m_pText->delPrevChar();
		return false;
	}

	return true;
}
