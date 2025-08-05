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
#include "wg_objectinspector.h"
#include <wg_debugpanel.h>

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
		m_pObject = pObject;

		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		pBasePanel->slots << _createButtonRow(true,true);
		pBasePanel->slots << _createObjectHeader(pObject);

		auto pTypeInfo = &pObject->typeInfo();
		while (pTypeInfo != nullptr)
		{
			pBasePanel->slots << m_pHolder->createObjectInfoPanel(pTypeInfo, pObject);
			pTypeInfo = pTypeInfo->pSuperClass;
		}

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ObjectInspector::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


