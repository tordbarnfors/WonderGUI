#include "testsuite.h"

// Tints beyond straight two-color gradients: radial shapes, spread modes,
// hard edges, sRGB interpolation, mixes and tints on top of tint colors.
// Covers fills, subpixel fills, blits and edgemap segment tints.

class TintTests : public TestSuite
{
public:
	TintTests()
	{
		name = "TintTests";

		addTest("MultiStopLinearFill", &TintTests::setMultiStop, &TintTests::fill, &TintTests::cleanup);
		addTest("HardEdgesReflectFill", &TintTests::setHardEdgesReflect, &TintTests::fill, &TintTests::cleanup);
		addTest("RadialFitFill", &TintTests::setRadialFit, &TintTests::fill, &TintTests::cleanup);
		addTest("RadialCircleRepeatFill", &TintTests::setRadialCircleRepeat, &TintTests::fill, &TintTests::cleanup);
		addTest("MixFill", &TintTests::setMix, &TintTests::fill, &TintTests::cleanup);
		addTest("TintAndTintColorFill", &TintTests::setRadialAndTintColor, &TintTests::fill, &TintTests::cleanup);
		addTest("RadialSubpixelFill", &TintTests::setRadialFit, &TintTests::subpixelFill, &TintTests::cleanup);
		addTest("MixSubpixelFill", &TintTests::setMix, &TintTests::subpixelFill, &TintTests::cleanup);
		addTest("RadialBlit", &TintTests::setRadialFitBlit, &TintTests::blit, &TintTests::cleanup);
		addTest("MixBlit", &TintTests::setMixBlit, &TintTests::blit, &TintTests::cleanup);
		addTest("MixRotScaleBlit", &TintTests::setMixBlit, &TintTests::rotScaleBlit, &TintTests::cleanup);
		addTest("SegmentRadialTints", &TintTests::segmentTints);
		addTest("SegmentMixTintsRot90", &TintTests::segmentTintsRot90);
		addTest("SegmentTintsWithDeviceTint", &TintTests::setMultiStop, &TintTests::segmentTints, &TintTests::cleanup);
		addTest("MixBlitWithTintColor", &TintTests::setMixBlitAndTintColor, &TintTests::blit, &TintTests::cleanup);
		addTest("SegmentTintsWithTintColor", &TintTests::setTintColor, &TintTests::segmentTints, &TintTests::cleanup);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		m_pImg = pAppAPI->loadSurface("resources/mipmaptest.png", pDevice->surfaceFactory());
		if (!m_pImg)
			return false;

		HiColor red = Color::Red;
		HiColor green = Color::Green;
		HiColor blue = Color::Blue;
		HiColor white = Color::White;
		HiColor yellow = Color::Yellow;

		m_pMultiStop = Tint::create( { {0.f, red}, {0.3f, yellow}, {0.6f, green}, {1.f, blue} }, {0.1f, 0.f}, {0.9f, 1.f}, ColorSpace::sRGB );

		{
			Tint::Blueprint bp;
			bp.begin = { 0.f, 0.f };
			bp.end = { 0.25f, 0.1f };
			bp.spread = TintSpread::Reflect;
			bp.stops = { {0.f, red}, {0.5f, red}, {0.5f, white}, {1.f, blue} };
			m_pHardEdges = Tint::create(bp);
		}

		{
			Tint::Blueprint bp;
			bp.shape = TintShape::Radial;
			bp.center = { 0.4f, 0.6f };
			bp.radius = { 0.5f, 0.3f };
			bp.stops = { {0.f, white}, {0.5f, yellow}, {1.f, HiColor(0, 0, 4096, 2048)} };
			m_pRadialFit = Tint::create(bp);
		}

		{
			Tint::Blueprint bp;
			bp.shape = TintShape::Radial;
			bp.radiusMode = TintRadius::Circle;
			bp.spread = TintSpread::Repeat;
			bp.center = { 0.5f, 0.5f };
			bp.radius = { 0.15f, 0.15f };
			bp.colorSpace = ColorSpace::sRGB;
			bp.stops = { {0.f, green}, {1.f, blue} };
			m_pRadialCircleRepeat = Tint::create(bp);
		}

		// Linear to radial can't morph, so this is a mix of the two.

		m_pMix = Tint::blend( Tint::create(red, blue, {0,0}, {1,0}), m_pRadialFit, 0.4f );

		// Segment tints, placed in the edgemap's own rectangle.

		for( int i = 0 ; i < c_segments ; i++ )
		{
			Tint::Blueprint bp;
			bp.shape = TintShape::Radial;
			bp.center = { 0.2f + 0.1f * i, 0.5f };
			bp.radius = { 0.3f, 0.6f };
			bp.stops = { {0.f, white}, {1.f, m_segmentColors[i]} };
			m_segmentTints[i] = Tint::create(bp);

			m_segmentMixTints[i] = Tint::blend( Tint::create(m_segmentColors[i], HiColor::Black, {0,0}, {1,1}), m_segmentTints[i], 0.5f );
		}

		spx samples[c_segments-1][512+1];

		for( int edge = 0 ; edge < c_segments-1 ; edge++ )
		{
			for( int spl = 0 ; spl < 512+1 ; spl++ )
				samples[edge][spl] = (int) ((60 + edge * 55 + std::sin(spl / 20.0 + edge) * 25) * 64);
		}

		auto pFactory = pDevice->edgemapFactory();
		SizeI size = canvas.size() / 64;
		size.w = std::min(size.w, 512);			// We have samples for this much.

		m_pSegmentEdgemap = pFactory->createEdgemap( WGBP(Edgemap, _.size = size, _.segments = c_segments, _.colors = m_segmentColors, _.tints = m_segmentTints ),
													 SampleOrigo::Top, samples[0], c_segments-1, 512+1 );

		m_pSegmentMixEdgemap = pFactory->createEdgemap( WGBP(Edgemap, _.size = size, _.segments = c_segments, _.colors = m_segmentColors, _.tints = m_segmentMixTints ),
													 SampleOrigo::Top, samples[0], c_segments-1, 512+1 );

		return m_pSegmentEdgemap && m_pSegmentMixEdgemap;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pImg = nullptr;
		m_pMultiStop = nullptr;
		m_pHardEdges = nullptr;
		m_pRadialFit = nullptr;
		m_pRadialCircleRepeat = nullptr;
		m_pMix = nullptr;

		for( int i = 0 ; i < c_segments ; i++ )
		{
			m_segmentTints[i] = nullptr;
			m_segmentMixTints[i] = nullptr;
		}

		m_pSegmentEdgemap = nullptr;
		m_pSegmentMixEdgemap = nullptr;
		return true;
	}

