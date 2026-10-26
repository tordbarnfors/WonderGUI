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
#ifndef	WG_AREACHARTINFOSECTION_DOT_H
#define WG_AREACHARTINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_areachart.h>

namespace wg
{
	class AreaChartInfoSection;
	typedef	StrongPtr<AreaChartInfoSection>	AreaChartInfoSection_p;
	typedef	WeakPtr<AreaChartInfoSection>	AreaChartInfoSection_wp;



	class AreaChartInfoSection : public TypedInfoSection<AreaChart>
	{
	public:

		//.____ Creation __________________________________________

		static AreaChartInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, AreaChart * pInspected) { return AreaChartInfoSection_p(new AreaChartInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		AreaChartInfoSection(const DebugTheme& theme, IDebugContext* pContext, AreaChart * pInspected );
		~AreaChartInfoSection() {}
	};

} // namespace wg
#endif //WG_AREACHARTINFOSECTION_DOT_H
