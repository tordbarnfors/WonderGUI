#include <string.h>

#include "textsystem.h"

#include <wondergui.h>
#include <wax_freetypefont.h>
#include <wax_device_gen2.h>
#include <wax_softbackend.h>
#include <wax_softkernels_default.h>
#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>

#include <vector>
#include <algorithm>

using namespace wg;
using namespace wax;


//____ TestText _______________________________________________________________
//
// Minimal TextItem so tests can drive a TextLayout directly.

class TestText : public TextItem
{
public:
	TestText( TextLayout * pLayout, TextStyle * pStyle, std::string_view text, SizeSPX size = { 10000*64, 1000*64 } )
		: m_pLayout(pLayout), m_pStyle(pStyle), m_text(text), m_size(size)
	{
		m_pLayout->addText(this);
	}

	~TestText()
	{
		m_pLayout->removeText(this);
	}

	void setText( std::string_view text )
	{
		int removed = int(m_text.size());
		m_text = text;
		m_pLayout->onTextModified(this, 0, removed, int(m_text.size()));
	}

	void setSize( SizeSPX size )
	{
		SizeSPX oldSize = m_size;
		m_size = size;
		m_pLayout->onResized(this, size, oldSize, 64, 64);
	}

	int	nbLines() const
	{
		int lines = 0;
		while( m_pLayout->lineBegin(this, lines) >= 0 )
			lines++;
		return lines;
	}

	TextLayout *	m_pLayout;

private:
	SizeSPX		_textSize() const override { return m_size; }
	State		_textState() const override { return State::Default; }
	TextStyle * _textStyle() const override { return m_pStyle; }

	const std::string& _text() const override { return m_text; }

	bool		_caretVisible() const override { return false; }
	int			_caretOffset() const override { return 0; }
	std::tuple<int, int>	_selectedText() const override { return std::make_tuple(0,0); }

	void		_mapperRequestRender() override {}
	void		_mapperRequestRender(const RectSPX& rect) override {}
	void		_mapperRequestResize() override {}

	int			_scale() const override { return 64; }

	TextStyle *	m_pStyle;
	std::string	m_text;
	SizeSPX		m_size;
};



TextSystemTest::TextSystemTest()
{
	ADD_TEST(utf8Test);
	ADD_TEST(caretNavigationTest);
	ADD_TEST(lineBreakTest);
	ADD_TEST(wrapTest);
	ADD_TEST(hitTestTest);
	ADD_TEST(wordNavigationTest);
	ADD_TEST(renderTest);
	ADD_TEST(componentEditTest);
	ADD_TEST(numberLayoutTest);
	ADD_TEST(markdownMarkersTest);
	ADD_TEST(markdownBlocksTest);
	ADD_TEST(markdownLinksTest);
	ADD_TEST(markdownSourceModeTest);
	ADD_TEST(markdownRenderTest);
	ADD_TEST(sizeScaleTest);
	ADD_TEST(decorationTest);
}

TextSystemTest::~TextSystemTest()
{
}

bool TextSystemTest::init(std::ostream& output)
{
	return true;
}

//____ _createStyle() _________________________________________________________

TextStyle_p TextSystemTest::_createStyle( int size )
{
	auto pBlob = loadBlob("resources/NotoSans-Regular.ttf");
	if( !pBlob )
		return nullptr;

	auto pFont = FreeTypeFont::create(pBlob);
	return WGCREATE(TextStyle, _.color = HiColor::White, _.font = pFont, _.size = size);
}

//____ utf8Test() _____________________________________________________________

