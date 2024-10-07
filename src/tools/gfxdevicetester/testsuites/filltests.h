#include "testsuite.h"

class FillTests : public TestSuite
{
public:
	FillTests()
	{
		name = "FillTests";

		addTest("StraightFill", &FillTests::straightFill );
		addTest("BlendFill", &FillTests::blendFill );
		addTest("TintFill", &FillTests::tintFill);
		addTest("GradientXFill", &FillTests::gradientXFill);
		addTest("GradientYFill", &FillTests::gradientYFill);
		addTest("GradientXYFill", &FillTests::gradientXYFill);
		addTest("NonGradientFill", &FillTests::nonGradientFill);
		addTest("SubpixelFill", &FillTests::subPixelFill);
		addTest("SubpixelGradientXFill", &FillTests::subPixelGradientXFill);
	}




	bool straightFill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(canvas, Color::RosyBrown);
		pDevice->setBlendMode(BlendMode::Blend);

		return true;
	}

	bool blendFill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTintColor( Color(255,255,255,128) );

		pDevice->fill(canvas, Color::RosyBrown);

		pDevice->setTintColor(Color::White);

		return true;
	}

	bool tintFill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTintColor(Color(255,0,0,255));
		pDevice->fill( RectSPX(0,0,32,32)*64 + canvas.pos(), Color::White);

		pDevice->setTintColor(Color(0,255,0,255));
		pDevice->fill(RectSPX(40, 0, 32, 32)*64 + canvas.pos(), Color::White);

		pDevice->setTintColor(Color(0,0,255,255));
		pDevice->fill(RectSPX(80, 0, 32, 32)*64 + canvas.pos(), Color::White);

		pDevice->setTintColor(Color::White);

		return true;
	}

	bool nonGradientFill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->fill(RectSPX(0, 0, 64, 64)*64 + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(64, 0, 256, 80)*64 + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(canvas.w - 256*64, canvas.h - 256*64, 256*64, 256*64) + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(0, canvas.h - 350*64, 250*64, 350*64) + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(canvas.w - 100*64, 0*64, 100*64, 240*64) + canvas.pos(), Color::White);

		return true;
	}

	bool gradientXFill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTintGradient(canvas,  Gradient(Placement::West, Color(255,0,0,255), Color(0,0,255,255)));
		pDevice->fill(RectSPX(0, 0, 64, 64)*64 + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(64, 0, 256, 80)*64 + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(canvas.w-256*64, canvas.h-256*64, 256*64, 256*64) + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(0, canvas.h - 350*64, 250*64, 350*64) + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(canvas.w-100*64, 0*64, 100*64, 240*64) + canvas.pos(), Color::White);

		pDevice->clearTintGradient();

		return true;
	}

	bool gradientYFill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTintGradient(canvas, Gradient(Placement::North, Color(255,0,0,255), Color(0,0,255,255)));
		pDevice->fill(RectSPX(0, 0, 64, 64) * 64 + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(64, 0, 256, 80) * 64 + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(canvas.w - 256 * 64, canvas.h - 256 * 64, 256 * 64, 256 * 64) + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(0, canvas.h - 350 * 64, 250 * 64, 350 * 64) + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(canvas.w - 100 * 64, 0 * 64, 100 * 64, 240 * 64) + canvas.pos(), Color::White);

		pDevice->clearTintGradient();

		return true;
	}

	bool gradientXYFill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTintGradient(canvas, Gradient(Placement::NorthWest, Color(255,0,0,255), Color(0,0,255,255)));
		pDevice->fill(RectSPX(0, 0, 64, 64) * 64 + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(64, 0, 256, 80) * 64 + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(canvas.w - 256 * 64, canvas.h - 256 * 64, 256 * 64, 256 * 64) + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(0, canvas.h - 350 * 64, 250 * 64, 350 * 64) + canvas.pos(), Color::White);
		pDevice->fill(RectSPX(canvas.w - 100 * 64, 0 * 64, 100 * 64, 240 * 64) + canvas.pos(), Color::White);

		pDevice->clearTintGradient();

		return true;
	}



	bool subPixelFill(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < 5; i++)
		{
			RectF rect = { 10.f + i * 25.f, 10.f + i * 0.25f, 20.f, 10.f };

			for (int j = 0; j < 5; j++)
			{
				pDevice->fill(RectSPX(rect*64), Color::White);
				rect.y += 12.f;
				rect.x += 0.25f;
			}
		}

		return true;
	}

	bool subPixelGradientXFill(GfxDevice* pDevice, const RectSPX& canvas)
	{
		pDevice->setTintGradient(canvas, Gradient(Placement::West, Color(255,0,0,255), Color(0,0,255,255)));

		for (int i = 0; i < 10; i++)
		{
			RectF rect = { 10.f + i * 25.f, 10.f + i * 0.25f, 20.f, 10.f };

			for (int j = 0; j < 10; j++)
			{
				pDevice->fill(RectSPX(rect*64), Color::White);
				rect.y += 12.f;
				rect.x += 0.25f;
			}
		}

		pDevice->clearTintGradient();

		return true;
	}




};
