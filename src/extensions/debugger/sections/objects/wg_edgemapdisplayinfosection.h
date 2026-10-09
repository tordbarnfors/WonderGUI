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
#ifndef	WG_EDGEMAPDISPLAYINFOSECTION_DOT_H
#define WG_EDGEMAPDISPLAYINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_edgemapdisplay.h>

namespace wg
{
	class EdgemapDisplayInfoSection;
	typedef	StrongPtr<EdgemapDisplayInfoSection>	EdgemapDisplayInfoSection_p;
	typedef	WeakPtr<EdgemapDisplayInfoSection>	EdgemapDisplayInfoSection_wp;



	class EdgemapDisplayInfoSection : public TypedInfoSection<EdgemapDisplay>
	{
	public:

		//.____ Creation __________________________________________

		static EdgemapDisplayInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, EdgemapDisplay * pInspected) { return EdgemapDisplayInfoSection_p(new EdgemapDisplayInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		EdgemapDisplayInfoSection(const DebugTheme& theme, IDebugContext* pContext, EdgemapDisplay * pInspected );
		~EdgemapDisplayInfoSection() {}
	};

} // namespace wg
#endif //WG_EDGEMAPDISPLAYINFOSECTION_DOT_H