bool TextSystemTest::utf8Test(std::ostream& output)
{
	std::string s = "a\xC3\x85\xE2\x82\xAC\xF0\x9F\x98\x80";		// "aÅ€😀", 1, 2, 3 and 4 bytes.

	TEST_ASSERT(s.size() == 10);
	TEST_ASSERT(Utf8::isValid(s));
	TEST_ASSERT(Utf8::countCodepoints(s) == 4);

	int ofs = 0;
	TEST_ASSERT(Utf8::decode(s, ofs) == 'a' && ofs == 1);
	TEST_ASSERT(Utf8::decode(s, ofs) == 0xC5 && ofs == 3);
	TEST_ASSERT(Utf8::decode(s, ofs) == 0x20AC && ofs == 6);
	TEST_ASSERT(Utf8::decode(s, ofs) == 0x1F600 && ofs == 10);

	TEST_ASSERT(Utf8::next(s, 1) == 3 && Utf8::next(s, 6) == 10 && Utf8::next(s, 10) == 10);
	TEST_ASSERT(Utf8::prev(s, 10) == 6 && Utf8::prev(s, 3) == 1 && Utf8::prev(s, 0) == 0);
	TEST_ASSERT(Utf8::alignToCodepoint(s, 2) == 1 && Utf8::alignToCodepoint(s, 9) == 6);
	TEST_ASSERT(Utf8::offsetOfCodepoint(s, 3) == 6);

	TEST_ASSERT(Utf8::fromUtf32(Utf8::toUtf32(s)) == s);
	TEST_ASSERT(Utf8::fromUtf16(Utf8::toUtf16(s)) == s);
	TEST_ASSERT(Utf8::toUtf16(s).size() == 5);					// Emoji needs a surrogate pair.

	// Broken, overlong, surrogate and out of range sequences.

	std::string bad = "a\xC3" "b\xE2\x82" "\xC0\xAF" "\xED\xA0\x80" "\xF4\x90\x80\x80" "c";
	TEST_ASSERT(!Utf8::isValid(bad));

	std::string clean = Utf8::sanitize(std::string_view(bad));
	TEST_ASSERT(Utf8::isValid(clean));

	// Stepping back and forth through invalid text must agree.

	int nbCodepoints = 0;
	for( int i = 0 ; i < int(bad.size()) ; i = Utf8::next(bad, i) )
	{
		TEST_ASSERT(Utf8::prev(bad, Utf8::next(bad, i)) == i);
		nbCodepoints++;
	}
	TEST_ASSERT(nbCodepoints == Utf8::countCodepoints(bad));

	return true;
}

//____ caretNavigationTest() __________________________________________________

bool TextSystemTest::caretNavigationTest(std::ostream& output)
{
	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	auto pLayout = PlainTextLayout::create();

	// "Hå 😀\nrad två"
	//  H(0) å(1-2) space(3) 😀(4-7) \n(8) r(9) a(10) d(11) space(12) t(13) v(14) å(15-16) length 17

	TestText text( pLayout, pStyle, "H\xC3\xA5 \xF0\x9F\x98\x80\nrad tv\xC3\xA5" );

	std::vector<int> positions = { 0, 1, 3, 4, 8, 9, 10, 11, 12, 13, 14, 15, 17 };

	spx wanted = -1;
	int ofs = 0;
	for( size_t i = 1 ; i < positions.size() ; i++ )
	{
		ofs = pLayout->caretRight(&text, ofs, wanted);
		TEST_ASSERT(ofs == positions[i]);
	}
	TEST_ASSERT(pLayout->caretRight(&text, ofs, wanted) == 17);

	for( int i = int(positions.size()) - 2 ; i >= 0 ; i-- )
	{
		ofs = pLayout->caretLeft(&text, ofs, wanted);
		TEST_ASSERT(ofs == positions[i]);
	}
	TEST_ASSERT(pLayout->caretLeft(&text, 0, wanted) == 0);

	// Lines

	TEST_ASSERT(text.nbLines() == 2);
	TEST_ASSERT(pLayout->charLine(&text, 0) == 0);
	TEST_ASSERT(pLayout->charLine(&text, 8) == 0);
	TEST_ASSERT(pLayout->charLine(&text, 9) == 1);
	TEST_ASSERT(pLayout->charLine(&text, 17) == 1);			// Virtual terminator belongs to last line.
	TEST_ASSERT(pLayout->lineBegin(&text, 1) == 9);
	TEST_ASSERT(pLayout->lineEnd(&text, 0) == 9);
	TEST_ASSERT(pLayout->lineEnd(&text, 1) == 18);

	TEST_ASSERT(pLayout->caretHome(&text, 12, wanted) == 9);
	TEST_ASSERT(pLayout->caretEnd(&text, 2, wanted) == 8);
	TEST_ASSERT(pLayout->caretEnd(&text, 10, wanted) == 17);

	// Up and down keep column as good as possible.

	int down = pLayout->caretDown(&text, 1, wanted);
	TEST_ASSERT(pLayout->charLine(&text, down) == 1);
	int up = pLayout->caretUp(&text, down, wanted);
	TEST_ASSERT(up == 1);

	return true;
}

//____ lineBreakTest() ________________________________________________________

