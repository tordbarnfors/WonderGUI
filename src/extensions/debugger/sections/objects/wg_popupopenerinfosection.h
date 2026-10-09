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
#ifndef	WG_POPUPOPENERINFOSECTION_DOT_H
#define WG_POPUPOPENERINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_popupopener.h>

namespace wg
{
	class PopupOpenerInfoSection;
	typedef	StrongPtr<PopupOpenerInfoSection>	PopupOpenerInfoSection_p;
	typedef	WeakPtr<PopupOpenerInfoSection>	PopupOpenerInfoSection_wp;



	class PopupOpenerInfoSection : public TypedInfoSection<PopupOpener>
	{
	public:

		//.____ Creation __________________________________________

		static PopupOpenerInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, PopupOpener * pInspected) { return PopupOpenerInfoSection_p(new PopupOpenerInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		PopupOpenerInfoSection(const DebugTheme& theme, IDebugContext* pContext, PopupOpener * pInspected );
		~PopupOpenerInfoSection() {}

		DrawerPanel_p	m_pLabelDrawer;
		DrawerPanel_p	m_pIconDrawer;
	};

} // namespace wg
#endif //WG_POPUPOPENERINFOSECTION_DOT_H
