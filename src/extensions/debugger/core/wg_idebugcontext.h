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
#ifndef	WG_IDEBUGCONTEXT_DOT_H
#define WG_IDEBUGCONTEXT_DOT_H
#pragma once

#include <wg_object.h>
#include <wg_pointers.h>
#include <wg_component.h>
#include <wg_slot.h>

#include <wg_debugtheme.h>

namespace wg
{
	class PackPanel;

	//____ IDebugContext ________________________________________________________
	//
	// What an InfoSection or InspectorView needs from its surroundings: the
	// shared theme, a way to report that the user picked an object, and a way
	// to build the stack of info sections for an object, slot or component.
	// Implemented by DebugBackend.

	class IDebugContext
	{
	public:

		virtual const DebugTheme& theme() = 0;

		virtual void	objectSelected(Object * pSelected, Object * pCaller) = 0;

		// Add one info section per class in the inspected object's class chain
		// (most derived first) to the end of pPanel.

		virtual void	addObjectInfoSections(PackPanel* pPanel, Object* pObject) = 0;
		virtual void	addSlotInfoSections(PackPanel* pPanel, StaticSlot* pSlot) = 0;
		virtual void	addComponentInfoSections(PackPanel* pPanel, Component* pComponent) = 0;
	};

} // namespace wg
#endif //WG_IDEBUGCONTEXT_DOT_H
