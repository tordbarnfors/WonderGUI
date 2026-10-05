
#include <wglegacygui.h>
#include <wglegacyplugin.h>
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


Blob_p loadBlob(const std::string& path);

TextStyle_p		g_pTextStyle;

WgPluginRoot * 	g_pPluginRoot;

//____ init() _________________________________________________________________

DLLEXPORTPREFIX int init( wg_plugin_interface * pInterface, void * pRealHostBridge, wg_obj hPluginCapsule, wg_obj hGfxDevice, wg_obj hSurfaceFactory, wg_obj hEdgemapFactory )
{
	if( !WgPluginBase::init(pInterface, pRealHostBridge) )
		return 0;								// Host's plugin interface is older than ours.
	
	auto pSurfaceFactory 	= PluginSurfaceFactory::create(hSurfaceFactory);
	auto pEdgemapFactory	= PluginEdgemapFactory::create(hEdgemapFactory);
	auto pGfxDevice 		= PluginGfxDevice::create(hGfxDevice,pSurfaceFactory, pEdgemapFactory);
	
	WgPluginBase::setDefaultSurfaceFactory(pSurfaceFactory);
	WgPluginBase::setDefaultGfxDevice(pGfxDevice);
	
	
	auto pPluginRoot = new WgPluginRoot(hPluginCapsule, WgPluginBase::context() );

	auto pFontBlob = loadBlob("resources/DroidSans.ttf");
	auto pFont = FreeTypeFont::create(pFontBlob);

	g_pTextStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 14,
									_.color = Color8::Black,
									_.states = {{State::Disabled, Color8::DarkGrey}
	} ));

//	WgPluginBase::setDefaultStyle(g_pTextStyle);

	auto pSkin1 = ColorSkin::create({ .padding = 1, .states = { {State::Default, Color::Red}, {State::Hovered, Color::Pink}, {State::Pressed, Color::DarkRed} }  });
	auto pSkin2 = ColorSkin::create({ .padding = 1, .states = { {State::Default, Color::Green}, {State::Hovered, Color::LightGreen}, {State::Pressed, Color::DarkGreen} }  });
	auto pSkin3 = ColorSkin::create({ .states = { {State::Default, Color::Blue}, {State::Hovered, Color::LightBlue}, {State::Pressed, Color::DarkBlue} }  });

	
	auto pPacker = new WgPackPanel();
	pPacker->SetOrientation(wg::Axis::Y);
	
	auto pButton = new WgButton();
	pButton->SetTextStyle(g_pTextStyle);
	pButton->SetText( "BUTTON" );
	pButton->SetSkin( pSkin2 );
	
	pPacker->AddChild(pButton);
	
	auto pEditor = new WgTextDisplay();
	pEditor->SetTextStyle(g_pTextStyle);
	pEditor->SetText( "Editable Text" );
	pEditor->SetSkin( ColorSkin::create( Color8::White ) );
	pEditor->SetEditMode(WgTextEditMode::Editable);
	
	pPacker->AddChild(pEditor);
	
	pPacker->SetSkin( ColorSkin::create( Color8::Blue ) );
	
	pPluginRoot->SetChild(pPacker);
	
	
	

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
	WgPluginBase::exit();
}

//____ mapInputKey() _______________________________________________________

DLLEXPORTPREFIX void mapInputKey(uint32_t native_keycode, wg_key translated_keycode)
{
	WgPluginBase::MapKey((WgKey) translated_keycode, native_keycode);
}

//____ mapInputCommand() ___________________________________________________

DLLEXPORTPREFIX void mapInputCommand(uint32_t native_keycode, wg_modkeys modkeys, wg_editCommand command)
{
//	WgPluginBase::inputHandler()->mapCommand(native_keycode, (WgModKeys) modkeys, (EditCmd) command);
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

