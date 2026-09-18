
#include "gfxbench.h"

#include <wondergui.h>
#include <wg_freetypefont.h>
#include <string>



using namespace wg;
using namespace wapp;
using namespace std;

//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new GfxBench();
}

//____ init() _________________________________________________________________

bool GfxBench::init(API * pAPI)
{
	m_pAPI = pAPI;

	if (!_setupGUI(pAPI))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}

	return true;
}

//____ update() _______________________________________________________________

bool GfxBench::update()
{
	return m_bContinue;
}

//____ exit() _________________________________________________________________

void GfxBench::exit()
{

}

//____ closeWindow() __________________________________________________________

void GfxBench::closeWindow(Window* pWindow)
{
	m_pWindow = nullptr;
	m_bContinue = false;
}

//____ _setupGUI() ____________________________________________________________

bool GfxBench::_setupGUI(API* pAPI)
{
	m_pWindow = Window::create(pAPI, { .size = {800,600}, .title = "GfxBench" });

	//

	auto pFontBlob = pAPI->loadBlob("resources/DroidSans.ttf");

	if (!pFontBlob)
		return false;

	auto pFont = FreeTypeFont::create(pFontBlob);

	m_pTextStyle = TextStyle::create({
		.color = Color8::Black,
		.font = pFont,
		.size = 14,
	});

	m_pTextLayoutCentered = BasicTextLayout::create({ .placement = Placement::Center });

	//

	auto pBgSkin = ColorSkin::create( Color::PapayaWhip );

	auto pFlexPanel = FlexPanel::create({ 
		.edgePolicy = EdgePolicy::Clip,
		.skin = pBgSkin
		});

	auto pFiller = Filler::create({
		.skin = ColorSkin::create(Color::Navy)
		});

	pFlexPanel->slots.pushBack(pFiller, { .pos = {10,10}, .size = {150,50} });

	m_pWindow->mainCapsule()->slot = pFlexPanel;
	return true;
}
