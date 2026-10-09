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
#ifndef	WG_METERINFOSECTION_DOT_H
#define WG_METERINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_meter.h>

namespace wg
{
	class MeterInfoSection;
	typedef	StrongPtr<MeterInfoSection>	MeterInfoSection_p;
	typedef	WeakPtr<MeterInfoSection>	MeterInfoSection_wp;



	class MeterInfoSection : public TypedInfoSection<Meter>
	{
	public:

		//.____ Creation __________________________________________

		static MeterInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Meter * pInspected) { return MeterInfoSection_p(new MeterInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		MeterInfoSection(const DebugTheme& theme, IDebugContext* pContext, Meter * pInspected );
		~MeterInfoSection() {}
	};

} // namespace wg
#endif //WG_METERINFOSECTION_DOT_H
