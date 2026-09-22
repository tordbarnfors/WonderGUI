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
#include "wg_scrollpanelinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo ScrollPanelInfoSection::TYPEINFO = { "ScrollPanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ScrollPanelInfoSection::ScrollPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, ScrollPanel * pInspected)
		: TypedInfoSection<ScrollPanel>( theme, pContext, ScrollPanel::TYPEINFO.className, pInspected )
	{
		auto pContainer = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow  ( "Child placement: ",              [](ScrollPanel* p) { return toString(p->childPlacement()); } ),
			textRow  ( "Width constraint: ",             [](ScrollPanel* p) { return toString(p->widthConstraint()); } ),
			textRow  ( "Height constraint: ",            [](ScrollPanel* p) { return toString(p->heightConstraint()); } ),
			boolRow  ( "Overlay scrollbar X: ",          [](ScrollPanel* p) { return p->overlayScrollbarX(); } ),
			boolRow  ( "Overlay scrollbar Y: ",          [](ScrollPanel* p) { return p->overlayScrollbarY(); } ),
			boolRow  ( "Autohide scrollbar X: ",         [](ScrollPanel* p) { return p->autohideScrollbarX(); } ),
			boolRow  ( "Autohide scrollbar Y: ",         [](ScrollPanel* p) { return p->autohideScrollbarY(); } ),
			ptsRow   ( "View X offset (pts): ",          [](ScrollPanel* p) { return p->viewOffset().x; } ),
			ptsRow   ( "View Y offset (pts): ",          [](ScrollPanel* p) { return p->viewOffset().y; } ),
			ptsRow   ( "View width (pts): ",             [](ScrollPanel* p) { return p->viewSize().w; } ),
			ptsRow   ( "View height (pts): ",            [](ScrollPanel* p) { return p->viewSize().h; } ),
			ptsRow   ( "Content width (pts): ",          [](ScrollPanel* p) { return p->contentSize().w; } ),
			ptsRow   ( "Content height (pts): ",         [](ScrollPanel* p) { return p->contentSize().h; } ),
			ptsRow   ( "Step size X (pts): ",            [](ScrollPanel* p) { return p->stepSizeX(); } ),
			ptsRow   ( "Step size Y (pts): ",            [](ScrollPanel* p) { return p->stepSizeY(); } ),
			ptsRow   ( "Wheel step size X (pts): ",      [](ScrollPanel* p) { return p->wheelStepSizeX(); } ),
			ptsRow   ( "Wheel step size Y (pts): ",      [](ScrollPanel* p) { return p->wheelStepSizeY(); } ),
			ptsRow   ( "Page overlap X (pts): ",         [](ScrollPanel* p) { return p->pageOverlapX(); } ),
			ptsRow   ( "Page overlap Y (pts): ",         [](ScrollPanel* p) { return p->pageOverlapY(); } ),
			textRow  ( "Wheel one axis: ",               [](ScrollPanel* p) { return toString(p->wheelOneAxis()); } ),
			textRow  ( "Wheel two axis: ",               [](ScrollPanel* p) { return toString(p->wheelTwoAxis()); } ),
			textRow  ( "Wheel axis shift combo: ",       [](ScrollPanel* p) { return toString(p->wheelAxisShift()); } ),
			boolRow  ( "Steal wheel from scrollbars: ",  [](ScrollPanel* p) { return p->stealWheelFromScrollbars(); } ),
			boolRow  ( "Auto scroll X: ",                [](ScrollPanel* p) { return p->autoscrollX(); } ),
			boolRow  ( "Auto scroll Y: ",                [](ScrollPanel* p) { return p->autoscrollY(); } ),
			objectRow( "Default transition: ",           [](ScrollPanel* p) -> Object* { return p->transition(); } )
		});

		m_pScrollbarXDrawer = _createComponentDrawer("Scrollbar X", &pInspected->scrollbarX);
		m_pScrollbarYDrawer = _createComponentDrawer("Scrollbar Y", &pInspected->scrollbarY);
		m_pSlotDrawer = _createSingleSlotDrawer("Slot", &pInspected->slot);

		pContainer->slots.pushBack({ pTable, m_pScrollbarXDrawer, m_pScrollbarYDrawer, m_pSlotDrawer });
		this->slot = pContainer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScrollPanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ScrollPanelInfoSection::refresh()
	{
		TypedInfoSection<ScrollPanel>::refresh();

		_refreshComponentDrawer(m_pScrollbarXDrawer);
		_refreshComponentDrawer(m_pScrollbarYDrawer);
		_refreshSingleSlotDrawer(m_pSlotDrawer, &inspected()->slot);
	}

} // namespace wg
