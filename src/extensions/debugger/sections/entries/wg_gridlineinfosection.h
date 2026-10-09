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
#ifndef	WG_GRIDLINEINFOSECTION_DOT_H
#define WG_GRIDLINEINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_chart.h>

namespace wg
{
	class GridLineInfoSection;
	typedef	StrongPtr<GridLineInfoSection>	GridLineInfoSection_p;
	typedef	WeakPtr<GridLineInfoSection>	GridLineInfoSection_wp;


	//____ GridLineInfoSection ___________________________________________________
	//
	// GridLine is an entry in a Chart's xLines/yLines, not an Object, so this
	// section isn't registered in the backend. ChartInfoSection creates one per
	// line and points it at a new GridLine with setInspected() when the vector
	// has changed.

	class GridLineInfoSection : public TypedInfoSection<GridLine>
	{
	public:

		//.____ Creation __________________________________________

		static GridLineInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, GridLine * pInspected) { return GridLineInfoSection_p(new GridLineInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		GridLineInfoSection(const DebugTheme& theme, IDebugContext* pContext, GridLine * pInspected );
		~GridLineInfoSection() {}

		DrawerPanel_p	m_pColorDrawer;
		HiColor			m_displayedColor;
	};

} // namespace wg
#endif //WG_GRIDLINEINFOSECTION_DOT_H