bool TextSystemTest::lineBreakTest(std::ostream& output)
{
	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	auto pLayout = PlainTextLayout::create();

	TestText text( pLayout, pStyle, "" );
	TEST_ASSERT(text.nbLines() == 1);
	TEST_ASSERT(pLayout->lineEnd(&text, 0) == 1);

	SizeSPX emptySize = pLayout->defaultSize(&text, 64);
	TEST_ASSERT(emptySize.h > 0);

	text.setText("a\n\nb");
	TEST_ASSERT(text.nbLines() == 3);
	TEST_ASSERT(pLayout->lineBegin(&text, 1) == 2 && pLayout->lineEnd(&text, 1) == 3);
	TEST_ASSERT(pLayout->lineBegin(&text, 2) == 3 && pLayout->lineEnd(&text, 2) == 5);

	SizeSPX threeLines = pLayout->defaultSize(&text, 64);
	TEST_ASSERT(threeLines.h > emptySize.h * 2);

	text.setText("a\n");
	TEST_ASSERT(text.nbLines() == 2);
	TEST_ASSERT(pLayout->lineBegin(&text, 1) == 2);

	text.setText("a");
	spx narrow = pLayout->defaultSize(&text, 64).w;
	text.setText("aaaa");
	spx wide = pLayout->defaultSize(&text, 64).w;
	TEST_ASSERT(wide > narrow);

	// Default size scales with scale.

	SizeSPX doubleSize = pLayout->defaultSize(&text, 128);
	TEST_ASSERT(doubleSize.w > wide);

	return true;
}

//____ wrapTest() _____________________________________________________________

bool TextSystemTest::wrapTest(std::ostream& output)
{
	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	auto pPlainLayout = PlainTextLayout::create();
	auto pWrapLayout = PlainTextLayout::create( WGBP(PlainTextLayout, _.wrap = true) );

	// Measure two first words without wrap.

	spx twoWordsWidth;
	{
		TestText measure( pPlainLayout, pStyle, "aaaa bbbb" );
		twoWordsWidth = pPlainLayout->defaultSize(&measure, 64).w;
	}

	TestText text( pWrapLayout, pStyle, "aaaa bbbb cccc" );

	SizeSPX unwrapped = pWrapLayout->defaultSize(&text, 64);
	TEST_ASSERT(unwrapped.w > twoWordsWidth);

	text.setSize( { twoWordsWidth, 1000*64 } );
	TEST_ASSERT(text.nbLines() == 2);
	TEST_ASSERT(pWrapLayout->lineBegin(&text, 1) == 10);

	spx oneLineHeight = pWrapLayout->matchingHeight(&text, unwrapped.w, 64);
	spx twoLinesHeight = pWrapLayout->matchingHeight(&text, twoWordsWidth, 64);
	TEST_ASSERT(twoLinesHeight > oneLineHeight);

	// Line narrower than any glyph must still make progress, one glyph per line.

	text.setSize( { 64, 1000*64 } );
	TEST_ASSERT(text.nbLines() == 12);

	// Multibyte characters are never split.

	text.setText("\xC3\xA5\xC3\xA5\xC3\xA5");
	TEST_ASSERT(text.nbLines() == 3);
	TEST_ASSERT(pWrapLayout->lineBegin(&text, 1) == 2);
	TEST_ASSERT(pWrapLayout->lineBegin(&text, 2) == 4);

	return true;
}

//____ hitTestTest() __________________________________________________________

bool TextSystemTest::hitTestTest(std::ostream& output)
{
	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	auto pLayout = PlainTextLayout::create();

	std::string str = "Hello w\xC3\xB6rld\nline two";
	TestText text( pLayout, pStyle, str );

	spx wanted = -1;
	int ofs = 0;
	int len = int(str.size());
	int nbTested = 0;

	while( ofs < len )
	{
		int next = pLayout->caretRight(&text, ofs, wanted);
		TEST_ASSERT(next > ofs);

		RectSPX rect = pLayout->charRect(&text, ofs);
		CoordSPX pos = pLayout->charPos(&text, ofs);

		TEST_ASSERT(pos.x == rect.x);

		if( rect.w > 0 )
		{
			CoordSPX center( rect.x + rect.w/2, rect.y + rect.h/2 );
			TEST_ASSERT(pLayout->charAtPos(&text, center) == ofs);

			CoordSPX leftEdge( rect.x + 1, rect.y + rect.h/2 );
			TEST_ASSERT(pLayout->caretToPos(&text, leftEdge, wanted) == ofs);
			nbTested++;
		}

		ofs = next;
	}

	TEST_ASSERT(nbTested >= 19);

	// Outside of text

	TEST_ASSERT(pLayout->charAtPos(&text, { -100, 10 }) == -1);
	TEST_ASSERT(pLayout->caretToPos(&text, { 100000*64, 0 }, wanted) == 12);	// End of first line, before \n.

	return true;
}

