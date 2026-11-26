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

	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{
		m_pStdBitmap = pAppAPI->loadBitmap("resources/mipmaptest.png", pDevice->bitmapFactory(), { .sampleMethod = SampleMethod::Bilinear } );
		if (!m_pStdBitmap)
			return false;

		m_pMipBitmap = pAppAPI->loadBitmap("resources/mipmaptest.png", pDevice->bitmapFactory(), { .mipmap = true, .sampleMethod = SampleMethod::Bilinear } );
		if (!m_pMipBitmap)
			return false;

		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pStdBitmap = nullptr;
		m_pMipBitmap = nullptr;
		return true;
	}


	bool dummy(GfxDevice * pDevice, const RectI& canvas)
	{
		return true;
	}

	bool setMipmapped(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pMipBitmap);
		return true;
	}

	bool setStandard(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pStdBitmap);
		return true;
	}


	bool	blit_040(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->stretchBlit({ 0,0,int(256 * 0.4)*64,int(256 * 0.4)*64 });
		return true;
	}


private:
	Bitmap_p	m_pStdBitmap;
	Bitmap_p	m_pMipBitmap;
};
