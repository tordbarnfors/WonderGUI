
#pragma once

#include <wondergui.h>

#include <wonderapp.h>

using namespace wg;

class TextBuffer;
typedef StrongPtr<TextBuffer>	TextBuffer_p;
typedef WeakPtr<TextBuffer>		TextBuffer_wp;

class Editera;

class TextBuffer : public Object
{
public:

	//.____ Creation ___________________________________________________________
	
	static TextBuffer_p	create(Editera* pApp, wapp::API* pAPI, Theme* pTheme, TextLayout* pTextLayout, TextStyle* pTextStyle) 
	{ 
		return TextBuffer_p(new TextBuffer(pApp, pAPI, pTheme, pTextLayout, pTextStyle )); 
	}

	//.____ Identification _________________________________________________

	virtual const TypeInfo& typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;

	//.____ Behavior _________________________________________________________

	void			grabFocus() { m_pEditor->grabFocus(true); }

	//.____ Content _____________________________________________________

	bool			loadFromFile(const std::string& path);
	bool			saveToFile(const std::string& path);

	void			clear();
	bool			isEmpty();

	TextEditor_p	editor() { return m_pEditor; }

	const std::string& path() const { return m_path; }

	void			setContent(const char* pContent);

	enum LineEnding
	{
		CR,
		LF,
		CRLF,
		Mixed
	};


protected:
	TextBuffer(Editera* pApp, wapp::API* pAPI, Theme* pTheme, TextLayout* pTextLayout, TextStyle* pTextStyle);
	~TextBuffer();

	Editera*		m_pApp;
	wapp::API*		m_pAPI;

	TextEditor_p	m_pEditor;
	bool			m_bContentModified = false;


	std::string		m_path;

	LineEnding		m_lineEnding = LF;
	CodePage		m_codePage = CodePage::Undefined;
};