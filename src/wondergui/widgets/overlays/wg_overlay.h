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
#ifndef WG_OVERLAY_DOT_H
#define WG_OVERLAY_DOT_H
#pragma once

#include <wg_container.h>
#include <wg_slot.h>
#include <wg_util.h>


namespace wg
{

	class Overlay;
	typedef	StrongPtr<Overlay>	Overlay_p;
	typedef	WeakPtr<Overlay>		Overlay_wp;


	//____ Overlay ______________________________________________________________
	/**
	 * @brief	Base class for containers that provides layers of different kinds.
	 *
	 * Overlay is the base class for widget containers that are used as overlays.
	 *
	 * The concept of Overlays in WonderGUI is that an Overlay is a widget that provides
	 * a special purpose overlay on top of the rest of the GUI hierarchy. The placement of
	 * this overlay in the widget hierarchy decides where things such as popups,
	 * modal dialog boxes, tooltips, notifications etc appear. (note: not all of these
	 * mentioned conceptual layers are implemented yet).
	 *
	 * For example, a Menubar widget that opens a menu will place the menu widget in the
	 * first PopupOverlay it encounters while traversing it ancestors from itself upwards.
	 * In similar way, placing a widget in a ModalOverlay will make it modal only against the
	 * children of that specific overlay.
	 *
	 * All overlays have a base child, which is the child through which the normal hierarchy continues.
	 * Children that resides within the layer itself are usually referred to as layer children.
	 *
	 **/

	class Overlay : public Container
	{

	public:

		//____ Slot ___________________________________________________________

		class Slot : public StaticSlot
		{
			friend class Overlay;

		public:

			//.____ Identification ________________________________________________

			const static TypeInfo	TYPEINFO;

			//.____ Geometry _________________________________________________

			inline Coord	pos() const { return Util::spxToPts(m_geo.pos(),m_pHolder->_scale()); }
			inline Size		size() const { return Util::spxToPts(m_geo.size(), m_pHolder->_scale()); }
			inline Rect		geo() const { return Util::spxToPts(m_geo, m_pHolder->_scale()); }

		protected:
			Slot(SlotHolder * pHolder) : StaticSlot(pHolder) {}

			inline void _setSize(SizeSPX size, int scale) { m_geo.setSize(size); StaticSlot::_setSize(size, scale); }
			inline void _setGeo(RectSPX geo, int scale) { m_geo = geo; StaticSlot::_setSize(geo.size(), scale); }

			RectSPX		m_geo;
			bool		m_bVisible = true;
		};


		//.____ Components _______________________________________

		DynamicSlot		mainSlot;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal ______________________________________________________

		virtual spx		_matchingHeight(spx width, int scale) const override;
		virtual spx		_matchingWidth(spx height, int scale) const override;

		SizeSPX			_defaultSize(int scale) const override;

	protected:
		Overlay();
		template<class BP> Overlay(const BP& bp) : mainSlot(this), Container(bp) {}

		// Overloaded from Container

		Widget *	_firstChild() const override;
		Widget *	_lastChild() const override;

		void		_firstSlotWithGeo(SlotWithGeo& package) const override;
		void		_nextSlotWithGeo(SlotWithGeo& package) const override;

		RectSPX		_slotGeo( const StaticSlot * pSlot ) const override;

		void		_childRequestRender( StaticSlot * pSlot, const RectSPX& rect ) override;
//		void		_childRequestResize( StaticSlot * pSlot ) override;

		Widget *	_prevChild( const StaticSlot * pSlot ) const override;
		Widget *	_nextChild( const StaticSlot * pSlot ) const override;

		void		_releaseChild( StaticSlot * pSlot ) override;
		void		_replaceChild(StaticSlot * pSlot, Widget * pNewWidget) override;

		// Overloaded from Widget

		void		_resize(const SizeSPX& size, int scale) override;



		//

		virtual	void	_onRequestRender( const RectSPX& rect, const Slot * pSlot );	// rect is in our coordinate system.

		virtual const Slot * _beginOverlaySlots() const = 0;
		virtual const Slot * _endOverlaySlots() const = 0;
		virtual int			_sizeOfOverlaySlot() const = 0;

		inline Slot * _beginOverlaySlots() { return const_cast<Slot*>(const_cast<const Overlay*>(this)->_beginOverlaySlots()); }
		inline Slot * _endOverlaySlots() { return const_cast<Slot*>(const_cast<const Overlay*>(this)->_endOverlaySlots()); }

		inline Slot * _incOverlaySlot( Slot * pSlot, int sizeOf ) const { return (Slot*) (((char*)pSlot)+sizeOf); }
		inline const Slot * _incOverlaySlot( const Slot * pSlot, int sizeOf ) const { return (const Slot*) (((char*)pSlot)+sizeOf); }

		inline Slot * _decOverlaySlot( Slot * pSlot, int sizeOf ) const { return (Slot*) (((char*)pSlot)-sizeOf); }
		inline const Slot * _decOverlaySlot( const Slot * pSlot, int sizeOf ) const { return (const Slot*) (((char*)pSlot)-sizeOf); }

	};



} // namespace wg
#endif //WG_OVERLAY_DOT_H
