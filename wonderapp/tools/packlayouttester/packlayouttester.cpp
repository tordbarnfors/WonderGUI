
#include "packlayouttester.h"

#include <wondergui.h>

#include <wax_freetypefont.h>
#include <string>
#include <fstream>

using namespace wg;
using namespace wax;
using namespace std;

//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(wapp::API* pAPI)
{
	if (!_setupGUI(pAPI))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}

	_updateLayout();

	return true;
}

//____ update() _______________________________________________________________

bool MyApp::update()
{
	return m_pWindow != nullptr;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{
}

//____ closeWindow() __________________________________________________________

void MyApp::closeWindow(wapp::Window* pWindow)
{
	if (pWindow == m_pWindow)
		m_pWindow = nullptr;
}

//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(wapp::API* pAPI)
{
	m_pWindow = wapp::Window::create(pAPI, { .size = {800,700}, .title = "PackLayout Tester" });

	//

	auto pFontBlob = pAPI->loadBlob("resources/DroidSans.ttf");
	auto pFont = FreeTypeFont::create(pFontBlob);

	m_pTextStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 14,
									_.color = Color::Black,
									_.states = {{State::Disabled, Color::DarkGrey}} ));

	m_pLabelStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 16,
									_.color = Color::Black));


	wg::Base::setDefaultStyle(m_pTextStyle);

	m_pTextLayoutCentered = PlainTextLayout::create(WGBP(PlainTextLayout,
		_.placement = Placement::Center));


	//

	if (!_loadSkins(pAPI))
		return false;



		
	
	auto pBasePanel = FlexPanel::create();
	pBasePanel->setSkin(ColorSkin::create(Color::Beige));
	auto pControlPanel = _createControlPanel();
	auto pStretchPanel = _createStretchPanel();

	pBasePanel->slots.pushBack(pControlPanel, { .pin1 = Placement::NorthWest, .pin2 = FlexPos(Placement::NorthEast, Coord(0, 250)) });
	pBasePanel->slots.pushBack(pStretchPanel, { .pos = {2, 260}, .size = {500, 200} });

	m_pBasePanel = pBasePanel;

	m_pWindow->mainCapsule()->slot = pBasePanel;
	return true;
}

//____ _createControlPanel() _________________________________________________________________

