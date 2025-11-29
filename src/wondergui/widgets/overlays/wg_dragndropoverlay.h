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
#ifndef WG_DRAGNDROPOVERLAY_DOT_H
#define WG_DRAGNDROPOVERLAY_DOT_H
#pragma once

#include <wg_overlay.h>
#include <wg_dataset.h>
#include <wg_key.h>

namespace wg
{

	class   DragNDropOverlay;
	typedef	StrongPtr<DragNDropOverlay>	DragNDropOverlay_p;
	typedef	WeakPtr<DragNDropOverlay>		DragNDropOverlay_wp;

	//____ DragNDropOverlay ____________________________________________________________

	class DragNDropOverlay : public Overlay
	{
	public:

		//____ Slot ___________________________________________________________

		class Slot : public Overlay::Slot
		{
			friend class DragNDropOverlay;
		public:

			//.____ Identification ________________________________________________

			const static TypeInfo	TYPEINFO;

		protected:

			Slot(SlotHolder * pHolder) : Overlay::Slot(pHolder) {}
		};




		//.____ Creation __________________________________________

		static DragNDropOverlay_p  create() { return DragNDropOverlay_p(new DragNDropOverlay()); }
		static DragNDropOverlay_p  create( const Blueprint& blueprint ) { return DragNDropOverlay_p(new DragNDropOverlay(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ State _______________________________________________________________

		bool			isDragInProgress() const;

	protected:
		DragNDropOverlay();
		template<class BP> DragNDropOverlay(const BP& bp) : m_dragSlot(this), Overlay(bp) {}
		virtual ~DragNDropOverlay();


//        DragNDropOverlay *    _getDragNDropOverlay() const { return const_cast<DragNDropOverlay*>(this); }

		// Overloaded from Widget

		void            _receive(Msg * pMsg) override;
		void            _render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;

		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		Widget *        _findWidget( const CoordSPX& ofs, SearchMode mode ) override;

		void            _childRequestResize(StaticSlot * pSlot) override;
		void			_releaseChild(StaticSlot * pSlot) override;

		// Overloaded from Overlay

		const Overlay::Slot *	_beginOverlaySlots() const override;
		const Overlay::Slot *	_endOverlaySlots() const override;
		int					_sizeOfOverlaySlot() const override;

		void				_onRequestRender(const RectSPX& rect, const Overlay::Slot * pSlot) override;    // rect is in our coordinate system.

		// Internal

		void            _complete( Widget * pDeliveredTo );
		void            _cancel();
		void            _replaceDragWidget( Widget * pNewWidget );



		enum DragState
		{
			Idle,
			Picking,		// Mouse button pressed, awaiting drag to pass treshold
			Picked,			// Drag passed treshold, PickMsg sent.
			Dragging,		// We have a dataset, a drag-widget and are dragging.
			Targeting,      // We are hovering a widget that has accepted our target probing.
			Delivering,     // We have released mouse button on a targeted widget. Deliver + Complete/Cancel cycle is taking place.
		};

		DragState		m_dragState = DragState::Idle;

		Slot			m_dragSlot;            // Slot for widget being dragged, when it is dragged.

		Widget_p		m_pPicked;
		DropType		m_dropType = DropType::Undefined;
		int				m_category = 0;
		BasicDataset_p	m_pDataset;
		RectSPX			m_dragRestriction = {0, 0, 0, 0};

		pts				m_dragStartTreshold;
		CoordSPX		m_dragWidgetOfs;                // Drag widgets offset from pointer.
		CoordSPX		m_hotspotOfs;					// Hotspot offset from pointer.

		Widget_wp       m_pProbed;                     // Last widget we sent a DropProbeMsg to. To avoid sending multiple messages in a row to same while hovering.
		Widget_wp       m_pTargeted;                   // Widget targeted while in state Targeting.
	};




} // namespace wg
#endif //WG_DRAGNDROPOVERLAY_DOT_H

