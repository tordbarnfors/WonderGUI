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

#include <wg2_lineeditor.h>

#include	<wg2_key.h>
#include	<wg_font.h>
#include 	<wg2_gfxdevice.h>
#include 	<wg2_pen.h>
#include 	<wg2_eventhandler.h>
#include    <wg2_base.h>

#include <algorithm>

static const char	c_widgetType[] = {"LineEditor"};

//____ Constructor ____________________________________________________________

WgLineEditor::WgLineEditor()
{
	m_pText			= &m_text;
	m_text.setHolder( this );
	m_text.SetWrap(false);
	m_text.SetAutoEllipsis(IsAutoEllipsisDefault());
	m_text.SetEditMode( WgTextEditMode::Editable );
	m_bPasswordMode = false;
	m_pwGlyph		= '*';
	m_viewOfs		= 0;
	m_pointerStyle	= WgPointerStyle::Ibeam;
	m_bResetCursorOnFocus = true;
}

//____ Destructor _____________________________________________________________

WgLineEditor::~WgLineEditor()
{
}

//____ Type() _________________________________________________________________

const char * WgLineEditor::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgLineEditor::GetClass()
{
	return c_widgetType;
}

//____ SetEditMode() __________________________________________________________

void WgLineEditor::SetEditMode(WgTextEditMode mode)
{
	m_text.SetEditMode( mode );

	if( _isSelectable() )
	{
		m_pointerStyle = WgPointerStyle::Ibeam;
	}
	else
	{
		m_pointerStyle = WgPointerStyle::Arrow;
	}
}

//____ GoBOL() ________________________________________________________________

void WgLineEditor::GoBOL()
{
	if( _isEditable() && m_bFocused )
		m_pText->goBOL();
}

//____ GoEOL() ________________________________________________________________

void WgLineEditor::GoEOL()
{
	if( _isEditable() && m_bFocused )
		m_pText->goEOL();
}

//____ SetPasswordGlyph() _____________________________________________________

void WgLineEditor::SetPasswordGlyph( Uint16 glyph )
{
	assert(glyph);
	if(glyph)
	{
		m_pwGlyph = glyph;
	}
}

//____ InsertTextAtCursor() ___________________________________________________

Uint32 WgLineEditor::InsertTextAtCursor( const wg::CharSeq& str )
{
	if( !_isEditable() )
		return 0;

	if( !m_bFocused )
		if( !GrabFocus() )
			return 0;				// Couldn't get input focus...

	return m_pText->putText( str );
}

//____ InsertCharAtCursor() ___________________________________________________

bool WgLineEditor::InsertCharAtCursor( Uint16 c )
{
	if( !_isEditable() )
		return 0;

	if( !m_bFocused )
		if( !GrabFocus() )
			return false;				// Couldn't get input focus...

	if( !m_pText->putChar( c ) )
		return false;

	return true;
}


//____ SetTextWrap() __________________________________________________________

bool WgLineEditor::SetTextWrap(bool bWrap)
{
	return !bWrap;
}

//____ PreferredPixelSize() __________________________________________________________

WgSize WgLineEditor::PreferredPixelSize() const
{
	WgSize sz = m_text.unwrappedSize();
	if (m_pSkin)
		sz = _skinSizeForContent(m_pSkin, sz, m_scale);
	return sz;
}


//____ _onCloneContent() _______________________________________________________

void WgLineEditor::_onCloneContent( const WgWidget * _pOrg )
{
	WgLineEditor * pOrg = (WgLineEditor*) _pOrg;

	Wg_Interface_TextHolder::_onCloneContent( pOrg );

	//TODO: Implement!
}

//____ _onRender() _____________________________________________________________

void WgLineEditor::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	WgWidget::_onRender(pDevice, _canvas, _window);

	WgRect textCanvas = m_pSkin ? _skinContentRect( m_pSkin, _canvas, WgStateEnum::Default, m_scale) : _canvas;

	WgText * pText = &m_text;
	if( m_bPasswordMode )
	{
		int nChars = m_text.nbChars();
		Uint16 * pContent = new Uint16[nChars+1];
		for( int i = 0 ; i < nChars ; i++ )
			pContent[i] = m_pwGlyph;
		pContent[nChars] = 0;
		pText = new WgText( pContent );
		delete [] pContent;

		pText->SetWrap(false);
		pText->SetAutoEllipsis(false);
		pText->SetScale(m_text.Scale());
		pText->setAlignment(m_text.alignment());
		pText->setStyle(m_text.getStyle());
		pText->setState(m_text.state());

		pText->SetEditMode(m_text.GetEditMode());
		pText->showCursor();
		pText->gotoSoftPos( m_text.line(), m_text.column() );
		pText->incTime( m_text.time() );

		int sl, sc, el, ec;
		if( m_text.getSelection(sl, sc, el, ec) )
			pText->selectText(sl, sc, el, ec);

	}

	WgRect r = textCanvas;
	r.x -= m_viewOfs;
	r.w += m_viewOfs;

	if( m_bFocused && _isEditable() )
		pText->showCursor();
	else
		pText->hideCursor();

	WgGfxDevice::PrintText( pDevice, pText, r );

	if( pText != &m_text )
		delete pText;
}

