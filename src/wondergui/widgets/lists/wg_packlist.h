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
#ifndef WG_PACKLIST_DOT_H
#define WG_PACKLIST_DOT_H
#pragma once

#include <functional>

#include <wg_list.h>
#include <wg_columnheader.h>
#include <wg_dynamicslotvector.h>
#include <wg_util.h>


namespace wg
{

	class PackList;
	typedef	StrongPtr<PackList>		PackList_p;
	typedef	WeakPtr<PackList>		PackList_wp;

	//____ PackList ____________________________________________________________

	/**
	 * @brief	List widget aligning its entries horizontally or vertically.
	 *
	 * PackList provides a standard list, where all contained children are
	 * aligned ("packed") either vertically or horizontally and can be selected
	 * using mouse or keyboard.
	 *
	 */

	class PackList : public List
	{
		friend class Slot;

	public:

		//____ Slot ____________________________________________________________

		class Slot : public List::Slot
		{
			friend class PackList;
			template<class S> friend class SlotVector;
			friend class DynamicSlotVector<Slot>;

		public:

			//.____ Blueprint _____________________________________________________
			
			struct Blueprint
			{
				bool		visible = true;
			};
			
			//.____ Identification ________________________________________________

			const static TypeInfo	TYPEINFO;

			//.____ State __________________________________________________________

				inline void	select() { static_cast<PackList*>(_holder())->_selectSlots(this, 1); }
				inline void	unselect() { static_cast<PackList*>(_holder())->_selectSlots(this, 1); }
				inline void	setSelected(bool bSelected) { if (bSelected) static_cast<PackList*>(_holder())->_selectSlots(this, 1); else static_cast<PackList*>(_holder())->_unselectSlots(this, 1); }
				inline bool isSelected() const { return m_bSelected; }

			//.____ Appearance _________________________________________________

				inline void		hide() { static_cast<PackList*>(_holder())->_hideSlots(this, 1); }
				inline void		unhide() { static_cast<PackList*>(_holder())->_unhideSlots(this, 1); }
				inline void		setVisible(bool bVisible) { if (bVisible) static_cast<PackList*>(_holder())->_unhideSlots(this, 1); else static_cast<PackList*>(_holder())->_hideSlots(this, 1); }
				inline bool		isVisible() const { return m_bVisible; }

			//.____ Operators __________________________________________

			inline void operator=(Widget * pWidget) { setWidget(pWidget); }

		protected:

			Slot(SlotHolder *pHolder) : List::Slot(pHolder) {}

			bool _setBlueprint( const Blueprint& bp );

			Slot(Slot&& o) = default;
			Slot& operator=(Slot&& o) = default;

			spx				m_ofs;				// Offset for start of this list item.
			spx				m_length;			// Length of this list item. Includes widget padding.
			spx				m_prefBreadth;		// Prefereed breadth of this widget.
		};

		using		iterator = DynamicSlotVector<Slot>::iterator;

		//.____ Blueprint _________________________________________
		
		struct Blueprint
		{
			Axis					axis 		= Axis::X;
			Object_p				baggage;
			bool					disabled 	= false;
			bool					dropTarget 	= false;
			Skin_p					entrySkin;
			Skin_p					entrySkin2;
			Finalizer_p				finalizer 	= nullptr;
			ColumnHeader::Blueprint	header;
			int						id 			= 0;
			Skin_p					lassoSkin;
			MarkPolicy				markPolicy 	= MarkPolicy::AlphaTest;
			Size					maxEntrySize= { 1000000,1000000 };
			Size					minEntrySize= {0,0};
			bool					pickable 	= false;
			uint8_t					pickCategory= 0;
			bool					pickHandle = false;
			PointerStyle			pointer 	= PointerStyle::Undefined;
			bool					selectable 	= false;
			Skin_p					skin;
			std::function<int(const Widget *, const Widget *)> sortFunc;
			SortOrder				sortOrder	= SortOrder::Ascending;
			bool					stickyFocus = false;
			bool					tabLock 	= false;
			String					tooltip;
			bool					usePickHandles = false;
		};
		
		//.____ Creation __________________________________________

		static PackList_p	create() { return PackList_p(new PackList()); }
		static PackList_p	create(const Blueprint& blueprint) { return PackList_p(new PackList(blueprint)); }

		//.____ Components _______________________________________

		ColumnHeader			header;
		DynamicSlotVector<Slot>	slots;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;
	
		//.____ Geometry ____________________________________________

		void				setAxis( Axis axis );
		Axis				axis() const { return m_bHorizontal?Axis::X:Axis::Y; }

		bool				setMinEntrySize(Size min);
		bool				setMaxEntrySize(Size max);
		Size				minEntrySize() const { return m_minEntrySize; }
		Size				maxEntrySize() const { return m_maxEntrySize; }

		//.____ Ordering ______________________________________________________

		void				setSortOrder( SortOrder order );
		SortOrder			sortOrder() const { return m_sortOrder; }

		void				setSortFunction( std::function<int(const Widget *, const Widget *)> func );
		std::function<int(const Widget *, const Widget *)> sortFunction() const { return m_sortFunc; }

		bool				sortSlots();

		
		//.____ Misc _____________________________________________________________
		
		iterator			insertSorted(Widget * pWidget);
		
		void				selectSlots(int index, int amount);
		void				selectSlots(iterator beg, iterator end);

		void				unselectSlots(int index, int amount);
		void				unselectSlots(iterator beg, iterator end);

		
		//.____ Internal _________________________________________________________

		SizeSPX				_defaultSize(int scale = -1) const override;
		spx					_matchingHeight(spx width, int scale = -1) const override;
		spx					_matchingWidth(spx height, int scale = -1) const override;


