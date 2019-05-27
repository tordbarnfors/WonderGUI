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

#ifndef	WG_ICHILDREN_IMPL_DOT_H
#define	WG_ICHILDREN_IMPL_DOT_H
#pragma once


#define INSTANTIATE_CHILDREN(SlotType,HolderType)		template class IChildrenSubclass< SlotType, HolderType >;

namespace wg
{

	template < class SlotType, class HolderType>
	void IChildrenSubclass<SlotType, HolderType>::_releaseGuardPointer(Widget * pToRelease, SlotType ** pPointerToGuard)
	{
		Container * pParent = pToRelease->_parent();

		if (pParent)
		{
			Slot * pReleaseFromSlot = pToRelease->_slot();

			if (m_pSlotArray->contains(pReleaseFromSlot))
			{
				// We are releasing a widget from our own slot array, so we need to make sure pointer still is correct afterwards.

				int ofs = (int)((*pPointerToGuard) - m_pSlotArray->first());
				if (*pPointerToGuard > pReleaseFromSlot)
					ofs--;

				pToRelease->releaseFromParent();
				*pPointerToGuard = m_pSlotArray->first() + ofs;
			}
			else
				pToRelease->releaseFromParent();
		}
	}

} // namespace wg

#endif //WG_ICHILDREN_IMPL_DOT_H

