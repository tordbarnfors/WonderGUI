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
#include "wg_scalecapsuleinfosection.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo ScaleCapsuleInfoSection::TYPEINFO = { "ScaleCapsuleInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ScaleCapsuleInfoSection::ScaleCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, ScaleCapsule * pCapsule) : InfoSection( theme, pContext, ScaleCapsule::TYPEINFO.className )
	{
		m_pInspected = pCapsule;
		m_pTable = _createTable(1,2);

		_initIntegerEntry(m_pTable, 0, "Scale set: ");

		refresh();

		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScaleCapsuleInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ScaleCapsuleInfoSection::refresh()
	{
		int scale = m_pInspected->isScaleSet() ? m_pInspected->scale() : 0;

		_refreshIntegerEntry(m_pTable, 0, scale);
	}

} // namespace wg


