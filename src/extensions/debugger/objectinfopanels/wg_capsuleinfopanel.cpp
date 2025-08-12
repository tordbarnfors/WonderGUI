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
#include "wg_capsuleinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo CapsuleInfoPanel::TYPEINFO = { "CapsuleInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	CapsuleInfoPanel::CapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Capsule * pCapsule) : DebugPanel( blueprint, pHolder, Capsule::TYPEINFO.className )
	{
		m_pInspected = pCapsule;
		m_pSlotDrawer = _createSingleSlotDrawer("Slot", &m_pInspected->slot);
		this->slot = m_pSlotDrawer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& CapsuleInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void CapsuleInfoPanel::refresh()
	{
		_refreshSingleSlotDrawer(m_pSlotDrawer, &m_pInspected->slot);
	}

} // namespace wg


