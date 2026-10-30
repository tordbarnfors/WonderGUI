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

			RectSPX		launcherGeo() const { return m_launcherGeo; }
			Placement	attachPoint() const { return m_attachPoint; }
			bool		peekMode() const { return m_bPeek; }
			bool		closeOnSelect() const { return m_bCloseOnSelect; }
			bool		takeFocus() const { return m_bTakeFocus; }
			SizeSPX		maxSize() const { return m_maxSize; }
			Widget_wp	opener() const { return m_pOpener; }

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
			bool		m_bTakeFocus = true;		// Take keyboard focus when opened and give it back when closed.
			State		m_state;
			int			m_stateCounter;		// Counts millisec the slot has been in a transitative state (Delay, Opening, Coundown and Closing).
			SizeSPX		m_maxSize = { spx_max, spx_max };
			Widget_wp	m_pOpener;			// Widget that opened this popup.
			Widget_wp	m_pKeyFocus;		// Pointer at widget that held focus before this popup took it.
		};


		//____ MySlots ________________________________________________________

		class MySlots : public StaticSlotVector<Slot>
		{
			friend class PopupOverlay;

		public:

			//.____ Content _______________________________________________________

			void	pushFront(const Widget_p& pPopup, Widget * pOpener, const Rect& launcherGeo, Placement attachPoint = Placement::NorthEast, bool bPeek = false, bool bCloseOnSelect = true, Size maxSize = Size(1000000, 1000000), Border overflow = Border(), bool bTakeFocus = true );
			void	pop(int nb = 1);
			void	pop(Widget * pPopup);
			void	clear();

			//.____ Internal ______________________________________________________

			void	_pushFront(const Widget_p& pPopup, Widget* pOpener, const RectSPX& launcherGeo, Placement attachPoint, bool bPeek, bool bCloseOnSelect, SizeSPX maxSize, BorderSPX overflow, bool bTakeFocus );


		protected:

			MySlots(SlotHolder * pHolder) : StaticSlotVector<Slot>(pHolder) {}

			const PopupOverlay *	_holder() const { return static_cast<const PopupOverlay*>(StaticSlotVector<Slot>::_holder()); }
			PopupOverlay *	_holder() { return static_cast<PopupOverlay*>(StaticSlotVector<Slot>::_holder()); }
		};

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			int				aimStillTime = 100;						// Millisec without movement before a pointer is no longer considered aiming at a submenu.
			int				aimTimeout = 300;						// Millisec on the same widget before a pointer is no longer considered aiming at a submenu. 0 disables aiming.
			pts				aimTolerance = 20;						// Points the near edge of a submenu is extended by at both ends when checking if the pointer aims at it.
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
			bool			takesFocusFromChild = false;
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

		//.____ Internal ______________________________________________________

		bool			_isAimingPast( Widget * pHovered, const CoordSPX& pointerPos, int64_t timestamp, int64_t hoveredSince );

	protected:
		PopupOverlay();
		template<class BP> PopupOverlay(const BP& bp) : popupSlots(this), Overlay(bp)
		{
			m_bBlockLeftMouseButton = bp.blockLeftMouseButton;

			m_openingDelayMs	= bp.openingDelay;
			m_openingFadeMs		= bp.openingFade;
			m_closingDelayMs	= bp.closingDelay;
			m_closingFadeMs		= bp.closingFade;

			m_aimStillTimeMs	= bp.aimStillTime;
			m_aimTimeoutMs		= bp.aimTimeout;
			m_aimTolerance		= bp.aimTolerance;
		}
		virtual ~PopupOverlay();

		PopupOverlay *	_getPopupOverlay() const override { return const_cast<PopupOverlay*>(this); }

		void			_stealKeyboardFocus();
		void			_restoreKeyboardFocus( Widget * pFocus );
		void			_updateGeo(Slot * pSlot, bool bForceResize = false);

		void			_refreshStayEnteredList();

		void			_processPointer( const CoordSPX& pointerPos, int64_t timestamp );

		void			_addAimSample( const CoordSPX& pointerPos, int64_t timestamp );
		Slot *			_slotOf( Widget * pWidget );
		Slot *			_aimTarget( Widget * pHovered );
		bool			_isAimingAt( Slot * pTarget, const CoordSPX& pointerPos, int64_t timestamp, int64_t hoveredSince );

		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		Widget *		_findWidget( const CoordSPX& ofs, SearchMode mode ) override;
		Widget *		_findWidgetIgnoringAim( const CoordSPX& ofs, SearchMode mode );


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

		void			_addSlot(Widget * pPopup, Widget * pOpener, const RectSPX& launcherGeo, Placement attachPoint, bool bPeek, bool bCloseOnSelect, SizeSPX maxSize, BorderSPX overflow, bool bTakeFocus);

		int				m_openingDelayMs = 100;
		int				m_openingFadeMs = 100;
		int				m_closingDelayMs = 200;
		int				m_closingFadeMs = 200;

		bool			m_bBlockLeftMouseButton = true;

		// Safe triangle: a submenu stays open, and its siblings wait with opening, while the
		// pointer moves towards it across other entries of the menu it was opened from.

		int				m_aimStillTimeMs = 100;
		int				m_aimTimeoutMs = 300;
		pts				m_aimTolerance = 20;

		const static int c_maxAimSamples = 8;
		const static int c_minAimDistance = 8;			// Points between the pointer and the start of the triangle, unless all samples are closer.
		const static int c_maxAimSlope = 3;				// Steepest heading that counts as aiming, as distance along the near edge per distance towards it.

		CoordSPX		m_aimSamples[c_maxAimSamples];	// Latest distinct pointer positions, newest first.
		int				m_nbAimSamples = 0;
		int64_t			m_aimMoveTimestamp = 0;			// When the pointer last moved.
		Widget_wp		m_pAimHovered;					// Widget under the pointer when it last moved.
		int64_t			m_aimHoveredTimestamp = 0;		// When m_pAimHovered came under the pointer.
		bool			m_bAimHeld = false;				// A popup was kept from closing since the pointer aims at it.
		bool			m_bAimRedirected = false;		// The opener of a popup the pointer aims at was marked instead of the opener under it.
	};

} // namespace wg
#endif //WG_POPUPOVERLAY_DOT_H
