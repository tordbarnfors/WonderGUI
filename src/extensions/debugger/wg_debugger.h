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

#ifndef	WG_DEBUGGER_DOT_H
#define WG_DEBUGGER_DOT_H
#pragma once

#include <map>
#include <functional>

#include <wg_object.h>
#include <wg_pointers.h>
#include <wg_widget.h>
#include <wg_debugpanel.h>

namespace wg
{


	class Debugger;
	typedef	StrongPtr<Debugger>		Debugger_p;
	typedef	WeakPtr<Debugger>		Debugger_wp;


	class Debugger : public Object , protected DebugPanel::Holder
	{
	public:
		//.____ Creation __________________________________________

		static Debugger_p	create() { return Debugger_p(new Debugger()); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc __________________________________________

		Widget_p		createObjectInfoPanel( const DebugPanel::Blueprint& blueprint, const TypeInfo * pType, Object * pObject );
		Widget_p		createSlotInfoPanel( const DebugPanel::Blueprint& blueprint, const TypeInfo * pType, StaticSlot * pSlot );
		Widget_p		createComponentInfoPanel(const DebugPanel::Blueprint& blueprint, const TypeInfo* pType, Component* pComponent);

		Widget_p		createWidgetTreePanel(const DebugPanel::Blueprint& blueprint, Widget * pWidget);
		Widget_p		createMsgLogPanel(const DebugPanel::Blueprint& blueprint);

		void			setObjectSelectedCallback(std::function<void(Object*,Object*)> pCallback);



	protected:
		Debugger();
		~Debugger() {}

		void			objectSelected(Object* pSelected, Object* pCaller);

		std::map<const TypeInfo*,Widget_p(*)(const DebugPanel::Blueprint&, DebugPanel::Holder *, Object *)>	m_objectInfoFactories;
		std::map<const TypeInfo*,Widget_p(*)(const DebugPanel::Blueprint&, DebugPanel::Holder *, StaticSlot *)>	m_slotInfoFactories;
	 	std::map<const TypeInfo*, Widget_p(*)(const DebugPanel::Blueprint&, DebugPanel::Holder*, Component*)>	m_componentInfoFactories;

		std::vector<const TypeInfo*>	m_ignoreClasses;

		std::function<void(Object*, Object*)> m_objectSelectedCallback;
	};


} // namespace wg
#endif //WG_DEBUGGER_DOT_H

