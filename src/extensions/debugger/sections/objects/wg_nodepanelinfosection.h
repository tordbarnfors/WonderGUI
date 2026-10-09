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
#ifndef	WG_NODEPANELINFOSECTION_DOT_H
#define WG_NODEPANELINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_nodepanel.h>

namespace wg
{
	class NodePanelInfoSection;
	typedef	StrongPtr<NodePanelInfoSection>	NodePanelInfoSection_p;
	typedef	WeakPtr<NodePanelInfoSection>	NodePanelInfoSection_wp;



	class NodePanelInfoSection : public TypedInfoSection<NodePanel>
	{
	public:

		//.____ Creation __________________________________________

		static NodePanelInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, NodePanel * pInspected) { return NodePanelInfoSection_p(new NodePanelInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		NodePanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, NodePanel * pInspected );
		~NodePanelInfoSection() {}

		DrawerPanel_p	m_pSlotsDrawer;
	};

} // namespace wg
#endif //WG_NODEPANELINFOSECTION_DOT_H
