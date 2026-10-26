#include "testsuite.h"

class EdgemapTests : public TestSuite
{
public:
	EdgemapTests()
	{
		name = "EdgemapTests";

		addTest("Edgemap SPX topdown", &EdgemapTests::spxTopDown );
		addTest("Edgemap SPX topdown rot90", &EdgemapTests::spxTopDown90 );

		addTest("Edgemap SPX gradients X", &EdgemapTests::spxTopDownGradientsX );
		addTest("Edgemap SPX gradients Y", &EdgemapTests::spxTopDownGradientsY );
		addTest("Edgemap SPX gradients XY", &EdgemapTests::spxTopDownGradientsXY );

		addTest("Edgemap grad X rot90", &EdgemapTests::spxTopDownGradientsX90 );
		addTest("Edgemap grad Y rot90", &EdgemapTests::spxTopDownGradientsY90 );
		addTest("Edgemap grad XY rot90", &EdgemapTests::spxTopDownGradientsXY90 );

		addTest("Edgemap grad XY flipX", &EdgemapTests::spxTopDownGradientsXYFlipX );
		addTest("Edgemap grad XY flipY", &EdgemapTests::spxTopDownGradientsXYFlipY );

		addTest("Edgemap grad XY rot90 flipX", &EdgemapTests::spxTopDownGradientsXYRot90FlipX );
		addTest("Edgemap grad XY rot90 flipY", &EdgemapTests::spxTopDownGradientsXYRot90FlipY );

		addTest("Edgemap global tint", &EdgemapTests::globalTint);
		addTest("Edgemap global flat tint", &EdgemapTests::globalFlatTint);

		addTest("Edgemap tile", &EdgemapTests::tile);
		addTest("Edgemap tile rot90 shifted", &EdgemapTests::tileRot90Shifted);
		addTest("Edgemap tile gradients XY", &EdgemapTests::tileGradientsXY);
		addTest("Edgemap tile global tint", &EdgemapTests::tileGlobalTint);

	}


	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{
		
		spx sampleBuffer[7][512+1];
		
		int ofs = 20;
		
		for (int wave = 0; wave < 7; wave++)
		{
			ofs += (wave+1)*12;
			
			for (int spl = 0; spl < 512+1; spl++)
			{
				
				//m_edgemap1[wave][spl] = 10*(wave+1)*64;
				
				sampleBuffer[wave][spl] = (int)((ofs+sin(spl / 10.0) * (wave+1)*10) * 64);
			}
		}
		
		
		auto pFactory = pDevice->edgemapFactory();
		
		m_pEdgemapSPX1 = pFactory->createEdgemap( WGBP(Edgemap, _.size = canvas.size()/64, _.segments = 8, _.colors = m_rainbowColors ),
												 SampleOrigo::Top, sampleBuffer[0], 7 );
		
		
		Tint_p tintsX[8];
		Tint_p tintsY[8];
		Tint_p tintsXY[8];

		for( int i = 0 ; i < 8 ; i++ )
		{
			tintsX[i] = Tint::create(m_rainbowColors[i], Color::Black, {0,0}, {1,0});
			tintsY[i] = Tint::create(m_rainbowColors[i], Color::Black, {0,0}, {0,1});
			tintsXY[i] = Tint::create(m_rainbowColors[i], Color::Black, {0,0}, {1,1});

		}
		
		
		m_pEdgemapSPXGradientX = pFactory->createEdgemap( WGBP(Edgemap, _.size = canvas.size()/64, _.segments = 8, _.tints = tintsX ),
												  SampleOrigo::Top, sampleBuffer[0], 7 );

		m_pEdgemapSPXGradientY = pFactory->createEdgemap( WGBP(Edgemap, _.size = canvas.size()/64, _.segments = 8, _.tints = tintsY ),
												  SampleOrigo::Top, sampleBuffer[0], 7 );

		m_pEdgemapSPXGradientXY = pFactory->createEdgemap( WGBP(Edgemap, _.size = canvas.size()/64, _.segments = 8, _.tints = tintsXY ),
												  SampleOrigo::Top, sampleBuffer[0], 7 );

//		m_pSmallEdgemap = pFactory->createEdgemap(WGBP(Edgemap, _.size = { 100, 50 }, _.segments = 4, _.tints = tintsXY),
//			SampleOrigo::Top, sampleBuffer[0], 3, 513 );

		m_pSmallEdgemap = pFactory->createEdgemap(WGBP(Edgemap, _.colors = m_rainbowColors, _.size = {100, 50}, _.segments = 4),
			SampleOrigo::Top, sampleBuffer[0], 3, 513 );

		m_pGlobalTint = Tint::create(HiColor::TransparentWhite, HiColor::White, {0,0}, {0,1});

		// Tiling edgemaps. Samples wrap around horizontally, so there is one sample per pixel of width.

		const int tileW = 64;
		const int tileH = 40;

		spx tileSamples[3][tileW];

		for (int spl = 0; spl < tileW; spl++)
		{
			double a = spl * 2 * 3.14159265 / tileW;
			tileSamples[0][spl] = (int)((6 + sin(a) * 4) * 64);
			tileSamples[1][spl] = (int)((20 + sin(a * 2) * 8) * 64);
			tileSamples[2][spl] = (int)((32 + cos(a * 3) * 5) * 64);
		}

		m_pTilingEdgemap = pFactory->createEdgemap(WGBP(Edgemap, _.colors = m_rainbowColors, _.size = {tileW, tileH}, _.segments = 4, _.tiling = true),
			SampleOrigo::Top, tileSamples[0], 3 );

		m_pTilingEdgemapGradientXY = pFactory->createEdgemap(WGBP(Edgemap, _.tints = tintsXY, _.size = {tileW, tileH}, _.segments = 4, _.tiling = true),
			SampleOrigo::Top, tileSamples[0], 3 );

		return true;
	}



