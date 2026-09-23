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
#ifndef	WG_CONTAINERINFOSECTION_DOT_H
#define WG_CONTAINERINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_container.h>

namespace wg
{
	class ContainerInfoSection;
	typedef	StrongPtr<ContainerInfoSection>	ContainerInfoSection_p;
	typedef	WeakPtr<ContainerInfoSection>	ContainerInfoSection_wp;



	class ContainerInfoSection : public TypedInfoSection<Container>
	{
	public:

		//.____ Creation __________________________________________

		static ContainerInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Container * pInspected) { return ContainerInfoSection_p(new ContainerInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		ContainerInfoSection(const DebugTheme& theme, IDebugContext* pContext, Container * pInspected );
		~ContainerInfoSection() {}
	};

} // namespace wg
#endif //WG_CONTAINERINFOSECTION_DOT_H
