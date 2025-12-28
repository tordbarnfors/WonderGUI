
#pragma once

#include <wonderapp.h>

#include <wondergui.h>
#include <wg_freetypefont.h>

#include <initializer_list>
#include <string>

#include "editorwindow.h"

using namespace wg;

class MyApp : public WonderApp
{
public:
	
	bool		init(wapp::API* pAPI) override;
	bool		update() override;
	void		exit() override;

	void		closeWindow(wapp::Window* pWindow) override;

	bool		setupGUI();
	bool 		createEditorWindow( const std::string& windowTitle, const std::string& path );
	
	// These are made public for our subclasses.

	wapp::API* 		m_pAPI = nullptr;
	wg::Theme_p		m_pTheme;

	PackLayout_p	m_pLayout;

	TextStyle_p		m_pLabelStyle;
	TextStyle_p		m_pEditorStyle;

	TextLayout_p	m_pTextLayoutCentered;
	
private:

	std::string		_createWindowTitle( const std::string& path );
	
	std::vector<EditorWindow_p>	m_editorWindows;
	
};
