#include "testsuite.h"

class A8Tests : public TestSuite
{
public:
	A8Tests()
	{
		name = "A8Tests";

		addTest("A8_BlitFromRGBA", &A8Tests::setCanvas, &A8Tests::blit, &A8Tests::finalize);
		addTest("A8_Fills", &A8Tests::setCanvas, &A8Tests::fills, &A8Tests::finalize);
	}

	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{
		m_pCanvas = pDevice->surfaceFactory()->createSurface( { .canvas = true, .format = PixelFormat::Alpha_8, .size = canvas.size()/64 } );

		m_pImg = pAppAPI->loadSurface("resources/splash.png", pDevice->surfaceFactory());
		if (!m_pImg)
			return false;


		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pCanvas = nullptr;
		m_pImg = nullptr;
		return true;
	}

	bool dummy(GfxDevice * pDevice, const RectI& canvas)
	{
		return true;
	}

	bool finalize(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->endCanvasUpdate();
		pDevice->setBlitSource(m_pCanvas);
		pDevice->blit({ 0,0 });
		return true;
	}


	bool setCanvas(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pCanvas->fill(Color::Transparent);
		pDevice->beginCanvasUpdate(m_pCanvas);
		return true;
	}

	bool	blit(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		pDevice->blit(canvas);
		return true;
	}

	bool	fills(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->fill(canvas - BorderSPX(64*64), Color8(0,0,0,64) );

		int ofs = 0;

		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill( RectI(6,80+ofs*20,500,15)*64, Color8(0, 0, 0, 128));
		ofs++;

		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->fill(RectI(6, 80 + ofs * 20, 500, 15)*64, Color8(0, 0, 0, 128));
		ofs++;
                
        pDevice->setMorphFactor(0.5f);
		pDevice->setBlendMode(BlendMode::Morph);
		pDevice->fill(RectI(6, 80 + ofs * 20, 500, 15)*64, Color8(0, 0, 0, 128));
		ofs++;

		pDevice->setBlendMode(BlendMode::Add);
		pDevice->fill(RectI(6, 80 + ofs * 20, 500, 15)*64, Color8(0, 0, 0, 128));
		ofs++;

		pDevice->setBlendMode(BlendMode::Subtract);
		pDevice->fill(RectI(6, 80 + ofs * 20, 500, 15)*64, Color8(0, 0, 0, 128));
		ofs++;

		pDevice->setBlendMode(BlendMode::Multiply);
		pDevice->fill(RectI(6, 80 + ofs * 20, 500, 15)*64, Color8(0,0,0,128) );
		ofs++;

		pDevice->setBlendMode(BlendMode::Min);
		pDevice->fill(RectI(6, 80 + ofs * 20, 500, 15)*64, Color8(0, 0, 0, 128));
		ofs++;

		pDevice->setBlendMode(BlendMode::Max);
		pDevice->fill(RectI(6, 80 + ofs * 20, 500, 15)*64, Color8(0, 0, 0, 128));
		ofs++;

		pDevice->setBlendMode(BlendMode::Invert);
		pDevice->fill(RectI(6, 80 + ofs * 20, 500, 15)*64, Color::White);
		ofs++;

		pDevice->setBlendMode(BlendMode::Blend);

		return true;
	}



private:
	Surface_p	m_pCanvas;
	Surface_p	m_pImg;

	float rot = 35;
	float scale = 1.3f;
};
