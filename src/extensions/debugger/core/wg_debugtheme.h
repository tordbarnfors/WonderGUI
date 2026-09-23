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
#ifndef	WG_DEBUGTHEME_DOT_H
#define WG_DEBUGTHEME_DOT_H
#pragma once

#include <wg_capsule.h>
#include <wg_labelcapsule.h>
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_tablepanel.h>
#include <wg_drawerpanel.h>
#include <wg_paddingcapsule.h>

#include <widgetkits/wg_oldskool.h>

namespace wg
{
	namespace dbgkit = oldskool;

	//____ DebugTheme ___________________________________________________________
	//
	// Shared look for everything the debugger builds: icons, text styles, layouts
	// and prototype blueprints for the recurring widgets. Built once with
	// DebugTheme::create() by the frontend (DebugFrontend or DebugOverlay) and
	// handed to DebugBackend, which passes it on to every InfoSection and
	// InspectorView it creates.

	struct DebugTheme
	{
		//.____ Creation __________________________________________
		//
		// Both frontends get their look from here, so that they can't drift apart.
		// The icons surface holds 16x16 icons, the transparency grid is a
		// chessboard pattern or similar, shown behind partly transparent surfaces.

		static DebugTheme			create( Surface * pIcons, Surface * pTransparencyGrid );

		//.____ Resources _________________________________________

		Surface_p					icons;

		Surface_p					transparencyGrid;

		//.____ Icon skins ________________________________________
		//
		// Blocks picked out of the icons surface.

		Skin_p						refreshIcon;
		Skin_p						selectIcon;
		Skin_p						unselectIcon;
		Skin_p						expandIcon;
		Skin_p						condenseIcon;

		//.____ Widget blueprints _________________________________

		Capsule::Blueprint			mainCapsule;
		LabelCapsule::Blueprint		classCapsule;
		TextDisplay::Blueprint		listEntryLabel;
		TextDisplay::Blueprint		listEntryText;
		NumberDisplay::Blueprint	listEntryInteger;
		NumberDisplay::Blueprint	listEntryBool;
		NumberDisplay::Blueprint	listEntrySPX;
		NumberDisplay::Blueprint	listEntryPts;
		NumberDisplay::Blueprint	listEntryDecimal;
		TextDisplay::Blueprint		textField;
		TextDisplay::Blueprint		infoDisplay;
		TablePanel::Blueprint		table;
		dbgkit::TreeListDrawer::Blueprint	listEntryDrawer;
		dbgkit::TreeListEntry::Blueprint	selectableListEntryCapsule;
	};

} // namespace wg
#endif //WG_DEBUGTHEME_DOT_H
