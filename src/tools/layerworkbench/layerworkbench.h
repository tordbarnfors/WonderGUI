
#pragma once

#include <wonderapp.h>
#include <wondergui.h>

#include <initializer_list>
#include <string>

using namespace wg;

class MyApp : public WonderApp
{
public:
	bool		init(wapp::API* pAPI) override;
	bool		update() override;
	void		exit() override;

	void		closeWindow(wapp::Window* pWindow) override;

private:
	
	bool		setupGUI(wapp::API * pAPI);
	
	Widget_p	createMovableBox( Skin * pSkin, FlexPanel * pParent );
	Widget_p	makeMovable(const Widget_p& pWidget, FlexPanel* pParent);


	wapp::Window_p	m_pWindow;
	
};


