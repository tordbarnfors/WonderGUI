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
#ifndef	WG_NODEPANELSLOTINFOSECTION_DOT_H
#define WG_NODEPANELSLOTINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_nodepanel.h>

namespace wg
{
	class NodePanelSlotInfoSection;
	typedef	StrongPtr<NodePanelSlotInfoSection>	NodePanelSlotInfoSection_p;
	typedef	WeakPtr<NodePanelSlotInfoSection>	NodePanelSlotInfoSection_wp;



	class NodePanelSlotInfoSection : public TypedInfoSection<NodePanelSlot>
	{
	public:

		//.____ Creation __________________________________________

		static NodePanelSlotInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, NodePanelSlot * pInspected) { return NodePanelSlotInfoSection_p(new NodePanelSlotInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		NodePanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, NodePanelSlot * pInspected );
		~NodePanelSlotInfoSection() {}

	};

} // namespace wg
#endif //WG_NODEPANELSLOTINFOSECTION_DOT_H
