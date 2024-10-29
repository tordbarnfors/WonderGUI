
#include <wondergui.h>
#include <wonderplugin.h>
#include <wg_freetypefont.h>

#include <string>
#include <fstream>

using namespace wg;
using namespace std;


#ifdef WIN32
#define	DLLEXPORTPREFIX __declspec(dllexport)
#else
#define	DLLEXPORTPREFIX __attribute__ ((visibility("default")))
#endif

extern "C" {
	DLLEXPORTPREFIX int		init( wg_plugin_interface * pInterface, void * pRealHostBridge,
								  wg_obj hPluginCapsule, wg_obj hGfxDevice, wg_obj hSurfaceFactory, wg_obj hEdgemapFactory );
	DLLEXPORTPREFIX int		update(void);
	DLLEXPORTPREFIX void	exitX(void);

	DLLEXPORTPREFIX void	mapInputKey(uint32_t native_keycode, wg_key translated_keycode);
	DLLEXPORTPREFIX void	mapInputCommand(uint32_t native_keycode, wg_modkeys modkeys, wg_editCommand command);

}

bool areaChartTest(ComponentPtr<DynamicSlot> pEntry);
bool canvasCapsuleGlowTest(ComponentPtr<DynamicSlot> pEntry);


Blob_p loadBlob(const std::string& path);

TextStyle_p		g_pTextStyle;

PluginRoot_p 	g_pPluginRoot;

//____ init() _________________________________________________________________

DLLEXPORTPREFIX int init( wg_plugin_interface * pInterface, void * pRealHostBridge, wg_obj hPluginCapsule, wg_obj hGfxDevice, wg_obj hSurfaceFactory, wg_obj hEdgemapFactory )
{
	PluginBase::init(pInterface, pRealHostBridge);
	
	auto pSurfaceFactory 	= PluginSurfaceFactory::create(hSurfaceFactory);
	auto pEdgemapFactory	= PluginEdgemapFactory::create(hEdgemapFactory);
	auto pGfxDevice 		= PluginGfxDevice::create(hGfxDevice, pSurfaceFactory, pEdgemapFactory);
	
	Base::setDefaultSurfaceFactory(pSurfaceFactory);
	Base::setDefaultEdgemapFactory(pEdgemapFactory);
	Base::setDefaultGfxDevice(pGfxDevice);
	
	
	auto pPluginRoot = PluginRoot::create(hPluginCapsule, Base::context());

	auto pFontBlob = loadBlob("resources/DroidSans.ttf");
	auto pFont = FreeTypeFont::create(pFontBlob);

	g_pTextStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 14,
									_.color = Color8::Black,
									_.states = {{State::Disabled, Color8::DarkGrey}
	} ));

	Base::setDefaultStyle(g_pTextStyle);

	
	
	
	
	auto pMainPanel = PackPanel::create();
	pMainPanel->setAxis(Axis::Y);
	pMainPanel->setSkin(ColorSkin::create({ .color = Color::DarkGray, .padding = 4 }));

	auto pLayout = PackLayout::create({
		.expandFactor = PackLayout::Factor::Weight
		});

	pMainPanel->setLayout(pLayout);

	auto pSkin1 = ColorSkin::create({ .states = { {State::Default, Color::Red}, {State::Hovered, Color::Pink}, {State::Pressed, Color::DarkRed} }  });
	auto pSkin2 = ColorSkin::create({ .states = { {State::Default, Color::Green}, {State::Hovered, Color::LightGreen}, {State::Pressed, Color::DarkGreen} }  });
	auto pSkin3 = ColorSkin::create({ .states = { {State::Default, Color::Blue}, {State::Hovered, Color::LightBlue}, {State::Pressed, Color::DarkBlue} }  });

	
	auto pFiller1 = TextEditor::create({ .skin = pSkin1 });
	auto pFiller2 = TextEditor::create({ .skin = pSkin2 });
	auto pFiller3 = SizeCapsule::create();

	pMainPanel->slots << pFiller1;
	pMainPanel->slots << pFiller2;
	pMainPanel->slots << pFiller3;

	pPluginRoot->slot = pMainPanel;

//	areaChartTest( {pFiller3, pFiller3->slot} );
	canvasCapsuleGlowTest({pFiller3, pFiller3->slot});

	
	g_pPluginRoot = pPluginRoot;
	
	
	
	return 1;
}

//____ update() _______________________________________________________________

DLLEXPORTPREFIX int update(void)
{
	return 1;
}

//____ exit() _________________________________________________________________

