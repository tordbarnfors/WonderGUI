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
#include <wg_togglebuttoninfosection.h>
#include <wg_knobinfosection.h>
#include <wg_sliderinfosection.h>
#include <wg_rangesliderinfosection.h>
#include <wg_meterinfosection.h>
#include <wg_scrollbarinfosection.h>
#include <wg_pianokeyboardinfosection.h>
#include <wg_lineeditorinfosection.h>
#include <wg_numberdisplayinfosection.h>
#include <wg_fpsdisplayinfosection.h>
#include <wg_animplayerinfosection.h>
#include <wg_canvasdisplayinfosection.h>
#include <wg_edgemapdisplayinfosection.h>
#include <wg_imageinfosection.h>
#include <wg_nodewiresinfosection.h>
#include <wg_popupopenerinfosection.h>
#include <wg_selectboxinfosection.h>
#include <wg_sidecanvasinfosection.h>
#include <wg_skindisplayinfosection.h>
#include <wg_surfacedisplayinfosection.h>
#include <wg_timerinfosection.h>

#include <wg_chartinfosection.h>
#include <wg_areachartinfosection.h>
#include <wg_plotchartinfosection.h>
#include <wg_scrollchartinfosection.h>
#include <wg_areascrollchartinfosection.h>

#include <wg_containerinfosection.h>

#include <wg_panelinfosection.h>
#include <wg_packpanelinfosection.h>
#include <wg_flexpanelinfosection.h>
#include <wg_scrollpanelinfosection.h>
#include <wg_twoslotpanelinfosection.h>
#include <wg_drawerpanelinfosection.h>
#include <wg_splitpanelinfosection.h>
#include <wg_tablepanelinfosection.h>
#include <wg_lambdapanelinfosection.h>
#include <wg_nodepanelinfosection.h>
#include <wg_stackpanelinfosection.h>

#include <wg_overlayinfosection.h>
#include <wg_dragndropoverlayinfosection.h>
#include <wg_modaloverlayinfosection.h>
#include <wg_popupoverlayinfosection.h>
#include <wg_tooltipoverlayinfosection.h>

