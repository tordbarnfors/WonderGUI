#include <string.h>

#include "softgfxdevice.h"

#include <wondergui.h>
#include <wg_gfxdevice_gen2.h>
#include <wg_softbackend.h>

#include <wg_string.h>

using namespace wg;

SoftGfxDeviceTest::SoftGfxDeviceTest()
{

	ADD_TEST(drawRingTest);

}

SoftGfxDeviceTest::~SoftGfxDeviceTest()
{
}


bool SoftGfxDeviceTest::init(std::ostream& output)
{
	return true;
}


bool SoftGfxDeviceTest::drawRingTest(std::ostream& output)
{

	SoftSurface_p pCanvas =	SoftSurface::create( { .canvas = true, .size = {4096,4096} } );

	auto pBackend = SoftBackend::create();
	GfxDevice_p pDevice = GfxDeviceGen2::create(pBackend);
	
	pDevice->beginRender();
	
	pDevice->beginCanvasUpdate(pCanvas);

	RingSlice slice = { 1.f, HiColor( 4096, 0, 0, 4096), nullptr };

	RingSpec ring;
	ring.rect = {59,59,1288,1288};
	ring.thickness = 10*64+3;
	ring.nSlices = 1;
	ring.pSlices = &slice;
	pDevice->drawRing(ring);

	
	pDevice->endCanvasUpdate();
	
	pDevice->endRender();

	
	

	return true;
}

