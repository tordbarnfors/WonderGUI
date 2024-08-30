#include "testsuite.h"

#include <wg_canvaslayers.h>
//#include <wg_plugincanvaslayers.h>

class CanvasLayerTests : public TestSuite
{
public:
	CanvasLayerTests()
	{
		name = "CanvasLayerTests";

		addTest("CanvasLayerTest1", &CanvasLayerTests::prep1, &CanvasLayerTests::test1, &CanvasLayerTests::cleanup);
//		addTest("BlendModeUndefined", &BlitBlendTests::setUndefined, &BlitBlendTests::blit, &BlitBlendTests::cleanup);
	}


	bool init(GfxDevice * pDevice, const RectI& canvas, WonderApp::Visitor * pAppVisitor)
	{
		m_pOne = pAppVisitor->loadSurface("resources/one.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Nearest } );
		m_pTwo = pAppVisitor->loadSurface("resources/two.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Nearest } );
		m_pThree = pAppVisitor->loadSurface("resources/three.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Nearest } );
		if (!m_pOne || !m_pTwo || !m_pThree)
			return false;

		m_pBackground = pAppVisitor->loadSurface("resources/What-Goes-Up-3.bmp", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Nearest } );
		if (!m_pBackground)
			return false;

		m_pCanvas = pDevice->surfaceFactory()->createSurface( WGBP(Surface,
																   _.size = canvas/64,
																   _.format = PixelFormat::BGRA_8,
																   _.canvas = true
																   ));

		CanvasLayers::Blueprint bp = WGBP(CanvasLayers,
			_.baseLayer = 2,
			_.layers = {{ .format = PixelFormat::Alpha_8, .blendFunc = [](GfxDevice* pDevice) { pDevice->setBlendMode(BlendMode::Subtract); pDevice->blit({0,0}); }},
						{ .format = PixelFormat::BGRA_8,  .blendFunc = [](GfxDevice* pDevice) { pDevice->setBlendMode(BlendMode::Blend); pDevice->blit({0,0}); }},
						{ .format = PixelFormat::Alpha_8, .blendFunc = [](GfxDevice* pDevice) { pDevice->setBlendMode(BlendMode::Add);  pDevice->blit({0,0}); }},
						{ .format = PixelFormat::BGRA_8,  .blendFunc = [](GfxDevice* pDevice) { pDevice->setBlendMode(BlendMode::Blend);  pDevice->blit({0,0}); }}
			});

/*		if (pDevice->isInstanceOf(PluginGfxDevice::TYPEINFO))
		{
			m_pLayerStack1 = PluginCanvasLayers::create(static_cast<PluginGfxDevice*>(pDevice), bp);
		}
		else
*/		{
			m_pLayerStack1 = CanvasLayers::create(bp);
		}
		
		return true;
	}

	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pCanvas = nullptr;
		m_pLayerStack1 = nullptr;
		m_pOne = nullptr;
		m_pTwo = nullptr;
		m_pThree = nullptr;
		m_pBackground = nullptr;
		return true;
	}


	bool cleanup(GfxDevice * pDevice, const RectI& canvas)
	{
		return true;
	}

	bool prep1(GfxDevice * pDevice, const RectI& canvas)
	{
		return true;
	}

	
	bool test1(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->beginCanvasUpdate(m_pCanvas, 0, nullptr, m_pLayerStack1);
		pDevice->setRenderLayer(0);

		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(m_pBackground);
		pDevice->blit({ 0,0 });
		pDevice->setBlendMode(BlendMode::Blend);

		pDevice->setRenderLayer(1);
		pDevice->setBlitSource(m_pOne);
		pDevice->setTintColor(HiColor(1.f, 1.f, 1.f,0.25f) );
		pDevice->stretchBlit(canvas);

		pDevice->setRenderLayer(2);
		pDevice->setBlitSource(m_pTwo);
		pDevice->setTintColor(HiColor(0.5f, 0.5f, 0.5f,1.f) );
		pDevice->blit( {canvas.w/2,canvas.h/2} );

		pDevice->setRenderLayer(3);
		pDevice->setTintColor(HiColor::White);
		pDevice->fill( {0,0,canvas.w/2+m_pTwo->pixelWidth()*64/2,canvas.h}, HiColor(4096,4096,4096,1024) );
		
		pDevice->setRenderLayer(4);
		pDevice->setBlitSource(m_pThree);
		pDevice->setTintColor(Color8::Blue);
		pDevice->blit( {canvas.w/2,0 } );

		pDevice->endCanvasUpdate();

		pDevice->setBlitSource(m_pCanvas);
		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->blit({ 0,0 });
		pDevice->setBlendMode(BlendMode::Blend);

		return true;
	}


	Surface_p	m_pCanvas;
	Surface_p	m_pBackground;
	Surface_p	m_pOne, m_pTwo, m_pThree;
	CanvasLayers_p	m_pLayerStack1;

};
