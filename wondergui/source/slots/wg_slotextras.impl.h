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
#ifndef WG_SLOTEXTRAS_IMPL_DOT_H
#define WG_SLOTEXTRAS_IMPL_DOT_H
#pragma once

#include <wg_slotextras.h>


namespace wg
{
	//____ setPadding() _______________________________________________________

	template<class SlotType, class IteratorType, class HolderType>
	bool PaddedSlotCollectionMethods<SlotType,IteratorType,HolderType>::setPadding(int index, int amount, Border padding)
	{
		//TODO: Assert

		_holder()->_repadSlots(_slot(index), amount, padding);
		return true;
	}

	template<class SlotType, class IteratorType, class HolderType>
	bool PaddedSlotCollectionMethods<SlotType,IteratorType,HolderType>::setPadding(const IteratorType& beg, const IteratorType& end, Border padding)
	{
		//TODO: Assert

		auto pBeg = beg;
		auto pEnd = end;
		_holder()->_repadSlots(pBeg, int(pEnd - pBeg), padding);
		return true;
	}

	template<class SlotType, class IteratorType, class HolderType>
	bool PaddedSlotCollectionMethods<SlotType,IteratorType,HolderType>::setPadding(int index, int amount, std::initializer_list<Border> padding)
	{
		//TODO: Assert

		_holder()->_repadSlots(_slot(index), amount, padding.begin());
		return true;
	}

	template<class SlotType, class IteratorType, class HolderType>
	bool PaddedSlotCollectionMethods<SlotType,IteratorType,HolderType>::setPadding(const IteratorType& beg, const IteratorType& end, std::initializer_list<Border> padding)
	{
		//TODO: Assert

		auto pBeg = beg;
		auto pEnd = end;
		_holder()->_repadSlots(pBeg, int(pEnd - pBeg), padding.begin());
		return true;
	}


	//____ select() ___________________________________________________________

	template<class SlotType, class IteratorType, class HolderType>
	void SelectableSlotCollectionMethods<SlotType,IteratorType,HolderType>::select(int index, int amount)
	{
		//TODO: Error handling!

		_holder()->_selectSlots(_slot(index), amount);
	};

	template<class SlotType, class IteratorType, class HolderType>
	void SelectableSlotCollectionMethods<SlotType,IteratorType,HolderType>::select(const IteratorType& beg, const IteratorType& end)
	{
		//TODO: Error handling!

		auto pBeg = beg;
		auto pEnd = end;
		_holder()->_selectSlots(pBeg, int(pEnd - pBeg));
	};

	//____ selectAll() ________________________________________________________

	template<class SlotType, class IteratorType, class HolderType>
	void SelectableSlotCollectionMethods<SlotType,IteratorType,HolderType>::selectAll()
	{
		_holder()->_selectSlots(_slot(0), _size());
	};

	//____ unselect() _________________________________________________________

	template<class SlotType, class IteratorType, class HolderType>
	void SelectableSlotCollectionMethods<SlotType,IteratorType,HolderType>::unselect(int index, int amount)
	{
		//TODO: Error handling!

		_holder()->_unselectSlots(_slot(index), amount);
	};

	template<class SlotType, class IteratorType, class HolderType>
	void SelectableSlotCollectionMethods<SlotType,IteratorType,HolderType>::unselect(const IteratorType& beg, const IteratorType& end)
	{
		//TODO: Error handling!

		auto pBeg = beg;
		auto pEnd = end;
		_holder()->_unselectSlots(pBeg, int(pEnd - pBeg));
	};

	//____ unselectAll() ______________________________________________________

	template<class SlotType, class IteratorType, class HolderType>
	void SelectableSlotCollectionMethods<SlotType,IteratorType,HolderType>::unselectAll()
	{
		_holder()->_unselectSlots( _slot(0), _size() );
	};


	//____ hide() ________________________________________________________________

	template<class SlotType, class IteratorType, class HolderType>
	void HideableSlotCollectionMethods<SlotType, IteratorType, HolderType>::hide(int index, int amount)
	{
		//TODO: Error handling!

		_holder()->_hideSlots(_slot(index), amount);
	};

	template<class SlotType, class IteratorType, class HolderType>
	void HideableSlotCollectionMethods<SlotType, IteratorType, HolderType>::hide(const IteratorType& beg, const IteratorType& end)
	{
		//TODO: Error handling!

		SlotType* pBeg = beg;
		SlotType* pEnd = end;
		_holder()->_hideSlots(pBeg, int(pEnd - pBeg));
	};

	//____ hideAll() _____________________________________________________________

	template<class SlotType, class IteratorType, class HolderType>
	void HideableSlotCollectionMethods<SlotType, IteratorType, HolderType>::hideAll()
	{
		_holder()->_hideSlots(_slot(0), _size());
	};

	//____ unhide() ______________________________________________________________

	template<class SlotType, class IteratorType, class HolderType>
	void HideableSlotCollectionMethods<SlotType, IteratorType, HolderType>::unhide(int index, int amount)
	{
		//TODO: Error handling!

		_holder()->_unhideSlots(_slot(index), amount);
	};

	template<class SlotType, class IteratorType, class HolderType>
	void HideableSlotCollectionMethods<SlotType, IteratorType, HolderType>::unhide(const IteratorType& beg, const IteratorType& end)
	{
		//TODO: Error handling!

		SlotType* pBeg = beg;
		SlotType* pEnd = end;
		_holder()->_unhideSlots(pBeg, int(pEnd - pBeg));
	};

	//____ unhideAll() ________________________________________________________

	template<class SlotType, class IteratorType, class HolderType>
	void HideableSlotCollectionMethods<SlotType, IteratorType, HolderType>::unhideAll()
	{
		_holder()->_unhideSlots(_slot(0), _size());
	};
}

#endif //WG_SLOTEXTRAS_IMPL_DOT_H
