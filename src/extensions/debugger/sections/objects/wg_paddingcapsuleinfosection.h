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
#ifndef	WG_PADDINGCAPSULEINFOSECTION_DOT_H
#define WG_PADDINGCAPSULEINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_infosection.h>
#include <wg_paddingcapsule.h>

namespace wg
{
	class PaddingCapsuleInfoSection;
	typedef	StrongPtr<PaddingCapsuleInfoSection>	PaddingCapsuleInfoSection_p;
	typedef	WeakPtr<PaddingCapsuleInfoSection>	PaddingCapsuleInfoSection_wp;



	class PaddingCapsuleInfoSection : public InfoSection
	{
	public:

		//.____ Creation __________________________________________

		static PaddingCapsuleInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, PaddingCapsule * pPanel) { return PaddingCapsuleInfoSection_p(new PaddingCapsuleInfoSection(theme, pContext, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		PaddingCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, PaddingCapsule * pPanel );
		~PaddingCapsuleInfoSection() {}

		PaddingCapsule *	m_pInspected;
		DrawerPanel_p		m_pPaddingDrawer;

		Border				m_displayedPadding;
	};

} // namespace wg
#endif //WG_PADDINGCAPSULEINFOSECTION_DOT_H

