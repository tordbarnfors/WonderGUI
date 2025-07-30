#include "testsuite.h"

class WaveTests : public TestSuite
{
public:
	WaveTests()
	{
		name = "WaveTests";

		addTest("SimpleWave", &WaveTests::simpleWave );
		addTest("SimpleWaveRot90", &WaveTests::simpleWaveRot90);
		addTest("SimpleWaveRot180", &WaveTests::simpleWaveRot180);
		addTest("SimpleWaveRot270", &WaveTests::simpleWaveRot270);
		addTest("SimpleWaveFlipX", &WaveTests::simpleWaveFlipX);
		addTest("SimpleWaveFlipY", &WaveTests::simpleWaveFlipY);
		addTest("BackfaceWave", &WaveTests::backfaceWave);
		addTest("BackfaceWaveSameColor", &WaveTests::backfaceWaveSameColor);
	}


	bool init(GfxDevice * pDevice, const RectI& canvas, WonderApp::Visitor * pAppVisitor)
	{
		// Generate simple wave

		m_simpleWaveTopLine.color = Color::White;
		m_simpleWaveTopLine.hold = canvas.h / 2 * 64;
		m_simpleWaveTopLine.length = 513;
		m_simpleWaveTopLine.pWave = m_simpleWaveTopSamples;
		m_simpleWaveTopLine.thickness = 64;

		m_simpleWaveBottomLine.color = Color::HotPink;
		m_simpleWaveBottomLine.hold = canvas.h / 2;
		m_simpleWaveBottomLine.length = 30;
		m_simpleWaveBottomLine.pWave = m_simpleWaveBottomSamples;
		m_simpleWaveBottomLine.thickness = 5*64;

		for (int i = 0; i < 512+1; i++)
		{
			m_simpleWaveTopSamples[i] = (int)((100+sin(i / 10.0) * 80) * 64);
			m_simpleWaveBottomSamples[i] = (int)((300 + sin(i / 20.0) * 6) * 64);
		}

		m_simpleWaveBottomSamples[30] = 0;
		m_simpleWaveBottomSamples[31] = 0;

		// Generate backface wave

		m_backfaceWaveTopLine.color = Color::White;
		m_backfaceWaveTopLine.hold = canvas.h / 2 * 64;
		m_backfaceWaveTopLine.length = 513;
		m_backfaceWaveTopLine.pWave = m_backfaceWaveTopSamples;
		m_backfaceWaveTopLine.thickness = 64;

		m_backfaceWaveBottomLine.color = Color::HotPink;
		m_backfaceWaveBottomLine.hold = canvas.h / 2;
		m_backfaceWaveBottomLine.length = 513;
		m_backfaceWaveBottomLine.pWave = m_backfaceWaveBottomSamples;
		m_backfaceWaveBottomLine.thickness = 5*64;

		for (int i = 0; i < 512 + 1; i++)
		{
			m_backfaceWaveTopSamples[i] = (int)((100 + sin(i / 10.0) * 80) * 64);
			m_backfaceWaveBottomSamples[i] = (int)((100 + sin(i / 20.0) * 6) * 64);
		}


		return true;
	}



	bool simpleWave(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawWave(canvas, &m_simpleWaveTopLine, &m_simpleWaveBottomLine, Color::Green);
		return true;
	}

	bool simpleWaveRot90(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipDrawWave(canvas, &m_simpleWaveTopLine, &m_simpleWaveBottomLine, Color::Green, GfxFlip::Rot90);
		return true;
	}

	bool simpleWaveRot180(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipDrawWave(canvas, &m_simpleWaveTopLine, &m_simpleWaveBottomLine, Color::Green, GfxFlip::Rot180);
		return true;
	}

	bool simpleWaveRot270(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipDrawWave(canvas, &m_simpleWaveTopLine, &m_simpleWaveBottomLine, Color::Green, GfxFlip::Rot270);
		return true;
	}

	bool simpleWaveFlipX(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipDrawWave(canvas, &m_simpleWaveTopLine, &m_simpleWaveBottomLine, Color::Green, GfxFlip::FlipX);
		return true;
	}

	bool simpleWaveFlipY(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipDrawWave(canvas, &m_simpleWaveTopLine, &m_simpleWaveBottomLine, Color::Green, GfxFlip::FlipY);
		return true;
	}

	bool backfaceWave(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawWave(canvas, &m_backfaceWaveTopLine, &m_backfaceWaveBottomLine, Color::Green );
		return true;
	}

	bool backfaceWaveSameColor(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawWave(canvas, &m_backfaceWaveTopLine, &m_backfaceWaveBottomLine, Color::Green);
		return true;
	}


private:

	WaveLine	m_simpleWaveTopLine;
	WaveLine	m_simpleWaveBottomLine;

	int			m_simpleWaveTopSamples[512 + 1];
	int			m_simpleWaveBottomSamples[512 + 1];

	WaveLine	m_backfaceWaveTopLine;
	WaveLine	m_backfaceWaveBottomLine;

	int			m_backfaceWaveTopSamples[512 + 1];
	int			m_backfaceWaveBottomSamples[512 + 1];



	HiColor	m_rainbowColors[8] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue, Color::Indigo, Color::Violet, Color( 255,255,255,128 ) };
};
 
