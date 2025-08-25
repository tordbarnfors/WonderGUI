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
#include "wg_glowinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_enumextras.h>
#include <wg_packpanel.h>

namespace wg
{

	const TypeInfo GlowInfoPanel::TYPEINFO = { "GlowInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	GlowInfoPanel::GlowInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Glow* pGlow) : DebugPanel(blueprint, pHolder, Glow::TYPEINFO.className)
	{
		m_pInspected = pGlow;

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		m_pTable = _createTable(12, 2);

		int row = 0;
		_initBoolEntry(m_pTable, row++, "Active: ");
		_initObjectPointerEntry(m_pTable, row++, "Blurbrush: ");
		_initIntegerEntry(m_pTable, row++, "Refresh rate (fps): ");
		_initIntegerEntry(m_pTable, row++, "Resolution X (pixels): ");
		_initIntegerEntry(m_pTable, row++, "Resolution Y (pixels): ");
		_initTextEntry(m_pTable, row++, "Resize placement: ");
		_initBoolEntry(m_pTable, row++, "Stretch on resize: ");
		_initBoolEntry(m_pTable, row++, "Clear on resize: ");
		_initObjectPointerEntry(m_pTable, row++, "Surface 1: ");
		_initObjectPointerEntry(m_pTable, row++, "Surface 2: ");
		_initTextEntry(m_pTable, row++, "Seed blend: ");
		_initTextEntry(m_pTable, row++, "Glow blend: ");

		m_displayedGlowTint = pGlow->glowTint();
		m_displayedSeedTint = pGlow->seedTint();

		m_pGlowTintDrawer = _createColorDrawer( "Glow tint: ", pGlow->glowTint() );
		m_pSeedTintDrawer = _createColorDrawer( "Seed tint: ", pGlow->seedTint());

		pPanel->slots.pushBack({ m_pTable, m_pGlowTintDrawer, m_pSeedTintDrawer });
		this->slot = pPanel;

		refresh();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GlowInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void GlowInfoPanel::refresh()
	{
		int row = 0;
		_refreshBoolEntry(m_pTable, row++, m_pInspected->isActive());
		_refreshObjectPointerEntry(m_pTable, row++, m_pInspected->blurbrush(), m_displayedBrushPtr);
		_refreshIntegerEntry(m_pTable, row++, m_pInspected->refreshRate());
		_refreshIntegerEntry(m_pTable, row++, m_pInspected->pixelResolution().w);
		_refreshIntegerEntry(m_pTable, row++, m_pInspected->pixelResolution().h);
		_refreshTextEntry(m_pTable, row++, toString(m_pInspected->resizePlacement()));
		_refreshBoolEntry(m_pTable, row++, m_pInspected->stretchOnResize());
		_refreshBoolEntry(m_pTable, row++, m_pInspected->clearOnResize());
		_refreshObjectPointerEntry(m_pTable, row++, m_pInspected->m_surface[0], m_displayedSurface1Ptr);
		_refreshObjectPointerEntry(m_pTable, row++, m_pInspected->m_surface[1], m_displayedSurface2Ptr);
		_refreshTextEntry(m_pTable, row++, toString(m_pInspected->seedBlend()));
		_refreshTextEntry(m_pTable, row++, toString(m_pInspected->glowBlend()));

		_refreshColorDrawer(m_pGlowTintDrawer, m_pInspected->glowTint(), m_displayedGlowTint);
		_refreshColorDrawer(m_pSeedTintDrawer, m_pInspected->seedTint(), m_displayedSeedTint);
	}



} // namespace wg


