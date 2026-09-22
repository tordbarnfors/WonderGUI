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
#ifndef	WG_STATICTEXTINFOSECTION_DOT_H
#define WG_STATICTEXTINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_infosection.h>
#include <wg_statictext.h>

namespace wg
{
	class StaticTextInfoSection;
	typedef	StrongPtr<StaticTextInfoSection>	StaticTextInfoSection_p;
	typedef	WeakPtr<StaticTextInfoSection>	StaticTextInfoSection_wp;



	class StaticTextInfoSection : public InfoSection
	{
	public:

		//.____ Creation __________________________________________

		static StaticTextInfoSection_p		create(const DebugTheme& theme, IDebugContext* pContext, StaticText* pStaticText) { return StaticTextInfoSection_p(new StaticTextInfoSection(theme, pContext, pStaticText)); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		StaticTextInfoSection(const DebugTheme& theme, IDebugContext* pContext, StaticText* pStaticText);
		~StaticTextInfoSection() {}

		TablePanel_p	m_pTable;
		StaticText *	m_pInspected;
		TextDisplay_p	m_pTextDisplay;

		Object_p		m_displayedStylePtr;
		Object_p		m_displayedLayoutPtr;

	};

} // namespace wg
#endif //WG_StaticTextINFOSECTION_DOT_H

