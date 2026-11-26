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
#include <algorithm>
#include <wg_editabletext.h>
#include <wax_utf8.h>
#include <wg_msg.h>
#include <wg_inputhandler.h>
#include <wg_util.h>

// TODO: Go to next/previous word.
// TODO: Correctly render selected text.
// TODO: Move cursor by mouse, with selection.
// TODO: Optimize rendering.
// TODO: Optimize text changes.
// TODO: Double/tripple click to select word/line
// TODO: Support for cut/copy/paste
// TODO: Support for undo/redo
// TODO: Cursor have correct size.
// TODO: Check that we request resize properly.


namespace wg
{
	using namespace Util;

	const TypeInfo	EditableText::TYPEINFO = { "EditableText", &DynamicText::TYPEINFO };


	//____ constructor _____________________________________________________________

	EditableText::EditableText(Widget * pWidget ) : DynamicText(pWidget)
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& EditableText::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setMaxLines() _________________________________________________________

	bool EditableText::setMaxLines(int maxLines)
	{
		if (maxLines < 0)
			return false;

		m_maxLines = maxLines;

		//TODO: Cut text and refresh if larger than limit.

		return true;
	}

	//____ setMaxChars() _________________________________________________________

	bool EditableText::setMaxChars(int maxChars)
	{
		if (maxChars < 0)
			return false;

		m_maxChars = maxChars;

		//TODO: Cut text and refresh if larger than limit.

		return true;
	}

	//____ _initFromBlueprint() _______________________________________________

	void EditableText::_initFromBlueprint(const Blueprint& bp)
	{
		if (bp.style)
			setStyle(bp.style);

		if (bp.layout)
			setLayout(bp.layout);

		if (!bp.text.empty())
			_setText(bp.text);

		m_editMode = bp.editMode;
		m_maxChars = bp.maxChars;
		m_maxLines = bp.maxLines;
	}



	//____ _receive() ___________________________________________________________

	void EditableText::_receive( Msg * pMsg )
	{
		DynamicText::_receive( pMsg );

		MsgType type = pMsg->type();

		switch( type )
		{
			case MsgType::KeyPress:
			case MsgType::KeyRepeat:
			{
				if( m_editState.bCaret )
				{
					KeyMsg * p = static_cast<KeyMsg*>(pMsg);
					ModKeys modKeys = p->modKeys();

					switch( p->translatedKeyCode() )
					{
					case Key::Shift:
						m_editState.bShiftDown = true;
						break;
					case Key::Left:
						if( modKeys & ModKeys::StdCtrl )
							caretPrevWord();
						else
							caretLeft();
						break;
					case Key::Right:
						if( modKeys & ModKeys::StdCtrl)
							caretNextWord();
						else
							caretRight();
						break;

					case Key::Up:
							caretUp();
						break;

					case Key::Down:
							caretDown();
						break;

					case Key::Backspace:
						if( hasSelection() )
							eraseSelected();
						else if( modKeys & ModKeys::StdCtrl)
							caretErasePrevWord();
						else
							caretErasePrevChar();
						break;

					case Key::Delete:
						if( hasSelection() )
							eraseSelected();
						else if( modKeys & ModKeys::StdCtrl)
							caretEraseNextWord();
						else
							caretEraseNextChar();
						break;

					case Key::Home:
						if( modKeys & ModKeys::StdCtrl)
							caretTextBegin();
						else
							caretLineBegin();
						break;

					case Key::End:
						if( modKeys & ModKeys::StdCtrl)
							caretTextEnd();
						else
							caretLineEnd();
						break;

					case Key::Return:
						caretPut( "\n" );
						break;

					case Key::PageUp:
						caretPageUp();
						break;

					case Key::PageDown:
						caretPageDown();
						break;

					default:
						break;

					}

				}
				break;
			}

			case MsgType::KeyRelease:
				if(static_cast<KeyMsg*>(pMsg)->translatedKeyCode() == Key::Shift )
					m_editState.bShiftDown = false;
				break;

			case MsgType::TextInput:
				caretPut(static_cast<TextInputMsg*>(pMsg)->text() );
				break;

			case MsgType::EditCommand:
			{
				EditCmd cmd = static_cast<EditCommandMsg*>(pMsg)->command();

				switch( cmd )
				{
					case EditCmd::Cut:
						if( hasSelection() )
						{
							Base::setClipboardText(getSelection());
							eraseSelected();
						}
					break;

					case EditCmd::Copy:
						if( hasSelection() )
						{
							Base::setClipboardText(getSelection());
						}
					break;

					case EditCmd::Paste:
						if( m_editState.bCaret )
						{
							caretPut(Base::getClipboardText());
						}
					break;

					case EditCmd::Undo:
					break;

					case EditCmd::Redo:
					break;

					case EditCmd::SelectAll:
						selectAll();
					break;

					default:
					break;
				}

				break;
			}

			case MsgType::MousePress:			// Move caret pos and selection pos. Shift-press: only move caret pos.
			{
				auto p = static_cast<MousePressMsg*>(pMsg);
				if( p->button() == MouseButton::Left && m_editMode != TextEditMode::Static )
				{
					if( m_state.isFocused() )
					{
						caretToPos( _toLocal(p->pointerSpxPos()) );
						m_editState.bButtonDown = true;
					}
					else
						_requestFocus(true);
				}
				break;
			}

			case MsgType::MouseDrag:			// Move only caret pos
			{
				auto p = static_cast<MouseDragMsg*>(pMsg);
				if( p->button() == MouseButton::Left && m_editMode != TextEditMode::Static )
					if( m_state.isFocused() )
						caretToPos( _toLocal(p->pointerSpxPos()) );
				break;
			}

			case MsgType::MouseRelease:
			{
				auto p = static_cast<MouseReleaseMsg*>(pMsg);
				if( p->button() == MouseButton::Left && m_editMode != TextEditMode::Static )
					m_editState.bButtonDown = false;
				break;
			}
			case MsgType::MouseDoubleClick:		// Select word
			{
				auto p = static_cast<MouseDoubleClickMsg*>(pMsg);
				if( p->button() == MouseButton::Left && m_editMode != TextEditMode::Static )
					if( m_state.isFocused() )
						caretSelectWord();
				break;
			}

			default:
				break;

		}
	}



