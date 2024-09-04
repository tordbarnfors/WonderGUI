#include "testsuite.h"

class EdgemapTests : public TestSuite
{
public:
	EdgemapTests()
	{
		name = "EdgemapTests";

		addTest("Edgemap SPX topdown", &EdgemapTests::spxTopDown );
		addTest("Edgemap SPX topdown flip 90", &EdgemapTests::spxTopDown90 );

		addTest("Edgemap SPX gradients X", &EdgemapTests::spxTopDownGradientsX );
		addTest("Edgemap SPX gradients Y", &EdgemapTests::spxTopDownGradientsY );
		addTest("Edgemap SPX gradients XY", &EdgemapTests::spxTopDownGradientsXY );

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
			tintmapsXY[i] = Gradyent::create(m_rainbowColors[i], Color::Black, Color::White, Color::Black);
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
 
