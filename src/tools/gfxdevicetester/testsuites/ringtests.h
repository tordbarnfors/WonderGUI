#include "testsuite.h"

#include <wg_tint.h>

// Circles, rings, pies and gauges drawn with GfxDevice::drawRing().

class RingTests : public TestSuite
{
public:
	RingTests()
	{
		name = "RingTests";

		addTest("Pies", &RingTests::pies);
		addTest("Donuts", &RingTests::donuts);
		addTest("Gauges", &RingTests::gauges);
		addTest("Axes", &RingTests::axes);
		addTest("ManySlices", &RingTests::manySlices);
		addTest("Square", &RingTests::square);
		addTest("Tinted", &RingTests::tinted);
		addTest("SmallAndSubpixel", &RingTests::smallAndSubpixel);
		addTest("BlendModes", &RingTests::blendModes);
	}

	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{
		m_pRadialTint = Tint::create(WGBP(Tint, _.shape = TintShape::Radial, _.stops = { {0.f, HiColor(4096,4096,4096,4096)}, {1.f, HiColor(1500,1500,1500,4096)} }));
		m_pLinearTint = Tint::create(HiColor(4096, 4096, 4096, 4096), HiColor(1000, 1000, 2500, 4096));

		for (int i = 0; i < 7; i++)
			m_rainbow[i] = { m_sizes[i], m_colors[i] };

		return true;
	}

	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pRadialTint = nullptr;
		m_pLinearTint = nullptr;
		return true;
	}

	RingSpec ring(spx x, spx y, spx diameter, spx thickness, float start, int nSlices, const RingSlice* pSlices)
	{
		RingSpec spec;
		spec.rect = { x, y, diameter, diameter };
		spec.thickness = thickness;
		spec.start = start;
		spec.nSlices = nSlices;
		spec.pSlices = pSlices;
		return spec;
	}

	bool pies(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawRing(ring(64 * 10 + 13, 64 * 10 + 41, 64 * 230, 0, 0.f, 7, m_rainbow));
		pDevice->drawRing(ring(64 * 260 + 5, 64 * 10 + 11, 64 * 230, 0, 0.13f, 7, m_rainbow));

		RingSpec spec = ring(64 * 10 + 13, 64 * 260 + 41, 64 * 230, 0, 0.7f, 3, m_rainbow);
		spec.outline = 64 * 3;
		spec.outlineColor = HiColor::Black;
		spec.backColor = HiColor(1000, 1000, 1000, 4096);
		pDevice->drawRing(spec);

		RingSlice single = { 1.f, HiColor(4096, 2000, 0, 4096) };
		spec = ring(64 * 260 + 5, 64 * 260 + 11, 64 * 230, 0, 0.f, 1, &single);
		spec.outline = 64 * 6;
		spec.outlineColor = HiColor(1000, 500, 0, 4096);
		pDevice->drawRing(spec);
		return true;
	}

	bool donuts(GfxDevice * pDevice, const RectI& canvas)
	{
		RingSpec spec = ring(64 * 10 + 13, 64 * 10 + 41, 64 * 230, 64 * 50, 0.f, 7, m_rainbow);
		pDevice->drawRing(spec);

		spec = ring(64 * 260 + 5, 64 * 10 + 11, 64 * 230, 64 * 60, 0.05f, 7, m_rainbow);
		spec.outline = 64 * 2;
		spec.outlineColor = HiColor::Black;
		spec.hubColor = HiColor(4096, 4096, 4096, 4096);
		pDevice->drawRing(spec);

		RingSlice single = { 1.f, HiColor(0, 2000, 4096, 4096) };
		spec = ring(64 * 10 + 13, 64 * 260 + 41, 64 * 230, 64 * 8, 0.f, 1, &single);
		pDevice->drawRing(spec);

		spec = ring(64 * 260 + 5, 64 * 260 + 11, 64 * 230, 64 * 30, 0.f, 1, &single);
		spec.outline = 64 * 4;
		spec.outlineColor = HiColor(0, 0, 1500, 4096);
		pDevice->drawRing(spec);
		return true;
	}

	// Progress rings and gauges, slices not reaching a full turn.

	bool gauges(GfxDevice * pDevice, const RectI& canvas)
	{
		for (int i = 0; i < 4; i++)
		{
			RingSlice slices[2] = { { 0.1f + i * 0.15f, HiColor(0, 3500, 1000, 4096) }, { 0.05f, HiColor(4096, 4096, 0, 4096) } };

			RingSpec spec = ring(64 * 10 + i * 64 * 125 + i * 9, 64 * 20, 64 * 115, 64 * 15, 0.625f, 2, slices);
			spec.backColor = HiColor(1500, 1500, 1500, 4096);
			spec.hubColor = HiColor(3800, 3800, 3800, 4096);
			pDevice->drawRing(spec);
		}

		for (int i = 0; i < 4; i++)
		{
			RingSlice slices[1] = { { 0.2f + i * 0.2f, HiColor(4096, 1000, 0, 4096) } };

			RingSpec spec = ring(64 * 10 + i * 64 * 125 + i * 17, 64 * 200 + i * 13, 64 * 115, 64 * 10, 0.f, 1, slices);
			spec.backColor = HiColor(4096, 4096, 4096, 1500);
			pDevice->drawRing(spec);
		}
		return true;
	}

	// Rings centered on a pixel center, so that pixel centers lie exactly on the axes through
	// it, in each direction within a slice of its own color.

	bool axes(GfxDevice * pDevice, const RectI& canvas)
	{
		RingSlice slices[4] = { { 0.25f, HiColor(4096, 0, 0, 4096) }, { 0.25f, HiColor(0, 3500, 0, 4096) },
								{ 0.25f, HiColor(0, 0, 4096, 4096) }, { 0.25f, HiColor(4096, 4096, 0, 4096) } };

		RingSpec pie = ring(64 * 10, 64 * 10, 64 * 115, 0, 0.875f, 4, slices);
		pDevice->drawRing(pie);

		RingSpec donut = ring(64 * 135, 64 * 10, 64 * 115, 64 * 30, 0.875f, 4, slices);
		donut.outline = 64 * 2;
		donut.hubColor = HiColor(3800, 3800, 3800, 4096);
		pDevice->drawRing(donut);

		RingSpec gauge = ring(64 * 260, 64 * 10, 64 * 115, 64 * 20, 0.875f, 3, slices);
		gauge.backColor = HiColor(1500, 1500, 1500, 4096);
		pDevice->drawRing(gauge);

		pDevice->drawRing(ring(64 * 385, 64 * 10, 64 * 15, 0, 0.875f, 4, slices));
		pDevice->drawRing(ring(64 * 385, 64 * 40, 64 * 31, 64 * 8, 0.875f, 4, slices));
		return true;
	}

	// Many slices meeting at the center.

	bool manySlices(GfxDevice * pDevice, const RectI& canvas)
	{
		RingSlice slices[256];

		for (int i = 0; i < 256; i++)
		{
			slices[i].size = 1.f / 256;
			slices[i].color = HiColor((i * 16) % 4097, 4096 - (i * 16) % 4097, (i * 64) % 4097, 4096);
		}

		pDevice->drawRing(ring(64 * 10 + 13, 64 * 10 + 41, 64 * 300, 0, 0.f, 256, slices));
		pDevice->drawRing(ring(64 * 320 + 13, 64 * 10 + 41, 64 * 180, 0, 0.f, 64, slices + 64));
		pDevice->drawRing(ring(64 * 320 + 13, 64 * 200 + 41, 64 * 180, 64 * 40, 0.f, 32, slices + 128));
		return true;
	}

	// Square outer edge, as for rectangular meters.

	bool square(GfxDevice * pDevice, const RectI& canvas)
	{
		RingSpec spec = ring(64 * 10 + 13, 64 * 10 + 41, 64 * 230, 0, 0.f, 7, m_rainbow);
		spec.bSquare = true;
		pDevice->drawRing(spec);

		spec = ring(64 * 260 + 5, 64 * 10 + 11, 64 * 230, 64 * 50, 0.1f, 7, m_rainbow);
		spec.bSquare = true;
		spec.hubColor = HiColor(4096, 4096, 4096, 4096);
		spec.outline = 64 * 2;
		spec.outlineColor = HiColor::Black;
		pDevice->drawRing(spec);

		spec = ring(64 * 10 + 13, 64 * 260 + 41, 64 * 480, 0, 0.25f, 3, m_rainbow);
		spec.rect.h = 64 * 200;
		spec.backColor = HiColor(1000, 1000, 1000, 4096);
		pDevice->drawRing(spec);
		return true;
	}

	bool tinted(GfxDevice * pDevice, const RectI& canvas)
	{
		RingSlice slices[7];
		for (int i = 0; i < 7; i++)
			slices[i] = { m_sizes[i], m_colors[i], i % 2 == 0 ? m_pRadialTint.rawPtr() : nullptr };

		RingSpec spec = ring(64 * 10 + 13, 64 * 10 + 41, 64 * 230, 0, 0.f, 7, slices);
		pDevice->drawRing(spec);

		spec = ring(64 * 260 + 5, 64 * 10 + 11, 64 * 230, 64 * 60, 0.f, 7, m_rainbow);
		spec.outline = 64 * 5;
		spec.outlineColor = HiColor::White;
		spec.pOutlineTint = m_pLinearTint;
		spec.hubColor = HiColor::White;
		spec.pHubTint = m_pRadialTint;
		pDevice->drawRing(spec);

		pDevice->setTint(canvas, m_pLinearTint);
		pDevice->drawRing(ring(64 * 10 + 13, 64 * 260 + 41, 64 * 230, 64 * 70, 0.3f, 7, m_rainbow));
		pDevice->clearTint();

		pDevice->setTintColor(HiColor(4096, 4096, 4096, 2048));
		pDevice->drawRing(ring(64 * 260 + 5, 64 * 260 + 11, 64 * 230, 0, 0.6f, 7, m_rainbow));
		pDevice->clearTintColor();
		return true;
	}

	// Small rings and pies moved in subpixel steps.

	bool smallAndSubpixel(GfxDevice * pDevice, const RectI& canvas)
	{
		for (int i = 0; i < 64; i++)
		{
			spx x = 64 * 10 + (i % 8) * 64 * 60 + i * 8;
			spx y = 64 * 10 + (i / 8) * 64 * 60 + i * 8;
			spx size = 64 * 4 + i * 64 * 3 / 4;

			RingSpec spec = ring(x, y, size, i % 2 ? size / 4 : 0, i * 0.02f, 7, m_rainbow);
			spec.outline = i % 3 == 0 ? 64 : 0;
			spec.outlineColor = HiColor::Black;
			pDevice->drawRing(spec);
		}
		return true;
	}

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

			RingSpec spec = ring(64 * 10 + (i % 5) * 64 * 100 + i * 7, 64 * 40 + (i / 5) * 64 * 200 + i * 11, 64 * 90, 64 * 25, 0.f, 7, m_rainbow);
			spec.outline = 64 * 3;
			spec.outlineColor = HiColor(500, 500, 4096, 4096);
			pDevice->drawRing(spec);
		}

		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

private:
	Tint_p		m_pRadialTint;
	Tint_p		m_pLinearTint;

	RingSlice	m_rainbow[7];

	const float	m_sizes[7] = { 0.1f, 0.2f, 0.05f, 0.15f, 0.25f, 0.13f, 0.12f };

	const HiColor m_colors[7] = {	HiColor(4096, 0, 0, 4096), HiColor(4096, 2000, 0, 4096), HiColor(4096, 4096, 0, 4096), HiColor(0, 4096, 0, 4096),
									HiColor(0, 1000, 4096, 4096), HiColor(1500, 0, 4096, 4096), HiColor(3000, 0, 3000, 4096) };
};
