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
#ifndef	WG_FPSDISPLAYINFOSECTION_DOT_H
#define WG_FPSDISPLAYINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_fpsdisplay.h>

namespace wg
{
	class FpsDisplayInfoSection;
	typedef	StrongPtr<FpsDisplayInfoSection>	FpsDisplayInfoSection_p;
	typedef	WeakPtr<FpsDisplayInfoSection>	FpsDisplayInfoSection_wp;



	class FpsDisplayInfoSection : public TypedInfoSection<FpsDisplay>
	{
	public:

		//.____ Creation __________________________________________

		static FpsDisplayInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, FpsDisplay * pInspected) { return FpsDisplayInfoSection_p(new FpsDisplayInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		FpsDisplayInfoSection(const DebugTheme& theme, IDebugContext* pContext, FpsDisplay * pInspected );
		~FpsDisplayInfoSection() {}

		DrawerPanel_p	m_pLabelsDrawer;
		DrawerPanel_p	m_pValuesDrawer;
	};

} // namespace wg
#endif //WG_FPSDISPLAYINFOSECTION_DOT_H
