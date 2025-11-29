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
#ifndef	WG_WIDGETINSPECTOR_DOT_H
#define WG_WIDGETINSPECTOR_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugwindow.h>

namespace wg
{
	class WidgetInspector;
	typedef	StrongPtr<WidgetInspector>	WidgetInspector_p;
	typedef	WeakPtr<WidgetInspector>	WidgetInspector_wp;



	class WidgetInspector : public DebugWindow
	{
	public:

		//.____ Creation __________________________________________

		static WidgetInspector_p		create( const Blueprint& blueprint, IDebugger * pHolder, Widget * pWidget) { return WidgetInspector_p(new WidgetInspector(blueprint, pHolder, pWidget) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		WidgetInspector(const Blueprint& blueprint, IDebugger * pHolder, Widget * pWidget );
		~WidgetInspector() {}

		Widget_p	m_pWidget;			// Keep our widget alive as long as we are, so refresh won't fail.

		ToggleButton_p	m_pAutoRefreshButton;
	};

} // namespace wg
#endif //WG_WIDGETINSPECTOR_DOT_H


