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

#ifndef	WG_DYNAMICSLOTVECTOR_IMPL_DOT_H
#define	WG_DYNAMICSLOTVECTOR_IMPL_DOT_H
#pragma once

#include <wg_dynamicslotvector.h>
#include <cstring>

//#define INSTANTIATE_CHILDREN(SlotType)		template class CSlotVector< SlotType >;

namespace wg
{
	class Widget;

	//____ pushFront() _________________________________________________________________

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::pushFront(const Widget_p& pWidget)
	{
		if( !pWidget )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Pushing a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
			return end();
		}
		
		pWidget->releaseFromParent();								// Always release first, in case widget already was in our array.

		SlotType * pSlot = _pushFrontEmpty();
		pSlot->_setWidget(pWidget);
		m_pHolder->_didAddSlots(pSlot, 1);
		return iterator(pSlot);
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::pushFront(const Widget_p& pWidget, const struct SlotType::Blueprint& blueprint)
	{
		if( !pWidget )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Pushing a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
			return end();
		}

		pWidget->releaseFromParent();								// Always release first, in case widget already was in our array.

		SlotType * pSlot = _pushFrontEmpty();
		pSlot->_setWidget(pWidget);
		pSlot->_setBlueprint(blueprint);
		m_pHolder->_didAddSlots(pSlot, 1);
		return iterator(pSlot);
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::pushFront(const std::initializer_list<ChildWithBP>& entries)
	{
		for (auto& entry : entries)
		{
			if( !entry.m_pChild )
			{
				Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Pushing a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
				return end();
			}
		}
		
		for (auto& entry : entries)
			entry.m_pChild->releaseFromParent();

		SlotType * pInsertionPoint = _pushFrontEmpty( (int) entries.size());

		SlotType * pSlot = pInsertionPoint;
		for (auto& entry : entries)
		{
			pSlot->_setWidget(entry.m_pChild);
			if( entry.m_pBP	)
				pSlot->_setBlueprint(*entry.m_pBP);
			pSlot++;
		}

		m_pHolder->_didAddSlots(pInsertionPoint, (int) entries.size());
		return iterator(pInsertionPoint);
	}

	//____ pushBack() _________________________________________________________________

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::pushBack(Widget * pWidget)
	{
		if( !pWidget )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Pushing a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
			return end();
		}

		pWidget->releaseFromParent();								// Always release first, in case widget already was in our array.

		SlotType* pSlot = _pushBackEmpty();
		pSlot->_setWidget(pWidget);
		m_pHolder->_didAddSlots(pSlot, 1);
		return iterator(pSlot);
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::pushBack(Widget * pWidget, const struct SlotType::Blueprint& blueprint)
	{
		if( !pWidget )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Pushing a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
			return end();
		}

		pWidget->releaseFromParent();								// Always release first, in case widget already was in our array.