//____ _onEvent() ______________________________________________________________

void WgLineEditor::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	WgEventType event = pEvent->Type();

	if( event == WG_EVENT_TICK )
	{
		if( _isSelectable() && m_bFocused )
		{
			m_pText->incTime( static_cast<const WgEvent::Tick*>(pEvent)->Millisec() );
			_requestRender();					//TODO: Should only render the cursor and selection!
		}
		return;
	}

	if( (event == WG_EVENT_MOUSEBUTTON_PRESS || event == WG_EVENT_MOUSEBUTTON_DRAG) && static_cast<const WgEvent::MouseButtonEvent*>(pEvent)->Button() == 1 )
	{
		if( !m_bFocused )
			GrabFocus();

		if( m_bFocused )
		{
			if( _isSelectable() && (pEvent->ModKeys() & WG_MODKEY_SHIFT) )
			{
				m_pText->setSelectionMode(true);
			}

			WgCoord ofs = pEvent->PointerPixelPos();
			int x = ofs.x + m_viewOfs;
			int y = 0;

			if (m_pSkin)
				x -= _skinContentOfs( m_pSkin, WgStateEnum::Default, m_scale).x;

			if( m_bPasswordMode )
			{
				wg::TextAttr	attr;
				m_pText->GetBaseAttr( attr );

				WgPen	pen;
                pen.SetScale(m_scale);
				pen.SetAttributes( attr );
				pen.SetChar(m_pwGlyph);
				pen.AdvancePos();

				int spacing = pen.GetPosX();
				int height = pen.GetLineSpacing();

				int line = y/height;
				int col = (x+spacing/2)/spacing;
				if(col < 0)
				{
					col = 0;
					line = 0;
				}
				m_pText->gotoSoftPos(line,col);
			}
			else
			{
				m_pText->CursorGotoCoord( WgCoord(x, 0), WgRect(0,0,1000000,1000000) );
			}

			if(_isSelectable() && event == WG_EVENT_MOUSEBUTTON_PRESS && !(pEvent->ModKeys() & WG_MODKEY_SHIFT))
			{
				m_pText->clearSelection();
				m_pText->setSelectionMode(true);
			}
		}
		_adjustViewOfs();
	}

	if( event == WG_EVENT_MOUSEBUTTON_RELEASE )
	{
		if( m_bFocused && static_cast<const WgEvent::MouseButtonEvent*>(pEvent)->Button() == 1 )
			m_pText->setSelectionMode(false);
	}

	if( event == WG_EVENT_CHARACTER )
	{
		int ch = static_cast<const WgEvent::Character*>(pEvent)->Char();

		if( _isEditable() && m_bFocused && ch >= 32 && ch != 127)
		{

			if(m_pText->hasSelection())
				m_pText->delSelection();
			m_pText->setSelectionMode(false);

			m_pText->putChar(ch);
		}
	}

	if( event == WG_EVENT_KEY_RELEASE && m_bFocused )
	{
		int key = static_cast<const WgEvent::KeyEvent*>(pEvent)->TranslatedKeyCode();
		switch( key )
		{
			case WG_KEY_SHIFT:
				if(!pHandler->IsMouseButtonPressed(1))
					m_pText->setSelectionMode(false);
			break;
		}
	}

	if( (event == WG_EVENT_KEY_PRESS || event == WG_EVENT_KEY_REPEAT) && _isEditable() && m_bFocused )
	{
		const WgModKeyMap& modKeyMap = pHandler->GetModKeyMap();

		int key = static_cast<const WgEvent::KeyEvent*>(pEvent)->TranslatedKeyCode();
		switch( key )
		{
			case WG_KEY_LEFT:
				if( pEvent->ModKeys() & WG_MODKEY_SHIFT )
					m_pText->setSelectionMode(true);

				if( pEvent->ModKeys() & modKeyMap.stepWord )
				{
					if( m_bPasswordMode )
						m_pText->goBOL();
					else
						m_pText->gotoPrevWord();
				}
				else if( pEvent->ModKeys() & modKeyMap.beginEndLine )
				{
					m_pText->goBOL();
				}
				else
				{
					m_pText->goLeft();
				}
				break;
			case WG_KEY_RIGHT:
				if( pEvent->ModKeys() & WG_MODKEY_SHIFT )
					m_pText->setSelectionMode(true);

				if( pEvent->ModKeys() & modKeyMap.stepWord )
				{
					if( m_bPasswordMode )
						m_pText->goEOL();
					else
						m_pText->gotoNextWord();
				}
				else if( pEvent->ModKeys() & modKeyMap.beginEndLine )
				{
						m_pText->goEOL();
				}
				else
				{
					m_pText->goRight();
				}
				break;

			case WG_KEY_BACKSPACE:
			{
				if(m_pText->hasSelection())
					m_pText->delSelection();
				else if( (pEvent->ModKeys() & modKeyMap.stepWord) && !m_bPasswordMode)
					m_pText->delPrevWord();
				else if( (pEvent->ModKeys() & modKeyMap.beginEndLine) && !m_bPasswordMode)
					m_pText->delToBOL();
				else
					m_pText->delPrevChar();
				break;
			}

			case WG_KEY_DELETE:
			{
				if(m_pText->hasSelection())
					m_pText->delSelection();
				else if( (pEvent->ModKeys() & modKeyMap.stepWord) && !m_bPasswordMode)
					m_pText->delNextWord();
				else if( (pEvent->ModKeys() & modKeyMap.beginEndLine) && !m_bPasswordMode)
					m_pText->delToEOL();
				else
					m_pText->delNextChar();
				break;
			}

			case WG_KEY_HOME:

				/*
				 *	I am not sure if this is the proper way to this, but in my opinion, the default
				 *	"actions" has to be separated from any modifier key action combination
				 */
				switch( pEvent->ModKeys() )
				{

				case WG_MODKEY_STD_CTRL:
					break;

				default: // no modifier key was pressed
					if(pEvent->ModKeys() & WG_MODKEY_SHIFT )
						m_pText->setSelectionMode(true);

					m_pText->goBOL();
					break;
				}

				break;

			case WG_KEY_END:

				/*
			 	 *	I am not sure if this is the proper way to this, but in my opinion, the default
		 		 *	"actions" has to be separated from any modifier key action combination
				 */
				switch( pEvent->ModKeys() )
				{

				case WG_MODKEY_STD_CTRL:
					break;

				default: // no modifier key was pressed
					if( pEvent->ModKeys() & WG_MODKEY_SHIFT )
						m_pText->setSelectionMode(true);

					m_pText->goEOL();
					break;
				}

				break;

			default:
				break;
		}
		_adjustViewOfs();
	}

	// Forward event depending on rules.

	if( pEvent->IsMouseButtonEvent() )
	{
		if( static_cast<const WgEvent::MouseButtonEvent*>(pEvent)->Button() != 1 )
			WgWidget::_onEvent(pEvent,pHandler);
	}
	else if( pEvent->IsKeyEvent() )
	{
		int key = static_cast<const WgEvent::KeyEvent*>(pEvent)->TranslatedKeyCode();
		if( static_cast<const WgEvent::KeyEvent*>(pEvent)->IsMovementKey() == false &&
			key != WG_KEY_DELETE && key != WG_KEY_BACKSPACE )
			WgWidget::_onEvent(pEvent,pHandler);

		//TODO: Would be good if we didn't forward any character-creating keys either...
	}
	else if( event != WG_EVENT_CHARACTER )
		WgWidget::_onEvent(pEvent,pHandler);
}


