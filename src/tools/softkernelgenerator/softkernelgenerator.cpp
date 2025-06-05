
#include "softkernelgenerator.h"

#include <wg_freetypefont.h>
#include <string>
#include <fstream>

using namespace wg;
using namespace std;

//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(Visitor* pVisitor)
{
	m_pDB = new KernelDB();
	m_pVisitor = pVisitor;

	if (!_setupGUI(pVisitor))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}

	return true;
}

//____ update() _______________________________________________________________

bool MyApp::update()
{
	return true;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{
	delete m_pDB;
}


//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(Visitor* pVisitor)
{
	m_pWindow = pVisitor->createWindow({ .size = {800,700}, .title = "SoftKernel Generator" });

	auto pRoot = m_pWindow->rootPanel();

	//

	auto pFontBlob = pVisitor->loadBlob("resources/DroidSans.ttf");
	auto pFont = FreeTypeFont::create(pFontBlob);

	m_pTextStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 14,
									_.color = Color8::Black,
									_.states = {{State::Disabled, Color8::DarkGrey}} ));

	m_pLabelStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 16,
									_.color = Color8::Black));


	Base::setDefaultStyle(m_pTextStyle);

	m_pTextLayoutCentered = BasicTextLayout::create(WGBP(BasicTextLayout,
		_.placement = Placement::Center));


	//

	if (!_loadSkins(pVisitor))
		return false;

	auto pBasePanel = PackPanel::create( { .axis = Axis::Y });

	auto pWindow = ScrollPanel::create();
	pWindow->setSkin(ColorSkin::create(Color8::AntiqueWhite));
	pWindow->scrollbarY.setBackground(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color8::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black)));
	pWindow->scrollbarY.setBar(m_pPlateSkin);
	pWindow->setAutohideScrollbars(true, true);
	pWindow->setSizeConstraints(SizeConstraint::Equal, SizeConstraint::GreaterOrEqual);
	pWindow->slot = _buildList();
		
	pBasePanel->slots << _buildButtonRow();
	pBasePanel->slots << pWindow;
	pBasePanel->setSlotWeight(0, 1, 0.f);

	pRoot->slot = pBasePanel;

	m_pScrollPanel = pWindow;

	return true;
}


//____ tintModeToggled() ______________________________________________________

void MyApp::tintModeToggled(Msg* _pMsg)
{
	auto pMsg = static_cast<ToggleMsg*>(_pMsg);
	auto pWidget = static_cast<Widget*>(pMsg->sourceRawPtr());

	m_pDB->setTintMode((TintMode)pWidget->id(), pMsg->isSet());
	_refreshSummary();
}

//____ blendModeToggled() ______________________________________________________

void MyApp::blendModeToggled(Msg* _pMsg)
{
	auto pMsg = static_cast<ToggleMsg*>(_pMsg);
	auto pWidget = static_cast<Widget*>(pMsg->sourceRawPtr());

	m_pDB->setBlendMode((BlendMode)pWidget->id(), pMsg->isSet());
	_refreshSummary();
}

//____ sourceFormatToggled() ______________________________________________________

void MyApp::sourceFormatToggled(Msg* _pMsg)
{
	auto pMsg = static_cast<ToggleMsg*>(_pMsg);
	auto pWidget = static_cast<Widget*>(pMsg->sourceRawPtr());

	m_pDB->setSrcFormat((PixelFormat)pWidget->id(), pMsg->isSet());
	_refreshSummary();
}

//____ destFormatToggled() ______________________________________________________

void MyApp::destFormatToggled(Msg* _pMsg)
{
	auto pMsg = static_cast<ToggleMsg*>(_pMsg);
	auto pWidget = static_cast<Widget*>(pMsg->sourceRawPtr());

	m_pDB->setDestFormat((PixelFormat)pWidget->id(), pMsg->isSet());
	_refreshSummary();
}

//____ customBlitTypeToggled() __________________________________________

