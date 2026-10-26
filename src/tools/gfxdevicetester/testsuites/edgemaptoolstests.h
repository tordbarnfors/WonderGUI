#include "testsuite.h"

#include <wg_edgemaptools.h>

class EdgemapToolsTests : public TestSuite
{
public:
	EdgemapToolsTests()
	{
		name = "EdgemapToolsTests";

		addTest("DrawOutlinedDonut", &EdgemapToolsTests::drawOutlinedDonut );
	}


	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{

		
		return true;
	}

	
	bool drawOutlinedDonut(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto pDonut = EdgemapTools::createDonut(canvas.size(), 20*64, Color::Blue, 2*64, Color::White, pDevice->edgemapFactory());
		pDevice->drawEdgemap(canvas, pDonut );
		return true;
	}
	

private:

	HiColor	m_rainbowColors[8] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue, Color::Indigo, Color::Violet, Color( 255,255,255,128 ) };
};
 
