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
#ifndef	WG_SCROLLCHARTINFOSECTION_DOT_H
#define WG_SCROLLCHARTINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_scrollchart.h>

namespace wg
{
	class ScrollChartInfoSection;
	typedef	StrongPtr<ScrollChartInfoSection>	ScrollChartInfoSection_p;
	typedef	WeakPtr<ScrollChartInfoSection>	ScrollChartInfoSection_wp;



	class ScrollChartInfoSection : public TypedInfoSection<ScrollChart>
	{
	public:

		//.____ Creation __________________________________________

		static ScrollChartInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, ScrollChart * pInspected) { return ScrollChartInfoSection_p(new ScrollChartInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		ScrollChartInfoSection(const DebugTheme& theme, IDebugContext* pContext, ScrollChart * pInspected );
		~ScrollChartInfoSection() {}
	};

} // namespace wg
#endif //WG_SCROLLCHARTINFOSECTION_DOT_H
