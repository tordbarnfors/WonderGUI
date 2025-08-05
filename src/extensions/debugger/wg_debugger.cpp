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
#include <windows/wg_msglogviewer.h>
#include <windows/wg_objectinspector.h>
#include <windows/wg_objectlist.h>
#include <windows/wg_skininspector.h>
#include <windows/wg_surfaceinspector.h>
#include <windows/wg_textstyleinspector.h>
#include <windows/wg_widgettreeview.h>

#include <objectinfopanels/wg_objectinfopanel.h>
#include <objectinfopanels/wg_widgetinfopanel.h>
#include <objectinfopanels/wg_fillerinfopanel.h>
#include <objectinfopanels/wg_containerinfopanel.h>
#include <objectinfopanels/wg_flexpanelinfopanel.h>
#include <objectinfopanels/wg_panelinfopanel.h>
#include <objectinfopanels/wg_packpanelinfopanel.h>
#include <objectinfopanels/wg_textdisplayinfopanel.h>
#include <objectinfopanels/wg_twoslotpanelinfopanel.h>

#include <objectinfopanels/wg_skininfopanel.h>

#include <slotinfopanels/wg_staticslotinfopanel.h>
#include <slotinfopanels/wg_panelslotinfopanel.h>
#include <slotinfopanels/wg_packpanelslotinfopanel.h>
#include <slotinfopanels/wg_flexpanelslotinfopanel.h>
#include <slotinfopanels/wg_twoslotpanelslotinfopanel.h>

#include <componentinfopanels/wg_statictextinfopanel.h>