//____ wordNavigationTest() ___________________________________________________

bool TextSystemTest::wordNavigationTest(std::ostream& output)
{
	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	auto pLayout = PlainTextLayout::create();

	TestText text( pLayout, pStyle, "one  two\nthree" );

	TEST_ASSERT(pLayout->caretNextWord(&text, 0) == 5);
	TEST_ASSERT(pLayout->caretNextWord(&text, 5) == 8);
	TEST_ASSERT(pLayout->caretNextWord(&text, 8) == 9);
	TEST_ASSERT(pLayout->caretNextWord(&text, 9) == 14);

	TEST_ASSERT(pLayout->caretPrevWord(&text, 14) == 9);
	TEST_ASSERT(pLayout->caretPrevWord(&text, 9) == 8);
	TEST_ASSERT(pLayout->caretPrevWord(&text, 8) == 5);
	TEST_ASSERT(pLayout->caretPrevWord(&text, 5) == 0);

	TEST_ASSERT(pLayout->wordBegin(&text, 6) == 5);
	TEST_ASSERT(pLayout->wordEnd(&text, 5) == 8);

	text.setText("d\xC3\xA5 d\xC3\xA5");
	TEST_ASSERT(pLayout->caretNextWord(&text, 0) == 4);
	TEST_ASSERT(pLayout->wordEnd(&text, 0) == 3);

	return true;
}

//____ renderTest() ___________________________________________________________

bool TextSystemTest::renderTest(std::ostream& output)
{
	Base::setDefaultSurfaceFactory( SoftSurfaceFactory::create() );

	auto pStyle = _createStyle(20);
	TEST_ASSERT(pStyle != nullptr);

	SizeI canvasSize(160, 40);

	auto pBackend = SoftBackend::create();
	addDefaultSoftKernels(pBackend);
	auto pDevice = GfxDeviceGen2::create(pBackend);

	auto pCanvas = SoftSurface::create(WGBP(Surface, _.size = canvasSize, _.format = PixelFormat::ARGB_8, _.canvas = true));

	auto countLitPixels = [&]( TextLayout * pLayout, std::string_view str )
	{
		TestText text( pLayout, pStyle, str, canvasSize*64 );

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(pCanvas);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor::Black);
		pDevice->setBlendMode(BlendMode::Blend);

		pLayout->render(&text, pDevice, RectSPX(0, 0, canvasSize.w*64, canvasSize.h*64));

		pDevice->endCanvasUpdate();
		pDevice->endRender();

		auto buffer = pCanvas->allocPixelBuffer();
		pCanvas->pushPixels(buffer);

		int lit = 0;
		for( int y = 0 ; y < canvasSize.h ; y++ )
		{
			uint32_t * pLine = (uint32_t*) (buffer.pixels + y * buffer.pitch);
			for( int x = 0 ; x < canvasSize.w ; x++ )
				if( (pLine[x] & 0xFF) > 128 )
					lit++;
		}

		pCanvas->freePixelBuffer(buffer);
		return lit;
	};

	auto pLayout = PlainTextLayout::create();

	int empty = countLitPixels(pLayout, "");
	int hello = countLitPixels(pLayout, "Hello");
	int helloWorld = countLitPixels(pLayout, "Hello\nWorld");
	int unicode = countLitPixels(pLayout, "\xC3\x85\xC3\x84\xC3\x96 \xE2\x82\xAC \xF0\x9F\x98\x80");	// ÅÄÖ € and an emoji not in font.

	TEST_ASSERT(empty == 0);
	TEST_ASSERT(hello > 0);
	TEST_ASSERT(helloWorld > hello);
	TEST_ASSERT(unicode > 0);

	// Text not fitting gets an ellipsis and is clipped.

	int overflow = countLitPixels(pLayout, "This text is far too long to fit in the canvas");
	TEST_ASSERT(overflow > 0);

	return true;
}

//____ componentEditTest() ____________________________________________________

