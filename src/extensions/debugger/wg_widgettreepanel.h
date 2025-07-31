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
#include <wg_selectcapsule.h>
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

		static WidgetTreePanel_p		create( const Blueprint& blueprint, DebugPanel::Holder * pHolder, Widget * pRoot ) { return WidgetTreePanel_p(new WidgetTreePanel(blueprint, pHolder, pRoot) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _______________________________________________

		void	collapseAll();
		void	expandAll();

		void	select(Widget* pWidget);


	protected:
		WidgetTreePanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, Widget * pRoot );
		~WidgetTreePanel();

		Widget_p	_generateInfoTree( const Blueprint& blueprintWidget, Widget * pWidget, int indentation = 0);
		void		_expandOrCollapseRecursively(Widget* pWidget, bool bExpand);
		Widget_p	_findWidgetRecursively(int idToFind, Widget* pPos);

		Skin_p				m_pPaddingSkin;
		Skin_p				m_pDrawerButtonSkin;
		PackLayout_p		m_pPackLayout;

		SelectCapsule_p		m_pSelectCapsule;

		Widget_p			m_pSelectedWidget;

		RouteId				m_routeIdForSelect;

		std::vector<Widget_wp>	m_realWidgets;

	};

} // namespace wg
#endif //WG_WIDGETTREEPANEL_DOT_H


