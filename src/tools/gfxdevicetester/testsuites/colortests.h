#include "testsuite.h"

class ColorTests : public TestSuite
{
public:
	ColorTests()
	{
		name = "ColorTests";

		addTest("ColorPalette", &ColorTests::drawPalette );
	}




	bool drawPalette(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < 7; i++)
		{
			RectSPX c = { i * 64, 0, 64, canvas.h/64 };
			pDevice->fill(c*64, m_rainbowColors[i] );
		}

		return true;
	}


private:

	HiColor	m_rainbowColors[7] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue, Color::Indigo, Color::Violet };

};
