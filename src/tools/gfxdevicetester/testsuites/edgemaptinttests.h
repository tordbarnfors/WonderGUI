#include "testsuite.h"

// Tinted edgemaps in all twelve flips. Segment tints that vary in different directions,
// device tints on top of them and Replace mode, with a transparent first segment whose
// edge is blended with the segment below it.

class EdgemapTintTests : public TestSuite
{
public:
	EdgemapTintTests()
	{
		name = "EdgemapTintTests";

		addTest("SegmentsFlat", &EdgemapTintTests::segFlat);
		addTest("SegmentsVertical", &EdgemapTintTests::segVertical);
		addTest("SegmentsHorizontal", &EdgemapTintTests::segHorizontal);
		addTest("SegmentsRadial", &EdgemapTintTests::segRadial);
		addTest("SegmentsVerticalRepeat", &EdgemapTintTests::segVerticalRepeat);
		addTest("SegmentsMix", &EdgemapTintTests::segMix);
		addTest("SegmentsVerticalAndFlat", &EdgemapTintTests::segVerticalAndFlat);
		addTest("DeviceTintColor", &EdgemapTintTests::devTintColor);
		addTest("DeviceTintVertical", &EdgemapTintTests::devVertical);
		addTest("DeviceTintHorizontal", &EdgemapTintTests::devHorizontal);
		addTest("DeviceTintRadial", &EdgemapTintTests::devRadial);
		addTest("ReplaceVerticalAndFlat", &EdgemapTintTests::replaceVerticalAndFlat);
		addTest("ReplaceMix", &EdgemapTintTests::replaceMix);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		m_colA = Color8(255, 220, 120, 255);
		m_colB = Color8(60, 120, 255, 200);
		m_colC = Color8(255, 80, 80, 255);
		m_colD = Color8(80, 255, 120, 160);

		HiColor colors[c_nbSegments] = { HiColor::Transparent, m_colA, m_colB, m_colC };

		float samples[c_nbEdges * (c_width + 1)];
		for (int e = 0; e < c_nbEdges; e++)
			for (int x = 0; x <= c_width; x++)
				samples[e * (c_width + 1) + x] = 0.2f + 0.25f * e + 0.1f * sinf(x * 0.08f + e);

		auto pFactory = pDevice->edgemapFactory();

		for (int k = 0; k < c_nbKinds; k++)
		{
			Tint_p tints[c_nbSegments];
			bool bTints = false;

			for (int seg = 0; seg < c_nbSegments; seg++)
			{
				tints[seg] = _segmentTint(k, seg);
				bTints |= (tints[seg] != nullptr);
			}

			m_pEdgemaps[k] = pFactory->createEdgemap(WGBP(Edgemap, _.size = { c_width, c_height }, _.segments = c_nbSegments, _.colors = colors),
													 SampleOrigo::Top, samples, c_nbEdges);
			if (bTints)
				m_pEdgemaps[k]->setColors(0, c_nbSegments, tints);
		}

		m_pVertical = _linear({ 0, 0 }, { 0, 1 });
		m_pHorizontal = _linear({ 0, 0 }, { 1, 0 });
		m_pRadial = _radial();
		return true;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int k = 0; k < c_nbKinds; k++)
			m_pEdgemaps[k] = nullptr;
		m_pVertical = nullptr;
		m_pHorizontal = nullptr;
		m_pRadial = nullptr;
		return true;
	}

	bool segFlat(GfxDevice * pDevice, const RectSPX& canvas) { return _draw(pDevice, canvas, 0); }
	bool segVertical(GfxDevice * pDevice, const RectSPX& canvas) { return _draw(pDevice, canvas, 1); }
	bool segHorizontal(GfxDevice * pDevice, const RectSPX& canvas) { return _draw(pDevice, canvas, 2); }
	bool segRadial(GfxDevice * pDevice, const RectSPX& canvas) { return _draw(pDevice, canvas, 3); }
	bool segVerticalRepeat(GfxDevice * pDevice, const RectSPX& canvas) { return _draw(pDevice, canvas, 4); }
	bool segMix(GfxDevice * pDevice, const RectSPX& canvas) { return _draw(pDevice, canvas, 5); }
	bool segVerticalAndFlat(GfxDevice * pDevice, const RectSPX& canvas) { return _draw(pDevice, canvas, 6); }

	bool devTintColor(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTintColor(HiColor(Color8(255, 200, 150, 220)));
		_draw(pDevice, canvas, 1);
		pDevice->clearTintColor();
		return true;
	}

	bool devVertical(GfxDevice * pDevice, const RectSPX& canvas) { return _drawWithDeviceTint(pDevice, canvas, m_pVertical); }
	bool devHorizontal(GfxDevice * pDevice, const RectSPX& canvas) { return _drawWithDeviceTint(pDevice, canvas, m_pHorizontal); }
	bool devRadial(GfxDevice * pDevice, const RectSPX& canvas) { return _drawWithDeviceTint(pDevice, canvas, m_pRadial); }

	bool replaceVerticalAndFlat(GfxDevice * pDevice, const RectSPX& canvas) { return _draw(pDevice, canvas, 6, BlendMode::Replace); }
	bool replaceMix(GfxDevice * pDevice, const RectSPX& canvas) { return _draw(pDevice, canvas, 5, BlendMode::Replace); }

