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

	}


	bool init(GfxDevice * pDevice, const RectI& canvas, WonderApp::Visitor * pAppVisitor)
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
		
		
		Tintmap_p tintmapsX[8];
		Tintmap_p tintmapsY[8];
		Tintmap_p tintmapsXY[8];

		for( int i = 0 ; i < 8 ; i++ )
		{
			tintmapsX[i] = Gradyent::create(Color::White, Color::White, m_rainbowColors[i], Color::Black);
			tintmapsY[i] = Gradyent::create(m_rainbowColors[i], Color::Black, Color::White, Color::White);
			tintmapsXY[i] = Gradyent::create(m_rainbowColors[i], Color::Black, Color::Red, Color::Green);
		}
		
		
		m_pEdgemapSPXGradientX = pFactory->createEdgemap( WGBP(Edgemap, _.size = canvas.size()/64, _.segments = 8, _.tintmaps = tintmapsX ),
												  SampleOrigo::Top, sampleBuffer[0], 7 );

		m_pEdgemapSPXGradientY = pFactory->createEdgemap( WGBP(Edgemap, _.size = canvas.size()/64, _.segments = 8, _.tintmaps = tintmapsY ),
												  SampleOrigo::Top, sampleBuffer[0], 7 );

		m_pEdgemapSPXGradientXY = pFactory->createEdgemap( WGBP(Edgemap, _.size = canvas.size()/64, _.segments = 8, _.tintmaps = tintmapsXY ),
												  SampleOrigo::Top, sampleBuffer[0], 7 );

		return true;
	}



	bool spxTopDown(GfxDevice * pDevice, const RectI& canvas)
	{
		
		pDevice->drawEdgemap(canvas, m_pEdgemapSPX1 );
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

	Edgemap_p	m_pEdgemapSPX1;
	Edgemap_p	m_pEdgemapSPXGradientX;
	Edgemap_p	m_pEdgemapSPXGradientY;
	Edgemap_p	m_pEdgemapSPXGradientXY;


	HiColor	m_rainbowColors[8] = { Color::Red, Color::Orange, Color::Yellow, Color::Green, Color::Blue, Color::Indigo, Color::Violet, Color( 255,255,255,128 ) };
};
 
