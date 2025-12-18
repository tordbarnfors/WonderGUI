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
#ifndef	WG_TEXTDISPLAYINFOPANEL_DOT_H
#define WG_TEXTDISPLAYINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_textdisplay.h>

namespace wg
{
	class TextDisplayInfoPanel;
	typedef	StrongPtr<TextDisplayInfoPanel>	TextDisplayInfoPanel_p;
	typedef	WeakPtr<TextDisplayInfoPanel>	TextDisplayInfoPanel_wp;



	class TextDisplayInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static TextDisplayInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, TextDisplay * pTextDisplay) { return TextDisplayInfoPanel_p(new TextDisplayInfoPanel(blueprint, pHolder, pTextDisplay) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		TextDisplayInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, TextDisplay * pTextDisplay );
		~TextDisplayInfoPanel() {}

	};

} // namespace wg
#endif //WG_TEXTDISPLAYINFOPANEL_DOT_H