#include <wg_capsuleinfosection.h>
#include <wg_sizecapsuleinfosection.h>
#include <wg_reordercapsuleinfosection.h>
#include <wg_selectcapsuleinfosection.h>
#include <wg_labelcapsuleinfosection.h>
#include <wg_paddingcapsuleinfosection.h>
#include <wg_renderlayercapsuleinfosection.h>
#include <wg_scalecapsuleinfosection.h>
#include <wg_canvascapsuleinfosection.h>
#include <wg_blockingcapsuleinfosection.h>
#include <wg_scrollcapsuleinfosection.h>


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


	//____ constructor _____________________________________________________________

	DebugBackend::DebugBackend()
	{
		_registerObject<Object, ObjectInfoSection>();
		_registerObject<Widget, WidgetInfoSection>();
		_registerObject<Filler, FillerInfoSection>();
		_registerObject<TextDisplay, TextDisplayInfoSection>();
		_registerObject<TextEditor, TextEditorInfoSection>();
		_registerObject<Button, ButtonInfoSection>();
		_registerObject<ToggleButton, ToggleButtonInfoSection>();
		_registerObject<Knob, KnobInfoSection>();
		_registerObject<Slider, SliderInfoSection>();
		_registerObject<RangeSlider, RangeSliderInfoSection>();
		_registerObject<Meter, MeterInfoSection>();
		_registerObject<Scrollbar, ScrollbarInfoSection>();
		_registerObject<PianoKeyboard, PianoKeyboardInfoSection>();
		_registerObject<LineEditor, LineEditorInfoSection>();
		_registerObject<NumberDisplay, NumberDisplayInfoSection>();
		_registerObject<FpsDisplay, FpsDisplayInfoSection>();
		_registerObject<AnimPlayer, AnimPlayerInfoSection>();
		_registerObject<CanvasDisplay, CanvasDisplayInfoSection>();
		_registerObject<EdgemapDisplay, EdgemapDisplayInfoSection>();
		_registerObject<Image, ImageInfoSection>();
		_registerObject<NodeWires, NodeWiresInfoSection>();
		_registerObject<PopupOpener, PopupOpenerInfoSection>();
		_registerObject<SelectBox, SelectBoxInfoSection>();
		_registerObject<SideCanvas, SideCanvasInfoSection>();
		_registerObject<SkinDisplay, SkinDisplayInfoSection>();
		_registerObject<SurfaceDisplay, SurfaceDisplayInfoSection>();
		_registerObject<Timer, TimerInfoSection>();

		_registerObject<Chart, ChartInfoSection>();
		_registerObject<AreaChart, AreaChartInfoSection>();
		_registerObject<PlotChart, PlotChartInfoSection>();
		_registerObject<ScrollChart, ScrollChartInfoSection>();
		_registerObject<AreaScrollChart, AreaScrollChartInfoSection>();

		_registerObject<Container, ContainerInfoSection>();
		_registerObject<Panel, PanelInfoSection>();
		_registerObject<PackPanel, PackPanelInfoSection>();
		_registerObject<FlexPanel, FlexPanelInfoSection>();
		_registerObject<TwoSlotPanel, TwoSlotPanelInfoSection>();
		_registerObject<ScrollPanel, ScrollPanelInfoSection>();
		_registerObject<DrawerPanel, DrawerPanelInfoSection>();
		_registerObject<SplitPanel, SplitPanelInfoSection>();
		_registerObject<TablePanel, TablePanelInfoSection>();
		_registerObject<LambdaPanel, LambdaPanelInfoSection>();
		_registerObject<NodePanel, NodePanelInfoSection>();
		_registerObject<StackPanel, StackPanelInfoSection>();

		_registerObject<Overlay, OverlayInfoSection>();
		_registerObject<DragNDropOverlay, DragNDropOverlayInfoSection>();
		_registerObject<ModalOverlay, ModalOverlayInfoSection>();
		_registerObject<PopupOverlay, PopupOverlayInfoSection>();
		_registerObject<TooltipOverlay, TooltipOverlayInfoSection>();

		_registerObject<Capsule, CapsuleInfoSection>();
		_registerObject<SizeCapsule, SizeCapsuleInfoSection>();
		_registerObject<ReorderCapsule, ReorderCapsuleInfoSection>();
		_registerObject<SelectCapsule, SelectCapsuleInfoSection>();
		_registerObject<LabelCapsule, LabelCapsuleInfoSection>();
		_registerObject<PaddingCapsule, PaddingCapsuleInfoSection>();
		_registerObject<RenderLayerCapsule, RenderLayerCapsuleInfoSection>();
		_registerObject<ScaleCapsule, ScaleCapsuleInfoSection>();
		_registerObject<CanvasCapsule, CanvasCapsuleInfoSection>();
		_registerObject<BlockingCapsule, BlockingCapsuleInfoSection>();
		_registerObject<ScrollCapsule, ScrollCapsuleInfoSection>();

		_registerObject<Skin, SkinInfoSection>();

		_registerSlot<StaticSlot, StaticSlotInfoSection>();
		_registerSlot<PanelSlot, PanelSlotInfoSection>();
		_registerSlot<PackPanelSlot, PackPanelSlotInfoSection>();
		_registerSlot<FlexPanelSlot, FlexPanelSlotInfoSection>();
		_registerSlot<TwoSlotPanel::Slot, TwoSlotPanelSlotInfoSection>();

		_registerComponent<StaticText, StaticTextInfoSection>();
		_registerComponent<EditableText, EditableTextInfoSection>();
		_registerComponent<Icon, IconInfoSection>();
		_registerComponent<Scroller, ScrollerInfoSection>();
		_registerComponent<Glow, GlowInfoSection>();

		_ignore<DynamicSlot>();
		_ignore<Receiver>();
		_ignore<Component>();
		_ignore<DynamicText>();
		_ignore<StateSkin>();
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

	//____ _addInfoSections() __________________________________________________
	//
	// Walks the class chain of pInspected from most derived to Object and adds
	// one info section per class to pPanel. Classes on the ignore list are
	// skipped, classes without a registered section get a DummyInfoSection.

	template<class T>
	void DebugBackend::_addInfoSections(PackPanel* pPanel, T* pInspected, const std::map<const TypeInfo*, InfoSectionFactory<T>>& factories)
	{
		for( const TypeInfo * pType = &pInspected->typeInfo() ; pType != nullptr ; pType = pType->pSuperClass )
		{
			auto pSection = _createInfoSection(pType, pInspected, factories);
			if( pSection )
				pPanel->slots << pSection;
		}
	}

	//____ _createInfoSection() ________________________________________________

	template<class T>
	Widget_p DebugBackend::_createInfoSection(const TypeInfo* pType, T* pInspected, const std::map<const TypeInfo*, InfoSectionFactory<T>>& factories)
	{
		auto it = factories.find(pType);
		if( it != factories.end() )
			return it->second(m_theme, this, pInspected);

		if (std::find(m_ignoreClasses.begin(), m_ignoreClasses.end(), pType) != m_ignoreClasses.end())
			return nullptr;

		return DummyInfoSection::create(m_theme, this, pType->className, pInspected);
	}

	//____ addObjectInfoSections() _____________________________________________

	void DebugBackend::addObjectInfoSections(PackPanel* pPanel, Object* pObject)
	{
		_addInfoSections(pPanel, pObject, m_objectInfoFactories);
	}

	//____ addSlotInfoSections() _______________________________________________

	void DebugBackend::addSlotInfoSections(PackPanel* pPanel, StaticSlot* pSlot)
	{
		_addInfoSections(pPanel, pSlot, m_slotInfoFactories);
	}

	//____ addComponentInfoSections() __________________________________________

	void DebugBackend::addComponentInfoSections(PackPanel* pPanel, Component* pComponent)
	{
		_addInfoSections(pPanel, pComponent, m_componentInfoFactories);
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

