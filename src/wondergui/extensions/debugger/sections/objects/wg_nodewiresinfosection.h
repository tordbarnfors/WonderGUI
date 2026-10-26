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
#ifndef	WG_NODEWIRESINFOSECTION_DOT_H
#define WG_NODEWIRESINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_nodewires.h>

namespace wg
{
	class NodeWiresInfoSection;
	typedef	StrongPtr<NodeWiresInfoSection>	NodeWiresInfoSection_p;
	typedef	WeakPtr<NodeWiresInfoSection>	NodeWiresInfoSection_wp;



	class NodeWiresInfoSection : public TypedInfoSection<NodeWires>
	{
	public:

		//.____ Creation __________________________________________

		static NodeWiresInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, NodeWires * pInspected) { return NodeWiresInfoSection_p(new NodeWiresInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		NodeWiresInfoSection(const DebugTheme& theme, IDebugContext* pContext, NodeWires * pInspected );
		~NodeWiresInfoSection() {}

		DrawerPanel_p	m_pWireColorDrawer;
		DrawerPanel_p	m_pAnchorInsetDrawer;

		HiColor			m_displayedWireColor;
		Border			m_displayedAnchorInset;
	};

} // namespace wg
#endif //WG_NODEWIRESINFOSECTION_DOT_H
