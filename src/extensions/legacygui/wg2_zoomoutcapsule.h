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
#ifndef WG2_GIZMO_MODZOOM_DOT_H
#define WG2_GIZMO_MODZOOM_DOT_H


#ifndef WG2_WIDGET_DOT_H
#	include <wg2_capsule.h>
#endif

#include <wg_surfacefactory.h>
#include <wg2_types.h>
#include <wg2_color.h>

//____ WgZoomOutCapsule __________________________________________________________

class WgZoomOutCapsule : public WgCapsule
{
public:
	WgZoomOutCapsule();
	virtual ~WgZoomOutCapsule();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgZoomOutCapsule(); };

	void            SetOutlineSkin(wg::Skin * pSkin);

	void            SetButtonSkin(wg::Skin * pSkin);
	wg::Skin_p       ButtonSkin() const { return m_pButtonSkin; };

	int				MatchingPixelHeight(int pixelWidth) const;
	int				MatchingPixelWidth(int pixelHeight) const;

	WgSize          MinPixelSize() const;
	void            SetMinPointSize( WgSize min );


	WgSize          PreferredPixelSize() const;

	WgSize          UnzoomedPreferredPixelSize() const;

    void            SetPickableContent(bool bPickable);
    
	void            SetInOutTransition(float factor );                                        // 0.0 -> 1.0. From non-existent (size 0,0) to full size and fully faded in.
	float           InOutTransition() const { return m_transitionFactor; }

	void            SetOutlineMode(bool bOutline);
	bool            IsOutlineMode() const { return m_bOutlineMode; }

	void            SetTint( WgColor tint );
	WgColor         Tint() const { return m_tint; }


	WgRect          ContentAreaPixels() const;
	WgRect          ContentAreaPoints() const;

	WgWidget *      FindWidget( const WgCoord& ofs, WgSearchMode mode );

	void            SetStaticScreenshot( wg::Surface *  pSurface );

protected:

	WgCoord _childPos();
	WgSize  _childSize();
	WgRect  _childGeo();

	void    _renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches );
	void    _onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip );
	void    _onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode );

	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	bool	_onAlphaTest( const WgCoord& ofs );
	void    _onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );

	void    _setScale( int scale );

	WgRect  _screenshotArea( WgRect canvas ) const;
	WgRect  _buttonArea( WgRect canvas ) const;
	void    _regenScreenshot();

	void    _updateButtonState( WgSize gizmoCanvas, WgCoord pointerPos );

private:
	bool        m_bTakingScreenshot = false;
	bool        m_bStaticScreenshot = false;
	wg::Surface_p m_pScreenshot;
	wg::Skin_p   m_pButtonSkin;
	wg::Skin_p   m_pOutlineSkin;

	float       m_transitionFactor = 1.f;
	float       m_outerZoom = 1.f;
	WgColor     m_tint = WgColor::White;

	WgState     m_buttonState = WgStateEnum::Default;
	bool        m_bButtonPressed = false;

	bool        m_bOutlineMode = false;
    bool        m_bPickableContent = false;
	WgSize      m_minPointSize;
};


#endif //WG2_GIZMO_MODZOOM_DOT_H
