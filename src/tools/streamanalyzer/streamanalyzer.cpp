
#include "streamanalyzer.h"

#include <wg_softbackend.h>
#include <wg_softbackend_kernels.h>
#include <wg_softkernels_rgb555be_extras.h>

#include <wg_softkernels_rgb565be_base.h>
#include <wg_softkernels_rgb565be_extras.h>

#include <wg_softkernels_bgr565srgb_base.h>
#include <wg_softkernels_bgr565srgb_extras.h>


#include <wg_softkernels_default.h>

#include <wg_gfxdevice_gen2.h>

#include <wg_linearbackend.h>

#include <wg_frametrimmer.h>
#include <themes/simplistic/wg_simplistic.h>

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
		
	auto arguments = pVisitor->programArguments();
/*
	if ( !arguments.empty() )
	{

		if (!loadStream(arguments[0] ))
		{
			printf("ERROR: Could not load stream '%s'!\n", arguments[0].c_str() );
			return false;
		}
	}
*/

	auto pSoftGfxDevice = wg_dynamic_cast<SoftGfxDevice_p>(Base::defaultGfxDevice());
	if( pSoftGfxDevice )
	{
//		addExtraSoftKernelsForRGB555BECanvas(pSoftGfxDevice);
//		addExtraSoftKernelsForRGB565BECanvas(pSoftGfxDevice);

//		addExtraSoftKernelsForBGR565sRGBCanvas(pSoftGfxDevice);
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
	Base::setErrorHandler( nullptr );
}


//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(Visitor* pVisitor)
{
	m_pWindow = pVisitor->createWindow({ .minSize = { 600, 200 }, .size = {1600,800}, .title = "Stream Analyzer" });

	m_pWindow->setCloseRequestHandler([](void) {
		return true;
	});
	
	auto pRoot = m_pWindow->rootPanel();
	
	//

	auto path = pVisitor->resourceDirectory();
	
	auto pFontBlob = pVisitor->loadBlob( path + "DroidSans.ttf");
	auto pFont = FreeTypeFont::create(pFontBlob);
	m_pFont = pFont;
	
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

	m_pBigWhiteStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 20,
									_.color = HiColor::White));

	

	Base::setDefaultStyle(m_pTextStyle);

	m_pTextLayoutCentered = BasicTextLayout::create(WGBP(BasicTextLayout,
		_.placement = Placement::Center));


	//

	if (!_loadSkins(pVisitor))
		return false;

	m_pLayout = PackLayout::create({ .wantedSize = PackLayout::WantedSize::Default,
	.expandFactor = PackLayout::Factor::Weight, .shrinkFactor = PackLayout::Factor::Weight });

	m_pDebugger = Debugger::create();

	auto pDbgFont1Blob = pVisitor->loadBlob("resources/NotoSans-Regular.ttf");
	auto pDbgFont2Blob = pVisitor->loadBlob("resources/NotoSans-Bold.ttf");
	auto pDbgFont3Blob = pVisitor->loadBlob("resources/NotoSans-Italic.ttf");
	auto pDbgFont4Blob = pVisitor->loadBlob("resources/DroidSansMono.ttf");

	auto pDbgFont1 = FreeTypeFont::create(pDbgFont1Blob);
	auto pDbgFont2 = FreeTypeFont::create(pDbgFont2Blob);
	auto pDbgFont3 = FreeTypeFont::create(pDbgFont3Blob);
	auto pDbgFont4 = FreeTypeFont::create(pDbgFont4Blob);

	auto pTheme = Simplistic::create(pDbgFont1, pDbgFont2, pDbgFont3, pDbgFont4);

	m_pDebugOverlay = DebugOverlay::create( { .debugger = m_pDebugger, .theme = pTheme } );

	m_pDebugOverlay->setActivated(true);

	auto pPopupOverlay = PopupOverlay::create();


	
	auto pBasePanel = PackPanel::create();
	pBasePanel->setAxis(Axis::Y);
	pBasePanel->setLayout(m_pLayout);

	pBasePanel->slots << createTopBar();



	auto pSplitPanel = SplitPanel::create();
	pSplitPanel->setAxis(Axis::Y);
	pSplitPanel->setResizeRatio(1.f);
//	pSplitPanel->setSplit(0.5f);
	pSplitPanel->setHandleSkin(m_pButtonSkin);
	
	pSplitPanel->slots[0] = createDisplayPanel();
	pSplitPanel->slots[1] = createLowerPanel();

	pBasePanel->slots << pSplitPanel;

	pBasePanel->setSlotWeight(0, 2, {0.f,1.f});

	pPopupOverlay->mainSlot = pBasePanel;

	m_pDebugOverlay->mainSlot = pPopupOverlay;

	pRoot->slot = m_pDebugOverlay;

	pSplitPanel->setSplit(0.5f);

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
											_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = "Load" )
											));

	auto pSpacer1 = Filler::create( WGBP(Filler, _.defaultSize = { 20,1 } ));

	auto pSpacer2 = Filler::create(WGBP(Filler, _.defaultSize = { 20,1 }));

	
	m_pDisplayToggles = PackPanel::create();



	pBar->slots << pLoadButton;
	pBar->slots << pSpacer1;
	pBar->slots << m_pDisplayToggles;
	pBar->slots << pSpacer2;


	pBar->setSlotWeight( 0, 4, {0.f, 1.f, 0.f, 1.f});
	


	Base::msgRouter()->addRoute( pLoadButton, MsgType::Select, [this](Msg*pMsg){this->selectAndLoadStream();});


	return pBar;
}

//____ createDisplayPanel() _____________________________________________________

Widget_p MyApp::createDisplayPanel()
{
	auto pWindow = _standardScrollPanel();

	pWindow->setSkin(ColorSkin::create({ .color = Color::DarkSlateBlue }));
	pWindow->setSizeConstraints(SizeConstraint::None, SizeConstraint::GreaterOrEqual);
	pWindow->setPlacement(Placement::Center);

	auto pLineup = PackPanel::create( { .axis = Axis::X,
										.slotAlignment = Alignment::Center,
										.spacing = 12,
										.spacingAfter = 6,
										.spacingBefore = 6
	});

	pWindow->slot = pLineup;
	
	m_pScreenLineup = pLineup;

	return pWindow;
}

