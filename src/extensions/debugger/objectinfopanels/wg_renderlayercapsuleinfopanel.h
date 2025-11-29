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
#ifndef	WG_RENDERLAYERCAPSULEINFOPANEL_DOT_H
#define WG_RENDERLAYERCAPSULEINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_renderlayercapsule.h>

namespace wg
{
	class RenderLayerCapsuleInfoPanel;
	typedef	StrongPtr<RenderLayerCapsuleInfoPanel>	RenderLayerCapsuleInfoPanel_p;
	typedef	WeakPtr<RenderLayerCapsuleInfoPanel>	RenderLayerCapsuleInfoPanel_wp;



	class RenderLayerCapsuleInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static RenderLayerCapsuleInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, RenderLayerCapsule * pPanel) { return RenderLayerCapsuleInfoPanel_p(new RenderLayerCapsuleInfoPanel(blueprint, pHolder, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		RenderLayerCapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, RenderLayerCapsule * pPanel );
		~RenderLayerCapsuleInfoPanel() {}

		RenderLayerCapsule *	m_pInspected;
		TablePanel_p			m_pTable;
	};

} // namespace wg
#endif //WG_RENDERLAYERCAPSULEINFOPANEL_DOT_H

