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
#ifndef	WG_SCALECAPSULEINFOSECTION_DOT_H
#define WG_SCALECAPSULEINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_infosection.h>
#include <wg_scalecapsule.h>

namespace wg
{
	class ScaleCapsuleInfoSection;
	typedef	StrongPtr<ScaleCapsuleInfoSection>	ScaleCapsuleInfoSection_p;
	typedef	WeakPtr<ScaleCapsuleInfoSection>	ScaleCapsuleInfoSection_wp;



	class ScaleCapsuleInfoSection : public InfoSection
	{
	public:

		//.____ Creation __________________________________________

		static ScaleCapsuleInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, ScaleCapsule * pPanel) { return ScaleCapsuleInfoSection_p(new ScaleCapsuleInfoSection(theme, pContext, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		ScaleCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, ScaleCapsule * pPanel );
		~ScaleCapsuleInfoSection() {}

		ScaleCapsule *	m_pInspected;
		TablePanel_p	m_pTable;
	};

} // namespace wg
#endif //WG_SCALECAPSULEINFOSECTION_DOT_H

