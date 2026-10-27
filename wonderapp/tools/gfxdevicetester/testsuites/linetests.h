#include "testsuite.h"

class LineTests : public TestSuite
{
public:
	LineTests()
	{
		name = "LineTests";

		addTest("SolarisLines", &LineTests::solarisLines );

		// Lines tinted by Tints, which vary along and across the lines.

		addTest("LinearTintLines", &LineTests::setLinearTint, &LineTests::tintedSolarisLines, &LineTests::clearTint);
		addTest("HardEdgeTintLines", &LineTests::setHardEdgeTint, &LineTests::tintedSolarisLines, &LineTests::clearTint);
		addTest("RadialTintAndTintColorLines", &LineTests::setRadialTintAndTintColor, &LineTests::tintedSolarisLines, &LineTests::clearTint);
		addTest("MixTintLines", &LineTests::setMixTint, &LineTests::tintedSolarisLines, &LineTests::clearTint);
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

		HiColor red = Color::Red;
		HiColor green = Color::Green;
		HiColor blue = Color::Blue;
		HiColor white = Color::White;
		HiColor yellow = Color::Yellow;

		m_pLinearTint = Tint::create( { {0.f, red}, {0.3f, yellow}, {0.6f, green}, {1.f, blue} }, {0.1f, 0.f}, {0.9f, 1.f} );

		{
			Tint::Blueprint bp;
			bp.begin = { 0.f, 0.f };
			bp.end = { 0.1f, 0.05f };
			bp.spread = TintSpread::Reflect;
			bp.stops = { {0.f, red}, {0.5f, red}, {0.5f, white}, {1.f, blue} };
			m_pHardEdgeTint = Tint::create(bp);
		}

		{
			Tint::Blueprint bp;
			bp.shape = TintShape::Radial;
			bp.center = { 0.5f, 0.5f };
			bp.radius = { 0.5f, 0.3f };
			bp.stops = { {0.f, white}, {0.5f, yellow}, {1.f, HiColor(0, 0, 4096, 1024)} };
			m_pRadialTint = Tint::create(bp);
		}

		m_pMixTint = Tint::mix( Tint::create(red, blue, {0,0}, {1,0}), m_pRadialTint, 0.4f );

		return true;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pLinearTint = nullptr;
		m_pHardEdgeTint = nullptr;
		m_pRadialTint = nullptr;
		m_pMixTint = nullptr;
		return true;
	}

	//

	bool setLinearTint(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pLinearTint);
		return true;
	}

	bool setHardEdgeTint(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pHardEdgeTint);
		return true;
	}

	bool setRadialTintAndTintColor(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pRadialTint);
		pDevice->setTintColor(HiColor(4096, 2048, 4096, 3072));
		return true;
	}

	bool setMixTint(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pMixTint);
		return true;
	}

	bool clearTint(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTintColor(Color::White);
		pDevice->clearTint();
		return true;
	}

	//

	bool solarisLines(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < c_nbSolarisLines; i++)
		{
			pDevice->drawLine(m_begCoordsSolaris[i], m_endCoordsSolaris[i], Color::Yellow, 64);
		}

		return true;
	}

	// White lines of different thickness, so we see the Tint. The thickest ones are wide enough
	// for the Tint to vary across them.

	bool tintedSolarisLines(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < c_nbSolarisLines; i++)
		{
			spx thickness = 64 + (i % 4) * 3 * 64;

			pDevice->drawLine(m_begCoordsSolaris[i], m_endCoordsSolaris[i], Color::White, thickness);
		}

		return true;
	}


	const static int c_nbSolarisLines = 30;

	CoordSPX	m_begCoordsSolaris[c_nbSolarisLines];
	CoordSPX	m_endCoordsSolaris[c_nbSolarisLines];

	Tint_p		m_pLinearTint;
	Tint_p		m_pHardEdgeTint;
	Tint_p		m_pRadialTint;
	Tint_p		m_pMixTint;
};
