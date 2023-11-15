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

#ifndef	WG_DYNAMICSLOTVECTOR_DOT_H
#define	WG_DYNAMICSLOTVECTOR_DOT_H
#pragma once

#include <wg_base.h>

namespace wg
{




	//____ DynamicSlotVector _________________________________________________________

	template<class SlotType> 
	class DynamicSlotVector
	{
	public:

		using		iterator = SlotType*;
		using		const_iterator = const SlotType*;

		DynamicSlotVector(SlotHolder * pHolder) : m_pHolder(pHolder) {}
		~DynamicSlotVector() { _killBlock(_begin(), _end()); free(m_pBuffer); }


		class ChildWithBP
		{
			friend class DynamicSlotVector<SlotType>;

		public:
			template<typename T>
			constexpr ChildWithBP(const T pChild) : m_pChild(pChild), m_pBP(nullptr)
			{
			}

			ChildWithBP(Widget* pChild, const struct SlotType::Blueprint& bp) : m_pChild(pChild), m_pBP(&bp)
			{
			}

		protected:
			Widget* m_pChild;
			const struct SlotType::Blueprint* m_pBP;
		};

		
		//.____ Content _______________________________________________________

		inline int		size() const { return m_size; }
		inline bool		isEmpty() const { return (m_size == 0); }

		inline SlotType& at(int index) const
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

			if (pSlot >= m_pArray && pSlot < m_pArray + m_size )
				return int(pSlot - m_pArray);

			return -1;
		}

		inline iterator	find(const Widget * pWidget) const
		{
			auto pSlot = static_cast<SlotType*>(pWidget->_slot());

			if (pSlot >= m_pArray && pSlot < m_pArray + m_size )
				return iterator(pSlot);

			return iterator(_end());
		}

		inline int		frontCapacity() const { return (int) (m_pArray - m_pBuffer); }
		inline int		backCapacity() const { return int((m_pBuffer + m_capacity) - (m_pArray + m_size)); }

		inline void		reserveFront(int n) { if (m_pBuffer + n > m_pArray) _reallocArray(m_size + n, n); }
		inline void		reserveBack(int n) { if (m_pArray + m_size + n > m_pBuffer + m_capacity) _reallocArray(m_size + n, 0); }

		iterator		pushFront(const Widget_p& pWidget);
		iterator		pushFront(const Widget_p& pWidget, const struct SlotType::Blueprint& blueprint );
		iterator		pushFront( const std::initializer_list<ChildWithBP>& entries );

		template<typename Iterator>
		iterator		pushFront(Iterator beg, Iterator end);
		
		iterator		pushBack( Widget * pWidget);
		iterator		pushBack( Widget * pWidget, const struct SlotType::Blueprint& blueprint);
		iterator		pushBack( const std::initializer_list<ChildWithBP>& entries );
		
		template<typename Iterator>
		iterator		pushBack(Iterator beg, Iterator end);


		iterator		insert(int index, const Widget_p& pWidget);
		iterator		insert(iterator it, const Widget_p& pWidget);

		iterator		insert(int index, const Widget_p& pWidget, const struct SlotType::Blueprint& blueprint);
		iterator		insert(iterator it, const Widget_p& pWidget, const struct SlotType::Blueprint& blueprint);
		
		iterator		insert( int index, const std::initializer_list<ChildWithBP>& entries );
		iterator		insert( iterator it, const std::initializer_list<ChildWithBP>& entries );

		template<typename Iterator>
		iterator		insert(int index, Iterator beg, Iterator end);

		template<typename Iterator>
		iterator		insert(iterator it, Iterator beg, Iterator end);
		
		void			erase(int index);
		iterator		erase(iterator pos);
		void			erase(int index, int amount);
		iterator		erase(iterator beg, iterator end);

		void			clear();

		//.____ Ordering ______________________________________________________

		void			moveToFront(int index);
		iterator		moveToFront(iterator it);

		void			moveToBack(int index);
		iterator		moveToBack(iterator it);

		void			moveBefore(int index, int sibling);
		iterator		moveBefore(iterator it, iterator sibling);


		//.____ Misc _______________________________________________________

		inline iterator	begin() const { return iterator(_begin()); }
		inline iterator	end() const { return iterator(_end()); }

		inline SlotType&	front() const { return * m_pArray; }
		inline SlotType&	back() const { return  m_pArray[m_size - 1]; }

		
		//.____ Operators __________________________________________

		inline DynamicSlotVector<SlotType>& operator<<(const Widget_p& pWidget) { pushBack(pWidget); return *this; }

		inline SlotType& operator[](int index) { return m_pArray[index]; }
		inline const SlotType& operator[](int index) const { return m_pArray[index]; }

		//.____ Internal ____________________________________________
		
