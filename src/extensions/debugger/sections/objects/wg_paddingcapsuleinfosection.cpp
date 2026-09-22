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
#include "wg_paddingcapsuleinfosection.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo PaddingCapsuleInfoSection::TYPEINFO = { "PaddingCapsuleInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	PaddingCapsuleInfoSection::PaddingCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, PaddingCapsule * pCapsule) : InfoSection( theme, pContext, PaddingCapsule::TYPEINFO.className )
	{
		m_pInspected = pCapsule;
		m_displayedPadding = pCapsule->padding();

		m_pPaddingDrawer =_createBorderDrawer("Padding: ", m_displayedPadding);


		this->slot = m_pPaddingDrawer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PaddingCapsuleInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void PaddingCapsuleInfoSection::refresh()
	{
		_refreshBorderDrawer(m_pPaddingDrawer, m_pInspected->padding(), m_displayedPadding);
	}

} // namespace wg


