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

#ifndef	WG_GLOWINFOPANEL_DOT_H
#define WG_GLOWINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_glow.h>

namespace wg
{
	class GlowInfoPanel;
	typedef	StrongPtr<GlowInfoPanel>	GlowInfoPanel_p;
	typedef	WeakPtr<GlowInfoPanel>	GlowInfoPanel_wp;



	class GlowInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static GlowInfoPanel_p		create(const Blueprint& blueprint, IDebugger* pHolder, Glow* pGlow) { return GlowInfoPanel_p(new GlowInfoPanel(blueprint, pHolder, pGlow)); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		GlowInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Glow* pGlow);
		~GlowInfoPanel() {}

		TablePanel_p	m_pTable;
		Glow*			m_pInspected;

		DrawerPanel_p	m_pSeedTintDrawer;
		DrawerPanel_p	m_pGlowTintDrawer;


		Object_p		m_displayedSkinPtr;
		Object_p		m_displayedBrushPtr;
		Object_p		m_displayedSurface1Ptr;
		Object_p		m_displayedSurface2Ptr;

		HiColor			m_displayedSeedTint;
		HiColor			m_displayedGlowTint;
	};

} // namespace wg
#endif //WG_GLOWINFOPANEL_DOT_H

