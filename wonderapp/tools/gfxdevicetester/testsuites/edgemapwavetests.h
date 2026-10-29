#include "testsuite.h"
#include <wax_edgemapwave.h>

class EdgemapWaveTests : public TestSuite
{
public:
	EdgemapWaveTests()
	{
		name = "EdgemapWaveTests";

		addTest("BasicWave", &EdgemapWaveTests::basicWave );
//		addTest("CrossingWave", &EdgemapWaveTests::simpleWave );
	}


	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{

		m_pWave = EdgemapWave::create( WGBP(EdgemapWave,
											 _.size = canvas.size(),
											 _.color = Color::Blue,
											 _.bottomOutlineThickness = 10*64,
											 _.topOutlineThickness = 3*64,
											 _.outlineColor = Color::Red,
											 _.factory = pDevice->edgemapFactory()
											 ) );
		
		spx		topSamples[513];
		spx		bottomSamples[513];
		
		
		for (int i = 0; i < 513; i++)
		{
			topSamples[i] = (int)((100+sin(i / 10.0) * 80) * 64);
			bottomSamples[i] = (int)((300 + sin(i / 20.0) * 6) * 64);
		}

		m_pWave->setSamples( 0, 513, topSamples, bottomSamples );
		
//		m_pWave->setFlatTopLine(0, 513, 64*20);
//		m_pWave->setFlatBottomLine(0, 513, 64*256);

		return true;
	}



	bool basicWave(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pWave->refresh();
		
		m_pWave->draw(pDevice, canvas.pos());
		return true;
	}

private:

	EdgemapWave_p	m_pWave;

};
 