//____ createLowerPanel() _____________________________________________________

Widget_p MyApp::createLowerPanel()
{
	auto pBase = TwoSlotPanel::create();
	pBase->setAxis(Axis::X);

	auto pLayout = PackLayout::create( { .expandFactor = PackLayout::Factor::Weight } );
	
	pBase->setLayout(pLayout);
	
//	pBase->slots << createNavigationPanel();
//	pBase->slots << createLogPanel();

	
	pBase->slots[0] = createNavigationPanel();
	pBase->slots[1] = createLogPanel();

	pBase->slots[0].setWeight(0.f);
	
	return pBase;
}

//____ createLogPanel() _______________________________________________________

Widget_p MyApp::createLogPanel()
{
	auto pMain = PackPanel::create();
	pMain->setAxis(Axis::Y);
	pMain->setLayout(m_pLayout);
	pMain->setSkin(m_pPlateSkin);

	auto pLogButtonRow = PackPanel::create();
	pLogButtonRow->setAxis(Axis::X);


	auto pFrameLogButton = ToggleButton::create( WGBP(ToggleButton,
											_.skin = m_pToggleButtonSkin,
											_.selected = true,
											_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = "Frame data" )
											));

	auto pFullLogButton = ToggleButton::create( WGBP(ToggleButton,
											_.skin = m_pToggleButtonSkin,
											_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = "File data" )
											));

	auto pOptimizerInLogButton = ToggleButton::create( WGBP(ToggleButton,
											_.skin = m_pToggleButtonSkin,
											_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = "Optimizer input" )
											));

	auto pOptimizerOutLogButton = ToggleButton::create( WGBP(ToggleButton,
											_.skin = m_pToggleButtonSkin,
											_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = "Optimizer output" )
											));

	
	auto pResourcesButton = ToggleButton::create( WGBP(ToggleButton,
											_.skin = m_pToggleButtonSkin,
											_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = "Resources" )
											));

	auto pStatisticsButton = ToggleButton::create( WGBP(ToggleButton,
											_.skin = m_pToggleButtonSkin,
											_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = "Statistics" )
											));

	auto pErrorLogButton = ToggleButton::create( WGBP(ToggleButton,
											_.skin = m_pToggleButtonSkin,
											_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = "Errors" )
											));
	
	
	auto pToggleGroup = ToggleGroup::create();
	pToggleGroup->add(pFrameLogButton);
	pToggleGroup->add(pFullLogButton);
	pToggleGroup->add(pOptimizerInLogButton);
	pToggleGroup->add(pOptimizerOutLogButton);
	pToggleGroup->add(pResourcesButton);
	pToggleGroup->add(pStatisticsButton);
	pToggleGroup->add(pErrorLogButton);

	Base::msgRouter()->addRoute(pFrameLogButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->showFrameLog();
		});
	
	Base::msgRouter()->addRoute(pFullLogButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->showFullLog();
		});

	Base::msgRouter()->addRoute(pOptimizerInLogButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->showOptimizerInLog();
		});
	
	Base::msgRouter()->addRoute(pOptimizerOutLogButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->showOptimizerOutLog();
		});
	
	Base::msgRouter()->addRoute(pResourcesButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->showResources();
		});

	Base::msgRouter()->addRoute(pStatisticsButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->showStatistics();
		});

	Base::msgRouter()->addRoute(pErrorLogButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->showErrors();
		});
	
	pLogButtonRow->slots << pFrameLogButton;
	pLogButtonRow->slots << pFullLogButton;
	pLogButtonRow->slots << pOptimizerInLogButton;
	pLogButtonRow->slots << pOptimizerOutLogButton;
	pLogButtonRow->slots << pResourcesButton;
	pLogButtonRow->slots << pStatisticsButton;
	pLogButtonRow->slots << pErrorLogButton;
	
	pLogButtonRow->setSlotWeight(0, 7, 0.f );
	
	TextEditor::Blueprint displayBP;
	displayBP.skin = ColorSkin::create(Color8::LightYellow);
	displayBP.editor.editMode = TextEditMode::Selectable;
	
	// Create frame log hierarchy
	
	auto pFrameLogWindow = _standardScrollPanel();
	
	auto pFrameLogText = TextEditor::create( displayBP );
	pFrameLogWindow->slot = pFrameLogText;

	m_pFrameLogDisplay = pFrameLogText;

	m_pFrameLogContainer = pFrameLogWindow;

	// Create full log hierarchy
	
	auto pFullLogWindow = _standardScrollPanel();

	auto pFullLogText = TextEditor::create( displayBP );
	pFullLogWindow->slot = pFullLogText;

	m_pFullLogDisplay = pFullLogText;

	m_pFullLogContainer = pFullLogWindow;

	// Create optimizer in log hierarchy
	
	auto pOptimizerInLogWindow = _standardScrollPanel();

	auto pOptimizerInLogText = TextEditor::create( displayBP );
	pOptimizerInLogWindow->slot = pOptimizerInLogText;

	m_pOptimizerInLogDisplay = pOptimizerInLogText;

	m_pOptimizerInLogContainer = pOptimizerInLogWindow;
	
	// Create optimizer out log hierarchy
	
	auto pOptimizerOutLogWindow = _standardScrollPanel();

	auto pOptimizerOutLogText = TextEditor::create( displayBP );
	pOptimizerOutLogWindow->slot = pOptimizerOutLogText;

	m_pOptimizerOutLogDisplay = pOptimizerOutLogText;

	m_pOptimizerOutLogContainer = pOptimizerOutLogWindow;
	
	// Create error log
	
	auto pErrorsWindow = _standardScrollPanel();

	auto pErrorsText = TextEditor::create( displayBP );
	pErrorsWindow->slot = pErrorsText;

	m_pErrorsDisplay = pErrorsText;

	m_pErrorsContainer = pErrorsWindow;
	
	Base::setErrorHandler( [this](Error& err)
	{
		char temp[1024];
		
		snprintf(temp, 1024, "%s: %s - %s\n", toString( err.severity ), err.function, err.message );
		
		m_pErrorsDisplay->editor.append(temp);
	});
	
	
	// Create resources view
	
	auto pResourcesWindow = _standardScrollPanel();
	
	auto pResourcesPanel = PackPanel::create();
	pResourcesPanel->setAxis(Axis::Y);
	pResourcesPanel->setSkin(ColorSkin::create(Color::Black));
	pResourcesWindow->slot = pResourcesPanel;
	
	m_pResourcePanel = pResourcesPanel;
	
	m_pResourceContainer = pResourcesWindow;
	
	// Create statistics view hierarchy
	
	auto pStatisticsContainer = _standardScrollPanel();

	auto pStatisticsDisplay = TextEditor::create( displayBP );
	pStatisticsContainer->slot = pStatisticsDisplay;

	m_pStatisticsDisplay = pStatisticsDisplay;

	m_pStatisticsContainer = pStatisticsContainer;
	
	
	// Finish it it up
	
	auto pLogCapsule = SizeCapsule::create();
	pLogCapsule->slot = pFrameLogWindow;
	m_pLogCapsule = pLogCapsule;
	
	pMain->slots << pLogButtonRow;
	pMain->slots << pLogCapsule;

	pMain->setSlotWeight(0, 1, 0.f);
	
	return pMain;
}

