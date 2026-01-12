
#pragma once

#include <wonderapp.h>

#include <wondergui.h>
#include <wg_freetypefont.h>

#include "textbuffer.h"

#include <initializer_list>
#include <string>

using namespace wg;


class EditorWindow;
typedef StrongPtr<EditorWindow>		EditorWindow_p;

class Editera;

class EditorWindow : public wapp::Window
{
public:

	static EditorWindow_p create(wapp::API* pAPI, Editera* pApp, std::string title, std::string path) { return EditorWindow_p(new EditorWindow(pAPI, pApp, title, path)); }

	void	update();

	void	focus() { m_pTextBuffer->grabFocus(); }
	
	const std::string& title() const { return m_title; }

protected:

	EditorWindow(wapp::API* pAPI, Editera* pApp, std::string title, std::string path );
	~EditorWindow();

	bool			_setupGUI();
	Widget_p		_createTopBar();

	bool			_selectAndLoadFile();
	bool			_selectAndSaveFile();
	bool			_saveFileCallback();
	void			_setTitle(const std::string& path);

	Editera*		m_pApp;
	wapp::API*		m_pAPI;

	RootPanel_p		m_pRootPanel;

	Button_p		m_pSaveButton;
	Button_p		m_pSaveAsButton;

	std::string		m_title;

	TextBuffer_p	m_pTextBuffer;
	bool			m_bContentModified = false;
};

