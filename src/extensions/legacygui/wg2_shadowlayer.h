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
#ifndef WG2_SHADOWLAYER_DOT_H
#define WG2_SHADOWLAYER_DOT_H
#pragma once

#include <vector>
#include <wg2_layer.h>
#include <wg_surface.h>

class WgShadowLayer;

//____ WgShadow _____________________________________________________________________

class WgShadow
{
	friend class WgShadowLayer;
public:
	/** @private */
	WgShadow(WgWidget* pWidget, wg::Skin* pSkin, WgRect geo ) : m_pWidget(pWidget), m_pSkin(pSkin), m_geo(geo) {}

	WgWidget *    widget() const { return m_pWidget.GetRealPtr(); }
	wg::Skin *      shadow() const { return m_pSkin.rawPtr(); }

private:
	WgWidgetWeakPtr m_pWidget;
	wg::Skin_p       m_pSkin;
	WgRect          m_geo;
};


//____ WgShadowHook ___________________________________________________________

class WgShadowHook : public WgLayerHook     /** @private */
{
	friend class WgShadowLayer;

public:
	const char *Type(void) const;
	static const char * ClassType();

	WgShadowLayer *    Parent() const;

protected:

	WgShadowHook(WgShadowLayer * pParent) : m_pParent(pParent) {}

	void        _requestResize();

	WgHook *    _prevHook() const;
	WgHook *    _nextHook() const;
	WgContainer* _parent() const;

	WgShadowLayer * m_pParent;
};

//____ WgShadowLayer ____________________________________________________________


class WgShadowLayer : public WgLayer
{
	friend class WgShadowHook;
public:
	WgShadowLayer();
	virtual ~WgShadowLayer();

	virtual const char *Type(void) const override;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const override { return new WgShadowLayer(); };

	WgHook *		SetFront(WgWidget * pWidget);
	WgWidget *		Front();
	bool			DeleteFront();
	WgWidget *		ReleaseFront();

	bool			DeleteChild(WgWidget * pWidget) override;
	WgWidget *		ReleaseChild(WgWidget * pWidget) override;

	bool			DeleteAllChildren() override;
	bool			ReleaseAllChildren() override;

	int             NbShadows() const { return (int) m_shadows.size(); }

	void            ClearShadows();
	bool            AddShadow(WgWidget * pWidget, wg::Skin * pShadow);

	void            RemoveShadow(int index);
	void            RemoveShadow(WgWidget * pWidget);

	void            SetShadowTint( uint8_t alpha);
	uint8_t         ShadowTint() { return m_shadowTint; }

	// Overloaded from Widget

	int             MatchingPixelHeight( int width ) const override;
	int             MatchingPixelWidth( int height ) const override;

	WgSize          PreferredPixelSize() const override;

protected:

	// Overloaded from Layer

	WgLayerHook *    _firstLayerHook() const override;

	WgHook*          _firstHook() const override;        // Fist Hook returned is the normal child, then follows the dragged one.
	WgHook*          _lastHook() const override;        //

	WgHook *        _firstHookWithGeo(WgRect& geo) const override;
	WgHook *        _nextHookWithGeo(WgRect& geo, WgHook * pHook) const override;

	WgHook *        _lastHookWithGeo(WgRect& geo) const override;
	WgHook *        _prevHookWithGeo(WgRect& geo, WgHook * pHook) const override;


	// Overloaded from Widget

	void            _onCloneContent(const WgWidget * _pOrg) override;
	void            _onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler) override;
	void            _onNewSize(const WgSize& size) override;

	void            _preRender() override;
	void            _renderPatches(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches) override;

	//

	void			_willRemoveShadows(int ofs, int nb);
	void            _requestRenderShadows(int ofs, int nb);
	void			_onFrontChanged();

	bool			m_bPreRenderRequested = false;

	WgShadowHook    m_frontHook;
	wg::Surface_p   m_pShadowSurface;
	uint8_t         m_shadowTint = 255;

	std::vector<WgShadow>	m_shadows;


};




#endif //WG2_SHADOWLAYER_DOT_H

