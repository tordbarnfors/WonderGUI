#include "testsuite.h"

class BlendTests : public TestSuite
{
public:
	BlendTests()
	{
		name = "BlendTests";

		addTest("BlendModeReplace", &BlendTests::setReplace, &BlendTests::draw, &BlendTests::cleanup);
		addTest("BlendModeBlend", &BlendTests::setBlend, &BlendTests::draw, &BlendTests::cleanup);
		addTest("BlendModeAdd", &BlendTests::setAdd, &BlendTests::draw, &BlendTests::cleanup);
		addTest("BlendModeSubtract", &BlendTests::setSubtract, &BlendTests::draw, &BlendTests::cleanup);
		addTest("BlendModeMultiply", &BlendTests::setMultiply, &BlendTests::draw, &BlendTests::cleanup);
		addTest("BlendModeMin", &BlendTests::setMin, &BlendTests::draw, &BlendTests::cleanup);
		addTest("BlendModeMax", &BlendTests::setMax, &BlendTests::draw, &BlendTests::cleanup);
		addTest("BlendModeInvert", &BlendTests::setInvert, &BlendTests::draw, &BlendTests::cleanup);
		addTest("BlendModeIgnore", &BlendTests::setIgnore, &BlendTests::draw, &BlendTests::cleanup);
//		addTest("BlendModeUndefined", &BlendTests::setUndefined, &BlendTests::draw, &BlendTests::cleanup);
	}


	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		return true;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		return true;
	}


	bool cleanup(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

	bool setReplace(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		return true;
	}

	bool setBlend(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Blend);
		return true;
	}

	bool setAdd(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Add);
		return true;
	}

	bool setSubtract(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Subtract);
		return true;
	}

	bool setMultiply(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Multiply);
		return true;
	}

	bool setMin(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Min);
		return true;
	}

	bool setMax(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Max);
		return true;
	}

	bool setInvert(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Invert);
		return true;
	}

	bool setIgnore(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Ignore);
		return true;
	}

	bool setUndefined(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setBlendMode(BlendMode::Undefined);
		return true;
	}

	// The fills are drawn onto stripes of their own, since on a black background Subtract,
	// Multiply and Min have nothing to darken and Add and Max give the same result. Channels
	// are not only 0 and 255 either, since Multiply and Min (or Add and Max) give the same
	// result for those. The last fill is semi-transparent, to show how alpha controls the
	// strength of the effect.

	bool draw(GfxDevice * pDevice, const RectSPX& canvas)
	{
		HiColor stripes[6] = { Color8(0, 0, 0), Color8(96, 96, 96), Color8(200, 200, 200),
							   Color8(255, 255, 255), Color8(220, 60, 40), Color8(40, 90, 210) };

		BlendMode mode = pDevice->blendMode();
		pDevice->setBlendMode(BlendMode::Replace);
		for (int i = 0; i < 6; i++)
			pDevice->fill( RectSPX(80 + i * 35, 80, 35, 215)*64, stripes[i]);
		pDevice->setBlendMode(mode);

		pDevice->fill( RectSPX(100, 100, 100, 100)*64, Color8(255, 128, 64, 255));
		pDevice->fill( RectSPX(150, 100, 100, 100)*64, Color8(64, 255, 128, 255));
		pDevice->fill( RectSPX(100, 150, 100, 100)*64, Color8(128, 64, 255, 255));
		pDevice->fill( RectSPX(175, 175, 100, 100)*64, Color8(192, 160, 32, 128));
		return true;
	}

};
