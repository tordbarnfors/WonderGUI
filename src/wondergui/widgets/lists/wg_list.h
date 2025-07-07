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

#ifndef	WG_LIST_DOT_H
#define	WG_LIST_DOT_H
#pragma once

#include <wg_widget.h>
#include <wg_container.h>
#include <wg_skin.h>
#include <wg_slot.h>

namespace wg
{

	class List;
	typedef	StrongPtr<List>		List_p;
	typedef	WeakPtr<List>		List_wp;


	//____ List _________________________________________________________________

	/**
	* @brief	Base class for widgets providing lists of selectable entries.
	*
	* List is the base class for widgets that provide lists of selectable entries.
	*
	*/

	class List : public Container
	{
	public:

		//____ Slot ____________________________________________________________

		class Slot : public DynamicSlot
		{
			friend class List;
		public:

			//.____ Identification ________________________________________________

			const static TypeInfo	TYPEINFO;

		protected:
			Slot(SlotHolder * pHolder) : DynamicSlot(pHolder) {}

			bool		m_bVisible = false;
			bool		m_bSelected = false;
		};

		//.____ Blueprint _________________________________________
		
		struct Blueprint
		{
			Object_p				baggage;
			bool					disabled 	= false;
			bool					dropTarget 	= false;
			Skin_p					entrySkin;
			Skin_p					entrySkin2;
			Finalizer_p				finalizer 	= nullptr;
			int						id 			= 0;
			Skin_p					lassoSkin;
			MarkPolicy				markPolicy 	= MarkPolicy::AlphaTest;
			bool					pickable 	= false;
			uint8_t					pickCategory= 0;
			PointerStyle			pointer 	= PointerStyle::Undefined;
			bool					selectable 	= false;
			SelectMode				selectMode 	= SelectMode::SingleEntry;
			Skin_p					skin;
			bool					tabLock 	= false;
			String					tooltip;
		};

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance _________________________________________________

		void				setLassoSkin(Skin* pSkin);
		Skin_p				lassoSkin() const { return m_lassoSkin.get(); }

		virtual bool		setEntrySkin( Skin * pSkin );
		virtual bool		setEntrySkin( Skin * pOddEntrySkin, Skin * pEvenEntrySkin );
		Skin_p				oddEntrySkin() const { return m_pEntrySkin[0]; }
		Skin_p				evenEntrySkin() const { return m_pEntrySkin[1]; }

		//.____ Behavior ________________________________________________________

		virtual bool		setSelectMode( SelectMode mode );
		SelectMode			selectMode() const { return m_selectMode; }


	protected:

		List();
		template<class BP> List(const BP& bp) : m_lassoSkin(this), m_selectMode(SelectMode::SingleEntry), Container(bp)
		{
			m_lassoSkin.set(bp.lassoSkin);

			if (bp.entrySkin)
			{
				m_pEntrySkin[0] = bp.entrySkin;

				if (bp.entrySkin && bp.entrySkin2)
					m_pEntrySkin[1] = bp.entrySkin2;
				else
					m_pEntrySkin[1] = bp.entrySkin;

				m_entryPadding = bp.entrySkin->_contentBorder(m_scale, State::Default);
				m_maxEntryOverflow = bp.entrySkin->_geoOverflow(m_scale);

				if (!m_maxEntryOverflow.isEmpty())
				{
					m_overflow.growToContain(m_maxEntryOverflow - m_skin.contentBorder(m_scale, State::Default));
					m_bOverflow = !m_overflow.isEmpty();
				}
			}
		}
		
		virtual ~List();

		virtual void	_receive( Msg * pMsg ) override;

		int				_selectSlot(Slot * pSlot, bool bPostMsg) { return _setSlotSelection(pSlot, _nextSlot(pSlot), true, bPostMsg); }
		int				_selectSlots(Slot * pBegin, Slot * pEnd, bool bPostMsg) { return _setSlotSelection(pBegin, pEnd, true, bPostMsg); }
		int				_unselectSlots(Slot * pBegin, Slot * pEnd, bool bPostMsg) { return _setSlotSelection(pBegin, pEnd, false, bPostMsg); }

		Widget*			_firstChild() const override;
		Widget*			_lastChild() const override;


		virtual int		_setSlotSelection(Slot * pBegin, Slot * pEnd, bool bSelect, bool bPostMsg);

		virtual int		_flipSelection( Slot * pBegin, Slot * pEnd, bool bPostMsg );

		virtual Slot * _findEntry( const CoordSPX& ofs ) = 0;
		virtual void	_getEntryGeo( RectSPX& geo, const Slot * pSlot ) const = 0;

		virtual RectSPX	_listArea() const = 0;										// Area for the entries (contentRect minus header).
		virtual RectSPX	_listWindow() const = 0;
		virtual RectSPX	_listCanvas() const = 0;

		virtual void	_onEntryPaddingChanged( BorderSPX oldPadding, BorderSPX newPadding ) = 0;
		virtual void	_onLassoUpdated( const RectSPX& oldLasso, const RectSPX& newLasso ) = 0;

		State			_skinState(const SkinSlot* pSlot) const override;
		SizeSPX			_skinSize(const SkinSlot* pSlot) const override;

		void			_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect) override;

		BorderSPX		_calcOverflow() override;
		void			_addEntryOverflow( const BorderSPX& entryOverflow );


		virtual Slot * _beginSlots() const = 0;
		virtual Slot * _endSlots() const = 0;

		inline Slot * _nextSlot( Slot * pSlot ) const { return (Slot*) (((char*)pSlot)+m_sizeOfSlot); }
		inline const Slot * _nextSlot( const Slot * pSlot, int sizeOf ) const { return (const Slot*) (((char*)pSlot)+m_sizeOfSlot); }

		inline Slot * _prevSlot( Slot * pSlot ) const { return (Slot*) (((char*)pSlot)-m_sizeOfSlot); }
		inline const Slot * _prevSlot( const Slot * pSlot ) const { return (const Slot*) (((char*)pSlot)-m_sizeOfSlot); }

		SkinSlot		m_lassoSkin;

		int				m_sizeOfSlot;

		SelectMode		m_selectMode;
		Skin_p			m_pEntrySkin[2];

		CoordSPX		m_lassoBegin;
		CoordSPX		m_lassoEnd;

		Widget_wp		m_pFocusedChild;
		Widget_wp		m_pHoveredChild;

		BorderSPX		m_entryPadding;				// Content padding of entry.
		BorderSPX		m_maxEntryOverflow;			// Max overflow in each direction for the entries, 
													// includes the two entry-skins and overflow of entry widgets.

	};




} // namespace wg
#endif //WG_LIST_DOT_H
