
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

class EditorWindow : public wapp::Window
{
public:

	static EditorWindow_p create(wapp::API* pAPI, MyApp* pApp, std::string title, std::string path) { return EditorWindow_p(new EditorWindow(pAPI, pApp, title, path)); }

	void	setContent(const char* pContent);

	void	focus() { m_pEditor->grabFocus(true); }
	
	const std::string& title() const { return m_title; }
	const std::string& path() const { return m_path; }

protected:

	EditorWindow(wapp::API* pAPI, MyApp* pApp, std::string title, std::string path );
	~EditorWindow();

	bool			_setupGUI();
	Widget_p		_createTopBar();

	void			_contentModified(Msg * pMsg);

	void			_clear();
	bool			_selectAndLoadFile();
	bool			_selectAndSaveFile();
	bool			_saveFileCallback();
	bool			_loadFile( const std::string& path );
	bool			_saveFile(const std::string& path);
	void			_setPathAndTitle(const std::string& path);

	MyApp*			m_pApp;
	wapp::API*		m_pAPI;

	RootPanel_p		m_pRootPanel;
	TextEditor_p	m_pEditor;

	Button_p		m_pSaveButton;
	Button_p		m_pSaveAsButton;

	std::string		m_title;
	std::string		m_path;
};

