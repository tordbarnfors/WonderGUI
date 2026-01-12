
#include "editorwindow.h"
#include "editera.h"

#include <wondergui.h>
#include <wg_freetypefont.h>
#include <string>
#include <fstream>


using namespace wg;
using namespace wapp;
using namespace std;


//____ constructor ____________________________________________________________

EditorWindow::EditorWindow(API * pAPI, Editera* pApp, std::string title, std::string path ) : Window(pAPI,{ .open = true, .resizable = true, .size = {800,600}, .title = title })
{
	m_pAPI = pAPI;
	m_pAPI = pAPI;
	m_pApp = pApp;
	m_title = title;

	m_pTextBuffer = TextBuffer::create(pApp, pAPI, pApp->m_pTheme, pApp->m_pEditorLayout, pApp->m_pEditorStyle );

//	m_path = path;
		
	_setupGUI();
	
	if( !path.empty() )
		m_pTextBuffer->loadFromFile(path);
}

//____ destructor _____________________________________________________________

EditorWindow::~EditorWindow()
{

}

//____ update() _______________________________________________________________

void EditorWindow::update()
{
	if( m_bContentModified != m_pTextBuffer->isEmpty() )
	{
		m_bContentModified = m_pTextBuffer->isEmpty();

		bool bDisableSave = m_pTextBuffer->isEmpty();

		m_pSaveButton->setDisabled(bDisableSave);
		m_pSaveAsButton->setDisabled(bDisableSave);
	}

}

//____ _setupGUI() _______________________________________________________

bool EditorWindow::_setupGUI()
{
	//

	auto pTheme = m_pApp->m_pTheme;

	auto pMainContainer = PackPanel::create();
	pMainContainer->setAxis(Axis::Y);

	// Create and setup the top-bar

	auto pTopBar = _createTopBar();

	// Create and setup a scrollpanel to wrap the text editor.

	auto pScrollPanel = ScrollPanel::create(pTheme->scrollPanelXY());

	//	pScrollPanel->setAutohideScrollbars(true, true);
	//	pScrollPanel->setSizeConstraints(SizeConstraint::GreaterOrEqual, SizeConstraint::GreaterOrEqual);

	pScrollPanel->setTransition(CoordTransition::create(300000));


	pScrollPanel->slot = m_pTextBuffer->editor();


	// 

	pMainContainer->slots << pTopBar;
	pMainContainer->slots << pScrollPanel;
	pMainContainer->setSlotWeight(0, 2, { 0.f, 1.f });


	pMainContainer->setLayout(PackLayout::create( {
		.expandFactor = PackLayout::Factor::Weight,
		.shrinkFactor = PackLayout::Factor::Weight
	} ));


	mainCapsule()->slot = pMainContainer;
	return true;
}



//____ _createTopBar() ________________________________________________________

Widget_p EditorWindow::_createTopBar()
{
	auto pTheme = m_pApp->m_pTheme;

	auto pBar = WGCREATE( PackPanel, _.axis = Axis::X, _.layout = m_pApp->m_pLayout, _.skin = pTheme->plateSkin() );

	auto pClearButton 	= WGCREATE( Button, _= pTheme->pushButton(), _.label.text = "Clear");
	auto pNewButton 	= WGCREATE( Button, _= pTheme->pushButton(), _.label.text = "New");
	auto pLoadButton 	= WGCREATE( Button, _= pTheme->pushButton(), _.label.text = "Load");
	auto pSaveButton 	= WGCREATE( Button, _= pTheme->pushButton(), _.label.text = "Save");
	auto pSaveAsButton 	= WGCREATE( Button, _= pTheme->pushButton(), _.label.text = "Save as...");

	auto pSpacer = WGCREATE( Filler, _.defaultSize = { 20,1 } );

	pBar->slots.pushBack({ { pClearButton, {.weight = 0} },
						   { pNewButton, {.weight = 0} },
						   { pLoadButton, {.weight = 0} },
						   { pSaveButton, {.weight = 0 }},
						   { pSaveAsButton, {.weight = 0 }},
							pSpacer });

	auto pRouter = Base::msgRouter();

	pRouter->addRoute(pClearButton, MsgType::Select, [this](Msg* pMsg) {this->m_pTextBuffer->clear(); });
	pRouter->addRoute(pNewButton, MsgType::Select, [this](Msg* pMsg) {this->m_pApp->createEditorWindow( "", "" ); });

	pRouter->addRoute(pLoadButton, MsgType::Select, [this](Msg* pMsg) {this->_selectAndLoadFile(); });
	pRouter->addRoute(pSaveButton, MsgType::Select, [this](Msg* pMsg) {this->_saveFileCallback(); });
	pRouter->addRoute(pSaveAsButton, MsgType::Select, [this](Msg* pMsg) {this->_selectAndSaveFile(); });

	m_pSaveButton = pSaveButton;
	m_pSaveAsButton = pSaveAsButton;
	 
	return pBar;
}


//____ _selectAndLoadFile() ___________________________________________________

bool EditorWindow::_selectAndLoadFile()
{
	auto path = m_pAPI->openFileDialog("Load File", "", "", {"*.txt"}, "Text Files");

	if( path.empty() )
		return false;
	
	
	if( !m_pTextBuffer->isEmpty() )
		return m_pApp->createEditorWindow("", path );
	else
		return m_pTextBuffer->loadFromFile(path);
}

//____ _selectAndSaveFile() ___________________________________________________

bool EditorWindow::_selectAndSaveFile()
{
	auto selectedFile = m_pAPI->saveFileDialog("Save File", "", "", { "*.*" }, "Text Files");

	if (selectedFile.empty())
		return false;

	return m_pTextBuffer->saveToFile(selectedFile);
}

//____ _saveFileCallback() ___________________________________________________

bool EditorWindow::_saveFileCallback()
{
	if( m_pTextBuffer->path().empty())
	{
		return _selectAndSaveFile();
	}
	else
		return m_pTextBuffer->saveToFile(m_pTextBuffer->path());
}

//____ _setTitle() ___________________________________________________

void EditorWindow::_setTitle(const std::string& path)
{
	auto lastSlash = path.find_last_of("/\\");
	if (lastSlash != std::string::npos)
		m_title = path.substr(lastSlash + 1);
	else
		m_title = path;

	setTitle(m_title);
}
