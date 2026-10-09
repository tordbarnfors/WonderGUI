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
#ifndef	WG_SCROLLBARINFOSECTION_DOT_H
#define WG_SCROLLBARINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_scrollbar.h>

namespace wg
{
	class ScrollbarInfoSection;
	typedef	StrongPtr<ScrollbarInfoSection>	ScrollbarInfoSection_p;
	typedef	WeakPtr<ScrollbarInfoSection>	ScrollbarInfoSection_wp;



	class ScrollbarInfoSection : public TypedInfoSection<Scrollbar>
	{
	public:

		//.____ Creation __________________________________________

		static ScrollbarInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Scrollbar * pInspected) { return ScrollbarInfoSection_p(new ScrollbarInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		ScrollbarInfoSection(const DebugTheme& theme, IDebugContext* pContext, Scrollbar * pInspected );
		~ScrollbarInfoSection() {}

		DrawerPanel_p	m_pScrollbarDrawer;
	};

} // namespace wg
#endif //WG_SCROLLBARINFOSECTION_DOT_H
