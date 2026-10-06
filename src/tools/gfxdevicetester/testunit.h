
#include <string>
#include <wondergui.h>

using namespace wg;
using namespace std;




class TestUnit
{
public:
	TestUnit() {}
	virtual ~TestUnit() {}

	virtual const string	name() const = 0;
	virtual bool			init(GfxDevice * pDevice, const RectI& canvas ) { return true;  }
	virtual bool			run(GfxDevice * pDevice, const RectI& canvas) = 0;

	bool		bSelected = false;



};


namespace test
{

	class StraightFill : public TestUnit
	{
	public:

		const string name() const { return "StraightFill"; }

		bool run(GfxDevice * pDevice, const RectI& canvas)
		{
			pDevice->setBlendMode(BlendMode::Replace);
			pDevice->fill( canvas, Color::RosyBrown );
			pDevice->setBlendMode(BlendMode::Blend);

			return true;
		}

	};

	class BlendFill : public TestUnit
	{
	public:

		const string name() const { return "BlendFill"; }

		bool run(GfxDevice * pDevice, const RectI& canvas)
		{
			pDevice->setTintColor({ 255,255,255,128 });

			pDevice->fill( canvas, Color::RosyBrown );

			pDevice->setTintColor({ 255,255,255,255 });

			return true;
		}

	};


	class OffscreenBGRACanvas : public TestUnit
	{
	public:
		const string	name() const { return "OffscreenBGRACanvas";  }
		bool			init(GfxDevice * pDevice, const RectI& canvas )
		{
			m_pMyCanvas = pDevice->surfaceFactory()->createSurface(canvas, PixelFormat::ARGB_8);

			m_pBlitSource = FileUtil::loadSurface("resources/splash.png", pDevice->surfaceFactory());
			return(m_pMyCanvas != nullptr && m_pBlitSource != nullptr);
		}

		bool			run(GfxDevice * pDevice, const RectI& canvas)
		{

			m_pMyCanvas->fill(Color::Transparent);

			pDevice->fill(canvas.size(), Color(0,255,0));
			Surface_p pOldCanvas = pDevice->canvas();

			pDevice->setCanvas(m_pMyCanvas);

			pDevice->blit(canvas, m_pBlitSource, CoordI(50, 50));

			pDevice->drawLine(canvas.pos() + CoordI(10, 10), canvas.pos() + CoordI(canvas.size().w, canvas.size().h) - CoordI(10, 20), Color::Red, 3.f);
			pDevice->drawLine(canvas.pos() + CoordI(10, 20), canvas.pos() + CoordI(canvas.size().w, canvas.size().h) - CoordI(10, 10), Color(0,0,255,128), 3.f);

			CoordI	fillOfs = { canvas.x, canvas.y + canvas.h / 2 };
			SizeI	fillSize = { 50,50 };
			CoordI	stepping = { 60, 0 };

			pDevice->fill({ fillOfs, fillSize }, Color::Red);
			pDevice->fill({ fillOfs + stepping, fillSize }, Color(0, 0, 255, 128));
			pDevice->fill({ fillOfs + stepping*2, fillSize }, Color(0, 0, 255, 64));
			pDevice->fill({ fillOfs + stepping * 3, fillSize }, Color(0, 0, 255, 32));
			pDevice->fill({ fillOfs + stepping * 4, fillSize }, Color(0, 0, 255, 16));


			pDevice->setCanvas(pOldCanvas);
			pDevice->blit({ 0,0 }, m_pMyCanvas);

			return true;
		}

	protected:

		Surface_p	m_pMyCanvas;
		Surface_p	m_pBlitSource;
	};

	class StretchBlitBlends : public TestUnit
	{
	public:
		const string	name() const { return "StretchBlitBlends"; }
		bool			init(GfxDevice * pDevice, const RectI& canvas)
		{
			m_pMyCanvas = pDevice->surfaceFactory()->createSurface(canvas, PixelFormat::ARGB_8);

			m_pBlitSource = FileUtil::loadSurface("resources/splash.png", pDevice->surfaceFactory());
			return(m_pMyCanvas != nullptr && m_pBlitSource != nullptr);
		}

		bool			run(GfxDevice * pDevice, const RectI& canvas)
		{

//			m_pMyCanvas->fill(Color::Transparent);

//			pDevice->fill(canvas.size(), Color(0, 255, 0));
//			Surface_p pOldCanvas = pDevice->canvas();

//			pDevice->setCanvas(m_pMyCanvas);

			pDevice->stretchBlit(RectI(50, 50, 256, 128), m_pBlitSource);
			pDevice->stretchBlit(RectI(0, 0, 128, 64), m_pBlitSource);
			pDevice->stretchBlit(RectI(0, 256, 400, 140), m_pBlitSource);

//			pDevice->setCanvas(pOldCanvas);
//			pDevice->clipBlit(canvas, m_pMyCanvas);

			return true;
		}

