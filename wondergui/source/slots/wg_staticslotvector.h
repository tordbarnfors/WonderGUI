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
#ifndef	WG_STATICSLOTVECTOR_DOT_H
#define	WG_STATICSLOTVECTOR_DOT_H
#pragma once

#include <wg_base.h>

namespace wg
{

	//____ StaticSlotVector _________________________________________________________

	template<class SlotType> class StaticSlotVector
	{
	public:

		using		iterator = SlotType*;
		using		const_iterator = const SlotType*;

		//.____ Content _______________________________________________________

		inline int		size() const { return m_size; }
		inline bool		isEmpty() const { return (m_size == 0); }

		inline SlotType& at(int index)
		{
			if (index < 0 || index >= m_size)
			{
				auto pObject = dynamic_cast<Object*>(m_pHolder);
				const TypeInfo* pTypeInfo = pObject ? &pObject->typeInfo() : nullptr;
				Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Slot index out of range", pObject, pTypeInfo, __func__, __FILE__, __LINE__);
			}

			return m_pArray[index];
		}

		inline const SlotType& at(int index) const
		{
			if (index < 0 || index >= m_size)
			{
				auto pObject = dynamic_cast<Object*>(m_pHolder);
				const TypeInfo* pTypeInfo = pObject ? &pObject->typeInfo() : nullptr;
				Base::throwError(ErrorLevel::Error, ErrorCode::OutOfRange, "Slot index out of range", pObject, pTypeInfo, __func__, __FILE__, __LINE__);
			}

			return m_pArray[index];
		}

		
		inline int		index(const Widget * pWidget) const
		{
			auto pSlot = static_cast<SlotType*>(pWidget->_slot());

			if (pSlot >= m_pArray && pSlot < m_pArray + m_size)
				return int(pSlot - m_pArray);

			return -1;
		}

		//.____ Misc _______________________________________________________

		inline iterator	begin() const { return iterator(_begin()); }
		inline iterator	end() const { return iterator(_end()); }

		//.____ Operators __________________________________________

		inline SlotType& operator[](int index) { return m_pArray[index]; }
		inline const SlotType& operator[](int index) const { return m_pArray[index]; }

	protected:
		StaticSlotVector(SlotHolder * pHolder) : m_pHolder(pHolder) {}
		~StaticSlotVector() { _killBlock(_begin(), _end()); free(m_pBuffer); }

		int		_releaseUpdateIndex(Widget* pToRelease, int index);

	//////
		inline SlotHolder *			_holder() { return m_pHolder; }
		inline const SlotHolder *	_holder() const { return m_pHolder; }

		SlotType*		_pushFrontEmpty()
						{
							if (m_pArray == m_pBuffer )
							{
								int capacity = (m_capacity + 1) * 2;
								_reallocArray(capacity, capacity - m_size);
							}
							m_size++;
							m_pArray--;
							_initBlock(m_pArray);
							return m_pArray;
						}

		SlotType*		_pushFrontEmpty(int entries)
						{
							if (m_pBuffer + entries > m_pArray)
							{
								_reallocArray(m_size + entries, entries);
							}

							m_size += entries;
							m_pArray -= entries;
							_initBlock(m_pArray, m_pArray + entries);
							return m_pArray;
						}

		SlotType*		_pushBackEmpty() { if (m_pArray + m_size == m_pBuffer + m_capacity) _reallocArray(((m_capacity + 1) * 2), 0); _initBlock(_end()); return &m_pArray[m_size++]; }
		SlotType*		_pushBackEmpty(int entries) { if (m_pArray + m_size + entries > m_pBuffer + m_capacity) _reallocArray(m_size + entries, 0); _initBlock(_end(), _end() + entries); int ofs = m_size; m_size += entries; return &m_pArray[ofs]; }

		SlotType*		_insertEmpty(int index) { return _insertBlock(&m_pArray[index], 1); }
		SlotType*		_insertEmpty(int index, int entries) { return _insertBlock(&m_pArray[index], entries); }
		SlotType*		_insertEmpty(SlotType * pPos) { return _insertBlock(pPos, 1); }
		SlotType*		_insertEmpty(SlotType * pPos, int entries) { return _insertBlock(pPos, entries); }

		SlotType*		_erase(int index) { return _deleteBlock(&m_pArray[index], &m_pArray[index + 1]); }
		SlotType*		_erase(int index, int entries) { return _deleteBlock(&m_pArray[index], &m_pArray[index + entries]); }
		SlotType*		_erase(SlotType * pPos) { return _deleteBlock(pPos, pPos + 1); }
		SlotType*		_erase(SlotType * pPos, SlotType * pEnd) { return _deleteBlock(pPos, pEnd); }

		void			_move(int index, int newIndex) { _move(&m_pArray[index], &m_pArray[newIndex]); }
		void			_move(SlotType * pFrom, SlotType * pTo);

		bool			_contains(const SlotType * pSlot) const { return (pSlot >= m_pArray && pSlot <= _last()); }

		void			_clear() { _killBlock(_begin(), _end()); free(m_pBuffer); m_pBuffer = nullptr;  m_pArray = nullptr; m_capacity = 0; m_size = 0; }


		SlotType*	_slot(int index) const { return &m_pArray[index]; }

		int			_index(const SlotType* pSlot) const { return int(pSlot - m_pArray); }

		SlotType *	_find(const Widget* pWidget) const;

		SlotType*	_first() const { return m_pArray; }
		SlotType*	_last() const { return m_pArray + (m_size - 1); }

		SlotType*	_begin() const { return m_pArray; }
		SlotType*	_end() const { return m_pArray + m_size; }

		void		_reorder(int order[]);

	private:

		void	_reallocArray(int capacity, int offset);

		void	_reallocBlock(SlotType * pBeg, SlotType * pEnd);

		SlotType* _deleteBlock(SlotType * pBeg, SlotType * pEnd);

		SlotType*	_insertBlock(SlotType * pPos, int entries);

		inline void	_killBlock(SlotType * pBlock)
		{
			pBlock->~SlotType();
		}

		void	_killBlock(SlotType * pBeg, SlotType * pEnd);

		inline void	_initBlock(SlotType * pBlock)
		{
			new (pBlock) SlotType(m_pHolder);
		}

		void	_initBlock(SlotType * pBeg, SlotType * pEnd);

		SlotType *	m_pBuffer = nullptr;
		int			m_capacity = 0;

		SlotType *	m_pArray = nullptr;
		int			m_size = 0;

		SlotHolder * m_pHolder;
	};

} // namespace wg
#endif //WG_STATICSLOTVECTOR_DOT_H