	//____ _clear() _________________________________________________________________

	void EditableText::_clear()
	{
		m_editState.caretOfs = 0;
		m_editState.selectOfs = 0;
		m_editState.wantedOfs = -1;

		DynamicText::_clear();
		_updateDisplayArea();
	}

	//____ _setText() ___________________________________________________________________

	void EditableText::_setText( std::string_view text )
	{
		//TODO: Cut sequence if too many lines or chars.

		DynamicText::_setText( text );
		_caretToEnd();
	}

	//____ _append() ________________________________________________________________

	int EditableText::_append( std::string_view text )
	{
		//TODO: Cut sequence if too many lines or chars.

		// Appending text should move carret if carret is at end of text and nothing is selected,
		// otherwise not affect caret or selection.

		if( m_editState.caretOfs == m_editState.selectOfs && m_editState.caretOfs == int(m_text.size()) )
		{
			int added = DynamicText::_append( text );
			int newOfs = m_editState.caretOfs + added;
			_layout()->caretMove( this, newOfs, m_editState.caretOfs );

			m_editState.caretOfs = newOfs;
			m_editState.selectOfs = newOfs;
			m_editState.wantedOfs = -1;
			_updateDisplayArea();
			return added;
		}

		return DynamicText::_append( text );
	}

	//____ _insert() ________________________________________________________________

	int EditableText::_insert( int ofs, std::string_view text )
	{
		//TODO: Cut sequence if too many lines or chars.

		int len = 0;
		_clampRange( ofs, len );

		int added = DynamicText::_insert(ofs,text);

		/* Inserting text should affect the selection as little as possible. Therefore:
		 *
		 * Selected text remains selected when new is inserted before or after.
		 * If text is inserted into the selection, the selection grows.
		 *
		 * Inserting text to position of caretOfs or selectOfs is considered to
		 * insert the text before or after the selection, it will not grow the selection.
		 *
		 * Inserting text to caretOfs will move caretOfs only if no text is selected.
		 */

		int caretOfs = m_editState.caretOfs;
		int selectOfs = m_editState.selectOfs;

		int selBeg = std::min(caretOfs,selectOfs);
		int selEnd = std::max(caretOfs,selectOfs);

		if( ofs <= selBeg )				// move selection or caret
		{
			caretOfs += added;
			selectOfs += added;
		}
		else if( ofs < selEnd )			// grow selection
		{
			if( caretOfs == selEnd )
				caretOfs += added;
			else
				selectOfs += added;
		}

		if( caretOfs != m_editState.caretOfs || selectOfs != m_editState.selectOfs )
		{
			if( caretOfs == selectOfs )
				_layout()->caretMove( this, caretOfs, m_editState.caretOfs );
			else
				_layout()->selectionChange( this, selectOfs, caretOfs, m_editState.caretOfs, m_editState.selectOfs );

			m_editState.caretOfs = caretOfs;
			m_editState.selectOfs = selectOfs;
			m_editState.wantedOfs = -1;
			_updateDisplayArea();
		}
		return added;
	}

