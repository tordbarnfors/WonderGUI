
#include "bitmapviewer.h"

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
	m_pAppAPI = pAPI;

	if (!_setupGUI(pAPI))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}
	
	// Add any image file from the argument list

	auto arguments = pAPI->programArguments();

	for ( auto& arg : arguments )
	{
		auto sz = arg.size();
		if ( sz >= 5 && ( arg.find(".qoi", sz - 4) != string::npos ||
			 arg.find(".surf", sz - 5) != string::npos) )
			m_imagePaths.push_back(arg);
	}

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
	m_pWindow = nullptr;
}



//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(wapp::API* pAPI)
{
	m_pWindow = wapp::Window::create(pAPI, { .size = {800,600}, .title = "WonderGUI Bitmap Viewer" });

	//

	auto pFontBlob = pAPI->loadBlob("resources/DroidSans.ttf");
	auto pFont = FreeTypeFont::create(pFontBlob);

	m_pTextStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 14,
									_.color = Color::Black,
									_.states = {{State::Disabled, Color::DarkGrey}
	} ));

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

	m_pLayout = PackLayout::create({ .wantedSize = PackLayout::WantedSize::Default,
	.expandFactor = PackLayout::Factor::Weight, .shrinkFactor = PackLayout::Factor::Zero });

	
	auto pBasePanel = PackPanel::create();
	pBasePanel->setAxis(Axis::Y);
	pBasePanel->setLayout(m_pLayout);
	
	auto pSplitPanel = SplitPanel::create();
	pSplitPanel->setAxis(Axis::X);
	pSplitPanel->setResizeRatio(1.f);
	pSplitPanel->setSplit(0.5f);			//TODO: SplitPanel::setSplitFactor() does not work! Replace with some other function?
	pSplitPanel->setHandleSkin(m_pButtonSkin);
	
	pSplitPanel->slots[0] = createImagePanel();
	pSplitPanel->slots[1] = createInfoPanel();

	pBasePanel->slots << createTopBar();
	pBasePanel->slots << pSplitPanel;

	pBasePanel->setSlotWeight(0, 2, {0.f,1.f});
	
	m_pWindow->mainCapsule()->slot = pBasePanel;

	return true;
}


//____ createTopBar() ________________________________________________________

Widget_p MyApp::createTopBar()
{
	
	auto pBar = PackPanel::create();
	pBar->setAxis(Axis::X);
	pBar->setLayout(m_pLayout);
	pBar->setSkin(m_pPlateSkin);

	auto pLoadButton = Button::create( WGBP(Button,
											_.skin = m_pButtonSkin,
											_.label = WGBP(DynamicText, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = "Load" )
											));

	auto pSpacer = Filler::create( WGBP(Filler, _.defaultSize = { 20,1 } ));

	
	auto pLeftButton = Button::create( WGBP(Button,
											_.skin = m_pButtonSkin,
											_.label = WGBP(DynamicText, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = " < " )
											));

	

	auto pPath = TextDisplay::create( WGBP(TextDisplay,
											 _.skin = m_pSectionSkin,
											 _.display = WGBP(DynamicText, _.style = m_pTextStyle, _.layout = m_pTextLayoutCentered)
										   ) );
	
	auto pRightButton = Button::create( WGBP(Button,
											_.skin = m_pButtonSkin,
											_.label = WGBP(DynamicText, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = " > " )
											));

	pBar->slots << pLoadButton;
	pBar->slots << pSpacer;
	pBar->slots << pLeftButton;
	pBar->slots << pPath;
	pBar->slots << pRightButton;

	pBar->setSlotWeight( 0, 5, {0.f, 0.f, 0.f, 1.f, 0.f});
	


	wg::Base::msgRouter()->addRoute( pLoadButton, MsgType::Select, [this](Msg*pMsg){this->selectAndLoadImage();});

	wg::Base::msgRouter()->addRoute(pLeftButton, MsgType::Select, [this](Msg* pMsg) 
		{
			if (m_imageIdx > 0)
				this->loadImage(--m_imageIdx);
		});

	wg::Base::msgRouter()->addRoute(pRightButton, MsgType::Select, [this](Msg* pMsg)
		{
			if (m_imageIdx < m_imagePaths.size()-1 )
				this->loadImage(++m_imageIdx);
		});

	m_pPathDisplay = pPath;
	m_pPrevButton = pLeftButton;
	m_pNextButton = pRightButton;


	auto pKnobSkin = FillMeterSkin::create({ .defaultSize = {100,20 }, .direction = Direction::Right});
	auto pKnob = Knob::create({ .skin = pKnobSkin });

	pBar->slots << pKnob;

	return pBar;
}

//____ createImagePanel() _____________________________________________________

Widget_p MyApp::createImagePanel()
{
	auto pWindow = ScrollPanel::create();

	pWindow->scrollbarX.setBackSkin(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color::Black)));
	pWindow->scrollbarX.setBarSkin(m_pPlateSkin);

	pWindow->scrollbarY.setBackSkin(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color::Black)));
	pWindow->scrollbarY.setBarSkin(m_pPlateSkin);

	pWindow->setAutohideScrollbars(true, true);
	pWindow->setSizeConstraints(SizeConstraint::GreaterOrEqual, SizeConstraint::GreaterOrEqual);

	m_pImageDisplay = BitmapDisplay::create( WGBP(BitmapDisplay,
											_.skin = ColorSkin::create( Color::AntiqueWhite)
											));
	
	pWindow->slot = m_pImageDisplay;
	
	return pWindow;
}

//____ createInfoPanel() _____________________________________________________

Widget_p MyApp::createInfoPanel()
{
	auto pBase = PackPanel::create();
	pBase->setAxis(Axis::Y);
	pBase->setLayout(m_pLayout);
	pBase->setSkin(m_pPlateSkin);

	
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

//____ selectAndLoadImage() ___________________________________________________

void MyApp::selectAndLoadImage()
{
	auto selectedFiles = m_pAppAPI->openMultiFileDialog("Select Images", "", "", { "*.surf", "*.qoi" }, "Image files");
	
	if( selectedFiles.empty()  )
		return;

	m_imagePaths = selectedFiles;
	loadImage(0);
}

//____ loadImage() ____________________________________________________________

bool MyApp::loadImage(int idx)
{
	if (idx < 0 || idx >= m_imagePaths.size())
		return false;

	auto pBitmap = m_pAppAPI->loadBitmap(m_imagePaths[idx]);

	m_pImageDisplay->setBitmap(pBitmap);
	m_pPathDisplay->display.setText(m_imagePaths[idx]);

	m_pPrevButton->setDisabled( idx == 0 );
	m_pNextButton->setDisabled( idx == m_imagePaths.size()-1 );

	return true;
}
