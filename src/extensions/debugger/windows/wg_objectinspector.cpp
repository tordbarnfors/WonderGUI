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
#include <wg_textdisplay.h>
#include <wg_skindisplay.h>
#include <wg_boxskin.h>
#include <wg_packpanel.h>



namespace wg
{

	const TypeInfo ObjectInspector::TYPEINFO = { "ObjectInspector", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	ObjectInspector::ObjectInspector(const Blueprint& blueprint, DebugPanel::Holder * pHolder, Object * pObject) : DebugPanel( blueprint, pHolder )
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		pBasePanel->slots << _createObjectHeader(pObject);

		auto bp = m_blueprint;

		auto pTypeInfo = &pObject->typeInfo();

		while (pTypeInfo != nullptr)
		{
			bp.classCapsule.label.text = pTypeInfo->className;
			pBasePanel->slots << m_pHolder->createObjectInfoPanel(bp, pTypeInfo, pObject);
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


