#include "testsuite.h"

class PieChartTests : public TestSuite
{
public:
	PieChartTests()
	{
		name = "PieChartTests";

		addTest("SquarePieChart", &PieChartTests::squarePieChart);
		addTest("RolledPieChart", &PieChartTests::rolledPieChart);
		addTest("100PctPieChart", &PieChartTests::_100PctPieChart);
		addTest("100PctRolledPieChart", &PieChartTests::_100PctRolledPieChart);
		addTest("90PctRolledPieChart", &PieChartTests::_90PctRolledPieChart);
		addTest("OverflowPieChart", &PieChartTests::overflowPieChart);

		addTest("SquarePieChartWithHole", &PieChartTests::squarePieChartWithHole);
		addTest("RolledPieChartWithHole", &PieChartTests::rolledPieChartWithHole);
		addTest("RolledPieChartWit99PcthHole", &PieChartTests::rolledPieChartWith99PctHole);
		addTest("SquarePieChartWith100PctHole", &PieChartTests::squarePieChartWith100PctHole);

	}


	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{
		return true;
	}


	bool squarePieChart(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawPieChart(canvas, 0.f, 7, m_sliceSizes1, m_rainbowColors, 1.f, Color::Transparent, Color::Transparent, true);
		return true;
	}

	bool rolledPieChart(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawPieChart(canvas, 0.03f, 7, m_sliceSizes1, m_rainbowColors);
		return true;
	}

	bool _100PctPieChart(GfxDevice * pDevice, const RectI& canvas)
	{
		float	sliceSize = 1.f;

		pDevice->drawPieChart(canvas, 0.f, 1, &sliceSize, m_rainbowColors);
		return true;
	}

	bool _100PctRolledPieChart(GfxDevice * pDevice, const RectI& canvas)
	{
		float	sliceSize = 1.f;

		pDevice->drawPieChart(canvas, 0.1f, 1, &sliceSize, m_rainbowColors);
		return true;
	}

	bool _90PctRolledPieChart(GfxDevice * pDevice, const RectI& canvas)
	{
		float	sliceSize = 0.9f;

		pDevice->drawPieChart(canvas, 0.05f, 1, &sliceSize, m_rainbowColors);
		return true;
	}

	bool overflowPieChart(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawPieChart(canvas, 0.125f, 7, m_sliceSizesOverflow, m_rainbowColors);
		return true;
	}

	bool squarePieChartWithHole(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawPieChart(canvas, 0.f, 7, m_sliceSizes1, m_rainbowColors, 0.6f, Color::Transparent, Color::Transparent, true);
		return true;
	}

	bool rolledPieChartWithHole(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawPieChart(canvas, 0.03f, 7, m_sliceSizes1, m_rainbowColors, 0.6f);
		return true;
	}

	bool rolledPieChartWith99PctHole(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawPieChart(canvas, 0.03f, 7, m_sliceSizes1, m_rainbowColors, 0.99999f);
		return true;
	}

	bool squarePieChartWith100PctHole(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawPieChart(canvas, 0.f, 7, m_sliceSizes1, m_rainbowColors, 1.0f, Color::Transparent, Color::Transparent, true);
		return true;
	}


private:

	float	m_sliceSizes1[7] = { 0.1f, 0.1f, 0.1f, 0.2f, 0.15f, 0.125f, 0.2f };
	float	m_sliceSizesOverflow[7] = { 0.30f, 0.20f, 0.20f, 0.20f, 0.09f, 0.125f, 0.2f };
	HiColor	m_rainbowColors[7] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue, Color::Indigo, Color::Violet };

};