bool TextSystemTest::componentEditTest(std::ostream& output)
{
	auto pDisplay = TextDisplay::create();
	auto& text = pDisplay->display;

	text.setText("abc");
	TEST_ASSERT(text.text() == "abc");
	TEST_ASSERT(text.length() == 3);

	TEST_ASSERT(text.insert(2, "\xC3\xA5") == 2);					// "abåc"
	TEST_ASSERT(text.text() == "ab\xC3\xA5" "c");

	TEST_ASSERT(text.erase(3, 1) == 2);							// Offset within 'å' erases whole codepoint.
	TEST_ASSERT(text.text() == "abc");

	text.insert(10, "d");										// Offset beyond end appends.
	TEST_ASSERT(text.text() == "abcd");

	text.replace(1, 2, "\xE2\x82\xAC");							// "a€d"
	TEST_ASSERT(text.text() == "a\xE2\x82\xAC" "d");

	text.setText("a\xFF" "b");									// Invalid UTF-8 is replaced.
	TEST_ASSERT(text.text() == "a\xEF\xBF\xBD" "b");

	text.append("!");
	TEST_ASSERT(text.text() == "a\xEF\xBF\xBD" "b!");

	text.clear();
	TEST_ASSERT(text.isEmpty());

	// Selection in an editor never splits codepoints.

	auto pEditor = TextEditor::create();
	pEditor->editor.setText("x\xC3\xA5y");
	TEST_ASSERT(pEditor->editor.select(0, 2));
	TEST_ASSERT(pEditor->editor.getSelection() == "x");
	TEST_ASSERT(pEditor->editor.select(0, 3));
	TEST_ASSERT(pEditor->editor.getSelection() == "x\xC3\xA5");

	// Tooltips are plain strings.

	pDisplay->setTooltip("Tip \xE2\x82\xAC");
	TEST_ASSERT(pDisplay->tooltip() == "Tip \xE2\x82\xAC");

	return true;
}

//____ numberLayoutTest() _____________________________________________________

bool TextSystemTest::numberLayoutTest(std::ostream& output)
{
	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	TextStyle_p pOldDefault = Base::defaultStyle();
	Base::setDefaultStyle(pStyle);

	auto pLayout = BasicNumberLayout::create( WGBP(BasicNumberLayout,
		_.decimalMax = 2,
		_.decimalSign = 0xB7,
		_.prefix = "\xE2\x82\xAC",
		_.style = pStyle ) );

	SizeSPX small = pLayout->defaultSize(1.0, 64, State::Default);
	SizeSPX large = pLayout->defaultSize(-123456.75, 64, State::Default);

	TEST_ASSERT(small.w > 0 && small.h > 0);
	TEST_ASSERT(large.w > small.w);

	Base::setDefaultStyle(pOldDefault);
	return true;
}

//____ markdownMarkersTest() __________________________________________________

bool TextSystemTest::markdownMarkersTest(std::ostream& output)
{
	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	auto pPlain = PlainTextLayout::create();
	auto pMarkdown = MarkdownTextLayout::create();

	auto widthOf = [&]( TextLayout * pLayout, std::string_view str )
	{
		TestText text( pLayout, pStyle, str );
		return pLayout->defaultSize(&text, 64).w;
	};

	// Hidden markers take no space.

	TEST_ASSERT(widthOf(pMarkdown, "**ab**") == widthOf(pPlain, "ab"));
	TEST_ASSERT(widthOf(pMarkdown, "*a* _b_ `c` ~~d~~") == widthOf(pPlain, "a b c d"));
	TEST_ASSERT(widthOf(pMarkdown, "# Title") == widthOf(pPlain, "Title"));
	TEST_ASSERT(widthOf(pMarkdown, "\\*not emphasis\\*") == widthOf(pPlain, "*not emphasis*"));

	// Escaped text displays as it is.

	std::string tricky = "1. *Stars*, _under_ [brackets] & <tags> # hash";
	TEST_ASSERT(widthOf(pMarkdown, MarkdownTextLayout::escape(tricky)) == widthOf(pPlain, tricky));

	// Caret steps over hidden markers, one visible character per step.

	TestText text( pMarkdown, pStyle, "**ab**" );

	std::vector<int> positions = { 0, 3, 6 };
	spx wanted = -1;
	int ofs = 0;
	for( size_t i = 1 ; i < positions.size() ; i++ )
	{
		ofs = pMarkdown->caretRight(&text, ofs, wanted);
		TEST_ASSERT(ofs == positions[i]);
	}
	for( int i = int(positions.size()) - 2 ; i >= 0 ; i-- )
	{
		ofs = pMarkdown->caretLeft(&text, ofs, wanted);
		TEST_ASSERT(ofs == positions[i]);
	}

	TEST_ASSERT(pMarkdown->charRect(&text, 0).w == 0);
	TEST_ASSERT(pMarkdown->charRect(&text, 2).w > 0);

	// Entities are displayed as one character, caret steps over them.

	text.setText("a&amp;b");
	TEST_ASSERT(pMarkdown->caretRight(&text, 1, wanted) == 6);
	TEST_ASSERT(pMarkdown->caretLeft(&text, 6, wanted) == 1);
	TEST_ASSERT(widthOf(pMarkdown, "a&amp;b") == widthOf(pPlain, "a&b"));

	// Single newlines in a paragraph are spaces, blank line separates paragraphs.

	text.setText("a\nb");
	TEST_ASSERT(text.nbLines() == 1);
	TEST_ASSERT(widthOf(pMarkdown, "a\nb") == widthOf(pPlain, "a b"));

	text.setText("a\n\nb");
	TEST_ASSERT(text.nbLines() == 3);

	text.setText("a  \nb");								// Hard line break.
	TEST_ASSERT(text.nbLines() == 2);

	return true;
}

