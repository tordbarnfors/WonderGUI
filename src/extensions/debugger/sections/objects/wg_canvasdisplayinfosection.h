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
#ifndef	WG_CANVASDISPLAYINFOSECTION_DOT_H
#define WG_CANVASDISPLAYINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_canvascapsule.h>
#include <wg_canvasdisplay.h>

namespace wg
{
	class CanvasDisplayInfoSection;
	typedef	StrongPtr<CanvasDisplayInfoSection>	CanvasDisplayInfoSection_p;
	typedef	WeakPtr<CanvasDisplayInfoSection>	CanvasDisplayInfoSection_wp;



	class CanvasDisplayInfoSection : public TypedInfoSection<CanvasDisplay>
	{
	public:

		//.____ Creation __________________________________________

		static CanvasDisplayInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, CanvasDisplay * pInspected) { return CanvasDisplayInfoSection_p(new CanvasDisplayInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		CanvasDisplayInfoSection(const DebugTheme& theme, IDebugContext* pContext, CanvasDisplay * pInspected );
		~CanvasDisplayInfoSection() {}
	};

} // namespace wg
#endif //WG_CANVASDISPLAYINFOSECTION_DOT_H
