#include "testsuite.h"

class BlitConsistencyTests : public TestSuite
{
public:
	BlitConsistencyTests()
	{
		name = "BlitConsistencyTests";

		addTest("Blit3x3Inter", &BlitConsistencyTests::setPaletteInterpolate, &BlitConsistencyTests::blitX, &BlitConsistencyTests::cleanup);
		addTest("Blit3x3Near", &BlitConsistencyTests::setPaletteNearest, &BlitConsistencyTests::blitX, &BlitConsistencyTests::cleanup);

		addTest("BlitPaletteInter", &BlitConsistencyTests::setPaletteInterpolate, &BlitConsistencyTests::blitPalette, &BlitConsistencyTests::cleanup);
		addTest("BlitPaletteNear", &BlitConsistencyTests::setPaletteNearest, &BlitConsistencyTests::blitPalette, &BlitConsistencyTests::cleanup);

		addTest("BlitFrameInter", &BlitConsistencyTests::setFrameInterpolate, &BlitConsistencyTests::blitFrame, &BlitConsistencyTests::cleanup);
		addTest("BlitFrameNear", &BlitConsistencyTests::setFrameNearest, &BlitConsistencyTests::blitFrame, &BlitConsistencyTests::cleanup);

		addTest("BlitInsideFrameInter", &BlitConsistencyTests::setFrameInterpolate, &BlitConsistencyTests::blitInsideFrame, &BlitConsistencyTests::cleanup);
		addTest("BlitInsideFrameNear", &BlitConsistencyTests::setFrameNearest, &BlitConsistencyTests::blitInsideFrame, &BlitConsistencyTests::cleanup);

        addTest("BlitGridInter", &BlitConsistencyTests::setGridInterpolate, &BlitConsistencyTests::blitGrid, &BlitConsistencyTests::cleanup);
        addTest("BlitGridNear", &BlitConsistencyTests::setGridNearest, &BlitConsistencyTests::blitGrid, &BlitConsistencyTests::cleanup);

	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		m_pPalette = pAppAPI->loadSurface("resources/stretchblit_consistency.png", pDevice->surfaceFactory(), { .buffered = true, .sampleMethod = SampleMethod::Nearest });
		if (!m_pPalette)
			return false;

		m_pFrame = pAppAPI->loadSurface("resources/white_frame_256x256.png", pDevice->surfaceFactory(), { .buffered = true, .sampleMethod = SampleMethod::Nearest });
		if (!m_pFrame)
			return false;

		m_pGrid = pAppAPI->loadSurface("resources/grid_31x31.png", pDevice->surfaceFactory(), { .buffered = true, .sampleMethod = SampleMethod::Nearest });
        if (!m_pGrid)
            return false;

		m_pPaletteBilinear = m_pPalette->convert( { .sampleMethod = SampleMethod::Bilinear }, pDevice->surfaceFactory() );
		m_pFrameBilinear = m_pFrame->convert( { .sampleMethod = SampleMethod::Bilinear }, pDevice->surfaceFactory() );
		m_pGridBilinear = m_pGrid->convert( { .sampleMethod = SampleMethod::Bilinear }, pDevice->surfaceFactory() );

		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pPalette = nullptr;
		m_pFrame = nullptr;
		return true;
	}


	bool cleanup(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(nullptr);
		return true;
	}

	bool setPaletteInterpolate(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pPaletteBilinear);
		return true;
	}

	bool setPaletteNearest(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pPalette);
		return true;
	}

	bool setFrameInterpolate(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pFrameBilinear);
		return true;
	}

	bool setFrameNearest(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlitSource(m_pFrame);
		return true;
	}

    bool setGridInterpolate(GfxDevice * pDevice, const RectSPX& canvas)
    {
        pDevice->setBlitSource(m_pGridBilinear);
        return true;
    }
    
    bool setGridNearest(GfxDevice * pDevice, const RectSPX& canvas)
    {
        pDevice->setBlitSource(m_pGrid);
        return true;
    }

	bool	blitPalette(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->stretchBlit( RectSPX( 64,0,190,64 )*64, RectSPX( 1,1,3,3 )*64 );
		pDevice->stretchBlit( RectSPX( 0,64,64,190 )*64, RectSPX( 5,1,3,3 )*64 );
		pDevice->stretchBlit( RectSPX( 1,1,60,60 )*64, RectSPX(1,5,1,1)*64 );

        pDevice->stretchBlit( RectSPX( 80,80,30,3 )*64, RectSPX(1, 1, 3, 3)*64);

        for( int i = 0 ; i < 10 ; i++ )
            pDevice->stretchBlit( RectSPX( 80,100+i*5,i,3 )*64, RectSPX(1, 1, 3, 3)*64);

        return true;
	}

	bool	blitFrame(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for( int i = 0 ; i < 82 ; i++ )
			pDevice->stretchBlit( RectSPX( i*3,i*3,512-i*6,512-i*6 )*64, RectSPX(0, 0, 256, 256)*64);

		return true;
	}

	bool	blitInsideFrame(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < 82 ; i++)
			pDevice->stretchBlit( RectSPX( i * 3,i * 3,512 - i * 6,512 - i * 6 )*64, RectSPX(1, 1, 254, 254)*64);

		return true;
	}

	bool	blitX(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->stretchBlit( RectSPX( 10,10,3,3 )*64, RectSPX(1, 1, 3, 3)*64);
		return true;
	}

    bool    blitGrid(GfxDevice * pDevice, const RectSPX& canvas)
    {
		int width = canvas.w / 64;
		pDevice->stretchBlit( RectSPX( 0,0, width, 31 )*64 );

        int ofsY = 31;
        for( int y = 40; y > 0 ; y-- )
        {
            for( int i = 0; i < width ; i+=y )
            {
                if( pDevice->blitSource()->sampleMethod() == SampleMethod::Bilinear )
                    pDevice->precisionBlit( RectSPX(i,ofsY,y,10)*64, RectF(i*30.f/(width-1), 0, y*30.f/(width-1), 10)*64 );
                else
                    pDevice->precisionBlit( RectSPX(i,ofsY,y,10)*64, RectF(i*31.f/width, 0, y*31.f/width, 10)*64 );

            }
            ofsY += 10;
        }

        return true;
    }


private:
	Surface_p	m_pPalette;
	Surface_p	m_pPaletteBilinear;
	Surface_p	m_pFrame;
	Surface_p	m_pFrameBilinear;
	Surface_p   m_pGrid;
	Surface_p   m_pGridBilinear;
};
