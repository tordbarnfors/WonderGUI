
#include "surfviewer.h"

#include <wondergui.h>
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
	m_pAppVisitor = pVisitor;

	if (!_setupGUI(pVisitor))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}
	
	// Add any image file from the argument list

	auto arguments = pVisitor->programArguments();

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
	return true;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{

}


//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(Visitor* pVisitor)
{
	m_pWindow = pVisitor->createWindow({ .size = {800,600}, .title = "WonderGUI Surface Viewer" });

	//

	auto pFontBlob = pVisitor->loadBlob("resources/DroidSans.ttf");
	auto pFont = FreeTypeFont::create(pFontBlob);

	m_pTextStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 14,
									_.color = Color8::Black,
									_.states = {{State::Disabled, Color8::DarkGrey}
	} ));

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
	
	m_pWindow->setContent(pBasePanel);

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
	


	Base::msgRouter()->addRoute( pLoadButton, MsgType::Select, [this](Msg*pMsg){this->selectAndLoadImage();});

	Base::msgRouter()->addRoute(pLeftButton, MsgType::Select, [this](Msg* pMsg) 
		{
			if (m_imageIdx > 0)
				this->loadImage(--m_imageIdx);
		});

	Base::msgRouter()->addRoute(pRightButton, MsgType::Select, [this](Msg* pMsg)
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

	pWindow->scrollbarX.setBackground(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color8::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black)));
	pWindow->scrollbarX.setBar(m_pPlateSkin);

	pWindow->scrollbarY.setBackground(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color8::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black)));
	pWindow->scrollbarY.setBar(m_pPlateSkin);

	pWindow->setAutohideScrollbars(true, true);
	pWindow->setSizeConstraints(SizeConstraint::GreaterOrEqual, SizeConstraint::GreaterOrEqual);

	m_pImageDisplay = SurfaceDisplay::create( WGBP(SurfaceDisplay,
											_.skin = ColorSkin::create( Color8::AntiqueWhite)
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
		_.padding = { 4,10,4,10 },
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
		_.padding = { 2,2,2,2 }
	));


	return true;
}

//____ selectAndLoadImage() ___________________________________________________

void MyApp::selectAndLoadImage()
{
	auto selectedFiles = m_pAppVisitor->openMultiFileDialog("Select Images", "", { "*.surf", "*.qoi" }, "Image files");
	
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

	auto pSurface = m_pAppVisitor->loadSurface(m_imagePaths[idx]);

	m_pImageDisplay->setSurface(pSurface);
	m_pPathDisplay->display.setText(m_imagePaths[idx]);

	m_pPrevButton->setDisabled( idx == 0 );
	m_pNextButton->setDisabled( idx == m_imagePaths.size()-1 );

	return true;
}