void MyApp::customBlitTypeToggled(int index, wg::Msg* _pMsg)
{
	auto pMsg = static_cast<ToggleMsg*>(_pMsg);
	auto pWidget = static_cast<Widget*>(pMsg->sourceRawPtr());

	m_pDB->customBlitEntry(index)->blitTypes[pWidget->id()] = pMsg->isSet();
	_refreshList();
}

//____ customBlitTintModeToggled() __________________________________________

void MyApp::customBlitTintModeToggled(int index, wg::Msg* _pMsg)
{
	auto pMsg = static_cast<ToggleMsg*>(_pMsg);
	auto pWidget = static_cast<Widget*>(pMsg->sourceRawPtr());

	m_pDB->customBlitEntry(index)->tintModes[pWidget->id()] = pMsg->isSet();
	_refreshList();
}

//____ customBlitBlendModeToggled() __________________________________________

void MyApp::customBlitBlendModeToggled(int index, wg::Msg* _pMsg)
{
	auto pMsg = static_cast<ToggleMsg*>(_pMsg);
	auto pWidget = static_cast<Widget*>(pMsg->sourceRawPtr());

	m_pDB->customBlitEntry(index)->blendModes[pWidget->id()] = pMsg->isSet();
	_refreshList();
}

//____ customBlitSourceFormatToggled() __________________________________________

void MyApp::customBlitSourceFormatToggled(int index, wg::Msg* _pMsg)
{
	auto pMsg = static_cast<ToggleMsg*>(_pMsg);
	auto pWidget = static_cast<Widget*>(pMsg->sourceRawPtr());

	m_pDB->customBlitEntry(index)->sourceFormats[pWidget->id()] = pMsg->isSet();
	_refreshList();
}

//____ customBlitDestFormatToggled() __________________________________________

void MyApp::customBlitDestFormatToggled(int index, wg::Msg* _pMsg)
{
	auto pMsg = static_cast<ToggleMsg*>(_pMsg);
	auto pWidget = static_cast<Widget*>(pMsg->sourceRawPtr());

	m_pDB->customBlitEntry(index)->destFormats[pWidget->id()] = pMsg->isSet();
	_refreshList();
}


//____ eraseCustomBlitEntry() _______________________________________________

void MyApp::eraseCustomBlitEntry(int index)
{
	m_pDB->deleteCustomBlitEntry(index);
	_refreshList();
}

//____ exportSource() _______________________________________________________

bool MyApp::exportSource()
{
	auto path = m_pVisitor->saveFileDialog("Export Kernels", "", {}, "" );
	
	if( path.empty() )
		return false;
	
	String kernelLabel = m_pKernelFuncNameEditor->editor.text();
	
	auto labelString = CharSeq(kernelLabel).getStdString();
	
	// Export source
	
	ofstream	myStream;
	myStream.open(path + ".cpp");
	m_pDB->generateSource(myStream, labelString);
	myStream.close();
	
	// Export CPP header
	
	myStream.open(path + ".h");

	myStream << "#pragma once" << endl;
	myStream << "#include <wg_softbackend.h>" << endl << endl;
	myStream << "namespace wg" << endl;
	myStream << "{" << endl;
	myStream << "	bool " << labelString << "( SoftBackend * pBackend );" << endl;
	myStream << "};" << endl;
	
	myStream.close();
	
	// Export C header
	
	myStream.open(path + "_c.h");

	myStream << "#ifdef __cplusplus" << endl;
	myStream << "extern \"C\" {" << endl;
	myStream << "#endif" << endl;
	myStream << "	int	wg_" << labelString << "( wg_obj backend );" << endl;
	myStream << "#ifdef __cplusplus" << endl;
	myStream << "}" << endl;
	myStream << "#endif" << endl;
	
	myStream.close();

	
	return true;
}

//____ addOptimizedBlitEntry() ________________________________________________

void MyApp::addOptimizedBlitEntry()
{
	m_pDB->addCustomBlitEntry();
	_refreshList();
}


//____ clear() _________________________________________________________________