//____ markdownBlocksTest() ___________________________________________________

bool TextSystemTest::markdownBlocksTest(std::ostream& output)
{
	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	auto pMarkdown = MarkdownTextLayout::create( WGBP(MarkdownTextLayout, _.indentation = 16) );
	spx step = 16*64;

	// Lists get bullets, nested lists are indented.

	TestText text( pMarkdown, pStyle, "- a\n  - b\n- c" );
	//  '-'0 ' '1 'a'2 '\n'3 ' '4 ' '5 '-'6 ' '7 'b'8 '\n'9 '-'10 ' '11 'c'12

	TEST_ASSERT(text.nbLines() == 3);
	TEST_ASSERT(pMarkdown->charRect(&text, 0).w > 0);			// Bullet is displayed.
	TEST_ASSERT(pMarkdown->charPos(&text, 0).x == 0);
	TEST_ASSERT(pMarkdown->charPos(&text, 6).x == step);
	TEST_ASSERT(pMarkdown->charPos(&text, 10).x == 0);

	spx wanted = -1;
	TEST_ASSERT(pMarkdown->caretRight(&text, 3, wanted) == 6);	// Hidden indentation is skipped.

	// Ordered lists keep their numbers.

	text.setText("1. one\n2. two");
	TEST_ASSERT(text.nbLines() == 2);
	TEST_ASSERT(pMarkdown->charRect(&text, 0).w > 0);
	TEST_ASSERT(pMarkdown->charRect(&text, 1).w > 0);

	// Quotes are indented.

	text.setText("> q");
	TEST_ASSERT(pMarkdown->charPos(&text, 2).x == step);
	TEST_ASSERT(pMarkdown->charRect(&text, 0).w == 0);

	// Code blocks keep their indentation.

	text.setText("```\n  x\n```");
	TEST_ASSERT(pMarkdown->charRect(&text, 4).w > 0);
	TEST_ASSERT(pMarkdown->charRect(&text, 0).w == 0);

	// Wrapped lines of list items are indented to the text.

	auto pWrapping = MarkdownTextLayout::create( WGBP(MarkdownTextLayout, _.indentation = 16, _.wrap = true) );
	TestText wrapped( pWrapping, pStyle, "- aaaa bbbb cccc dddd" );
	wrapped.setSize( { pWrapping->defaultSize(&wrapped, 64).w / 2, 1000*64 } );

	TEST_ASSERT(wrapped.nbLines() >= 2);
	int line1 = pWrapping->lineBegin(&wrapped, 1);
	TEST_ASSERT(pWrapping->charPos(&wrapped, line1).x == step);

	return true;
}

//____ markdownLinksTest() ____________________________________________________

bool TextSystemTest::markdownLinksTest(std::ostream& output)
{
	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	auto pLinkStyle = WGCREATE(TextStyle, _.color = Color::Blue, _.decoration = TextDecoration::Underline);
	auto pMarkdown = MarkdownTextLayout::create( WGBP(MarkdownTextLayout, _.link = pLinkStyle) );

	TestText text( pMarkdown, pStyle, "See [docs](https://example.com \"Tip\") now" );
	//  'S'0 ... '['4 'd'5 'o'6 'c'7 's'8 ']'9 ...

	TextLink_p pLink = pMarkdown->linkAt(&text, 6);
	TEST_ASSERT(pLink != nullptr);
	TEST_ASSERT(pLink->link() == "https://example.com");
	TEST_ASSERT(pLink->tooltip() == "Tip");

	TEST_ASSERT(pMarkdown->linkAt(&text, 1) == nullptr);
	TEST_ASSERT(pMarkdown->linkAt(&text, 5) == pMarkdown->linkAt(&text, 8));

	text.setText("<https://auto.link>");
	pLink = pMarkdown->linkAt(&text, 5);
	TEST_ASSERT(pLink != nullptr && pLink->link() == "https://auto.link");

	return true;
}

