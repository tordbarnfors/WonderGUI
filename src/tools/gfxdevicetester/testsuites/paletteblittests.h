#include "testsuite.h"

class PaletteBlitTests : public TestSuite
{
public:
	PaletteBlitTests()
	{
		name = "PaletteBlitTests";

		addTest("PaletteBlit", &PaletteBlitTests::setNearestPicture, &PaletteBlitTests::blit, &PaletteBlitTests::dummy);
		addTest("PaletteRotScaleBlit", &PaletteBlitTests::setNearestPicture, &PaletteBlitTests::rotScaleBlit, &PaletteBlitTests::dummy);
		addTest("PaletteRotScaleBlitInterpolate", &PaletteBlitTests::setInterpolatedPicture, &PaletteBlitTests::rotScaleBlit, &PaletteBlitTests::dummy);
		addTest("PaletteStretchBlit", &PaletteBlitTests::setNearestPicture, &PaletteBlitTests::stretchBlit, &PaletteBlitTests::dummy);
		addTest("PaletteStretchBlitInterpolated", &PaletteBlitTests::setInterpolatedPicture, &PaletteBlitTests::stretchBlit, &PaletteBlitTests::dummy);

		addTest("PaletteBlitGradX", &PaletteBlitTests::setNearestPictureGradient, &PaletteBlitTests::rotScaleBlit, &PaletteBlitTests::clearTint);
		addTest("PaletteBlitInterpolateGradX", &PaletteBlitTests::setInterpolatedPictureGradient, &PaletteBlitTests::rotScaleBlit, &PaletteBlitTests::clearTint);

	}

	bool init(GfxDevice * pDevice, const RectI& canvas, WonderApp::Visitor * pAppVisitor)
	{
		m_pLeaves = pAppVisitor->loadSurface("resources/blaetter.gif", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Nearest } );
		if (!m_pLeaves)
			return false;

		m_pLeavesInterpolated = pAppVisitor->loadSurface("resources/blaetter.gif", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Bilinear } );
		if (!m_pLeavesInterpolated)
			return false;

		return true;
	}

	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pLeaves = nullptr;
		return true;
	}


	bool dummy(GfxDevice * pDevice, const RectI& canvas)
	{
		return true;
	}

	bool clearTint(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->clearTint();
		return true;
	}


	bool setNearestPicture(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pLeaves);
		return true;
	}

	bool setInterpolatedPicture(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pLeavesInterpolated);
		return true;
	}

	bool setNearestPictureGradient(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pLeaves);
		pDevice->setTintGradient(canvas, Gradient(Placement::West, Color::Red, Color::Blue));
		return true;
	}

	bool setInterpolatedPictureGradient(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pLeavesInterpolated);
		pDevice->setTintGradient(canvas, Gradient(Placement::West, Color::Red, Color::Blue));
		return true;
	}


	bool	blit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->blit(canvas);
		return true;
	}

	bool	flipBlit(GfxDevice * pDevice, const RectI& canvas)
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
		pDevice->stretchBlit(canvas, RectSPX(0, 0, m_pLeaves->pixelSize()*64));
		return true;
	}

private:
	Surface_p	m_pLeaves;
	Surface_p	m_pLeavesInterpolated;

	float rot = 35;
	float scale = 5.0f;
};