void MyApp::clear()
{
	m_pDB->clear();
	_refreshList();
}

//____ reset() ________________________________________________________________

void MyApp::reset()
{
	m_pDB->reset();
	_refreshList();
}

//____ load() _________________________________________________________________

void MyApp::load()
{

}

//____ save() _________________________________________________________________

void MyApp::save()
{

}


//____ _loadSkins() ___________________________________________________________

bool MyApp::_loadSkins(Visitor * pVisitor)
{
	string path = "resources/greyskin/";

	auto pPlateSurf = pVisitor->loadSurface(path + "plate.bmp");
	auto pButtonSurf = pVisitor->loadSurface(path + "button.bmp");
	auto pStateButtonSurf = pVisitor->loadSurface(path + "state_button.bmp");
	auto pCheckBoxSurf = pVisitor->loadSurface(path + "checkbox.png");

	if (!pPlateSurf || !pButtonSurf || !pStateButtonSurf)
		return false;

	m_pPlateSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pPlateSurf,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = 4
	));

	m_pButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pButtonSurf,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = { 4,6,4,6 },
		_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
	));

	m_pToggleButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pStateButtonSurf,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = 4,
		_.states = { State::Default, State::Hovered, State::Checked, State::Checked + State::Hovered, State::Disabled }
	));

	m_pCheckBoxSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pCheckBoxSurf,
		_.axis = Axis::Y,
		_.frame = 3,
//		_.defaultSize = { 12,12 },
		_.states = { State::Default, State::Checked }
	));

	m_pSectionSkin = BoxSkin::create(WGBP(BoxSkin,
		_.color = HiColor::White,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black,
		_.padding = { 4,2,4,2 }
	));

	m_pInputBoxSkin = BoxSkin::create(WGBP(BoxSkin,
		_.color = HiColor::White,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Grey,
		_.padding = { 4,2,4,2 }
	));


	return true;
}


//____ _refreshSummary() _______________________________________________________

void MyApp::_refreshSummary()
{
	_refreshList();
	//	m_pList->slots.erase(2, 1);
	//	m_pList->slots.insert(2, _buildListSummarySection() );
}

//____ _refreshList() _________________________________________________________

void MyApp::_refreshList()
{
	m_pScrollPanel->slot = _buildList();
}



//____ _buildButtonRow() ______________________________________________________

Widget_p	MyApp::_buildButtonRow()
{
	auto pButtonRow = PackPanel::create({ .axis = Axis::X, .skin = m_pPlateSkin });
	pButtonRow->setSpacing(2, 4, 2);

	auto pClearButton = WGCREATE(Button,
		_.skin = m_pButtonSkin,
		_.label.text = "Clear");

	/*
		auto pClearButton = Button::create(WGBP(Button,
			_.skin = m_pButtonSkin,
			_.label.text = "Clear"));
	*/
	auto pResetButton = Button::create(WGBP(Button,
		_.skin = m_pButtonSkin,
		_.label.text = "Reset"));

	auto pLoadButton = Button::create(WGBP(Button,
		_.skin = m_pButtonSkin,
		_.label.text = "Load",
		_.disabled = true));
	/*
		auto pSaveButton = Button::create(WGBP(Button,
			_.skin = m_pButtonSkin,
			_.label.text = "Save",
			_.disabled = true ));
	*/

	auto pSaveButton = Button::create({
		.disabled = true,
		.label = { .text = "Save" },
		.skin = m_pButtonSkin
	});


	Base::msgRouter()->addRoute( pClearButton, MsgType::Select, [this](Msg*) {this->clear(); });
	Base::msgRouter()->addRoute( pResetButton, MsgType::Select, [this](Msg*) {this->reset(); });
	Base::msgRouter()->addRoute( pLoadButton, MsgType::Select, [this](Msg*) {this->load(); });
	Base::msgRouter()->addRoute( pSaveButton, MsgType::Select, [this](Msg*) {this->save(); });


	pButtonRow->slots << pClearButton;
	pButtonRow->slots << pResetButton;
	pButtonRow->slots << pLoadButton;
	pButtonRow->slots << pSaveButton;

	pButtonRow->setSlotWeight(pButtonRow->slots.begin(), pButtonRow->slots.end(), 0.f);

	return pButtonRow;
}


