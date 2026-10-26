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
#include "wg_stackpanelinfosection.h"
#include <wg_stackpanel.h>


namespace wg
{

	const TypeInfo StackPanelInfoSection::TYPEINFO = { "StackPanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	StackPanelInfoSection::StackPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, StackPanel * pInspected)
		: TypedInfoSection<StackPanel>( theme, pContext, StackPanel::TYPEINFO.className, pInspected )
	{
		m_pSlotsDrawer = _createSlotsDrawer("Slots", pInspected->slots.begin(), pInspected->slots.end());

		this->slot = m_pSlotsDrawer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StackPanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void StackPanelInfoSection::refresh()
	{
		TypedInfoSection<StackPanel>::refresh();

		_refreshSlotsDrawer(m_pSlotsDrawer, inspected()->slots.begin(), inspected()->slots.end());
	}

} // namespace wg
