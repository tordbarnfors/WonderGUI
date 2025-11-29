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
#include "wg_objectinspector.h"
#include <wg_debugpanel.h>

#include <wg_scrollpanel.h>
#include <wg_textdisplay.h>
#include <wg_skindisplay.h>
#include <wg_boxskin.h>
#include <wg_blockskin.h>
#include <wg_packpanel.h>
#include <wg_msgrouter.h>



namespace wg
{

	const TypeInfo ObjectInspector::TYPEINFO = { "ObjectInspector", &DebugWindow::TYPEINFO };


	//____ constructor _____________________________________________________________

	ObjectInspector::ObjectInspector(const Blueprint& bp, IDebugger * pHolder, Object * pObject) : DebugWindow( bp, pHolder )
	{
		m_title = _createObjectTitle(pObject);

		m_pObject = pObject;

		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		pBasePanel->slots.pushBack( _createButtonRow(true,true), WGBP(PackPanelSlot, _.weight = 0.f) );
		pBasePanel->slots.pushBack(_createClassInfoPanels(bp, pObject));

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ObjectInspector::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg


