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
#ifndef WG2_LINEEDITOR_DOT_H
#define WG2_LINEEDITOR_DOT_H


#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef	WG2_INTERFACE_EDITTEXT_DOT_H
#	include <wg2_interface_edittext.h>
#endif

#ifndef	WG2_TEXT_DOT_H
#	include <wg2_text.h>
#endif

#ifndef WG2_CURSORINSTANCE_DOT_H
#	include <wg2_cursorinstance.h>
#endif


//____ WgLineEditor ____________________________________________________________

class WgLineEditor : public WgWidget, public WgInterfaceEditText
{
public:
	WgLineEditor();
	virtual ~WgLineEditor();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgLineEditor(); };


	//____ Methods __________________________________________

	inline void	SetPasswordMode( bool on_off ) { m_bPasswordMode = on_off; };
	inline bool PasswordMode() {return m_bPasswordMode;};
	inline Uint16 PasswordGlyph() const				 { return m_pwGlyph; };
	void		SetPasswordGlyph( Uint16 glyph );

	void		GoBOL();
	void		GoEOL();
	void		GoBOF() { GoBOL(); }
	void		GoEOF() { GoEOL(); }

	Uint32		InsertTextAtCursor( const wg::CharSeq& str );
	bool		InsertCharAtCursor( Uint16 c );

	virtual void			SetEditMode(WgTextEditMode mode);
	virtual WgTextEditMode	GetEditMode() const { return m_text.GetEditMode(); }
	virtual bool			TempIsInputField() const	{ return _isEditable(); }
	virtual Wg_Interface_TextHolder* TempGetText(){ return this; }

	bool		SetTextWrap(bool bWrap);						// Overloading SetTextWrap since we don't support wrapping text.

	WgSize		PreferredPixelSize() const;
	bool		IsAutoEllipsisDefault() const { return false; };


	void        SelectAll() { m_text.selectAll(); }

protected:

	bool	_isEditable() const { return m_text.IsEditable(); }
	bool	_isSelectable() const { return m_text.IsSelectable(); }

	void	_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	void	_onGotInputFocus();
	void	_onLostInputFocus();
	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	void	_onEnable();
	void	_onDisable();
	void	_onNewSize( const WgSize& size );
	void 	_setScale( int scale );

	void	_textModified();

    void    _setState( WgState state );


private:

	void	_adjustViewOfs();

	WgText				m_text;

	bool				m_bResetCursorOnFocus;
	bool				m_bPasswordMode;
	Uint16				m_pwGlyph;
	int					m_viewOfs;
};


#endif //WG2_LINEEDITOR_DOT_H
