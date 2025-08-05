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

#ifndef	WG_PACKPANELINFOPANEL_DOT_H
#define WG_PACKPANELINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_packpanel.h>

namespace wg
{
	class PackPanelInfoPanel;
	typedef	StrongPtr<PackPanelInfoPanel>	PackPanelInfoPanel_p;
	typedef	WeakPtr<PackPanelInfoPanel>	PackPanelInfoPanel_wp;



	class PackPanelInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static PackPanelInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, PackPanel * pPanel) { return PackPanelInfoPanel_p(new PackPanelInfoPanel(blueprint, pHolder, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		PackPanelInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, PackPanel * pPanel );
		~PackPanelInfoPanel() {}
	};

} // namespace wg
#endif //WG_PACKPANELINFOPANEL_DOT_H

