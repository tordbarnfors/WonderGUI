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
#ifndef WG_PLUGINROOT_DOT_H
#define WG_PLUGINROOT_DOT_H
#pragma once

#include <wg_plugininterface.h>
#include <wg_widget.h>
#include <wg_slotholder.h>
#include <wg_geo.h>
#include <wg_patches.h>
#include <wg_msgrouter.h>
#include <wg_gfxdevice.h>
#include <wg_slot.h>
#include <wg_root.h>
#include <wg_pluginbase.h>

namespace wg
{

	class Widget;


	class PluginRoot;
	typedef	StrongPtr<PluginRoot>	PluginRoot_p;
	typedef	WeakPtr<PluginRoot>	PluginRoot_wp;




	//____ PluginRoot ____________________________________________________________

	/**
	 * @brief	The root widget to use in a plugin.
	 *
	 * The root widget to use in a plugin.
	 *
	 *
	 */


	class PluginRoot : public Root, protected SlotHolder
	{
		friend class Widget;
		friend class Container;
		friend class InputHandler;

	public:

		//.____ Creation __________________________________________

		static PluginRoot_p	create();
		static PluginRoot_p	create( wg_obj myPluginCapsule, PluginContext * pContext );

		//.____ Components ____________________________________

		DynamicSlot			slot;

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;
		
		//.____ Geometry _________________________________________________

		int					scale() const override;
		Rect				geo() const override;

		//.____ State _________________________________________________

		inline Widget_p		focusedChild() const { return _focusedChild(); }
		
		//.____ Appearance ____________________________________________________

		void				setPointerStyle( PointerStyle style );
		inline PointerStyle pointerStyle() const override { return m_pointerStyle; }

		//.____ Misc ___________________________________________________________

		Widget_p			findWidget(const Coord& ofs, SearchMode mode) override;

		void				addDirtyPatch(const RectSPX& rect) override;
		bool				addPreRenderCall(Widget* pWidget) override;

		//.____ Internal ________________________________________________________

		spx			_matchingHeight(spx width, int scale = -1) const;
		spx			_matchingWidth(spx height, int scale = -1) const;

		SizeSPX		_defaultSize(int scale = -1) const;
		SizeSPX		_minSize(int scale = -1) const;
		SizeSPX		_maxSize(int scale = -1) const;

		bool		_markTest(const CoordSPX& ofs);

		void    	_preRender();
		void		_render(wg_obj device, const RectSPX& _canvas, const RectSPX& _window);

		void		_resize(const SizeSPX& size, int scale = -1);
		void		_setState(State state);

		void		_receive(Msg* pMsg);

		PointerStyle	_pointerStyle() const;
		
		void		_setPointerPos( CoordSPX pos, int64_t timestamp );
		void 		_setButtonState( int button, bool bPressed, int64_t timestamp );
		void 		_setKeyState( int nativeKeyCode, bool bPressed, int64_t timestamp );
		void		_putText( const char * pUTF8Text );
		void		_wheelRoll( int wheel, pts distance, bool bInvert, int64_t timestamp );

		
		void		_update(int microPassed, int64_t microsecTimestamp);


	protected:
		PluginRoot();
		PluginRoot(wg_obj myPluginCapsule, PluginContext * pContext);
		~PluginRoot();

		// SlotHolder methods

		const TypeInfo& _slotTypeInfo(const StaticSlot* pSlot) const override;

		Container*		_container() override;
		Root*			_root() override;
		int				_scale() const override;

		RectSPX			_slotGeo(const StaticSlot* pSlot) const override;
		void			_childOverflowChanged( StaticSlot * pSlot, const BorderSPX& oldOverflow, const BorderSPX& newOverflow ) override;

		RectSPX			_childLocalToGlobal(const StaticSlot* pSlot, const RectSPX& rect) const override;
		RectSPX			_globalToChildLocal(const StaticSlot* pSlot, const RectSPX& rect) const override;
		RectSPX			_globalPtsToChildLocalSpx(const StaticSlot* pSlot, const Rect& rect) const override;
		Rect			_childLocalSpxToGlobalPts(const StaticSlot* pSlot, const RectSPX& rect) const override;

		bool			_isChildVisible(const StaticSlot* pSlot) const override;
		RectSPX			_childWindowSection(const StaticSlot* pSlot) const override;

		void			_childRequestRender(StaticSlot* pSlot, const RectSPX& rect) override;
		void			_childRequestResize(StaticSlot* pSlot) override;

		bool			_childRequestFocus(StaticSlot* pSlot, Widget* pWidget, bool bRaiseWindow) override;
		bool			_childReleaseFocus(StaticSlot* pSlot, Widget* pWidget) override;

		void			_childRequestInView(StaticSlot* pSlot) override;
		void			_childRequestInView(StaticSlot* pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea) override;

		Widget*			_prevChild(const StaticSlot* pSlot) const override;
		Widget*			_nextChild(const StaticSlot* pSlot) const override;

		void			_releaseChild(StaticSlot* pSlot) override;
		void			_replaceChild(StaticSlot* pSlot, Widget* pNewChild) override;

		void			_didAddSlots(StaticSlot* pSlot, int nb) override;
		void			_didMoveSlots(StaticSlot* pFrom, StaticSlot* pTo, int nb) override;
		void			_willEraseSlots(StaticSlot* pSlot, int nb) override;





		Widget*			_findWidget(const CoordSPX& ofs, SearchMode mode);

		//		void				_setFocusedChild( Widget * pWidget );


		Widget*			_child() const override;
		Widget*			_focusedChild() const override;

		wg_obj			m_pluginCapsule;

		std::vector<Widget_wp>   m_preRenderCalls;

		PointerStyle		m_pointerStyle = PointerStyle::Undefined;
		
		GfxDevice_p			m_pGfxDevice;
		CanvasLayers_p		m_pCanvasLayers;

		RectSPX				m_size;
		int					m_scale = 64;
		State				m_state;

		Widget_wp			m_pFocusedChild;
		
		wg_pluginroot_calls	m_interface;

		PluginContext_p		m_pContext;
	};


} // namespace wg
#endif //WG_ROOTPANEL_DOT_H
