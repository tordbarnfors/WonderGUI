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
#include "wg_glowinfosection.h"
#include <wg_packpanel.h>

namespace wg
{

	const TypeInfo GlowInfoSection::TYPEINFO = { "GlowInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	GlowInfoSection::GlowInfoSection(const DebugTheme& theme, IDebugContext* pContext, Glow * pInspected)
		: TypedInfoSection<Glow>( theme, pContext, Glow::TYPEINFO.className, pInspected )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			boolRow  ( "Active: ",                 [](Glow* g) { return g->isActive(); } ),
			objectRow( "Blurbrush: ",              [](Glow* g) -> Object* { return g->blurbrush(); } ),
			intRow   ( "Refresh rate (fps): ",     [](Glow* g) { return g->refreshRate(); } ),
			intRow   ( "Resolution X (pixels): ",  [](Glow* g) { return g->pixelResolution().w; } ),
			intRow   ( "Resolution Y (pixels): ",  [](Glow* g) { return g->pixelResolution().h; } ),
			textRow  ( "Resize placement: ",       [](Glow* g) { return toString(g->resizePlacement()); } ),
			boolRow  ( "Stretch on resize: ",      [](Glow* g) { return g->stretchOnResize(); } ),
			boolRow  ( "Clear on resize: ",        [](Glow* g) { return g->clearOnResize(); } ),
			objectRow( "Surface 1: ",              [](Glow* g) -> Object* { return g->m_surface[0]; } ),
			objectRow( "Surface 2: ",              [](Glow* g) -> Object* { return g->m_surface[1]; } ),
			textRow  ( "Seed blend: ",             [](Glow* g) { return toString(g->seedBlend()); } ),
			textRow  ( "Glow blend: ",             [](Glow* g) { return toString(g->glowBlend()); } )
		});

		m_displayedGlowTint = pInspected->glowTint();
		m_displayedSeedTint = pInspected->seedTint();

		m_pGlowTintDrawer = _createColorDrawer("Glow tint: ", m_displayedGlowTint);
		m_pSeedTintDrawer = _createColorDrawer("Seed tint: ", m_displayedSeedTint);

		pPanel->slots.pushBack({ pTable, m_pGlowTintDrawer, m_pSeedTintDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GlowInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void GlowInfoSection::refresh()
	{
		TypedInfoSection<Glow>::refresh();

		_refreshColorDrawer(m_pGlowTintDrawer, inspected()->glowTint(), m_displayedGlowTint);
		_refreshColorDrawer(m_pSeedTintDrawer, inspected()->seedTint(), m_displayedSeedTint);
	}

} // namespace wg
