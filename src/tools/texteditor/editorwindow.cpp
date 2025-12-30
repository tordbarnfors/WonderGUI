
#include "editorwindow.h"
#include "texteditor.h"

#include <wondergui.h>
#include <wg_freetypefont.h>
#include <string>
#include <fstream>


using namespace wg;
using namespace wapp;
using namespace std;


//____ constructor ____________________________________________________________

EditorWindow::EditorWindow(API * pAPI, MyApp* pApp, std::string title, std::string path ) : Window(pAPI,{ .open = true, .resizable = true, .size = {800,600}, .title = title })
{
	m_pAPI = pAPI;
	m_pAPI = pAPI;
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

	auto pTheme = m_pApp->m_pTheme;

	auto pMainContainer = PackPanel::create();
	pMainContainer->setAxis(Axis::Y);

	// Create and setup the top-bar

	auto pTopBar = _createTopBar();


	// Create our TextEditor widget

	auto pTextEditor = TextEditor::create( WGOVR( pTheme->textEditor(), _.editor.layout = m_pApp->m_pEditorLayout, _.editor.style = m_pApp->m_pEditorStyle, _.stickyFocus = true ));

	Base::msgRouter()->addRoute(pTextEditor, MsgType::TextEdit, [this](Msg * pMsg) { this->_contentModified(pMsg); } );
	

	// Create and setup a scrollpanel to wrap the text editor.

	auto pScrollPanel = ScrollPanel::create(pTheme->scrollPanelXY());

	//	pScrollPanel->setAutohideScrollbars(true, true);
	//	pScrollPanel->setSizeConstraints(SizeConstraint::GreaterOrEqual, SizeConstraint::GreaterOrEqual);

	pScrollPanel->setTransition(CoordTransition::create(300000));


	pScrollPanel->slot = pTextEditor;


	// 

	pMainContainer->slots << pTopBar;
	pMainContainer->slots << pScrollPanel;
	pMainContainer->setSlotWeight(0, 2, { 0.f, 1.f });


	pMainContainer->setLayout(PackLayout::create( {
		.expandFactor = PackLayout::Factor::Weight,
		.shrinkFactor = PackLayout::Factor::Weight
	} ));


	mainCapsule()->slot = pMainContainer;
	m_pEditor = pTextEditor;
	return true;
}



//____ _createTopBar() ________________________________________________________

Widget_p EditorWindow::_createTopBar()
{
	auto pTheme = m_pApp->m_pTheme;

	auto pBar = PackPanel::create();
	pBar->setAxis(Axis::X);
	pBar->setLayout(m_pApp->m_pLayout);
	pBar->setSkin( pTheme->plateSkin() );

	auto pClearButton = Button::create(WGOVR(pTheme->pushButton(), _.label.text = "Clear"));
	auto pNewButton = Button::create(WGOVR(pTheme->pushButton(), _.label.text = "New"));
	auto pLoadButton = Button::create(WGOVR(pTheme->pushButton(), _.label.text = "Load"));
	auto pSaveButton = Button::create(WGOVR(pTheme->pushButton(), _.label.text = "Save"));
	auto pSaveAsButton = Button::create(WGOVR(pTheme->pushButton(), _.label.text = "Save as..."));

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
	Base::msgRouter()->addRoute(pSaveButton, MsgType::Select, [this](Msg* pMsg) {this->_saveFileCallback(); });
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
	auto path = m_pAPI->openFileDialog("Load File", "", "", {"*.txt"}, "Text Files");

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
	auto selectedFile = m_pAPI->saveFileDialog("Save File", "", "", { "*.*" }, "Text Files");

	if (selectedFile.empty())
		return false;

	return _saveFile(selectedFile);
}

//____ _loadFile() ____________________________________________________

bool EditorWindow::_loadFile( const std::string& path )
{
	auto pBlob = m_pAPI->loadBlob(path,true);
	if( !pBlob )
		return false;
	
	m_pEditor->editor.setText((char*) pBlob->data());
	_setPathAndTitle(path);

	return true;
}

//____ _saveFileCallback() ___________________________________________________

bool EditorWindow::_saveFileCallback()
{
	if( m_path.empty() )
	{
		return _selectAndSaveFile();
	}
	else
		return _saveFile(m_path);
}

//____ _contentModified() ____________________________________________________

void EditorWindow::_contentModified(Msg* pMsg)
{
	bool bDisableSave = m_pEditor->editor.length() == 0;
	
	m_pSaveButton->setDisabled(bDisableSave);
	m_pSaveAsButton->setDisabled(bDisableSave);
}

//____ saveFile() ___________________________________________________

bool EditorWindow::_saveFile(const std::string& path)
{
	String decoratedText = m_pEditor->editor.text();

	CharSeq	converter(decoratedText);

	auto basket = converter.getUtf8();

	FILE* fp = fopen(path.c_str(), "w");
	if (!fp)
		return false;

	int elementsWritten = fwrite(basket.ptr, basket.length, 1, fp);
	fclose(fp);

	if (elementsWritten != 1)
		m_pAPI->messageBox("Not saved", "We failed to save the file. Maybe the destination is write protected or out of space?", wapp::DialogType::Ok, wapp::IconType::Error, wapp::DialogButton::Ok);
	else
		_setPathAndTitle(path);

	return (elementsWritten == 1);
}

//____ _setPathAndTitle() ___________________________________________________

void EditorWindow::_setPathAndTitle(const std::string& path)
{
	m_path = path;

	auto lastSlash = path.find_last_of("/\\");
	if (lastSlash != std::string::npos)
		m_title = path.substr(lastSlash + 1);
	else
		m_title = path;

	setTitle(m_title);
}
