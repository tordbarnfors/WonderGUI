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
#include "wg_splitpanelslotinfosection.h"

namespace wg
{

	const TypeInfo SplitPanelSlotInfoSection::TYPEINFO = { "SplitPanelSlotInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	SplitPanelSlotInfoSection::SplitPanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, SplitPanel::Slot * pInspected)
		: TypedInfoSection<SplitPanel::Slot>( theme, pContext, SplitPanel::Slot::TYPEINFO.className, pInspected )
	{
		m_displayedGeo = pInspected->geo();
		m_pGeoDrawer = _createRectDrawer("Geo: ", m_displayedGeo);
		this->slot = m_pGeoDrawer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SplitPanelSlotInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void SplitPanelSlotInfoSection::refresh()
	{
		TypedInfoSection<SplitPanel::Slot>::refresh();

		_refreshRectDrawer(m_pGeoDrawer, inspected()->geo(), m_displayedGeo);
	}

} // namespace wg
