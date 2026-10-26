#include "testsuite.h"

class CanvasFormatTests : public TestSuite
{
public:
	CanvasFormatTests()
	{
		name = "CanvasFormatTests";

		addTest("DrawToXRGB_8_linear", &CanvasFormatTests::init_XRGB_8_linear, &CanvasFormatTests::drawPrimitives, &CanvasFormatTests::exit_drawToSurface );
		addTest("DrawToARGB_8", &CanvasFormatTests::init_ARGB_8, &CanvasFormatTests::drawPrimitives, &CanvasFormatTests::exit_drawToSurface);
		addTest("DrawToXRGB_8", &CanvasFormatTests::init_XRGB_8, &CanvasFormatTests::drawPrimitives, &CanvasFormatTests::exit_drawToSurface);
		addTest("DrawToRGB_565_linear", &CanvasFormatTests::init_RGB_565_linear, &CanvasFormatTests::drawPrimitives, &CanvasFormatTests::exit_drawToSurface);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		m_pCanvasXRGB_8_linear = pDevice->surfaceFactory()->createSurface( WGBP(Surface,
																		_.size = canvas/64,
																		_.format = PixelFormat::XRGB_8,
																		_.colorSpace = ColorSpace::Linear,
																		_.canvas = true ));
		m_pCanvasARGB_8 = pDevice->surfaceFactory()->createSurface( WGBP(Surface,
																		 _.size = canvas/64,
																		 _.format = PixelFormat::ARGB_8,
																		 _.canvas = true ));
																   
																   
																   
		m_pCanvasXRGB_8 = pDevice->surfaceFactory()->createSurface( WGBP(Surface,
																		 _.size = canvas/64,
																		 _.format = PixelFormat::XRGB_8,
																		 _.canvas = true ));
		m_pCanvasRGB_565_linear = pDevice->surfaceFactory()->createSurface( WGBP(Surface,
																		  _.size = canvas/64,
																		  _.format = PixelFormat::RGB_565,
																		  _.colorSpace = ColorSpace::Linear,
																		  _.canvas = true ));
		return true;
	}

	bool	init_XRGB_8_linear(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pActiveCanvas = m_pCanvasXRGB_8_linear;
		return init_drawToSurface(pDevice, canvas);
	}

	bool	init_ARGB_8(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pActiveCanvas = m_pCanvasARGB_8;
		return init_drawToSurface(pDevice, canvas);
	}

	bool	init_XRGB_8(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pActiveCanvas = m_pCanvasXRGB_8;
		return init_drawToSurface(pDevice, canvas);
	}

	bool	init_RGB_565_linear(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pActiveCanvas = m_pCanvasRGB_565_linear;
		return init_drawToSurface(pDevice, canvas);
	}

	bool	init_drawToSurface(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pActiveCanvas->fill(Color::Transparent);
		pDevice->beginCanvasUpdate(m_pActiveCanvas);
		return true;
	}

	bool	exit_drawToSurface(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->endCanvasUpdate();
		pDevice->setBlitSource(m_pActiveCanvas);
		pDevice->blit({ 0,0 });
		m_pActiveCanvas = nullptr;
		return true;
	}


	bool	drawPrimitives(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->drawLine(canvas.pos() + CoordI(10, 10)*64, canvas.pos() + CoordI(canvas.size().w, canvas.size().h) - CoordI(10, 20)*64, Color::Red, 3*64);
		pDevice->drawLine(canvas.pos() + CoordI(10, 20)*64, canvas.pos() + CoordI(canvas.size().w, canvas.size().h) - CoordI(10, 10)*64, Color8(0, 0, 255, 128), 3*64);

		pDevice->drawLine(canvas.pos() + CoordI(5, 100)*64, canvas.pos() + CoordI(40,101)*64, Color::Green, 3*64);
		pDevice->drawLine(canvas.pos() + CoordI(5, 105)*64, canvas.pos() + CoordI(6, 145)*64, Color::Green, 3*64);


		CoordI	fillOfs = { canvas.x, canvas.y + canvas.h / 2 };
		SizeI	fillSize = { 50*64,50*64 };
		CoordI	stepping = { 60*64, 0 };

		pDevice->fill({ fillOfs, fillSize }, Color::Red);
		pDevice->fill({ fillOfs + stepping, fillSize }, Color8(0, 0, 255, 128));
		pDevice->fill({ fillOfs + stepping * 2, fillSize }, Color8(0, 0, 255, 64));
		pDevice->fill({ fillOfs + stepping * 3, fillSize }, Color8(0, 0, 255, 32));
		pDevice->fill({ fillOfs + stepping * 4, fillSize }, Color8(0, 0, 255, 16));

		return true;
	}



private:

	Surface_p		m_pActiveCanvas;


	Surface_p		m_pCanvasXRGB_8_linear;
	Surface_p		m_pCanvasARGB_8;
	Surface_p		m_pCanvasXRGB_8;
	Surface_p		m_pCanvasRGB_565_linear;
};
