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
#ifndef WG_LEGACYROOTCAPSULE_DOT_H
#define WG_LEGACYROOTCAPSULE_DOT_H
#pragma once

#include <wg_widget.h>
#include <wg2_eventhandler.h>

#include <wg2_container.h>
#include <wg2_geo.h>
#include <wg2_root.h>

#include <cstring>

//TODO: Need to support skin padding.

namespace wg
{

	class LegacyRootCapsule;
	typedef	StrongPtr<LegacyRootCapsule>	LegacyRootCapsule_p;
	typedef	WeakPtr<LegacyRootCapsule>		LegacyRootCapsule_wp;



	class LegacyRootCapsule : public Widget, public WgWidgetHolder, public WgRoot
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{

			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool 			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Arrow;
			int				scale = 64;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Creation __________________________________________

		static LegacyRootCapsule_p	create( );
		static LegacyRootCapsule_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance ____________________________________________________

		String			tooltip() const override;
		PointerStyle	pointerStyle() const override;

		//.____ Internal _________________________________________________

		spx			_matchingHeight(spx width, int scale = -1) const override;
		spx			_matchingWidth(spx height, int scale = -1) const override;

		SizeSPX		_defaultSize(int scale = -1) const override;
		SizeSPX		_minSize(int scale = -1) const override;
		SizeSPX		_maxSize(int scale = -1) const override;

		void    	_preRender() override;
		void		_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;

		void		_resize(const SizeSPX& size, int scale = -1) override;
		void		_setState(State state) override;

		void		_receive(Msg* pMsg) override;
		bool		_alphaTest(const CoordSPX& ofs) override;

		void		_update(int microPassed, int64_t microsecTimestamp) override;


		//.____ Misc _________________________________________________
		// WG2 Interface

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

	protected:
		LegacyRootCapsule();

		template<class BP> LegacyRootCapsule(const BP& bp) : Widget(bp), m_hook(this)
		{
			m_pEventHandler = new WgEventHandler(this);

			_routeKeyEvents();

			_startReceiveUpdates();

		}

		~LegacyRootCapsule();

		void _routeKeyEvents();

		inline void         _addPreRenderCall(WgWidget * pWidget) override { m_preRenderCalls.push_back(pWidget); }

		WgHook*				_firstHook() const override { return m_hook.Widget()? const_cast<Hook*>(&m_hook):0; }
		WgHook*				_lastHook() const override { return m_hook.Widget()? const_cast<Hook*>(&m_hook):0; }

		bool 				_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting ) override;
		bool 				_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing ) override;

		void                _inViewRequested( WgHook * pChild ) override;
		void                _inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea ) override;

		void				_childRequestResize();

		void				_childRequestRender();
		void				_childRequestRender( const WgRect& rect );


		class Hook : public WgHook
		{
			friend class LegacyRootCapsule;
		public:
			Hook(LegacyRootCapsule* pRoot) : m_pRoot(pRoot) {};				// So we can make them members and then make placement new...
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


			virtual void	_requestRender();
			virtual void	_requestRender( const WgRect& rect );
			virtual void	_requestResize();

			virtual bool	_requestFocus();
			virtual bool	_releaseFocus();

			virtual void    _requestInView();
			virtual void    _requestInView( const WgRect& mustHaveArea, const WgRect& niceToHaveArea );

			WgHook *		_prevHook() const;
			WgHook *		_nextHook() const;
			WgWidgetHolder * _holder() const;
			WgContainer *	_parent() const;

			LegacyRootCapsule *	m_pRoot;
		};

		Hook				m_hook;
		WgEventHandler *	m_pEventHandler = nullptr;

		std::vector<WgWidgetWeakPtr> m_preRenderCalls;

		int						m_microsecStored = 0;
		bool					m_bBlockRequestResize = false;
		bool					m_bMouseInside = false;

		RouteId		m_keyPressRoute;
		RouteId		m_keyReleaseRoute;	

	};

} // namespace wg
#endif //WG_LEGACYROOTCAPSULE_DOT_H