DLLEXPORTPREFIX void exitX(void)
{
	g_pTextStyle = nullptr;
	g_pPluginRoot = nullptr;
	PluginBase::exit();
}

//____ mapInputKey() _______________________________________________________

DLLEXPORTPREFIX void mapInputKey(uint32_t native_keycode, wg_key translated_keycode)
{
	Base::inputHandler()->mapKey(native_keycode, (Key) translated_keycode);
}

//____ mapInputCommand() ___________________________________________________

DLLEXPORTPREFIX void mapInputCommand(uint32_t native_keycode, wg_modkeys modkeys, wg_editCommand command)
{
	Base::inputHandler()->mapCommand(native_keycode, (ModKeys) modkeys, (EditCmd) command);
}


//____ loadBlob() _________________________________________________________

Blob_p loadBlob(const std::string& path)
{
	FILE* fp;

#ifdef WIN32
	errno_t err = fopen_s(&fp, path.c_str(), "rb");
#else
	fp = fopen(path.c_str(), "rb");
#endif
	if (!fp)
		return 0;

	fseek(fp, 0, SEEK_END);
	int size = (int)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	Blob_p pBlob = Blob::create(size);

	int nRead = (int)fread(pBlob->data(), 1, size, fp);
	fclose(fp);

	if (nRead < size)
		return 0;

	return pBlob;
}

//____ areaChartTest() ______________________________________________________

bool areaChartTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pFlex = FlexPanel::create();

	pFlex->setSkin(ColorSkin::create(Color::LightYellow));

	auto pGraph = AreaChart::create(WGBP(AreaChart,
		_.displayCeiling = 0.5f,
		_.displayFloor = - 0.5f,
		_.displaySkin = BoxSkin::create( WGBP(BoxSkin,
												_.color = Color::Black,
												_.outlineColor = Color::Green,
												_.padding = 2,
												_.outlineThickness = 2 )),
		_.skin = ColorSkin::create(Color::Pink)
	));




	pFlex->slots.pushBack(pGraph, { .pos = {10,10}, .size = {200,200} });

	*pEntry = pFlex;


	pGraph->entries.pushBack( {
		.bottomOutlineThickness = 0,
		.outlineColor = Color::Red,
		.topOutlineThickness = 5
		});


	static float topSamples[5] = { 0, -0.25f, 0.25f, 0.23f, 0.5f };

	static float topSamples2[5] = { 0, 0.25f, -0.25f, -0.23f, -0.5f };
	static float bottomSamples2[1] = { 0.f };


	pGraph->entries.back().setTopSamples(5, topSamples);

	auto pTransition = ValueTransition::create(2000000, TransitionCurve::Bezier);

	pGraph->entries.back().transitionSamples(pTransition, 5, topSamples2, 1, bottomSamples2);


	static float curveSamples[6][201];

	static float beg[1] = { 0.5f };
	static float end[1] = { -0.5f };

	auto pLinear = ValueTransition::create(201, TransitionCurve::Linear);
	auto pEaseIn = ValueTransition::create(201, TransitionCurve::EaseIn);
	auto pEaseOut = ValueTransition::create(201, TransitionCurve::EaseOut);
	auto pEaseInOut = ValueTransition::create(201, TransitionCurve::EaseInOut);
	auto pBezier = ValueTransition::create(201, TransitionCurve::Bezier);
	auto pParametric = ValueTransition::create(201, TransitionCurve::Parametric);

	for (int i = 0 ; i < 201 ; i++ )
	{
		pLinear->snapshot(i, 1, beg, end, &curveSamples[0][i]);
		pEaseIn->snapshot(i, 1, beg, end, &curveSamples[1][i]);
		pEaseOut->snapshot(i, 1, beg, end, &curveSamples[2][i]);
		pEaseInOut->snapshot(i, 1, beg, end, &curveSamples[3][i]);
		pBezier->snapshot(i, 1, beg, end, &curveSamples[4][i]);
		pParametric->snapshot(i, 1, beg, end, &curveSamples[5][i]);
	}


	Color colors[6] = { Color::Red, Color::Green, Color::Blue, Color::Yellow, Color::Pink, Color::Brown };

	for (int i = 0; i < 6; i++)
	{
		pGraph->entries.pushBack({
			.bottomOutlineThickness = 0,
			.color = Color::Transparent,
			.outlineColor = colors[i],
			.topOutlineThickness = 2,
			});

		pGraph->entries.back().setTopSamples(201, curveSamples[i] );
	}

	// Setup grid

	pGraph->xLines.pushBack({ .label = "-0.5", .pos = -0.5f, .thickness = 0.5f });
	pGraph->xLines.pushBack({ .label = "-0.25", .pos = -0.25f, .thickness = 0.5f });
	pGraph->xLines.pushBack({ .label = "0.0", .pos = 0.0f, .thickness = 1.f });
	pGraph->xLines.pushBack({ .label = "0.25", .pos = 0.25f, .thickness = 0.5f });

	pGraph->yLines.pushBack({ .label = "0.0", .pos = 0.0f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "0.25", .pos = 0.25f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "0.5", .pos = 0.5f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "1.0", .pos = 1.f, .thickness = 0.5f });

