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
#include "wg_containerinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo ContainerInfoPanel::TYPEINFO = { "ContainerInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	ContainerInfoPanel::ContainerInfoPanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, Container * pContainer) : DebugPanel( blueprint, pHolder )
	{
		auto pTable = _createTable(1,2);

		_setBoolEntry(pTable, 0, "Use pick handles: ", pContainer->usePickHandles());
		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ContainerInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


