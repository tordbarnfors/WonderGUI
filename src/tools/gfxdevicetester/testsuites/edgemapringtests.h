#include "testsuite.h"

#include <wg_edgemapring.h>

class EdgemapRingTests : public TestSuite
{
public:
	EdgemapRingTests()
	{
		name = "EdgemapRingTests";

		addTest("OutlinedDonut", &EdgemapRingTests::outlinedDonut );
		addTest("PieChart", &EdgemapRingTests::pieChart );
		addTest("DonutChart", &EdgemapRingTests::donutChart );
		addTest("Gauge", &EdgemapRingTests::gauge );
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		auto create = [pDevice](RingSpec spec, const std::vector<RingSlice>& slices)
		{
			spec.nSlices = int(slices.size());
			spec.pSlices = slices.data();
			return EdgemapRing::create(WGBP(EdgemapRing, _.factory = pDevice->edgemapFactory()), spec);
		};

		std::vector<RingSlice> seven = { { 0.1f, Color::Red }, { 0.2f, Color::Orange }, { 0.05f, Color::Yellow }, { 0.15f, Color::Green },
										 { 0.25f, Color::Blue }, { 0.13f, Color::Indigo }, { 0.12f, Color::Violet } };

		RingSpec spec;

		spec.rect = canvas;
		spec.thickness = 20 * 64;
		spec.outline = 2 * 64;
		spec.outlineColor = Color::White;
		m_pOutlinedDonut = create(spec, { { 1.f, Color::Blue } });

		// Center between pixels.

		spec = {};
		spec.rect = { canvas.x + 20 * 64 + 19, canvas.y + 20 * 64 + 38, canvas.w - 41 * 64, canvas.w - 41 * 64 };
		spec.start = 0.1f;
		m_pPieChart = create(spec, seven);

		spec = {};
		spec.rect = { canvas.x + 20 * 64, canvas.y + 20 * 64, canvas.w - 40 * 64, canvas.w - 40 * 64 };
		spec.thickness = canvas.w / 6;
		spec.outline = 2 * 64;
		spec.outlineColor = Color::White;
		spec.hubColor = Color::DarkGray;
		m_pDonutChart = create(spec, seven);

		spec = {};
		spec.rect = { canvas.x + 40 * 64, canvas.y + 40 * 64, canvas.w - 80 * 64, canvas.w - 80 * 64 };
		spec.thickness = 24 * 64;
		spec.start = 0.625f;
		spec.backColor = HiColor(4096, 4096, 4096, 1024);
		m_pGauge = create(spec, { { 0.45f, Color::Cyan }, { 0.3f, HiColor::Undefined } });

		return m_pOutlinedDonut && m_pPieChart && m_pDonutChart && m_pGauge;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pOutlinedDonut = nullptr;
		m_pPieChart = nullptr;
		m_pDonutChart = nullptr;
		m_pGauge = nullptr;
		return true;
	}

	bool outlinedDonut(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pOutlinedDonut->draw(pDevice);
		return true;
	}

	bool pieChart(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pPieChart->draw(pDevice);
		return true;
	}

	bool donutChart(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pDonutChart->draw(pDevice);
		return true;
	}

	bool gauge(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pGauge->draw(pDevice);
		return true;
	}

private:

	EdgemapRing_p	m_pOutlinedDonut;
	EdgemapRing_p	m_pPieChart;
	EdgemapRing_p	m_pDonutChart;
	EdgemapRing_p	m_pGauge;
};
