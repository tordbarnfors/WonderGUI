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
#include "wg_sizecapsuleinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo SizeCapsuleInfoPanel::TYPEINFO = { "SizeCapsuleInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	SizeCapsuleInfoPanel::SizeCapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, SizeCapsule * pCapsule) : DebugPanel( blueprint, pHolder, SizeCapsule::TYPEINFO.className )
	{
		m_pInspected = pCapsule;
		m_pTable = _createTable(6,2);

		_initPtsEntry(m_pTable, 0, "Default width: ");
		_initPtsEntry(m_pTable, 1, "Default height: ");
		_initPtsEntry(m_pTable, 2, "Min width: ");
		_initPtsEntry(m_pTable, 3, "Min height: ");
		_initPtsEntry(m_pTable, 4, "Max width: ");
		_initPtsEntry(m_pTable, 5, "Max height: ");

		refresh();

		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SizeCapsuleInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void SizeCapsuleInfoPanel::refresh()
	{
		Size	def = m_pInspected->defaultSize();
		Size	min = m_pInspected->minSize();
		Size	max = m_pInspected->maxSize();

		_refreshPtsEntry(m_pTable, 0, def.w);
		_refreshPtsEntry(m_pTable, 1, def.h);
		_refreshPtsEntry(m_pTable, 2, min.w);
		_refreshPtsEntry(m_pTable, 3, min.h);
		_refreshPtsEntry(m_pTable, 4, max.w);
		_refreshPtsEntry(m_pTable, 5, max.h);
	}

} // namespace wg


