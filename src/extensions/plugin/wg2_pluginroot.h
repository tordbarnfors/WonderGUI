/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/
#ifndef WG2_PLUGINROOT_DOT_H
#define WG2_PLUGINROOT_DOT_H
#pragma once

#ifndef WG2_CONTAINER_DOT_H
#	include <wg2_container.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

#include <wg2_root.h>
#include <wg2_pluginbase.h>

#include <wg_skin.h>
#include <wg_gfxdevice.h>
#include <wg_surface.h>

#include <wg_plugininterface.h>


class WgPluginRoot : public WgWidgetHolder, public WgRoot
{
	friend class WgWidget;

public:
	WgPluginRoot( wg_obj myPluginCapsule, WgPluginContext * pContext );
	~WgPluginRoot();

	WgWidget *				FindWidget( const WgCoord& ofs, WgSearchMode mode ) override;

	bool					DeleteChild( WgWidget * pWidget ) override;
	WgWidget *				ReleaseChild( WgWidget * pWidget ) override;

	bool					DeleteAllChildren() override;
	bool					ReleaseAllChildren() override;

	WgRoot *				CastToRoot() override;
	const WgRoot *			CastToRoot() const override;

	bool					IsRoot() const override;

	inline WgEventHandler *	EventHandler() const override { return m_pEventHandler; }
	inline int				Scale() const override { return m_scale*WG_SCALE_BASE/64; }

	inline wg::SizeI        CanvasSize() const { return m_size/64; }
	WgRect					PixelGeo() const { return m_size/64; }

	inline WgWidget *		Child() const { return m_hook.Widget(); }
	bool					SetChild( WgWidget * pWidget ) override;
	inline void				DeleteChild() { SetChild(0); }
	WgWidget * 				ReleaseChild();



	//.____ Internal ________________________________________________________

	wg::spx			_matchingHeight(wg::spx width, int scale = -1) const;
	wg::spx			_matchingWidth(wg::spx height, int scale = -1) const;

	wg::SizeSPX		_defaultSize(int scale = -1) const;
	wg::SizeSPX		_minSize(int scale = -1) const;
	wg::SizeSPX		_maxSize(int scale = -1) const;

	bool			_markTest(const wg::CoordSPX& ofs);

	void    		_preRender();
	void			_render(wg_obj device, const wg::RectSPX& _canvas, const wg::RectSPX& _window);

	void			_resize(const wg::SizeSPX& size, int scale = -1);
	void			_setState(wg::State state);

	wg::PointerStyle _pointerStyle() const;

	void			_setPointerPos( wg::CoordSPX pos, int64_t timestamp );
	void 			_setButtonState( int button, bool bPressed, int64_t timestamp );
	void 			_setKeyState( int nativeKeyCode, bool bPressed, int64_t timestamp );
	void			_putText( const char * pUTF8Text );
	void			_wheelRoll( int wheel, float distance, bool bInvert, int64_t timestamp );

	void			_update(int microPassed, int64_t microsecTimestamp);


protected:

	class Hook : public WgHook
	{
		friend class WgPluginRoot;
	public:
		Hook() : m_pRoot(0) {};				// So we can make them members and then make placement new...
		~Hook();

		const char *Type( void ) const;
		static const char * ClassType();

		WgCoord			PixelPos() const;
		WgSize			PixelSize() const;
		WgRect			PixelGeo() const;
		WgCoord			ScreenPixelPos() const;
		WgRect			ScreenPixelGeo() const;

		WgRoot *		Root() const;

	protected:

		void			_requestRender();
		void			_requestRender( const WgRect& rect );
		void			_requestResize();

		WgHook *		_prevHook() const;
		WgHook *		_nextHook() const;
		WgWidgetHolder * _holder() const;
		WgContainer *	_parent() const;

		WgPluginRoot *	m_pRoot;
		WgPluginContext_p	m_pContext;
	};

	void				_addPreRenderCall(WgWidget * pWidget) override;

	WgHook*				_firstHook() const override { return m_hook.Widget()? const_cast<Hook*>(&m_hook):0; }
	WgHook*				_lastHook() const override { return m_hook.Widget()? const_cast<Hook*>(&m_hook):0; }

	bool 				_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting ) override;
	bool 				_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing ) override;

	void                _inViewRequested( WgHook * pChild ) override;
	void                _inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea ) override;

	void				_childRequestResize();

	void				_childRequestRender();
	void				_childRequestRender( const WgRect& rect );


protected:
	Hook				m_hook;
	WgEventHandler *	m_pEventHandler = nullptr;

	wg_obj					m_pluginCapsule;

	std::vector<WgWidgetWeakPtr> m_preRenderCalls;

//	wg::GfxDevice_p			m_pGfxDevice;
//	wg::CanvasLayers_p		m_pCanvasLayers;

	wg::SizeSPX				m_size;
	int						m_scale = 64;
	wg::State				m_state;

	wg_pluginroot_calls		m_interface;

	int						m_microsecStored = 0;
	bool					m_bBlockRequestResize = false;

	WgPluginContext_p		m_pContext;

};


#endif //WG2_ROOTPANEL_DOT_H