Widget_p MyApp::_createControlPanel()
{
	auto pBase = FlexPanel::create();
	pBase->setSkin(m_pPlateSkin);



	m_pStartSizeSelector = _createDropdown();
	m_pStartSizeSelector->entries.pushBack({ 
		{ int(PackLayout::WantedSize::Default), "Default"},
		{ int(PackLayout::WantedSize::MaxDefault), "MaxDefault" },
		{ int(PackLayout::WantedSize::DefaultTimesWeight), "DefaultTimesWeight" },
		{ int(PackLayout::WantedSize::MaxDefaultTimesWeight), "MaxDefaultTimesWeight" },
	});

	m_pExpandFactorSelector = _createDropdown();
	m_pExpandFactorSelector->entries.pushBack({
		{ int(PackLayout::Factor::Zero), "Zero"},
		{ int(PackLayout::Factor::One), "One" },
		{ int(PackLayout::Factor::Margin), "Margin" },
		{ int(PackLayout::Factor::Size), "Size" },
		{ int(PackLayout::Factor::Weight), "Weight" },
		{ int(PackLayout::Factor::InverseWeight), "InverseWeight" },
		{ int(PackLayout::Factor::SizeTimesWeight), "SizeTimesWeight" },
		{ int(PackLayout::Factor::SizeTimesInverseWeight), "SizeTimesInverseWeight" },
		{ int(PackLayout::Factor::MarginTimesWeight), "MarginTimesWeight" },
		{ int(PackLayout::Factor::MarginTimesInverseWeight), "MarginTimesInverseWeight" },
		{ int(PackLayout::Factor::List1), "List1" },
		{ int(PackLayout::Factor::List2), "List2" },
		{ int(PackLayout::Factor::List3), "List3" },
		{ int(PackLayout::Factor::Ordered), "Ordered" },
		});

	m_pShrinkFactorSelector = _createDropdown();
	m_pShrinkFactorSelector->entries.pushBack({
		{ int(PackLayout::Factor::Zero), "Zero"},
		{ int(PackLayout::Factor::One), "One" },
		{ int(PackLayout::Factor::Margin), "Margin" },
		{ int(PackLayout::Factor::Size), "Size" },
		{ int(PackLayout::Factor::Weight), "Weight" },
		{ int(PackLayout::Factor::InverseWeight), "InverseWeight" },
		{ int(PackLayout::Factor::SizeTimesWeight), "SizeTimesWeight" },
		{ int(PackLayout::Factor::SizeTimesInverseWeight), "SizeTimesInverseWeight" },
		{ int(PackLayout::Factor::MarginTimesWeight), "MarginTimesWeight" },
		{ int(PackLayout::Factor::MarginTimesInverseWeight), "MarginTimesInverseWeight" },
		{ int(PackLayout::Factor::List1), "List1" },
		{ int(PackLayout::Factor::List2), "List2" },
		{ int(PackLayout::Factor::List3), "List3" },
		{ int(PackLayout::Factor::Ordered), "Ordered" },
		});

	m_pHideSelector = _createDropdown();
	m_pHideSelector->entries.pushBack({
		{ int(PackLayout::HideSetting::Never), "Never"},
		{ int(PackLayout::HideSetting::Restrained), "Restrained" },
		{ int(PackLayout::HideSetting::Aggressive), "Aggressive" },
		});

	m_pMinSizeSourceSelector = _createSizeSourceDropdown();
	m_pDefaultSizeSourceSelector = _createSizeSourceDropdown();
	m_pMaxSizeSourceSelector = _createSizeSourceDropdown();


	m_pList1Input = TextEditor::create({ .skin = m_pSectionSkin });
	m_pList2Input = TextEditor::create({ .skin = m_pSectionSkin });
	m_pList3Input = TextEditor::create({ .skin = m_pSectionSkin });

	m_pExpandOrderInput = TextEditor::create({ .skin = m_pSectionSkin });
	m_pShrinkOrderInput = TextEditor::create({ .skin = m_pSectionSkin });


	Coord pos(4, 22);
	Coord labelOffset(4, -18);

	pts xSpacing = 8;

	pts xSpacingSizeSelectors = 100;

	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "MinSize"} }), { .pos = pos + labelOffset } );
	pBase->slots.pushBack(m_pMinSizeSourceSelector, { .pos = pos } );

	pos.x += xSpacingSizeSelectors;
	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "DefaultSize"} }), { .pos = pos + labelOffset } );
	pBase->slots.pushBack(m_pDefaultSizeSourceSelector, { .pos = pos });

	pos.x += xSpacingSizeSelectors;
	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "MaxSize"} }), { .pos = pos + labelOffset } );
	pBase->slots.pushBack(m_pMaxSizeSourceSelector, { .pos = pos });

	pos.x = 4;
	pos.y += 50;

	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "Start size"} }), { .pos = pos + labelOffset } );
	pBase->slots.pushBack(m_pStartSizeSelector, { .pos = pos });

	pos.x += m_pStartSizeSelector->size().w + xSpacing;
	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "Expand factor"} }), { .pos = pos + labelOffset } );
	pBase->slots.pushBack(m_pExpandFactorSelector, { .pos = pos });

	pos.x += m_pExpandFactorSelector->size().w + xSpacing;
	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "Shrink factor"} }), { .pos = pos + labelOffset } );
	pBase->slots.pushBack(m_pShrinkFactorSelector, { .pos = pos });

	pos.x += m_pShrinkFactorSelector->size().w + xSpacing;
	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "Hide setting"} }), { .pos = pos + labelOffset } );
	pBase->slots.pushBack(m_pHideSelector, { .pos = pos });

	pos.x = 4;
	pos.y += 40;
	pts inputFieldOfsX = 90;
	pts inputFieldSpacingY = 24;

	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "List 1"} }), { .pos = pos });
	pBase->slots.pushBack(m_pList1Input, { .pin1 = { Placement::NorthWest, {pos.x + inputFieldOfsX,pos.y } }, .pin2 = { Placement::NorthEast, {-10,pos.y + 20} }});

	pos.y += inputFieldSpacingY;
	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "List 2"} }), { .pos = pos });
	pBase->slots.pushBack(m_pList2Input, { .pin1 = { Placement::NorthWest, {pos.x + inputFieldOfsX,pos.y } }, .pin2 = { Placement::NorthEast, {-10,pos.y + 20} }});

	pos.y += inputFieldSpacingY;
	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "List 3"} }), { .pos = pos });
	pBase->slots.pushBack(m_pList3Input, { .pin1 = { Placement::NorthWest, {pos.x + inputFieldOfsX,pos.y } }, .pin2 = { Placement::NorthEast, {-10,pos.y + 20} }});

	pos.y += inputFieldSpacingY;
	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "Expand order"} }), { .pos = pos });
	pBase->slots.pushBack(m_pExpandOrderInput, { .pin1 = { Placement::NorthWest, {pos.x + inputFieldOfsX,pos.y } }, .pin2 = { Placement::NorthEast, {-10,pos.y + 20} }});

	pos.y += inputFieldSpacingY;
	pBase->slots.pushBack(TextDisplay::create({ .display = {.text = "Shrink order"} }), { .pos = pos });
	pBase->slots.pushBack(m_pShrinkOrderInput, { .pin1 = { Placement::NorthWest, {pos.x + inputFieldOfsX,pos.y } }, .pin2 = { Placement::NorthEast, {-10,pos.y + 20} }});


	m_pStartSizeSelector->selectEntryById(int(PackLayout::WantedSize::Default));
	m_pExpandFactorSelector->selectEntryById(int(PackLayout::Factor::Weight));
	m_pShrinkFactorSelector->selectEntryById(int(PackLayout::Factor::Size));
	m_pHideSelector->selectEntryById(int(PackLayout::HideSetting::Never));
	m_pMinSizeSourceSelector->selectEntryById(int(PackLayout::SizeSource::Item));
	m_pDefaultSizeSourceSelector->selectEntryById(int(PackLayout::SizeSource::Item));
	m_pMaxSizeSourceSelector->selectEntryById(int(PackLayout::SizeSource::Item));


	auto pRouter = wg::Base::msgRouter();

	pRouter->addRoute(m_pStartSizeSelector, MsgType::Select, [this](Msg* pMsg) { this->_updateLayout();  });
	pRouter->addRoute(m_pExpandFactorSelector, MsgType::Select, [this](Msg* pMsg) { this->_updateLayout();  });
	pRouter->addRoute(m_pShrinkFactorSelector, MsgType::Select, [this](Msg* pMsg) { this->_updateLayout();  });
	pRouter->addRoute(m_pHideSelector, MsgType::Select, [this](Msg* pMsg) { this->_updateLayout();  });
	pRouter->addRoute(m_pMinSizeSourceSelector, MsgType::Select, [this](Msg* pMsg) { this->_updateLayout();  });
	pRouter->addRoute(m_pDefaultSizeSourceSelector, MsgType::Select, [this](Msg* pMsg) { this->_updateLayout();  });
	pRouter->addRoute(m_pMaxSizeSourceSelector, MsgType::Select, [this](Msg* pMsg) { this->_updateLayout();  });



	return pBase;
}

