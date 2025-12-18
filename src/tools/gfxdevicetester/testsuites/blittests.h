#include "testsuite.h"

class PatchBlitTests : public TestSuite
{
public:
	PatchBlitTests()
	{
		name = "PatchBlitTests";

		addTest("Blit", &PatchBlitTests::setSplash, &PatchBlitTests::blit, &PatchBlitTests::dummy);
		addTest("BlitAlphaOnly", &PatchBlitTests::setAlphaOnly, &PatchBlitTests::blit, &PatchBlitTests::dummy);
		addTest("TintBlitAlphaOnly", &PatchBlitTests::setAlphaOnlyWithTint, &PatchBlitTests::blit, &PatchBlitTests::resetTint);
		addTest("TintShrinkBlitAlphaOnly", &PatchBlitTests::setAlphaOnlyWithTint, &PatchBlitTests::shrinkBlit, &PatchBlitTests::resetTint);

		addTest("FlipBlit90", &PatchBlitTests::setSplash, &PatchBlitTests::flipBlit90, &PatchBlitTests::dummy);
		addTest("FlipBlit180", &PatchBlitTests::setSplash, &PatchBlitTests::flipBlit180, &PatchBlitTests::dummy);
		addTest("FlipBlit270", &PatchBlitTests::setSplash, &PatchBlitTests::flipBlit270, &PatchBlitTests::dummy);

		addTest("StretchBlit", &PatchBlitTests::setSplash, &PatchBlitTests::stretchBlit, &PatchBlitTests::dummy);

		addTest("RotScaleBlit", &PatchBlitTests::setClockFace, &PatchBlitTests::rotScaleBlit, &PatchBlitTests::dummy);
	}

	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{

		m_pClockFace = pAppAPI->loadSurface( "resources/clockface_2500.png", pDevice->surfaceFactory(), { .buffered = true, .sampleMethod = SampleMethod::Bilinear } );
		if (!m_pClockFace)
			return false;

		m_pSplash = pAppAPI->loadSurface("resources/splash.png", pDevice->surfaceFactory(), { .buffered = true, .sampleMethod = SampleMethod::Bilinear });
		if (!m_pSplash)
			return false;

		m_pAlphaOnly = pDevice->surfaceFactory()->createSurface( WGBP(Surface,
																	  _.size = { 400,400 },
																	  _.format = PixelFormat::Alpha_8 ));

		RectI	copySource = RectI(m_pClockFace->pixelSize()).center({ 400,400 });
		m_pAlphaOnly->copy({ 0,0 },m_pClockFace, copySource );


		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pClockFace = nullptr;
		m_pSplash = nullptr;
		return true;
	}


	bool dummy(GfxDevice * pDevice, const RectI& canvas)
	{
		return true;
	}

	bool setClockFace(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pClockFace);
		return true;
	}

	bool setSplash(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pSplash);
		return true;
	}

	bool setAlphaOnly(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pAlphaOnly);
		return true;
	}

	bool setAlphaOnlyWithTint(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pAlphaOnly);
		pDevice->setTintColor( Color::DarkBlue );
		return true;
	}

	bool resetTint(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setTintColor( Color::White );
		return true;
	}

	
	bool	blit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->blit(canvas);
		return true;
	}

	bool	shrinkBlit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->stretchBlit( canvas/4 );
		return true;
	}

	
	bool	flipBlit90(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipBlit(canvas, GfxFlip::Rot90);
		return true;
	}

	bool	flipBlit180(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipBlit(canvas, GfxFlip::Rot180);
		return true;
	}

	bool	flipBlit270(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipBlit(canvas, GfxFlip::Rot270);
		return true;
	}


	bool	rotScaleBlit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->rotScaleBlit(canvas, rot, scale);
		return true;
	}

	bool	stretchBlit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->stretchBlit(canvas, RectI( 0,0, m_pSplash->pixelSize()*64 ) );
		return true;
	}

private:
	Surface_p	m_pClockFace;
	Surface_p	m_pSplash;
	Surface_p	m_pAlphaOnly;

	float rot = 35;
	float scale = 1.3f;
};
