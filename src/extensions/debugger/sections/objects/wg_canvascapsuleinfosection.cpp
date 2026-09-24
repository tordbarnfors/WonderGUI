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
#include "wg_canvascapsuleinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo CanvasCapsuleInfoSection::TYPEINFO = { "CanvasCapsuleInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	CanvasCapsuleInfoSection::CanvasCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, CanvasCapsule * pInspected)
		: TypedInfoSection<CanvasCapsule>( theme, pContext, CanvasCapsule::TYPEINFO.className, pInspected )
	{
		//TODO: Drawer with side display pointers
		//TODO: Transitions
		//TODO: Canvas geo

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			objectRow( "Canvas: ",                  [](CanvasCapsule* c) -> Object* { return c->canvas(); } ),
			objectRow( "Glow canvas: ",             [](CanvasCapsule* c) -> Object* { return c->glowCanvas(); } ),
			objectRow( "Layers: ",                  [](CanvasCapsule* c) -> Object* { return c->canvasLayers(); } ),
			objectRow( "Surface factory: ",         [](CanvasCapsule* c) -> Object* { return c->surfaceFactory(); } ),
			textRow  ( "Pixel format: ",            [](CanvasCapsule* c) { return toString(c->format()); } ),
			intRow   ( "Render layer: ",            [](CanvasCapsule* c) { return c->renderLayer(); } ),
			objectRow( "Tint: ",                    [](CanvasCapsule* c) -> Object* { return c->tint(); } ),
			textRow  ( "Blend mode: ",              [](CanvasCapsule* c) { return toString(c->blendMode()); } ),
			boolRow  ( "Scale canvas: ",            [](CanvasCapsule* c) { return c->isCanvasScaling(); } ),
			boolRow  ( "Skin around canvas: ",      [](CanvasCapsule* c) { return c->isSkinAroundCanvas(); } ),
			textRow  ( "Placement: ",               [](CanvasCapsule* c) { return toString(c->placement()); } ),
			intRow   ( "Number of side displays: ", [](CanvasCapsule* c) { return c->nbSideDisplays(); } )
		});

		m_displayedClearColor = pInspected->clearColor();
		m_displayedTintColor = pInspected->tintColor();

		m_pClearColorDrawer = _createColorDrawer("Clear color: ", m_displayedClearColor);
		m_pTintColorDrawer = _createColorDrawer("Tint color: ", m_displayedTintColor);
		m_pGlowDrawer = _createComponentDrawer("Glow", &pInspected->glow);

		pPanel->slots.pushBack({ pTable, m_pClearColorDrawer, m_pTintColorDrawer, m_pGlowDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& CanvasCapsuleInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void CanvasCapsuleInfoSection::refresh()
	{
		TypedInfoSection<CanvasCapsule>::refresh();

		_refreshColorDrawer(m_pClearColorDrawer, inspected()->clearColor(), m_displayedClearColor);
		_refreshColorDrawer(m_pTintColorDrawer, inspected()->tintColor(), m_displayedTintColor);
		_refreshComponentDrawer(m_pGlowDrawer);
	}

} // namespace wg