		SlotType* pSlot = _pushBackEmpty();
		pSlot->_setWidget(pWidget);
		pSlot->_setBlueprint(blueprint);
		m_pHolder->_didAddSlots(pSlot, 1);
		return iterator(pSlot);
	}

	
	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::pushBack(const std::initializer_list<ChildWithBP>& entries)
	{
		for (auto& entry : entries)
		{
			if( !entry.m_pChild )
			{
				Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Pushing a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
				return end();
			}
		}

		for (auto& entry : entries)
			entry.m_pChild->releaseFromParent();

		SlotType * pInsertionPoint = _pushBackEmpty( (int) entries.size());

		SlotType * pSlot = pInsertionPoint;
		for (auto& entry : entries)
		{
			pSlot->_setWidget(entry.m_pChild);
			if( entry.m_pBP	)
				pSlot->_setBlueprint(*entry.m_pBP);
			pSlot++;
		}

		m_pHolder->_didAddSlots(pInsertionPoint, (int) entries.size());
		return iterator(pInsertionPoint);
	}


	//____ insert() ______________________________________________________________

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::insert(int index, const Widget_p& pWidget)
	{
		//TODO: Check insertion point!
		
		if( !pWidget )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Inserting a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
			return end();
		}		

		index = this->_releaseUpdateIndex(pWidget, index);

		SlotType* pSlot = _insertEmpty(index);
		pSlot->_setWidget(pWidget);
		m_pHolder->_didAddSlots(pSlot, 1);
		return iterator(pSlot);
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::insert(iterator pos, const Widget_p& pWidget)
	{
		//TODO: Check insertion point!

		if( !pWidget )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Inserting a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
			return end();
		}
		
		int index = this->_releaseUpdateIndex(pWidget, int(pos - _first()));

		SlotType* pSlot = _insertEmpty(index);
		pSlot->_setWidget(pWidget);
		m_pHolder->_didAddSlots(pSlot, 1);
		return iterator(pSlot);
	}


	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::insert(int index, const Widget_p& pWidget, const struct SlotType::Blueprint& blueprint )
	{
		//TODO: Check insertion point!

		if( !pWidget )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Inserting a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
			return end();
		}

		index = this->_releaseUpdateIndex(pWidget, index);

		SlotType * pSlot = _insertEmpty(index);
		pSlot->_setWidget(pWidget);
		pSlot->_setBlueprint(blueprint);
		m_pHolder->_didAddSlots(pSlot, 1);
		return iterator(pSlot);
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::insert(iterator pos, const Widget_p& pWidget, const struct SlotType::Blueprint& blueprint)
	{
		//TODO: Check insertion point!

		if( !pWidget )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Inserting a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
			return end();
		}

		int index = this->_releaseUpdateIndex(pWidget, int(pos - _first()));

		SlotType * pSlot = _insertEmpty(index);
		pSlot->_setWidget(pWidget);
		pSlot->_setBlueprint(blueprint);
		m_pHolder->_didAddSlots(pSlot, 1);
		return iterator(pSlot);
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::insert( int index, const std::initializer_list<ChildWithBP>& entries )
	{
		//TODO: Check insertion point!

		for (auto& entry : entries)
		{
			if( !entry.m_pChild )
			{
				Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Inserting a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
				return end();
			}
		}

		for (auto& entry : entries)
			index = this->_releaseUpdateIndex(entry.m_pChild, index);

		SlotType* pInsertionPoint = _insertEmpty(index, (int)entries.size());

		SlotType* pSlot = pInsertionPoint;
		for (auto& entry : entries)
		{
			pSlot->_setWidget(entry.m_pChild);

			if( entry.m_pBP)
				pSlot->_setBlueprint(*entry.m_pBP);
			pSlot++;
		}

		m_pHolder->_didAddSlots(pInsertionPoint, (int) entries.size());
		return iterator(pInsertionPoint);
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::insert( iterator pos, const std::initializer_list<ChildWithBP>& entries )
	{
		//TODO: Check insertion point!

		for (auto& entry : entries)
		{
			if( !entry.m_pChild )
			{
				Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Inserting a nullptr into a SlotVector is not allowed.", m_pHolder->_container(), nullptr, __func__, __FILE__, __LINE__ );
				return end();
			}
		}

		int index = int(pos - _first());

		for (auto& entry : entries)
			index = this->_releaseUpdateIndex(entry.m_pChild, index);

		SlotType* pInsertionPoint = _insertEmpty(index, (int)entries.size());

		SlotType* pSlot = pInsertionPoint;
		for (auto& entry : entries)
		{
			pSlot->_setWidget(entry.m_pChild);

			if (entry.m_pBP)
				pSlot->_setBlueprint(*entry.m_pBP);
			pSlot++;
		}

		m_pHolder->_didAddSlots(pInsertionPoint, (int)entries.size());
		return iterator(pInsertionPoint);
	}

	//____ erase() ______________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::erase(int index)
	{
		//TODO: Add assert

		SlotType * pSlot = _slot(index);
		m_pHolder->_willEraseSlots(pSlot, 1);
		_erase(pSlot);
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::erase(iterator pos)
	{
		//TODO: Add assert

		SlotType * pSlot = pos;
		m_pHolder->_willEraseSlots(pSlot, 1);
		return iterator(_erase(pSlot));
	}

	template < class SlotType>
	void DynamicSlotVector<SlotType>::erase(int index, int amount)
	{
		//TODO: Add assert

		SlotType * pSlot = _slot(index);
		m_pHolder->_willEraseSlots(pSlot, amount);
		_erase(pSlot, pSlot+amount);
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::erase(iterator beg, iterator end)
	{
		//TODO: Add assert

		SlotType * pBeg = beg;
		SlotType * pEnd = end;

		m_pHolder->_willEraseSlots(pBeg, int(pEnd-pBeg) );
		return iterator(_erase(pBeg, pEnd));
	}

	//____ clear() _______________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::clear()
	{
		if (isEmpty())
			return;
		m_pHolder->_willEraseSlots(_begin(), size());
		_clear();
	}

	//____ moveToBack() __________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::moveToBack(int index)
	{
		//TODO: Assert

		SlotType * pFrom = _slot(index);
		SlotType * pTo = _end()-1;

		if (pFrom != pTo)
		{
			_move(pFrom, pTo);
			m_pHolder->_didMoveSlots(pFrom, pTo, 1);
		}
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::moveToBack(iterator it)
	{
		//TODO: Assert

		SlotType * pFrom = it;
		SlotType * pTo = _end()-1;

		if (pFrom != pTo)
		{
			_move(pFrom, pTo);
			m_pHolder->_didMoveSlots(pFrom, pTo, 1);
		}
		return iterator(_last());
	}

	//____ moveToFront() _________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::moveToFront(int index)
	{
		//TODO: Assert

		SlotType * pFrom = _slot(index);
		SlotType * pTo = _begin();

		if (pFrom != pTo)
		{
			_move(pFrom, pTo);
			m_pHolder->_didMoveSlots(pFrom, pTo, 1);
		}
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::moveToFront(iterator it)
	{
		//TODO: Assert

		SlotType * pFrom = it;
		SlotType * pTo = _begin();

		if (pFrom != pTo)
		{
			_move(pFrom, pTo);
			m_pHolder->_didMoveSlots(pFrom, pTo, 1);
		}

		return iterator(_begin());
	}

	//____ moveBefore() __________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::moveBefore(int index, int sibling)
	{
		//TODO: Assert

		SlotType * pFrom = _slot(index);
		SlotType * pTo = _slot(sibling);


		if (pFrom < pTo)
			pTo--;

		if (pFrom != pTo)
		{
			_move(pFrom, pTo);
			m_pHolder->_didMoveSlots(pFrom, pTo, 1);
		}
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::iterator DynamicSlotVector<SlotType>::moveBefore(iterator it, iterator sibling)
	{
		//TODO: Assert

		SlotType * pFrom = it;
		SlotType * pTo = sibling;

		if (pFrom < pTo)
			pTo--;

		if (pFrom != pTo)
		{
			_move(pFrom, pTo);
			m_pHolder->_didMoveSlots(pFrom, pTo, 1);
		}

		return iterator(pTo);
	}

	//____ makeLink() ____________________________________________________________

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::SlotLink DynamicSlotVector<SlotType>::makeLink(int index)
	{
		//TODO: Bounds checks!

		return DynamicSlotVector<SlotType>::SlotLink( this, index);
	}

	template < class SlotType>
	typename DynamicSlotVector<SlotType>::SlotLink DynamicSlotVector<SlotType>::makeLink(iterator it)
	{
		//TODO: Bounds checks!

		return DynamicSlotVector<SlotType>::SlotLink( this, int(it - _begin()) );
	}

	//____ _releaseUpdateIndex() _____________________________________________

	template < class SlotType>
	int DynamicSlotVector<SlotType>::_releaseUpdateIndex(Widget* pToRelease, int index)
	{

		StaticSlot* pReleaseFromSlot = pToRelease->_slot();
		if (!pReleaseFromSlot)
			return index;

		if (_contains(static_cast<SlotType*>(pReleaseFromSlot)))
		{
			// We are releasing a widget from our own slot array, so we need to make sure index still is correct afterwards.

			if (&m_pArray[index] > pReleaseFromSlot)
				index--;
		}

		pToRelease->releaseFromParent();
		return index;
	}


	//____ _move() ____________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_move(SlotType * pFrom, SlotType * pTo)
	{
		int fromIndex = int(pFrom - m_pArray);
		int toIndex = int(pTo - m_pArray);

		SlotType temp = std::move(*pFrom);
		if (SlotType::safe_to_relocate)
		{
			if (pFrom < pTo)
			{
				memmove((void*)pFrom, &pFrom[1], sizeof(SlotType) * (pTo - pFrom));
				_reallocBlock(pFrom, pTo);
			}
			else
			{
				memmove((void*)&pTo[1], pTo, sizeof(SlotType) * (pFrom - pTo));
				_reallocBlock(pTo+1, pFrom+1);
			}
			pTo->m_pWidget = nullptr;
		}
		else
		{
			if (pFrom < pTo)
			{
				for (int i = 0; i < pTo - pFrom; i++)
					pFrom[i] = std::move(pFrom[i + 1]);
			}
			else
			{
				for (int i = int(pFrom - pTo); i > 0; i--)
					pTo[i] = std::move(pTo[i - 1]);
			}
		}
		*pTo = std::move(temp);

		_moveLink(fromIndex,toIndex);
	}

	//____ _reorder() _________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_reorder(int order[])
	{
		if (m_size == 0)
			return;

		int size = sizeof(SlotType)*m_capacity;
		SlotType* pOldArray = m_pArray;
		SlotType* pOldBuffer = m_pBuffer;
		m_pBuffer = (SlotType*)malloc(size);
		m_pArray = m_pBuffer + (pOldArray - pOldBuffer);
		for (int i = 0; i < m_size; i++)
		{
			int ofs = order[i];
			new (&m_pArray[i]) SlotType(std::move(pOldArray[ofs]));
		}
		free(pOldBuffer);

		_reorderLinks(order);
	}

	//____ _find() ____________________________________________________________

	template < class SlotType>
	SlotType * DynamicSlotVector<SlotType>::_find(const Widget* pWidget) const
	{
		for (auto p = _begin(); p < _end(); p++)
			if (p->_widget() == pWidget)
				return p;

		return nullptr;
	}

	//____ _clearLinks() _________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_clearLinks()
	{
		SlotLink * pLink = m_pFirstLink;

		while( pLink )
		{
			SlotLink * pNextLink = pLink->m_pNext;

			pLink->m_pSlot		= nullptr;
			pLink->m_pVector	= nullptr;
			pLink->m_index		= -1;
			pLink->m_pNext		= nullptr;
			pLink->m_pPrev		= nullptr;

			pLink = pNextLink;
		}
	}

	//____ _updateLinks() __________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_updateLinks()
	{
		SlotLink * pLink = m_pFirstLink;

		while( pLink )
		{
			pLink->m_pSlot = m_pArray + pLink->m_index;
			pLink = pLink->m_pNext;
		}
	}

	//____ _updateLinksAfterInsertion() __________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_updateLinksAfterInsertion( int insertionPoint, int nbInserted )
	{
		SlotLink * pLink = m_pFirstLink;

		while( pLink )
		{
			if( pLink->m_index >= insertionPoint )
				pLink->m_index += nbInserted;

			pLink->m_pSlot = m_pArray + pLink->m_index;

			pLink = pLink->m_pNext;
		}
	}

	//____ _updateLinksAfterRemoval() __________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_updateLinksAfterRemoval( int removalPoint, int nbRemoved )
	{
		SlotLink * pLink = m_pFirstLink;

		while( pLink )
		{
			SlotLink * pNextLink = pLink->m_pNext;

			if( pLink->m_index >= removalPoint )
			{
				if( pLink->m_index < removalPoint + nbRemoved )
				{
					pLink->_disconnect();

					pLink->m_pSlot		= nullptr;
					pLink->m_pVector	= nullptr;
					pLink->m_index		= -1;
					pLink->m_pNext		= nullptr;
					pLink->m_pPrev		= nullptr;
				}
				else
				{
					pLink->m_index -= nbRemoved;
					pLink->m_pSlot = m_pArray + pLink->m_index;
				}
			}
			else
				pLink->m_pSlot = m_pArray + pLink->m_index;

			pLink = pNextLink;
		}
	}

	//____ _moveLink() ___________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_moveLink( int oldIndex, int newIndex )
	{
		SlotLink * pLink = m_pFirstLink;

		while( pLink )
		{
			if( pLink->m_index == oldIndex )
				pLink->m_index = newIndex;

			pLink->m_pSlot = m_pArray + pLink->m_index;

			pLink = pLink->m_pNext;
		}
	}

	//____ _reorderLinks() _______________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_reorderLinks(int order[])
	{
		SlotLink * pLink = m_pFirstLink;

		while( pLink )
		{
			pLink->m_index = order[pLink->m_index];
			pLink->m_pSlot = m_pArray + pLink->m_index;

			pLink = pLink->m_pNext;
		}

	}

	//____ _reallocArray() ____________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_reallocArray(int capacity, int offset)
	{
		int size = sizeof(SlotType)*capacity;
		SlotType* pOldBuffer = m_pBuffer;
		SlotType* pOldArray = m_pArray;

		m_pBuffer = (SlotType*)malloc(size);
		m_pArray = m_pBuffer + offset;
		m_capacity = capacity;

		if (pOldBuffer)
		{
			if (SlotType::safe_to_relocate)
			{
				memcpy((void*)m_pArray, pOldArray, sizeof(SlotType)*m_size);
				_reallocBlock(_begin(), _end());
			}
			else
			{
				for (int i = 0; i < m_size; i++)
					new (&m_pArray[i]) SlotType(std::move(pOldArray[i]));
			}
			free(pOldBuffer);
		}

		_updateLinks();
	}

	//____ _reallocBlock() ____________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_reallocBlock(SlotType * pBeg, SlotType * pEnd)
	{
		while (pBeg < pEnd)
		{
			pBeg->_relink();
			pBeg++;
		}
	}

	//____ _deleteBlock() _____________________________________________________

	template < class SlotType>
	SlotType* DynamicSlotVector<SlotType>::_deleteBlock(SlotType * pBeg, SlotType * pEnd)
	{
		int begIndex = int(pBeg - m_pArray);

		if (m_pBuffer == m_pArray)
		{
			int blocksToMove = int(_end() - pEnd);
			if (SlotType::safe_to_relocate)
			{
				_killBlock(pBeg, pEnd);

				if (blocksToMove > 0)
				{
					memmove(pBeg, pEnd, sizeof(SlotType) * blocksToMove);
					_reallocBlock(pBeg, pBeg + blocksToMove);
				}
			}
			else
			{
				for (int i = 0; i < blocksToMove; i++)
					pBeg[i] = std::move(pEnd[i]);

				_killBlock(pBeg + blocksToMove, pEnd);
			}
		}
		else
		{
			int blocksToMove = int(pBeg - _begin());
			if (SlotType::safe_to_relocate)
			{
				_killBlock(pBeg, pEnd);

				if( blocksToMove > 0 )
				{
					memmove(pEnd - blocksToMove, pBeg - blocksToMove, sizeof(SlotType) * blocksToMove);
					_reallocBlock(pEnd - blocksToMove, pEnd);
				}
			}
			else
			{
				for (int i = 0; i < blocksToMove; i++)
					pEnd[-1 - i] = std::move(pBeg[-1 - i]);

				_killBlock(pBeg + blocksToMove, pEnd);
			}

			m_pArray += pEnd - pBeg;
		}


		m_size -= pEnd - pBeg;
		assert( m_size >= 0 );

		_updateLinksAfterRemoval(begIndex, int(pEnd - pBeg));
		return pBeg;
	}

	//____ _insertBlock() _____________________________________________________

	template < class SlotType>
	SlotType* DynamicSlotVector<SlotType>::_insertBlock(SlotType * pPos, int entries)
	{
		int insertionIndex = int(pPos - m_pArray);

		if (entries <= m_capacity - m_size)
		{
			if (m_pBuffer == m_pArray)
			{
				// We grow at the end, so make space for insertion by moving entries after insertion point backward.

				int nToMove = (int)(_end() - pPos);
				if (nToMove > 0)
				{
					if (SlotType::safe_to_relocate)
					{
						memmove((void*)&pPos[entries], pPos, sizeof(SlotType) * nToMove);
						_reallocBlock(&pPos[entries], &pPos[entries + nToMove]);
					}
					else
					{
						int nMoveToNew = std::min(entries, nToMove);
						int nMoveToExisting = nToMove - nMoveToNew;

						SlotType * pFrom = _end();
						SlotType * pTo = pFrom + entries;

						// Move first batch of entries to uninitialized objects

						for (int i = 0; i < nMoveToNew; i++)
							new (--pTo) SlotType(std::move(*(--pFrom)));

						// Move the rest to existing objects

						for (int i = 0; i < nMoveToExisting; i++)
							*(--pTo) = std::move(*(--pFrom));
					}
				}
			}
			else
			{
				// We grow at the front, so make space for insertion by moving entries before insertion point forward.

				int nToMove = int(pPos - _begin());
				if (nToMove > 0)
				{
					if (SlotType::safe_to_relocate)
					{
						memmove(m_pArray - entries, m_pArray, sizeof(SlotType) * nToMove);
						_reallocBlock(m_pArray - entries, m_pArray - entries + nToMove);
					}
					else
					{
						int nMoveToNew = std::min(entries, nToMove);
						int nMoveToExisting = nToMove - nMoveToNew;

						SlotType * pFrom = m_pArray;
						SlotType * pTo = m_pArray - entries;

						// Move first batch of entries to uninitialized objects

						for (int i = 0; i < nMoveToNew; i++)
							new (pTo++) SlotType(std::move(*(pFrom++)));

						// Move the rest to existing objects

						for (int i = 0; i < nMoveToExisting; i++)
							*(pTo++) = std::move(*(pFrom++));
					}
				}
				m_pArray -= entries;
				pPos -= entries;
			}
		}
		else
		{
			int grow = entries - (m_capacity - m_size);
			if (grow == 1)
				m_capacity = (m_capacity + 1) * 2;
			else
				m_capacity += grow;

			SlotType* pOldBuffer = m_pBuffer;
			SlotType* pOldArray = m_pArray;

			m_pBuffer = (SlotType*)malloc(sizeof(SlotType) * m_capacity);
			m_pArray = pOldArray == pOldBuffer ? m_pBuffer : m_pBuffer + m_capacity - m_size - entries;


			if (pOldBuffer)
			{
				int nPreceeding = int(pPos - pOldArray);
				int nSucceeding = m_size - nPreceeding;

				if (SlotType::safe_to_relocate)
				{
					memcpy((void*)m_pArray, pOldArray, sizeof(SlotType) * nPreceeding);
					memcpy((void*)(m_pArray + nPreceeding + entries), (void*)(pOldArray + nPreceeding), sizeof(SlotType) * nSucceeding);
					_reallocBlock(m_pArray, m_pArray + nPreceeding);
					_reallocBlock(m_pArray + nPreceeding + entries, m_pArray + nPreceeding + entries + nSucceeding);
				}
				else
				{
					for (int i = 0; i < nPreceeding; i++)
						new (&m_pArray[i]) SlotType(std::move(pOldArray[i]));

					SlotType * pDest = m_pArray + nPreceeding + entries;
					for (int i = 0; i < nSucceeding; i++)
						new (&pDest[i]) SlotType(std::move(pPos[i]));
				}

				free(pOldBuffer);
			}

			pPos = m_pArray + (pPos - pOldArray);
		}

		m_size += entries;
		_initBlock(pPos, &pPos[entries]);

		_updateLinksAfterInsertion(insertionIndex, entries);
		return pPos;
	}

	//____ _killBlock() __________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_killBlock(SlotType * pBeg, SlotType * pEnd)
	{
		while (pBeg < pEnd)
			(pBeg++)->~SlotType();
	}

	//____ _initBlock() __________________________________________________________

	template < class SlotType>
	void DynamicSlotVector<SlotType>::_initBlock(SlotType * pBeg, SlotType * pEnd)
	{
		while (pBeg < pEnd)
			new (pBeg++) SlotType(m_pHolder);
	}

} // namespace wg

#endif //WG_DYNAMICSLOTVECTOR_IMPL_DOT_H

