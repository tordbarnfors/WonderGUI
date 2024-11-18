#include "testsuite.h"

class SegmentTests : public TestSuite
{
public:
	SegmentTests()
	{
		name = "SegmentTests";

		addTest("RainbowSpread7", &SegmentTests::rainbowSpread7);
		addTest("RainbowSpread8", &SegmentTests::rainbowSpread8 );
		addTest("Flip90Rainbow", &SegmentTests::flip90Rainbow);
		addTest("Flip180Rainbow", &SegmentTests::flip180Rainbow);
		addTest("Flip270Rainbow", &SegmentTests::flip270Rainbow);
		addTest("WildRainbow", &SegmentTests::wildRainbow);
		addTest("Circles", &SegmentTests::circles);
		addTest("Elipses", &SegmentTests::elipses);
		addTest("LargeCircle", &SegmentTests::largeCircle);
		addTest("BrokenElipse", &SegmentTests::brokenElipse);

		addTest("SegBlendReplace", &SegmentTests::setBlendModeReplace, &SegmentTests::rainbowSpread8, &SegmentTests::resetBlendMode);
		addTest("SegBlendAdd", &SegmentTests::setBlendModeAdd, &SegmentTests::rainbowSpread8, &SegmentTests::resetBlendMode);
		addTest("SegBlendSubtract", &SegmentTests::setBlendModeSubtract, &SegmentTests::rainbowSpread8, &SegmentTests::resetBlendMode);
		addTest("SegBlendMultiply", &SegmentTests::setBlendModeMultiply, &SegmentTests::rainbowSpread8, &SegmentTests::resetBlendMode);
		addTest("SegBlendInvert", &SegmentTests::setBlendModeInvert, &SegmentTests::rainbowSpread8, &SegmentTests::resetBlendMode);

		addTest("SegPrecision", &SegmentTests::precisionTest);

	}


	bool init(GfxDevice * pDevice, const RectSPX& canvas, WonderApp::Visitor * pAppVisitor)
	{
		// Generate rainbow segments

		for (int i = 0; i <= canvas.w/64; i++)
		{
			m_rainbowEdges[i][0] = i * 32/4;
			m_rainbowEdges[i][1] = i * 64/4;
			m_rainbowEdges[i][2] = i * 96/4;
			m_rainbowEdges[i][3] = i * 128/4;
			m_rainbowEdges[i][4] = i * 160/4;
			m_rainbowEdges[i][5] = i * 192/4;
			m_rainbowEdges[i][6] = i * 224/4;
		}

		return true;
	}


