#include "testsuite.h"

class TintBlitTests : public TestSuite
{
public:
	TintBlitTests()
	{
		name = "TintBlitTests";

		addTest("NoTintBlit", &TintBlitTests::setNoTint, &TintBlitTests::blit, &TintBlitTests::cleanup);
		addTest("FlatTintBlit", &TintBlitTests::setFlatTint, &TintBlitTests::blit, &TintBlitTests::cleanup);
		addTest("FlatTintFlipBlit", &TintBlitTests::setFlatTint, &TintBlitTests::flipBlit90, &TintBlitTests::cleanup);
		addTest("FlatTintStretchBlit", &TintBlitTests::setFlatTint, &TintBlitTests::stretchBlit, &TintBlitTests::cleanup);
		addTest("TintXBlit", &TintBlitTests::setTintX, &TintBlitTests::blit, &TintBlitTests::cleanup);
		addTest("TintXFlipBlit", &TintBlitTests::setTintX, &TintBlitTests::flipBlit90, &TintBlitTests::cleanup);
		addTest("TintXStretchBlit", &TintBlitTests::setTintX, &TintBlitTests::stretchBlit, &TintBlitTests::cleanup);
		addTest("TintYBlit", &TintBlitTests::setTintY, &TintBlitTests::blit, &TintBlitTests::cleanup);
		addTest("TintYFlipBlit", &TintBlitTests::setTintY, &TintBlitTests::flipBlit90, &TintBlitTests::cleanup);
		addTest("TintYStretchBlit", &TintBlitTests::setTintY, &TintBlitTests::stretchBlit, &TintBlitTests::cleanup);
		addTest("TintXYBlit", &TintBlitTests::setTintXY, &TintBlitTests::blit, &TintBlitTests::cleanup);
		addTest("TintXYFlipBlit", &TintBlitTests::setTintXY, &TintBlitTests::flipBlit90, &TintBlitTests::cleanup);
		addTest("TintXYStretchBlit", &TintBlitTests::setTintXY, &TintBlitTests::stretchBlit, &TintBlitTests::cleanup);
		addTest("TintXYOffsetBlit", &TintBlitTests::setOffsetTintXY, &TintBlitTests::offsetBlit, &TintBlitTests::cleanup);
	}

	bool init(GfxDevice * pDevice, const RectI& canvas, WonderApp::Visitor * pAppVisitor)
	{
		m_pImg = pAppVisitor->loadSurface("resources/mipmaptest.png", pDevice->surfaceFactory());
		if (!m_pImg)
			return false;


		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pImg = nullptr;
		return true;
	}


	bool dummy(GfxDevice * pDevice, const RectI& canvas)
	{
		return true;
	}

	bool cleanup(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setTintColor(Color::White);
		pDevice->clearTintGradient();
		return true;
	}


	bool setNoTint(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		return true;
	}

	bool setFlatTint(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		pDevice->setTintColor(Color::Blue);
		return true;
	}

	bool setTintX(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		pDevice->setTintGradient(canvas, Gradient(Placement::West, Color::Red, Color::Blue) );
		return true;
	}

	bool setTintY(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		pDevice->setTintGradient(canvas, Gradient(Placement::North, Color::Red, Color::Blue) );
		return true;
	}

	bool setTintXY(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		pDevice->setTintGradient(canvas, Gradient(Placement::NorthWest, Color::Red, Color::Blue));
		return true;
	}

	bool setOffsetTintXY(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		pDevice->setTintGradient(RectI( 200,200,100,100 )*64, Gradient(Placement::NorthWest, Color::Red, Color::Blue));
		return true;
	}

	bool	offsetBlit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->stretchBlit(RectI(200,200,100,100)*64);
		return true;
	}

	bool	blit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->blit(canvas);
		return true;
	}

	bool	flipBlit90(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipBlit(canvas, GfxFlip::Rot90);
		return true;
	}


	bool	rotScaleBlit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->rotScaleBlit(canvas, rot, scale);
		return true;
	}

	bool	stretchBlit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->stretchBlit(canvas, RectSPX( 0,0, m_pImg->pixelSize()*64 ) );
		return true;
	}

private:
	Surface_p	m_pImg;

	float rot = 35;
	float scale = 1.3f;
};
