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
#ifndef	WG_SURFACEDISPLAYINFOSECTION_DOT_H
#define WG_SURFACEDISPLAYINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_surfacedisplay.h>

namespace wg
{
	class SurfaceDisplayInfoSection;
	typedef	StrongPtr<SurfaceDisplayInfoSection>	SurfaceDisplayInfoSection_p;
	typedef	WeakPtr<SurfaceDisplayInfoSection>	SurfaceDisplayInfoSection_wp;



	class SurfaceDisplayInfoSection : public TypedInfoSection<SurfaceDisplay>
	{
	public:

		//.____ Creation __________________________________________

		static SurfaceDisplayInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, SurfaceDisplay * pInspected) { return SurfaceDisplayInfoSection_p(new SurfaceDisplayInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		SurfaceDisplayInfoSection(const DebugTheme& theme, IDebugContext* pContext, SurfaceDisplay * pInspected );
		~SurfaceDisplayInfoSection() {}
	};

} // namespace wg
#endif //WG_SURFACEDISPLAYINFOSECTION_DOT_H
