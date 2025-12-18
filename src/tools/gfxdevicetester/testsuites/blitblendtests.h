#include "testsuite.h"

class BlitBlendTests : public TestSuite
{
public:
	BlitBlendTests()
	{
		name = "BlitBlendTests";

		addTest("BlitReplace", &BlitBlendTests::setReplace, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
		addTest("BlitBlend", &BlitBlendTests::setBlend, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
		addTest("BlitAdd", &BlitBlendTests::setAdd, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
		addTest("BlitSubtract", &BlitBlendTests::setSubtract, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
		addTest("BlitMultiply", &BlitBlendTests::setMultiply, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
		addTest("BlitMin", &BlitBlendTests::setMin, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
		addTest("BlitMax", &BlitBlendTests::setMax, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
		addTest("BlitInvert", &BlitBlendTests::setInvert, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
		addTest("BlitIgnore", &BlitBlendTests::setIgnore, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
//		addTest("BlendModeUndefined", &BlitBlendTests::setUndefined, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
	}


	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{
		m_pSplash = pAppAPI->loadSurface("resources/splash.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Nearest } );
		if (!m_pSplash)
			return false;

		m_pBackground = pAppAPI->loadSurface("resources/What-Goes-Up-3.bmp", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Nearest } );
		if (!m_pBackground)
			return false;

		return true;
	}

	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pSplash = nullptr;
		m_pBackground = nullptr;
		return true;
	}


	bool cleanup(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

	bool setReplace(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pSplash);

		pDevice->setBlendMode(BlendMode::Replace);
		return true;
	}

	bool setBlend(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pSplash);

		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

	bool setAdd(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pSplash);

		pDevice->setBlendMode(BlendMode::Add);
		return true;
	}

	bool setSubtract(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pSplash);

		pDevice->setBlendMode(BlendMode::Subtract);
		return true;
	}

	bool setMultiply(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pSplash);

		pDevice->setBlendMode(BlendMode::Multiply);
		return true;
	}

	bool setMin(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pSplash);

		pDevice->setBlendMode(BlendMode::Min);
		return true;
	}

	bool setMax(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pSplash);

		pDevice->setBlendMode(BlendMode::Max);
		return true;
	}

	bool setInvert(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pSplash);

		pDevice->setBlendMode(BlendMode::Invert);
		return true;
	}

	bool setIgnore(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pSplash);

		pDevice->setBlendMode(BlendMode::Ignore);
		return true;
	}

	bool setUndefined(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlitSource(m_pSplash);

		pDevice->setBlendMode(BlendMode::Undefined);
		return true;
	}

	bool blit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->blit({ 0,0 });
		return true;
	}

	Surface_p	m_pBackground;
	Surface_p	m_pSplash;


};
