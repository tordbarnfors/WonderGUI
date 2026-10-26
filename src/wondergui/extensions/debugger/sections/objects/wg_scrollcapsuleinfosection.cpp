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
#include "wg_scrollcapsuleinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo ScrollCapsuleInfoSection::TYPEINFO = { "ScrollCapsuleInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ScrollCapsuleInfoSection::ScrollCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, ScrollCapsule * pInspected)
		: TypedInfoSection<ScrollCapsule>( theme, pContext, ScrollCapsule::TYPEINFO.className, pInspected )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			ptsRow   ( "View X offset (pts): ",     [](ScrollCapsule* c) { return c->viewOffset().x; } ),
			ptsRow   ( "View Y offset (pts): ",     [](ScrollCapsule* c) { return c->viewOffset().y; } ),
			ptsRow   ( "View width (pts): ",        [](ScrollCapsule* c) { return c->viewSize().w; } ),
			ptsRow   ( "View height (pts): ",       [](ScrollCapsule* c) { return c->viewSize().h; } ),
			ptsRow   ( "Content width (pts): ",     [](ScrollCapsule* c) { return c->contentSize().w; } ),
			ptsRow   ( "Content height (pts): ",    [](ScrollCapsule* c) { return c->contentSize().h; } ),
			objectRow( "Default transition: ",      [](ScrollCapsule* c) -> Object* { return c->transition().rawPtr(); } ),
			boolRow  ( "Transitioning: ",           [](ScrollCapsule* c) { return c->isTransitioning(); } ),
			boolRow  ( "Scroll X: ",                [](ScrollCapsule* c) { return c->scrollX(); } ),
			boolRow  ( "Scroll Y: ",                [](ScrollCapsule* c) { return c->scrollY(); } ),
			boolRow  ( "Overlay scrollbars: ",      [](ScrollCapsule* c) { return c->overlayScrollbars(); } ),
			boolRow  ( "Autohide scrollbars: ",     [](ScrollCapsule* c) { return c->autoHideScrollbars(); } ),
			textRow  ( "Auto scroll axis: ",        [](ScrollCapsule* c) { return toString(c->autoScrollAxis()); } ),
			ptsRow   ( "Step size X (pts): ",       [](ScrollCapsule* c) { return c->stepSizeX(); } ),
			ptsRow   ( "Step size Y (pts): ",       [](ScrollCapsule* c) { return c->stepSizeY(); } ),
			ptsRow   ( "Page overlap X (pts): ",    [](ScrollCapsule* c) { return c->pageOverlapX(); } ),
			ptsRow   ( "Page overlap Y (pts): ",    [](ScrollCapsule* c) { return c->pageOverlapY(); } ),
			ptsRow   ( "Wheel step size X (pts): ", [](ScrollCapsule* c) { return c->wheelStepSizeX(); } ),
			ptsRow   ( "Wheel step size Y (pts): ", [](ScrollCapsule* c) { return c->wheelStepSizeY(); } ),
			textRow  ( "Wheel axis: ",              [](ScrollCapsule* c) { return toString(c->wheelAxis()); } ),
			textRow  ( "Wheel axis modifier: ",     [](ScrollCapsule* c) { return toString(c->wheelAxisModifier()); } ),
			textRow  ( "Wheel accelerator: ",       [](ScrollCapsule* c) { return toString(c->wheelAccelerator()); } ),
			intRow   ( "Wheel accel factor: ",      [](ScrollCapsule* c) { return c->wheelAccelFactor(); } ),
			boolRow  ( "Wheel follows scrollbar: ", [](ScrollCapsule* c) { return c->wheelFollowsScrollbar(); } ),
			objectRow( "Corner skin: ",             [](ScrollCapsule* c) -> Object* { return c->cornerSkin().rawPtr(); } )
		});

		m_pScrollbarXDrawer = _createComponentDrawer("Scrollbar X", &pInspected->scrollbarX);
		m_pScrollbarYDrawer = _createComponentDrawer("Scrollbar Y", &pInspected->scrollbarY);

		pPanel->slots.pushBack({ pTable, m_pScrollbarXDrawer, m_pScrollbarYDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScrollCapsuleInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ScrollCapsuleInfoSection::refresh()
	{
		TypedInfoSection<ScrollCapsule>::refresh();

		_refreshComponentDrawer(m_pScrollbarXDrawer);
		_refreshComponentDrawer(m_pScrollbarYDrawer);
	}

} // namespace wg
