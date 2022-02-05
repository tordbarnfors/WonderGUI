#include "testsuite.h"

class MipmapTests : public TestSuite
{
public:
	MipmapTests()
	{
		name = "MipmapTests";

		addTest("Mipmapped_0.4", &MipmapTests::setMipmapped, &MipmapTests::blit_040, &MipmapTests::dummy);
		addTest("Standard_0.4", &MipmapTests::setStandard, &MipmapTests::blit_040, &MipmapTests::dummy);

	}

	bool init(GfxDevice * pDevice, const RectI& canvas, AppVisitor * pAppVisitor)
	{
		m_pStdSurface = pAppVisitor->loadSurface("resources/mipmaptest.png", pDevice->surfaceFactory());
		if (!m_pStdSurface)
			return false;
		m_pStdSurface->setScaleMode(ScaleMode::Interpolate);

		m_pMipSurface = pAppVisitor->loadSurface("resources/mipmaptest.png", pDevice->surfaceFactory(), SurfaceFlag::Mipmapped );
		if (!m_pMipSurface)
			return false;
		m_pMipSurface->setScaleMode(ScaleMode::Interpolate);

		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pStdSurface = nullptr;
		m_pMipSurface = nullptr;
		return true;
	}


	bool dummy(GfxDevice * pDevice, const RectI& canvas)
	{
		return true;
	}

	bool setMipmapped(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pMipSurface);
		return true;
	}

	bool setStandard(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pStdSurface);
		return true;
	}


	bool	blit_040(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->stretchBlit({ 0,0,int(256 * 0.4)*64,int(256 * 0.4)*64 });
		return true;
	}


private:
	Surface_p	m_pStdSurface;
	Surface_p	m_pMipSurface;
};
