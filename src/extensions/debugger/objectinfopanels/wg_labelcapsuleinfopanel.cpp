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
#include "wg_labelcapsuleinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo LabelCapsuleInfoPanel::TYPEINFO = { "LabelCapsuleInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	LabelCapsuleInfoPanel::LabelCapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, LabelCapsule * pCapsule) : DebugPanel( blueprint, pHolder, LabelCapsule::TYPEINFO.className )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		m_pInspected = pCapsule;
		m_pTable = _createTable(2,2);

		_initTextEntry(m_pTable, 0, "Label placement: ");
		_initObjectPointerEntry(m_pTable, 1, "Label skin: ");

		m_pLabelDrawer = _createComponentDrawer("Label", &pCapsule->label);

		pPanel->slots.pushBack({ m_pTable, m_pLabelDrawer });

		refresh();

		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& LabelCapsuleInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void LabelCapsuleInfoPanel::refresh()
	{
		_refreshTextEntry(m_pTable, 0, toString(m_pInspected->labelPlacement()));
		_refreshObjectPointerEntry(m_pTable, 1, m_pInspected->labelSkin(), m_pDisplayedLabelSkin );

		_refreshComponentDrawer(m_pLabelDrawer);
	}

} // namespace wg


