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
#ifndef	WG_KNOBINFOSECTION_DOT_H
#define WG_KNOBINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_knob.h>

namespace wg
{
	class KnobInfoSection;
	typedef	StrongPtr<KnobInfoSection>	KnobInfoSection_p;
	typedef	WeakPtr<KnobInfoSection>	KnobInfoSection_wp;



	class KnobInfoSection : public TypedInfoSection<Knob>
	{
	public:

		//.____ Creation __________________________________________

		static KnobInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Knob * pInspected) { return KnobInfoSection_p(new KnobInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		KnobInfoSection(const DebugTheme& theme, IDebugContext* pContext, Knob * pInspected );
		~KnobInfoSection() {}
	};

} // namespace wg
#endif //WG_KNOBINFOSECTION_DOT_H