	bool spxTopDown(GfxDevice * pDevice, const RectI& canvas)
	{
		
		pDevice->drawEdgemap(canvas, m_pEdgemapSPX1 );
		return true;
	}

	bool globalTint(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setTint(RectSPX(100, 50, 250, 100)*64, m_pGlobalTint );
		pDevice->flipDrawEdgemap(CoordSPX(100, 50) * 64, m_pSmallEdgemap, GfxFlip::Rot270);

		pDevice->clearTint();
		return true;
	}

	bool globalFlatTint(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setTintColor( HiColor{4096,4096,4096,1024} );
		pDevice->drawEdgemap(CoordSPX(100, 50) * 64, m_pSmallEdgemap);
		pDevice->clearTintColor();
		return true;
	}

	bool tile(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->tileEdgemap(canvas, m_pTilingEdgemap);
		return true;
	}

	bool tileRot90Shifted(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->flipTileEdgemap(canvas, m_pTilingEdgemap, GfxFlip::Rot90, CoordSPX(13, -7) * 64);
		return true;
	}

	bool tileGradientsXY(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->tileEdgemap(canvas, m_pTilingEdgemapGradientXY, CoordSPX(-20, 15) * 64);
		return true;
	}

	bool tileGlobalTint(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setTint(canvas, m_pGlobalTint);
		pDevice->flipTileEdgemap(canvas, m_pTilingEdgemap, GfxFlip::FlipY);
		pDevice->clearTint();
		return true;
	}

	bool spxTopDownGradientsX(GfxDevice * pDevice, const RectI& canvas)
	{
		
		pDevice->drawEdgemap(canvas, m_pEdgemapSPXGradientX );
		return true;
	}

	bool spxTopDownGradientsY(GfxDevice * pDevice, const RectI& canvas)
	{
		
		pDevice->drawEdgemap(canvas, m_pEdgemapSPXGradientY );
		return true;
	}

	bool spxTopDownGradientsXY(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->drawEdgemap(canvas, m_pEdgemapSPXGradientXY );
		return true;
	}

	bool spxTopDownGradientsX90(GfxDevice * pDevice, const RectI& canvas)
	{
		
		pDevice->flipDrawEdgemap(canvas, m_pEdgemapSPXGradientX, GfxFlip::Rot90 );
		return true;
	}

	bool spxTopDownGradientsY90(GfxDevice * pDevice, const RectI& canvas)
	{
		
		pDevice->flipDrawEdgemap(canvas, m_pEdgemapSPXGradientY, GfxFlip::Rot90 );
		return true;
	}

	bool spxTopDownGradientsXY90(GfxDevice * pDevice, const RectI& canvas)
	{
		
		pDevice->flipDrawEdgemap(canvas, m_pEdgemapSPXGradientXY, GfxFlip::Rot90 );
		return true;
	}

	bool spxTopDownGradientsXYFlipX(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipDrawEdgemap(canvas, m_pEdgemapSPXGradientXY, GfxFlip::FlipX );
		return true;
	}

	bool spxTopDownGradientsXYFlipY(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipDrawEdgemap(canvas, m_pEdgemapSPXGradientXY, GfxFlip::FlipY );
		return true;
	}

	bool spxTopDownGradientsXYRot90FlipX(GfxDevice * pDevice, const RectI& canvas)
	{
		
		pDevice->flipDrawEdgemap(canvas, m_pEdgemapSPXGradientXY, GfxFlip::Rot90FlipX );
		return true;
	}

	bool spxTopDownGradientsXYRot90FlipY(GfxDevice * pDevice, const RectI& canvas)
	{
		
		pDevice->flipDrawEdgemap(canvas, m_pEdgemapSPXGradientXY, GfxFlip::Rot90FlipY );
		return true;
	}

	
	
	bool spxTopDown90(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipDrawEdgemap(canvas, m_pEdgemapSPX1, GfxFlip::Rot90 );
		return true;
	}

	

private:

	Tint_p	m_pGlobalTint;

	Edgemap_p	m_pEdgemapSPX1;
	Edgemap_p	m_pEdgemapSPXGradientX;
	Edgemap_p	m_pEdgemapSPXGradientY;
	Edgemap_p	m_pEdgemapSPXGradientXY;

	Edgemap_p	m_pSmallEdgemap;

	Edgemap_p	m_pTilingEdgemap;
	Edgemap_p	m_pTilingEdgemapGradientXY;

	HiColor	m_rainbowColors[8] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue, Color::Indigo, Color::Violet, Color( 255,255,255,128 ) };
};
 