//____ createNavigationPanel() ________________________________________________

Widget_p MyApp::createNavigationPanel()
{
	auto pMain = PackPanel::create();
	pMain->setAxis(Axis::Y);
	pMain->setLayout(m_pLayout);
	pMain->setSkin(m_pPlateSkin);

	// Setup slider and progress counter

	auto pProgressStyle = TextStyle::create(WGBP(TextStyle,
									_.font = m_pFont,
									_.size = 48,
									_.color = HiColor::Black));

	
	auto pProgressText = TextDisplay::create(WGBP(TextDisplay,
		_.display = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = pProgressStyle, _.text = "-/-")
	));
	m_pProgressText = pProgressText;

	auto pSlider = Slider::create({ .handle = m_pButtonSkin,
									 .skin = ColorSkin::create(Color::Black)
		});

	Base::msgRouter()->addRoute(pSlider, MsgType::ValueUpdate, [this](Msg* pMsg)
		{
			auto pMyMsg = static_cast<ValueUpdateMsg*>(pMsg);
	this->setFrame(int(pMyMsg->value() * this->m_frames.size() + 0.5f));
		});

	m_pProgressSlider = pSlider;

	// Setup play buttons

	auto pPlayButtons = PackPanel::create();
	pPlayButtons->setAxis(Axis::X);
	pPlayButtons->setLayout(m_pLayout);

	auto pLongLeftButton = Button::create(WGBP(Button,
		_.skin = m_pButtonSkin,
		_.selectOnPress = true,
		_.label = WGBP(Text, _.layout = m_pTextLayoutCentered,_.style = m_pTextStyle, _.text = " << ")
	));

	auto pLeftButton = Button::create(WGBP(Button,
		_.skin = m_pButtonSkin,
		_.selectOnPress = true,
		_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = " < ")
	));

	auto pPauseButton = Button::create(WGBP(Button,
		_.skin = m_pButtonSkin,
		_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = " || ")
	));

	auto pRightButton = Button::create(WGBP(Button,
		_.skin = m_pButtonSkin,
		_.selectOnPress = true,
		_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = " > ")
	));

	auto pLongRightButton = Button::create(WGBP(Button,
		_.skin = m_pButtonSkin,
		_.selectOnPress = true,
		_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = " >> ")
	));

	pPlayButtons->slots << Filler::create();
	pPlayButtons->slots << pLongLeftButton;
	pPlayButtons->slots << pLeftButton;
	pPlayButtons->slots << pPauseButton;
	pPlayButtons->slots << pRightButton;
	pPlayButtons->slots << pLongRightButton;
	pPlayButtons->slots << Filler::create();

	pPlayButtons->setSlotWeight(1, 5, 0.f);

	Base::msgRouter()->addRoute(pLongLeftButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->setFrame(this->m_currentFrame - 5);
		});

	Base::msgRouter()->addRoute(pLeftButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->setFrame(this->m_currentFrame - 1);
		});

	Base::msgRouter()->addRoute(pRightButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->setFrame(this->m_currentFrame + 1);
		});

	Base::msgRouter()->addRoute(pLongRightButton, MsgType::Select, [this](Msg* pMsg)
		{
			this->setFrame(this->m_currentFrame + 5);
		});

	// Setup skip buttons

	static const char * skipButtonLabels[5] = { " +2 ", " +4 ", " +6 ", " +8 ", " +10 " };
	static int skipButtonAdvances[5] = { 2, 4, 6, 8, 10 };


	auto pSkipButtons = PackPanel::create();
	pSkipButtons->setAxis(Axis::X);
	pSkipButtons->setLayout(m_pLayout);
	pSkipButtons->slots << Filler::create();

	for (int i = 0; i < 5; i++)
	{
		auto pButton = Button::create(WGBP(Button,
			_.skin = m_pButtonSkin,
			_.label = WGBP(Text, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = skipButtonLabels[i] )
		));

		pSkipButtons->slots << pButton;

		int skip = skipButtonAdvances[i];
		Base::msgRouter()->addRoute(pButton, MsgType::Select, [this,skip](Msg* pMsg)
			{
				m_recordedSteps.clear();
				this->m_bRecordSteps = this->m_pRecordStepsToggle->isSelected();
				this->skipFrames( skip );
				this->m_bRecordSteps = false;
			});

	}

	pSkipButtons->slots << Filler::create();
	pSkipButtons->setSlotWeight(1, 5, 0.f);



	// Setup extra controls

	auto pExtraControls = PackPanel::create( { .axis = Axis::Y, .layout = m_pLayout } );

	//
	
	auto pRectToggle = ToggleButton::create(WGBP(ToggleButton,
		_.label = WGBP(Text, _.style = m_pTextStyle, _.text = "Show debug rectangles"),
		_.icon = WGBP(Icon, _.skin = m_pToggleButtonSkin, _.padding = { 0,8,0,0 })
	));

	Base::msgRouter()->addRoute(pRectToggle, MsgType::Toggle, [this](Msg* pMsg)
		{
			this->toggleDebugRects(static_cast<ToggleMsg*>(pMsg)->isSet());
		});


	m_pDebugRectsToggle = pRectToggle;
	pExtraControls->slots << pRectToggle;

	//
	
	auto pRecordStepsToggle = ToggleButton::create(WGBP(ToggleButton,
		_.label = WGBP(Text, _.style = m_pTextStyle, _.text = "Record optimize steps"),
		_.icon = WGBP(Icon, _.skin = m_pToggleButtonSkin, _.padding = { 0,8,0,0 })
	));

	m_pRecordStepsToggle = pRecordStepsToggle;
	
	
	pExtraControls->slots << pRecordStepsToggle;

	//
	
	auto pShowRecordedSteps = Button::create({
		.label = { .layout = m_pTextLayoutCentered, .style = m_pTextStyle, .text = "Show recorded steps" },
		.skin = m_pButtonSkin
	});

	Base::msgRouter()->addRoute(pShowRecordedSteps, MsgType::Select, [this](Msg* pMsg)
	{
		this->openRecordedStepsWindow();
	});
	
	pExtraControls->slots << pShowRecordedSteps;

	//
	
	auto pSkipTo = Button::create({
		.label = { .layout = m_pTextLayoutCentered, .style = m_pTextStyle, .text = "Skip to 236" },
		.skin = m_pButtonSkin
	});

	Base::msgRouter()->addRoute(pSkipTo, MsgType::Select, [this](Msg* pMsg)
	{

		m_recordedSteps.clear();
		this->m_bRecordSteps = this->m_pRecordStepsToggle->isSelected();

		int frame = 235;
		

		_resetStream();
		_playFrames( 0, frame, true );
		_playFrames( frame, frame+1, false );

		// Update the log
		
		_logFrames( frame, frame+1, false, m_pFrameLogDisplay );

		_logFrames( 0, frame, false, m_pOptimizerInLogDisplay);
		_logFrames( 0, frame, true, m_pOptimizerOutLogDisplay);
		
		//

		m_currentFrame = frame;
		
		// Update slider and frame counter
		
		_updateFrameCounterAndSlider();
		_updateResourcesView();

		if (m_bShowDebugRects)
			_updateDebugOverlays();

		this->m_bRecordSteps = false;



	});
	
	pExtraControls->slots << pSkipTo;


	//

	auto pLog8Frames = Button::create({
		.label = {.layout = m_pTextLayoutCentered, .style = m_pTextStyle, .text = "Log optimized 8 frames" },
		.skin = m_pButtonSkin
		});

	Base::msgRouter()->addRoute(pLog8Frames, MsgType::Select, [this](Msg* pMsg)
		{
			_logFrames(m_currentFrame, m_currentFrame +8, false, m_pOptimizerInLogDisplay);
			_logFrames(m_currentFrame, m_currentFrame +8, true, m_pOptimizerOutLogDisplay);
		});


	pExtraControls->slots << pLog8Frames;

	
	// Put it all together

	pMain->slots << pProgressText;
	pMain->slots << pSlider;
	pMain->slots << pPlayButtons;
	pMain->slots << pSkipButtons;
	pMain->slots << pExtraControls;

	pMain->slots << Filler::create();

	pMain->setSlotWeight(pMain->slots.begin(), pMain->slots.end() - 1, 0);
	pMain->setSpacing(2,4,2);

	return pMain;
}


