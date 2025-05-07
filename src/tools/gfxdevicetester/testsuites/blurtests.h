#include "testsuite.h"
#include <wg_blurbrush.h>

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

	bool init(GfxDevice * pDevice, const RectI& canvas, WonderApp::Visitor * pAppVisitor)
	{
//		m_pImg = pAppVisitor->loadSurface("resources/mipmaptest.png", pDevice->surfaceFactory());
//		if (!m_pImg)
//			return false;

		m_pCanvasSurface = pDevice->surfaceFactory()->createSurface( { .canvas = true, .size = canvas.size()/64  } );
		m_pBlurSurface[0] = pDevice->surfaceFactory()->createSurface( { .canvas = true, .format = PixelFormat::BGRX_8, .size = canvas.size() / 64 } );
		m_pBlurSurface[1] = pDevice->surfaceFactory()->createSurface( { .canvas = true, .format = PixelFormat::BGRX_8, .size = canvas.size() / 64 } );

		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pCanvasSurface = nullptr;
		m_pBlurSurface[0] = nullptr;
		m_pBlurSurface[1] = nullptr;
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

	bool setTintAndBlurbrush(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setTint(Color::Blue);
		setBlurbrush(pDevice, canvas);
		return true;
	}


	bool setGradientAndBlurbrush(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setTintGradient(RectSPX(0, 0, 512, 512) * 64, Gradient(Placement::West, Color::Red, Color::Blue));
		setBlurbrush(pDevice, canvas);
		return true;
	}

	bool setBlurbrush(GfxDevice* pDevice, const RectI& canvas)
	{
		m_pBlurSurface[0]->fill(HiColor::Transparent);
		m_pBlurSurface[1]->fill(HiColor::Transparent);

		float blue[9] = { 0.15f, 0.15f, 0.15f, 0, 0.4f, 0, 0,0,0 };
		float green[9] = {0,0,0, 0,0.7f,0, 0,0,0};
		float red[9] = { 0.14f,0.1f,0.14f, 0.1f, 0.0f, 0.1f, 0.14f, 0.1f, 0.14f };

		m_pBrush = Blurbrush::create({
			.blue = blue,
			.green = green,
			.red = red,
			.size = 64 * 8
		});

		pDevice->beginCanvasUpdate(m_pCanvasSurface);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor::Transparent);
		pDevice->fill( RectSPX(100,100,64,64)*64, HiColor::White);
		pDevice->endCanvasUpdate();

		for( int i = 0 ; i < 2 ; i++ )
		{
			pDevice->beginCanvasUpdate(m_pBlurSurface[i]);
			pDevice->setBlendMode(BlendMode::Replace);
			pDevice->fill(HiColor::Black);
			pDevice->endCanvasUpdate();
		}
		return true;
 
	}


	bool blur(GfxDevice * pDevice, const RectI& canvas)
	{
		for( int i = 0 ; i < 10 ; i++ )
		{
			pDevice->beginCanvasUpdate(m_pBlurSurface[0]);
			pDevice->fill({100*64,100*64,200*64,200*64}, HiColor::White);
			pDevice->endCanvasUpdate();

			pDevice->beginCanvasUpdate(m_pBlurSurface[1]);
			pDevice->setBlitSource(m_pBlurSurface[0]);

			pDevice->setBlurbrush(m_pBrush);

			pDevice->blur({0,0});
			pDevice->endCanvasUpdate();

			std::swap( m_pBlurSurface[0], m_pBlurSurface[1] );
		}
		
		pDevice->setBlitSource(m_pBlurSurface[0]);

		pDevice->setBlurbrush(m_pBrush);
		pDevice->blur(canvas);
		return true;
	}


private:
	Surface_p	m_pCanvasSurface;
	Surface_p	m_pBlurSurface[2];

	Blurbrush_p	m_pBrush;
};
