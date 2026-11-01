#include "edgemapwave.h"

#include <wondergui.h>
#include <wax_device_gen2.h>
#include <wax_softbackend.h>
#include <wax_softsurface.h>
#include <wax_softedgemapfactory.h>
#include <wax_softkernels_default.h>
#include <wax_edgemapwave.h>

#include <cmath>
#include <cstring>
#include <functional>
#include <vector>

using namespace wg;
using namespace wax;

// EdgemapWave changing its edgemap only in generate(), and in place only when nothing else holds
// it, since draws are rendered when the canvas update ends.

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

	const int c_fillSegment = 2;					// Transparent, outline, fill, outline, transparent.

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
// Settings reach the edgemap in generate(). It is updated in place when only the wave holds it and
// replaced when held elsewhere or when the size has changed.

bool EdgemapWaveTest::heldEdgemapTest(std::ostream& output)
{
	const HiColor red(4096, 0, 0, 4096), blue(0, 0, 4096, 4096), green(0, 4096, 0, 4096);

	auto pWave = makeWave(red);
	setSamples(pWave, 0.2f);
	pWave->generate();

	Edgemap* pFirst = pWave->edgemap().rawPtr();
	TEST_ASSERT(pFirst->flatColors()[c_fillSegment] == red);

	// In place.

	setSamples(pWave, 0.3f);
	pWave->generate();
	TEST_ASSERT(pWave->edgemap().rawPtr() == pFirst);

	pWave->setColor(blue);
	TEST_ASSERT(pFirst->flatColors()[c_fillSegment] == red);					// Not until generate().
	pWave->generate();
	TEST_ASSERT(pWave->edgemap().rawPtr() == pFirst && pFirst->flatColors()[c_fillSegment] == blue);

	// Held elsewhere, by samples...

	Edgemap_p pHeld = pWave->edgemap();

	setSamples(pWave, 0.4f);
	pWave->generate();
	TEST_ASSERT(pWave->edgemap() != pHeld);

	// ...and by colors, which the held edgemap keeps.

	pHeld = pWave->edgemap();
	pWave->setColor(green);
	TEST_ASSERT(pWave->edgemap() == pHeld);
	pWave->generate();
	TEST_ASSERT(pWave->edgemap() != pHeld);
	TEST_ASSERT(pHeld->flatColors()[c_fillSegment] == blue && pWave->edgemap()->flatColors()[c_fillSegment] == green);

	// Nothing changed, nothing replaced.

	pHeld = pWave->edgemap();
	pWave->generate();
	TEST_ASSERT(pWave->edgemap() == pHeld);
	pHeld = nullptr;

	// A new size takes a new edgemap, also when not held.

	Edgemap* pBefore = pWave->edgemap().rawPtr();
	TEST_ASSERT(pWave->setSize({ 50 * 64, 30 * 64 }));
	TEST_ASSERT(pWave->edgemap().rawPtr() == pBefore);
	pWave->generate();
	TEST_ASSERT(pWave->edgemap().rawPtr() != pBefore && pWave->edgemap()->pixelSize() == SizeI(50, 30));
	return true;
}

//____ drawnTwiceTest() _______________________________________________________
// One EdgemapWave drawn several times within one canvas update, with other samples and colors in
// between, draws as separate waves. draw() generates, so a color set without generate() is drawn.

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
		pWave->generate();
		pWave->draw(pDevice, { 10 * 64, 80 * 64 });

		pWave->setColor(red);
		pWave->draw(pDevice, { 90 * 64, 80 * 64 });
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

		auto pFourth = makeWave(red);
		setSamples(pFourth, 0.45f);
		pFourth->draw(pDevice, { 90 * 64, 80 * 64 });
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