	//____ _replace() _______________________________________________________________

	int EditableText::_replace( int ofs, int nDelete, std::string_view text )
	{
		//TODO: Cut sequence if too many lines or chars.

		_clampRange( ofs, nDelete );

		int sizeModif = DynamicText::_replace(ofs,nDelete,text);

		// Replacing text should not affect caret or selection except where necessary.

		if( m_editState.caretOfs >= ofs + nDelete )
			m_editState.caretOfs += sizeModif;
		else if( m_editState.caretOfs > ofs )
			m_editState.caretOfs = ofs;

		if( m_editState.selectOfs >= ofs + nDelete )
			m_editState.selectOfs += sizeModif;
		else if( m_editState.selectOfs > ofs )
			m_editState.selectOfs = ofs;

		_updateDisplayArea();
		return sizeModif;
	}

	//____ _erase() ________________________________________________________________

	int EditableText::_erase( int ofs, int len )
	{
		_clampRange( ofs, len );

		/* Inserting text should affect the selection as little as possible. Therefore:
		 *
		 * All text that was selected before any text was erased and wasn't erased stays selected.
		 *
		 * Caret stays where it is if possible. If caret position was erased, caret is moved to
		 * first character after erased section.
		 */

		int caretOfs = m_editState.caretOfs;
		if( caretOfs > ofs )
		{
			if( caretOfs < ofs + len )
				caretOfs = ofs;
			else
				caretOfs -= len;
		}

		int selectOfs = m_editState.selectOfs;
		if( selectOfs > ofs )
		{
			if( selectOfs < ofs + len )
				selectOfs = ofs;
			else
				selectOfs -= len;
		}

		if( caretOfs != m_editState.caretOfs || selectOfs != m_editState.selectOfs )
		{
			if( m_editState.caretOfs == m_editState.selectOfs )
				_layout()->caretMove( this, caretOfs, m_editState.caretOfs );
			else
				_layout()->selectionChange( this, selectOfs, caretOfs, m_editState.selectOfs, m_editState.caretOfs );

			m_editState.caretOfs = caretOfs;
			m_editState.selectOfs = selectOfs;
			m_editState.wantedOfs = -1;
			_updateDisplayArea();
		}

		int ret = DynamicText::_erase( ofs, len );
		_updateDisplayArea();
		return ret;
	}

	//____ _setState() ______________________________________________________________

	void EditableText::_setState( State state )
	{
		if( state.isFocused() != m_state.isFocused() && m_editMode == TextEditMode::Editable )
		{
			if( state.isFocused() )
			{
				// Enable and place caret

				m_editState.bCaret = true;
//				m_editState.caretOfs = int(m_text.size());		//TODO: Not always reset caretOfs.
//				m_editState.selectOfs = m_editState.caretOfs;
				m_editState.wantedOfs = -1;

				// Check modifier keys, update status

				ModKeys modKeys = Base::inputHandler()->modifierKeys();

				if( modKeys & ModKeys::Shift )
					m_editState.bShiftDown = true;
				else
					m_editState.bShiftDown = false;

				// Restart caret animation

				_layout()->caretMove(this, m_editState.caretOfs, m_editState.caretOfs);
			}
			else
			{
				_requestRender(_layout()->rectForCaret(this));				
				m_editState.bCaret = false;
			}

		}


		// Set this last, so that bCaret is set when we call

		DynamicText::_setState(state);
	}


	//____ render() ______________________________________________________________

	void EditableText::_render( GfxDevice * pDevice, const RectSPX& canvas )
	{
		_layout()->render(this, pDevice, canvas );
	}

	//____ setEditMode() _______________________________________________________

	void EditableText::setEditMode( TextEditMode mode )
	{
		if( mode == m_editMode )
			return;
		
		switch( mode )
		{
			case TextEditMode::Static:
				m_editState.bCaret = false;
				m_editState.selectOfs = 0;
				m_editState.caretOfs = 0;
				m_editState.wantedOfs = -1;
				break;
				
			case TextEditMode::Selectable:
				m_editState.bCaret = false;
				m_editState.wantedOfs = -1;
				break;
			case TextEditMode::Editable:
				if( m_editState.selectOfs != m_editState.caretOfs )
				{
					//TODO: Do we need to do something here?
				}
			break;
		}

		m_editMode = mode;
	}