//____ _buildList() ___________________________________________________________

Widget_p MyApp::_buildList()
{
	auto pList = PackPanel::create( { .axis = Axis::Y, .layout = m_pNoExpandLayout } );

	pList->slots << _buildGlobalSettingsSection();
	pList->slots << _buildOptimizedBlitsSection();
	pList->slots << _buildListSummarySection();
	pList->slots << _buildExportSection();

	pList->setSlotWeight(pList->slots.begin(), pList->slots.end(), 0.f);
	
	m_pList = pList;

	return pList;
}

//____ _buildToggleButtonRow() _________________________________________________


PackPanel_p MyApp::_buildToggleButtonRow(std::string title, std::vector<KernelDB::BlitType> blitTypes, const bool selected[], std::function<void(wg::Msg*)> pressCallback)
{
	auto pColumn = PackPanel::create( { .axis = Axis::Y, .layout = m_pNoExpandLayout } );
	pColumn->slots << TextDisplay::create(WGBP(TextDisplay,
		_.display.text = title,
		_.display.style = m_pLabelStyle));

	const static string labels[] = {	"Straight Blit", "Straight Tile", "Straight Blur", 
										"Trans Blit Nearest", "Trans Clip Blit Nearest", "Trans Tile Nearest", "Trans Blur Nearest",
										"Trans Blit Bilinear", "Trans Clip Blit Bilinear", "Trans Tile Bilinear", "Trans Blur Bilinear" };

	for (auto blitType : blitTypes)
	{
		auto pWidget = ToggleButton::create(WGBP(ToggleButton,
			_.id = (int)blitType;
		_.icon.skin = m_pCheckBoxSkin,
			_.icon.padding = { 0,4,0,0 },
			_.label.text = labels[int(blitType)],
			_.selected = selected[int(blitType)]));

		auto pObj = this;
		Base::msgRouter()->addRoute(pWidget, MsgType::Toggle, pressCallback);
		pColumn->slots << pWidget;
	}

	pColumn->setSlotWeight(pColumn->slots.begin(), pColumn->slots.end(), 0.f);
	return pColumn;
}


PackPanel_p MyApp::_buildToggleButtonRow(string title, std::vector<BlendMode> blendModes, const bool selected[], function<void(Msg*)> pressCallback )
{
	auto pColumn = PackPanel::create( { .axis = Axis::Y, .layout = m_pNoExpandLayout } );
	pColumn->slots << TextDisplay::create(WGBP(TextDisplay,
			_.display.text = title,
			_.display.style = m_pLabelStyle));

	for (auto blendMode : blendModes)
	{
		auto pWidget = ToggleButton::create(WGBP(ToggleButton,
			_.id = (int)blendMode;
			_.icon.skin = m_pCheckBoxSkin,
			_.icon.padding = { 0,4,0,0 },
			_.label.text = toString(blendMode),
			_.selected = selected[int(blendMode)] ));

		auto pObj = this;
		Base::msgRouter()->addRoute(pWidget, MsgType::Toggle, pressCallback );
		pColumn->slots << pWidget;
	}

	pColumn->setSlotWeight(pColumn->slots.begin(), pColumn->slots.end(), 0.f);
	return pColumn;
}

