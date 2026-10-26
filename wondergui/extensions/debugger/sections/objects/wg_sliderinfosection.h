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
#ifndef	WG_SLIDERINFOSECTION_DOT_H
#define WG_SLIDERINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_slider.h>

namespace wg
{
	class SliderInfoSection;
	typedef	StrongPtr<SliderInfoSection>	SliderInfoSection_p;
	typedef	WeakPtr<SliderInfoSection>	SliderInfoSection_wp;



	class SliderInfoSection : public TypedInfoSection<Slider>
	{
	public:

		//.____ Creation __________________________________________

		static SliderInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Slider * pInspected) { return SliderInfoSection_p(new SliderInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		SliderInfoSection(const DebugTheme& theme, IDebugContext* pContext, Slider * pInspected );
		~SliderInfoSection() {}
	};

} // namespace wg
#endif //WG_SLIDERINFOSECTION_DOT_H
