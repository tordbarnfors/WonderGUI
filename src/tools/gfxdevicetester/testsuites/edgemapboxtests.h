#include "testsuite.h"

#include <wg_edgemapbox.h>

class EdgemapBoxTests : public TestSuite
{
public:
	EdgemapBoxTests()
	{
		name = "EdgemapBoxTests";

		addTest("RoundedBox", &EdgemapBoxTests::roundedBox );
		addTest("BoxBetweenPixels", &EdgemapBoxTests::boxBetweenPixels );
		addTest("Pill", &EdgemapBoxTests::pill );
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		auto create = [pDevice](const RoundedBoxSpec& spec)
		{
			return EdgemapBox::create(WGBP(EdgemapBox, _.factory = pDevice->edgemapFactory()), spec);
		};

		RoundedBoxSpec spec;

		spec.rect = { canvas.x + 20 * 64, canvas.y + 20 * 64, canvas.w - 40 * 64, canvas.h - 40 * 64 };
		spec.radius = { 40 * 64, 40 * 64, 40 * 64, 40 * 64 };
		spec.outline = { 3 * 64, 3 * 64, 3 * 64, 3 * 64 };
		spec.fillColor = Color::Blue;
		spec.outlineColor = Color::White;
		m_pRoundedBox = create(spec);

		spec.rect = { canvas.x + 20 * 64 + 19, canvas.y + 40 * 64 + 38, canvas.w - 60 * 64 - 13, canvas.h - 100 * 64 - 22 };
		spec.radius = { 16 * 64, 60 * 64, 0, 30 * 64 };
		spec.outline = { 96, 3 * 64, 32, 6 * 64 };
		spec.fillColor = Color::Orange;
		spec.outlineColor = Color::Indigo;
		m_pBetweenPixels = create(spec);

		spec.rect = { canvas.x + 20 * 64, canvas.y + canvas.h / 2 - 30 * 64, canvas.w - 40 * 64, 60 * 64 };
		spec.radius = { 30 * 64, 30 * 64, 30 * 64, 30 * 64 };
		spec.outline = { 2 * 64, 2 * 64, 2 * 64, 2 * 64 };
		spec.fillColor = Color::Green;
		spec.outlineColor = Color::White;
		m_pPill = create(spec);

		return m_pRoundedBox && m_pBetweenPixels && m_pPill;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pRoundedBox = nullptr;
		m_pBetweenPixels = nullptr;
		m_pPill = nullptr;
		return true;
	}

	bool roundedBox(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pRoundedBox->draw(pDevice);
		return true;
	}

	bool boxBetweenPixels(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pBetweenPixels->draw(pDevice);
		return true;
	}

	bool pill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pPill->draw(pDevice);
		return true;
	}

private:

	EdgemapBox_p	m_pRoundedBox;
	EdgemapBox_p	m_pBetweenPixels;
	EdgemapBox_p	m_pPill;
};
