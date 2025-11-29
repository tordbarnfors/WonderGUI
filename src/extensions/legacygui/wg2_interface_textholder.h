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
#ifndef WG2_INTERFACE_TEXTHOLDER_DOT_H
#define WG2_INTERFACE_TEXTHOLDER_DOT_H

#ifndef WG2_TEXT_DOT_H
#	include <wg2_text.h>
#endif

#ifndef WG2_COLOR_DOT_H
#	include <wg2_color.h>
#endif

#include <wg_string.h>

//____ Wg_Interface_TextHolder ________________________________________________

class Wg_Interface_TextHolder : protected WgTextHolder
{

public:
    virtual ~Wg_Interface_TextHolder() = default;
	virtual WgTextEditMode GetEditMode() const  { return WgTextEditMode::Static; }

	inline wg::TextStyle_p GetTextStyle() { return m_pText->getStyle(); }

	inline WgColor			GetTextColor() const { return m_pText->getColor(); }
	inline WgColor			GetTextColor(wg::State state) const { return m_pText->getColor(state); }
	inline wg::Font_p		GetFont() const { return m_pText->getFont(); }
	inline wg::TextLink_p   GetLink() const { return m_pText->getLink(); }

// -------------

	inline void			SetTextStyle( wg::TextStyle* pStyle )		{ m_pText->setStyle(pStyle); _textModified(); }

	inline void			SetTextColor( const WgColor color )					{ if( m_pText->setColor(color) ) _textModified(); }
	inline void			SetTextColor( const WgColor color, wg::State state )		{ if( m_pText->setColor(color,state) )  _textModified(); }

	inline void			SetFont( wg::Font * pFont )							{ m_pText->setFont(pFont); _textModified(); }
	inline void			SetLink( wg::TextLink * pLink )						{ m_pText->setLink(pLink); _textModified(); }

// -------------

	inline void			ClearTextStyle()								{ m_pText->clearStyle(); _textModified(); }

	inline void			ClearTextColor()										{ m_pText->clearColor(); _textModified(); }
	inline void			ClearTextColor( wg::State state )						{ m_pText->clearColor(state); _textModified(); }

	inline void			ClearFont()											{ m_pText->clearFont(); _textModified(); }
	inline void			ClearLink()											{ m_pText->clearLink(); _textModified(); }

//	-----------

	inline void			SetCursor( WgCursor * pCursor ) { m_pText->setCursorStyle(pCursor); _textModified(); }
	inline WgCursor *	GetCursor() const { return m_pText->getCursorStyle(); }

//	-----------

	inline void			SetTextBaseColors( const WgColorsetPtr& pColors ) { m_pText->SetBaseColors(pColors); }
	inline WgColorsetPtr TextBaseColors() const { return m_pText->BaseColors(); }

//	-----------

	inline bool			SetMaxChars( int max ) { bool ret = m_pText->SetMaxChars(max); _textModified(); return ret; }
	inline int			MaxChars() const { return m_pText->MaxChars(); }


//	inline void			SetSelectionColor( WgColor color )							{ m_pText->setSelectionBgColor(color); }
//	inline WgColor		GetSelectionColor() const									{ return m_pText->getSelectionBgColor(); }

	void				SetTextState( wg::State state );
	void				SetTextAlignment( const WgOrigo alignment );
	void				SetTextTintMode( WgTintMode mode );

	void				ClearText();

	void		SetText( const wg::CharBuffer * buffer );
	void		SetText( const wg::String& str );
	void		SetText( const WgText * pText );

	void        SetText( const std::string& str ) { SetText(wg::String(str)); }
	void        SetText( const char* str ) { SetText(wg::String(str)); }
	
	int			AddText( const wg::CharSeq& seq );
	int			InsertText( int ofs, const wg::CharSeq& seq );
	int			ReplaceText( int ofs, int nDelete, const wg::CharSeq& seq );
	int			DeleteText( int ofs, int len );
	void		DeleteSelectedText();

	int			AddChar( const wg::Char& character );
	int			InsertChar( int ofs, const wg::Char& character );
	int			ReplaceChar( int ofs, const wg::Char& character );
	int			DeleteChar( int ofs );

	void				SetLineSpaceAdjustment( float adjustment );
	float				GetLineSpaceAdjustment() const;

	virtual bool		SetTextWrap(bool bWrap);
	inline bool			GetTextWrap() const { return m_pText->IsWrap(); }

	bool				SetAutoEllipsis(bool bAutoEllipsis);
	inline bool			AutoEllipsis() const { return m_pText->IsAutoEllipsis(); }
	virtual bool		IsAutoEllipsisDefault() const = 0;

	inline wg::State	TextState() const { return m_pText->state(); }
	inline WgTintMode	TextTintMode() const { return m_pText->tintMode(); }
	inline WgOrigo TextAlignment() const;
	inline WgText *		TextObj() const;
	inline void			SetTextObj(WgText* text) const;
	inline int			TextLines() const;

	inline Uint32		TextWidth() const { return m_pText->width(); }
	inline Uint32		TextHeight() const { return m_pText->height(); }

    std::string         GetStdString() const;

protected:
	Wg_Interface_TextHolder();

	inline void			_textModified( WgText * pText ) { _textModified(); }
	virtual void		_textModified() = 0;
	void				_cloneInterface( Wg_Interface_TextHolder * _pClone );
	void				_onCloneContent( const Wg_Interface_TextHolder * pOrg );

	WgText *			m_pText;			// Must always be set by derived class!

};

//____ inline methods __________________________________________________________


WgOrigo Wg_Interface_TextHolder::TextAlignment() const
{
	return m_pText->alignment();
}

WgText* Wg_Interface_TextHolder::TextObj() const
{
	return m_pText;
}

int Wg_Interface_TextHolder::TextLines() const
{
	return m_pText->nbLines();
}



#endif // WG2_INTERFACE_TEXTHOLDER_DOT_H
