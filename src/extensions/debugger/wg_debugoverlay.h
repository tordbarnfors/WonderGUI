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
#include <wg_debugger.h>
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

		//____ ToolboxSlot ___________________________________________________________

		class ToolboxSlot : public Overlay::Slot
		{
			friend class DebugOverlay;
			friend class CDesignToolboxSlotVector;
			template<class S> friend class StaticSlotVector;
			template<class S> friend class SlotVector;

		public:

			//.____ Identification ________________________________________________

			const static TypeInfo	TYPEINFO;

		protected:
			ToolboxSlot(SlotHolder * pHolder) : Overlay::Slot(pHolder) {}

			inline SlotHolder * _holder() { return static_cast<SlotHolder*>(Overlay::Slot::_holder()); }
			inline const SlotHolder * _holder() const { return static_cast<const SlotHolder*>(Overlay::Slot::_holder()); }

			const static bool safe_to_relocate = false;

			Placement	m_placement = Placement::NorthWest;
			CoordSPX	m_placementPos;			// Widgets pos relative placement.
			SizeSPX		m_chosenSize;
		};


		//____ CToolboxVector _________________________________________________

		class CToolboxVector : public StaticSlotVector<ToolboxSlot>
		{
			friend class DebugOverlay;

			CToolboxVector(SlotHolder * pHolder) : StaticSlotVector<ToolboxSlot>(pHolder) {}
		};

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			Debugger_p		debugger;									// Mandatory!!!
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
		};

		//.____ Creation __________________________________________

		static DebugOverlay_p	create( const Blueprint& blueprint ) { return DebugOverlay_p(new DebugOverlay(blueprint)); }

		//.____ Components _______________________________________

		CToolboxVector	toolboxes;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance _________________________________________________

		void				setToolboxSkin(Skin * pSkin);
		inline Skin_p		paletteSkin() const;

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

		// Toolbox creators

		std::tuple<Widget_p, PackPanel_p> _createToolbox( const char * pTitle );

		void			_createSlotWidgetToolbox();
		void			_createWidgetTreeToolbox();




		Widget_p		_createSlotInfoPanel(StaticSlot * pSlot);
		Widget_p		_createWidgetInfoPanel(Widget * pWidget);

		//

		void			_createResources();

		Placement		_boxSection( CoordSPX pos, int boxIndex );
		int				_boxIndex(Widget* pWidget);						// Lookup which box the widget is part of.


		RectSPX			_selectionGeo() const;
		void			_refreshRealGeo(ToolboxSlot * pSlot, bool bForceResize = false);
		void			_selectWidget(Widget * pWidget);
		//

		Debugger_p		m_pDebugger;

		PointerStyle	m_generatedPointerStyle = PointerStyle::Undefined;

		ToggleButton_p	m_pPickWidgetButton;

		bool			m_bActivated = false;
		bool			m_bSelectMode = false;

		Widget_wp		m_pSelectedWidget;
		Skin_p			m_pSelectionSkin;
		Skin_p			m_pToolboxSkin;

		PackPanel_p		m_pSlotTools;
		PackPanel_p		m_pWidgetTools;

		ScrollPanel_p	m_pWidgetTreeContainer;

		// Variables for toolbox drag

		int				m_movingToolbox = -1;			// Index for toolbox that is being moved.
		CoordSPX		m_movingToolboxStartOfs;

		// Variables for toolbox resize

		int				m_resizingToolbox = -1;			// Index for toolbox that is being resized.
		Placement		m_resizingToolboxDirection = Placement::Undefined;
		RectSPX			m_resizingToolboxStartGeo;

		// Resources

		Theme_p			m_pTheme;

		DebugPanel::Blueprint	m_debugPanelBP;

	};

	//____ paletteSkin() ______________________________________________________

	Skin_p DebugOverlay::paletteSkin() const
	{
		return m_pToolboxSkin;
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