//____ _loadSkins() ___________________________________________________________

bool MyApp::_loadSkins(Visitor * pVisitor)
{
	
	auto path = pVisitor->resourceDirectory();


#ifndef __APPLE__
	path.append("greyskin/");
#endif

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
		_.states = { State::Default, State::Hovered, State::Selected, State::SelectedHovered, State::Disabled }
	));

	m_pCheckBoxSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pCheckBoxSurf,
		_.axis = Axis::Y,
		_.frame = 3,
//		_.defaultSize = { 12,12 },
		_.states = { State::Default, State::Selected }
	));

	m_pSectionSkin = BoxSkin::create(WGBP(BoxSkin,
		_.color = HiColor::White,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black,
		_.padding = { 2,2,2,2 }
	));

	m_pRedOutlinedBlackSkin = BoxSkin::create(WGBP(BoxSkin,
		_.color = HiColor::Black,
		_.outlineThickness = 2,
		_.outlineColor = Color8::Red,
		_.padding = { 3,3,3,3 }
	));

	return true;
}

//____ selectAndLoadStream() ___________________________________________________

void MyApp::selectAndLoadStream()
{
	auto selectedFile = m_pAppVisitor->openFileDialog("Select GfxStream", "", { "*.wax", "*.dat" }, "Stream files");
	
	if( selectedFile.empty()  )
		return;

	loadStream(selectedFile);
}

//____ loadStream() ____________________________________________________________