/*
	pGraph->xLines.pushBack(WGBP(GridLine,
							_.value = -0.25f
							));
*/

	//

	pGraph->glow.setActive(true);


	return true;
}

//____ canvasCapsuleGlowTest() ______________________________________________________

bool canvasCapsuleGlowTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBack = FlexPanel::create({ .skin = ColorSkin::create(Color::Black) });
	*pEntry = pBack;

	auto pMyStyle = TextStyle::create( WGOVR(Base::defaultStyle()->blueprint(), _.size = 30, _.color = Color::White ));

	auto pSimpleButtonSkin = ColorSkin::create( Color::Blue );
	

	auto pGlowCapsule = CanvasCapsule::create();

	auto pBlurbrush = Blurbrush::create({
			.blue = { 0.1f, 0.1f, 0.1f,
					 0.1f, 0.1f, 0.1f,
					 0.1f, 0.1f, 0.1f },
			.green = { 0.1f, 0.1f, 0.1f,
					 0.1f, 0.1f, 0.1f,
					 0.1f, 0.1f, 0.1f },
			.red = { 0.1f, 0.1f, 0.1f,
					 0.1f, 0.15f, 0.1f,
					 0.1f, 0.1f, 0.1f },
		});

	//	pGlowCapsule->setResolution( {64,64} );

	pGlowCapsule->glow.setBlurbrush(pBlurbrush);
	pGlowCapsule->glow.setRefreshRate(30);
	pGlowCapsule->glow.setActive(true);


	pGlowCapsule->setSkin(BoxSkin::create({ .color = HiColor::Transparent, .outlineColor = Color::Yellow, .outlineThickness = 50, .padding = 50 }));

	pBack->slots.pushBack(pGlowCapsule, { .pos = {  50, 50}, .size = {300,0} });


	auto pWrapTextLayout = BasicTextLayout::create({ .wrap = true });


	auto pContent = TextEditor::create({ .editor = {.layout = pWrapTextLayout, .style = pMyStyle, .text = "HEJ" } });
	pGlowCapsule->slot = pContent;
//	pBack->slots.pushBack( pContent );

	auto pTintWhite = Button::create({ .label = {.text = "WHITE" }, .skin = pSimpleButtonSkin });
	auto pTintBlack = Button::create({ .label = {.text = "BLACK" }, .skin = pSimpleButtonSkin });

	auto pTransition = ColorTransition::create(1000*1000, TransitionCurve::EaseOut);

	auto pGradientWhite = Button::create({ .label = {.text = "GRADIENT WHITE" }, .skin = pSimpleButtonSkin });
	auto pGradientBlack = Button::create({ .label = {.text = "GRADIENT BLACK" }, .skin = pSimpleButtonSkin });

	Base::msgRouter()->addRoute(pTintWhite, MsgType::Select, [pGlowCapsule,pTransition](Msg* pMsg) {pGlowCapsule->setTintColor(Color::White, pTransition); });
	Base::msgRouter()->addRoute(pTintBlack, MsgType::Select, [pGlowCapsule, pTransition](Msg* pMsg) {pGlowCapsule->setTintColor(Color::Black, pTransition); });

	Base::msgRouter()->addRoute(pGradientWhite, MsgType::Select, [pGlowCapsule, pTransition](Msg* pMsg) {pGlowCapsule->setTintGradient(Gradient( Placement::NorthWest, HiColor::White, HiColor::Black), pTransition); });
	Base::msgRouter()->addRoute(pGradientBlack, MsgType::Select, [pGlowCapsule, pTransition](Msg* pMsg) {pGlowCapsule->setTintGradient(Gradient(Placement::SouthEast, HiColor::White, HiColor::Black), pTransition); });


	auto pButtons = PackPanel::create({ .axis = Axis::X });

	pButtons->slots << pTintWhite << pTintBlack << pGradientWhite << pGradientBlack;

	pBack->slots.pushBack( pButtons, { .origo = Placement::SouthWest } );

	return true;
}
