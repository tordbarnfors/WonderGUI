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

#ifndef	WG_PANEL_IMPL_DOT_H
#define	WG_PANEL_IMPL_DOT_H
#pragma once

#include <vector>
#include <wg_panel.h>
#include <wg_util.h>
#include <wg_patches.h>
#include <wg_gfxdevice.h>
#include <wg_slot.h>

namespace wg
{

	//____ constructor _____________________________________________________________

	template < class SlotType>
	PanelTemplate<SlotType>::PanelTemplate() : slots(this)
	{
	}

	//____ hideSlots() ___________________________________________________________

	template < class SlotType>
	void PanelTemplate<SlotType>::hideSlots(int index, int amount)
	{
		_hideSlots( &slots[index], amount);
	}

	template < class SlotType>
	void PanelTemplate<SlotType>::hideSlots(iterator beg, iterator end)
	{
		_hideSlots( beg, int(end - beg));
	}

	//____ unhideSlots() ___________________________________________________________

	template < class SlotType>
	void PanelTemplate<SlotType>::unhideSlots(int index, int amount)
	{
		_unhideSlots( &slots[index], amount);
	}

	template < class SlotType>
	void PanelTemplate<SlotType>::unhideSlots(iterator beg, iterator end)
	{
		_unhideSlots( beg, int(end - beg));
	}

	//____ _firstChild() __________________________________________________________

	template < class SlotType>
	Widget * PanelTemplate<SlotType>::_firstChild() const
	{
		if (slots.isEmpty())
			return nullptr;

		return slots.front()._widget();
	}

	//____ _lastChild() __________________________________________________________

	template < class SlotType>
	Widget * PanelTemplate<SlotType>::_lastChild() const
	{
		if (slots.isEmpty())
			return nullptr;

		return slots.back()._widget();
	}


	//____ _firstSlotWithGeo() _____________________________________________________

	template < class SlotType>
	void PanelTemplate<SlotType>::_firstSlotWithGeo( SlotWithGeo& package ) const
	{
		if( slots.isEmpty() )
			package.pSlot = nullptr;
		else
		{
			auto * pSlot = slots.begin();

			while (!static_cast<PanelSlot*>(pSlot)->m_bVisible)
			{
				if (pSlot == &slots.back())
				{
					package.pSlot = nullptr;
					return;
				}
				pSlot++;
			}

			package.pSlot = pSlot;
			package.geo = pSlot->m_geo;
		}
	}

	//____ _nextSlotWithGeo() _____________________________________________________

	template < class SlotType>
	void PanelTemplate<SlotType>::_nextSlotWithGeo( SlotWithGeo& package ) const
	{
		auto pSlot = (SlotType*) package.pSlot;

		if( pSlot == &slots.back() )
			package.pSlot = nullptr;
		else
		{
			pSlot++;

			while (!static_cast<PanelSlot*>(pSlot)->m_bVisible)
			{
				if (pSlot == &slots.back())
				{
					package.pSlot = nullptr;
					return;
				}
				pSlot++;
			}

			package.pSlot = pSlot;
			package.geo = pSlot->m_geo;
		}
	}

	//____ _prevChild() _______________________________________________________

	template < class SlotType>
	Widget * PanelTemplate<SlotType>::_prevChild(const StaticSlot * _pSlot) const
	{
		auto pSlot = static_cast<const SlotType*>(_pSlot);

		if (pSlot > slots.begin())
			return pSlot[-1]._widget();

		return nullptr;
	}

	//____ _nextChild() _______________________________________________________

	template < class SlotType>
	Widget * PanelTemplate<SlotType>::_nextChild(const StaticSlot * _pSlot) const
	{
		auto pSlot = static_cast<const SlotType*>(_pSlot);

		if (pSlot < &slots.back())
			return pSlot[1]._widget();

		return nullptr;
	}

} // namespace wg

#endif //WG_PANEL_IMPL_DOT_H
