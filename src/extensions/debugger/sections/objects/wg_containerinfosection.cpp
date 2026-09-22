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
#include "wg_containerinfosection.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo ContainerInfoSection::TYPEINFO = { "ContainerInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ContainerInfoSection::ContainerInfoSection(const DebugTheme& theme, IDebugContext* pContext, Container * pContainer) : InfoSection( theme, pContext, Container::TYPEINFO.className )
	{
		m_pInspected = pContainer;

		m_pTable = _createTable(1,2);

		_setBoolEntry(m_pTable, 0, "Use pick handles: ", pContainer->usePickHandles());
		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ContainerInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ContainerInfoSection::refresh()
	{
		_refreshBoolEntry(m_pTable, 0, m_pInspected->usePickHandles());
	}

} // namespace wg