	//____ select() ____________________________________________________________

	bool EditableText::select( int beg, int end )
	{
		if( m_editMode == TextEditMode::Static )
			return false;

		int max = int(m_text.size());

		limit( beg, 0, max );
		limit( end, 0, max );

		beg = Utf8::alignToCodepoint( m_text, beg );
		end = Utf8::alignToCodepoint( m_text, end );

		if( beg != m_editState.selectOfs || end != m_editState.caretOfs )
		{
			_layout()->selectionChange( this, beg, end, m_editState.selectOfs, m_editState.caretOfs );

			m_editState.selectOfs = beg;
			m_editState.caretOfs = end;
		}

		_updateDisplayArea();
		return true;
	}

	//____ selectAll() _________________________________________________________

	bool EditableText::selectAll()
	{
		return select( 0, int(m_text.size()) );
	}

	//____ unselect() __________________________________________________________

	bool EditableText::unselect()
	{
		if( m_editMode == TextEditMode::Static )
			return false;

		if( m_editState.selectOfs != m_editState.caretOfs )
		{
			_layout()->selectionChange( this, m_editState.caretOfs, m_editState.caretOfs, m_editState.selectOfs, m_editState.caretOfs );

			m_editState.selectOfs = m_editState.caretOfs;
		}

		return true;
	}

	//____ selectionBegin() ____________________________________________________

	int EditableText::selectionBegin() const
	{
		return m_editState.selectOfs;
	}

	//____ selectionEnd() ______________________________________________________

	int EditableText::selectionEnd() const
	{
		return m_editState.caretOfs;
	}

	//____ selectionSize() ______________________________________________________

	int EditableText::selectionSize() const
	{
		return abs(m_editState.caretOfs-m_editState.selectOfs);
	}

	//____ getSelection() _______________________________________________________

	std::string EditableText::getSelection() const
	{
		int beg = m_editState.selectOfs;
		int end = m_editState.caretOfs;
		
		if( beg == end )
			return std::string();

		if( beg > end )
			std::swap( beg, end );

		return m_text.substr(beg, end-beg);
	}


	//____ eraseSelected() _____________________________________________________

	int EditableText::eraseSelected()
	{
		if( m_editMode == TextEditMode::Static )
			return -1;

		int beg = m_editState.selectOfs;
		int end = m_editState.caretOfs;

		if( beg == end )
			return 0;

		if( beg > end )
			std::swap( beg, end );

		return erase( beg, end - beg );
	}


	//____ setCaretPos() _______________________________________________________

	bool EditableText::setCaretOfs( int ofs )
	{
		if( m_editMode != TextEditMode::Editable )
			return false;

		limit( ofs, 0, int(m_text.size()) );
		ofs = Utf8::alignToCodepoint( m_text, ofs );

		return _moveCaret( ofs, MoveMethod::ApiCall );
	}

	//____ caretPos() ___________________________________________________________

	int EditableText::caretOfs() const
	{
		if( !m_editState.bCaret )
			return -1;

		return m_editState.caretOfs;
	}
	//____ caretPut() __________________________________________________________

	int EditableText::caretPut( std::string_view _text )
	{
		if( !m_editState.bCaret )
			return 0;

		std::string text = Utf8::isValid(_text) ? std::string(_text) : Utf8::sanitize(_text);

		// Remove any newline characters if we only have one line (special case)

		if( m_maxLines == 1 )
		{
			text.erase( std::remove_if(text.begin(), text.end(), [](char c) { return c == '\n' || c == '\r'; }), text.end() );
		}
		else if( m_maxLines > 1 )	// Limit input to lines that will fit
		{
			//TODO: Implement
		}

		// Limit amount of characters

		if( m_maxChars > 0 )
		{
			int selBeg = std::min(m_editState.selectOfs, m_editState.caretOfs);
			int selEnd = std::max(m_editState.selectOfs, m_editState.caretOfs);

			int nbChars = Utf8::countCodepoints(m_text) - Utf8::countCodepoints( std::string_view(m_text).substr(selBeg, selEnd-selBeg) );
			int max = std::max( 0, m_maxChars - nbChars );

			text.resize( Utf8::offsetOfCodepoint(text, max) );
		}

		//

		int retVal;

		if( m_editState.caretOfs != m_editState.selectOfs )		// Selection should be replaced with put content.
		{
			int beg = m_editState.selectOfs;
			int end = m_editState.caretOfs;

			if( beg > end )
				std::swap( beg, end );
			
			retVal = replace( beg, end-beg, text );
			
			int newPos = beg + int(text.size());
			m_editState.caretOfs = newPos;
			m_editState.selectOfs = newPos;
		}
		else													// Just insert the put content
		{
			retVal = insert( m_editState.caretOfs, text );
		}
//		_updateDisplayArea();		// Already done in both replace() and insert().

		return retVal;
	}

