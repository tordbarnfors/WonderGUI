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
#include <wg2_cursorinstance.h>

#ifndef WG2_TEXT_DOT_H
#	include <wg2_text.h>
#endif

#	include <wg_texttool.h>
#	include <wg_font.h>


//____ Constructor() __________________________________________________________

WgCursorInstance::WgCursorInstance( WgText& text )
{
	m_pText 		= &text;
	m_bHidden		= true;
	m_bInsert		= true;
	m_bSelectMode	= false;
	m_wantedOfsX	= -1;

	m_time 			= 0;

	m_line 			= text.nbLines()-1;
	m_column 		= text.getLine(m_line)->nChars;
	m_selStartLine	= m_line;
	m_selStartColumn= m_column;
}

//____ incTime() _______________________________________________________________

bool WgCursorInstance::incTime( int ms )
{
	m_time += ms;
	return true; //TODO: Should only return true if cursor needs to be re-rendered.
}

//____ hide() _________________________________________________________________

void WgCursorInstance::hide()
{
	m_bHidden = true;
}

//____ show() _________________________________________________________________

void WgCursorInstance::show()
{
	m_bHidden = false;
}

//____ isHidden() _____________________________________________________________

bool WgCursorInstance::isHidden() const
{
	return m_bHidden;
}



//____ insertMode() ____________________________________________________________

void WgCursorInstance::insertMode( bool bInsert )
{
	m_bInsert = bInsert;
}


//____ gotoSoftLine() _________________________________________________________

void WgCursorInstance::gotoSoftLine( int line, const WgRect& container )
{
	// First we need to update m_wantedOfsX...

	if( m_wantedOfsX == -1 )
	{
		int	ln = m_line;
		int	col = m_column;
		m_pText->posHard2Soft( ln, col );

		m_wantedOfsX = m_pText->getSoftLineWidthPart( ln, 0, col );
	}

	// Set our line and convert to hard

	int ln = line;
	int col = m_pText->CoordToColumn( line, WgCoord( m_wantedOfsX+container.x, 0), container, true );


	m_pText->posSoft2Hard( ln, col );
	_updateLocation(ln, col);
}


//____ gotoColumn() ____________________________________________________________

void WgCursorInstance::gotoColumn( int col )
{
	const int minLine = 0;
	const int maxLine = m_pText->nbLines()-1;

	int line = m_line;

	if( line > maxLine )
		line = maxLine;

	int maxCol = m_pText->getLine(line)->nChars;

	// Check whether to hop down/up on lines
	if(col < 0)
	{
		line = line - 1;

		// Have we reached the top?
		if(line < minLine)
		{
			// Yes
			line = minLine;
			col = 0;
		}
		else
		{
			// No, set cursor column to end of line
			col = m_pText->getLine(line)->nChars;
		}
	}
	else if( col > (int)maxCol )
	{
		line = line + 1;

		// Have we reached the bottom?
		if(line > maxLine)
		{
			// Yes, set cursor on end of line
			line = maxLine;
			col = m_pText->getLine(line)->nChars;
		}
		else
		{
			// No, set cursor column to 0
			col = 0;
		}
	}

	_updateLocation(line, col);

	m_wantedOfsX = -1;
}

//____ gotoPrevWord() _________________________________________________________

void WgCursorInstance::gotoPrevWord()
{
	gotoBeginningOfWord();

	if(m_line == 0 && m_column == 0)
		return;

	int line = m_line;
	int col = m_column;

	if(line > m_pText->nbLines() - 1)
		line = m_pText->nbLines() - 1;

	if(col > m_pText->getLine(line)->nChars)
		col = m_pText->getLine(line)->nChars;

	const wg::Char* pText = m_pText->getLineText(line) + col - 1;

	while( !isspace(pText->code()) && !ispunct(pText->code()) )
	{
		if(col == 0)
		{
			if(line == 0)
				break;
			line--;
			col = m_pText->getLine(line)->nChars;
		}
		else
		{
			col--;
			if(line == 0 && col == 0)
				break;
		}
		pText--;
	}

	_updateLocation(line, col);
	m_wantedOfsX = -1;
}

//_____________________________________________________________________________
void WgCursorInstance::gotoBeginningOfWord()
{
	if(m_line == 0 && m_column == 0)
		return;

	int line = m_line;
	int col = m_column;

	if(line > m_pText->nbLines() - 1)
		line = m_pText->nbLines() - 1;

	if(col > m_pText->getLine(line)->nChars)
		col = m_pText->getLine(line)->nChars;

	const wg::Char* pText = m_pText->getLineText(line) + col - 1;

	while( isspace(pText->code()) || ispunct(pText->code()) )
	{
		if(col == 0)
		{
			if(line == 0)
				break;
			line--;
			col = m_pText->getLine(line)->nChars;
		}
		else
		{
			col--;
			if(line == 0 && col == 0)
				break;
		}
		pText--;
	}

	_updateLocation(line, col);
	m_wantedOfsX = -1;
}

//____ gotoNextWord() _________________________________________________________

