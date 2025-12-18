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
#ifndef	WG_STATICSLOTVECTOR_IMPL_DOT_H
#define	WG_STATICSLOTVECTOR_IMPL_DOT_H
#pragma once

#include <wg_staticslotvector.h>
#include <cstring>

//#define INSTANTIATE_CHILDREN(SlotType)		template class StaticSlotVector< SlotType >;

namespace wg
{
	class Widget;

	//____ _releaseUpdateIndex() _____________________________________________

	template < class SlotType>
	int StaticSlotVector<SlotType>::_releaseUpdateIndex(Widget* pToRelease, int index)
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
	void StaticSlotVector<SlotType>::_move(SlotType * pFrom, SlotType * pTo)
	{
		SlotType temp = std::move(*pFrom);
		if (pFrom < pTo)
		{
			if (SlotType::safe_to_relocate)
			{
				memmove((void*)pFrom, &pFrom[1], sizeof(SlotType) * (pTo - pFrom));
				_reallocBlock(pFrom, pTo);
			}
			else
			{
				for (int i = 0; i < pTo - pFrom; i++)
					pFrom[i] = std::move(pFrom[i + 1]);
			}
		}
		else
		{
			if (SlotType::safe_to_relocate)
			{
				memmove((void*)&pTo[1], pTo, sizeof(SlotType) * (pFrom - pTo));
				_reallocBlock(pTo, pFrom);
			}
			else
			{
				for (int i = int(pFrom - pTo); i > 0; i--)
					pTo[i] = std::move(pTo[i - 1]);
			}
		}
		*pTo = std::move(temp);
	}

	//____ _reorder() _________________________________________________________

	template < class SlotType>
	void StaticSlotVector<SlotType>::_reorder(int order[])
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
	}

	//____ _find() ____________________________________________________________

	template < class SlotType>
	SlotType * StaticSlotVector<SlotType>::_find(const Widget* pWidget) const
	{
		for (auto p = _begin(); p < _end(); p++)
			if (p->_widget() == pWidget)
				return p;

		return nullptr;
	}

	//____ _reallocArray() ____________________________________________________

	template < class SlotType>
	void StaticSlotVector<SlotType>::_reallocArray(int capacity, int offset)
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
	}

	//____ _reallocBlock() ____________________________________________________

	template < class SlotType>
	void StaticSlotVector<SlotType>::_reallocBlock(SlotType * pBeg, SlotType * pEnd)
	{
		while (pBeg < pEnd)
		{
			pBeg->_relink();
			pBeg++;
		}
	}

	//____ _deleteBlock() _____________________________________________________

	template < class SlotType>
	SlotType* StaticSlotVector<SlotType>::_deleteBlock(SlotType * pBeg, SlotType * pEnd)
	{
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
			if (blocksToMove > 0)
			{
				if (SlotType::safe_to_relocate)
				{
					_killBlock(pBeg, pEnd);

					memmove(pEnd - blocksToMove, pBeg - blocksToMove, sizeof(SlotType) * blocksToMove);
					_reallocBlock(pEnd - blocksToMove, pEnd);
				}
				else
				{
					for (int i = 0; i < blocksToMove; i++)
						pEnd[-1 - i] = std::move(pBeg[-1 - i]);

					_killBlock(pBeg + blocksToMove, pEnd);
				}
			}
			m_pArray += pEnd - pBeg;
		}

		m_size -= pEnd - pBeg;
		return pBeg;
	}

	//____ _insertBlock() _____________________________________________________

	template < class SlotType>
	SlotType* StaticSlotVector<SlotType>::_insertBlock(SlotType * pPos, int entries)
	{
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
		return pPos;
	}

	//____ _killBlock() __________________________________________________________

	template < class SlotType>
	void StaticSlotVector<SlotType>::_killBlock(SlotType * pBeg, SlotType * pEnd)
	{
		while (pBeg < pEnd)
			(pBeg++)->~SlotType();
	}

	//____ _initBlock() __________________________________________________________

	template < class SlotType>
	void StaticSlotVector<SlotType>::_initBlock(SlotType * pBeg, SlotType * pEnd)
	{
		while (pBeg < pEnd)
			new (pBeg++) SlotType(m_pHolder);
	}




} // namespace wg

#endif //WG_STATICSLOTVECTOR_IMPL_DOT_H