bool MyApp::loadStream(std::string path)
{

	auto pStream = m_pAppVisitor->loadBlob(path);

	m_pStreamBlob = pStream;

	// Parse stream, find all frames

	m_frames.clear();

	for (auto pChunk = GfxStream::iterator(pStream->begin()); pChunk != GfxStream::iterator(pStream->end()); pChunk++ )
	{
		if (pChunk->type() == GfxStream::ChunkId::BeginRender)
			m_frames.push_back(pChunk);
	}

	// Setup streamwrapper and pump
	
/*
	auto pStreamGfxDevice = LinearGfxDevice::create(
		[this](CanvasRef ref, int bytes)
		{
			m_pLinearCanvasBuffer = new char[bytes];
			memset(m_pLinearCanvasBuffer, 0, bytes);
			
			return m_pLinearCanvasBuffer;
		},
		[this](CanvasRef ref, int nSegments, const LinearGfxDevice::Segment * pSegments)
		{
			int ofs = 0;
			while( m_screens[ofs]->identity() != (int) ref )
				ofs++;

			Surface_p pScreen = m_screens[ofs];

			RecordedSteps rec;
			
			if( this->m_bRecordSteps )
			{
				rec.canvas = ref;
				rec.before = pScreen->convert(pScreen->blueprint());
				
			}
			
			auto pixbuf = pScreen->allocPixelBuffer();
			pScreen->pushPixels(pixbuf);

			int pixelBytes = Util::pixelFormatToDescription(pixbuf.format).bits / 8;
			
			for( int i = 0 ; i < nSegments ; i++ )
			{
				auto& seg = pSegments[i];
				
				uint8_t * pSrc = seg.pBuffer;
				uint8_t * pDst = pixbuf.pixels + seg.rect.y * pixbuf.pitch + seg.rect.x * pixelBytes;
				
				for( int y = 0 ; y < seg.rect.h ; y++ )
				{
					memcpy( pDst, pSrc, seg.rect.w * pixelBytes );
					pSrc += seg.pitch;
					pDst += pixbuf.pitch;
				}
			}
			
			pScreen->pullPixels(pixbuf);
			pScreen->freePixelBuffer(pixbuf);
			
			delete [] m_pLinearCanvasBuffer;
			m_pLinearCanvasBuffer = nullptr;

			if( this->m_bRecordSteps )
			{
				rec.after = pScreen->convert(pScreen->blueprint());
				m_recordedSteps.push_back(rec);
			}
		} );
*/
	auto pStreamGfxBackend = SoftBackend::create();

	auto pTrimGfxBackend = FrameTrimmer::create(pStreamGfxBackend);

/*
	RectSPX mask = { 256*64,100*64,256 * 64, 200 * 64 };
	pTrimGfxBackend->pushMask(&mask, &mask + 1);
*/

	pTrimGfxBackend->setTrimLevel(4);

	auto pStreamGfxDevice = GfxDeviceGen2::create(pTrimGfxBackend);

//	auto pStreamGfxDevice = wg_dynamic_cast<SoftGfxDevice_p>(Base::defaultGfxDevice());

	addDefaultSoftKernels(pStreamGfxBackend);
/*
	addBaseSoftKernelsForRGB555BECanvas(pStreamGfxBackend);
	addExtraSoftKernelsForRGB555BECanvas(pStreamGfxBackend);

	addBaseSoftKernelsForRGB565BECanvas(pStreamGfxBackend);
	addExtraSoftKernelsForRGB565BECanvas(pStreamGfxBackend);
*/

	addBaseSoftKernelsForBGR565sRGBCanvas(pStreamGfxDevice);
	addExtraSoftKernelsForBGR565sRGBCanvas(pStreamGfxDevice);


	m_pStreamSurfaceFactory = pStreamGfxDevice->surfaceFactory();
	m_pStreamGfxDevice = pStreamGfxDevice;
	m_pStreamGfxBackend = pStreamGfxBackend;
	m_pStreamTrimGfxBackend = pTrimGfxBackend;
	
	m_pStreamPlayer	= StreamPlayer::create( pTrimGfxBackend, m_pStreamSurfaceFactory, pTrimGfxBackend->edgemapFactory() );
	m_pStreamPlayer->setStoreDirtyRects(true);
	m_pStreamPlayer->setMaxDirtyRects(10000);
	m_pStreamPlayer->setCanvasInfoCallback([this](const CanvasInfo * pBegin, const CanvasInfo * pEnd) { setupScreens(pBegin,pEnd); } );


	m_pStreamPump = StreamPump::create( StreamSource_p(), StreamSink_p(m_pStreamPlayer.rawPtr(),m_pStreamPlayer->input) );
	
	// Pump through first chunks to get ID

	m_screens.clear();

	auto pWrapper = StreamWrapper::create( m_pStreamBlob->begin(), m_pStreamBlob->end() );

	m_pStreamPump->setInput({pWrapper, pWrapper->output});

	bool bKeepProcessing = true;
	while( bKeepProcessing )
	{
		auto chunkId = m_pStreamPump->peekChunk();
		switch( chunkId )
		{
			case GfxStream::ChunkId::ProtocolVersion:
			case GfxStream::ChunkId::CanvasList:
				m_pStreamPump->pumpChunk();
				break;

			default:
				bKeepProcessing = false;
				break;
		}
	}

	m_pStreamPlayer->setCanvasInfoCallback(nullptr);

	if( m_screens.empty() )
		setupScreens();
	updateGUIAfterReload();
	m_currentFrame = 100000000;			// To avoid early out in setFrame().
	setFrame(0);
	
	return true;
}

//____ setupScreens() _________________________________________________________

void MyApp::setupScreens(const CanvasInfo* pBeg, const CanvasInfo* pEnd)
{
	SurfaceFactory_p	pFactory = GfxBase::defaultSurfaceFactory();

	// Ugly typecast! Will only work with SoftGfxDevice!

	LinearBackend_p		pLinearBackend = wg_dynamic_cast<LinearBackend_p>(m_pStreamGfxBackend);
	SoftBackend_p		pSoftBackend = wg_dynamic_cast<SoftBackend_p>(m_pStreamGfxBackend);

	for( auto pCanvas = pBeg ; pCanvas < pEnd ; pCanvas++ )
	{
		auto pSurf = pFactory->createSurface({ .format = pCanvas->format, .identity = int(pCanvas->ref), .scale = pCanvas->scale, .size = pCanvas->size/64 });
		pSurf->fill(HiColor::Black);

		m_screens.push_back(pSurf);

		if( pLinearBackend )
			pLinearBackend->defineCanvas(pCanvas->ref, pCanvas->size, pCanvas->format, pCanvas->scale );
		else
			pSoftBackend->defineCanvas(pCanvas->ref, wg_dynamic_cast<SoftSurface_p>(pSurf));
	}
}