void WgCursorInstance::gotoNextWord()
{
	gotoEndOfWord();

	int line = m_line;
	int col = m_column;

	if(line > m_pText->nbLines() - 1)
		line = m_pText->nbLines() - 1;

	if(col > m_pText->getLine(line)->nChars)
		col = m_pText->getLine(line)->nChars;

	const wg::Char* pText = m_pText->getLineText(line) + col;

	while( !pText->isEndOfText() && (isspace(pText->code()) || ispunct(pText->code())) )
	{
		if(pText->isEndOfLine())
			line++, col = 0;
		else
			col++;
		pText++;
	}

	_updateLocation(line, col);
	m_wantedOfsX = -1;
}

//_____________________________________________________________________________
void WgCursorInstance::gotoEndOfWord()
{
	int line = m_line;
	int col = m_column;

	if(line > m_pText->nbLines() - 1)
		line = m_pText->nbLines() - 1;

	if(col > m_pText->getLine(line)->nChars)
		col = m_pText->getLine(line)->nChars;

	const wg::Char* pText = m_pText->getLineText(line) + col;

	while( !pText->isEndOfText() && !isspace(pText->code()) && !ispunct(pText->code()) )
	{
		if(pText->isEndOfLine())
			line++, col = 0;
		else
			col++;
		pText++;
	}

	_updateLocation(line, col);
	m_wantedOfsX = -1;
}

//____ gotoHardPos() __________________________________________________________

void WgCursorInstance::gotoHardPos( int line, int col )
{
	_gotoPos( line, col );
	m_wantedOfsX = -1;
}


//____ _gotoPos() ______________________________________________________________

void WgCursorInstance::_gotoPos( int line, int col )
{
	int maxLine = m_pText->nbLines()-1;
	if( line > maxLine )
		line = maxLine;

	int maxCol = m_pText->getLine(line)->nChars;
	if( col > maxCol )
		col = maxCol;

	_updateLocation(line, col);
}

//____ gotoSoftPos() __________________________________________________________

void WgCursorInstance::gotoSoftPos( int line, int col )
{
	m_pText->posSoft2Hard( line, col );

	_updateLocation(line, col);

	m_wantedOfsX = -1;
}

//____ getSoftPos() ___________________________________________________________

void WgCursorInstance::getSoftPos( int &line, int &col ) const
{
	line	= m_line;
	col		= m_column;

	m_pText->posHard2Soft( line, col );
}



//____ putChar() ______________________________________________________________

bool WgCursorInstance::putChar( Uint16 character )
{
	m_wantedOfsX = -1;

	int ret;

	int line = m_line;
	int column = m_column;

	int ofs = m_pText->LineColToOffset( line, column );

	if( m_bInsert )
		ret = m_pText->insertChar( ofs, character );

	else
		ret = m_pText->replaceChar( ofs, character );


	if( character == '\n' )
	{
		line++;
		column = 0;
	}
	else if( ret == 1 )
		column++;


	_updateLocation(line, column);

	return ret==1?true:false;
}

//____ putText() ______________________________________________________________

int	WgCursorInstance::putText( const wg::CharSeq& seq )
{
	m_wantedOfsX = -1;

	int nInserted;
	int nLines = m_pText->nbLines();

	int ofs = m_pText->LineColToOffset( m_line, m_column );

	if( m_bInsert )
		nInserted = m_pText->insertText( ofs, seq );
	else
		nInserted = m_pText->replaceText( ofs, seq.length(), seq );

	nLines = m_pText->nbLines() - nLines;

	_updateLocation(m_line + nLines, m_column + nInserted);

	return nInserted;
}

//______________________________________________________________
void WgCursorInstance::unputText( int nChar )
{
	if(nChar < 0)
		return;

	// TODO: FIX!
	for(; nChar; nChar--)
		delPrevChar();
}

//_____________________________________________________________________
void WgCursorInstance::delPrevWord()
{
	int	ofs1 = m_pText->LineColToOffset( m_line, m_column );
	gotoPrevWord();
	int ofs2 = m_pText->LineColToOffset( m_line, m_column );
	m_pText->deleteText( ofs2, ofs1-ofs2 );
}

//_____________________________________________________________________
void WgCursorInstance::delNextWord()
{
	int	ofs1 = m_pText->LineColToOffset( m_line, m_column );
	int line = m_line;
	int column = m_column;
	gotoNextWord();
	int ofs2 = m_pText->LineColToOffset( m_line, m_column );
	m_pText->deleteText(ofs1, ofs2-ofs1);
	_updateLocation(line, column);
}

//_____________________________________________________________________
void WgCursorInstance::delToBOL()
{
	int    ofs1 = m_pText->LineColToOffset( m_line, m_column );
	goBOL();
	int ofs2 = m_pText->LineColToOffset( m_line, m_column );
	m_pText->deleteText( ofs2, ofs1-ofs2 );
}

