
#include "editorwindow.h"
#include "texteditor.h"

#include <wondergui.h>
#include <wg_freetypefont.h>
#include <string>
#include <fstream>


using namespace wg;
using namespace std;


//____ constructor ____________________________________________________________

EditorWindow::EditorWindow(Window_p pWindow, MyApp* pApp, std::string title, std::string path )
{
	m_pWindow = pWindow;

	m_pApp = pApp;
	m_title = title;
	m_path = path;
		
	_setupGUI();
	
	if( !path.empty() )
		_loadFile(path);
}

//____ destructor _____________________________________________________________

EditorWindow::~EditorWindow()
{

}

//____ setContent() _____________________________________________________

void EditorWindow::setContent(const char* pContent)
{
	m_pEditor->editor.setText(pContent);
}


//____ _setupGUI() _______________________________________________________

bool EditorWindow::_setupGUI()
{
	//

	auto pMainContainer = PackPanel::create();
	pMainContainer->setAxis(Axis::Y);

	// Create and setup the top-bar

	auto pTopBar = _createTopBar();


	// Create our TextEditor widget

	auto pTextEditor = TextEditor::create( { 	.editor = { .style = m_pApp->m_pEditorStyle },
												.skin = m_pApp->m_pSectionSkin });

	Base::msgRouter()->addRoute(pTextEditor, MsgType::TextEdit, [this](Msg * pMsg) { this->_contentModified(pMsg); } );
	

	// Create and setup a scrollpanel to wrap the text editor.

	auto pScrollPanel = m_pApp->createScrollPanel();

	pScrollPanel->slot = pTextEditor;


	// 

	pMainContainer->slots << pTopBar;
	pMainContainer->slots << pScrollPanel;
	pMainContainer->setSlotWeight(0, 2, { 0.f, 1.f });


	pMainContainer->setLayout(PackLayout::create( {
		.expandFactor = PackLayout::Factor::Weight,
		.shrinkFactor = PackLayout::Factor::Weight
	} ));


	m_pWindow->setContent( pMainContainer );
	m_pEditor = pTextEditor;
	return true;
}



//____ _createTopBar() ________________________________________________________

Widget_p EditorWindow::_createTopBar()
{

	auto pBar = PackPanel::create();
	pBar->setAxis(Axis::X);
	pBar->setLayout(m_pApp->m_pLayout);
	pBar->setSkin(m_pApp->m_pPlateSkin);

	auto pClearButton = m_pApp->createButton("Clear");
	auto pNewButton = m_pApp->createButton("New");
	auto pLoadButton = m_pApp->createButton("Load");
	auto pSaveButton = m_pApp->createButton("Save");
	auto pSaveAsButton = m_pApp->createButton("Save as...");

	auto pSpacer = Filler::create(WGBP(Filler, _.defaultSize = { 20,1 }));
	
	pBar->slots.pushBack({ { pClearButton, {.weight = 0} },
						   { pNewButton, {.weight = 0} },
						   { pLoadButton, {.weight = 0} },
						   { pSaveButton, {.weight = 0 }},
						   { pSaveAsButton, {.weight = 0 }},
							pSpacer });

	Base::msgRouter()->addRoute(pClearButton, MsgType::Select, [this](Msg* pMsg) {this->_clear(); });
	Base::msgRouter()->addRoute(pNewButton, MsgType::Select, [this](Msg* pMsg) {this->m_pApp->createEditorWindow( "", "" ); });

	Base::msgRouter()->addRoute(pLoadButton, MsgType::Select, [this](Msg* pMsg) {this->_selectAndLoadFile(); });
	Base::msgRouter()->addRoute(pSaveButton, MsgType::Select, [this](Msg* pMsg) {this->_saveFile(); });
	Base::msgRouter()->addRoute(pSaveAsButton, MsgType::Select, [this](Msg* pMsg) {this->_selectAndSaveFile(); });

	m_pSaveButton = pSaveButton;
	m_pSaveAsButton = pSaveAsButton;
	
	return pBar;
}


//____ _clear() ___________________________________________________

void EditorWindow::_clear()
{
	m_pEditor->editor.clear();
}


//____ _selectAndLoadFile() ___________________________________________________

bool EditorWindow::_selectAndLoadFile()
{
	auto path = m_pApp->m_pAppVisitor->openFileDialog("Load File", "", { "*.txt", "*.*" }, "Text Files");
	
	if( path.empty() )
		return false;
	
	
	if( !m_pEditor->editor.isEmpty() )
		return m_pApp->createEditorWindow("", path );
	else
		return _loadFile(path);
}

//____ _selectAndSaveFile() ___________________________________________________

bool EditorWindow::_selectAndSaveFile()
{
	/*
	 auto selectedFiles = m_pAppVisitor->openMultiFileDialog("Select Images", "", { "*.surf", "*.qoi" }, "Image files");

		if( selectedFiles.empty()  )
			return;

		m_imagePaths = selectedFiles;
		loadImage(0);
	*/
	
	return false;
}

//____ _loadFile() ____________________________________________________

bool EditorWindow::_loadFile( const std::string& path )
{
	auto pBlob = m_pApp->m_pAppVisitor->loadBlob(path);
	if( !pBlob )
		return false;
	
	m_pEditor->editor.setText((char*) pBlob->data());

	return true;
}


//____ _saveFile() ___________________________________________________

bool EditorWindow::_saveFile()
{
	if( m_path.empty() )
	{
		return _selectAndSaveFile();
	}
	
	
	
	return false;
}

//____ _contentModified() ____________________________________________________

void EditorWindow::_contentModified(Msg* pMsg)
{
	bool bDisableSave = m_pEditor->editor.length() == 0;
	
	m_pSaveButton->setDisabled(bDisableSave);
	m_pSaveAsButton->setDisabled(bDisableSave);
}

