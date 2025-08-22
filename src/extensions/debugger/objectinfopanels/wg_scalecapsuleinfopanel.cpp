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
#include "wg_scalecapsuleinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo ScaleCapsuleInfoPanel::TYPEINFO = { "ScaleCapsuleInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	ScaleCapsuleInfoPanel::ScaleCapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, ScaleCapsule * pCapsule) : DebugPanel( blueprint, pHolder, ScaleCapsule::TYPEINFO.className )
	{
		m_pInspected = pCapsule;
		m_pTable = _createTable(1,2);

		_initIntegerEntry(m_pTable, 0, "Scale set: ");

		refresh();

		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScaleCapsuleInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ScaleCapsuleInfoPanel::refresh()
	{
		int scale = m_pInspected->isScaleSet() ? m_pInspected->scale() : 0;

		_refreshIntegerEntry(m_pTable, 0, scale);
	}

} // namespace wg