private:

	Tint_p _linear(CoordF begin, CoordF end, TintSpread spread = TintSpread::Pad)
	{
		Tint::Blueprint bp;
		bp.stops = { { 0.f, m_colA }, { 0.6f, m_colC }, { 1.f, m_colB } };
		bp.begin = begin;
		bp.end = end;
		bp.spread = spread;
		return Tint::create(bp);
	}

	Tint_p _radial()
	{
		Tint::Blueprint bp;
		bp.shape = TintShape::Radial;
		bp.stops = { { 0.f, m_colA }, { 1.f, m_colD } };
		return Tint::create(bp);
	}

	Tint_p _segmentTint(int kind, int seg)
	{
		switch (kind)
		{
			case 1: return _linear({ 0, 0 }, { 0, 1 });
			case 2: return _linear({ 0, 0 }, { 1, 0 });
			case 3: return _radial();
			case 4: return _linear({ 0, 0.1f }, { 0, 0.3f }, TintSpread::Repeat);
			case 5:
			{
				Tint_p a = _linear({ 0, 0 }, { 0, 1 });
				Tint_p b = _radial();
				Tint* components[2] = { a, b };
				float weights[2] = { 0.7f, 0.3f };
				return Tint::createMix(2, components, weights);
			}
			case 6: return seg % 2 ? _linear({ 0, 1 }, { 0, 0 }) : nullptr;
			default: return nullptr;
		}
	}

	bool _drawWithDeviceTint(GfxDevice * pDevice, const RectSPX& canvas, Tint * pTint)
	{
		pDevice->setTint({ 64 * 20, 64 * 30, 64 * 470, 64 * 460 }, pTint);
		_draw(pDevice, canvas, 1);
		pDevice->clearTint();
		return true;
	}

	bool _draw(GfxDevice * pDevice, const RectSPX& canvas, int kind, BlendMode blendMode = BlendMode::Blend)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(canvas, HiColor(Color8(30, 40, 50, 255)));
		pDevice->setBlendMode(blendMode);

		for (int flip = 0; flip < 12; flip++)
		{
			CoordSPX pos = { 64 * (8 + (flip % 4) * 126), 64 * (8 + (flip / 4) * 168) };
			pDevice->flipDrawEdgemap(pos, m_pEdgemaps[kind], GfxFlip(flip));
		}

		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

	static const int c_width = 110;
	static const int c_height = 80;
	static const int c_nbEdges = 3;
	static const int c_nbSegments = c_nbEdges + 1;
	static const int c_nbKinds = 7;

	HiColor		m_colA, m_colB, m_colC, m_colD;

	Edgemap_p	m_pEdgemaps[c_nbKinds];
	Tint_p		m_pVertical;
	Tint_p		m_pHorizontal;
	Tint_p		m_pRadial;
};
