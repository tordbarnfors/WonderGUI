#include "textbuffer.h"

#include "texteditor.h"


const TypeInfo TextBuffer::TYPEINFO = { "TextBuffer", &Object::TYPEINFO };

//____ constructor ____________________________________________________________

TextBuffer::TextBuffer(MyApp* pApp, wapp::API* pAPI, Theme * pTheme, TextLayout * pTextLayout, TextStyle * pTextStyle )
	: m_pApp(pApp)
	, m_pAPI(pAPI)
{

	// Create our TextEditor widget

	auto pTextEditor = TextEditor::create(WGOVR(pTheme->textEditor(), _.editor.layout = pApp->m_pEditorLayout, _.editor.style = pApp->m_pEditorStyle, _.stickyFocus = true));
	m_pEditor = pTextEditor;

	Base::msgRouter()->addRoute(pTextEditor, MsgType::TextEdit, [this](Msg* pMsg) { this->m_bContentModified = true; });

}

//____ destructor _____________________________________________________________

TextBuffer::~TextBuffer()
{

}

//____ typeInfo ______________________________________________________________

const TypeInfo& TextBuffer::typeInfo(void) const
{
	return TYPEINFO;

}

//____ setContent() _____________________________________________________

void TextBuffer::setContent(const char* pContent)
{
	m_pEditor->editor.setText(pContent);
}


//____ loadFromFile __________________________________________________________

bool TextBuffer::loadFromFile(const std::string& path)
{
	auto pBlob = m_pAPI->loadBlob(path, true);
	if (!pBlob)
		return false;

	m_pEditor->editor.setText((char*)pBlob->data());
	m_path = path;

	return true;
}

//____ saveToFile ____________________________________________________________

bool TextBuffer::saveToFile(const std::string& path)
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
		m_path = path;

	return (elementsWritten == 1);
}

//____ clear ________________________________________________________________

void TextBuffer::clear() 
{ 
	m_pEditor->editor.clear(); 
	m_path.clear(); 
	m_bContentModified = false; 
}

//____ isEmpty ______________________________________________________________

bool TextBuffer::isEmpty()
{
	return m_pEditor->editor.isEmpty();
}

