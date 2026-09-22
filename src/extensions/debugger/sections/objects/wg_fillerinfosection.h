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
#ifndef	WG_FILLERINFOSECTION_DOT_H
#define WG_FILLERINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_infosection.h>
#include <wg_filler.h>

namespace wg
{
	class FillerInfoSection;
	typedef	StrongPtr<FillerInfoSection>	FillerInfoSection_p;
	typedef	WeakPtr<FillerInfoSection>	FillerInfoSection_wp;



	class FillerInfoSection : public InfoSection
	{
	public:

		//.____ Creation __________________________________________

		static FillerInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Filler * pFiller) { return FillerInfoSection_p(new FillerInfoSection(theme, pContext, pFiller) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		FillerInfoSection(const DebugTheme& theme, IDebugContext* pContext, Filler * pFiller );
		~FillerInfoSection() {}

		Filler *		m_pInspected;
		TablePanel_p	m_pTable;

	};

} // namespace wg
#endif //WG_FILLERINFOSECTION_DOT_H

