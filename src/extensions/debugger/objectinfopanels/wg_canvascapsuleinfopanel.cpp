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
#include "wg_canvascapsuleinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo CanvasCapsuleInfoPanel::TYPEINFO = { "CanvasCapsuleInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	CanvasCapsuleInfoPanel::CanvasCapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, CanvasCapsule* pCapsule) : DebugPanel(blueprint, pHolder, CanvasCapsule::TYPEINFO.className)
	{
		//TODO: Drawer with side display pointers
		//TODO: Transitions
		//TODO: Canvas geo

		m_pInspected = pCapsule;

		auto pPanel = PackPanel::create(WGBP(PackPanel, _.axis = Axis::Y));
			
		m_pTable = _createTable(12, 2);

		int row = 0;
		_initObjectPointerEntry(m_pTable, row++, "Canvas: ");
		_initObjectPointerEntry(m_pTable, row++, "Glow canvas: ");
		_initObjectPointerEntry(m_pTable, row++, "Layers: ");
		_initObjectPointerEntry(m_pTable, row++, "Surface factory: ");
		_initTextEntry(m_pTable, row++, "Pixel format: ");
		_initIntegerEntry(m_pTable, row++, "Render layer: ");
		_initObjectPointerEntry(m_pTable, row++, "Tintmap: ");
		_initTextEntry(m_pTable, row++, "Blend mode: ");
		_initBoolEntry(m_pTable, row++, "Scale canvas: ");
		_initBoolEntry(m_pTable, row++, "Skin around canvas: ");
		_initTextEntry(m_pTable, row++, "Placement: ");
		_initIntegerEntry(m_pTable, row++, "Number of side displays: ");

		m_displayedClearColor = m_pInspected->clearColor();
		m_displayedTintColor = m_pInspected->tintColor();

		m_pClearColorDrawer = _createColorDrawer("Clear color: ", m_displayedClearColor);
		m_pTintColorDrawer = _createColorDrawer("Tint color: ", m_displayedTintColor);

		m_pGlowDrawer = _createComponentDrawer("Glow", &pCapsule->glow);

		pPanel->slots.pushBack({ m_pTable, m_pClearColorDrawer, m_pTintColorDrawer, m_pGlowDrawer });

		refresh();

		this->slot = pPanel;
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& CanvasCapsuleInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void CanvasCapsuleInfoPanel::refresh()
	{
		int row = 0;
		_refreshObjectPointerEntry(m_pTable, row++, m_pInspected->canvas(), m_displayedCanvasPtr );
		_refreshObjectPointerEntry(m_pTable, row++, m_pInspected->glowCanvas(), m_displayedGlowCanvasPtr);
		_refreshObjectPointerEntry(m_pTable, row++, m_pInspected->canvasLayers(), m_displayedLayersPtr);
		_refreshObjectPointerEntry(m_pTable, row++, m_pInspected->surfaceFactory(), m_displayedFactoryPtr);
		_refreshTextEntry(m_pTable, row++, toString(m_pInspected->format()));
		_refreshIntegerEntry(m_pTable, row++, m_pInspected->renderLayer());
		_refreshObjectPointerEntry(m_pTable, row++, m_pInspected->tintmap(), m_displayedTintmapPtr);
		_refreshTextEntry(m_pTable, row++, toString(m_pInspected->blendMode()));
		_refreshBoolEntry(m_pTable, row++, m_pInspected->isCanvasScaling());
		_refreshBoolEntry(m_pTable, row++, m_pInspected->isSkinAroundCanvas());
		_refreshTextEntry(m_pTable, row++, toString(m_pInspected->placement()) );
		_refreshIntegerEntry(m_pTable, row++, m_pInspected->nbSideDisplays() );

		m_displayedClearColor = m_pInspected->clearColor();
		m_displayedTintColor = m_pInspected->tintColor();

		m_pClearColorDrawer = _createColorDrawer("Clear color: ", m_displayedClearColor);
		m_pTintColorDrawer = _createColorDrawer("Tint color: ", m_displayedTintColor);

		_refreshComponentDrawer(m_pGlowDrawer);
	}

} // namespace wg


