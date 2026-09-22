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
#ifndef	WG_CANVASCAPSULEINFOSECTION_DOT_H
#define WG_CANVASCAPSULEINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_infosection.h>
#include <wg_canvascapsule.h>

namespace wg
{
	class CanvasCapsuleInfoSection;
	typedef	StrongPtr<CanvasCapsuleInfoSection>	CanvasCapsuleInfoSection_p;
	typedef	WeakPtr<CanvasCapsuleInfoSection>	CanvasCapsuleInfoSection_wp;



	class CanvasCapsuleInfoSection : public InfoSection
	{
	public:

		//.____ Creation __________________________________________

		static CanvasCapsuleInfoSection_p		create(const DebugTheme& theme, IDebugContext* pContext, CanvasCapsule* pPanel) { return CanvasCapsuleInfoSection_p(new CanvasCapsuleInfoSection(theme, pContext, pPanel)); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		CanvasCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, CanvasCapsule* pPanel);
		~CanvasCapsuleInfoSection() {}

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
#endif //WG_CANVASCAPSULEINFOSECTION_DOT_H

