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
#include <wg_packpanel.h>



namespace wg
{

	const TypeInfo ObjectInspector::TYPEINFO = { "ObjectInspector", &InspectorView::TYPEINFO };


	//____ constructor _____________________________________________________________

	ObjectInspector::ObjectInspector(const DebugTheme& bp, IDebugContext * pContext, Object * pObject) : InspectorView( bp, pContext )
	{
		m_title = _createObjectTitle(pObject);

		m_pObject = pObject;

		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		pBasePanel->slots.pushBack( _createButtonRow(true,true), WGBP(PackPanelSlot, _.weight = 0.f) );
		pBasePanel->slots.pushBack(_createClassInfoSections(bp, pObject));

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ObjectInspector::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg


