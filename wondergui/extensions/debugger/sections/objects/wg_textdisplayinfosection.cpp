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
#include "wg_textdisplayinfosection.h"
#include <wg_textdisplay.h>


namespace wg
{

	const TypeInfo TextDisplayInfoSection::TYPEINFO = { "TextDisplayInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	TextDisplayInfoSection::TextDisplayInfoSection(const DebugTheme& theme, IDebugContext* pContext, TextDisplay * pInspected)
		: TypedInfoSection<TextDisplay>( theme, pContext, TextDisplay::TYPEINFO.className, pInspected )
	{
		m_pDisplayDrawer = _createComponentDrawer("Display", &pInspected->display);
		this->slot = m_pDisplayDrawer;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextDisplayInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void TextDisplayInfoSection::refresh()
	{
		TypedInfoSection<TextDisplay>::refresh();

		_refreshComponentDrawer(m_pDisplayDrawer);
	}

} // namespace wg
