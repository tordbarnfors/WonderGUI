#include "testsuite.h"

#include <wax_tint.h>

// Rounded boxes drawn with GfxDevice::drawRoundedBox().

class RoundedBoxTests : public TestSuite
{
public:
	RoundedBoxTests()
	{
		name = "RoundedBoxTests";

		addTest("Buttons", &RoundedBoxTests::buttons);
		addTest("Radii", &RoundedBoxTests::radii);
		addTest("Outlines", &RoundedBoxTests::outlines);
		addTest("SubpixelSteps", &RoundedBoxTests::subpixelSteps);
		addTest("SquareCorners", &RoundedBoxTests::squareCorners);
		addTest("Tinted", &RoundedBoxTests::tinted);
		addTest("BlendModes", &RoundedBoxTests::blendModes);
		addTest("Clipped", &RoundedBoxTests::clipped);
	}

	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{
		m_pVerticalTint = Tint::create(HiColor(4096, 4096, 4096, 4096), HiColor(1000, 1500, 4096, 4096));
		m_pDiagonalTint = Tint::create(HiColor(4096, 4096, 4096, 4096), HiColor(500, 500, 500, 4096), { 0.f, 0.f }, { 1.f, 1.f });
		m_pRadialTint = Tint::create(WGBP(Tint, _.shape = TintShape::Radial, _.stops = { {0.f, HiColor(4096,4096,0,4096)}, {1.f, HiColor(4096,0,0,4096)} }));
		return true;
	}

	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pVerticalTint = nullptr;
		m_pDiagonalTint = nullptr;
		m_pRadialTint = nullptr;
		return true;
	}

	RoundedBoxSpec box(spx x, spx y, spx w, spx h, spx radius, spx outline, HiColor fill, HiColor outlineColor)
	{
		RoundedBoxSpec spec;
		spec.rect = { x, y, w, h };
		spec.radius[0] = spec.radius[1] = spec.radius[2] = spec.radius[3] = radius;
		spec.outline = { outline, outline, outline, outline };
		spec.fillColor = fill;
		spec.outlineColor = outlineColor;
		return spec;
	}

	// Typical buttons at subpixel positions.

	bool buttons(GfxDevice * pDevice, const RectI& canvas)
	{
		for (int i = 0; i < 12; i++)
		{
			spx x = 64 * 20 + (i % 3) * 64 * 160 + i * 5;
			spx y = 64 * 20 + (i / 3) * 64 * 50 + i * 7;

			pDevice->drawRoundedBox(box(x, y, 64 * 120, 64 * 32, 64 * (2 + i), 64, HiColor(1200, 2400, 4096, 4096), HiColor::Black));
		}
		return true;
	}

	// Mixed radii, pills and circles.

	bool radii(GfxDevice * pDevice, const RectI& canvas)
	{
		RoundedBoxSpec spec = box(64 * 20 + 13, 64 * 20 + 41, 64 * 200, 64 * 140, 0, 64 * 2, HiColor(4096, 3000, 1000, 4096), HiColor(2000, 1000, 0, 4096));
		spec.radius[0] = 64 * 60;
		spec.radius[1] = 64 * 10;
		spec.radius[2] = 64 * 2 + 32;
		spec.radius[3] = 64 * 30;
		pDevice->drawRoundedBox(spec);

		pDevice->drawRoundedBox(box(64 * 250 + 20, 64 * 20 + 9, 64 * 240, 64 * 40 + 30, 64 * 1000, 64 * 3, HiColor(1000, 4096, 1000, 4096), HiColor(0, 1500, 0, 4096)));
		pDevice->drawRoundedBox(box(64 * 250 + 20, 64 * 80 + 9, 64 * 30 + 30, 64 * 120, 64 * 1000, 64 * 3, HiColor(1000, 4096, 1000, 4096), HiColor(0, 1500, 0, 4096)));
		pDevice->drawRoundedBox(box(64 * 300 + 40, 64 * 80 + 9, 64 * 150, 64 * 150, 64 * 1000, 64 * 6, HiColor(4096, 1000, 1000, 4096), HiColor(1500, 0, 0, 4096)));

		for (int i = 0; i < 8; i++)
			pDevice->drawRoundedBox(box(64 * 20 + i * 64 * 30 + i * 9, 64 * 260 + i * 5, 64 * 25, 64 * 25, i * 32, 0, HiColor(4096, 4096, 4096, 4096), HiColor::Black));

		return true;
	}

	// Per-side outlines, outlines thicker than the radius, subpixel outlines, outline only.

	bool outlines(GfxDevice * pDevice, const RectI& canvas)
	{
		RoundedBoxSpec spec = box(64 * 20 + 13, 64 * 20 + 41, 64 * 200, 64 * 100, 64 * 20, 0, HiColor(3500, 3500, 3500, 4096), HiColor(1000, 1000, 1000, 4096));
		spec.outline = { 64 * 2, 64 * 6, 64 * 8, 64 * 2 };
		pDevice->drawRoundedBox(spec);

		pDevice->drawRoundedBox(box(64 * 250 + 31, 64 * 20 + 5, 64 * 200, 64 * 100, 64 * 10, 64 * 24, HiColor(4096, 4096, 1000, 4096), HiColor(2000, 0, 2000, 4096)));
		pDevice->drawRoundedBox(box(64 * 20 + 51, 64 * 160 + 22, 64 * 200, 64 * 100, 64 * 16, 20, HiColor(4096, 4096, 4096, 4096), HiColor::Black));
		pDevice->drawRoundedBox(box(64 * 250 + 3, 64 * 160 + 37, 64 * 200, 64 * 100, 64 * 16, 64 * 3, HiColor::Transparent, HiColor(0, 0, 4096, 4096)));
		pDevice->drawRoundedBox(box(64 * 20, 64 * 300, 64 * 200, 64 * 100, 64 * 16, 64 * 4, HiColor(4096, 0, 0, 2048), HiColor(0, 0, 4096, 2048)));

		return true;
	}

	// Same box moved in 1/8 pixel steps.

	bool subpixelSteps(GfxDevice * pDevice, const RectI& canvas)
	{
		for (int i = 0; i < 64; i++)
		{
			spx x = 64 * 10 + (i % 8) * 64 * 60 + i * 8;
			spx y = 64 * 10 + (i / 8) * 64 * 60 + i * 8;

			pDevice->drawRoundedBox(box(x, y, 64 * 40 + i * 3, 64 * 40 + i * 5, 64 * 12, 64 + i, HiColor(4096, 4096, 4096, 4096), HiColor::Black));
		}
		return true;
	}

	// Square corners at subpixel positions, where pixels just outside a corner are partly
	// covered. Every other box has two rounded corners, one of them with a square inner corner
	// as its outline is thicker than its radius.

	bool squareCorners(GfxDevice * pDevice, const RectI& canvas)
	{
		for (int i = 0; i < 20; i++)
		{
			spx x = 64 * 10 + (i % 5) * 64 * 96 + i * 11;
			spx y = 64 * 10 + (i / 5) * 64 * 110 + i * 13;

			RoundedBoxSpec spec = box(x, y, 64 * 70 + i * 7, 64 * 80 + i * 5, 0, (i % 4) * 64 + i * 5, HiColor(1000, 2000, 4096, 4096), HiColor::Black);

			if (i % 2 == 1)
			{
				spec.radius[0] = 64 * 2;
				spec.radius[2] = 64 * 16;
			}

			pDevice->drawRoundedBox(spec);
		}
		return true;
	}

	// Tinted fills and outlines, with and without device tint.

	bool tinted(GfxDevice * pDevice, const RectI& canvas)
	{
		RoundedBoxSpec spec = box(64 * 20 + 13, 64 * 20 + 41, 64 * 200, 64 * 100, 64 * 24, 64 * 6, HiColor::White, HiColor::White);
		spec.pFillTint = m_pVerticalTint;
		spec.pOutlineTint = m_pDiagonalTint;
		pDevice->drawRoundedBox(spec);

		spec.rect.x += 64 * 240;
		spec.pFillTint = m_pRadialTint;
		pDevice->drawRoundedBox(spec);

		pDevice->setTint(canvas, m_pVerticalTint);

		spec.rect = { 64 * 20 + 13, 64 * 160 + 41, 64 * 200, 64 * 100 };
		spec.pFillTint = nullptr;
		spec.pOutlineTint = nullptr;
		spec.fillColor = HiColor(4096, 2000, 1000, 4096);
		spec.outlineColor = HiColor(1000, 1000, 1000, 4096);
		pDevice->drawRoundedBox(spec);

		spec.rect.x += 64 * 240;
		spec.pFillTint = m_pRadialTint;
		pDevice->drawRoundedBox(spec);

		pDevice->clearTint();
		pDevice->setTintColor(HiColor(4096, 4096, 4096, 2048));

		spec.rect = { 64 * 20 + 13, 64 * 300 + 41, 64 * 440, 64 * 100 };
		pDevice->drawRoundedBox(spec);

		pDevice->clearTintColor();
		return true;
	}

	// One box in each blend mode over stripes.

	bool blendModes(GfxDevice * pDevice, const RectI& canvas)
	{
		for (int i = 0; i < 16; i++)
			pDevice->fill(RectSPX(0, i * 64 * 32, canvas.w, 64 * 16), HiColor(i * 256, 4096 - i * 256, 2048, 4096));

		BlendMode modes[] = { BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::Subtract, BlendMode::Multiply,
							  BlendMode::Invert, BlendMode::Min, BlendMode::Max, BlendMode::Morph, BlendMode::BlendFixedColor };

		pDevice->setMorphFactor(0.5f);
		pDevice->setFixedBlendColor(HiColor(0, 4096, 4096, 4096));

		for (int i = 0; i < 10; i++)
		{
			pDevice->setBlendMode(modes[i]);

			spx x = 64 * 10 + (i % 5) * 64 * 100 + i * 7;
			spx y = 64 * 40 + (i / 5) * 64 * 200 + i * 11;

			pDevice->drawRoundedBox(box(x, y, 64 * 80, 64 * 160, 64 * 25, 64 * 5, HiColor(4096, 2000, 500, 3000), HiColor(500, 500, 4096, 4096)));
		}

		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

	// Clipped by a clip list.

	bool clipped(GfxDevice * pDevice, const RectI& canvas)
	{
		RectSPX clips[3] = { { 64 * 40, 64 * 40, 64 * 100, 64 * 100 }, { 64 * 200, 64 * 60, 64 * 200, 64 * 50 }, { 64 * 100, 64 * 250, 64 * 300, 64 * 200 } };

		pDevice->setClipList(3, clips);
		pDevice->drawRoundedBox(box(64 * 20 + 13, 64 * 20 + 41, 64 * 460, 64 * 440, 64 * 120, 64 * 8, HiColor(4096, 3000, 1000, 4096), HiColor(2000, 1000, 0, 4096)));
		pDevice->resetClipList();
		return true;
	}

private:
	Tint_p	m_pVerticalTint;
	Tint_p	m_pDiagonalTint;
	Tint_p	m_pRadialTint;
};
