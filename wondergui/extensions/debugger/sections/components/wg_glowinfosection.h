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
#ifndef	WG_GLOWINFOSECTION_DOT_H
#define WG_GLOWINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_glow.h>

namespace wg
{
	class GlowInfoSection;
	typedef	StrongPtr<GlowInfoSection>	GlowInfoSection_p;
	typedef	WeakPtr<GlowInfoSection>	GlowInfoSection_wp;



	class GlowInfoSection : public TypedInfoSection<Glow>
	{
	public:

		//.____ Creation __________________________________________

		static GlowInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Glow * pInspected) { return GlowInfoSection_p(new GlowInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		GlowInfoSection(const DebugTheme& theme, IDebugContext* pContext, Glow * pInspected );
		~GlowInfoSection() {}

		DrawerPanel_p	m_pGlowTintDrawer;
		DrawerPanel_p	m_pSeedTintDrawer;

		HiColor			m_displayedGlowTint;
		HiColor			m_displayedSeedTint;
	};

} // namespace wg
#endif //WG_GLOWINFOSECTION_DOT_H