		void	_releaseGuardPointer(Widget * pToRelease, SlotType ** pPointerToGuard);
		void	_reorder(int order[]);

		
	protected:


		inline SlotHolder *		_holder() { return m_pHolder; }
		inline const SlotHolder *	_holder() const { return m_pHolder; }

		SlotType*		_pushFrontEmpty()
		{
			if (m_pArray == m_pBuffer)
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

		SlotType*		_pushBackEmpty() { if (m_pBuffer == nullptr || (m_pArray + m_size == m_pBuffer + m_capacity)) _reallocArray(((m_capacity + 1) * 2), 0); _initBlock(_end()); return &m_pArray[m_size++]; }
		SlotType*		_pushBackEmpty(int entries) { if (m_pBuffer == nullptr || (m_pArray + m_size + entries > m_pBuffer + m_capacity)) _reallocArray(m_size + entries, 0); _initBlock(_end(), _end() + entries); int ofs = m_size; m_size += entries; return &m_pArray[ofs]; }

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


	//____ pushFront() ___________________________________________________________

	template < class SlotType>
	template<typename Iterator>
	typename DynamicSlotVector<SlotType>::iterator	DynamicSlotVector<SlotType>::pushFront(Iterator beg, Iterator end)
	{
		static_assert(std::is_convertible<typename std::iterator_traits<Iterator>::iterator_category, std::input_iterator_tag>::value &&
			std::is_convertible<typename std::iterator_traits<Iterator>::value_type, Widget*>::value,
			"Begin and end parameters must be iterators or raw pointers to some kind of Widget pointers.");

		int nbElements = std::distance(beg,end);

		SlotType * pInsertionPoint = _pushFrontEmpty(nbElements);

		SlotType * pSlot = pInsertionPoint;
		while( beg != end)
		{
			pSlot->_setWidget(*beg);
			pSlot++;
			beg++;
		}

		m_pHolder->_didAddSlots(pInsertionPoint, nbElements);
		return iterator(pInsertionPoint);
	}

	//____ pushBack() ___________________________________________________________

	template < class SlotType>
	template<typename Iterator>
	typename DynamicSlotVector<SlotType>::iterator	DynamicSlotVector<SlotType>::pushBack(Iterator beg, Iterator end)
	{
		static_assert(std::is_convertible<typename std::iterator_traits<Iterator>::iterator_category, std::input_iterator_tag>::value &&
			std::is_convertible<typename std::iterator_traits<Iterator>::value_type, Widget*>::value,
			"Begin and end parameters must be iterators or raw pointers to some kind of Widget pointers.");

		int nbElements = (int) std::distance(beg,end);

		SlotType * pInsertionPoint = _pushBackEmpty(nbElements);

		SlotType * pSlot = pInsertionPoint;
		while( beg != end)
		{
			pSlot->_setWidget(*beg);
			pSlot++;
			beg++;
		}

		m_pHolder->_didAddSlots(pInsertionPoint, nbElements);
		return iterator(pInsertionPoint);
	}

	//____ insert() ______________________________________________________________

	template < class SlotType>
	template<typename Iterator>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::insert(int index, Iterator beg, Iterator end)
	{
		static_assert(std::is_convertible<typename std::iterator_traits<Iterator>::iterator_category, std::input_iterator_tag>::value &&
			std::is_convertible<typename std::iterator_traits<Iterator>::value_type, Widget*>::value,
			"Begin and end parameters must be iterators or raw pointers to some kind of Widget pointers.");

		int nbElements = std::distance(beg,end);

		SlotType * pInsertionPoint = _insertEmpty(index, nbElements);

		SlotType * pSlot = pInsertionPoint;
		while( beg != end)
		{
			pSlot->_setWidget(*beg);
			pSlot++;
			beg++;
		}

		m_pHolder->_didAddSlots(pInsertionPoint, nbElements);
		return iterator(pInsertionPoint);
	}

	template < class SlotType>
	template<typename Iterator>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::insert(iterator it, Iterator beg, Iterator end)
	{
		static_assert(std::is_convertible<typename std::iterator_traits<Iterator>::iterator_category, std::input_iterator_tag>::value &&
			std::is_convertible<typename std::iterator_traits<Iterator>::value_type, Widget*>::value,
			"Begin and end parameters must be iterators or raw pointers to some kind of Widget pointers.");

		int nbElements = std::distance(beg,end);

		SlotType * pInsertionPoint = _insertEmpty(it, nbElements);

		SlotType * pSlot = pInsertionPoint;
		while( beg != end)
		{
			pSlot->_setWidget(*beg);
			pSlot++;
			beg++;
		}

		m_pHolder->_didAddSlots(pInsertionPoint, nbElements);
		return iterator(pInsertionPoint);
	}


} // namespace wg
#endif //WG_DYNAMICSLOTVECTOR_DOT_H
