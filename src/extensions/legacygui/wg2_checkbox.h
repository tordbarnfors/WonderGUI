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
#ifndef	WG2_CHECKBOX_DOT_H
#define	WG2_CHECKBOX_DOT_H

#ifndef	WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_INTERFACE_TEXTHOLDER_DOT_H
#	include <wg2_interface_textholder.h>
#endif

#ifndef WG2_TEXT_DOT_H
#	include <wg2_text.h>
#endif

#ifndef WG2_BLOCKSET_DOT_H
#	include <wg2_blockset.h>
#endif

#ifndef WG2_ICONHOLDER_DOT_H
#	include <wg2_iconholder.h>
#endif

class	WgCheckBox : public WgWidget, public Wg_Interface_TextHolder, public WgIconHolder
{
public:
	WgCheckBox();
	virtual ~WgCheckBox();

	virtual const char * Type() const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgCheckBox(); };

	enum ClickArea
	{
		DEFAULT,		// Full geometry of icon (no alpha test) + text + area between + alpha test on background.
		ALPHA,			// Alpha test on background and icon.
		GEO,			// Full geometry of Widget is clickable.
		ICON,			// Only the icon (alpha test) is clickable.
		TEXT			// Only the text is clickable.
	};

	void			SetClickArea( ClickArea clickArea );
	ClickArea		GetClickArea() const { return m_clickArea; }

	void			SetIcon( wg::Skin * pIconSkin, const WgOrigo& origo, WgBorders borders = WgBorders(0), float scale = 0.f, bool bPushText = true );
	void			SetIcon( wg::Skin * pIconSkin );

	inline bool		IsChecked() { return m_bChecked; };

	inline bool		Check( void ) { return SetState( true ); };
	inline bool		Uncheck( void ) { return SetState( false ); };
	virtual bool	SetState( bool state );
	inline bool		ChangeState( void ) { return SetState( !m_bChecked ); };

	void			SetFlipOnRelease( bool bFlipOnRelease );
	inline bool		FlipOnRelease() { return m_bFlipOnRelease; }

	Uint32			GetTextAreaWidth();

	WgSize			PreferredPixelSize() const;
	bool			IsAutoEllipsisDefault() const { return false; };


protected:

	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	void	_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	void	_onRefresh();
	void	_onNewSize( const WgSize& size );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onEnable();
	void	_onDisable();
	void 	_setScale( int scale );

	bool	_setState( bool state );

	bool			m_bChecked;

	virtual void	_iconModified();
	virtual void	_textModified();

private:


	void	_refreshTextArea();
	bool	_markTestTextArea( int _x, int _y );



	bool			m_bOver;						// Set when mouse is over.
	bool			m_bPressed;						// Set when mouse is pressed and over.
	bool			m_bFlipOnRelease;				// Set if we want to flip checkbox on press (default), not click.

	wg::Skin_p        m_pIconSkin;
	WgText			m_text;

	ClickArea		m_clickArea;
};




#endif //	WG2_CHECKBOX_DOT_H