//____ _createDropdown() ____________________________________________________________________

Dropdown_p MyApp::_createDropdown( )
{
	auto pWidget = Dropdown::create();
	pWidget->setSkin(m_pButtonSkin);
	pWidget->setListSkin(m_pPlateSkin);
	return pWidget;
}

//____ _createSizeSourceDropdown() ____________________________________________________________________

Dropdown_p MyApp::_createSizeSourceDropdown()
{
	auto pWidget = Dropdown::create();
	pWidget->setSkin(m_pButtonSkin);
	pWidget->setListSkin(m_pPlateSkin);

	pWidget->entries.pushBack({
		{ int(PackLayout::SizeSource::Item), "Item"},
		{ int(PackLayout::SizeSource::Weight), "Weight" },
		{ int(PackLayout::SizeSource::List1), "List 1" },
		{ int(PackLayout::SizeSource::List2), "List 2" },
		{ int(PackLayout::SizeSource::List3), "List 3" },
		});

	return pWidget;
}




//____ _createStretchPanel() _________________________________________________________________

Widget_p MyApp::_createStretchPanel()
{
	auto pBase = FlexPanel::create();

	auto pPackPanel = PackPanel::create();
	pPackPanel->setSkin(ColorSkin::create(Color::LightGrey));

	auto pItemSkin = BoxSkin::create(1, Color::AntiqueWhite, Color::Black, 2);

	for (int i = 0; i < 5; i++)
		pPackPanel->slots << TextEditor::create({ .skin = pItemSkin } );


	pBase->slots.pushBack(pPackPanel, { .pin1 = Placement::NorthWest, .pin2 = FlexPos(Placement::SouthEast, Coord(-8, 0)) });

	auto pHandle = Filler::create({ .skin = m_pTightButtonSkin });

	pBase->slots.pushBack(pHandle, { .pin1 = {Placement::NorthEast, {-8,0}}, .pin2 = Placement::SouthEast });

	wg::Base::msgRouter()->addRoute(pHandle, MsgType::MousePress, [this,pBase](Msg* _pMsg) {
			auto pMsg = static_cast<MousePressMsg*>(_pMsg);
			this->m_dragStartSize = pBase->size();
		});

	wg::Base::msgRouter()->addRoute(pHandle, MsgType::MouseDrag, [this,pBase](Msg* _pMsg) {
		auto pMsg = static_cast<MouseDragMsg*>(_pMsg);

		Coord dragged = pMsg->draggedTotal();

		Size sz = m_dragStartSize + Size(dragged.x,dragged.y);

		sz.w = std::max(8.f, sz.w);
		sz.h = std::max(1.f, sz.h);
		
		this->m_pBasePanel->slots[1].setSize(sz);
		});

	m_pPackPanel = pPackPanel;
	return pBase;
}


