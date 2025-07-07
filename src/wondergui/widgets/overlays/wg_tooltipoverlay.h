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

#ifndef WG_TOOLTIPOVERLAY_DOT_H
#define WG_TOOLTIPOVERLAY_DOT_H
#pragma once

#include <wg_overlay.h>
#include <wg_slot.h>

namespace wg
{


	class TooltipOverlay;
	typedef	StrongPtr<TooltipOverlay>	TooltipOverlay_p;
	typedef	WeakPtr<TooltipOverlay>	TooltipOverlay_wp;

	//____ TooltipOverlay ____________________________________________________________

	class TooltipOverlay : public Overlay
	{
	public:

		//

		struct Position
		{
			Placement	placement = Placement::SouthEast;
			Border		spacing = { 2, 16, 16, 2 };
			bool		bAroundPointer = true;
		};

		//.____ Blueprint __________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			int				delay = 300;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			std::function<Widget_p(Position& position, const Widget* pHovered, const Border& margin)> generator;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			Position		position;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
			bool			usePickHandles = false;
		};

		//.____ Creation __________________________________________

		static TooltipOverlay_p	create() { return TooltipOverlay_p(new TooltipOverlay()); }
		static TooltipOverlay_p	create(const Blueprint& blueprint) { return TooltipOverlay_p(new TooltipOverlay(blueprint)); }

		//.____ Identification ________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


		//.____ Control _______________________________________________________

		void setTooltipGenerator( const std::function<Widget_p(Position& position, const Widget* pHoveredWidget, const Border& widgetMargins )> func );
		void setTooltipPosition(Placement direction, Border spacing, bool bTooltipAroundPointer = true );

		void setDisplayTooltips(bool bDisplay);
		bool displayTooltips() const { return m_bDisplayTooltips;  }


	protected:
		TooltipOverlay();
		template<class BP> TooltipOverlay(const BP& bp) : m_tooltipSlot(this), Overlay(bp)
		{
			m_hoverMillisec = bp.delay;
			m_defaultPosition = bp.position;
			m_tooltipGenerator = bp.generator;
		}
		virtual ~TooltipOverlay();


		class Slot : public Overlay::Slot
		{
			friend class TooltipOverlay;
		public:
			const static TypeInfo	TYPEINFO;

		protected:

			Slot(SlotHolder* pHolder) : Overlay::Slot(pHolder) {}
		};


		void			_updateTooltipGeo();

		void			_openTooltip();
		void			_closeTooltip();

		static Widget_p		_defaultTooltipGenerator(Position& position, const Widget*, const Border&);

		// Overloaded from Widget

		void			_receive(Msg* pMsg) override;
		void			_update(int microPassed, int64_t microsecTimestamp) override;

		// Overloaded from Container

		const TypeInfo& _slotTypeInfo(const StaticSlot* pSlot) const override;

		Widget*			_findWidget(const CoordSPX& ofs, SearchMode mode) override;

		void			_childRequestResize(StaticSlot* pSlot) override;


		// Overloaded from Layer

		const Overlay::Slot*	_beginOverlaySlots() const override;
		const Overlay::Slot*	_endOverlaySlots() const override;
		int					_sizeOfOverlaySlot() const override;

		Slot		m_tooltipSlot;

		int			m_hoverMillisec		= 300;				// Number of millisec to hover before tooltip is displayed.
		CoordSPX	m_hoverPos			= { -1,-1 };
		int			m_hoverCountdown	= 0;
		Widget*		m_pHoverWidget = nullptr;

		int			m_openTooltipMaxMovement = 8;			// How many points the pointer may move in any direction
															// from hoverPos once tooltip is displayed before it is closed.
		bool		m_bBlockReopen		= false;
		bool		m_bDisplayTooltips	= true;

		Position	m_defaultPosition;
		Position	m_activePosition;

		std::function<Widget_p(Position& position, const Widget* pHovered, const Border& margin)>	m_tooltipGenerator = _defaultTooltipGenerator;

	};

} // namespace wg
#endif //WG_POPUPOVERLAY_DOT_H
