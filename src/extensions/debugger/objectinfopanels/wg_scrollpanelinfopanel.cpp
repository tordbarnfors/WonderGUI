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
#include "wg_scrollpanelinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo ScrollPanelInfoPanel::TYPEINFO = { "ScrollPanelInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	ScrollPanelInfoPanel::ScrollPanelInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, ScrollPanel * pPanel) : DebugPanel( blueprint, pHolder, ScrollPanel::TYPEINFO.className )
	{
		m_pInspected = pPanel;
		m_pTable = _createTable(26,2);

		auto pContainer = WGCREATE(PackPanel, _.axis = Axis::Y);

		_initTextEntry(m_pTable, 0, "Child placement: ");
		_initTextEntry(m_pTable, 1, "Width constraint: ");
		_initTextEntry(m_pTable, 2, "Height constraint: ");
		_initBoolEntry(m_pTable, 3, "Overlay scrollbar X: ");
		_initBoolEntry(m_pTable, 4, "Overlay scrollbar Y: ");
		_initBoolEntry(m_pTable, 5, "Autohide scrollbar X: ");
		_initBoolEntry(m_pTable, 6, "Autohide scrollbar Y: ");
		_initPtsEntry(m_pTable, 7, "View X offset (pts): ");
		_initPtsEntry(m_pTable, 8, "View Y offset (pts): ");
		_initPtsEntry(m_pTable, 9, "View width (pts): ");
		_initPtsEntry(m_pTable, 10, "View height (pts): ");
		_initPtsEntry(m_pTable, 11, "Content width (pts): ");
		_initPtsEntry(m_pTable, 12, "Content height (pts): ");
		_initPtsEntry(m_pTable, 13, "Step size X (pts): ");
		_initPtsEntry(m_pTable, 14, "Step size Y (pts): ");
		_initPtsEntry(m_pTable, 15, "Wheel step size X (pts): ");
		_initPtsEntry(m_pTable, 16, "Wheel step size Y (pts): ");
		_initPtsEntry(m_pTable, 17, "Page overlap X (pts): ");
		_initPtsEntry(m_pTable, 18, "Page overlap Y (pts): ");
		_initTextEntry(m_pTable, 19, "Wheel one axis: ");
		_initTextEntry(m_pTable, 20, "Wheel two axis: ");
		_initTextEntry(m_pTable, 21, "Wheel axis shift combo: ");
		_initBoolEntry(m_pTable, 22, "Steal wheel from scrollbars: ");
		_initBoolEntry(m_pTable, 23, "Auto scroll X: ");
		_initBoolEntry(m_pTable, 24, "Auto scroll Y: ");
		_initObjectPointerEntry(m_pTable, 25, "Default transition: ");

		m_pScrollbarXDrawer = _createComponentDrawer("Scrollbar X", &m_pInspected->scrollbarX);
		m_pScrollbarYDrawer = _createComponentDrawer("Scrollbar Y", &m_pInspected->scrollbarY);

		m_pSlotDrawer = _createSingleSlotDrawer("Slot", &m_pInspected->slot);

		pContainer->slots.pushBack( {
			m_pTable,
			m_pScrollbarXDrawer,
			m_pScrollbarYDrawer,
			m_pSlotDrawer
			});

		refresh();

		this->slot = pContainer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScrollPanelInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ScrollPanelInfoPanel::refresh()
	{
		_refreshTextEntry(m_pTable, 0, toString(m_pInspected->childPlacement() ));
		_refreshTextEntry(m_pTable, 1, toString(m_pInspected->heightConstraint() ));
		_refreshTextEntry(m_pTable, 2, toString(m_pInspected->widthConstraint()));
		_refreshBoolEntry(m_pTable, 3, m_pInspected->overlayScrollbarX());
		_refreshBoolEntry(m_pTable, 4, m_pInspected->overlayScrollbarY());
		_refreshBoolEntry(m_pTable, 5, m_pInspected->autohideScrollbarX());
		_refreshBoolEntry(m_pTable, 6, m_pInspected->autohideScrollbarY());

		Coord	viewOffset = m_pInspected->viewOffset();
		Size	viewSize = m_pInspected->viewSize();
		Size	contentSize = m_pInspected->contentSize();

		_refreshPtsEntry(m_pTable, 7, viewOffset.x);
		_refreshPtsEntry(m_pTable, 8, viewOffset.y);
		_refreshPtsEntry(m_pTable, 9, viewSize.w);
		_refreshPtsEntry(m_pTable, 10, viewSize.h);
		_refreshPtsEntry(m_pTable, 11, contentSize.w);
		_refreshPtsEntry(m_pTable, 12, contentSize.h);
		_refreshPtsEntry(m_pTable, 13, m_pInspected->stepSizeX());
		_refreshPtsEntry(m_pTable, 14, m_pInspected->stepSizeY());
		_refreshPtsEntry(m_pTable, 15, m_pInspected->wheelStepSizeX());
		_refreshPtsEntry(m_pTable, 16, m_pInspected->wheelStepSizeY());
		_refreshPtsEntry(m_pTable, 17, m_pInspected->pageOverlapX());
		_refreshPtsEntry(m_pTable, 18, m_pInspected->pageOverlapY());
		_refreshTextEntry(m_pTable, 19, toString(m_pInspected->wheelOneAxis()) );
		_refreshTextEntry(m_pTable, 20, toString(m_pInspected->wheelTwoAxis()));
		_refreshTextEntry(m_pTable, 21, toString(m_pInspected->wheelAxisShift()));
		_refreshBoolEntry(m_pTable, 22, m_pInspected->stealWheelFromScrollbars());
		_refreshBoolEntry(m_pTable, 23, m_pInspected->autoscrollX());
		_refreshBoolEntry(m_pTable, 24, m_pInspected->autoscrollY());
		_refreshObjectPointerEntry(m_pTable, 25, m_pInspected->transition(), m_displayedTransitionPtr );

		_refreshComponentDrawer(m_pScrollbarXDrawer);
		_refreshComponentDrawer(m_pScrollbarYDrawer);

		_refreshSingleSlotDrawer(m_pSlotDrawer, &m_pInspected->slot);
	}

} // namespace wg