PackPanel_p MyApp::_buildToggleButtonRow(string title, std::vector<TintMode> tintModes, const bool selected[], function<void(Msg*)> pressCallback)
{
	auto pColumn = PackPanel::create({ .axis = Axis::Y, .layout = m_pNoExpandLayout });
	pColumn->slots << TextDisplay::create(WGBP(TextDisplay,
		_.display.text = title,
		_.display.style = m_pLabelStyle));


	for (auto tintMode : tintModes)
	{
		auto pWidget = ToggleButton::create(WGBP(ToggleButton,
			_.id = (int)tintMode;
			_.icon.skin = m_pCheckBoxSkin,
			_.icon.padding = { 0,4,0,0 },
			_.label.text = toString(tintMode),
			_.selected = selected[int(tintMode)] ));

		auto pObj = this;
		Base::msgRouter()->addRoute(pWidget, MsgType::Toggle, pressCallback );
		pColumn->slots << pWidget;
	}

	pColumn->setSlotWeight(pColumn->slots.begin(), pColumn->slots.end(), 0.f);
	return pColumn;
}

PackPanel_p MyApp::_buildToggleButtonRow(string title, std::vector<PixelFormat> formats, const bool selected[], function<void(Msg*)> pressCallback)
{
	auto pColumn = PackPanel::create({ .axis = Axis::Y, .layout = m_pNoExpandLayout });
	pColumn->slots << TextDisplay::create(WGBP(TextDisplay,
		_.display.text = title,
		_.display.style = m_pLabelStyle));

	for (auto format : formats)
	{
		auto pWidget = ToggleButton::create(WGBP(ToggleButton,
			_.id = (int)format;
			_.icon.skin = m_pCheckBoxSkin,
			_.icon.padding = { 0,4,0,0 },
			_.label.text = toString(format),
			_.selected = selected[int(format)]));

		auto pObj = this;
		Base::msgRouter()->addRoute(pWidget, MsgType::Toggle, pressCallback );
		pColumn->slots << pWidget;
	}

	pColumn->setSlotWeight(pColumn->slots.begin(), pColumn->slots.end(), 0.f);
	return pColumn;
}


//____ _buildGlobalSettingsSection() __________________________________________

Widget_p MyApp::_buildGlobalSettingsSection()
{
	auto pTopSection = PackPanel::create( { .axis = Axis::X, .skin = m_pSectionSkin, .spacing = 10, .spacingAfter = 5, .spacingBefore = 5 } );

	// Togglebuttons for TintModes

	auto pColumn = _buildToggleButtonRow("Tint Modes", { TintMode::None, TintMode::Flat, TintMode::GradientX, TintMode::GradientY, TintMode::GradientXY },
		m_pDB->tintModes(), [this](Msg* pMsg) {this->tintModeToggled(pMsg); });

	for (auto& p : pColumn->slots)
		if (p->id() == int(TintMode::None) )
			p->setDisabled(true);

	pTopSection->slots << pColumn;

	// Togglebuttons for BlendModes

	pColumn = _buildToggleButtonRow("Blend Modes", { BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::Subtract, BlendMode::Multiply, BlendMode::Max, BlendMode::Min, BlendMode::Invert, BlendMode::Morph, BlendMode::BlendFixedColor },
		m_pDB->blendModes(), [this](Msg* pMsg) {this->blendModeToggled(pMsg); });

	for (auto& p : pColumn->slots)
		if (p->id() == int(BlendMode::Replace) || p->id() == int(BlendMode::Blend))
			p->setDisabled(true);

	pTopSection->slots << pColumn;

	// Togglebuttons for Source Format

	
	
	pColumn = _buildToggleButtonRow("Source Formats", { PixelFormat::BGR_8_linear, PixelFormat::BGR_8_sRGB,
														PixelFormat::BGRX_8_linear, PixelFormat::BGRX_8_sRGB,
														PixelFormat::BGRA_8_linear, PixelFormat::BGRA_8_sRGB,
														PixelFormat::Index_8_linear, PixelFormat::Index_8_sRGB,
														PixelFormat::Index_16_linear, PixelFormat::Index_16_sRGB,
														PixelFormat::BGRA_4_linear,
														PixelFormat::BGR_565_linear,
														PixelFormat::BGR_565_sRGB,
														PixelFormat::RGB_565_bigendian,
														PixelFormat::RGB_555_bigendian,
														PixelFormat::Alpha_8 },
		m_pDB->srcFormats(), [this](Msg* pMsg) {this->sourceFormatToggled(pMsg); });

	pTopSection->slots << pColumn;

	// Togglebuttons for Dest Format

	pColumn = _buildToggleButtonRow("Dest Formats", { 	PixelFormat::BGR_8_linear, PixelFormat::BGR_8_sRGB,
														PixelFormat::BGRX_8_linear, PixelFormat::BGRX_8_sRGB,
														PixelFormat::BGRA_8_linear, PixelFormat::BGRA_8_sRGB,
														PixelFormat::BGRA_4_linear, 
														PixelFormat::BGR_565_linear,
														PixelFormat::BGR_565_sRGB,
														PixelFormat::RGB_565_bigendian,
														PixelFormat::RGB_555_bigendian,
														PixelFormat::Alpha_8 },
		m_pDB->destFormats(), [this](Msg* pMsg) {this->destFormatToggled(pMsg); });

	pTopSection->slots << pColumn;

	//

	return pTopSection;
}

