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
#ifndef	WG2_TEXT_DOT_H
#define WG2_TEXT_DOT_H

#ifndef	WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef WG2_COLOR_DOT_H
#	include <wg2_color.h>
#endif

#ifndef WG2_COLORSET_DOT_H
#	include <wg2_colorset.h>
#endif

#include <wg_char.h>
#include <wg_charbuffer.h>


#ifndef	WG2_CURSORINSTANCE_DOT_H
	#include <wg2_cursorinstance.h>
#endif

#ifndef WG_EVENT_DOT_H
#	include <wg2_event.h>
#endif

class	WgValueFormat;

//____ WgTextHolder ___________________________________________________________

struct WgTextHolder
{
public:
	virtual void		_textModified( WgText * pText ) = 0;
};


//_____ WgTextLine _____________________________________________________________

struct WgTextLine
{
	int		nChars;			// Number of characters on this line (not including break).
	int		ofs;			// Offset in buffer for line.

	int		width;			// Width in pixels of line
	short	height;			// Height in pixels of line
	short	lineSpacing;	// Spacing from this line to next in pixels.
	short	baseline;		// Offset in pixels to the baseline.

	// Following characters can lead to a soft break:
	//
	// Whitespace, tab and BREAK_PERMITTED_HERE: Gives no hyphen.
	// HYPHEN_BREAK_PERMITTED_HERE: Gives additional hyphen when printed.
};

//____ WgText __________________________________________________________________

class WgText
{
friend class WgTextNode;

public:
	WgText();
	WgText( const wg::CharSeq& seq );
	WgText( const wg::CharBuffer * pBuffer );
	WgText( const wg::String& str );

	void	Init();

	~WgText();

	//TODO: operator= should copy the whole object, not just the text.

	inline void operator=( const WgText& t) { setText(&t); }; // Fastest order to do this in.

	void		setText( const wg::CharSeq& seq );
	void		setText( const wg::CharBuffer * buffer );
	void		setText( const wg::String& str );
	void		setText( const WgText * pText );

	int			addText( const wg::CharSeq& seq );
	int			insertText( int ofs, const wg::CharSeq& seq );
	int			replaceText( int ofs, int nDelete, const wg::CharSeq& seq );
	int			deleteText( int ofs, int len );
	void		deleteSelectedText();

	int			addChar( const wg::Char& character );
	int			insertChar( int ofs, const wg::Char& character );
	int			replaceChar( int ofs, const wg::Char& character );
	int			deleteChar( int ofs );


	int			nbChars() const;
	int			nbLines() const;

	inline bool	IsEmpty() const { return nbChars()==0?true:false; }

	inline const wg::Char * getText() const { return m_buffer.chars(); }
	inline wg::CharBuffer * getBuffer() { return &m_buffer; }
	inline const wg::CharBuffer * getBuffer() const { return &m_buffer; }

	const WgTextLine *	getLines() const;
	WgTextLine *		getLine( int line ) const;
	const wg::Char * 	getLineText( int line ) const;
	int					getLineWidth( int line ) const;
	int					getLineOfsY( int line ) const;

	const WgTextLine *	getSoftLines() const;
	WgTextLine *		getSoftLine( int line ) const;
	const wg::Char * 	getSoftLineText( int line ) const;
	int					getSoftLineWidth( int line ) const;
	int					getSoftLineWidthPart( int line, int startCol, int nCol = INT_MAX ) const;
	int					nbSoftLines() const;
	int					getSoftLineSelectionWidth( int line ) const;

	void				posSoft2Hard( int &line, int &col ) const;
	void				posHard2Soft( int &line, int &col ) const;


	void				clear();
	void				refresh();

//  --------------

	void				selectText( int startLine, int startCol, int endLine, int endCol );
	bool				getSelection( int& startLine, int& startCol, int& endLine, int& endCol ) const;
	WgRange				getSelection() const;
	void				clearSelection( );


//  --------------

	void				setHolder( WgTextHolder * pHolder ) { m_pHolder = pHolder; }

//  --------------

	inline wg::TextStyle_p      getStyle() const { return m_pBaseStyle; }
	inline WgColor				getColor() const { return m_pBaseStyle->color(wg::State::Default); }
	inline WgColor				getColor(WgState state) const { return m_pBaseStyle->color(state); }
	inline wg::Font_p			getFont() const { return m_pBaseStyle->font(); }
	inline wg::TextLink_p		getLink() const { return m_pBaseStyle->link(); }

//	--------------

	void				setStyle( wg::TextStyle * pStyle );

