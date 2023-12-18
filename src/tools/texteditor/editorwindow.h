
#pragma once

#include <wonderapp.h>

#include <wondergui.h>
#include <wg_freetypefont.h>

#include <initializer_list>
#include <string>

using namespace wg;


class EditorWindow;
typedef StrongPtr<EditorWindow>		EditorWindow_p;

class MyApp;

class EditorWindow : public Object
{
public:

	static EditorWindow_p create(Window_p pWindow, MyApp* pApp, std::string title, std::string path) { return EditorWindow_p(new EditorWindow(pWindow, pApp, title, path)); }

	void	setContent(const char* pContent);

	void	focus() { m_pEditor->grabFocus(true); }
	
	const std::string& title() const { return m_title; }
	const std::string& path() const { return m_path; }

protected:

	EditorWindow(Window_p pWindow, MyApp* pApp, std::string title, std::string path );
	~EditorWindow();

	bool			_setupGUI();
	Widget_p 		_createTopBar();

	
	void			_clear();
	bool			_selectAndLoadFile();
	bool			_selectAndSaveFile();
	bool			_saveFile();
	bool			_loadFile( const std::string& path );

	MyApp*			m_pApp;
	Window_p	    m_pWindow;

	RootPanel_p		m_pRootPanel;
	TextEditor_p	m_pEditor;

	std::string		m_title;
	std::string		m_path;
};

