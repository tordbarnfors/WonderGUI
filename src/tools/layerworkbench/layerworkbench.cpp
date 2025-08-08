
#include "layerworkbench.h"

#include <wondergui.h>

#include <wg_freetypefont.h>
#include <string>
#include <fstream>

using namespace wg;
using namespace std;

//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(Visitor* pVisitor)
{
	if (!setupGUI(pVisitor))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}

	
	
	
	return true;
}

//____ update() _______________________________________________________________

bool MyApp::update()
{
	return true;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{
}

//____ setupGUI() _____________________________________________________________

bool MyApp::setupGUI(Visitor* pVisitor)
{
	// Setup window
	
	auto pWindow = pVisitor->createWindow( {
		.title = "Layer Workbench"
	});
	
	m_pWindow = pWindow;

	auto pRoot = m_pWindow->rootPanel();
	
	auto pLayers = CanvasLayers::create({
		.baseLayer = 2,
		.layers = {
			{ .format = PixelFormat::Alpha_8,
				.blendFunc = [](GfxDevice* pDevice)
				{
					pDevice->setBlendMode(BlendMode::Subtract);
					pDevice->blit({0,0});
					pDevice->setBlendMode(BlendMode::Blend);
				},
				.clearFunc = [](GfxDevice* pDevice)
				{
					pDevice->setBlendMode(BlendMode::Replace);
					pDevice->fill(HiColor::Transparent);
					pDevice->setBlendMode(BlendMode::Blend);
				}
			},
			{ .format = PixelFormat::BGRA_8 },
			{ .format = PixelFormat::BGRA_8 }
		}
	});
	
	pRoot->setCanvasLayers(pLayers);
	
	pRoot->setSkin( ColorSkin::create({ .color = Color::Grey, .layer = 0} ));
	
	// Create skins
	
	auto pShadowSkin = ColorSkin::create( {
		.blendMode = BlendMode::Max,
		.color = HiColor{0,0,0,256},
		.layer = 1,
		.overflow = {0,20,20,0},
		.spacing = { 20,0,0,20 }
	});
	
	auto pGreenBox = BoxSkin::create( {
		.color = Color::Green,
		.outlineColor = Color::Black,
		.outlineThickness = 2
	});
	
	auto pRedBox = BoxSkin::create({
	.color = Color::Red,
	.outlineColor = Color::Black,
	.outlineThickness = 2
		});

	auto pBoxWithShadow = DoubleSkin::create({
		.skinInSkin = false,
		.skins = { pGreenBox, pShadowSkin }
	});

	auto pRedBoxWithShadow = DoubleSkin::create({
		.skinInSkin = false,
		.skins = { pRedBox, pShadowSkin }
		});


	auto pSplashSurface = pVisitor->loadSurface("resources/splash.png");



	auto pGlowSkin = BlockSkin::create({
		.color = HiColor{4096,4096,4096,2048},
		.layer = 3,
		.overflow = {50,50,50,50},
		.surface = pSplashSurface,
	});

	auto pBoxWithGlow = DoubleSkin::create({
		.skins = { pGlowSkin, pGreenBox }
		});

	
	// Setup layout
	
	auto pBaseFlex = FlexPanel::create();
	m_pWindow->setContent(pBaseFlex);

	pBaseFlex->slots << createMovableBox(pBoxWithGlow, pBaseFlex);
	pBaseFlex->slots << createMovableBox(pBoxWithShadow,pBaseFlex);
	pBaseFlex->slots << createMovableBox(pBoxWithShadow,pBaseFlex);
	pBaseFlex->slots << createMovableBox(pBoxWithShadow,pBaseFlex);

	// Create box with capsule

	auto pBox = Filler::create({ .defaultSize = {64,64}, .skin = pRedBoxWithShadow });

	pBaseFlex->slots << makeMovable(ScaleCapsule::create({ .child = pBox }), pBaseFlex);

	// Create Pack Panel with boxes

	auto pPack = PackPanel::create();

	pPack->slots << Filler::create({ .defaultSize = {64,64}, .skin = pBoxWithShadow });
	pPack->slots << Filler::create({ .defaultSize = {64,64}, .skin = pBoxWithGlow });
	pPack->slots << Filler::create({ .defaultSize = {64,64}, .skin = pBoxWithShadow });

	pBaseFlex->slots << makeMovable(pPack, pBaseFlex);

	pBaseFlex->slots.clear();
	
	// Create Split Panel

	auto pHandleSkin = BoxSkin::create( { 
		.color = Color::LightCoral,
		.outlineColor = Color::Black,
		.outlineThickness = 2,
		.padding = 8
	});
	
	auto pSplit = SplitPanel::create( { .axis = Axis::Y, .handleSkin = pHandleSkin });

	pSplit->slots[0] = Filler::create({ .defaultSize = {64,64}, .skin = pBoxWithGlow });
	pSplit->slots[1] = Filler::create({ .defaultSize = {64,64}, .skin = pBoxWithGlow });

	pBaseFlex->slots << makeMovable(pSplit, pBaseFlex);

	// Create ScrollPanel
	
	auto pScrollContent = PackPanel::create();

	pScrollContent->slots << Filler::create({ .defaultSize = {64,64}, .skin = pBoxWithShadow });
	pScrollContent->slots << Filler::create({ .defaultSize = {64,64}, .skin = pBoxWithGlow });
	pScrollContent->slots << Filler::create({ .defaultSize = {64,64}, .skin = pBoxWithShadow });
	
	
	auto pScroll = ScrollPanel::create({
		
		
	});
	
	pScroll->slot = pPack;
	
	pBaseFlex->slots << makeMovable(pScroll, pBaseFlex);

	pBaseFlex->slots.clear();
	
	// Create PackList
	
	auto pEntryOverflow = BoxSkin::create( {
		.color = HiColor::Transparent,
		.layer = 3,
		.outlineColor = HiColor::Transparent,
		.outlineThickness = 3,
		.overflow = 8,
		.states = {
			{State::Hovered, HiColor::Transparent, HiColor(0,4096,0,2048)},
			{State::Selected, HiColor::Transparent, HiColor(0,0,4096,2048)},
			{State::SelectedHovered, HiColor::Transparent, HiColor(0,4096,4096,2048)}
		}
	});
	
	auto pEntryMain1 = BoxSkin::create( {
		.color = Color::LightYellow,
		.outlineColor = Color::Black,
		.outlineThickness = 1,
		.padding = 4,
		.spacing = 2
	});

	auto pEntryMain2 = BoxSkin::create( {
		.color = Color::LightBlue,
		.outlineColor = Color::Black,
		.outlineThickness = 1,
		.padding = 4,
		.spacing = 2
	});

	
	auto pEntrySkin1 = DoubleSkin::create({
		.skinInSkin = false,
		.skins = { pEntryMain1, pEntryOverflow }
	});

	auto pEntrySkin2 = DoubleSkin::create({
		.skinInSkin = false,
		.skins = { pEntryMain2, pEntryOverflow }
	});

/*
	auto pList = PackList::create( { .entrySkin = pEntrySkin1, .entrySkin2 = pEntrySkin2, .skin = ColorSkin::create(Color::White) } );
	
	pList->slots << Filler::create({ .defaultSize = {100,40} });
	pList->slots << Filler::create({ .defaultSize = {100,40} });
	pList->slots << Filler::create({ .defaultSize = {100,40} });
	pList->slots << Filler::create({ .defaultSize = {100,40} });

	pBaseFlex->slots << makeMovable(pList, pBaseFlex);
*/

	
	return true;
}

//____ createMovableBox() _______________________________________________

Widget_p MyApp::createMovableBox( Skin * pSkin, FlexPanel * pParent )
{
	auto pBox = Filler::create({
		.defaultSize = {64,64},
		.skin = pSkin
	});
	
	return makeMovable(pBox, pParent);
}

//____ makeMovable() __________________________________________________________

Widget_p MyApp::makeMovable(const Widget_p& pWidget, FlexPanel* pParent)
{
	Base::msgRouter()->addRoute(pWidget, MsgType::MouseDrag, [pWidget, pParent](Msg* _pMsg)
		{
			auto pMsg = static_cast<MouseDragMsg*>(_pMsg);

			if( pMsg->button() != MouseButton::Right )
				return;
		
			auto it = pParent->slots.find(pWidget);

			it->move(pMsg->draggedNow());
		});

	return pWidget;
}