#include <wg_panel.h>
#include <wg_packpanel.h>
#include <wg_flexpanel.h>
#include <wg_stateskin.h>
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

		m_objectInfoFactories[&Object::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Object* pObject) { return (Widget_p) ObjectInfoPanel::create(panelBP, pHolder, pObject); };
		m_objectInfoFactories[&Widget::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Object* pObject) { return (Widget_p) WidgetInfoPanel::create(panelBP, pHolder, (Widget*) pObject); };
		m_objectInfoFactories[&Filler::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Object* pObject) { return (Widget_p) FillerInfoPanel::create(panelBP, pHolder, (Filler*)pObject); };
		m_objectInfoFactories[&TextDisplay::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Object* pObject) { return (Widget_p)TextDisplayInfoPanel::create(panelBP, pHolder, (TextDisplay*)pObject); };

		m_objectInfoFactories[&Container::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Object* pObject) { return (Widget_p)ContainerInfoPanel::create(panelBP, pHolder, (Container*)pObject); };
		m_objectInfoFactories[&Panel::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Object* pObject) { return (Widget_p)PanelInfoPanel::create(panelBP, pHolder, (Panel*)pObject); };
		m_objectInfoFactories[&PackPanel::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Object* pObject) { return (Widget_p)PackPanelInfoPanel::create(panelBP, pHolder, (PackPanel*)pObject); };
		m_objectInfoFactories[&FlexPanel::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Object* pObject) { return (Widget_p)FlexPanelInfoPanel::create(panelBP, pHolder, (FlexPanel*)pObject); };
		m_objectInfoFactories[&TwoSlotPanel::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Object* pObject) { return (Widget_p)TwoSlotPanelInfoPanel::create(panelBP, pHolder, (TwoSlotPanel*)pObject); };

		m_objectInfoFactories[&Skin::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Object* pObject) { return (Widget_p)SkinInfoPanel::create(panelBP, pHolder, (Skin*)pObject); };

		m_slotInfoFactories[&StaticSlot::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, StaticSlot* pSlot) { return (Widget_p) StaticSlotInfoPanel::create(panelBP, pHolder, pSlot); };
		m_slotInfoFactories[&PanelSlot::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, StaticSlot* pSlot) { return (Widget_p)PanelSlotInfoPanel::create(panelBP, pHolder, pSlot); };
		m_slotInfoFactories[&PackPanelSlot::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, StaticSlot* pSlot) { return (Widget_p)PackPanelSlotInfoPanel::create(panelBP, pHolder, pSlot); };
		m_slotInfoFactories[&FlexPanelSlot::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, StaticSlot* pSlot) { return (Widget_p)FlexPanelSlotInfoPanel::create(panelBP, pHolder, pSlot); };
		m_slotInfoFactories[&TwoSlotPanel::Slot::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, StaticSlot* pSlot) { return (Widget_p)TwoSlotPanelSlotInfoPanel::create(panelBP, pHolder, pSlot); };

		m_componentInfoFactories[&StaticText::TYPEINFO] = [](const DebugPanel::Blueprint& panelBP, IDebugger* pHolder, Component* pComponent) { return (Widget_p)StaticTextInfoPanel::create(panelBP, pHolder, (StaticText*)pComponent); };

		m_ignoreClasses.push_back(&DynamicSlot::TYPEINFO);
		m_ignoreClasses.push_back(&Receiver::TYPEINFO);
		m_ignoreClasses.push_back(&Component::TYPEINFO);
		m_ignoreClasses.push_back(&DynamicText::TYPEINFO);
		m_ignoreClasses.push_back(&StateSkin::TYPEINFO);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Debugger::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setBlueprint() ________________________________________________________

	void Debugger::setBlueprint(const IDebugger::Blueprint& blueprint)
	{
		m_blueprint = blueprint;
	}


	//____ blueprint() ___________________________________________________________

	const IDebugger::Blueprint& Debugger::blueprint()
	{
		return m_blueprint;
	}

	//____ createObjectInfoPanel() ____________________________________________________

	Widget_p Debugger::createObjectInfoPanel(const TypeInfo * pType, Object * pObject )
	{

		auto it = m_objectInfoFactories.find( pType );
		if( it == m_objectInfoFactories.end() )
		{
			// Check known classes we should ignore.

			if (std::find(m_ignoreClasses.begin(), m_ignoreClasses.end(), pType) != m_ignoreClasses.end())
				return nullptr;

			// Unknown class

			return DummyInfoPanel::create(m_blueprint,this, pType->className,pObject);
		}

		return it->second(m_blueprint,this,pObject);
	}

	//____ createSlotInfoPanel() ____________________________________________________

	Widget_p Debugger::createSlotInfoPanel(const TypeInfo * pType, StaticSlot * pSlot )
	{

		auto it = m_slotInfoFactories.find( pType );
		if( it == m_slotInfoFactories.end() )
		{
			// Check known classes we should ignore.

			if (std::find(m_ignoreClasses.begin(), m_ignoreClasses.end(), pType) != m_ignoreClasses.end())
				return nullptr;

			// Unknown class

			return DummyInfoPanel::create(m_blueprint,this, pType->className,pSlot);
		}

		return it->second(m_blueprint,this,pSlot);
	}

	//____ createComponentInfoPanel() ____________________________________________________

	Widget_p Debugger::createComponentInfoPanel(const TypeInfo* pType, Component* pComponent)
	{

		auto it = m_componentInfoFactories.find(pType);
		if (it == m_componentInfoFactories.end())
		{
			// Check known classes we should ignore.

			if (std::find(m_ignoreClasses.begin(), m_ignoreClasses.end(), pType) != m_ignoreClasses.end())
				return nullptr;

			// Unknown class

			return DummyInfoPanel::create(m_blueprint,this, pType->className,pComponent);
		}

		return it->second(m_blueprint, this, pComponent);
	}

	//____ createObjectInspector() ________________________________________________

	Widget_p Debugger::createObjectInspector(Object* pObject)
	{
		return ObjectInspector::create(m_blueprint, this, pObject);
	}

	//____ createSkinInspector() ________________________________________________

	Widget_p Debugger::createSkinInspector(Skin* pSkin)
	{
		return SkinInspector::create(m_blueprint, this, pSkin);
	}


	//____ createWidgetTreeView() ____________________________________________

	Widget_p Debugger::createWidgetTreeView(Widget* pRoot)
	{
		return WidgetTreeView::create(m_blueprint, this, pRoot);
	}

	//____ createMsgLogViewer() ________________________________________________

	Widget_p Debugger::createMsgLogViewer()
	{
		return MsgLogViewer::create(m_blueprint, this);
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

