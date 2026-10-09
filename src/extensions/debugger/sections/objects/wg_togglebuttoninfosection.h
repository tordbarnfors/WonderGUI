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
#ifndef	WG_TOGGLEBUTTONINFOSECTION_DOT_H
#define WG_TOGGLEBUTTONINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_togglebutton.h>

namespace wg
{
	class ToggleButtonInfoSection;
	typedef	StrongPtr<ToggleButtonInfoSection>	ToggleButtonInfoSection_p;
	typedef	WeakPtr<ToggleButtonInfoSection>	ToggleButtonInfoSection_wp;



	class ToggleButtonInfoSection : public TypedInfoSection<ToggleButton>
	{
	public:

		//.____ Creation __________________________________________

		static ToggleButtonInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, ToggleButton * pInspected) { return ToggleButtonInfoSection_p(new ToggleButtonInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		ToggleButtonInfoSection(const DebugTheme& theme, IDebugContext* pContext, ToggleButton * pInspected );
		~ToggleButtonInfoSection() {}

		DrawerPanel_p	m_pLabelDrawer;
		DrawerPanel_p	m_pIconDrawer;
	};

} // namespace wg
#endif //WG_TOGGLEBUTTONINFOSECTION_DOT_H
