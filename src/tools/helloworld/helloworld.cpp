
#include "helloworld.h"

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

bool MyApp::init(wapp::API * pAPI)
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
	return true;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{

}


//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(wapp::API* pAPI)
{
	m_pWindow = wapp::Window::create(pAPI, { .size = {800,600}, .title = "Hello World" });

	//

	auto pFontBlob = pAPI->loadBlob("resources/DroidSans.ttf");
	auto pFont = FreeTypeFont::create(pFontBlob);

	m_pTextStyle = TextStyle::create({
		.color = Color8::Black,
		.font = pFont, 
		.size = 14,
		.states = { {State::Disabled, Color8::DarkGrey}}
	});

	Base::setDefaultStyle(m_pTextStyle);

	m_pTextLayoutCentered = BasicTextLayout::create({ .placement = Placement::Center });

	//

	auto pBgSkin = ColorSkin::create( Color::PapayaWhip );

	auto pTextDisplay = TextDisplay::create({

		.display = {
			.layout = m_pTextLayoutCentered, 
			.style = m_pTextStyle, 
			.text = "Hello World!" 
		},

		.skin = pBgSkin
	});

	m_pWindow->mainCapsule()->slot = pTextDisplay;
	return true;
}
