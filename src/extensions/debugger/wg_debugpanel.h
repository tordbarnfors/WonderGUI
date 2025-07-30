
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

#ifndef	WG_DEBUGPANEL_DOT_H
#define WG_DEBUGPANEL_DOT_H
#pragma once

#include <wg_labelcapsule.h>
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_tablepanel.h>
#include <wg_drawerpanel.h>
#include <wg_paddingcapsule.h>
#include <wg_theme.h>

namespace wg
{
	class DebugPanel;
	typedef	StrongPtr<DebugPanel>	DebugPanel_p;
	typedef	WeakPtr<DebugPanel>		DebugPanel_wp;



	class DebugPanel : public LabelCapsule
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			Theme_p						theme;
			Surface_p					icons;

			LabelCapsule::Blueprint		mainCapsule;
			LabelCapsule::Blueprint		classCapsule;
			TextDisplay::Blueprint		listEntryLabel;
			TextDisplay::Blueprint		listEntryText;
			NumberDisplay::Blueprint	listEntryInteger;
			NumberDisplay::Blueprint	listEntryBool;
			NumberDisplay::Blueprint	listEntrySPX;
			NumberDisplay::Blueprint	listEntryPts;
			NumberDisplay::Blueprint	listEntryDecimal;
			NumberDisplay::Blueprint	listEntryPointer;
			TextDisplay::Blueprint		infoDisplay;
			TablePanel::Blueprint		table;
			DrawerPanel::Blueprint		listEntryDrawer;
			PaddingCapsule::Blueprint	selectableListEntryCapsule;

		};

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		DebugPanel(const Blueprint& blueprint );
		~DebugPanel() {}
	};

} // namespace wg
#endif //WG_OBJECTINFOPANEL_DOT_H