//____ _loadSkins() ___________________________________________________________

bool MyApp::_loadSkins(wapp::API * pAPI)
{
	string path = "resources/greyskin/";

	auto pPlateBitmap = pAPI->loadBitmap(path + "plate.bmp");
	auto pButtonBitmap = pAPI->loadBitmap(path + "button.bmp");
	auto pStateButtonBitmap = pAPI->loadBitmap(path + "state_button.bmp");
	auto pCheckBoxBitmap = pAPI->loadBitmap(path + "checkbox.png");

	if (!pPlateBitmap || !pButtonBitmap || !pStateButtonBitmap)
		return false;

	m_pPlateSkin = BlockSkin::create(WGBP(BlockSkin,
		_.bitmap = pPlateBitmap,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = 4
	));

	m_pButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.bitmap = pButtonBitmap,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = { 4,10,4,10 },
		_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
	));

	m_pTightButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.bitmap = pButtonBitmap,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = { 4,4,4,4 },
		_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
	));


	m_pToggleButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.bitmap = pStateButtonBitmap,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = 4,
		_.states = { State::Default, State::Hovered, State::Checked, State::Checked + State::Hovered, State::Disabled }
	));

	m_pCheckBoxSkin = BlockSkin::create(WGBP(BlockSkin,
		_.bitmap = pCheckBoxBitmap,
		_.axis = Axis::Y,
		_.frame = 3,
//		_.defaultSize = { 12,12 },
		_.states = { State::Default, State::Checked }
	));

	m_pSectionSkin = BoxSkin::create(WGBP(BoxSkin,
		_.color = Color::White,
		_.outlineThickness = 1,
		_.outlineColor = Color::Black,
		_.padding = { 2,2,2,2 }
	));


	return true;
}

//____ _updateLayout() ________________________________________________________

void MyApp::_updateLayout()
{
	PackLayout::Blueprint bp;

	bp.wantedSize = (PackLayout::WantedSize) m_pStartSizeSelector->selectedEntryId();
	bp.expandFactor = (PackLayout::Factor) m_pExpandFactorSelector->selectedEntryId();
	bp.shrinkFactor = (PackLayout::Factor) m_pShrinkFactorSelector->selectedEntryId();
	bp.hideSetting = (PackLayout::HideSetting) m_pHideSelector->selectedEntryId();

	bp.minSizeSource = (PackLayout::SizeSource)m_pMinSizeSourceSelector->selectedEntryId();
	bp.defaultSizeSource = (PackLayout::SizeSource)m_pDefaultSizeSourceSelector->selectedEntryId();
	bp.maxSizeSource = (PackLayout::SizeSource)m_pMaxSizeSourceSelector->selectedEntryId();


	/*
	wg::TextEditor_p	m_pList1Input;
	wg::TextEditor_p	m_pList2Input;
	wg::TextEditor_p	m_pList3Input;

	wg::TextEditor_p	m_pExpandOrderInput;
	wg::TextEditor_p	m_pShrinkOrderInput;
	*/



	m_pLayout = PackLayout::create(bp);
	m_pPackPanel->setLayout(m_pLayout);

}