	bool	resetBlendMode(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Blend);
	}

	bool	setBlendModeReplace(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Replace);
	}

	bool	setBlendModeAdd(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Add);
	}

	bool	setBlendModeSubtract(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Subtract);
	}

	bool	setBlendModeMultiply(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Multiply);
	}

	bool	setBlendModeInvert(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return pDevice->setBlendMode(BlendMode::Invert);
	}

	bool rainbowSpread7(GfxDevice * pDevice, const RectSPX& canvas)
	{

		pDevice->drawSegments(canvas, 7, m_rainbowColors, m_nRainbowEdges, &m_rainbowEdges[0][0], 7);

		return true;
	}

	bool rainbowSpread8(GfxDevice * pDevice, const RectSPX& canvas)
	{

		pDevice->drawSegments(canvas, 8, m_rainbowColors, m_nRainbowEdges, &m_rainbowEdges[0][0], 7);

		return true;
	}


	bool wildRainbow(GfxDevice * pDevice, const RectSPX& canvas)
	{
		HiColor colors[8] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue, Color::Indigo, Color::Violet, Color::Black };

		int edges[512 + 1][7];
		for (int i = 0; i <= canvas.w/64; i++)
		{

			if (i % 2 == 0)
			{
				for (int j = 0; j < 7; j++)
					edges[i][j] = 256 + 256 * j;
			}
			else
			{
				for (int j = 0; j < 7; j++)
					edges[i][j] = (canvas.h*64) + 256 * j;
			}
		}

		pDevice->drawSegments(canvas, 8, colors, m_nRainbowEdges, &edges[0][0], 7);

		return true;
	}

	bool largeCircle(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->drawElipse(canvas, 20*64, Color::Aquamarine, 2*64, Color::White);
		return true;
	}

	bool circles(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int y = 0; y < 10; y++)
		{
			for (int x = 0; x < 12; x++)
			{
				RectF geo = { x * 32.1f,y * 26 + y * y * 1.5f + x * 0.11f,y * 15.01f + x * 2.05f,y * 15.01f + x * 2.05f };

				RectSPX geospx = RectSPX(geo * 64);

				pDevice->drawElipse(geospx, 4*64, Color::Aquamarine, 64, Color::White);
			}
		}

		return true;
	}

	bool elipses(GfxDevice * pDevice, const RectSPX& canvas)
	{
		RectF geo1 = { 0.f,0.f,canvas.w * 3 / 4.f, canvas.w * 3 / 4 / 3.f };
		RectF geo2 = { canvas.w * 3 / 4.f,0.f,canvas.w / 4.f, canvas.h * 1.f };


		pDevice->drawElipse( RectSPX(geo1), 20*64, Color::Aquamarine, 5*64, Color::White);
		pDevice->drawElipse( RectSPX(geo2), 20*64, Color::Aquamarine, 5*64, Color::White);

		return true;
	}


	bool brokenElipse(GfxDevice * pDevice, const RectSPX& canvas)
	{
		int x = 0;
		int y = 1;

		RectF geo = { x * 30.1f,y * 20 + y * y * 1.5f + x * 0.11f,y * 15.01f + x * 2.05f,y * 10.01f + x * 2.05f };

		pDevice->drawElipse( RectSPX(geo*64), 4*64, Color::Aquamarine, 1*64, Color::White);

		return true;
	}


	bool rainbow(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->drawSegments(canvas, 8, m_rainbowColors, m_nRainbowEdges, (int*)m_rainbowEdges, 7);
		return true;
	}


	bool flip90Rainbow(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->flipDrawSegments(canvas, 8, m_rainbowColors, m_nRainbowEdges, (int*)m_rainbowEdges, 7, GfxFlip::Rot90);
		return true;
	}

	bool flip180Rainbow(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->flipDrawSegments(canvas, 8, m_rainbowColors, m_nRainbowEdges, (int*)m_rainbowEdges, 7, GfxFlip::Rot180);
		return true;
	}

	bool flip270Rainbow(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->flipDrawSegments(canvas, 8, m_rainbowColors, m_nRainbowEdges, (int*)m_rainbowEdges, 7, GfxFlip::Rot270);
		return true;
	}

	bool precisionTest(GfxDevice * pDevice, const RectSPX& canvas)
	{
//		pDevice->plotPixels(4, m_precisionPlotCoords, m_precisionPlotColors);

//		pDevice->drawSegments({ 2,1,1,4 }, 5, m_precisionColors, 2, &m_precisionEdges[0][0], 4);
//		pDevice->drawSegments({ canvas.w-1,1,1,4 }, 5, m_precisionColors, 2, &m_precisionEdges[0][0], 4);

		pDevice->drawSegments({ 0 * 64,50 * 64,1 * 64,340 * 64 }, 5, m_precisionColors, 2, &m_precisionEdges2[0][0], 4);
		pDevice->drawSegments({ canvas.w-2 * 64,50 * 64,1 * 64,340 * 64 }, 5, m_precisionColors, 2, &m_precisionEdges2[0][0], 4);

		return true;
	}


private:

	HiColor	m_rainbowColors[8] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue, Color::Indigo, Color::Violet, Color( 255,255,255,128 ) };

	int		m_nRainbowEdges = 512 + 1;
	int		m_rainbowEdges[512 + 1][7];

	int		m_precisionEdges[2][4] = { {0 * 64+32, 6 * 64, 6 * 64, 6 * 64}, {3 * 64+32, 6 * 64, 6 * 64, 6 * 64} };
	HiColor	m_precisionColors[5] = { Color(0,0,0,0), Color(255,255,255,255), Color(255,0,0,255), Color(255,0,0,255), Color(255,0,0,255) };

	int		m_precisionEdges2[2][4] = { {200 * 64 + 240/4, 219 * 64 + 89/4, 260 * 64, 260 * 64}, {206 * 64+170/4, 240 * 64, 260 * 64, 260 * 64} };


};

