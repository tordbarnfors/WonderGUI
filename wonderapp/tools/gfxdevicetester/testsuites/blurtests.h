#include "testsuite.h"
#include <wax_blurbrush.h>
#include <wax_tint.h>

class BlurTests : public TestSuite
{
public:
	BlurTests()
	{
		name = "BlurTests";

		addTest("Blur", &BlurTests::setBlurbrush, &BlurTests::blur, &BlurTests::dummy);
		addTest("TintBlur", &BlurTests::setTintAndBlurbrush, &BlurTests::blur, &BlurTests::clearTint);
		addTest("GradientBlur", &BlurTests::setGradientAndBlurbrush, &BlurTests::blur, &BlurTests::clearTint);
	}

	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{
//		m_pImg = pAppAPI->loadBitmap("resources/mipmaptest.png", pDevice->bitmapFactory());
//		if (!m_pImg)
//			return false;

		m_pCanvasBitmap = pDevice->bitmapFactory()->createBitmap( { .canvas = true, .size = canvas.size()/64  } );
		m_pBlurBitmap[0] = pDevice->bitmapFactory()->createBitmap( { .canvas = true, .format = PixelFormat::XRGB_8, .size = canvas.size() / 64 } );
		m_pBlurBitmap[1] = pDevice->bitmapFactory()->createBitmap( { .canvas = true, .format = PixelFormat::XRGB_8, .size = canvas.size() / 64 } );

		m_pGradient = Tint::create(Color::Red, Color::Blue, {0,0}, {1,0});

		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pCanvasBitmap = nullptr;
		m_pBlurBitmap[0] = nullptr;
		m_pBlurBitmap[1] = nullptr;
		return true;
	}

	bool dummy(GfxDevice * pDevice, const RectI& canvas)
	{
		return true;
	}

	bool clearTint(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->clearTintColor();
		pDevice->clearTint();
		return true;
	}

	bool setTintAndBlurbrush(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setTintColor(Color::Blue);
		setBlurbrush(pDevice, canvas);
		return true;
	}


	bool setGradientAndBlurbrush(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setTint(RectSPX(0, 0, 512, 512) * 64, m_pGradient );
		setBlurbrush(pDevice, canvas);
		return true;
	}

	bool setBlurbrush(GfxDevice* pDevice, const RectI& canvas)
	{
		m_pBlurBitmap[0]->fill(Color::Transparent);
		m_pBlurBitmap[1]->fill(Color::Transparent);

		float blue[9] = { 0.15f, 0.15f, 0.15f, 0, 0.4f, 0, 0,0,0 };
		float green[9] = {0,0,0, 0,0.7f,0, 0,0,0};
		float red[9] = { 0.14f,0.1f,0.14f, 0.1f, 0.0f, 0.1f, 0.14f, 0.1f, 0.14f };

		m_pBrush = Blurbrush::create({
			.blue = blue,
			.green = green,
			.red = red,
			.size = 64 * 8
		});

		pDevice->beginCanvasUpdate(m_pCanvasBitmap);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(Color::Transparent);
		pDevice->fill( RectSPX(100,100,64,64)*64, Color::White);
		pDevice->endCanvasUpdate();

		for( int i = 0 ; i < 2 ; i++ )
		{
			pDevice->beginCanvasUpdate(m_pBlurBitmap[i]);
			pDevice->setBlendMode(BlendMode::Replace);
			pDevice->fill(Color::Black);
			pDevice->endCanvasUpdate();
		}
		return true;
 
	}


	bool blur(GfxDevice * pDevice, const RectI& canvas)
	{
		for( int i = 0 ; i < 10 ; i++ )
		{
			pDevice->beginCanvasUpdate(m_pBlurBitmap[0]);
			pDevice->fill({100*64,100*64,200*64,200*64}, Color::White);
			pDevice->endCanvasUpdate();

			pDevice->beginCanvasUpdate(m_pBlurBitmap[1]);
			pDevice->setBlitSource(m_pBlurBitmap[0]);

			pDevice->setBlurbrush(m_pBrush);

			pDevice->blur({0,0});
			pDevice->endCanvasUpdate();

			std::swap( m_pBlurBitmap[0], m_pBlurBitmap[1] );
		}
		
		pDevice->setBlitSource(m_pBlurBitmap[0]);

		pDevice->setBlurbrush(m_pBrush);
		pDevice->blur(canvas);
		return true;
	}


private:
	Bitmap_p	m_pCanvasBitmap;
	Bitmap_p	m_pBlurBitmap[2];

	Blurbrush_p	m_pBrush;
	Tint_p	m_pGradient;
};
