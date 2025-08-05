/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef	WG_PANELINFOPANEL_DOT_H
#define WG_PANELINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_panel.h>

namespace wg
{
	class PanelInfoPanel;
	typedef	StrongPtr<PanelInfoPanel>	PanelInfoPanel_p;
	typedef	WeakPtr<PanelInfoPanel>	PanelInfoPanel_wp;



	class PanelInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static PanelInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, Panel * pPanel) { return PanelInfoPanel_p(new PanelInfoPanel(blueprint, pHolder, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		PanelInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Panel * pPanel );
		~PanelInfoPanel() {}
	};

} // namespace wg
#endif //WG_PANELINFOPANEL_DOT_H

