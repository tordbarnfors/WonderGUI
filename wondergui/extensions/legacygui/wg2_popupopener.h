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
#ifndef WG2_POPUPOPENER_DOT_H
#define WG2_POPUPOPENER_DOT_H

#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
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

//____ WgPopupOpener ____________________________________________________________

class WgPopupOpener : public WgWidget, public Wg_Interface_TextHolder, public WgIconHolder
{
public:
	WgPopupOpener();
	virtual ~WgPopupOpener();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgPopupOpener(); };




	//____ Methods __________________________________________

	bool			SetIcon( wg::Skin * pIconSkin, WgOrigo origo, WgBorders borders = WgBorders(0), float _scale = 0.f, bool _bPushText = true );
	void			SetIcon( wg::Skin * pIconSkin );
	wg::Skin_p		GetIconSource() const { return m_pIconSkin; }

	//.____ Behavior ____________________________________________

	void		SetPopup(WgWidget * pPopup);
	WgWidget*   Popup() const { return m_pPopup; }

	void		SetOpenOnHover(bool bOpen);
	bool		OpenOnHover() const { return m_bOpenOnHover; }

	void		SetAttachPoint(WgOrigo attachPoint);
	WgOrigo		AttachPoint() const { return m_attachPoint; }

	void		SetPopupOffset(WgCoord offset);					// Offset in points for attachPoint.
	WgCoord		PopupOffset() const { return m_popupOfs; }

	Uint32			GetTextAreaWidth();

	virtual int		MatchingPixelHeight( int width ) const;
//	virtual int		MatchingPixelWidth( int height ) const;

	virtual WgSize	PreferredPixelSize() const;

	bool			IsAutoEllipsisDefault() const { return false; };
    void            SetScale(int scale);

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


	void			_setState(WgState state);

	void			_open();
	void			_close();

	void			_textModified();
	void			_iconModified();


	WgText			m_text;

	wg::Skin_p		m_pIconSkin;

	bool			m_bReturnPressed;

	WgWidget *		m_pPopup = nullptr;

	WgOrigo			m_attachPoint;
	WgCoord			m_popupOfs;
	bool			m_bOpenOnHover;
	bool			m_bOpen;
	WgState			m_closeState;


};

#endif //WG2_POPUPOPENER_DOT_H
