/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG_DEBUGOVERLAY_DOT_H
#define WG_DEBUGOVERLAY_DOT_H
#pragma once

#include <wg_overlay.h>
#include <wg_staticslotvector.h>
#include <wg_packpanel.h>
#include <wg_togglebutton.h>
#include <wg_debugbackend.h>
#include <wg_theme.h>

#include <wg_scrollpanel.h>

namespace wg
{


	class DebugOverlay;
	typedef	StrongPtr<DebugOverlay>	DebugOverlay_p;
	typedef	WeakPtr<DebugOverlay>	DebugOverlay_wp;

	//____ DebugOverlay __________________________________________________________

	class DebugOverlay : public Overlay
	{

	public:

		//____ WindowSlot ___________________________________________________________

		class WindowSlot : public Overlay::Slot
		{
			friend class DebugOverlay;
			friend class CDesignWindowSlotVector;
			template<class S> friend class StaticSlotVector;
			template<class S> friend class SlotVector;

		public:

			//.____ Identification ________________________________________________

			const static TypeInfo	TYPEINFO;

		protected:
			WindowSlot(SlotHolder * pHolder) : Overlay::Slot(pHolder) {}

			inline SlotHolder * _holder() { return static_cast<SlotHolder*>(Overlay::Slot::_holder()); }
			inline const SlotHolder * _holder() const { return static_cast<const SlotHolder*>(Overlay::Slot::_holder()); }

			const static bool safe_to_relocate = false;

			Placement	m_placement = Placement::NorthWest;
			CoordSPX	m_placementPos;			// Widgets pos relative placement.
			SizeSPX		m_chosenSize;
		};


		//____ CWindowVector _________________________________________________

		class CWindowVector : public StaticSlotVector<WindowSlot>
		{
			friend class DebugOverlay;

			CWindowVector(SlotHolder * pHolder) : StaticSlotVector<WindowSlot>(pHolder) {}
		};

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			DebugBackend_p	backend;									// Mandatory!!!
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::Undefined;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			Theme_p			theme;										// Mandatory!!!
			String			tooltip;
			bool			usePickHandles = false;
			Surface_p		icons;										// Mandatory!!!
			Surface_p		transparencyGrid;							// Mandatory!!! Chessboard pattern or similar
		};

		//.____ Creation __________________________________________

		static DebugOverlay_p	create( const Blueprint& blueprint ) { return DebugOverlay_p(new DebugOverlay(blueprint)); }

		//.____ Components _______________________________________

		CWindowVector	windows;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance _________________________________________________

		void				setWindowSkin(Skin * pSkin);
		inline Skin_p		windowSkin() const;

		void				setSelectionSkin(Skin * pSkin);
		inline Skin_p		selectionSkin() const;

		PointerStyle 		pointerStyle() const override;

		//.____ Control ____________________________________________________

		void				setActivated(bool bActivated);
		inline bool			isActivated() const;

		//.____ Internal ______________________________________________________

		virtual spx		_matchingHeight(spx width, int scale) const override;
		virtual spx		_matchingWidth(spx height, int scale) const override;

		SizeSPX			_defaultSize(int scale) const override;


	protected:
		DebugOverlay(const Blueprint& blueprint);
		virtual ~DebugOverlay();

	private:

		// Overloaded from Container

		Widget *		_findWidget( const CoordSPX& ofs, SearchMode mode ) override;
		void			_render(GfxDevice * pDevice, const RectSPX& canvas, const RectSPX& window) override;

		// Overloaded from SlotHolder

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		//TODO: We should allow replacement of modal slots.
		void			_replaceChild(StaticSlot * pSlot, Widget * pNewChild) override { return Overlay::_replaceChild(pSlot, pNewChild); }

		void			_childRequestResize( StaticSlot * pSlot ) override;
		void			_releaseChild(StaticSlot * pSlot) override;

		// Overloaded from Overlay

		const Overlay::Slot * 	_beginOverlaySlots() const override;
		const Overlay::Slot * 	_endOverlaySlots() const override;
		int					_sizeOfOverlaySlot() const override;

		// Overloaded from Widget

		void			_resize( const SizeSPX& size, int scale ) override;
		void			_receive( Msg * pMsg ) override;

		// Window creators

		std::tuple<Widget_p, PackPanel_p> _createWindow( const char * pTitle );

		void			_createToolboxWindow();
		void			_createWidgetInfoWindow();
		void			_createWidgetTreeWindow();
		void			_createMsgLogWindow();
		void			_createObjectInfoWindow();
		void			_createSkinInfoWindow();

		//

		void			_createResources();

		Placement		_windowFrameSection( CoordSPX pos, int windowIndex );
		int				_windowIndex(Widget* pWidget);						// Lookup which window the widget is part of.


		RectSPX			_selectionGeo() const;
		void			_refreshRealGeo(WindowSlot * pSlot, bool bForceResize = false);
		void			_selectWidget(Widget * pWidget);
		void			_selectSkin(Skin* pSkin);
		void			_selectObject(Object* pSelected, Object * pSelectedFrom);

		//

		DebugBackend_p	m_pBackend;

		PackLayout_p	m_pPackLayoutForScrollingContent;

		PointerStyle	m_generatedPointerStyle = PointerStyle::Undefined;

		ToggleButton_p	m_pPickWidgetButton;

		bool			m_bActivated = false;
		bool			m_bSelectMode = false;

		Widget_wp		m_pSelectedWidget;
		Skin_p			m_pSelectionSkin;
		Skin_p			m_pWindowSkin;

		PackPanel_p		m_pWidgetTools;

		ScrollPanel_p	m_pWidgetTreeContainer;
		PackPanel_p		m_pSkinContainer;
		PackPanel_p		m_pAnyObjectContainer;			// Contains the object info panel for the currently selected object.

		// Variables for window drag

		int				m_movingWindow = -1;			// Index for Window that is being moved.
		CoordSPX		m_movingWindowStartOfs;

		// Variables for window resize

		int				m_resizingWindow = -1;			// Index for Window that is being resized.
		Placement		m_resizingWindowDirection = Placement::Undefined;
		RectSPX			m_resizingWindowStartGeo;

		// Resources

		Theme_p			m_pTheme;
		Surface_p		m_pIcons;
		Surface_p		m_pTransparencyGrid;

		Skin_p			m_pSelectIcon;

		Skin_p			m_pRefreshIcon;
		Skin_p			m_pExpandIcon;
		Skin_p			m_pCondenseIcon;

		TextLayout_p	m_pHeaderLayout;

		IDebugger::Blueprint	m_debugPanelBP;

	};

	//____ windowSkin() ______________________________________________________

	Skin_p DebugOverlay::windowSkin() const
	{
		return m_pWindowSkin;
	}

	//____ selectionSkin() ____________________________________________________

	Skin_p DebugOverlay::selectionSkin() const
	{
		return m_pSelectionSkin;
	}

	//____ isActivated() _______________________________________________________

	bool DebugOverlay::isActivated() const
	{
		return m_bActivated;
	}


} // namespace wg
#endif //WG_DEBUGOVERLAY_DOT_H