void MyApp::setupScreens()
{
	SurfaceFactory_p	pFactory = GfxBase::defaultSurfaceFactory();

	// Ugly typecast! Will only work with SoftGfxDevice!
	
	auto pBackend = wg_dynamic_cast<GfxDeviceGen2_p>(m_pStreamGfxDevice)->backend();

	LinearBackend_p		pLinearBackend = wg_dynamic_cast<LinearBackend_p>(pBackend);
	SoftBackend_p		pSoftBackend = wg_dynamic_cast<SoftBackend_p>(pBackend);

	for (int i = 0; i < 11; i++)
	{
		auto pSurf = pFactory->createSurface({ .format = PixelFormat::RGB_555_bigendian, .identity = int(CanvasRef::Default) + i, .size = {800,480}});
		pSurf->fill(HiColor::Black);

		m_screens.push_back(pSurf);

		if(pLinearBackend)
			pLinearBackend->defineCanvas(CanvasRef(int(CanvasRef::Default) + i), {240*64,240*64}, PixelFormat::RGB_555_bigendian, 64 );
		else
			pSoftBackend->defineCanvas(CanvasRef(int(CanvasRef::Default) + i), wg_dynamic_cast<SoftSurface_p>(pSurf));
	}
}


//____ updateGUIAfterReload() ________________________________________________

void MyApp::updateGUIAfterReload()
{

	m_pScreenLineup->slots.clear();
	m_pDisplayToggles->slots.clear();

	m_debugOverlays.clear();
	m_overlayDisplays.clear();


	auto pSurfFactory = GfxBase::defaultSurfaceFactory();


	int toggleNb = 0;

	for (auto pScreen : m_screens)
	{
		// Fill in screen lineup

		auto pPanel = TwoSlotPanel::create();
		pPanel->setAxis(Axis::Y);
		pPanel->setSkin(m_pPlateSkin);

		auto pLabel = TextDisplay::create( WGBP(TextDisplay,
											_.display.text = toString((CanvasRef)pScreen->identity()),
											_.display.layout = m_pTextLayoutCentered
										));

		auto pDisplay = SurfaceDisplay::create({ .surface = pScreen });


		auto pOverlaySurface = pSurfFactory->createSurface( WGBP(Surface, _.size = pScreen->pixelSize()) );
		auto pOverlayDisplay = SurfaceDisplay::create({ .surface = pOverlaySurface });

		m_debugOverlays.push_back(pOverlaySurface);
		m_overlayDisplays.push_back(pOverlayDisplay);

		auto pDisplayStack = StackPanel::create();
		m_screenStacks.push_back(pDisplayStack);
		
		pDisplayStack->slots << pOverlayDisplay;
		pDisplayStack->slots << pDisplay;

		pDisplayStack->slots[0].setVisible(false);

		pPanel->slots[0] = pLabel;
		pPanel->slots[1] = pDisplayStack;

		m_pScreenLineup->slots << pPanel;

		// Fill in toggles

		char label[6];
		snprintf(label, 6, " %d ", toggleNb);

		auto pToggle = ToggleButton::create( WGBP(ToggleButton,
												_.skin = m_pToggleButtonSkin,
												_.label.text = label,
												_.selected = true
											) );

		auto pScreenLineup = m_pScreenLineup;
		Base::msgRouter()->addRoute(pToggle, MsgType::Toggle, [toggleNb, pScreenLineup](Msg* pMsg)
			{
				auto pMessage = static_cast<ToggleMsg*>(pMsg);
				pScreenLineup->slots[toggleNb].setVisible(pMessage->isSet());
			});

		m_pDisplayToggles->slots << pToggle;

		toggleNb++;
	}

	m_pScreenLineup->setSpacing(12);

	//
	
	m_pProgressSlider->setSteps( (int) m_frames.size() );

	
	_logFullStream();
	_updateFrameCounterAndSlider();
	
	_generateFrameStatistics();
	_displayFrameStatistics();
	
}

//____ setFrame() _____________________________________________________________

void MyApp::setFrame( int frame )
{
	if( m_frames.size() == 0 )
		return;
	
	if( frame < 0 )
		frame = 0;
	if( frame >= m_frames.size() )
		frame = int(m_frames.size()) -1;
	
	if( frame == m_currentFrame )
		return;
	
	// Update the diplays and log
	
	if( frame > m_currentFrame )
	{
		_playFrames( m_currentFrame+1, frame, true );
		_playFrames( frame, frame+1, false );
	}
	else
	{
		_resetStream();
		_playFrames( 0, frame, true );
		_playFrames( frame, frame+1, false );
	}

	// Update the log
	
	_logFrames( frame, frame+1, false, m_pFrameLogDisplay );

	//

	m_currentFrame = frame;
	
	// Update slider and frame counter
	
	_updateFrameCounterAndSlider();
	_updateResourcesView();

	if (m_bShowDebugRects)
		_updateDebugOverlays();
}

//____ skipFrames() ___________________________________________________________

void MyApp::skipFrames(int frames)
{
	if (m_frames.size() == 0)
		return;

	int destFrame = m_currentFrame + frames;
	
	if( destFrame >= m_frames.size() )
		destFrame = int(m_frames.size()) -1;

	if( destFrame == m_currentFrame )
		return;
	
	_playFrames( m_currentFrame+1, destFrame+1, true );

	// Update the logs

	_logFrames( m_currentFrame, m_currentFrame+1, false, m_pFrameLogDisplay );

	_logFrames( m_currentFrame+1, destFrame+1, false, m_pOptimizerInLogDisplay);
	_logFrames( m_currentFrame+1, destFrame+1, true, m_pOptimizerOutLogDisplay);

	//

	m_currentFrame = destFrame;
	
	// Update slider and frame counter
	
	_updateFrameCounterAndSlider();
	_updateResourcesView();

	if (m_bShowDebugRects)
		_updateDebugOverlays();

}


//____ showFrameLog() _________________________________________________________

void MyApp::showFrameLog()
{
	m_pLogCapsule->slot = m_pFrameLogContainer;
}

