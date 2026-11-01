#include "edgemapwave.h"

#include <wondergui.h>
#include <wg_gfxdevice_gen2.h>
#include <wg_softbackend.h>
#include <wg_softsurface.h>
#include <wg_softedgemapfactory.h>
#include <wg_softkernels_default.h>
#include <wg_edgemapwave.h>

#include <cmath>
#include <cstring>
#include <functional>
#include <vector>

using namespace wg;

// EdgemapWave updating its edgemap in place only when nothing else holds it, since draws are
// rendered when the canvas update ends.

namespace
{
	const SizeI c_canvasSize = { 160, 140 };
	const SizeSPX c_waveSize = { 60 * 64, 40 * 64 };
	const int c_nbSamples = 61;

	std::vector<uint8_t> render(const std::function<void(GfxDevice*)>& draw)
	{
		auto pBackend = SoftBackend::create();
		addDefaultSoftKernels(pBackend);
		auto pDevice = GfxDeviceGen2::create(pBackend);

		auto pCanvas = SoftSurface::create(WGBP(Surface, _.size = c_canvasSize, _.format = PixelFormat::ARGB_8, _.colorSpace = ColorSpace::Linear, _.canvas = true));

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(pCanvas);
		pDevice->fill(HiColor::Black);
		draw(pDevice);
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		std::vector<uint8_t> out(c_canvasSize.w * c_canvasSize.h * 4);
		auto buffer = pCanvas->allocPixelBuffer();
		pCanvas->pushPixels(buffer);
		for (int y = 0; y < c_canvasSize.h; y++)
			std::memcpy(out.data() + y * c_canvasSize.w * 4, buffer.pixels + y * buffer.pitch, c_canvasSize.w * 4);
		pCanvas->freePixelBuffer(buffer);
		return out;
	}

	EdgemapWave_p makeWave(HiColor color)
	{
		return EdgemapWave::create(WGBP(EdgemapWave, _.size = c_waveSize, _.color = color, _.outlineColor = HiColor::White,
										_.topOutlineThickness = 2 * 64, _.bottomOutlineThickness = 64,
										_.factory = SoftEdgemapFactory::create()));
	}

	// Top line a wave of the given frequency, bottom line flat near the bottom.

	void setSamples(EdgemapWave* pWave, float frequency)
	{
		std::vector<spx> top(c_nbSamples), bottom(c_nbSamples);
		for (int i = 0; i < c_nbSamples; i++)
		{
			top[i] = spx((14 + 8 * std::sin(i * frequency)) * 64);
			bottom[i] = 34 * 64;
		}
		pWave->setSamples(0, c_nbSamples, top.data(), bottom.data());
	}
}

//____ constructor ____________________________________________________________

EdgemapWaveTest::EdgemapWaveTest()
{
	ADD_TEST(heldEdgemapTest);
	ADD_TEST(drawnTwiceTest);
}

EdgemapWaveTest::~EdgemapWaveTest()
{
}

//____ heldEdgemapTest() ______________________________________________________
// The edgemap is updated in place when only the wave holds it, replaced when held elsewhere.

bool EdgemapWaveTest::heldEdgemapTest(std::ostream& output)
{
	auto pWave = makeWave(HiColor(4096, 0, 0, 4096));
	setSamples(pWave, 0.2f);
	pWave->generate();

	Edgemap* pFirst = pWave->edgemap().rawPtr();

	setSamples(pWave, 0.3f);
	pWave->generate();
	TEST_ASSERT(pWave->edgemap().rawPtr() == pFirst);

	pWave->setColor(HiColor(0, 0, 4096, 4096));
	TEST_ASSERT(pWave->edgemap().rawPtr() == pFirst);

	// Held elsewhere, by samples...

	Edgemap_p pHeld = pWave->edgemap();

	setSamples(pWave, 0.4f);
	pWave->generate();
	TEST_ASSERT(pWave->edgemap() != pHeld);

	// ...and by colors.

	pHeld = pWave->edgemap();
	pWave->setColor(HiColor(0, 4096, 0, 4096));
	TEST_ASSERT(pWave->edgemap() != pHeld);

	// Nothing changed, nothing replaced.

	pHeld = pWave->edgemap();
	pWave->generate();
	TEST_ASSERT(pWave->edgemap() == pHeld);
	return true;
}

//____ drawnTwiceTest() _______________________________________________________
// One EdgemapWave drawn three times within one canvas update, with other samples and colors in
// between, draws as three separate waves. The last one only has its color changed, which applies
// right away without generate().

bool EdgemapWaveTest::drawnTwiceTest(std::ostream& output)
{
	const HiColor red(4096, 0, 0, 4096), blue(0, 0, 4096, 4096), green(0, 4096, 0, 4096);

	auto one = render([&](GfxDevice* pDevice)
	{
		auto pWave = makeWave(red);
		setSamples(pWave, 0.2f);
		pWave->generate();
		pWave->draw(pDevice, { 10 * 64, 10 * 64 });

		setSamples(pWave, 0.45f);
		pWave->setColor(blue);
		pWave->generate();
		pWave->draw(pDevice, { 90 * 64, 10 * 64 });

		pWave->setColor(green);
		pWave->draw(pDevice, { 10 * 64, 80 * 64 });
	});

	auto separate = render([&](GfxDevice* pDevice)
	{
		auto pFirst = makeWave(red);
		setSamples(pFirst, 0.2f);
		pFirst->generate();
		pFirst->draw(pDevice, { 10 * 64, 10 * 64 });

		auto pSecond = makeWave(blue);
		setSamples(pSecond, 0.45f);
		pSecond->generate();
		pSecond->draw(pDevice, { 90 * 64, 10 * 64 });

		auto pThird = makeWave(green);
		setSamples(pThird, 0.45f);
		pThird->generate();
		pThird->draw(pDevice, { 10 * 64, 80 * 64 });
	});

	for (size_t i = 0; i < one.size(); i++)
	{
		if (one[i] != separate[i])
		{
			int pixel = int(i / 4);
			output << "Pixel " << pixel % c_canvasSize.w << "," << pixel / c_canvasSize.w << " channel " << i % 4 << " is " << int(one[i])
				   << " with one wave, " << int(separate[i]) << " with separate waves." << std::endl;
			return false;
		}
	}
	return true;
}
