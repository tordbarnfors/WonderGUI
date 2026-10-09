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
#ifndef	WG_CHARTINFOSECTION_DOT_H
#define WG_CHARTINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_chart.h>

namespace wg
{
	class ChartInfoSection;
	typedef	StrongPtr<ChartInfoSection>	ChartInfoSection_p;
	typedef	WeakPtr<ChartInfoSection>	ChartInfoSection_wp;



	class ChartInfoSection : public TypedInfoSection<Chart>
	{
	public:

		//.____ Creation __________________________________________

		static ChartInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Chart * pInspected) { return ChartInfoSection_p(new ChartInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		ChartInfoSection(const DebugTheme& theme, IDebugContext* pContext, Chart * pInspected );
		~ChartInfoSection() {}

		DrawerPanel_p	m_pGlowDrawer;
	};

} // namespace wg
#endif //WG_CHARTINFOSECTION_DOT_H
