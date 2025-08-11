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

#ifndef	WG_ICONINFOPANEL_DOT_H
#define WG_ICONINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_statictext.h>

namespace wg
{
	class IconInfoPanel;
	typedef	StrongPtr<IconInfoPanel>	IconInfoPanel_p;
	typedef	WeakPtr<IconInfoPanel>	IconInfoPanel_wp;



	class IconInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static IconInfoPanel_p		create(const Blueprint& blueprint, IDebugger* pHolder, Icon* pIcon) { return IconInfoPanel_p(new IconInfoPanel(blueprint, pHolder, pIcon)); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		IconInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Icon* pIcon);
		~IconInfoPanel() {}

		TablePanel_p	m_pTable;
		Icon *			m_pInspected;

		Object_p		m_displayedSkinPtr;
	};

} // namespace wg
#endif //WG_ICONINFOPANEL_DOT_H

