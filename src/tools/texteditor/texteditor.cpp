
#include "texteditor.h"

#include <wondergui.h>
#include <wg_freetypefont.h>
#include <string>
#include <fstream>


using namespace wg;
using namespace wapp;
using namespace std;

uint64_t	startTime;

//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(API* pAPI)
{
	m_pAPI = pAPI;

	setupGUI();

	// Add any text-file from the argument list

	auto arguments = pAPI->programArguments();

	if( arguments.empty() )
	{
		createEditorWindow("Untitled 1", "");

	}
	else
	{
		for (auto& arg : arguments)
			createEditorWindow("", arg);
	}

	startTime = m_pAPI->time();

	return true;
}

//____ update() _______________________________________________________________

bool MyApp::update()
{
/*
	auto time = m_pAPI->time();
	
	if( time - startTime > 5000000 )
	{
		m_editorWindows.front()->focus();
		startTime = time;
	}
*/
	return !m_editorWindows.empty();
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{
}

//____ closeWindow() __________________________________________________________

void MyApp::closeWindow(wapp::Window* _pWindow)
{
	auto pClose = static_cast<EditorWindow*>(_pWindow);

	m_editorWindows.erase(std::remove_if(m_editorWindows.begin(),
	m_editorWindows.end(),
	[pClose](const EditorWindow_p& pWindow ) {return pWindow.rawPtr() == pClose;}  ),
	m_editorWindows.end());
}

//____ setupGUI() ____________________________________________________________

bool MyApp::setupGUI()
{

	m_pTheme = m_pAPI->initDefaultTheme();

	m_pEditorStyle = m_pTheme->monospaceStyle();
					
	m_pTextLayoutCentered = BasicTextLayout::create(WGBP(BasicTextLayout,
		_.placement = Placement::Center));


	//

	return true;
}

//____ createEditorWindow() ______________________________________________________

bool MyApp::createEditorWindow( const std::string& windowTitle, const std::string& path )
{
	std::string title;
	
	if( windowTitle.empty() )
		title = _createWindowTitle(path);
	else
		title = windowTitle;

	auto pEditorWindow = EditorWindow::create(m_pAPI, this, title, path );

	m_editorWindows.push_back(pEditorWindow);
	return true;
}

//____ _createWindowTitle() ___________________________________________________

std::string MyApp::_createWindowTitle( const std::string& path )
{
	std::string title;
	
	if( !path.empty() )
	{
		title = path.substr(path.find_last_of("/\\") + 1);
	}
	else
	{
		int nb = 1;

		bool bUnique = false;
		while(!bUnique)
		{
			title = "Untitled " + std::to_string(nb++);
			bUnique = true;

			for( auto pWin : m_editorWindows )
			{
				if( title.compare(pWin->title()) == 0 )
				{
					bUnique = false;
					break;
				}
			}
		}
	}
	
	return title;
}
