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

#ifndef	WG_CONTAINER_DOT_H
#define	WG_CONTAINER_DOT_H
#pragma once

#include <wg_widget.h>
#include <wg_slot.h>


namespace wg
{

	class ModalOverlay;
	class PopupOverlay;

	class Container;
	typedef	StrongPtr<Container>	Container_p;
	typedef	WeakPtr<Container>		Container_wp;

	/**
	 * @brief Base class for all widgets that can hold child widgets.
	 *
	 * Base class for all widgets that can hold child widgets.
	 */

	class Container : public Widget, protected SlotHolder
	{
		friend class SlotHolder;
		friend class CanvasCapsule;
		friend class SelectCapsule;

		public:

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
				String			tooltip;
				bool			usePickHandles = false;
			};

			//.____ Identification __________________________________________

			const TypeInfo&		typeInfo(void) const override;
			const static TypeInfo	TYPEINFO;

			bool					isContainer() const override;

			//.____ Hierarchy _________________________________________________

			inline Widget_p			firstChild() const { return Widget_p(_firstChild()); }
			inline Widget_p			lastChild() const { return Widget_p(_lastChild()); }

		
			//.____ Appearance ____________________________________________________

			void		setSkin(Skin* pSkin) override;

			//.____ Behavior ______________________________________________________

			void		setUsePickHandles( bool bUseHandles );
			bool		usePickHandles() { return m_bUsePickHandles;}

			//.____ Misc ______________________________________________________

			virtual Widget_p		findWidget( const Coord& ofs, SearchMode mode ) { return Widget_p(_findWidget(Util::ptsToSpx(ofs,m_scale),mode)); }
			bool					contains(const Widget * pWidget) const;

			//.____ Internal __________________________________________________

			virtual Widget* _findWidget(const CoordSPX& ofs, SearchMode mode);

			virtual ModalOverlay* _getModalOverlay() const;
			virtual PopupOverlay* _getPopupOverlay() const;


		protected:
			Container() {};
			template<class BP> Container( const BP& bp ) : Widget(bp) 
			{
				m_bUsePickHandles = bp.usePickHandles;
				m_overflow = m_skin.overflow(m_scale);
			}

			virtual ~Container() {};
		
			// SlotHolder methods

			Container *				_container() override;
			Root *					_root() override;
			virtual int				_scale() const override;									///< Default scale value for all children.

			virtual RectSPX			_slotGeo( const StaticSlot * pSlot ) const override = 0;				///< Get the local position of a child.
			virtual void			_childOverflowChanged( StaticSlot * pSlot, const BorderSPX& oldOverflow, const BorderSPX& newOverflow ) override;

			RectSPX					_childLocalToGlobal(const StaticSlot* pSlot, const RectSPX& rect) const override;
			RectSPX					_globalToChildLocal(const StaticSlot* pSlot, const RectSPX& rect) const override;
			RectSPX					_globalPtsToChildLocalSpx(const StaticSlot* pSlot, const Rect& rect) const override;
			Rect					_childLocalSpxToGlobalPts(const StaticSlot* pSlot, const RectSPX& rect) const override;

			virtual bool			_isChildVisible( const StaticSlot * pSlot ) const override;
			virtual RectSPX			_childWindowSection( const StaticSlot * pSlot ) const override;		// Returns the window section within the childs canvas.

			virtual void			_childRequestRender(StaticSlot * pSlot, const RectSPX& rect) override = 0;
			virtual void			_childRequestResize(StaticSlot * pSlot) override = 0;

			virtual bool			_childRequestFocus( StaticSlot * pSlot, Widget * pWidget, bool bRaiseWindow = false ) override;					// Request focus on behalf of me, child or grandchild.
			virtual bool			_childReleaseFocus( StaticSlot * pSlot, Widget * pWidget ) override;

			virtual void			_childRequestInView( StaticSlot * pSlot ) override;
			virtual void			_childRequestInView( StaticSlot * pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea ) override;

			virtual Widget *		_prevChild(const StaticSlot * pSlot) const override = 0;
			virtual Widget *		_nextChild(const StaticSlot * pSlot) const override = 0;

			virtual void			_releaseChild(StaticSlot * pSlot) override = 0;
			virtual void			_replaceChild(StaticSlot * pSlot, Widget * pNewChild) override = 0;

			virtual void			_didAddSlots(StaticSlot * pSlot, int nb) override;
			virtual void			_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb) override;
			virtual void			_willEraseSlots(StaticSlot * pSlot, int nb) override;

			
		
			//

			virtual Widget *		_firstChild() const = 0;
			virtual Widget *		_lastChild() const = 0;

			virtual void			_setState( State state ) override;

			virtual void			_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) override;
			virtual void			_resize(const SizeSPX& size, int scale) override;

			virtual BorderSPX		_calcOverflow();

			void					_refreshOverflow();
			void					_addChildOverflow( const RectSPX& childGeo, const BorderSPX& childOverflow );
			void 					_removeChildOverflow( const RectSPX& childGeo, const BorderSPX& childOverflow );
			void					_moveChildOverflow(StaticSlot * pSlot, const RectSPX& oldGeo, const RectSPX& newGeo);

			RectSPX					_renderBounds() const override;
			BorderSPX				_overflow() const override;
		
			struct SlotWithGeo
			{
				RectSPX				geo;
				const StaticSlot *	pSlot;
			};

			virtual void			_firstSlotWithGeo( SlotWithGeo& package ) const = 0;
			virtual void			_nextSlotWithGeo( SlotWithGeo& package ) const = 0;

			virtual void			_maskPatches( PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip ) override;

			bool					m_bSiblingsOverlap = true;	// Set if children (might be) overlapping each other (special considerations to be taken during rendering).		
			bool					m_bUsePickHandles = false;
			BorderSPX				m_overflow;

	};


} // namespace wg
#endif //WG_CONTAINER_DOT_H