	bool EditableText::caretPut( char32_t c )
	{
		return caretPut( Utf8::fromCodepoint(c) ) > 0;
	}

	//____ caretUp() ___________________________________________________________

	bool EditableText::caretUp()
	{
		if( !m_editState.bCaret )
			return false;

		int caretOfs = _layout()->caretUp(this, m_editState.caretOfs, m_editState.wantedOfs );
		return _moveCaret( caretOfs, MoveMethod::Keyboard );
	}

	//____ caretDown() _________________________________________________________

	bool EditableText::caretDown()
	{
		if( !m_editState.bCaret )
			return false;

		int caretOfs = _layout()->caretDown(this, m_editState.caretOfs, m_editState.wantedOfs );
		return _moveCaret( caretOfs, MoveMethod::Keyboard );
	}

	//____ caretLeft() _________________________________________________________

	bool EditableText::caretLeft()
	{
		if( !m_editState.bCaret )
			return false;

		int caretOfs = _layout()->caretLeft(this, m_editState.caretOfs, m_editState.wantedOfs );
		return _moveCaret( caretOfs, MoveMethod::Keyboard );
	}

	//____ caretRight() ________________________________________________________

	bool EditableText::caretRight()
	{
		if( !m_editState.bCaret )
			return false;

		int caretOfs = _layout()->caretRight(this, m_editState.caretOfs, m_editState.wantedOfs );
		return _moveCaret( caretOfs, MoveMethod::Keyboard );
	}

	//____ caretPageUp() _________________________________________________________

	bool EditableText::caretPageUp()
	{
//		RectSPX window = m_pWidget->_windowSection();
//		RectSPX geo = _geo();
		
		
		return false;
	}

	//____ caretPageDown() _______________________________________________________

	bool EditableText::caretPageDown()
	{
		return false;
	}


	//____ caretNextWord() _____________________________________________________

	bool EditableText::caretNextWord()
	{
		if( !m_editState.bCaret )
			return false;

		int caretOfs = _layout()->caretNextWord(this, m_editState.caretOfs);
		return _moveCaret( caretOfs, MoveMethod::Keyboard );
	}

	//____ caretPrevWord() _____________________________________________________

	bool EditableText::caretPrevWord()
	{
		if( !m_editState.bCaret )
			return false;

		int caretOfs = _layout()->caretPrevWord(this, m_editState.caretOfs);
		return _moveCaret( caretOfs, MoveMethod::Keyboard );
	}

	//____ caretEraseNextChar() ________________________________________________

	bool EditableText::caretEraseNextChar()
	{
		if( !m_editState.bCaret )
			return false;

		if( m_editState.caretOfs != m_editState.selectOfs )		// Selection should be replaced with put content.
		{
			eraseSelected();
			return true;
		}
		else if( m_editState.caretOfs < int(m_text.size()) )
		{
			int next = _layout()->caretRight( this, m_editState.caretOfs, m_editState.wantedOfs );
			erase( m_editState.caretOfs, next - m_editState.caretOfs );
			return true;
		}

		return false;
	}

	//____ caretErasePrevChar() ________________________________________________

	bool EditableText::caretErasePrevChar()
	{
		if( !m_editState.bCaret )
			return false;

		if( m_editState.caretOfs != m_editState.selectOfs )		// Selection should be replaced with put content.
		{
			eraseSelected();
			return true;
		}

		int caretOfs = m_editState.caretOfs;
		if( caretOfs == 0 )
		{
			_layout()->caretMove(this, 0, 0);		// Force restart of the caret animation cycle.
			return false;
		}
		else
		{
			int prev = _layout()->caretLeft( this, caretOfs, m_editState.wantedOfs );
			erase(prev, caretOfs - prev);
			return true;
		}
	}

