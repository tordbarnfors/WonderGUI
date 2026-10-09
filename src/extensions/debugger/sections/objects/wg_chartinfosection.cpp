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
#include "wg_chartinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo ChartInfoSection::TYPEINFO = { "ChartInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ChartInfoSection::ChartInfoSection(const DebugTheme& theme, IDebugContext* pContext, Chart * pInspected)
		: TypedInfoSection<Chart>( theme, pContext, Chart::TYPEINFO.className, pInspected )
	{
		//TODO: Range transition (private)

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			decimalRow( "Display ceiling: ",                 [](Chart* c) { return c->displayCeiling(); } ),
			decimalRow( "Display floor: ",                   [](Chart* c) { return c->displayFloor(); } ),
			objectRow ( "Display skin: ",                    [](Chart* c) -> Object* { return c->displaySkin().rawPtr(); } ),
			ptsRow    ( "Grid thickness (pts): ",            [](Chart* c) { return c->gridThickness(); } ),
			boolRow   ( "Grid on top: ",                     [](Chart* c) { return c->gridOnTop(); } ),
			objectRow ( "Label skin: ",                      [](Chart* c) -> Object* { return c->labelSkin().rawPtr(); } ),
			objectRow ( "Text style: ",                      [](Chart* c) -> Object* { return c->textStyle().rawPtr(); } ),
			objectRow ( "Text layout: ",                     [](Chart* c) -> Object* { return c->textLayout().rawPtr(); } ),
			textRow   ( "Left label placement: ",            [](Chart* c) { return toString(c->leftLabelPlacement()); } ),
			textRow   ( "Right label placement: ",           [](Chart* c) { return toString(c->rightLabelPlacement()); } ),
			textRow   ( "Top label placement: ",             [](Chart* c) { return toString(c->topLabelPlacement()); } ),
			textRow   ( "Bottom label placement: ",          [](Chart* c) { return toString(c->bottomLabelPlacement()); } ),
			ptsRow    ( "Left label spacing (pts): ",        [](Chart* c) { return c->leftLabelSpacing(); } ),
			ptsRow    ( "Right label spacing (pts): ",       [](Chart* c) { return c->rightLabelSpacing(); } ),
			ptsRow    ( "Top label spacing (pts): ",         [](Chart* c) { return c->topLabelSpacing(); } ),
			ptsRow    ( "Bottom label spacing (pts): ",      [](Chart* c) { return c->bottomLabelSpacing(); } )
		});

		m_displayedGridColor = pInspected->gridColor();
		m_pGridColorDrawer = _createColorDrawer("Grid color: ", m_displayedGridColor);

		m_pXLinesDrawer = _createGridLinesDrawer("X lines", pInspected->xLines);
		m_pYLinesDrawer = _createGridLinesDrawer("Y lines", pInspected->yLines);
		m_pGlowDrawer = _createComponentDrawer("Glow", &pInspected->glow);

		pPanel->slots.pushBack({ pTable, m_pGridColorDrawer, m_pXLinesDrawer, m_pYLinesDrawer, m_pGlowDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ChartInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ChartInfoSection::refresh()
	{
		TypedInfoSection<Chart>::refresh();

		_refreshColorDrawer(m_pGridColorDrawer, inspected()->gridColor(), m_displayedGridColor);
		_refreshGridLinesDrawer(m_pXLinesDrawer, inspected()->xLines);
		_refreshGridLinesDrawer(m_pYLinesDrawer, inspected()->yLines);
		_refreshComponentDrawer(m_pGlowDrawer);
	}

	//____ _createGridLinesDrawer() ______________________________________________
	//
	// One drawer per line, each holding a GridLineInfoSection. Header shows the
	// number of lines.

	DrawerPanel_p ChartInfoSection::_createGridLinesDrawer(const CharSeq& label, DynamicVector<GridLine>& lines)
	{
		auto pLineList = WGCREATE(PackPanel, _.axis = Axis::Y);

		_addGridLineDrawers(pLineList, lines, 0);

		auto pNumberLines = WGCREATE(NumberDisplay,
									 _ = m_pContext->theme().listEntryInteger,
									 _.display.value = lines.size());

		return _createDrawer(label, pNumberLines, pLineList);
	}

	//____ _refreshGridLinesDrawer() _____________________________________________
	//
	// Lines can be added, erased or moved in memory since last refresh, so the
	// sections are pointed at the lines again before being refreshed.

	void ChartInfoSection::_refreshGridLinesDrawer(DrawerPanel* pDrawer, DynamicVector<GridLine>& lines)
	{
		auto pLineList = static_cast<PackPanel*>(pDrawer->slots[1]._widget());

		int nLinesNow		= lines.size();
		int nLinesBefore	= pLineList->slots.size();

		int linesToRefresh = std::min(nLinesNow, nLinesBefore);

		for( int i = 0 ; i < linesToRefresh ; i++ )
		{
			auto pLineDrawer = static_cast<DrawerPanel*>(pLineList->slots[i]._widget());
			auto pSection = static_cast<GridLineInfoSection*>(pLineDrawer->slots[1]._widget());

			pSection->setInspected(&lines[i]);
			pSection->refresh();
		}

		if( nLinesNow < nLinesBefore )
			pLineList->slots.erase(nLinesNow, nLinesBefore - nLinesNow);
		else if( nLinesNow > nLinesBefore )
			_addGridLineDrawers(pLineList, lines, nLinesBefore);

		if( nLinesNow != nLinesBefore )
		{
			auto pNumberLines = WGCREATE(NumberDisplay,
										 _ = m_pContext->theme().listEntryInteger,
										 _.display.value = nLinesNow);
			_setDrawerHeaderValue(pDrawer, pNumberLines);
		}
	}

	//____ _addGridLineDrawers() _________________________________________________

	void ChartInfoSection::_addGridLineDrawers(PackPanel* pLineList, DynamicVector<GridLine>& lines, int begin)
	{
		for( int i = begin ; i < lines.size() ; i++ )
		{
			char buf[16];
			snprintf(buf, 16, "%d", i);

			auto pSection = GridLineInfoSection::create(m_pContext->theme(), m_pContext, &lines[i]);
			pLineList->slots << _createDrawer(buf, nullptr, pSection);
		}
	}

} // namespace wg
