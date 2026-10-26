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
#include "wg_labelcapsuleinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo LabelCapsuleInfoSection::TYPEINFO = { "LabelCapsuleInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	LabelCapsuleInfoSection::LabelCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, LabelCapsule * pInspected)
		: TypedInfoSection<LabelCapsule>( theme, pContext, LabelCapsule::TYPEINFO.className, pInspected )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow  ( "Label placement: ", [](LabelCapsule* c) { return toString(c->labelPlacement()); } ),
			objectRow( "Label skin: ",      [](LabelCapsule* c) -> Object* { return c->labelSkin(); } )
		});

		m_pLabelDrawer = _createComponentDrawer("Label", &pInspected->label);

		pPanel->slots.pushBack({ pTable, m_pLabelDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& LabelCapsuleInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void LabelCapsuleInfoSection::refresh()
	{
		TypedInfoSection<LabelCapsule>::refresh();

		_refreshComponentDrawer(m_pLabelDrawer);
	}

} // namespace wg
