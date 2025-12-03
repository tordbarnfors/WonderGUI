
#include "helloworld.h"

#include <wondergui.h>
#include <wg_freetypefont.h>
#include <string>



using namespace wg;
using namespace wapp;
using namespace std;

//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(API * pAPI)
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

bool MyApp::update()
{
	return m_bContinue;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{

}

//____ closeWindow() __________________________________________________________

void MyApp::closeWindow(Window* pWindow)
{
	m_pWindow = nullptr;
	m_bContinue = false;
}

//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(API* pAPI)
{
	m_pWindow = Window::create(pAPI, { .size = {800,600}, .title = "Hello World" });

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
/*
	auto pTextDisplay = TextDisplay::create({

		.display = {
			.layout = m_pTextLayoutCentered, 
			.style = m_pTextStyle, 
			.text = "Hello World!" 
		},

		.skin = pBgSkin
	});

	m_pWindow->mainCapsule()->slot = pTextDisplay;
*/

	auto pFiller = Filler::create({
		.skin = pBgSkin
		});

	m_pWindow->mainCapsule()->slot = pFiller;
	return true;
}
