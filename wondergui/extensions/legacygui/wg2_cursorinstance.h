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
#ifndef	WG2_CURSORINSTANCE_DOT_H
#define WG2_CURSORINSTANCE_DOT_H

#include <climits>

#ifndef	WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef WG2_CURSOR_DOT_H
#	include <wg2_cursor.h>
#endif

#ifndef WG2_COLOR_DOT_H
#	include <wg2_color.h>
#endif

#include <wg_charseq.h>


class WgText;
class WgPen;

//____ WgCursorInstance ________________________________________________________

class WgCursorInstance
{
	friend class WgPen;
public:

	WgCursorInstance( WgText& text );

	bool			incTime( int ms );
	void			insertMode( bool bInsert );

	void			hide();
	void			show();
	bool			isHidden() const;

	inline void		goLeft( int nChars = 1 );
	inline void		goRight( int nChars = 1 );

	inline void		goBOF();
	inline void		goEOF();
	void			goBOL();
	void			goEOL();

	void			gotoSoftLine( int line, const WgRect& container );

	void			gotoHardPos( int line, int col );
	void			gotoSoftPos( int line, int col );

	void			gotoColumn( int col );

	void			gotoPrevWord();
	void			gotoBeginningOfWord();
	void			gotoNextWord();
	void			gotoEndOfWord();

	void			getSoftPos( int &line, int &col ) const;

	bool			putChar( Uint16 character );
	int				putText( const wg::CharSeq& seq );
	void			unputText( int nChar );
	void 			delPrevWord();
	void 			delNextWord();
	void            delToBOL();
	void            delToEOL();
	bool			delPrevChar();
	bool			delNextChar();

	int				ofsX() const;				// Offset in pixels from beginning of line.
	int				ofsY() const;				// Offset in pixels from top of text.

	inline int		line() const;
	inline int		column() const;
	inline int		time() const;

	inline WgText *	text() const;

	WgCursor::Mode	cursorMode() const;

	void			setSelectionMode(bool bOn);
	bool			getSelectionMode() const { return m_bSelectMode; }
	bool			hasSelection();
	void			delSelection();
	void			clearSelection();
	void			selectAll();
	void			selectRange( WgRange range );


protected:
	void			_gotoPos( int line, int col );
	void			_updateLocation(int line, int col);

	WgText *		m_pText;
	bool			m_bHidden;
	bool			m_bInsert;
	bool			m_bSelectMode;
	int				m_selStartLine;
	int				m_selStartColumn;

	int				m_time;					// For the animation
	int				m_line;					// Line of the text containing the cursor (0+)
	int				m_column;				// Character in the line the cursor preceeds (0+)

	int				m_wantedOfsX;			// Desired ofsX for cursor when moving between lines. -1 when not applicable.

};

//____ line() __________________________________________________________________

inline int WgCursorInstance::line() const
{
	return m_line;
}

//____ column() ________________________________________________________________

inline int WgCursorInstance::column() const
{
	return m_column;
}

//____ time() __________________________________________________________________

inline int WgCursorInstance::time() const
{
	return m_time;
}

//____ text() __________________________________________________________________

inline WgText * WgCursorInstance::text() const
{
	return m_pText;
}


inline void WgCursorInstance::goLeft( int nChars )
{
	//if( m_column <= nChars )
	//	gotoColumn( 0 );
	//else
		gotoColumn( m_column - nChars );
}

inline void WgCursorInstance::goRight( int nChars )
{
	gotoColumn( m_column + nChars );
}

inline void WgCursorInstance::goBOF()
{
	gotoSoftPos( 0, 0 );
}

inline void WgCursorInstance::goEOF()
{
	gotoSoftPos( INT_MAX, INT_MAX );
}




#endif //WG2_CURSORINSTANCE_DOT_H
