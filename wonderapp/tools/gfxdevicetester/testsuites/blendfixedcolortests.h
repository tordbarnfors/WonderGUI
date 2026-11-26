#include "testsuite.h"
#include <wax_color.h>

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
		m_pSplash = pAppAPI->loadBitmap("resources/splash.png", pDevice->bitmapFactory(), { .sampleMethod = SampleMethod::Nearest } );
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
		
		Color bg[6] = { Color::Black, Color::White, Color::Red, Color::Green, Color::Blue, Color::Yellow };
		_background(pDevice, canvas, bg);
		pDevice->setBlendMode(BlendMode::BlendFixedColor);
		
		for( int y = 0 ; y < 6 ; y++ )
		{
			pDevice->setFixedBlendColor( bg[y] );

			for( int i = 0 ; i < 16 ; i++ )
			{
				pDevice->setTintColor(Color(4096, 4096, 4096, 4096*i/15));
				
				RectSPX r = {i*32, y*32, 32, 32};
				pDevice->stretchBlit( r*64 );
			}
		}

		pDevice->setBlendMode(oldBlendMode);
		pDevice->setTintColor(Color::White);
		return true;
	}

	bool circle(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto oldBlendMode = pDevice->blendMode();

		pDevice->setBlitSource(m_pSplash);
		
		Color bg[6] = { Color::Black, Color::White, Color::Red, Color::Green, Color::Blue, Color::Yellow };
		_background(pDevice, canvas, bg);
		pDevice->setBlendMode(BlendMode::BlendFixedColor);
		
		for( int y = 0 ; y < 6 ; y++ )
		{
			pDevice->setFixedBlendColor( bg[y] );

			for( int i = 0 ; i < 16 ; i++ )
			{
				RectSPX r = {i*32, y*32, 32, 32};
				RingSlice slice = { 1.f, Color(4096, 4096, 4096, 4096*i/15), nullptr };

				RingSpec ring;
				ring.rect = r*64;
				ring.thickness = 4*64;
				ring.nSlices = 1;
				ring.pSlices = &slice;
				pDevice->drawRing(ring);
			}
		}

		pDevice->setBlendMode(oldBlendMode);
		return true;
	}

	
	
	
	bool fill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto oldBlendMode = pDevice->blendMode();

	
		Color bg[6] = { Color::Black, Color::White, Color::Red, Color::Green, Color::Blue, Color::Yellow };
		_background(pDevice, canvas, bg);
		pDevice->setBlendMode(BlendMode::BlendFixedColor);
		

		for( int y = 0 ; y < 6 ; y++ )
		{
			pDevice->setFixedBlendColor( bg[y] );


			for( int i = 0 ; i < 16 ; i++ )
			{
				RectSPX r = {i*32, y*32, 32, 32};
				pDevice->fill( r*64, Color(4096, 4096, 4096, 4096*i/15) );
			}

		}

		pDevice->setBlendMode(oldBlendMode);
		return true;
	}

	bool line(GfxDevice * pDevice, const RectSPX& canvas)
	{
		auto oldBlendMode = pDevice->blendMode();

	
		Color bg[6] = { Color::Black, Color::White, Color::Red, Color::Green, Color::Blue, Color::Yellow };
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
				pDevice->drawLine( beg*64, end*64, Color(4096, 4096, 4096, 4096*i/15), 64*4 );
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

	void _background(GfxDevice * pDevice, const RectSPX& canvas, const Color * pColors)
	{
		pDevice->setBlendMode(BlendMode::Replace);

		for( int y = 0 ; y < 6 ; y++ )
			pDevice->fill( RectSPX(0, y*32*64, canvas.w, 32*64), pColors[y] );
	}

	Bitmap_p	m_pSplash;

};