	protected:

		Surface_p	m_pMyCanvas;
		Surface_p	m_pBlitSource;
	};




	class DrawTest : public TestUnit
	{
	public:

		bool			run(GfxDevice * pDevice, const RectI& canvas)
		{

			m_pMyCanvas->fill(Color::Transparent);

			Surface_p pOldCanvas = pDevice->canvas();
			pDevice->setCanvas(m_pMyCanvas);

			pDevice->drawLine(canvas.pos() + CoordI(10, 10), canvas.pos() + CoordI(canvas.size().w, canvas.size().h) - CoordI(10, 20), Color::Red, 3.f);
			pDevice->drawLine(canvas.pos() + CoordI(10, 20), canvas.pos() + CoordI(canvas.size().w, canvas.size().h) - CoordI(10, 10), Color(0, 0, 255, 128), 3.f);

			CoordI	fillOfs = { canvas.x, canvas.y + canvas.h / 2 };
			SizeI	fillSize = { 50,50 };
			CoordI	stepping = { 60, 0 };

			pDevice->fill({ fillOfs, fillSize }, Color::Red);
			pDevice->fill({ fillOfs + stepping, fillSize }, Color(0, 0, 255, 128));
			pDevice->fill({ fillOfs + stepping * 2, fillSize }, Color(0, 0, 255, 64));
			pDevice->fill({ fillOfs + stepping * 3, fillSize }, Color(0, 0, 255, 32));
			pDevice->fill({ fillOfs + stepping * 4, fillSize }, Color(0, 0, 255, 16));


			pDevice->setCanvas(pOldCanvas);
			pDevice->blit({ 0,0 }, m_pMyCanvas);

			return true;
		}

	protected:

		Surface_p	m_pMyCanvas;
	};




	class DrawToBGR_8 : public DrawTest
	{
	public:
		const string	name() const { return "DrawToBGR_8"; }
		bool			init(GfxDevice * pDevice, const RectI& canvas)
		{
			m_pMyCanvas = pDevice->surfaceFactory()->createSurface(canvas, PixelFormat::BGR_8);

			return(m_pMyCanvas != nullptr);
		}
	};

	class DrawToBGRA_8 : public DrawTest
	{
	public:
		const string	name() const { return "DrawToBGRA_8"; }
		bool			init(GfxDevice * pDevice, const RectI& canvas)
		{
			m_pMyCanvas = pDevice->surfaceFactory()->createSurface(canvas, PixelFormat::ARGB_8);

			return(m_pMyCanvas != nullptr);
		}
	};


	class DrawToBGRX_8 : public DrawTest
	{
	public:
		const string	name() const { return "DrawToBGRX_8"; }
		bool			init(GfxDevice * pDevice, const RectI& canvas)
		{
			m_pMyCanvas = pDevice->surfaceFactory()->createSurface(canvas, PixelFormat::BGRX_8);

			return(m_pMyCanvas != nullptr);
		}
	};


	class DrawToBGR_565 : public DrawTest
	{
	public:
		const string	name() const { return "DrawToBGR_565"; }
		bool			init(GfxDevice * pDevice, const RectI& canvas)
		{
			m_pMyCanvas = pDevice->surfaceFactory()->createSurface(canvas, PixelFormat::BGR_565);

			return(m_pMyCanvas != nullptr);
		}
	};


	class RotScaleBlend : public TestUnit
	{
	public:
		const string	name() const { return "RotScaleBlend"; }
		bool			init(GfxDevice * pDevice, const RectI& canvas)
		{
			m_pSource = FileUtil::loadSurface("resources/clockface_2500.png", pDevice->surfaceFactory());

			if (m_pSource)
			{
				m_pSource->setSampleMethod(SampleMethod::Bilinear);
				return true;
			}

			return false;
		}

		bool			run(GfxDevice * pDevice, const RectI& canvas)
		{
			CoordF center = { m_pSource->size().w / 2.f, m_pSource->size().h / 2.f };

			pDevice->rotScaleBlit(canvas, m_pSource, center, rot, scale);

/*
			rot += 0.1;
			scale += scaleInc;

			if (scale > 2 || scale < 0.3)
				scaleInc = -scaleInc;
*/
			return true;
		}

	protected:

		float rot = 45;
		float scale = 1.f;
		float scaleInc = 0.001f;

		Surface_p	m_pSource;
	};

}
