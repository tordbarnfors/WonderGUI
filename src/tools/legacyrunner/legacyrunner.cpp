
#include "legacyrunner.h"

#include <wondergui.h>

#include <wg_freetypefont.h>
#include <string>
#include <fstream>

#include <wg_legacyrootcapsule.h>
#include <wg2_knob.h>

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
	WgBase::Init( Base::hostBridge() );
	
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
	WgBase::Exit();
}


//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(Visitor* pVisitor)
{
	m_pWindow = pVisitor->createWindow({ .size = {800,700}, .title = "LegacyRunner" });

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

	
	auto pBasePanel = FlexPanel::create();
	pBasePanel->setSkin(ColorSkin::create(Color::Beige));

	m_pBasePanel = pBasePanel;

	m_pWindow->setContent(pBasePanel);
	
	
	auto pLegacyRoot = LegacyRootCapsule::create( { .skin = ColorSkin::create(Color::Black) });
	
//	auto pLegacyRoot = Filler::create( { .skin = ColorSkin::create(Color::Black) } );
	
	pBasePanel->slots.pushBack( pLegacyRoot, { .pos = {10,10} } );
		
	_setupLegacyKeyMap();
	auto pWg2Tree = _buildLegacyGUI();
	
	pLegacyRoot->SetChild(pWg2Tree);

	return true;
}

//____ _setupLegacyKeyMap() ___________________________________________________

void MyApp::_setupLegacyKeyMap()
{
	auto pInput = Base::inputHandler();
	for( auto key = Key::Left ; key < Key::Unmapped ; key = (Key) (int(key)+1) )
	{
		int native = pInput->nativeKeyCode(key);
		if( native != 0 )
			WgBase::MapKey( (WgKey) key, native);
	}
}

//____ _buildLegacyGUI() ______________________________________________________

WgWidget * MyApp::_buildLegacyGUI()
{
	auto pPanel = new WgPackPanel();
	
	
	auto pDisplay = new WgTextDisplay();
	pDisplay->SetEditMode(WgTextEditMode::Editable);
	pDisplay->SetText("\nTEXT  1.");
	pDisplay->SetSkin( ColorSkin::create({
		.states = { { State::Default, Color::Pink },
			{ State::Hovered, Color::Red }
		}
	}));

	pPanel->AddChild(pDisplay);
	
	
	auto pKnob = new WgKnob( Base::defaultSurfaceFactory() );
	
	pPanel->AddChild(pKnob);
	
	
	return pPanel;
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

	m_pTightButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pButtonSurf,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = { 4,4,4,4 },
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

