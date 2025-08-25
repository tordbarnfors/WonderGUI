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

#ifndef	WG_CANVASCAPSULEINFOPANEL_DOT_H
#define WG_CANVASCAPSULEINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_CanvasCapsule.h>

namespace wg
{
	class CanvasCapsuleInfoPanel;
	typedef	StrongPtr<CanvasCapsuleInfoPanel>	CanvasCapsuleInfoPanel_p;
	typedef	WeakPtr<CanvasCapsuleInfoPanel>	CanvasCapsuleInfoPanel_wp;



	class CanvasCapsuleInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static CanvasCapsuleInfoPanel_p		create(const Blueprint& blueprint, IDebugger* pHolder, CanvasCapsule* pPanel) { return CanvasCapsuleInfoPanel_p(new CanvasCapsuleInfoPanel(blueprint, pHolder, pPanel)); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		CanvasCapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, CanvasCapsule* pPanel);
		~CanvasCapsuleInfoPanel() {}

		CanvasCapsule*	m_pInspected;
		TablePanel_p	m_pTable;

		DrawerPanel_p	m_pClearColorDrawer;
		DrawerPanel_p	m_pTintColorDrawer;

		DrawerPanel_p	m_pGlowDrawer;

		Object_p		m_displayedCanvasPtr;
		Object_p		m_displayedGlowCanvasPtr;
		Object_p		m_displayedLayersPtr;
		Object_p		m_displayedFactoryPtr;
		Object_p		m_displayedTintmapPtr;


		HiColor			m_displayedClearColor;
		HiColor			m_displayedTintColor;

	};

} // namespace wg
#endif //WG_CANVASCAPSULEINFOPANEL_DOT_H