//____ showFullLog() __________________________________________________________

void MyApp::showFullLog()
{
	m_pLogCapsule->slot = m_pFullLogContainer;
}

//____ showOptimizerInLog() ___________________________________________________

void MyApp::showOptimizerInLog()
{
	m_pLogCapsule->slot = m_pOptimizerInLogContainer;

}

//____ showOptimizerOutLog() ___________________________________________________

void MyApp::showOptimizerOutLog()
{
	m_pLogCapsule->slot = m_pOptimizerOutLogContainer;
}


//____ showResources() ________________________________________________________

void MyApp::showResources()
{
	m_pLogCapsule->slot = m_pResourceContainer;
}

//____ showStatistics() ________________________________________________________

void MyApp::showStatistics()
{
	m_pLogCapsule->slot = m_pStatisticsContainer;
}

//____ showErrors() ___________________________________________________________

void MyApp::showErrors()
{
	m_pLogCapsule->slot = m_pErrorsContainer;
}

//____ toggleDebugRects() _____________________________________________________

void MyApp::toggleDebugRects(bool bShow)
{
	m_bShowDebugRects = bShow;

	// Flip visible flag on overlays

	for (auto pScreenStack : m_screenStacks)
	{
		pScreenStack->slots[0].setVisible(bShow);
	}

	//

	if (m_bShowDebugRects)
		_updateDebugOverlays();
}

//____ openRecordedStepsWindow() ______________________________________________

void MyApp::openRecordedStepsWindow()
{
	if( !m_pRecordedStepsWindow )
	{
		m_pRecordedStepsWindow = m_pAppVisitor->createWindow( { .title = "Recorded Steps" } );

		m_pRecordedStepsWindow->setCloseRequestHandler([this](void) {
			this->m_pRecordedStepsWindow = nullptr;
			return true;
		});
		
		auto pRoot = m_pRecordedStepsWindow->rootPanel();

		auto pScroll = ScrollPanel::create( { .skin = m_pPlateSkin } );
		pRoot->slot = pScroll;
		
		auto pStepList = PackPanel::create( { .axis = Axis::Y } );
		
		for( auto& step : m_recordedSteps )
		{
			auto pRow = PackPanel::create( { .axis = Axis::X, .spacing = 4 } );
			pRow->slots.pushBack( SurfaceDisplay::create( { .surface = step.before } ) );
			pRow->slots.pushBack( SurfaceDisplay::create( { .surface = step.after } ) );
			pStepList->slots.pushBack(pRow);
		}
		
		pScroll->slot = pStepList;
		
	}
	
	
}



//____ _resetStream() _________________________________________________________

void MyApp::_resetStream()
{
	m_pStreamPlayer->reset();

	auto pWrapper = StreamWrapper::create( m_pStreamBlob->begin(), m_pStreamBlob->end()  );
	m_pStreamPump->setInput({pWrapper, pWrapper->output});
	m_pStreamPump->pumpUntilFrame();
}

//____ _playFrames() __________________________________________________________

void MyApp::_playFrames( int begin, int end, bool bOptimize )
{
	uint8_t * pBegin = m_frames[begin];
	uint8_t * pEnd = end == m_frames.size() ? (uint8_t*) m_pStreamBlob->end() : (uint8_t*) m_frames[end];

	auto pWrapper = StreamWrapper::create( pBegin, pEnd );
	
	m_pStreamPump->setInput({pWrapper, pWrapper->output});

	m_pStreamPlayer->clearDirtyRects();
/*
	if( bOptimize )
	{
		m_pStreamPump->pumpAllFramesOptimizeClipping();
		m_pStreamPump->pumpAll();						// To make sure any data outside frame following it is included.
	}
	else
*/
		m_pStreamPump->pumpAll();
}

//____ _logFrames() ___________________________________________________________

void MyApp::_logFrames( int begin, int end, bool bOptimize, TextEditor * pDisplay )
{
	uint8_t * pBegin = m_frames[begin];
	uint8_t * pEnd = end == m_frames.size() ? (uint8_t*) m_pStreamBlob->end() : (uint8_t*) m_frames[end];

	auto pWrapper = StreamWrapper::create( pBegin, pEnd );

	std::ostringstream	logStream;
	auto pLogger = StreamLogger::create( logStream );
	pLogger->setDisplayOffset(false);
	
	auto pPump = StreamPump::create( {pWrapper, pWrapper->output}, {pLogger, pLogger->input} );

/*
	if( bOptimize )
		pPump->pumpAllFramesOptimizeClipping();
	else
*/		pPump->pumpAll();

	pDisplay->editor.setText( logStream.str() );
}

//____ _updateFrameCounterAndSlider() _________________________________________

void MyApp::_updateFrameCounterAndSlider()
{
	char temp[20];
	
	snprintf( temp, 20, "%d / %d", m_currentFrame+1, (int) m_frames.size() );
	
	
	m_pProgressText->display.setText(temp);
	
	m_pProgressSlider->setValue( m_currentFrame / float(m_frames.size()) );
}


//____ _logFullStream() _______________________________________________________

void MyApp::_logFullStream()
{
	auto pWrapper = StreamWrapper::create( m_pStreamBlob->begin(), m_pStreamBlob->end()  );

	std::ostringstream	logStream;
	auto pLogger = StreamLogger::create( logStream );
	pLogger->setDisplayChunkNb(true);
	
	auto pPump = StreamPump::create( {pWrapper, pWrapper->output}, {pLogger, pLogger->input} );
	pPump->pumpAll();

	m_pFullLogDisplay->editor.setText( logStream.str() );
}

//____ _updateResourcesView() _________________________________________________

void MyApp::_updateResourcesView()
{
	m_pResourcePanel->slots.clear();
	
	auto objects = m_pStreamPlayer->objects();
	
	int index = 0;
	for( auto pObj : objects )
	{
		if( pObj != nullptr && pObj->isInstanceOf(Surface::TYPEINFO) )
		{
			auto pSurf = wg_static_cast<Surface_p>(pObj);

			auto pWidget = _buildSurfaceDisplayWithIndexTag( pSurf, index );
			m_pResourcePanel->slots.pushBack( pWidget, { .weight = 0 } );
		}
		index++;
	}
	
	m_pResourcePanel->setSpacing(6,12,6);
}

