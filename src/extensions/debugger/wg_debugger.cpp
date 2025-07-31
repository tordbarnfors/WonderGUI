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
#include <wg_debugger.h>

#include <wg_dummyinfopanel.h>
#include <wg_widgettreepanel.h>
#include <wg_msglogpanel.h>

#include <objectinfopanels/wg_objectinfopanel.h>
#include <objectinfopanels/wg_widgetinfopanel.h>
#include <objectinfopanels/wg_containerinfopanel.h>
#include <objectinfopanels/wg_panelinfopanel.h>

#include <slotinfopanels/wg_staticslotinfopanel.h>
#include <slotinfopanels/wg_panelslotinfopanel.h>
#include <slotinfopanels/wg_packpanelslotinfopanel.h>
#include <slotinfopanels/wg_flexpanelslotinfopanel.h>

#include <wg_panel.h>
#include <wg_packpanel.h>
#include <wg_flexpanel.h>
#include <wg_msg.h>


namespace wg
{

	const TypeInfo Debugger::TYPEINFO = { "Debugger", &Object::TYPEINFO };


	Widget_p factory( Object *)
	{
		return Widget_p();
	}

	//____ constructor _____________________________________________________________

	Debugger::Debugger()
	{
		m_objectInfoFactories[&Object::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, DebugPanel::Holder* pHolder, Object* pObject) { return (Widget_p) ObjectInfoPanel::create(panelBP, pHolder, pObject); };
		m_objectInfoFactories[&Widget::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, DebugPanel::Holder* pHolder, Object* pObject) { return (Widget_p) WidgetInfoPanel::create(panelBP, pHolder, (Widget*) pObject); };
		m_objectInfoFactories[&Container::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, DebugPanel::Holder* pHolder, Object* pObject) { return (Widget_p)ContainerInfoPanel::create(panelBP, pHolder, (Container*)pObject); };
		m_objectInfoFactories[&Panel::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, DebugPanel::Holder* pHolder, Object* pObject) { return (Widget_p)PanelInfoPanel::create(panelBP, pHolder, (Panel*)pObject); };

		m_slotInfoFactories[&StaticSlot::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, DebugPanel::Holder* pHolder, StaticSlot* pSlot) { return (Widget_p) StaticSlotInfoPanel::create(panelBP, pHolder, pSlot); };
		m_slotInfoFactories[&PanelSlot::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, DebugPanel::Holder* pHolder, StaticSlot* pSlot) { return (Widget_p)PanelSlotInfoPanel::create(panelBP, pHolder, pSlot); };
		m_slotInfoFactories[&PackPanelSlot::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, DebugPanel::Holder* pHolder, StaticSlot* pSlot) { return (Widget_p)PackPanelSlotInfoPanel::create(panelBP, pHolder, pSlot); };
		m_slotInfoFactories[&FlexPanelSlot::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, DebugPanel::Holder* pHolder, StaticSlot* pSlot) { return (Widget_p)FlexPanelSlotInfoPanel::create(panelBP, pHolder, pSlot); };

		m_ignoreClasses.push_back(&DynamicSlot::TYPEINFO);
		m_ignoreClasses.push_back(&Receiver::TYPEINFO);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Debugger::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createObjectInfoPanel() ____________________________________________________

	Widget_p Debugger::createObjectInfoPanel( const DebugPanel::Blueprint& bp, const TypeInfo * pType, Object * pObject )
	{

		auto it = m_objectInfoFactories.find( pType );
		if( it == m_objectInfoFactories.end() )
		{
			// Check known classes we should ignore.

			if (std::find(m_ignoreClasses.begin(), m_ignoreClasses.end(), pType) != m_ignoreClasses.end())
				return nullptr;

			// Unknown class

			return DummyInfoPanel::create(bp,this,pObject);
		}

		return it->second(bp,this,pObject);
	}

	//____ createSlotInfoPanel() ____________________________________________________

	Widget_p Debugger::createSlotInfoPanel( const DebugPanel::Blueprint& bp, const TypeInfo * pType, StaticSlot * pSlot )
	{

		auto it = m_slotInfoFactories.find( pType );
		if( it == m_slotInfoFactories.end() )
		{
			// Check known classes we should ignore.

			if (std::find(m_ignoreClasses.begin(), m_ignoreClasses.end(), pType) != m_ignoreClasses.end())
				return nullptr;

			// Unknown class

			return DummyInfoPanel::create(bp,this,pSlot);
		}

		return it->second(bp,this,pSlot);
	}

	//____ createComponentInfoPanel() ____________________________________________________

	Widget_p Debugger::createComponentInfoPanel(const DebugPanel::Blueprint& bp, const TypeInfo* pType, Component* pComponent)
	{

		auto it = m_componentInfoFactories.find(pType);
		if (it == m_componentInfoFactories.end())
		{
			// Check known classes we should ignore.

			if (std::find(m_ignoreClasses.begin(), m_ignoreClasses.end(), pType) != m_ignoreClasses.end())
				return nullptr;

			// Unknown class

			return DummyInfoPanel::create(bp,this,pComponent);
		}

		return it->second(bp, this, pComponent);
	}


	//____ createWidgetTreePanel() ____________________________________________

	Widget_p Debugger::createWidgetTreePanel(const DebugPanel::Blueprint& blueprint, Widget* pRoot)
	{
		return WidgetTreePanel::create(blueprint, this, pRoot);
	}

	//____ createMsgLogPanel() ________________________________________________

	Widget_p Debugger::createMsgLogPanel(const DebugPanel::Blueprint& blueprint)
	{
		return MsgLogPanel::create(blueprint, this);
	}

	//____ setObjectSelectedCallback() ________________________________________

	void Debugger::setObjectSelectedCallback(std::function<void(Object*,Object*)> pCallback)
	{
		m_objectSelectedCallback = pCallback;
	}

	//____ objectSelected() ____________________________________________________

	void Debugger::objectSelected(Object* pSelected, Object* pCaller)
	{
		if (m_objectSelectedCallback)
			m_objectSelectedCallback(pSelected, pCaller);
	}



} // namespace wg

