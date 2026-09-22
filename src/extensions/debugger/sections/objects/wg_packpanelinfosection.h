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
#ifndef	WG_PACKPANELINFOSECTION_DOT_H
#define WG_PACKPANELINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_infosection.h>
#include <wg_packpanel.h>

namespace wg
{
	class PackPanelInfoSection;
	typedef	StrongPtr<PackPanelInfoSection>	PackPanelInfoSection_p;
	typedef	WeakPtr<PackPanelInfoSection>	PackPanelInfoSection_wp;



	class PackPanelInfoSection : public InfoSection
	{
	public:

		//.____ Creation __________________________________________

		static PackPanelInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, PackPanel * pPanel) { return PackPanelInfoSection_p(new PackPanelInfoSection(theme, pContext, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void 					refresh() override;

	protected:
		PackPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, PackPanel * pPanel );
		~PackPanelInfoSection() {}

		PackPanel *		m_pInspected;
		TablePanel_p	m_pTable;

		DrawerPanel_p	m_pSlotsDrawer;
		Object_p		m_displayedLayoutPointer;

	};

} // namespace wg
#endif //WG_PACKPANELINFOSECTION_DOT_H

