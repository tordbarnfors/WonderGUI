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
#include <wg_debugbackend.h>

#include <wg_dummyinfosection.h>
#include <wg_msglogviewer.h>
#include <wg_objectinspector.h>
#include <wg_skininspector.h>
#include <wg_textstyleinspector.h>
#include <wg_widgetinspector.h>
#include <wg_widgettreeview.h>

#include <wg_objectinfosection.h>
#include <wg_widgetinfosection.h>
#include <wg_fillerinfosection.h>
#include <wg_buttoninfosection.h>
#include <wg_textdisplayinfosection.h>
#include <wg_texteditorinfosection.h>
#include <wg_containerinfosection.h>

#include <wg_panelinfosection.h>
#include <wg_packpanelinfosection.h>
#include <wg_flexpanelinfosection.h>
#include <wg_scrollpanelinfosection.h>
#include <wg_twoslotpanelinfosection.h>

#include <wg_capsuleinfosection.h>
#include <wg_sizecapsuleinfosection.h>
#include <wg_reordercapsuleinfosection.h>
#include <wg_selectcapsuleinfosection.h>
#include <wg_labelcapsuleinfosection.h>
#include <wg_paddingcapsuleinfosection.h>
#include <wg_renderlayercapsuleinfosection.h>
#include <wg_scalecapsuleinfosection.h>
#include <wg_canvascapsuleinfosection.h>


#include <wg_skininfosection.h>

#include <wg_staticslotinfosection.h>
#include <wg_panelslotinfosection.h>
#include <wg_packpanelslotinfosection.h>
#include <wg_flexpanelslotinfosection.h>
#include <wg_twoslotpanelslotinfosection.h>

#include <wg_statictextinfosection.h>
#include <wg_editabletextinfosection.h>
#include <wg_iconinfosection.h>
#include <wg_scrollerinfosection.h>
#include <wg_glowinfosection.h>

#include <wg_panel.h>
#include <wg_packpanel.h>
#include <wg_flexpanel.h>
#include <wg_stateskin.h>
#include <wg_msg.h>


namespace wg
{

	const TypeInfo DebugBackend::TYPEINFO = { "DebugBackend", &Object::TYPEINFO };


	Widget_p factory( Object *)
	{
		return Widget_p();
	}

	//____ constructor _____________________________________________________________

