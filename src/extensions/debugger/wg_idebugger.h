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

#ifndef	WG_IDEBUGGER_DOT_H
#define WG_IDEBUGGER_DOT_H
#pragma once

#include <wg_object.h>
#include <wg_pointers.h>
#include <wg_component.h>

#include <wg_labelcapsule.h>
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_tablepanel.h>
#include <wg_drawerpanel.h>
#include <wg_paddingcapsule.h>


#include <wg_slot.h>

namespace wg
{
	class IDebugger
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			Theme_p						theme;
			Surface_p					icons;

			Surface_p					transparencyGrid;

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
			DrawerPanel::Blueprint		listEntryDrawer;
			PaddingCapsule::Blueprint	selectableListEntryCapsule;
		};

		virtual const Blueprint& blueprint() = 0;

		virtual void	objectSelected(Object * pSelected, Object * pCaller) = 0;

		virtual Widget_p createObjectInfoPanel(const TypeInfo* pType, Object* pObject) = 0;
		virtual Widget_p createSlotInfoPanel(const TypeInfo* pType, StaticSlot* pSlot) = 0;
		virtual Widget_p createComponentInfoPanel(const TypeInfo* pType, Component* pComponent) = 0;
	};

} // namespace wg
#endif //WG_IDEBUGGER_DOT_H

