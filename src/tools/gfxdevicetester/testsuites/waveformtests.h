#include "testsuite.h"
#include <wg_waveform.h>

class WaveformTests : public TestSuite
{
public:
	WaveformTests()
	{
		name = "WaveformTests";

		addTest("BasicWaveform", &WaveformTests::basicWaveform );
//		addTest("CrossingWaveform", &WaveformTests::simpleWave );
	}


	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{

		m_pWaveform = Waveform::create( WGBP(Waveform,
											 _.size = canvas.size()/64,
											 _.color = Color::Blue,
											 _.bottomOutlineThickness = 10*64,
											 _.topOutlineThickness = 3*64,
											 _.outlineColor = Color::Red
											 ), pDevice->edgemapFactory() );
		
		spx		topSamples[513];
		spx		bottomSamples[513];
		
		
		for (int i = 0; i < 513; i++)
		{
			topSamples[i] = (int)((100+sin(i / 10.0) * 80) * 64);
			bottomSamples[i] = (int)((300 + sin(i / 20.0) * 6) * 64);
		}

		m_pWaveform->setSamples( 0, 513, topSamples, bottomSamples );
		
//		m_pWaveform->setFlatTopLine(0, 513, 64*20);
//		m_pWaveform->setFlatBottomLine(0, 513, 64*256);

		return true;
	}



	bool basicWaveform(GfxDevice * pDevice, const RectI& canvas)
	{
		auto pEdgemap = m_pWaveform->refresh();
		
		pDevice->drawEdgemap(canvas, pEdgemap);
		return true;
	}

private:

	Waveform_p	m_pWaveform;

};
 