	bool				setColor( const WgColor color );
	bool				setColor( const WgColor color, wg::State state );

	void				setFont( wg::Font * pFont );
	void				setLink( wg::TextLink * pLink );

// -------------

	void				clearStyle();

	void				clearColor();
	void				clearColor( WgState state );

	void				clearFont();
	void				clearLink();

// -------------

//	void				setLinkProperties( const wg::TextStyle_p& pProp );
//	void				clearLinkProperties();
//	wg::TextStyle_p		getLinkProperties() const { return m_pLinkProp; }

//	void				setSelectionProperties( const wg::TextStyle_p& pProp );
//	void				clearSelectionProperties();
//	wg::TextStyle_p		getSelectionProperties() const { return m_pSelectionProp; }

// -------------

//	void				setSelectionBgColor(WgColor c);
//	inline WgColor		getSelectionBgColor() const { return m_pSelectionProp->Color(); }

// -------------

	void				setCursorStyle( WgCursor * pCursor );
	inline WgCursor *	getCursorStyle() const { return m_pCursorStyle; }

// -------------

	void				GetBaseAttr( wg::TextAttr& attr ) const;
	bool				GetCharAttr( wg::TextAttr& attr, int charOfs ) const;
	bool				IsCharUnderlined(	int charOfs ) const;
	WgColor				GetCharColor(		int charOfs ) const;
	WgColor				GetCharBgColor(		int charOfs ) const;
	int					GetCharSize(		int charOfs ) const;
	wg::Font_p			GetCharFont(		int charOfs ) const;
//	WgGlyphset *		GetCharGlyphset(	int charOfs ) const;
//	bool				GetCharVisibility(	int charOfs ) const;
	wg::TextLink_p		GetCharLink(		int charOfs ) const;
	bool				IsCharLink(			int charOfs ) const;

// -------------

	void				setValue( double value, const WgValueFormat& form );
	void				setScaledValue( Sint64 value, Uint32 scale, const WgValueFormat& form );
//	int				compareTo( const WgText * pOther, bool bCheckCase = true ) const;	// Textual compare in the style of strcmp().

	int					width() const;
	int					height() const;

	WgSize				unwrappedSize() const;
	int					unwrappedWidth() const;				// Width of text if no lines are wrapped.

	int					heightForWidth( int width ) const;

	int					softLineHeight( int line );
	int					softLineSpacing( int line );

	void				setLineWidth( int width );
	inline int			getLineWidth() const { return m_lineWidth; }

	void				SetWrap( bool bWrap );
	bool				IsWrap() const { return m_bWrap; }

	void				SetAutoEllipsis( bool bAutoEllipsis );
	bool				IsAutoEllipsis() const { return m_bAutoEllipsis; }

	bool				SetMaxChars( int max );
	int					MaxChars() const { return m_maxChars; }

	void				SetScale( int scale );
	int					Scale() const { return m_scale; }


	void					SetEditMode(WgTextEditMode mode);							// Maybe should be protected with Widgets/Items as friends?
	inline WgTextEditMode	GetEditMode() const { return m_editMode; }
	inline bool				IsEditable() const { return m_editMode == WgTextEditMode::Editable; }
	inline bool				IsSelectable() const { return m_editMode != WgTextEditMode::Static; }



	inline void			setState( wg::State state ) { m_state = state; }
	inline void			setAlignment( const WgOrigo alignment ) { m_alignment = alignment; }
	inline void			setTintMode( WgTintMode mode ) { m_tintMode = mode; }
	inline void			setLineSpaceAdjustment( float adjustment ) { m_lineSpaceAdj = adjustment; }


	inline wg::State	state() const { return m_state; }
	inline const WgOrigo alignment() const { return m_alignment; }
	inline WgTintMode	tintMode() const { return m_tintMode; }
	inline float		lineSpaceAdjustment() const { return m_lineSpaceAdj; }


	// Get-methods


	char *	getTextUTF8() const;
	int		getTextUTF8( char * pDest, int maxBytes ) const;
	int		getTextSizeUTF8() const;

	char *	getSelectedTextUTF8() const;


	// cursor instance forwards. TODO: merge cursor instance with WgText
	WgCursorInstance*GetCursor() const { return m_pCursor; }

	void			CursorGotoCoord( const WgCoord& coord, const WgRect& container );
	void			CursorGotoLine( int line, const WgRect& container );
	void			CursorGoUp( int nbLines, const WgRect& container );
	void			CursorGoDown( int nbLines, const WgRect& container );

