
#include "pluginworkbench.h"

#include <wondergui.h>
#include <wg_freetypefont.h>
#include <wg_key.h>

#include <string>
#include <fstream>

#ifdef WIN32
#	include <SDL.h>
#elif __APPLE__
#	include <SDL2/SDL.h>
#else
#	include <SDL2/SDL.h>
#endif


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

	
//	m_libId = pVisitor->openLibrary("legacytestplugin");
	m_libId = pVisitor->openLibrary("testplugin");
	if( m_libId == 0 )
		return false;
	
	
	 
	m_pInitClient = (initClientFunc) pVisitor->loadSymbol(m_libId, "init" );
	m_pUpdateClient = (updateClientFunc)pVisitor->loadSymbol(m_libId, "update" );
	m_pExitClient = (exitClientFunc)pVisitor->loadSymbol(m_libId, "exitX" );

	void * pCreateBitmapCache = pVisitor->loadSymbol(m_libId, "wg_getTypeInfo" );


	wg_plugin_interface	c_calls;
	wg_populatePluginInterface(&c_calls);

	m_pInitClient(&c_calls, Base::hostBridge(), m_pPluginCapsule,
				  Base::defaultGfxDevice(), Base::defaultSurfaceFactory(), Base::defaultEdgemapFactory());
	
	// Map keys and commands
	
	mapInputKeyFunc 	pMapKeyFunc 	= (mapInputKeyFunc) pVisitor->loadSymbol(m_libId, "mapInputKey" );
	mapInputCommandFunc pMapCommandFunc = (mapInputCommandFunc) pVisitor->loadSymbol(m_libId, "mapInputCommand" );

	
	pMapKeyFunc(SDLK_LEFT, WG_KEY_LEFT);
	pMapKeyFunc(SDLK_RIGHT, WG_KEY_RIGHT);
	pMapKeyFunc(SDLK_UP, WG_KEY_UP);
	pMapKeyFunc(SDLK_DOWN, WG_KEY_DOWN);
	pMapKeyFunc(SDLK_BACKSPACE, WG_KEY_BACKSPACE);
	pMapKeyFunc(SDLK_DELETE, WG_KEY_DELETE);
	pMapKeyFunc(SDLK_END, WG_KEY_END);
	pMapKeyFunc(SDLK_ESCAPE, WG_KEY_ESCAPE);
	pMapKeyFunc(SDLK_HOME, WG_KEY_HOME);
	pMapKeyFunc(SDLK_PAGEDOWN, WG_KEY_PAGE_DOWN);
	pMapKeyFunc(SDLK_PAGEUP, WG_KEY_UP);
	pMapKeyFunc(SDLK_RETURN, WG_KEY_RETURN);
	pMapKeyFunc(SDLK_SPACE, WG_KEY_SPACE);
	pMapKeyFunc(SDLK_TAB, WG_KEY_TAB);
	pMapKeyFunc(SDLK_F1, WG_KEY_F1);
	pMapKeyFunc(SDLK_F2, WG_KEY_F2);
	pMapKeyFunc(SDLK_F3, WG_KEY_F3);
	pMapKeyFunc(SDLK_F4, WG_KEY_F4);
	pMapKeyFunc(SDLK_F5, WG_KEY_F5);
	pMapKeyFunc(SDLK_F6, WG_KEY_F6);
	pMapKeyFunc(SDLK_F7, WG_KEY_F7);
	pMapKeyFunc(SDLK_F8, WG_KEY_F8);
	pMapKeyFunc(SDLK_F9, WG_KEY_F9);
	pMapKeyFunc(SDLK_F10, WG_KEY_F10);
	pMapKeyFunc(SDLK_F11, WG_KEY_F11);
	pMapKeyFunc(SDLK_F12, WG_KEY_F12);


	pMapKeyFunc(SDLK_LCTRL, WG_KEY_STD_CONTROL);
	pMapKeyFunc(SDLK_RCTRL, WG_KEY_STD_CONTROL);

	pMapKeyFunc(SDLK_LSHIFT, WG_KEY_SHIFT);
	pMapKeyFunc(SDLK_RSHIFT, WG_KEY_SHIFT);

	pMapKeyFunc(SDLK_LALT, WG_KEY_ALT);
	pMapKeyFunc(SDLK_RALT, WG_KEY_ALT);

	pMapKeyFunc(SDLK_LGUI, WG_KEY_OS);
	pMapKeyFunc(SDLK_RGUI, WG_KEY_OS);

	
	pMapKeyFunc(SDLK_KP_ENTER, WG_KEY_RETURN);

	pMapCommandFunc(SDLK_x, WG_MODKEYS_STD_CTRL, WG_EDITCMD_CUT);
	pMapCommandFunc(SDLK_c, WG_MODKEYS_STD_CTRL, WG_EDITCMD_COPY);
	pMapCommandFunc(SDLK_v, WG_MODKEYS_STD_CTRL, WG_EDITCMD_PASTE);

	pMapCommandFunc(SDLK_a, WG_MODKEYS_STD_CTRL, WG_EDITCMD_SELECT_ALL);

	pMapCommandFunc(SDLK_z, WG_MODKEYS_STD_CTRL, WG_EDITCMD_UNDO);
	pMapCommandFunc(SDLK_z, WG_MODKEYS_STD_CTRL_SHIFT, WG_EDITCMD_REDO);
	
	pMapCommandFunc(SDLK_ESCAPE, WG_MODKEYS_NONE, WG_EDITCMD_ESCAPE);
	
	return true;
}

//____ update() _______________________________________________________________

bool MyApp::update()
{
	m_pUpdateClient();
	return true;
} 

//____ exit() _________________________________________________________________

void MyApp::exit()
{
	if (m_libId)
	{
		m_pExitClient();
		m_pAppVisitor->closeLibrary(m_libId);
	}
}


//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(Visitor* pVisitor)
{
	m_pWindow = pVisitor->createWindow({ .size = {800,600}, .title = "CABI Workbench" });

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

	auto pPopupOverlay = PopupOverlay::create();
	
	
	
	auto pBasePanel = PackPanel::create();
	pBasePanel->setAxis(Axis::Y);
	pBasePanel->setLayout(m_pLayout);
	
	auto pSplitPanel = SplitPanel::create();
	pSplitPanel->setAxis(Axis::X);
	pSplitPanel->setResizeRatio(1.f);
	pSplitPanel->setSplit(0.5f);			//TODO: SplitPanel::setSplitFactor() does not work! Replace with some other function?
	pSplitPanel->setHandleSkin(m_pButtonSkin);
	
	auto pPluginCapsule = PluginCapsule::create();

	auto pTextEditor = TextEditor::create( { .skin = m_pSectionSkin });
	
	
	

	pSplitPanel->slots[0] = pPluginCapsule;
	pSplitPanel->slots[1] = pTextEditor;

	pBasePanel->slots << pSplitPanel;

	pBasePanel->setSlotWeight(0, 2, {0.f,1.f});

	pPopupOverlay->mainSlot = pBasePanel;
	
	m_pWindow->setContent(pPopupOverlay);

	m_pPluginCapsule = pPluginCapsule;

	return true;
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
