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
#include <wg_idebugger.h>

namespace wg
{
	class Debugger;
	typedef	StrongPtr<Debugger>		Debugger_p;
	typedef	WeakPtr<Debugger>		Debugger_wp;


	class Debugger : public Object , protected IDebugger
	{
	public:
		//.____ Creation __________________________________________

		static Debugger_p	create() { return Debugger_p(new Debugger()); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc __________________________________________


		void			setBlueprint(const IDebugger::Blueprint& blueprint);
		const IDebugger::Blueprint& blueprint() override;

		Widget_p		createObjectInfoPanel( const TypeInfo * pType, Object * pObject ) override;
		Widget_p		createSlotInfoPanel( const TypeInfo * pType, StaticSlot * pSlot ) override;
		Widget_p		createComponentInfoPanel( const TypeInfo* pType, Component* pComponent ) override;

		Widget_p		createObjectInspector(Object* pObject);
		Widget_p		createSkinInspector(Skin* pSkin);
		Widget_p		createWidgetTreeView(Widget * pWidget);
		Widget_p		createMsgLogViewer();

		void			setObjectSelectedCallback(std::function<void(Object*,Object*)> pCallback);



	protected:
		Debugger();
		~Debugger() {}

		void			objectSelected(Object* pSelected, Object* pCaller);

		IDebugger::Blueprint	m_blueprint;


		std::map<const TypeInfo*,Widget_p(*)(const DebugPanel::Blueprint&, IDebugger *, Object *)>	m_objectInfoFactories;
		std::map<const TypeInfo*,Widget_p(*)(const DebugPanel::Blueprint&, IDebugger *, StaticSlot *)>	m_slotInfoFactories;
	 	std::map<const TypeInfo*, Widget_p(*)(const DebugPanel::Blueprint&, IDebugger *, Component*)>	m_componentInfoFactories;

		std::vector<const TypeInfo*>	m_ignoreClasses;

		std::function<void(Object*, Object*)> m_objectSelectedCallback;
	};


} // namespace wg
#endif //WG_DEBUGGER_DOT_H