//____ _adjustViewOfs() ________________________________________________________

void WgLineEditor::_adjustViewOfs()
{
	// Possibly move viewOfs so that:
	//	1 Cursor remains inside view.
	//  2 At least one character is displayed before the cursor
	//  3 At least one character is displayed after the cursor (if there is one).

	if( m_bFocused && m_pText->getFont() )
	{
		WgCursor * pCursor = m_pText->getCursorStyle();
		if( !pCursor )
			pCursor = WgBase::GetDefaultCursor();

		if( !pCursor )
			return;

		int cursCol	= m_pText->column();

		wg::TextAttr	attr;
		m_pText->GetBaseAttr( attr );

		WgPen	pen;
        pen.SetScale(m_scale);
		pen.SetAttributes( attr );
		pen.SetChar(m_pwGlyph);
		pen.AdvancePos();

		int pwAdvance	= pen.GetPosX();
		int cursAdvance	= pCursor->Advance(m_pText->cursorMode() );
		int cursBearing	= pCursor->BearingX(m_pText->cursorMode() );
		int cursWidth	= pCursor->Width(m_pText->cursorMode() );

		int cursOfs;		// Cursor offset from beginning of line in pixels.
		int maxOfs;			// Max allowed view offset in pixels.
		int minOfs;			// Min allowed view offset in pixels.

		int geoWidth = PixelSize().w;
		int	lineWidth = m_pText->getSoftLineWidth( 0 ) + cursBearing+cursWidth;

		// Calculate cursOfs

		if( m_bPasswordMode )
			cursOfs = cursCol * pwAdvance;
		else
			cursOfs	= m_pText->getSoftLineWidthPart( 0, 0, cursCol );

		// Calculate maxOfs

		if( cursCol > 0 )
		{
			if( m_bPasswordMode )
				maxOfs = (cursCol-1) * pwAdvance;
			else
				maxOfs = m_pText->getSoftLineWidthPart( 0, 0, cursCol-1 );

			if( lineWidth < maxOfs + geoWidth )
				maxOfs = std::max( lineWidth - geoWidth, 0 );
		}
		else
			maxOfs = cursOfs;


		// Calculate minOfs

		if( cursCol < m_pText->getLine(0)->nChars )
		{
			if( m_bPasswordMode )
				minOfs = (cursCol+1) * pwAdvance + cursAdvance - geoWidth;
			else
				minOfs = m_pText->getSoftLineWidthPart( 0, 0, cursCol+1 ) + cursAdvance - geoWidth;	// Not 100% right, cursor might affect linewidth different from its own width.
		}
		else
			minOfs = cursOfs + cursBearing + cursWidth - geoWidth;

		// Check boundaries and update

		if( m_viewOfs > maxOfs )
			m_viewOfs = maxOfs;

		if( m_viewOfs < minOfs )
			m_viewOfs = minOfs;


	}
	else
		m_viewOfs = 0;				// Show beginning of line when cursor disappears.
}