	void			hideCursor() { if(m_pCursor) m_pCursor->hide(); }
	void			showCursor() { if(m_pCursor) m_pCursor->show(); }
	bool			isCursorShowing() const { return m_pCursor ? !m_pCursor->isHidden() : false; }

	bool			incTime( int ms ) { return m_pCursor ? m_pCursor->incTime(ms) : false; }
	void			insertMode( bool bInsert ) { if(m_pCursor) m_pCursor->insertMode(bInsert); }

	inline void		goLeft( int nChars = 1 ){ if(m_pCursor) m_pCursor->goLeft(nChars); }
	inline void		goRight( int nChars = 1 ){ if(m_pCursor) m_pCursor->goRight(nChars); }

	inline void		goBOF(){ if(m_pCursor) m_pCursor->goBOF(); }
	inline void		goEOF(){ if(m_pCursor) m_pCursor->goEOF(); }
	void			goBOL(){ if(m_pCursor) m_pCursor->goBOL(); }
	void			goEOL(){ if(m_pCursor) m_pCursor->goEOL(); }

	void			gotoSoftLine( int line, const WgRect& container ){ if(m_pCursor) m_pCursor->gotoSoftLine(line, container); }


	void			gotoSoftPos( int line, int col ){ if(m_pCursor) m_pCursor->gotoSoftPos( line, col );}
	void			gotoColumn( int col ){ if(m_pCursor) m_pCursor->gotoColumn( col );}

	void			gotoPrevWord(){ if(m_pCursor) m_pCursor->gotoPrevWord();}
	void			gotoBeginningOfWord(){ if(m_pCursor) m_pCursor->gotoBeginningOfWord();}
	void			gotoNextWord(){ if(m_pCursor) m_pCursor->gotoNextWord();}
	void			gotoEndOfWord(){ if(m_pCursor) m_pCursor->gotoEndOfWord();}

	void			getSoftPos( int &line, int &col ) const{if(m_pCursor) m_pCursor->getSoftPos( line, col );}

	bool			putChar( Uint16 character ){return m_pCursor ? m_pCursor->putChar( character ):false;}
	int				putText( const wg::CharSeq& seq ){return m_pCursor ? m_pCursor->putText( seq ):0;}
	void			unputText( int nChar ){if(m_pCursor) m_pCursor->unputText( nChar );}
	void 			delPrevWord(){if(m_pCursor) m_pCursor->delPrevWord();}
	void 			delNextWord(){if(m_pCursor) m_pCursor->delNextWord();}
	void            delToBOL(){if(m_pCursor) m_pCursor->delToBOL();}
	void            delToEOL(){if(m_pCursor) m_pCursor->delToEOL();}
	bool			delPrevChar(){return m_pCursor ? m_pCursor->delPrevChar():false;}
	bool			delNextChar(){return m_pCursor ? m_pCursor->delNextChar():false;}

	int				ofsX() const{return m_pCursor ? m_pCursor->ofsX():0;}
	int				ofsY() const{return m_pCursor ? m_pCursor->ofsY():0;}

	inline int		line() const{return m_pCursor ? m_pCursor->line():0;}
	inline int		column() const{return m_pCursor ? m_pCursor->column():0;}
	inline int		time() const{return m_pCursor ? m_pCursor->time():0;}

	WgCursor::Mode	cursorMode() const { return m_pCursor ? m_pCursor->cursorMode() : WgCursor::EOL; }

	void			setSelectionMode(bool bOn){ if(m_pCursor) m_pCursor->setSelectionMode(bOn); }
	bool			hasSelection()const { return m_pCursor ? m_pCursor->hasSelection() : false; }
	void			delSelection(){ if(m_pCursor) m_pCursor->delSelection(); }

	void			selectAll() { if(m_pCursor) m_pCursor->selectAll(); }
	void			selectRange( WgRange range ) { if(m_pCursor) m_pCursor->selectRange(range); }

	int				LineColToOffset(int line, int col) const;						// HARD LINES!!!!!
//	void			OffsetToSoftLineCol(int ofs, int* wpLine, int* wpCol) const;

	//

	int				LineStartX( int line, const WgRect& container ) const;
	int				LineStartY( int line, const WgRect& container ) const;
	WgCoord			LineStart( int line, const WgRect& container ) const;

