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

	ObjectInfoPanel::ObjectInfoPanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, Object * pObject) : DebugPanel( blueprint, pHolder )
	{
		auto pTable = _createTable(3,2);
		_setIntegerEntry(pTable, 0, "Refcount: ", pObject->refcount());
		_setIntegerEntry(pTable, 1, "Weak pointers: ", pObject->weakPointers() - 1); // -1 since our DebugOverlay has one weak pointer to object that doesn't count.
		_setPointerEntry(pTable, 2, "Finalizer: ", pObject->finalizer().rawPtr() );
		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ObjectInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