//____ _updateDebugOverlays() _________________________________________________

void MyApp::_updateDebugOverlays()
{
	auto pDevice = GfxBase::defaultGfxDevice();
	
	auto pOverlaySkin = BoxSkin::create( Border(1), HiColor(4096,0,0,1024), HiColor(4096,0,0,4096));
	
	pDevice->beginRender();
	
	for( int i = 0 ; i < m_debugOverlays.size() ; i++ )
	{
		CanvasRef canvas = (CanvasRef) (i+1);
		auto pOverlay = m_debugOverlays[i];
		auto [nRects, pRects] = m_pStreamPlayer->dirtyRects(canvas);

		pDevice->beginCanvasUpdate(m_debugOverlays[i]);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(Color::Transparent);
		pDevice->setBlendMode(BlendMode::Blend);
		for( int x = 0 ; x < nRects ; x++ )
		{
			pOverlaySkin->_render(pDevice, pRects[x], 64, State::Default);
		}
		pDevice->endCanvasUpdate();


		pDevice->endRender();
	}
}


//____ _standardScrollPanel() _________________________________________________

ScrollPanel_p MyApp::_standardScrollPanel()
{
	auto pWidget = ScrollPanel::create();

	pWidget->scrollbarX.setBackground(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color8::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black)));
	pWidget->scrollbarX.setBar(m_pPlateSkin);

	pWidget->scrollbarY.setBackground(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color8::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black)));
	pWidget->scrollbarY.setBar(m_pPlateSkin);

	pWidget->setAutohideScrollbars(true, true);
	pWidget->setSizeConstraints(SizeConstraint::GreaterOrEqual, SizeConstraint::GreaterOrEqual);

	return pWidget;
}


//____ _buildSurfaceDisplayWithIndexTag() ______________________________________

Widget_p MyApp::_buildSurfaceDisplayWithIndexTag( Surface * pSurf, int index )
{
	auto pBase = PackPanel::create( { .axis = Axis::Y, .id = index, .spacing = 4 });
	pBase->setAxis(Axis::Y);
	pBase->setId(index);
	
	char	label[256];
	snprintf( label, 256, "%d: %dx%d %s", index, pSurf->pixelWidth(), pSurf->pixelHeight(), toString(pSurf->pixelFormat()) );
	
	auto pLabel = TextDisplay::create( WGBP(TextDisplay,
						 _.display.style = m_pBigWhiteStyle,
						 _.display.text = label,
						_.display.layout = m_pTextLayoutCentered
				));
	
	auto pDisplay = SurfaceDisplay::create( WGBP(SurfaceDisplay,
												 _.surface = pSurf,
												 _.skin = m_pRedOutlinedBlackSkin ));
	
	auto pCentering = StackPanel::create();
	pCentering->slots << pDisplay;
	pCentering->slots[0].setPlacement(Placement::Center);
	pCentering->slots[0].setSizePolicy(SizePolicy2D::Original);

	
	pBase->slots << pLabel;
	pBase->slots << pCentering;
		
	return pBase;
}

//____ 	_generateFrameStatistics() ____________________________________________

void MyApp::_generateFrameStatistics()
{
	m_frameStatistics.clear();
	
	auto pDecoder = StreamDecoder::create();
	pDecoder->setInput( m_pStreamBlob->begin(), m_pStreamBlob->end() );

	GfxStream::Header header;
	
	* pDecoder >> header;
	while( header.type != GfxStream::ChunkId::OutOfData )
	{
		switch( header.type )
		{
			case GfxStream::ChunkId::BeginRender:
			{
				m_frameStatistics.push_back(FrameStats());
				pDecoder->skip(header.size);
				break;
			}
			case GfxStream::ChunkId::UpdateRects:
			{
				//TODO: Implement!!!

				break;
			}

/*
			case GfxStream::ChunkId::BeginCanvasUpdate:
			{
				uint16_t	surfaceId;
				CanvasRef	canvasRef;
				uint8_t		dummy;
					
				*pDecoder >> surfaceId;
				*pDecoder >> canvasRef;
				*pDecoder >> dummy;

				int nUpdateRects = (header.size - 4) / 16;
				int allocBytes = nUpdateRects*sizeof(RectSPX);
				
				RectSPX * pRects = (RectSPX *) Base::memStackAlloc(allocBytes);
				* pDecoder >> GfxStream::ReadSpxField{ nUpdateRects*4, header.spxFormat, (spx*) pRects };
				
				auto& frame = m_frameStatistics.back();
				
				for( int i = 0 ; i < nUpdateRects ; i++ )
					frame.canvasPixels[(int)canvasRef] += (pRects[i].w/64) * (pRects[i].h/64);
				
				frame.canvasRects[(int)canvasRef] = nUpdateRects;
				
				Base::memStackFree(allocBytes);
				break;
			}
 */
			default:
				pDecoder->skip(header.size);
				break;
		}
		* pDecoder >> header;
	}
}

//____ _displayFrameStatistics() ______________________________________________

void MyApp::_displayFrameStatistics()
{
	std::ostringstream	logStream;

	logStream << "frame ";
	for( int i = 1 ; i < m_screens.size() ; i++ )
	{
		logStream << ", " << toString((CanvasRef)i) << " rects";
		logStream << ", " << toString((CanvasRef)i) << " pixels";
	}
	logStream << std::endl;
	
	
	int frameNb = 0;
	for( auto& frame : m_frameStatistics )
	{
		logStream << frameNb++;

		for( int i = 1 ; i < m_screens.size() ; i++ )
		{
			logStream << ", " << frame.canvasRects[i];
			logStream << ", " << frame.canvasPixels[i];
		}
		logStream << std::endl;
	}

	m_pStatisticsDisplay->editor.setText( logStream.str() );

}
