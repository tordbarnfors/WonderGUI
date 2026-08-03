
#include "device.h"
#include "gfxdevicetester.h"


Device::Device( const string& name, GfxDevice * pDevice, CanvasRef canvasRef, Surface * pSurface )
  : m_name(name),
	m_pDevice(pDevice),
	m_pCanvas(pSurface),
	m_canvasRef(canvasRef)
{
	setSkin( wkit::Skins::Plate );

	auto pTitleBar = PackPanel::create();
	pTitleBar->setAxis(Axis::X);
	pTitleBar->setLayout(PackLayout::create( WGBP(PackLayout,
												  _.expandFactor = PackLayout::Factor::Weight )));
	

	auto pTesteeButton = WGCREATE( wkit::Button, _.label.text = "TEST");

	auto pDiffButton = WGCREATE( wkit::Button, _.label.text = "DIFF");

	auto pRefButton = WGCREATE( wkit::Button, _.label.text = "REF");

	
	auto pTitle = WGCREATE( TextDisplay, _.display.text = name );

	auto pCloseButton = WGCREATE( wkit::Button, _.label.text = "CLOSE");


	
	pTitleBar->slots << pTesteeButton;
	pTitleBar->slots << pDiffButton;
	pTitleBar->slots << pRefButton;

	pTitleBar->slots << pTitle;

	pTitleBar->slots << pCloseButton;

	pTitleBar->setSlotWeight(0, 5, {0.f,0.f,0.f,1.f,0.f});
	

	SizeI canvasSize = pSurface ? pSurface->pixelSize() : pDevice->canvas(CanvasRef::Default).size/64;
	
	
	m_pDisplaySurface = Base::defaultSurfaceFactory()->createSurface(WGBP(Surface, _.size = canvasSize ));
	
	
	m_pDisplay = SurfaceDisplay::create( WGBP(SurfaceDisplay,
											  _.skin = BoxSkin::create(WGBP(BoxSkin,
																			_.color = Color::Black,
																			_.outlineThickness = 2,
																			_.outlineColor = Color::White,
																			_.padding = 2 )),
											  _.surface = m_pDisplaySurface
											  )
										);
	
	setAxis(Axis::Y);
	slots << pTitleBar;
	slots << m_pDisplay;
};



GfxDevice_p Device::beginRender()
{
    m_pDevice->beginRender();
	if( m_canvasRef != CanvasRef::None )
		m_pDevice->beginCanvasUpdate(m_canvasRef);
	else
		m_pDevice->beginCanvasUpdate(m_pCanvas);

	return m_pDevice;
}


void Device::endRender()
{
    m_pDevice->endCanvasUpdate();
    m_pDevice->endRender();

	if( m_pCanvas )
		m_pDisplaySurface->copy( { 0,0 }, m_pCanvas );
	m_bNeedsRedraw = false;
}