	DebugBackend::DebugBackend()
	{

		m_objectInfoFactories[&Object::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p) ObjectInfoSection::create(theme, pContext, pObject); };
		m_objectInfoFactories[&Widget::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p) WidgetInfoSection::create(theme, pContext, (Widget*) pObject); };
		m_objectInfoFactories[&Filler::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p) FillerInfoSection::create(theme, pContext, (Filler*)pObject); };
		m_objectInfoFactories[&TextDisplay::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)TextDisplayInfoSection::create(theme, pContext, (TextDisplay*)pObject); };
		m_objectInfoFactories[&TextEditor::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)TextEditorInfoSection::create(theme, pContext, (TextEditor*)pObject); };
		m_objectInfoFactories[&Button::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)ButtonInfoSection::create(theme, pContext, (Button*)pObject); };

		m_objectInfoFactories[&Container::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)ContainerInfoSection::create(theme, pContext, (Container*)pObject); };
		m_objectInfoFactories[&Panel::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)PanelInfoSection::create(theme, pContext, (Panel*)pObject); };
		m_objectInfoFactories[&PackPanel::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)PackPanelInfoSection::create(theme, pContext, (PackPanel*)pObject); };
		m_objectInfoFactories[&FlexPanel::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)FlexPanelInfoSection::create(theme, pContext, (FlexPanel*)pObject); };
		m_objectInfoFactories[&TwoSlotPanel::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)TwoSlotPanelInfoSection::create(theme, pContext, (TwoSlotPanel*)pObject); };
		m_objectInfoFactories[&ScrollPanel::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)ScrollPanelInfoSection::create(theme, pContext, (ScrollPanel*)pObject); };

		m_objectInfoFactories[&Capsule::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)CapsuleInfoSection::create(theme, pContext, (Capsule*)pObject); };
		m_objectInfoFactories[&SizeCapsule::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)SizeCapsuleInfoSection::create(theme, pContext, (SizeCapsule*)pObject); };
		m_objectInfoFactories[&ReorderCapsule::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)ReorderCapsuleInfoSection::create(theme, pContext, (ReorderCapsule*)pObject); };
		m_objectInfoFactories[&SelectCapsule::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)SelectCapsuleInfoSection::create(theme, pContext, (SelectCapsule*)pObject); };
		m_objectInfoFactories[&LabelCapsule::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)LabelCapsuleInfoSection::create(theme, pContext, (LabelCapsule*)pObject); };
		m_objectInfoFactories[&PaddingCapsule::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)PaddingCapsuleInfoSection::create(theme, pContext, (PaddingCapsule*)pObject); };
		m_objectInfoFactories[&RenderLayerCapsule::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)RenderLayerCapsuleInfoSection::create(theme, pContext, (RenderLayerCapsule*)pObject); };
		m_objectInfoFactories[&ScaleCapsule::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)ScaleCapsuleInfoSection::create(theme, pContext, (ScaleCapsule*)pObject); };
		m_objectInfoFactories[&CanvasCapsule::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)CanvasCapsuleInfoSection::create(theme, pContext, (CanvasCapsule*)pObject); };


		m_objectInfoFactories[&Skin::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Object* pObject) { return (Widget_p)SkinInfoSection::create(theme, pContext, (Skin*)pObject); };

		m_slotInfoFactories[&StaticSlot::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, StaticSlot* pSlot) { return (Widget_p) StaticSlotInfoSection::create(theme, pContext, pSlot); };
		m_slotInfoFactories[&PanelSlot::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, StaticSlot* pSlot) { return (Widget_p)PanelSlotInfoSection::create(theme, pContext, pSlot); };
		m_slotInfoFactories[&PackPanelSlot::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, StaticSlot* pSlot) { return (Widget_p)PackPanelSlotInfoSection::create(theme, pContext, pSlot); };
		m_slotInfoFactories[&FlexPanelSlot::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, StaticSlot* pSlot) { return (Widget_p)FlexPanelSlotInfoSection::create(theme, pContext, pSlot); };
		m_slotInfoFactories[&TwoSlotPanel::Slot::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, StaticSlot* pSlot) { return (Widget_p)TwoSlotPanelSlotInfoSection::create(theme, pContext, pSlot); };

		m_componentInfoFactories[&StaticText::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Component* pComponent) { return (Widget_p)StaticTextInfoSection::create(theme, pContext, (StaticText*)pComponent); };
		m_componentInfoFactories[&EditableText::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Component* pComponent) { return (Widget_p)EditableTextInfoSection::create(theme, pContext, (EditableText*)pComponent); };
		m_componentInfoFactories[&Icon::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Component* pComponent) { return (Widget_p)IconInfoSection::create(theme, pContext, (Icon*)pComponent); };
		m_componentInfoFactories[&Scroller::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Component* pComponent) { return (Widget_p)ScrollerInfoSection::create(theme, pContext, (Scroller*)pComponent); };
		m_componentInfoFactories[&Glow::TYPEINFO] = [](const DebugTheme& theme, IDebugContext* pContext, Component* pComponent) { return (Widget_p)GlowInfoSection::create(theme, pContext, (Glow*)pComponent); };

		m_ignoreClasses.push_back(&DynamicSlot::TYPEINFO);
		m_ignoreClasses.push_back(&Receiver::TYPEINFO);
		m_ignoreClasses.push_back(&Component::TYPEINFO);
		m_ignoreClasses.push_back(&DynamicText::TYPEINFO);
		m_ignoreClasses.push_back(&StateSkin::TYPEINFO);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DebugBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setTheme() ________________________________________________________

	void DebugBackend::setTheme(const DebugTheme& theme)
	{
		m_theme = theme;
	}


	//____ theme() ___________________________________________________________

	const DebugTheme& DebugBackend::theme()
	{
		return m_theme;
	}

	//____ createObjectInfoSection() ____________________________________________________

	Widget_p DebugBackend::createObjectInfoSection(const TypeInfo * pType, Object * pObject )
	{

		auto it = m_objectInfoFactories.find( pType );
		if( it == m_objectInfoFactories.end() )
		{
			// Check known classes we should ignore.

			if (std::find(m_ignoreClasses.begin(), m_ignoreClasses.end(), pType) != m_ignoreClasses.end())
				return nullptr;

			// Unknown class

			return DummyInfoSection::create(m_theme,this, pType->className,pObject);
		}

		return it->second(m_theme,this,pObject);
	}

	//____ createSlotInfoSection() ____________________________________________________

	Widget_p DebugBackend::createSlotInfoSection(const TypeInfo * pType, StaticSlot * pSlot )
	{

		auto it = m_slotInfoFactories.find( pType );
		if( it == m_slotInfoFactories.end() )
		{
			// Check known classes we should ignore.

			if (std::find(m_ignoreClasses.begin(), m_ignoreClasses.end(), pType) != m_ignoreClasses.end())
				return nullptr;

			// Unknown class

			return DummyInfoSection::create(m_theme,this, pType->className,pSlot);
		}

		return it->second(m_theme,this,pSlot);
	}

	//____ createComponentInfoSection() ____________________________________________________

	Widget_p DebugBackend::createComponentInfoSection(const TypeInfo* pType, Component* pComponent)
	{

		auto it = m_componentInfoFactories.find(pType);
		if (it == m_componentInfoFactories.end())
		{
			// Check known classes we should ignore.

			if (std::find(m_ignoreClasses.begin(), m_ignoreClasses.end(), pType) != m_ignoreClasses.end())
				return nullptr;

			// Unknown class

			return DummyInfoSection::create(m_theme,this, pType->className,pComponent);
		}

		return it->second(m_theme, this, pComponent);
	}

	//____ createObjectInspector() ________________________________________________

	ObjectInspector_p DebugBackend::createObjectInspector(Object* pObject)
	{
		return ObjectInspector::create(m_theme, this, pObject);
	}

	//____ createWidgetInspector() ________________________________________________

	WidgetInspector_p DebugBackend::createWidgetInspector(Widget* pWidget)
	{
		return WidgetInspector::create(m_theme, this, pWidget);
	}

	//____ createSkinInspector() ________________________________________________

	SkinInspector_p DebugBackend::createSkinInspector(Skin* pSkin)
	{
		return SkinInspector::create(m_theme, this, pSkin);
	}

	//____ createWidgetTreeView() ____________________________________________

	WidgetTreeView_p DebugBackend::createWidgetTreeView(Widget* pRoot)
	{
		return WidgetTreeView::create(m_theme, this, pRoot);
	}

	//____ createMsgLogViewer() ________________________________________________

	MsgLogViewer_p DebugBackend::createMsgLogViewer()
	{
		return MsgLogViewer::create(m_theme, this);
	}

	//____ setObjectSelectedCallback() ________________________________________

	void DebugBackend::setObjectSelectedCallback(std::function<void(Object*,Object*)> pCallback)
	{
		m_objectSelectedCallback = pCallback;
	}

	//____ objectSelected() ____________________________________________________

	void DebugBackend::objectSelected(Object* pSelected, Object* pCaller)
	{
		if (m_objectSelectedCallback)
			m_objectSelectedCallback(pSelected, pCaller);
	}



} // namespace wg

