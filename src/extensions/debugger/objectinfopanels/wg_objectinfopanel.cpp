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
#include "wg_objectinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo ObjectInfoPanel::TYPEINFO = { "ObjectInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	ObjectInfoPanel::ObjectInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Object * pObject) : DebugPanel( blueprint, pHolder, Object::TYPEINFO.className )
	{
		m_pObject = pObject;
		m_pFinalizer = (void*) pObject->finalizer().rawPtr();

		m_pTable = _createTable(3,2);
		_setIntegerEntry(m_pTable, 0, "Refcount: ", pObject->refcount());
		_setIntegerEntry(m_pTable, 1, "Weak pointers: ", pObject->weakPointers());
		_setPointerEntry(m_pTable, 2, "Finalizer: ", (void *) pObject->finalizer().rawPtr() );
		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ObjectInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ObjectInfoPanel::refresh()
	{
		_refreshIntegerEntry(m_pTable, 0, m_pObject->refcount());
		_refreshIntegerEntry(m_pTable, 1, m_pObject->weakPointers());
		_refreshPointerEntry(m_pTable, 2, (void *) m_pObject->finalizer().rawPtr(), m_pFinalizer );

	}


} // namespace wg


