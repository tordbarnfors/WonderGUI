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
#ifndef	WG_ICONINFOSECTION_DOT_H
#define WG_ICONINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_icon.h>

namespace wg
{
	class IconInfoSection;
	typedef	StrongPtr<IconInfoSection>	IconInfoSection_p;
	typedef	WeakPtr<IconInfoSection>	IconInfoSection_wp;



	class IconInfoSection : public TypedInfoSection<Icon>
	{
	public:

		//.____ Creation __________________________________________

		static IconInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Icon * pInspected) { return IconInfoSection_p(new IconInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		IconInfoSection(const DebugTheme& theme, IDebugContext* pContext, Icon * pInspected );
		~IconInfoSection() {}
	};

} // namespace wg
#endif //WG_ICONINFOSECTION_DOT_H
