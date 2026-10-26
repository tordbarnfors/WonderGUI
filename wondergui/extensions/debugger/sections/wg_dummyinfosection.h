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
#ifndef	WG_DUMMYINFOSECTION_DOT_H
#define WG_DUMMYINFOSECTION_DOT_H
#pragma once

#include <wg_infosection.h>

namespace wg
{
	class DummyInfoSection;
	typedef	StrongPtr<DummyInfoSection>	DummyInfoSection_p;
	typedef	WeakPtr<DummyInfoSection>	DummyInfoSection_wp;



	class DummyInfoSection : public InfoSection
	{
	public:

		//.____ Creation __________________________________________

		static DummyInfoSection_p		create( const DebugTheme& theme, IDebugContext * pContext, const char * pLabel, void * pDummy) { return DummyInfoSection_p(new DummyInfoSection(theme, pContext, pLabel, pDummy) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		DummyInfoSection(const DebugTheme& theme, IDebugContext * pContext, const char * pLabel, void * pDummy );
		~DummyInfoSection() {}
	};

} // namespace wg
#endif //WG_DUMMYINFOSECTION_DOT_H


