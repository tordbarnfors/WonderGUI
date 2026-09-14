
#pragma once

#include <wonderapp.h>
#include <wondergui.h>

#include "test.h"

#include <vector>

using namespace wg;

class MyApp : public WonderApp
{
public:

	bool		init(wapp::API* pAPI) override;
	bool		update() override;
	void		exit() override;

	void		closeWindow(wapp::Window* pWindow) override;

private:

	bool		_setupGUI(wapp::API* pAPI);
	void		_selectTest(int index);

	wapp::API *		m_pAPI = nullptr;
	wapp::Window_p	m_pWindow;

	SelectBox_p		m_pPicker;
	Capsule_p		m_pContent;

	std::vector<const Test*>	m_sortedTests;
};
