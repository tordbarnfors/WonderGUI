
#include "widgetgallery.h"

#include <wondergui.h>
#include <wg_freetypefont.h>
#include <string>
#include <fstream>

#include <themes/simplistic/wg_simplistic.h>
//#include <themes/steel_n_red/wg_steel_n_red.h>



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

bool MyApp::_setupGUI(Visitor* pVisitor )
{
	m_pWindow = pVisitor->createWindow({ .size = {800,600}, .title = "WonderGUI Widget Gallery" });

	auto pRoot = m_pWindow->rootPanel();

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

	auto pThemeSurf = pVisitor->loadSurface("resources/newskin/skin_widgets.png");

	auto pTheme = Simplistic::create(pFont, pFont, pFont, pFont, pThemeSurf);



	m_pLayout = PackLayout::create({ .wantedSize = PackLayout::WantedSize::Default,
	.expandFactor = PackLayout::Factor::Weight, .shrinkFactor = PackLayout::Factor::Zero });

	auto pPopupOverlay = PopupOverlay::create();
	
	
	
	auto pBasePanel = PackPanel::create();
	pBasePanel->setAxis(Axis::Y);
	pBasePanel->setLayout(m_pLayout);
	
	

	pBasePanel->slots << createTopBar();
	pBasePanel->slots << createGallery(pTheme);

	pBasePanel->setSlotWeight(0, 2, {0.f,1.f});

	pPopupOverlay->mainSlot = pBasePanel;
	
	pRoot->slot = pPopupOverlay;

	return true;
}


//____ createTopBar() ________________________________________________________

Widget_p MyApp::createTopBar()
{
	
	auto pBar = PackPanel::create();
	pBar->setAxis(Axis::X);
	pBar->setLayout(m_pLayout);
	pBar->setSkin(BoxSkin::create({ .color = Color::LightGray, .outlineColor = Color::Black, .outlineThickness = 1, .padding = 3 } ));

	return pBar;
}

//____ createGallery() _____________________________________________________

Widget_p MyApp::createGallery( Theme * pTheme )
{

	auto pGalleryPanel = PackPanel::create();
	pGalleryPanel->setAxis(Axis::Y);
	pGalleryPanel->setSkin(BoxSkin::create({ .color = Color::LightGray, .outlineColor = Color::Black, .outlineThickness = 1 }));


	auto pRadioButton1= ToggleButton::create(WGOVR(pTheme->radioButton(), _.label.text = "Radio Button 1" ));
	auto pRadioButton2 = ToggleButton::create(WGOVR(pTheme->radioButton(), _.label.text = "Radio Button 2"));
	auto pRadioButton3 = ToggleButton::create(WGOVR(pTheme->radioButton(), _.label.text = "Radio Button 3"));

	pGalleryPanel->slots.pushBack( pRadioButton1, { .weight = 0.f } );
	pGalleryPanel->slots.pushBack( pRadioButton2, { .weight = 0.f } );
	pGalleryPanel->slots.pushBack( pRadioButton3, { .weight = 0.f } );
	pGalleryPanel->slots.pushBack( Filler::create(), { .weight = 1.f } );

	auto pRadioGroup = ToggleGroup::create();
	pRadioGroup->add(pRadioButton1);
	pRadioGroup->add(pRadioButton2);
	pRadioGroup->add(pRadioButton3);


	return pGalleryPanel;
	
	
/*
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
*/
}

