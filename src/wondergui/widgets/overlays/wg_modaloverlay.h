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
#ifndef WG_MODALOVERLAY_DOT_H
#define WG_MODALOVERLAY_DOT_H
#pragma once

#include <wg_overlay.h>
#include <wg_dynamicslotvector.h>

namespace wg
{

	class ModalOverlay;
	typedef	StrongPtr<ModalOverlay>	ModalOverlay_p;
	typedef	WeakPtr<ModalOverlay>		ModalOverlay_wp;

	//____ ModalOverlay __________________________________________________________

	class ModalOverlay : public Overlay
	{
		friend class Slot;
		friend class MySlots;

	public:

		//____ Slot ___________________________________________________________

		class Slot : public Overlay::Slot
		{
			friend class ModalOverlay;
			friend class MySlots;
			template<class S> friend class DynamicSlotVector;
			template<class S> friend class SlotVector;

		public:

			//.____ Blueprint _____________________________________________________
			
			struct Blueprint
			{
				Placement	origo = Placement::Center;
				Coord		pos;
				Size		size;
			};
			
			//.____ Identification ________________________________________________

			const static TypeInfo	TYPEINFO;

			//.____ Geometry _________________________________________________

			void			setOrigo(const Placement origo);
			inline Placement	origo() const { return m_origo; }

			void			setGeo(const Rect& geometry);

			void			setPos(const Coord& ofs);
			inline Coord	pos() const { return m_placementGeo.pos(); }

			void			setSize(const Size& size);

			void			move(const Coord& ofs);

		protected:

			Slot(SlotHolder * pHolder) : Overlay::Slot(pHolder) {}

			bool _setBlueprint( const Blueprint& bp );
			
			inline ModalOverlay * _holder() { return static_cast<ModalOverlay*>(Overlay::Slot::_holder()); }
			inline const ModalOverlay * _holder() const { return static_cast<const ModalOverlay*>(Overlay::Slot::_holder()); }

			const static bool safe_to_relocate = false;

			bool		m_bVisible = true;
			Placement	m_origo = Placement::Center;
			Rect		m_placementGeo;			// Widgets geo relative origo. Setting width and height to 0 uses Widgets defaultSize() dynamically.
												// Setting just one of them to 0 uses Widgets matchingHeight() or matchingWidth() dynamically.
			Widget_wp	m_pKeyFocus;			// Pointer at child that held focus when this modal was last on top.

		};

		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
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
			bool			takesFocusFromChild = false;
			String			tooltip;
			bool			usePickHandles = false;
		};

		//.____ Creation __________________________________________

		static ModalOverlay_p	create() { return ModalOverlay_p(new ModalOverlay()); }
		static ModalOverlay_p	create( const Blueprint& blueprint ) { return ModalOverlay_p(new ModalOverlay(blueprint)); }

		//.____ Components _______________________________________

		DynamicSlotVector<Slot>		modalSlots;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal ______________________________________________________

		spx				_matchingHeight(spx width, int scale) const override;
		spx				_matchingWidth(spx height, int scale) const override;

		SizeSPX			_defaultSize(int scale) const override;

	protected:
		ModalOverlay();
		
		template<class BP> ModalOverlay(const BP& bp) : modalSlots(this), Overlay(bp) {}
		
		virtual ~ModalOverlay();


	private:

		void            _refreshRealGeo(Slot * pSlot, bool bForceResize = false);

		ModalOverlay *	_getModalOverlay() const  override { return const_cast<ModalOverlay*>(this); }

		void			_updateKeyboardFocus();

		// Overloaded from Widget

		void			_resize(const SizeSPX& size, int scale) override;
		void			_receive(Msg * pMsg) override;

		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		Widget *		_findWidget( const CoordSPX& ofs, SearchMode mode ) override;

		//TODO: We should allow replacement of modal slots.
		void			_replaceChild(StaticSlot * pSlot, Widget * pNewChild) override { return Overlay::_replaceChild(pSlot, pNewChild); }

		void			_childRequestResize( StaticSlot * pSlot ) override;
		void			_releaseChild(StaticSlot * pSlot) override;

		void			_didAddSlots(StaticSlot * pSlot, int nb) override;
		void			_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb) override;
		void			_willEraseSlots(StaticSlot * pSlot, int nb) override;
		void			_hideSlots(StaticSlot *, int nb);
		void			_unhideSlots(StaticSlot *, int nb);

		// Overloaded from Overlay

		const Overlay::Slot * _beginOverlaySlots() const override;
		const Overlay::Slot * _endOverlaySlots() const override;
		int					_sizeOfOverlaySlot() const override;

		Widget_wp			m_pBaseKeyFocus;
	};


} // namespace wg
#endif //WG_MODALOVERLAY_DOT_H