//____ _buildHeaderWithCloseButton() __________________________________________

Widget_p MyApp::_buildHeaderWithCloseButton(std::string title, std::function<void(wg::Msg*)> pressCallback)
{
	auto pRow = PackPanel::create({ .axis = Axis::X });

	pRow->slots << TextDisplay::create(WGBP(TextDisplay,
		_.display.text = title,
		_.display.style = m_pLabelStyle,
		_.display.layout = m_pTextLayoutCentered
	));

	auto pButton = Button::create({ .label = {.text = "Delete" },
									.skin = m_pButtonSkin });



/*
	auto pButton =  Button::create(WGBP(Button,
		_.label.text = "CLOSE",
		_.skin = m_pButtonSkin));
*/

	Base::msgRouter()->addRoute(pButton, MsgType::Select, pressCallback);

	pRow->slots << pButton;

	pRow->slots[1].setWeight(0.f);

	return pRow;
}


//____ _buildOptimizedBlitsSection() __________________________________________

Widget_p MyApp::_buildOptimizedBlitsSection()
{
	auto pSection = PackPanel::create( { .axis = Axis::Y } );

	vector<KernelDB::BlitType>	activeBlitTypes = { KernelDB::BlitType::StraightBlit, KernelDB::BlitType::StraightTile, KernelDB::BlitType::StraightBlur,
		KernelDB::BlitType::TransformBlitNearest, KernelDB::BlitType::TransformClipBlitNearest, KernelDB::BlitType::TransformTileNearest, KernelDB::BlitType::TransformBlurNearest,
		KernelDB::BlitType::TransformBlitBilinear, KernelDB::BlitType::TransformClipBlitBilinear, KernelDB::BlitType::TransformTileBilinear, KernelDB::BlitType::TransformBlurBilinear };

	vector<TintMode>	activeTintModes;
	vector<BlendMode>	activeBlendModes;
	vector<PixelFormat>	activeSourceFormats;
	vector<PixelFormat>	activeDestFormats;

	for (int i = 0; i < TintMode_size; i++)
		if (m_pDB->tintMode(TintMode(i)))
			activeTintModes.push_back(TintMode(i));

	for (int i = 0; i < BlendMode_size; i++)
		if (m_pDB->blendMode(BlendMode(i)))
			activeBlendModes.push_back(BlendMode(i));

	for (int i = 0; i < PixelFormat_size; i++)
		if (m_pDB->srcFormat(PixelFormat(i)))
			activeSourceFormats.push_back(PixelFormat(i));

	for (int i = 0; i < PixelFormat_size; i++)
		if (m_pDB->destFormat(PixelFormat(i)))
			activeDestFormats.push_back(PixelFormat(i));

	// CUSTOM BLITS

	int index = 0;
	for (auto it = m_pDB->beginCustomBlits(); it < m_pDB->endCustomBlits(); it++)
	{
		auto pEntry = PackPanel::create( { .axis = Axis::Y, .skin = m_pSectionSkin } );

		// Top Row

		auto pTopRow = _buildHeaderWithCloseButton("CUSTOM BLIT KERNELS", [this, index](Msg* pMsg) {  this->eraseCustomBlitEntry(index); });

		// Body Row

		auto pBodyRow = PackPanel::create( { .axis = Axis::X, .spacing = 10, .spacingAfter = 5, .spacingBefore = 5 } );

		pBodyRow->slots << _buildToggleButtonRow("Operations", activeBlitTypes, it->blitTypes, [this, index](Msg* pMsg) { this->customBlitTypeToggled(index, pMsg); });
		pBodyRow->slots << _buildToggleButtonRow("Tint Modes", activeTintModes, it->tintModes, [this, index](Msg* pMsg) { this->customBlitTintModeToggled(index,pMsg); });
		pBodyRow->slots << _buildToggleButtonRow("Blend Modes", activeBlendModes, it->blendModes, [this, index](Msg* pMsg) { this->customBlitBlendModeToggled(index, pMsg); });
		pBodyRow->slots << _buildToggleButtonRow("Source Formats", activeSourceFormats, it->sourceFormats, [this, index](Msg* pMsg) { this->customBlitSourceFormatToggled(index, pMsg); });
		pBodyRow->slots << _buildToggleButtonRow("Dest Formats", activeDestFormats, it->destFormats, [this, index](Msg* pMsg) { this->customBlitDestFormatToggled(index, pMsg); });


		// Bottom Row

		auto pBottomRow = PackPanel::create( { .axis = Axis::X, .layout = m_pNoExpandLayout } );

		pBottomRow->slots << TextDisplay::create(WGBP(TextDisplay,
			_.display.style = m_pTextStyle,
			_.display.text = "Number of kernels: "));

		pBottomRow->slots << NumberDisplay::create(WGBP(NumberDisplay,
			_.display.value = m_pDB->countEntryKernels(index) ));

		pBottomRow->setSlotWeight(0, 2, 0.f);

		// Tie everything together

		pEntry->slots << pTopRow;
		pEntry->slots << pBodyRow;
		pEntry->slots << pBottomRow;

		pSection->slots << pEntry;
		index++;
	}

	// "Add Entry" button

	auto pRow = PackPanel::create( { .axis = Axis::X } );
	pRow->setSkin(BoxSkin::create(WGBP(BoxSkin,
		_.color = HiColor::White,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black,
		_.padding = { 4,2,4,2 }
	)));


	auto pButton = Button::create(WGBP(Button,
		_.skin = m_pButtonSkin,
		_.label.text = "Add Entry"
	));

	pRow->slots << Filler::create();
	pRow->slots << pButton;
	pRow->slots << Filler::create();

	pRow->slots[1].setWeight(0);

	pRow->setLayout(PackLayout::create({ .expandFactor = PackLayout::Factor::Weight }));

	Base::msgRouter()->addRoute(pButton, MsgType::Select, [this](Msg* pMsg) {this->addOptimizedBlitEntry(); });

	pSection->slots << pRow;


	return pSection;
}

