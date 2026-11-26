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

	HiColor	m_rainbowColors[7] = { Color8::Red, Color8::Orange, Color8::Yellow, Color8::Green, Color8::Blue, Color8::Indigo, Color8::Violet };

};
