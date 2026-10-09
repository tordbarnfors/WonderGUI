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
#include "wg_modaloverlayinfosection.h"


namespace wg
{

	const TypeInfo ModalOverlayInfoSection::TYPEINFO = { "ModalOverlayInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ModalOverlayInfoSection::ModalOverlayInfoSection(const DebugTheme& theme, IDebugContext* pContext, ModalOverlay * pInspected)
		: TypedInfoSection<ModalOverlay>( theme, pContext, ModalOverlay::TYPEINFO.className, pInspected )
	{
		m_pModalSlotsDrawer = _createSlotsDrawer("Modal slots", pInspected->modalSlots.begin(), pInspected->modalSlots.end());
		this->slot = m_pModalSlotsDrawer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ModalOverlayInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ModalOverlayInfoSection::refresh()
	{
		TypedInfoSection<ModalOverlay>::refresh();

		_refreshSlotsDrawer(m_pModalSlotsDrawer, inspected()->modalSlots.begin(), inspected()->modalSlots.end());
	}

} // namespace wg
