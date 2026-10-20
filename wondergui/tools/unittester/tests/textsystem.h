#include "../testframework.h"

#include <wondergui.h>


class TextSystemTest : public TestCollection
{
public:
	TextSystemTest();
	~TextSystemTest();

	std::string name() const override { return "TextSystem Test"; };

	bool init(std::ostream& output) override;

private:

	bool utf8Test(std::ostream& output);
	bool caretNavigationTest(std::ostream& output);
	bool lineBreakTest(std::ostream& output);
	bool wrapTest(std::ostream& output);
	bool hitTestTest(std::ostream& output);
	bool wordNavigationTest(std::ostream& output);
	bool renderTest(std::ostream& output);
	bool componentEditTest(std::ostream& output);
	bool numberLayoutTest(std::ostream& output);
	bool sizeScaleTest(std::ostream& output);

	wg::TextStyle_p		_createStyle( int size = 16 );		// Font and style are created per test, so nothing outlives Base::exit().

	wg::Blob_p			m_pFontBlob;
};
