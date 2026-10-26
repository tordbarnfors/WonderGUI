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
#ifndef	WG_SLOTTABLE_DOT_H
#define	WG_SLOTTABLE_DOT_H
#pragma once

#include <wg_base.h>
#include <wg_slot.h>

namespace wg
{




	//____ SlotTable _________________________________________________________

	class SlotTable
	{
	public:

		class Holder
		{
			friend class SlotTable;

			virtual void	_refreshSlots(int ofs, Axis axis, int nSlots) = 0;
			virtual Object*	_object() = 0;
		};

		class RowAccess
		{
			friend class SlotTable;
			
		public:
			inline DynamicSlot& operator[](int index) { return m_pSlot[index]; }
			inline const DynamicSlot&	operator[](int index) const { return m_pSlot[index]; }

		private:
			RowAccess(DynamicSlot * pSlot ) : m_pSlot(pSlot) {}
			
			DynamicSlot * m_pSlot;
		};
		
		class ConstRowAccess
		{
			friend class SlotTable;
		public:
			inline const DynamicSlot& operator[](int index) const { return m_pSlot[index]; }

		private:
			ConstRowAccess(const DynamicSlot* pSlot) : m_pSlot(pSlot) {}

			const DynamicSlot* m_pSlot;
		};



		using		iterator = std::vector<DynamicSlot>::iterator; // DynamicSlot*;
		using		const_iterator = std::vector<DynamicSlot>::const_iterator; // const DynamicSlot*;

		SlotTable(SlotHolder * pHolder, Holder * pTableHolder ) : m_pHolder(pHolder), m_pTableHolder(pTableHolder), m_dummySlot(pHolder) {}
		~SlotTable() {  }

		DynamicSlot*		data() { return m_slots.data(); }
		const DynamicSlot*	data() const { return m_slots.data(); }

		iterator			begin() { return m_slots.begin(); }
		const_iterator		begin() const { return m_slots.begin(); }

		iterator			end() { return m_slots.end(); }
		const_iterator		end() const { return m_slots.end(); }

		int					rows() const { return m_nRows; }
		int					columns() const { return m_nColumns; }
		int					slots() const { return (int) m_slots.size(); }

		DynamicSlot&		at(int row, int column);
		const DynamicSlot&	at(int row, int column) const;
				
		int					replaceRow(int index, const std::initializer_list<Widget_p>& widgets);
		int					replaceColumn(int index, const std::initializer_list<Widget_p>& widgets);

		void				clearRows(int index, int nb = 1 );
		void				clearColumns(int index, int nb = 1 );

		int					replaceWidgets(iterator it, Axis axis, const std::initializer_list<Widget_p>& widgets);
		void				clearWidgets(iterator it, Axis axis, int nb );

		//.____ Internal ______________________________________________________

		void				_resize(int nbRows, int nbColumns);
		void				_reserve(int rows, int columns);
		void				_shrinkToFit();

		void				_insertRows(int ofs, int nb);
//		void				_moveRows(int from, int to, int nb);
		void				_deleteRows(int ofs, int nb);

		void				_insertColumns(int ofs, int nb);
//		void				_moveColumns(int from, int to, int nb);
		void				_deleteColumns(int ofs, int nb);

		void				_slotsUpdated(int start, Axis axis, int nb);
		
		//.____ Operators __________________________________________

//		inline SlotTable<DynamicSlot>& operator<<(const Widget_p& pWidget) { pushBack(pWidget); return *this; }

		inline RowAccess 		operator[](int index) { return RowAccess(&m_slots[index*m_nColumns]); }
		inline const ConstRowAccess	operator[](int index) const { return ConstRowAccess(&m_slots[index*m_nColumns]); }

	protected:
		
		inline SlotHolder *			_holder() { return m_pHolder; }
		inline const SlotHolder *	_holder() const { return m_pHolder; }

		void		_clearRows( int startIndex, int nb );
		void		_clearColumns( int startIndex, int nb );
	
	private:

		int		m_nRows = 0;
		int		m_nColumns = 0;
		int		m_capacity = 0;
		
		DynamicSlot		m_dummySlot;
		
		std::vector<DynamicSlot>	m_slots;

		Holder *					m_pTableHolder;
		SlotHolder * 				m_pHolder;
	};


} // namespace wg
#endif //WG_SLOTTABLE_DOT_H
