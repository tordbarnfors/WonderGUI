#include "testsuite.h"
#include <wg_color.h>

class BlendFixedColorTests : public TestSuite
{
public:
	BlendFixedColorTests()
	{
		name = "BlendFixedColorTests";

		addTest("BlendFixedColorFill", &BlendFixedColorTests::fill);
		addTest("BlendFixedColorLine", &BlendFixedColorTests::line);
		addTest("BlendFixedColorBlit", &BlendFixedColorTests::blit);
		addTest("BlendFixedColorCircle", &BlendFixedColorTests::circle);

	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		m_pSplash = pAppAPI->loadSurface("resources/splash.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Nearest } );
		if (!m_pSplash)
			return false;

		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return true;
	}

	bool blit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto oldBlendMode = pDevice->blendMode();

		pDevice->setBlitSource(m_pSplash);
		
		HiColor bg[6] = { Color8::Black, Color8::White, Color8::Red, Color8::Green, Color8::Blue, Color8::Yellow };
		_background(pDevice, canvas, bg);
		pDevice->setBlendMode(BlendMode::BlendFixedColor);
		
		for( int y = 0 ; y < 6 ; y++ )
		{
			pDevice->setFixedBlendColor( bg[y] );

			for( int i = 0 ; i < 16 ; i++ )
			{
				pDevice->setTintColor(HiColor(4096, 4096, 4096, 4096*i/15));
				
				RectSPX r = {i*32, y*32, 32, 32};
				pDevice->stretchBlit( r*64 );
			}
		}

		pDevice->setBlendMode(oldBlendMode);
		pDevice->setTintColor(HiColor::White);
		return true;
	}

	bool circle(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto oldBlendMode = pDevice->blendMode();

		pDevice->setBlitSource(m_pSplash);
		
		HiColor bg[6] = { Color8::Black, Color8::White, Color8::Red, Color8::Green, Color8::Blue, Color8::Yellow };
		_background(pDevice, canvas, bg);
		pDevice->setBlendMode(BlendMode::BlendFixedColor);
		
		for( int y = 0 ; y < 6 ; y++ )
		{
			pDevice->setFixedBlendColor( bg[y] );

			for( int i = 0 ; i < 16 ; i++ )
			{
				RectSPX r = {i*32, y*32, 32, 32};
				pDevice->drawElipse(r*64, 4*64, HiColor(4096, 4096, 4096, 4096*i/15));
			}
		}

		pDevice->setBlendMode(oldBlendMode);
		return true;
	}

	
	
	
	bool fill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto oldBlendMode = pDevice->blendMode();

	
		HiColor bg[6] = { Color8::Black, Color8::White, Color8::Red, Color8::Green, Color8::Blue, Color8::Yellow };
		_background(pDevice, canvas, bg);
		pDevice->setBlendMode(BlendMode::BlendFixedColor);
		

		for( int y = 0 ; y < 6 ; y++ )
		{
			pDevice->setFixedBlendColor( bg[y] );


			for( int i = 0 ; i < 16 ; i++ )
			{
				RectSPX r = {i*32, y*32, 32, 32};
				pDevice->fill( r*64, HiColor(4096, 4096, 4096, 4096*i/15) );
			}

		}

		pDevice->setBlendMode(oldBlendMode);
		return true;
	}

	bool line(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto oldBlendMode = pDevice->blendMode();

	
		HiColor bg[6] = { Color8::Black, Color8::White, Color8::Red, Color8::Green, Color8::Blue, Color8::Yellow };
		_background(pDevice, canvas, bg);
		pDevice->setBlendMode(BlendMode::BlendFixedColor);
		

		for( int y = 0 ; y < 6 ; y++ )
		{
			pDevice->setFixedBlendColor( bg[y] );


			for( int i = 0 ; i < 16 ; i++ )
			{
				CoordSPX beg = {i*32, y*32};
				CoordSPX end = {i*32+30, y*32+30};

				
				RectSPX r = {i*32, y*32, 32, 32};
				pDevice->drawLine( beg*64, end*64, HiColor(4096, 4096, 4096, 4096*i/15), 64*4 );
			}

		}

		pDevice->setBlendMode(oldBlendMode);
		return true;
	}

	
	
	

private:

	// BlendFixedColor is a promise that what we draw onto has the fixed blend color, which
	// lets backends that support it skip reading the canvas. Backends are free to blend
	// normally instead, so the rows are filled with their fixed blend colors to make the
	// results the same either way.

	void _background(GfxDevice * pDevice, const RectSPX& canvas, const HiColor * pColors)
	{
		pDevice->setBlendMode(BlendMode::Replace);

		for( int y = 0 ; y < 6 ; y++ )
			pDevice->fill( RectSPX(0, y*32*64, canvas.w, 32*64), pColors[y] );
	}

	Surface_p	m_pSplash;

};
