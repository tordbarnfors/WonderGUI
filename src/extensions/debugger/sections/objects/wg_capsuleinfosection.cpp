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
#include "wg_capsuleinfosection.h"


namespace wg
{

	const TypeInfo CapsuleInfoSection::TYPEINFO = { "CapsuleInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	CapsuleInfoSection::CapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, Capsule * pInspected)
		: TypedInfoSection<Capsule>( theme, pContext, Capsule::TYPEINFO.className, pInspected )
	{
		m_pSlotDrawer = _createSingleSlotDrawer("Slot", &pInspected->slot);
		this->slot = m_pSlotDrawer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& CapsuleInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void CapsuleInfoSection::refresh()
	{
		TypedInfoSection<Capsule>::refresh();

		_refreshSingleSlotDrawer(m_pSlotDrawer, &inspected()->slot);
	}

} // namespace wg
