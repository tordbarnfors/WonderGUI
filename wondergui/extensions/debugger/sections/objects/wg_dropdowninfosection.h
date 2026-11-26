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
#ifndef	WG_DROPDOWNINFOSECTION_DOT_H
#define WG_DROPDOWNINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_dropdown.h>

namespace wg
{
	class DropdownInfoSection;
	typedef	StrongPtr<DropdownInfoSection>	DropdownInfoSection_p;
	typedef	WeakPtr<DropdownInfoSection>	DropdownInfoSection_wp;



	class DropdownInfoSection : public TypedInfoSection<Dropdown>
	{
	public:

		//.____ Creation __________________________________________

		static DropdownInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Dropdown * pInspected) { return DropdownInfoSection_p(new DropdownInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		DropdownInfoSection(const DebugTheme& theme, IDebugContext* pContext, Dropdown * pInspected );
		~DropdownInfoSection() {}

		ExpanderPanel_p	m_pDisplayDrawer;
	};

} // namespace wg
#endif //WG_DROPDOWNINFOSECTION_DOT_H