//____ _onEnable() _____________________________________________________________

void WgLineEditor::_onEnable()
{
    if( m_bFullStateSupport )
    {
        WgState state = m_state;
        state.setDisabled(false);
        _setState(state);
    }
    else
    {
        m_text.setState(wg::State::Default);
        _requestRender();
    }
}

//____ _onDisable() ____________________________________________________________

void WgLineEditor::_onDisable()
{
    if( m_bFullStateSupport )
    {
        WgState state = m_state;
        state.setDisabled(true);
        _setState(state);
    }
    else
    {
        m_text.setState(wg::State::Disabled);
        _requestRender();
    }
}

//____ _onGotInputFocus() ______________________________________________________

void WgLineEditor::_onGotInputFocus()
{
	WgWidget::_onGotInputFocus();

	if( _isEditable() )
	{
		_startReceiveTicks();
		if( m_bResetCursorOnFocus )
			m_pText->goEOL();
		_requestRender(); // render with cursor on
	}
}

//____ _onLostInputFocus() _____________________________________________________

void WgLineEditor::_onLostInputFocus()
{
	WgWidget::_onLostInputFocus();

	if( _isSelectable() )
	{
		m_pText->clearSelection();
		m_pText->setSelectionMode(false);
		m_bResetCursorOnFocus = false;
	}

	if( _isEditable() || m_viewOfs != 0 )
	{
		_stopReceiveTicks();
		_queueEvent( new WgEvent::TextSet(this, m_pText) );

		m_viewOfs = 0;
		_requestRender();
	}
}

//____ _onNewSize() ____________________________________________________________

void WgLineEditor::_onNewSize( const WgSize& size )
{
	_adjustViewOfs();
	_requestRender();
}

//____ _setScale() _____________________________________________________________

void WgLineEditor::_setScale( int scale )
{
	WgWidget::_setScale(scale);

	m_text.SetScale(scale);
	_requestResize();
}


//____ _textModified() _________________________________________________________

void WgLineEditor::_textModified()
{
	m_bResetCursorOnFocus = true;			// Any change to text while we don't have focus resets the position.
	_requestResize();
	_requestRender();
	_adjustViewOfs();
	_queueEvent(new WgEvent::TextModify(this, m_pText));
}

//____ _setState() _____________________________________________________________

void WgLineEditor::_setState( WgState state )
{
    m_text.setState(state);
    
    WgWidget::_setState(state);
}

