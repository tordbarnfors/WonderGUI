#include <string.h>

#include "softgfxdevice.h"

#include <wondergui.h>
#include <wg_gfxdevice_gen2.h>
#include <wg_softbackend.h>

#include <wg_string.h>

using namespace wg;

SoftGfxDeviceTest::SoftGfxDeviceTest()
{

	ADD_TEST(drawElipseTest);

}

SoftGfxDeviceTest::~SoftGfxDeviceTest()
{
}


bool SoftGfxDeviceTest::init(std::ostream& output)
{
	return true;
}


bool SoftGfxDeviceTest::drawElipseTest(std::ostream& output)
{

	SoftSurface_p pCanvas =	SoftSurface::create( { .canvas = true, .size = {4096,4096} } );

	auto pBackend = SoftBackend::create();
	GfxDevice_p pDevice = GfxDeviceGen2::create(pBackend);
	
	pDevice->beginRender();
	
	pDevice->beginCanvasUpdate(pCanvas);

	pDevice->drawElipse({59,59,1288,1288}, 10*64+3, HiColor( 4096, 0, 0, 4096) );

	
/*
	for( int j = 0 ; j < 64 ; j++ )
	{
		for( int i = 0 ; i < 64*64 ; i++ )
		{
			for( int k = 0 ; k < 64*64 ; k++ )
				pDevice->drawElipse({j,j,i,k}, 10*64+3, HiColor( 4096, 0, 0, 4096) );
		}
	}
*/
	pDevice->endCanvasUpdate();
	
	pDevice->endRender();

	
	

	return true;
}