//____ _buildListSummarySection() _____________________________________________

wg::Widget_p MyApp::_buildListSummarySection()
{
	auto pSection = PackPanel::create( { .axis = Axis::Y } );
	pSection->setSkin(BoxSkin::create(WGBP(BoxSkin,
		_.color = HiColor::White,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black,
		_.padding = { 4,2,4,2 }
	)));
	pSection->setSpacing(5, 10, 5);

	auto kernelCount = m_pDB->countKernels();

	pSection->slots << TextDisplay::create(WGBP(TextDisplay,
		_.display.text = "Kernels to be generated:",
		_.display.style = m_pLabelStyle));

	pSection->slots << _buildLabeledList(4, { 	{"Fill: ", kernelCount.fill},
												{"Line: ", kernelCount.line},
												{"ClipLine: ", kernelCount.clipLine},
												{"Segments: ", kernelCount.segment},
												{"Pass 1 blits - strght:", kernelCount.pass1blits_straight},
												{"Pass 1 blits - strght 8: ", kernelCount.pass1blits_straight_fast8},
												{"Pass 1 blits - transf:", kernelCount.pass1blits_transform},
												{"Pass 1 blits - transf 8: ", kernelCount.pass1blits_transform_fast8},
												{"Pass 2 blits: ", kernelCount.pass2blits},
												{"Pass 2 blits - fast 8: ", kernelCount.pass2blits_fast8},
												{"Custom blits: ", kernelCount.customBlits}
										});


	int total = kernelCount.fill + kernelCount.line + kernelCount.clipLine +
		kernelCount.segment + kernelCount.pass1blits_straight + kernelCount.pass1blits_straight_fast8 +
		kernelCount.pass1blits_transform + kernelCount.pass1blits_transform_fast8 +
		kernelCount.pass2blits + kernelCount.pass2blits_fast8 + kernelCount.customBlits;



	std::string amount = "Total amount: " + std::to_string(total);

	pSection->slots << TextDisplay::create(WGBP(TextDisplay,
		_.display.text = amount.c_str(),
		_.display.style = m_pLabelStyle));

	return pSection;

}

