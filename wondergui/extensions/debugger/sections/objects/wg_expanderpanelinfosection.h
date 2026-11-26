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
#ifndef	WG_EXPANDERPANELINFOSECTION_DOT_H
#define WG_EXPANDERPANELINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_expanderpanel.h>

namespace wg
{
	class ExpanderPanelInfoSection;
	typedef	StrongPtr<ExpanderPanelInfoSection>	ExpanderPanelInfoSection_p;
	typedef	WeakPtr<ExpanderPanelInfoSection>	ExpanderPanelInfoSection_wp;



	class ExpanderPanelInfoSection : public TypedInfoSection<ExpanderPanel>
	{
	public:

		//.____ Creation __________________________________________

		static ExpanderPanelInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, ExpanderPanel * pInspected) { return ExpanderPanelInfoSection_p(new ExpanderPanelInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		ExpanderPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, ExpanderPanel * pInspected );
		~ExpanderPanelInfoSection() {}

		ExpanderPanel_p	m_pSlotsDrawer;
	};

} // namespace wg
#endif //WG_EXPANDERPANELINFOSECTION_DOT_H