	int				CoordToLine( const WgCoord& coord, const WgRect& container, bool bCursorMode = false ) const;
	int				CoordToColumn( int line, const WgCoord& coord, const WgRect& container, bool bCursorMode = false ) const;
	WgTextPos		CoordToPos( const WgCoord& coord, const WgRect& container, bool bCursorMode = false ) const;
	int				CoordToOfs( const WgCoord& coord, const WgRect& container, bool bCursorMode = false ) const;
	wg::TextLink_p	CoordToLink( const WgCoord& coord, const WgRect& container ) const;

	// WgTextPos is automatically clamped for all PosTo...-methods.

	int				PosToCoordX( const WgTextPos& pos, const WgRect& container ) const;
	int				PosToCoordY( const WgTextPos& pos, const WgRect& container ) const;
	WgCoord			PosToCoord( const WgTextPos& pos, const WgRect& container ) const;
	int				PosToOfs( const WgTextPos& pos ) const;

	int				OfsToCoordX( int ofs, const WgRect& container ) const;
	int				OfsToCoordY( int ofs, const WgRect& container ) const;
	WgCoord			OfsToCoord( int ofs, const WgRect& container ) const;
	WgTextPos		OfsToPos( int ofs ) const;

	WgTextPos		ClampPos( WgTextPos ) const;

	//

	WgCoord			FocusWindowOnRange( const WgSize& canvas, const WgRect& window, WgRange range ) const;

	bool 			OnEvent( const WgEvent::Event * pEvent, WgEventHandler * pEventHandler, const WgRect& container );
//	bool			OnAction( WgInput::UserAction action, int button_key, const WgRect& textRect, const WgCoord& pointerOfs );

	wg::TextLink_p	GetMarkedLink() const { return m_pMarkedLink; }

	inline void		SetBaseColors( const WgColorsetPtr& pColors ) { m_pBaseColors = pColors; }
	inline WgColorsetPtr BaseColors() const { return m_pBaseColors; }

	inline void		SetBgBlockColors( const WgColorsetPtr& pColors ) { m_pBgBlockColors = pColors; }
	inline WgColorsetPtr BgBlockColors() const { return m_pBgBlockColors; }

protected:



	static const int	s_parseBufLen = 9+16+1+16+8;
	wg::Char *		_parseValue( double value, const WgValueFormat& form, wg::Char[s_parseBufLen] );
	wg::Char *		_parseScaledValue( Sint64 value, Uint32 scale, const WgValueFormat& form, wg::Char[s_parseBufLen] );

	void            _textModified();

	void			_regenHardLines();		// regenerate the softlines-array (if necessary).
	void			_regenSoftLines();		// regenerate the softlines-array (if necessary).
	int 			_countWriteSoftLines( int maxWidth, const wg::Char * pStart, WgTextLine * pWriteLines, int maxWrite ) const; // Central algorithm of regenSoftLines().

	void			_refreshAllLines();
	void			_refreshLineInfo( WgTextLine * pLine ) const;

	int				_cursorMaxWidth() const;

//    wg::TextStyle_p     _cloneBaseStyle() const;

	wg::CharBuffer	m_buffer;
	WgCursor*		m_pCursorStyle;
	WgCursorInstance*	m_pCursor;

	WgTextEditMode	m_editMode;


	WgTintMode		m_tintMode;
	WgOrigo	m_alignment;
	wg::State		m_state;

	wg::TextLink_p	m_pMarkedLink;
	wg::State		m_markedLinkState;

//	int				m_markedLinkOfs;	// Offset in buffer for first character of link currently marked or -1 if none.

	int				m_selStartLine;
	int				m_selEndLine;
	int				m_selStartCol;
	int				m_selEndCol;
	float			m_lineSpaceAdj;		// Adjustment of linespacing for this text.

	WgColorsetPtr	m_pBgBlockColors;	// Textcolors as defined by background blockset (lowest prio).

	WgColorsetPtr	m_pBaseColors;		// Default colors for this text (prio between m_pBgBlockColors and m_pBaseProp).
	wg::TextStyle_p	m_pBaseStyle;		// Default style for this text. Used for all characters who have
										// properties set to 0.

	WgTextLine*		m_pHardLines;
	WgTextLine*		m_pSoftLines;

	int				m_nSoftLines;
	int				m_nHardLines;

	int				m_lineWidth;

	int				m_newCursorPos;

	int				m_maxChars;			// Maximum number of characters allowed.

	bool			m_bWrap;
	bool			m_bAutoEllipsis;	// Use ellipsis for text that doesn't fit.

	int				m_scale;

	WgTextHolder *	m_pHolder;

	static WgTextLine	g_emptyLine;
};


#endif // WG2_TEXT_DOT_H