//____ _buildLabeledList() ____________________________________________________

wg::Widget_p MyApp::_buildLabeledList(int nColumns, std::initializer_list < std::pair<string, int>> list)
{
	auto pSection = PackPanel::create( { .axis = Axis::X } );

	PackPanel_p columns[16][2];

	for (int i = 0; i < nColumns; i++)
	{
		auto pMain = PackPanel::create( { .axis = Axis::X, .layout = m_pNoExpandLayout });

		auto pLabels = PackPanel::create( { .axis = Axis::Y, .layout = m_pNoExpandLayout });

		auto pValues = PackPanel::create( { .axis = Axis::Y, .layout = m_pNoExpandLayout });
		
		pMain->slots.pushBack( pLabels, { .weight = 0.f } );
		pMain->slots.pushBack( pValues, { .weight = 0.f } );

		columns[i][0] = pLabels;
		columns[i][1] = pValues;

		pSection->slots << pMain;
	}

	int column = 0;
	for (auto& entry : list)
	{
		columns[column][0]->slots.pushBack( TextDisplay::create(WGBP(TextDisplay,
																	 _.display.text = entry.first.c_str()
																	 )),
										   { .weight = 0.f });

		columns[column][1]->slots.pushBack( NumberDisplay::create(WGBP(NumberDisplay,
																	   _.display.value = entry.second 
																	   )),
										   { .weight = 0.f });

		column = (column + 1) % (nColumns);
	}


	return pSection;

}


//____ _buildExportSection() __________________________________________________

wg::Widget_p MyApp::_buildExportSection()
{
	auto pSection = PackPanel::create({ .axis = Axis::X });
	pSection->setSkin(BoxSkin::create(WGBP(BoxSkin,
		_.color = HiColor::White,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black,
		_.padding = { 4,2,4,2 }
	)));

	
	auto pEditorLabel = TextDisplay::create( WGBP(TextDisplay,
												  _.display.text = "AddKernel function name: "
											));
	
	auto pEditor = LineEditor::create( WGBP(LineEditor,
											_.skin = m_pInputBoxSkin,
											_.editor.text = "addDefaultSoftKernels"
											));
	
	m_pKernelFuncNameEditor = pEditor;
	
	auto pButton = Button::create(WGBP(Button,
		_.skin = m_pButtonSkin,
		_.label.text = "Export Source Code"
	));

	pSection->slots << pEditorLabel;
	pSection->slots << pEditor;
	pSection->slots << pButton;

	pSection->slots[0].setWeight(0.f);
	pSection->slots[1].setWeight(1.0f);
	pSection->slots[2].setWeight(0);

	pSection->setLayout(PackLayout::create({ .expandFactor = PackLayout::Factor::Weight }));

	Base::msgRouter()->addRoute(pButton, MsgType::Select, [this](Msg* pMsg) {this->exportSource(); });

	return pSection;
}