	bool cleanup(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTintColor(Color::White);
		pDevice->clearTint();
		return true;
	}

	//

	bool setMultiStop(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pMultiStop);
		return true;
	}

	bool setHardEdgesReflect(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pHardEdges);
		return true;
	}

	bool setRadialFit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pRadialFit);
		return true;
	}

	bool setRadialCircleRepeat(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pRadialCircleRepeat);
		return true;
	}

	bool setMix(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pMix);
		return true;
	}

	bool setRadialAndTintColor(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pRadialFit);
		pDevice->setTintColor(HiColor(4096, 2048, 4096, 3072));
		return true;
	}

	bool setRadialFitBlit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		pDevice->setTint(canvas, m_pRadialFit);
		return true;
	}

	bool setMixBlit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		pDevice->setTint(canvas, m_pMix);
		return true;
	}

	bool setMixBlitAndTintColor(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		pDevice->setTint(canvas, m_pMix);
		pDevice->setTintColor(HiColor(2048, 4096, 4096, 3072));
		return true;
	}

	bool setTintColor(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTintColor(HiColor(4096, 4096, 2048, 2048));
		return true;
	}

	//

	bool fill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->fill(canvas, Color::White);
		return true;
	}

	bool subpixelFill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->fill(RectSPX(canvas.x + 10*64 + 21, canvas.y + 12*64 + 40, canvas.w/2 + 13, canvas.h/3 + 7), Color::White);
		pDevice->fill(RectSPX(canvas.x + canvas.w/2 + 33, canvas.y + canvas.h/2 + 5, canvas.w/3 + 50, canvas.h/3 + 60), HiColor(4096, 4096, 4096, 2048));
		return true;
	}

	bool blit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->blit(canvas.pos());
		return true;
	}

	bool rotScaleBlit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->rotScaleBlit(canvas, 35.f, 1.3f);
		return true;
	}

	bool segmentTints(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->drawEdgemap(canvas.pos(), m_pSegmentEdgemap);
		return true;
	}

	bool segmentTintsRot90(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->flipDrawEdgemap(canvas.pos(), m_pSegmentMixEdgemap, GfxFlip::Rot90);
		return true;
	}

private:

	static const int c_segments = 5;

	HiColor		m_segmentColors[c_segments] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue };

	Surface_p	m_pImg;

	Tint_p		m_pMultiStop;
	Tint_p		m_pHardEdges;
	Tint_p		m_pRadialFit;
	Tint_p		m_pRadialCircleRepeat;
	Tint_p		m_pMix;

	Tint_p		m_segmentTints[c_segments];
	Tint_p		m_segmentMixTints[c_segments];

	Edgemap_p	m_pSegmentEdgemap;
	Edgemap_p	m_pSegmentMixEdgemap;
};
