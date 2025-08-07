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

#ifndef	WG_STATICTEXTINFOPANEL_DOT_H
#define WG_STATICTEXTINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_statictext.h>

namespace wg
{
	class StaticTextInfoPanel;
	typedef	StrongPtr<StaticTextInfoPanel>	StaticTextInfoPanel_p;
	typedef	WeakPtr<StaticTextInfoPanel>	StaticTextInfoPanel_wp;



	class StaticTextInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static StaticTextInfoPanel_p		create(const Blueprint& blueprint, IDebugger* pHolder, StaticText* pStaticText) { return StaticTextInfoPanel_p(new StaticTextInfoPanel(blueprint, pHolder, pStaticText)); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		StaticTextInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticText* pStaticText);
		~StaticTextInfoPanel() {}

		TablePanel_p	m_pTable;
		StaticText *	m_pInspected;
		TextDisplay_p	m_pTextDisplay;

		Object_p		m_displayedStylePtr;
		Object_p		m_displayedLayoutPtr;

	};

} // namespace wg
#endif //WG_StaticTextINFOPANEL_DOT_H

