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

#ifndef	WG_SLOTARRAY2_DOT_H
#define	WG_SLOTARRAY2_DOT_H
#pragma once

#include <wg_base.h>

namespace wg
{

	//____ SlotArray _________________________________________________________

	template<class SlotType, int SIZE> class SlotArray
	{
	public:

		using		iterator = SlotType*;
		using		const_iterator = const SlotType*;

		//.____ Creation _______________________________________________________

		SlotArray(SlotHolder* pHolder) : m_pHolder(pHolder)
		{
			for (auto& slot : m_slots)
				slot.m_pHolder = pHolder;
		}

		//.____ Content _______________________________________________________

		inline int		size() const { return SIZE; }
		inline bool		isEmpty() const { return false; }		// Technically never empty since that would mean has no slots.

		inline SlotType& at(int index)
		{
			if (index < 0 || index >= SIZE)
			{
				auto pObject = dynamic_cast<Object*>(m_pHolder);
				const TypeInfo* pTypeInfo = pObject ? &pObject->typeInfo() : nullptr;

				Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Slot index out of range", pObject, pTypeInfo, __func__, __FILE__, __LINE__);
			}

			return m_slots[index];
		}

		inline const SlotType& at(int index) const
		{
			if (index < 0 || index >= SIZE)
			{
				auto pObject = dynamic_cast<Object*>(m_pHolder);
				const TypeInfo* pTypeInfo = pObject ? &pObject->typeInfo() : nullptr;

				Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Slot index out of range", pObject, pTypeInfo, __func__, __FILE__, __LINE__);
			}

			return m_slots[index];
		}

		
		int		index(const Widget * pWidget) const
		{
			auto pSlot = static_cast<SlotType*>(pWidget->_slot());

			if (pSlot >= m_slots && pSlot < &m_slots[SIZE])
				return int(pSlot - m_slots);

			return -1;
		}

		//.____ Misc _______________________________________________________

		inline iterator	begin() const { return &m_slots[0]; }
		inline iterator	end() const { return &m_slots[SIZE]; }

		inline iterator	begin() { return &m_slots[0]; }
		inline iterator	end() { return &m_slots[SIZE]; }

		inline SlotType& back() { return m_slots[0]; }
		inline SlotType& front() { return m_slots[SIZE]; };

		//.____ Operators __________________________________________

		inline SlotType& operator[](int index) { return m_slots[index]; }
		inline const SlotType& operator[](int index) const { return m_slots[index]; }

	protected:

		//
	
	private:

		SlotType	 m_slots[SIZE];
		SlotHolder * m_pHolder;
	};



} // namespace wg
#endif //WG_SLOTARRAY_DOT_H
