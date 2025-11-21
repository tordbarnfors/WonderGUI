#include "testsuite.h"

class LineTests : public TestSuite
{
public:
	LineTests()
	{
		name = "LineTests";

		addTest("SolarisLines", &LineTests::solarisLines );
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		const double PI = 3.1415;

		CoordSPX center = { canvas.w / 2, canvas.h / 2 };

		for (int i = 0; i < c_nbSolarisLines; i++)
		{
			float xFactor = (float) sin(i * PI * 2 / c_nbSolarisLines  );
			float yFactor = (float) cos(i * PI * 2 / c_nbSolarisLines );

			CoordSPX beg = center + CoordSPX(int(50*64 * xFactor), int(50*64 * yFactor) );
			CoordSPX end = center + CoordSPX( int(250*64 * xFactor), int(250*64 * yFactor) );

			m_begCoordsSolaris[i] = beg;
			m_endCoordsSolaris[i] = end;
		}

		return true;
	}

	

	bool solarisLines(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < c_nbSolarisLines; i++)
		{
			pDevice->drawLine(m_begCoordsSolaris[i], m_endCoordsSolaris[i], Color::Yellow, 64);
		}

		return true;
	}


	const static int c_nbSolarisLines = 30;

	CoordSPX	m_begCoordsSolaris[c_nbSolarisLines];
	CoordSPX	m_endCoordsSolaris[c_nbSolarisLines];

};