//_____________________________________________________________________
void WgCursorInstance::delToEOL()
{
	int    ofs1 = m_pText->LineColToOffset( m_line, m_column );
	int line = m_line;
	int column = m_column;
	goEOL();
	int ofs2 = m_pText->LineColToOffset( m_line, m_column );
	m_pText->deleteText(ofs1, ofs2-ofs1);
	_updateLocation(line, column);
}

//____ delPrevChar() __________________________________________________________

bool WgCursorInstance::delPrevChar()
{
	m_wantedOfsX = -1;

	int column= m_column;
	int line = m_line;

	if( column == 0 )
	{
		if( line == 0 )
			return false;

		line -= 1;
		column = m_pText->getLine(line)->nChars;
	}
	else
		column--;


	int ret = m_pText->deleteChar( m_pText->LineColToOffset( m_line, m_column ) -1 );
	_updateLocation(line, column);
	return ret==1?true:false;
}

//____ delNextChar() __________________________________________________________

bool WgCursorInstance::delNextChar()
{
	m_wantedOfsX = -1;

	int ofs = m_pText->LineColToOffset( m_line, m_column );
	int nDel = m_pText->deleteChar( ofs );

	if( nDel )
		return true;
	else
		return false;
}

//____ ofsX() __________________________________________________________________

int WgCursorInstance::ofsX() const
{
	int	ln = m_line;
	int	col = m_column;
	m_pText->posHard2Soft( ln, col );

	return m_pText->getSoftLineWidthPart( ln, 0, col );
}

//____ ofsY() __________________________________________________________________

int WgCursorInstance::ofsY() const
{
	int    ln = m_line;
	int    col = m_column;
	m_pText->posHard2Soft( ln, col );

	int ofs = 0;

	for( int i = 0 ; i < (int) ln ; i++ )
		ofs += m_pText->softLineSpacing(i);

	return ofs;
}

//____ mode() __________________________________________________________________

WgCursor::Mode WgCursorInstance::cursorMode() const
{
	if( m_line >= m_pText->nbLines() || m_pText->getLine(m_line)->nChars == m_column )
		return WgCursor::EOL;

	if( m_bInsert )
		return WgCursor::INS;

	return WgCursor::OVR;
}

//____ goBOL() ________________________________________________________________

void WgCursorInstance::goBOL()
{
	int	ln = m_line;
	int	col = m_column;
	m_pText->posHard2Soft( ln, col );

	col = 0;
	m_pText->posSoft2Hard( ln, col );
	gotoHardPos( ln, col );

	m_wantedOfsX = -1;
}

//____ goEOL() ________________________________________________________________

void WgCursorInstance::goEOL()
{
	int	ln = m_line;
	int	col = m_column;
	m_pText->posHard2Soft( ln, col );

	col = INT_MAX;

	m_pText->posSoft2Hard( ln, col );
	gotoHardPos( ln, col );

	m_wantedOfsX = -1;
}

//////////////////////////////////////////////////////////////////////////
void WgCursorInstance::_updateLocation(int line, int column)
{
	m_line = line;
	m_column = column;

	if(m_bSelectMode)
	{
		m_pText->selectText(m_selStartLine, m_selStartColumn, line, column);
	}
	else
	{
		clearSelection();
	}
}

//////////////////////////////////////////////////////////////////////////
void WgCursorInstance::setSelectionMode(bool bOn)
{
	m_bSelectMode = bOn;
}

bool WgCursorInstance::hasSelection()
{
	if(m_selStartLine < 0)
		return false;

	return m_selStartLine != m_line || (m_selStartLine == m_line && m_selStartColumn != m_column);
}

void WgCursorInstance::delSelection()
{
	if(!hasSelection())
		return;

	m_wantedOfsX = -1;

	int line = m_line;
	int column = m_column;
	if(m_selStartLine < (int)m_line || (m_selStartLine == (int)m_line && m_selStartColumn < (int)m_column))
	{
		std::swap(line, m_selStartLine);
		std::swap(column, m_selStartColumn);
	}

	int ofs1 = m_pText->LineColToOffset(line, column);
	int ofs2 = m_pText->LineColToOffset(m_selStartLine, m_selStartColumn );
	m_pText->deleteText(ofs1, ofs2-ofs1);

	m_selStartLine = line;
	m_selStartColumn = column;

	_updateLocation(line, column);
	clearSelection();
}

void WgCursorInstance::clearSelection()
{
	m_pText->clearSelection();
	m_selStartLine = m_line;
	m_selStartColumn = m_column;
}

void WgCursorInstance::selectRange( WgRange range )
{
	WgTextPos beg = m_pText->OfsToPos( range.ofs );
	WgTextPos end = m_pText->OfsToPos( range.ofs + range.len );

	m_pText->clearSelection();
	setSelectionMode(true);
	m_selStartLine = beg.line;
	m_selStartColumn = beg.col;
	gotoHardPos( end.line, end.col );
	setSelectionMode(false);
}

void WgCursorInstance::selectAll()
{
	m_pText->clearSelection();
	setSelectionMode(true);
	m_selStartLine = 0;
	m_selStartColumn = 0;
	gotoHardPos(m_pText->nbLines(), INT_MAX);
	setSelectionMode(false);
}
