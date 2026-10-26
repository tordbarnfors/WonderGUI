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
#ifndef WG2_BUTTON_DOT_H
#define WG2_BUTTON_DOT_H

#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_BLOCKSET_DOT_H
#	include <wg2_blockset.h>
#endif

#ifndef	WG2_TEXT_DOT_H
#	include <wg2_text.h>
#endif

#ifndef	WG2_INTERFACE_TEXTHOLDER_DOT_H
#	include <wg2_interface_textholder.h>
#endif

#ifndef	WG2_ICONHOLDER_DOT_H
#	include <wg2_iconholder.h>
#endif

//____ WgButton ____________________________________________________________

class WgButton : public WgWidget, public Wg_Interface_TextHolder, public WgIconHolder
{
public:
	WgButton();
	virtual ~WgButton();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgButton(); };

	//____ Methods __________________________________________

	void			SetDownWhenMouseOutside( bool bDown );
	bool			DownWhenMouseOutside() const { return m_bDownOutside; }

	bool			SetIcon( wg::Skin * pIconSkin, WgOrigo origo, WgBorders borders = WgBorders(0), float _scale = 0.f, bool _bPushText = true );
	void			SetIcon( wg::Skin * pIconSkin );
	wg::Skin_p		GetIconSource() const { return m_pIconSkin; }

	// DEPRECATED!!!
	void            SetSource(wg::Skin * pSkin) { SetSkin(pSkin); }

	Uint32			GetTextAreaWidth();

	virtual int		MatchingPixelHeight( int width ) const;
//	virtual int		MatchingPixelWidth( int height ) const;

	virtual WgSize	PreferredPixelSize() const;

	bool			IsAutoEllipsisDefault() const { return false; };


protected:

	virtual void	_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	virtual void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	void			_onRefresh();
	virtual void	_onCloneContent( const WgWidget * _pOrg );
	bool			_onAlphaTest( const WgCoord& ofs );
	void			_onEnable();
	void			_onDisable();
	virtual void	_onNewSize( const WgSize& size );
	void 			_setScale( int scale );

	void			_onGotInputFocus();
	void			_onLostInputFocus();


	void			_textModified();
	void			_iconModified();

	virtual WgMode	_getRenderMode();


	WgText			m_text;

	wg::Skin_p		m_pIconSkin;

	WgMode			m_mode;

	bool			m_bDownOutside;			// Button remains down when pressed and mouse gets outside?

	bool			m_bPressed;				// Set when left mousebutton was pressed inside.
	bool			m_bReturnPressed;
	bool			m_bPointerInside;

};

#endif //WG2_BUTTON_DOT_H