//____ markdownSourceModeTest() _______________________________________________

bool TextSystemTest::markdownSourceModeTest(std::ostream& output)
{
	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	auto pMarkupStyle = WGCREATE(TextStyle, _.color = Color::Gray);
	auto pPlain = PlainTextLayout::create();
	auto pSource = MarkdownTextLayout::create( WGBP(MarkdownTextLayout, _.markup = pMarkupStyle, _.showMarkup = true) );

	std::string md = "# Head\n\n- **bold** &amp; *em*\n> quote\nsoft\nbreak";

	TestText plain( pPlain, pStyle, md );
	TestText source( pSource, pStyle, md );

	// Everything is displayed exactly as plain text, just styled.

	TEST_ASSERT(pSource->defaultSize(&source, 64) == pPlain->defaultSize(&plain, 64));
	TEST_ASSERT(source.nbLines() == plain.nbLines());

	spx wanted = -1;
	for( int ofs = 0 ; ofs < int(md.size()) ; ofs++ )
	{
		TEST_ASSERT(pSource->caretRight(&source, ofs, wanted) == pPlain->caretRight(&plain, ofs, wanted));
		TEST_ASSERT(pSource->charRect(&source, ofs) == pPlain->charRect(&plain, ofs));
	}

	return true;
}

//____ markdownRenderTest() ___________________________________________________

bool TextSystemTest::markdownRenderTest(std::ostream& output)
{
	Base::setDefaultSurfaceFactory( SoftSurfaceFactory::create() );

	auto pStyle = _createStyle();
	TEST_ASSERT(pStyle != nullptr);

	auto pBigStyle = WGCREATE(TextStyle, _.sizeScale = 1.5f);
	auto pCodeStyle = WGCREATE(TextStyle, _.backColor = HiColor(1024,1024,1024,4096));
	auto pLinkStyle = WGCREATE(TextStyle, _.color = Color::Blue, _.decoration = TextDecoration::Underline);

	auto pMarkdown = MarkdownTextLayout::create( WGBP(MarkdownTextLayout,
		_.code = pCodeStyle,
		_.codeBlock = pCodeStyle,
		_.heading[0] = pBigStyle,
		_.link = pLinkStyle,
		_.wrap = true ) );

	SizeI canvasSize(200, 300);

	auto pBackend = SoftBackend::create();
	addDefaultSoftKernels(pBackend);
	auto pDevice = GfxDeviceGen2::create(pBackend);
	auto pCanvas = SoftSurface::create(WGBP(Surface, _.size = canvasSize, _.format = PixelFormat::ARGB_8, _.canvas = true));

	std::string md = "# Heading\n\nSome *emphasis*, **strong** and `code` with a [link](http://x).\n\n"
					 "- one\n- two\n  1. nested\n\n> quoted\n> text\n\n```\ncode block\n  indented\n```\n"
					 "&copy; &#x1F600; &unknown; \xC3\xA5\xC3\xA4\xC3\xB6";

	TestText text( pMarkdown, pStyle, md, canvasSize*64 );

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(pCanvas);
	pDevice->setBlendMode(BlendMode::Replace);
	pDevice->fill(HiColor::Black);
	pDevice->setBlendMode(BlendMode::Blend);

	pMarkdown->render(&text, pDevice, RectSPX(0, 0, canvasSize.w*64, canvasSize.h*64));

	pDevice->endCanvasUpdate();
	pDevice->endRender();

	auto buffer = pCanvas->allocPixelBuffer();
	pCanvas->pushPixels(buffer);

	int lit = 0;
	for( int y = 0 ; y < canvasSize.h ; y++ )
	{
		uint32_t * pLine = (uint32_t*) (buffer.pixels + y * buffer.pitch);
		for( int x = 0 ; x < canvasSize.w ; x++ )
			if( (pLine[x] & 0xFFFFFF) != 0 )
				lit++;
	}
	pCanvas->freePixelBuffer(buffer);

	TEST_ASSERT(lit > 0);

	// Walking the whole text with caret and hit tests must stay within bounds.

	spx wanted = -1;
	int ofs = 0;
	int steps = 0;
	while( ofs < int(md.size()) && steps < 1000 )
	{
		RectSPX r = pMarkdown->charRect(&text, ofs);
		pMarkdown->charAtPos(&text, { r.x + r.w/2, r.y + r.h/2 });
		pMarkdown->caretDown(&text, ofs, wanted);
		wanted = -1;

		int next = pMarkdown->caretRight(&text, ofs, wanted);
		TEST_ASSERT(next > ofs);
		ofs = next;
		steps++;
	}
	TEST_ASSERT(ofs == int(md.size()));

	return true;
}

