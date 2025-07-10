/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef	WG_WIDGETTREEPANEL_DOT_H
#define WG_WIDGETTREEPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_transitions.h>

namespace wg
{
	class WidgetTreePanel;
	typedef	StrongPtr<WidgetTreePanel>	WidgetTreePanel_p;
	typedef	WeakPtr<WidgetTreePanel>	WidgetTreePanel_wp;



	class WidgetTreePanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static WidgetTreePanel_p		create( const Blueprint& blueprint, Widget * pRoot, const std::function<void(Widget* pSelected)>& selectCallback ) { return WidgetTreePanel_p(new WidgetTreePanel(blueprint, pRoot, selectCallback) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _______________________________________________

		void	collapseAll();
		void	expandAll();




	protected:
		WidgetTreePanel(const Blueprint& blueprint, Widget * pRoot, const std::function<void(Widget* pSelected)>& selectCallback );
		~WidgetTreePanel() {}

		Widget_p	_generateInfoTree( const Blueprint& blueprintWidget, Widget * pWidget, int indentation = 0);
		void		_expandOrCollapseRecursively(Widget* pWidget, bool bExpand);

		Skin_p				m_pPaddingSkin;
		PackLayout_p		m_pPackLayout;
		ValueTransition_p	m_pTransition;

		std::function<void(Widget* pSelected)> m_selectCallback;
		std::vector<Widget_wp>	m_realWidgets;

	};

} // namespace wg
#endif //WG_WIDGETTREEPANEL_DOT_H


