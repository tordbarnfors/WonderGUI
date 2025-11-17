
#pragma once

#include <wonderapp.h>

#include <wondergui.h>
#include <wg_freetypefont.h>

#include <initializer_list>
#include <string>

using namespace wg;

class MyApp : public WonderApp
{
public:

	bool		init(wapp::API* pAPI) override;
	bool		update() override;
	void		exit() override;


private:

	bool			_setupGUI(wapp::API* pAPI);

	wapp::API *		m_pAPI = nullptr;
	wapp::Window_p	m_pWindow = nullptr;
	
	TextStyle_p		m_pTextStyle;
	TextStyle_p		m_pLabelStyle;

	TextLayout_p	m_pTextLayoutCentered;
};