//____ sizeScaleTest() ________________________________________________________

bool TextSystemTest::sizeScaleTest(std::ostream& output)
{
	auto pStyle = _createStyle(20);
	TEST_ASSERT(pStyle != nullptr);

	auto pDouble = WGCREATE(TextStyle, _.sizeScale = 2.f);

	TextAttr attr;
	pStyle->exportAttr(State::Default, &attr, 64);
	spx baseSize = attr.size;

	pDouble->addToAttr(State::Default, &attr, 64);
	TEST_ASSERT(attr.size == baseSize * 2);

	// Absolute size wins over scale.

	auto pAbsolute = WGCREATE(TextStyle, _.size = 10, _.sizeScale = 2.f);
	pAbsolute->addToAttr(State::Default, &attr, 64);
	TEST_ASSERT(attr.size == 10*64);

	TEST_ASSERT(pDouble->blueprint().sizeScale == 2.f);

	// Headings with sizeScale are taller.

	auto pMarkdown = MarkdownTextLayout::create( WGBP(MarkdownTextLayout, _.heading[0] = pDouble) );
	TestText heading( pMarkdown, pStyle, "# Big" );
	TestText normal( pMarkdown, pStyle, "Big" );

	TEST_ASSERT(pMarkdown->defaultSize(&heading, 64).h > pMarkdown->defaultSize(&normal, 64).h * 3 / 2);

	return true;
}

//____ decorationTest() _______________________________________________________

bool TextSystemTest::decorationTest(std::ostream& output)
{
	Base::setDefaultSurfaceFactory( SoftSurfaceFactory::create() );

	auto pStyle = _createStyle(20);
	TEST_ASSERT(pStyle != nullptr);

	SizeI canvasSize(160, 40);

	auto pBackend = SoftBackend::create();
	addDefaultSoftKernels(pBackend);
	auto pDevice = GfxDeviceGen2::create(pBackend);
	auto pCanvas = SoftSurface::create(WGBP(Surface, _.size = canvasSize, _.format = PixelFormat::ARGB_8, _.canvas = true));

	auto pLayout = PlainTextLayout::create();
	int baseline = 0;

	// Renders text and returns number of lit pixels per row.

	auto renderRows = [&]( TextDecoration decoration )
	{
		auto pDecorated = WGCREATE(TextStyle, _.color = HiColor::White, _.decoration = decoration, _.font = pStyle->font(), _.size = 20);

		TestText text( pLayout, pDecorated, "xxxxxxxx", canvasSize*64 );
		baseline = pLayout->charPos(&text, 0).y / 64;

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(pCanvas);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor::Black);
		pDevice->setBlendMode(BlendMode::Blend);
		pLayout->render(&text, pDevice, RectSPX(0, 0, canvasSize.w*64, canvasSize.h*64));
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		auto buffer = pCanvas->allocPixelBuffer();
		pCanvas->pushPixels(buffer);

		std::vector<int> rows(canvasSize.h, 0);
		for( int y = 0 ; y < canvasSize.h ; y++ )
		{
			uint32_t * pLine = (uint32_t*) (buffer.pixels + y * buffer.pitch);
			for( int x = 0 ; x < canvasSize.w ; x++ )
				if( (pLine[x] & 0xFF) > 128 )
					rows[y]++;
		}

		pCanvas->freePixelBuffer(buffer);
		return rows;
	};

	auto none = renderRows(TextDecoration::None);
	auto underline = renderRows(TextDecoration::Underline);
	auto strikethrough = renderRows(TextDecoration::Strikethrough);

	// The decoration adds a solid line, wider than any row of plain glyphs,
	// below baseline for underline and above it for strikethrough.

	int maxPlain = *std::max_element(none.begin(), none.end());

	int underlineRow = -1, strikeRow = -1;
	for( int y = 0 ; y < canvasSize.h ; y++ )
	{
		if( underline[y] > maxPlain && underline[y] > none[y] )
			underlineRow = y;
		if( strikethrough[y] > maxPlain && strikethrough[y] > none[y] )
			strikeRow = y;
	}

	TEST_ASSERT(underlineRow > baseline);
	TEST_ASSERT(strikeRow >= 0 && strikeRow < baseline);
	TEST_ASSERT(none[strikeRow] < strikethrough[strikeRow]);

	return true;
}
