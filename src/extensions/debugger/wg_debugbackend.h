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

#ifndef	WG_DEBUGBACKEND_DOT_H
#define WG_DEBUGBACKEND_DOT_H
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
	class DebugBackend;
	typedef	StrongPtr<DebugBackend>		DebugBackend_p;
	typedef	WeakPtr<DebugBackend>		DebugBackend_wp;


	class DebugBackend : public Object , protected IDebugger
	{
	public:
		//.____ Creation __________________________________________

		static DebugBackend_p	create() { return DebugBackend_p(new DebugBackend()); }

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
		DebugBackend();
		~DebugBackend() {}

		void			objectSelected(Object* pSelected, Object* pCaller);

		IDebugger::Blueprint	m_blueprint;


		std::map<const TypeInfo*,Widget_p(*)(const DebugPanel::Blueprint&, IDebugger *, Object *)>	m_objectInfoFactories;
		std::map<const TypeInfo*,Widget_p(*)(const DebugPanel::Blueprint&, IDebugger *, StaticSlot *)>	m_slotInfoFactories;
	 	std::map<const TypeInfo*, Widget_p(*)(const DebugPanel::Blueprint&, IDebugger *, Component*)>	m_componentInfoFactories;

		std::vector<const TypeInfo*>	m_ignoreClasses;

		std::function<void(Object*, Object*)> m_objectSelectedCallback;
	};


} // namespace wg
#endif //WG_DEBUGBACKEND_DOT_H

