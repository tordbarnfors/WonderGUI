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

#ifndef WG_POPUPOVERLAY_DOT_H
#define WG_POPUPOVERLAY_DOT_H
#pragma once

#include <wg_overlay.h>
#include <wg_staticslotvector.h>

#include <vector>

namespace wg
{


	class PopupOverlay;
	typedef	StrongPtr<PopupOverlay>	PopupOverlay_p;
	typedef	WeakPtr<PopupOverlay>	PopupOverlay_wp;

	//____ PopupOverlay ____________________________________________________________

	class PopupOverlay : public Overlay
	{
		friend class Slot;
		friend class MySlots;

	public:

		//____ Slot ___________________________________________________________

		class Slot : public Overlay::Slot
		{
			friend class PopupOverlay;
			friend class MySlots;
			friend class StaticSlotVector<Slot>;

		public:
			const static bool safe_to_relocate = false;

			//.____ Identification ________________________________________________

			const static TypeInfo	TYPEINFO;

		protected:

			inline const RectSPX& _geo() const { return m_geo; }

			Slot(SlotHolder * pHolder) : Overlay::Slot(pHolder) {}

			enum class State
			{
				OpeningDelay,			// Popup is in "delayed opening" mode. Some ms before it starts to open.
				Opening,				// Popup is opening (fading in).
				PeekOpen,				// Popup is open until pointer leaves launcherGeo (mode only allowed if bPeek is set).
				WeakOpen,				// Popup is open, but closed if other entry of ancestors is peeked (mode only allowed if bPeek is set).
				FixedOpen,				// Popup is open until it is closed by a pop() call.
				ClosingDelay,			// Popup is in countdown to closing mode (mode only allowed if bPeek is set).
				Closing,				// Popup is closing (fading out).
			};

			RectSPX		m_launcherGeo;		// Launcher geo relative sibling or parent.
			BorderSPX	m_popupOverflow;	// Overflow graphics of popup when placing relative launcher.
			Placement	m_attachPoint = Placement::NorthWest;
			bool		m_bPeek;			// Has been opened in peek-mode.
			bool		m_bCloseOnSelect = true;	// Close popups on selection.
			State		m_state;
			int			m_stateCounter;		// Counts millisec the slot has been in a transitative state (Delay, Opening, Coundown and Closing).
			SizeSPX		m_maxSize = { spx_max, spx_max };
			Widget_wp	m_pOpener;			// Widget that opened this popup.
			Widget_wp	m_pKeyFocus;		// Pointer at widget that held focus when this popup was ontop.
		};


		//____ MySlots ________________________________________________________

		class MySlots : public StaticSlotVector<Slot>
		{
			friend class PopupOverlay;

		public:

			//.____ Content _______________________________________________________

			void	pushFront(const Widget_p& pPopup, Widget * pOpener, const Rect& launcherGeo, Placement attachPoint = Placement::NorthEast, bool bPeek = false, bool bCloseOnSelect = true, Size maxSize = Size(1000000, 1000000), Border overflow = Border() );
			void	pop(int nb = 1);
			void	pop(Widget * pPopup);
			void	clear();

			//.____ Internal ______________________________________________________

			void	_pushFront(const Widget_p& pPopup, Widget* pOpener, const RectSPX& launcherGeo, Placement attachPoint, bool bPeek, bool bCloseOnSelect, SizeSPX maxSize, BorderSPX overflow );


		protected:

			MySlots(SlotHolder * pHolder) : StaticSlotVector<Slot>(pHolder) {}

			const PopupOverlay *	_holder() const { return static_cast<const PopupOverlay*>(StaticSlotVector<Slot>::_holder()); }
			PopupOverlay *	_holder() { return static_cast<PopupOverlay*>(StaticSlotVector<Slot>::_holder()); }
		};

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			bool			blockLeftMouseButton = true;			// An open popup blocks other widgets from receiving mouse move and left button events. Other buttons still get through.
			int				closingDelay = 200;
			int				closingFade = 200;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			bool			pickHandle = false;
			int				openingDelay = 100;
			int				openingFade = 100;
			uint8_t			pickCategory = 0;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
			bool			usePickHandles = false;
		};

		//.____ Creation __________________________________________

		static PopupOverlay_p	create() { return PopupOverlay_p(new PopupOverlay()); }
		static PopupOverlay_p	create(const Blueprint& blueprint) { return PopupOverlay_p(new PopupOverlay(blueprint)); }

		//.____ Components _______________________________________

		MySlots	popupSlots;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		PopupOverlay();
		template<class BP> PopupOverlay(const BP& bp) : popupSlots(this), Overlay(bp)
		{
			m_bBlockLeftMouseButton = bp.blockLeftMouseButton;

			m_openingDelayMs	= bp.openingDelay;
			m_openingFadeMs		= bp.openingFade;
			m_closingDelayMs	= bp.closingDelay;
			m_closingFadeMs		= bp.closingFade;
		}
		virtual ~PopupOverlay();

		PopupOverlay *	_getPopupOverlay() const override { return const_cast<PopupOverlay*>(this); }

		void			_stealKeyboardFocus();
		void			_restoreKeyboardFocus();
		void			_updateGeo(Slot * pSlot, bool bForceResize = false);

		void			_refreshStayEnteredList();

		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		Widget *		_findWidget( const CoordSPX& ofs, SearchMode mode ) override;


		// Only base slot can have child replaced, we need error handling...
		void			_replaceChild(StaticSlot * pSlot, Widget * pNewChild) override { return Overlay::_replaceChild(pSlot, pNewChild); }

		void			_childRequestResize(StaticSlot * pSlot) override;
		void			_releaseChild(StaticSlot * pSlot) override;

		// Overloaded from Layer

		const Overlay::Slot * _beginOverlaySlots() const override;
		const Overlay::Slot * _endOverlaySlots() const override;
		int					_sizeOfOverlaySlot() const override;

		void				_onRequestRender(const RectSPX& rect, const Overlay::Slot * pSlot) override;	// rect is in our coordinate system.

		// Overloaded from container

//		void			_maskPatches(Patches& patches, const RectI& geo, const RectI& clip);


		// Overloaded from Widget

		void            _render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		void			_resize( const SizeSPX& size, int scale ) override;
		void			_receive( Msg * pMsg ) override;
		void			_update(int microPassed, int64_t microsecTimestamp) override;


		// Needed by MySlots

		void			_removeSlots(int ofs, int nb);
		void 			_removeTopSlotAndPeeks();

		void			_addSlot(Widget * pPopup, Widget * pOpener, const RectSPX& launcherGeo, Placement attachPoint, bool bPeek, bool bCloseOnSelect, SizeSPX maxSize, BorderSPX overflow);

		Widget_wp			m_pKeyFocus;	// Pointer at child that held focus before any menu was opened.

		int				m_openingDelayMs = 100;
		int				m_openingFadeMs = 100;
		int				m_closingDelayMs = 200;
		int				m_closingFadeMs = 200;

		bool			m_bBlockLeftMouseButton = true;
	};

} // namespace wg
#endif //WG_POPUPOVERLAY_DOT_H