	protected:
		PackList();
		template<class BP> PackList(const BP& bp) : header(this), slots(this), List(bp)
		{
			header._initFromBlueprint(bp.header);
			
			m_sizeOfSlot = sizeof(Slot);
			m_bSiblingsOverlap = false;

			m_contentBreadth = 0;
			m_contentLength = 0;
			m_contentDefaultLength = 0;
			m_contentDefaultBreadth = 0;
			m_nbEntriesOfDefaultBreadth = 0;
			
			m_bHorizontal = (bp.axis == Axis::X);
			m_sortOrder = bp.sortOrder;
			_header()._setSortOrder( m_sortOrder );

			m_sortFunc = bp.sortFunc;

			m_minEntrySize = bp.minEntrySize;
			m_minEntrySizeSPX = Util::align(Util::ptsToSpx(bp.minEntrySize, m_scale));

			m_maxEntrySize = bp.maxEntrySize;
			m_maxEntrySizeSPX = Util::align(Util::ptsToSpx(bp.maxEntrySize, m_scale));
		}
		
		virtual ~PackList();

		// Overloaded from Widget

		void			_maskPatches( PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip ) override;
		void			_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) override;
		void			_resize( const SizeSPX& size, int scale = -1) override;

		void			_receive( Msg * pMsg ) override;
		SizeSPX			_windowPadding() const override;

		// Overloaded from List

		List::Slot *	_findEntry(const CoordSPX& ofs) override;
		void			_getEntryGeo(RectSPX& geo, const List::Slot * pSlot) const override;

		RectSPX			_listArea() const override;
		RectSPX			_listWindow() const override;
		RectSPX			_listCanvas() const override;

		void			_onEntryPaddingChanged(BorderSPX oldPadding, BorderSPX newPadding) override;
		void			_onLassoUpdated(const RectSPX& oldLasso, const RectSPX& newLasso) override;

		List::Slot *	_beginSlots() const override;
		List::Slot *	_endSlots() const override;


		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		void			_firstSlotWithGeo(SlotWithGeo& package) const override;
		void			_nextSlotWithGeo(SlotWithGeo& package) const override;

		Widget * 		_findWidget(const CoordSPX& ofs, SearchMode mode) override;

		RectSPX			_slotGeo(const StaticSlot * pSlot) const override;

		void			_childRequestRender(StaticSlot * pSlot, const RectSPX& rect) override;
		void			_childRequestResize(StaticSlot * pSlot) override;

		Widget *		_prevChild(const StaticSlot * pSlot) const override;
		Widget *		_nextChild(const StaticSlot * pSlot) const override;

		void			_releaseChild(StaticSlot * pSlot) override;
		void			_replaceChild(StaticSlot * pSlot, Widget * pNewChild ) override;

		void			_didAddSlots(StaticSlot * pSlot, int nb) override;
		void			_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb) override;
		void			_willEraseSlots(StaticSlot * pSlot, int nb) override;

		void			_hideSlots(StaticSlot * pSlot, int nb);
		void			_unhideSlots(StaticSlot * pSlot, int nb);

		void			_selectSlots(StaticSlot * pSlot, int nb);
		void			_unselectSlots(StaticSlot * pSlot, int nb);


		// Overloaded from Component::Holder

		CoordSPX		_componentPos(const Component * pComponent) const override;
		SizeSPX			_componentSize(const Component * pComponent) const override;
		RectSPX			_componentGeo(const Component * pComponent) const override;

		void			_receiveComponentNotif(Component * pComponent, ComponentNotif notification, int value, void * pData) override;


		// Internal

		int				_getInsertionPoint(const Widget * pWidget) const;
		bool			_hasSortFunction() const { return m_sortFunc != nullptr; }


		void			_requestRenderChildren(Slot * pBegin, Slot * pEnd);

		void			_updateChildOfsFrom( Slot* pSlot );


		void			_getChildGeo( RectSPX& geo, const Slot * pSlot ) const;
		int				_getEntryAt( spx ofs ) const;
		RectSPX			_headerGeo() const;

		void			_refreshHeader();
		void			_refreshList();

		SizeSPX			_paddedLimitedDefaultSize( StaticSlot * pSlot, int scale );
		spx				_paddedLimitedMatchingHeight( StaticSlot * pSlot, spx paddedWidth );
		spx				_paddedLimitedMatchingWidth( StaticSlot * pSlot, spx paddedHeight );

		SizeSPX			_calcDefaultSize( int scale ) const;
		
		void			_addToContentDefaultSize(spx length, spx breadth);
		void			_subFromContentDefaultSize(spx length, spx breadth);

		class ColumnHeaderAccess : public ColumnHeader { friend class PackList; };
		const ColumnHeaderAccess& _header() const { return reinterpret_cast<const ColumnHeaderAccess&>(header); }
		ColumnHeaderAccess& _header() { return reinterpret_cast<ColumnHeaderAccess&>(header); }


		bool				m_bHorizontal;

		SortOrder			m_sortOrder;
		std::function<int(const Widget *,const Widget *)> m_sortFunc;

		spx					m_contentBreadth;
		spx					m_contentLength;

		SizeSPX				m_minEntrySizeSPX = { 0,0 };
		SizeSPX				m_maxEntrySizeSPX = { spx_max, spx_max };


		Size				m_minEntrySize = { 0,0 };
		Size				m_maxEntrySize = { 1000000,1000000 };

		//----

		spx				m_contentDefaultLength;
		spx				m_contentDefaultBreadth;
		int				m_nbEntriesOfDefaultBreadth;			// Number of entries whose default breadth are the same as m_defaultSize.
	};



} // namespace wg
#endif //WG_PACKLIST_DOT_H