	//____ caretEraseNextWord() ________________________________________________

	bool EditableText::caretEraseNextWord()
	{
		return false; //TODO: Implement!
	}

	//____ caretErasePrevWord() ________________________________________________

	bool EditableText::caretErasePrevWord()
	{
		return false; //TODO: Implement!
	}




	//_____ caretLineBegin() ___________________________________________________

	bool EditableText::caretLineBegin()
	{
		if( !m_editState.bCaret )
			return false;

		int caretOfs = _layout()->caretHome(this, m_editState.caretOfs, m_editState.wantedOfs );
		return _moveCaret( caretOfs, MoveMethod::Keyboard );
	}

	//____ caretLineEnd() ______________________________________________________

	bool EditableText::caretLineEnd()
	{
		if( !m_editState.bCaret )
			return false;

		int caretOfs = _layout()->caretEnd(this, m_editState.caretOfs, m_editState.wantedOfs );
		return _moveCaret( caretOfs, MoveMethod::Keyboard );
	}

	//____ caretTextBegin() ____________________________________________________

	bool EditableText::caretTextBegin()
	{
		if( !m_editState.bCaret )
			return false;

		int caretOfs = 0;
		m_editState.wantedOfs = -1;

		return _moveCaret( caretOfs, MoveMethod::Keyboard );
	}

	//____ caretTextEnd() ______________________________________________________

	bool EditableText::caretTextEnd()
	{
		if( !m_editState.bCaret )
			return false;

		int caretOfs = int(m_text.size());		// Caret placed on terminator char following the string.
		m_editState.wantedOfs = -1;

		return _moveCaret( caretOfs, MoveMethod::Keyboard );
	}

	//____ _caretToEnd() __________________________________________________________

	void EditableText::_caretToEnd()
	{
		m_editState.caretOfs = int(m_text.size());		// Caret placed on terminator char following the string.
		m_editState.selectOfs = m_editState.caretOfs;
		m_editState.wantedOfs = -1;
		_updateDisplayArea();
	}


	//____ caretToPos() __________________________________________________________

	bool EditableText::caretToPos( CoordSPX pos)
	{
		int ofs = _layout()->caretToPos(this, pos, m_editState.wantedOfs );
		return _moveCaret( ofs, MoveMethod::Mouse );
	}

	//____ caretIntoView() ______________________________________________________

	void EditableText::caretIntoView()
	{
		if( m_editState.bCaret )
			_updateDisplayArea();
	}


	//____ _caretSelectWord() __________________________________________________

	bool EditableText::caretSelectWord()
	{
		//TODO: Implement!
		return false;
	}

	//____ _caretSelectLine() __________________________________________________

	bool EditableText::caretSelectLine()
	{
		//TODO: Implement!
		return false;
	}


	//____ _moveCaret() ______________________________________________

	bool EditableText::_moveCaret( int caretOfs, MoveMethod method )
	{
		int selectOfs = m_editState.selectOfs;

		if( method == MoveMethod::ApiCall || (method == MoveMethod::Keyboard && !m_editState.bShiftDown) ||
			(method == MoveMethod::Mouse && !(m_editState.bShiftDown || m_editState.bButtonDown)) )
		{
			selectOfs = caretOfs;
		}

		// Notify textmapper of caret and selection changes

		if( m_editState.selectOfs == selectOfs || m_editState.selectOfs != m_editState.caretOfs )
			_layout()->selectionChange( this, selectOfs, caretOfs, m_editState.selectOfs, m_editState.caretOfs );
		else
			_layout()->caretMove( this, caretOfs, m_editState.caretOfs );

		// Finalize

		bool ret = (m_editState.caretOfs == caretOfs);

		m_editState.caretOfs = caretOfs;
		m_editState.selectOfs = selectOfs;
		_updateDisplayArea();

		return ret;
	}

	//____ _updateDisplayArea() _______________________________________________

	void EditableText::_updateDisplayArea()
	{
		// Make sure caret and surrounding characters are visible (and preferably whole selection, if any)
		//

		int beg = m_editState.caretOfs;
		int end = m_editState.selectOfs;

		if (end < beg)
			std::swap(beg, end);

		RectSPX niceToHave = _layout()->rectForRange(this, beg, end - beg);
		RectSPX mustHave = _layout()->rectForCaretWithMargin(this, 3);

		niceToHave = RectSPX::bounds(niceToHave,mustHave);
		
		_requestVisibility(mustHave, niceToHave);
	}

} // namespace wg
