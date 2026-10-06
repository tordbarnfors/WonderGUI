
#ifdef WIN32
#    include <SDL.h>
#    include <SDL_image.h>
#    include <GL/glew.h>
#elif __APPLE__
#    include <SDL2/SDL.h>
#    include <SDL2_image/SDL_image.h>
#else
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_image.h>
#   include <GL/glew.h>
#endif


#include <list>

#include <wondergui.h>
#include <wg_softgfxdevice.h>
#include <wg_softsurfacefactory.h>


#include <wg_gfxstreamdevice.h>
#include <wg_gfxstreamsurfacefactory.h>

//#include <wg_cabi.h>

//#include <wg_glgfxdevice.h>
//#include <wg_glsurface.h>
//#include <wg_glsurfacefactory.h>

#include "../device.h"
#include "../gfxdevicetester.h"
#include "../app.h"


bool		init_system( Rect windowGeo, float scale );
void		exit_system();

bool		process_system_events(const RootPanel_p& pRoot);
void		update_window_rects( const Rect * pRects, int nRects );

bool		init_wondergui();
void		exit_wondergui();


MouseButton translateSDLMouseButton(Uint8 button);


Size				g_windowPointSize;
SizeI				g_windowPixelSize;
float				g_scale;

SDL_Window *		g_pSDLWindow = nullptr;

Surface_p			g_pWindowSurface = nullptr;				// Set by init_system()
RootPanel_p			g_pRoot = nullptr;

//wg_c_callCollection	g_cCallsCollection;


/*
//____ OpenGLDevice ________________________________________________________

class OpenGLDevice : public Device
{
public:
	OpenGLDevice()
	{
	}

	const char * name() const
	{
		return m_pName;
	}

	bool init(SizeI canvasSize, PixelFormat canvasFormat)
	{
		m_pCanvas = GlSurface::create(canvasSize, canvasFormat);
		m_pDevice = GlGfxDevice::create(1);              // Needs a UBO binding point separate from other GfxDevice.
		return true;
	}

	void exit()
	{
		m_pDevice = nullptr;
		m_pCanvas = nullptr;
	}

	GfxDevice_p beginRender() const
	{
		m_pDevice->beginRender();
		m_pDevice->beginCanvasUpdate(m_pCanvas);
		return m_pDevice;
	}

	void endRender() const
	{
		m_pDevice->endCanvasUpdate();
		m_pDevice->endRender();
	}

	GfxDevice_p	gfxDevice() const
	{
		return m_pDevice;
	}

	Surface_p canvas() const
	{
		return m_pCanvas;
	}

private:

	const char * m_pName = { "OpenGL" };

	GlGfxDevice_p	m_pDevice;
	GlSurface_p		m_pCanvas;
};
*/


//____ MyAppVisitor ________________________________________________________

class MyAppVisitor : public AppVisitor
{
public:
	wg::RootPanel_p	rootPanel() override
	{
		return g_pRoot;
	}

	int64_t			time() override
	{
		return SDL_GetPerformanceCounter() * 1000000 / SDL_GetPerformanceFrequency();
	}

	wg::Blob_p		loadBlob(const char* pPath) override;
	wg::Surface_p	loadSurface(const char* pPath, SurfaceFactory* pFactory, const Surface::Blueprint& bp = Surface::Blueprint() ) override;

protected:
	void			convertSDLFormat(PixelDescription* pWGFormat, const SDL_PixelFormat* pSDLFormat);

};


//____ main() _________________________________________________________________

int main(int argc, char *argv[] )
{
	// Create app and visitor, make any app-specif initialization

	auto pApp = new GfxDeviceTester();
	auto pVisitor = new MyAppVisitor();

	// Get apps window size before we continue

	g_windowPointSize = pApp->startWindowSize();
	g_scale = 1.f;
	g_windowPixelSize = SizeI(g_windowPointSize * g_scale);


	if (!init_system({ 20,20, g_windowPointSize }, g_scale ))
		return -1;
	
	if (!init_wondergui() )
		return -1;


	// Prepare CABI interface

	wg_populateCallCollection(&g_cCallsCollection);
	PluginBase::init(&g_cCallsCollection);

	//

	pApp->addTestDevice(new SoftwareDevice());
//	pApp->addTestDevice(new CABIToSoftwareDevice());
//	pApp->addTestDevice(new StreamToSoftwareDevice());
	pApp->addTestDevice(new OpenGLDevice());

	// Initialize the app

	bool bContinue = pApp->init( pVisitor );

	// Main loop

	while (bContinue)
	{
		// Handle system events

		bContinue = process_system_events(g_pRoot);

		if (bContinue)
		{
			// Uppdate the app

			bContinue = pApp->update();

			// Dispatch messages

			Base::msgRouter()->dispatch();

			// Periodic update

			Base::update(SDL_GetPerformanceCounter() * 1000000 / SDL_GetPerformanceFrequency());

			// Render. We do this outside the app since we might want to 
			// handle updated rectangles in a system specific way.

			g_pRoot->render();
			SDL_UpdateWindowSurface(g_pSDLWindow);

			// Sleep for a while

			SDL_Delay(10);
		}
	}

	// Exit the app

	pApp->exit();

	// App specific exit calls

	pApp->destroy_testdevices();

	//

	delete pApp;
	delete pVisitor;

	exit_wondergui();
	exit_system();
	return 0;
}


//____ init_wondergui() _______________________________________________________

bool init_wondergui()
{
	Base::init(nullptr);

	Context_p pContext = Context::create();
    pContext->setSurfaceFactory(GlSurfaceFactory::create());
	pContext->setGammaCorrection(true);
	Base::setActiveContext(pContext);

	auto pGfxDevice = GlGfxDevice::create(0);

	int scale = 64;

	pGfxDevice->setDefaultCanvas(g_windowPixelSize*64,  scale);

	g_pRoot = RootPanel::create( CanvasRef::Default, pGfxDevice);

//	g_pRoot->setDebugMode(true);

	return true;
}

//____ exit_wondergui() _______________________________________________________

void exit_wondergui()
{
	g_pRoot = nullptr;

	Base::exit();
}


//____ init_system() _______________________________________________________

bool init_system( Rect windowGeo, float scale )
{
	// initialize SDL video
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		return false;
	}

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);

    
	SDL_Window * pWin = SDL_CreateWindow("GfxDevice TestApp", windowGeo.x, windowGeo.y, windowGeo.w, windowGeo.h, SDL_WINDOW_OPENGL /* | SDL_WINDOW_ALLOW_HIGHDPI */);
	if( pWin == nullptr )
	{
		printf("Unable to create SDL window: %s\n", SDL_GetError());
		return false;
	}
    
	SDL_GL_CreateContext(pWin);

//	SDL_GL_SetSwapInterval(1);

	//	SDL_Renderer * pRenderer = SDL_CreateRenderer(pWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

#if defined(_WIN32) || defined(__linux__)
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
#endif

	glDrawBuffer(GL_FRONT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();


//    GlGfxDevice_p pGfxDevice = GlGfxDevice::create(windowGeo.size());
//    GlGfxDevice::create( {100,100} );

/*
	SDL_Surface * pWinSurf = SDL_GetWindowSurface(pWin);
	if (pWinSurf == nullptr)
	{
		printf("Unable to get window SDL Surface: %s\n", SDL_GetError());
		return false;
	}

	PixelFormat format = PixelFormat::Unknown;

	switch (pWinSurf->format->BitsPerPixel)
	{
		case 32:
			format = PixelFormat::ARGB_8;
			break;
		case 24:
			format = PixelFormat::XRGB_8;
			break;
		default:
		{
			printf("Unsupported pixelformat of SDL Surface!\n");
			return false;
		}
	}

    GlGfxDevice_p pGfxDevice = GlGfxDevice::create(windowGeo.size());

    
	g_pSDLWindow = pWin;

	Blob_p pCanvasBlob = Blob::create(pWinSurf->pixels, 0);
	g_pWindowSurface = SoftSurface::create(SizeI(pWinSurf->w, pWinSurf->h), format, pCanvasBlob, pWinSurf->pitch);
*/
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	return true;
}

//____ exit_system() __________________________________________________________

void exit_system()
{
	IMG_Quit();
}

//____ update_window_rects() __________________________________________________

void update_window_rects(const Rect* pRects, int nRects)
{
	if (nRects == 0)
		return;

	std::vector<SDL_Rect>	rects;

	for (int i = 0; i < nRects; i++)
	{
		SDL_Rect r = { (int) pRects[i].x, (int) pRects[i].y, (int) pRects[i].w, (int) pRects[i].h };
		rects.push_back(r);
	}

	SDL_UpdateWindowSurfaceRects(g_pSDLWindow, &rects.front(), nRects);
}


//____ process_system_events() ________________________________________________

bool process_system_events(const RootPanel_p& pRoot)
{
	// Process all the SDL events in a loop

	InputHandler_p pInput = Base::inputHandler();

	SDL_Event e;
	while (SDL_PollEvent(&e)) {

		switch (e.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_MOUSEMOTION:
			pInput->setPointer(pRoot, Coord(e.motion.x, e.motion.y));
			break;

		case SDL_MOUSEBUTTONDOWN:
			pInput->setButton(translateSDLMouseButton(e.button.button), true);
			break;

		case SDL_MOUSEBUTTONUP:
			pInput->setButton(translateSDLMouseButton(e.button.button), false);
			break;

		case SDL_MOUSEWHEEL:
		{
			bool bInvertScroll = false;
			Coord distance(e.wheel.x, e.wheel.y);
			if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
			{
				bInvertScroll = true;
				distance *= -1;
			}

			if (e.wheel.y != 0)
				pInput->setWheelRoll(1, distance.y, bInvertScroll);

			if (e.wheel.x != 0)
				pInput->setWheelRoll(2, distance.x, bInvertScroll);
			break;
		}

		case SDL_KEYDOWN:
		{
			pInput->setKey(e.key.keysym.sym, true);
			break;
		}

		case SDL_KEYUP:
		{
			pInput->setKey(e.key.keysym.sym, false);
			break;
		}

		case SDL_TEXTINPUT:
			pInput->putText(e.text.text);
			break;


		default:
			break;
		}
	}

	return true;
}

//____ translateSDLMouseButton() __________________________________________________
//
// Translate SDL mouse button enums to WonderGUI equivalents.
//
MouseButton translateSDLMouseButton(Uint8 button)
{
	switch (button)
	{
	default:
	case SDL_BUTTON_LEFT:
		return MouseButton::Left;
	case SDL_BUTTON_MIDDLE:
		return MouseButton::Middle;
	case SDL_BUTTON_RIGHT:
		return MouseButton::Right;
	case SDL_BUTTON_X1:
		return MouseButton::X1;
	case SDL_BUTTON_X2:
		return MouseButton::X2;
	}
}

//____ convertSDLFormat() ______________________________________________________

void MyAppVisitor::convertSDLFormat(PixelDescription* pWGFormat, const SDL_PixelFormat* pSDLFormat)
{
	pWGFormat->format = PixelFormat::Undefined;
	pWGFormat->bits = pSDLFormat->BitsPerPixel;
	pWGFormat->bIndexed = (pSDLFormat->palette != nullptr);

	pWGFormat->R_mask = pSDLFormat->Rmask;
	pWGFormat->G_mask = pSDLFormat->Gmask;
	pWGFormat->B_mask = pSDLFormat->Bmask;
	pWGFormat->A_mask = pSDLFormat->Amask;

	pWGFormat->R_shift = pSDLFormat->Rshift;
	pWGFormat->G_shift = pSDLFormat->Gshift;
	pWGFormat->B_shift = pSDLFormat->Bshift;
	pWGFormat->A_shift = pSDLFormat->Ashift;

	pWGFormat->R_loss = pSDLFormat->Rloss;
	pWGFormat->G_loss = pSDLFormat->Gloss;
	pWGFormat->B_loss = pSDLFormat->Bloss;
	pWGFormat->A_loss = pSDLFormat->Aloss;

	pWGFormat->R_bits = 8 - pSDLFormat->Rloss;
	pWGFormat->G_bits = 8 - pSDLFormat->Gloss;
	pWGFormat->B_bits = 8 - pSDLFormat->Bloss;
	pWGFormat->A_bits = 8 - pSDLFormat->Aloss;

}

//____ loadBlob() _________________________________________________________

Blob_p MyAppVisitor::loadBlob(const char* pPath)
{
	FILE* fp;

#ifdef WIN32
	errno_t err = fopen_s(&fp, pPath, "rb");
#else
	fp = fopen(pPath, "rb");
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

//____ loadSurface() ______________________________________________________

Surface_p MyAppVisitor::loadSurface(const char* pPath, SurfaceFactory* pFactory, const Surface::Blueprint& _bp)
{
	Surface::Blueprint bp = _bp;


	PixelDescription format;

	auto pSDLSurf = IMG_Load(pPath);
	convertSDLFormat(&format, pSDLSurf->format);

	PixelFormat px;
	Color8* pPalette = nullptr;

	Color8 palette[256];

	if (format.bIndexed)
	{
		px = PixelFormat::Index_8;

		for (int i = 0; i < 256; i++)
		{
			palette[i].r = pSDLSurf->format->palette->colors[i].r;
			palette[i].g = pSDLSurf->format->palette->colors[i].g;
			palette[i].b = pSDLSurf->format->palette->colors[i].b;
			palette[i].a = pSDLSurf->format->palette->colors[i].a;
		}
		pPalette = palette;
	}

	else if (format.A_bits > 0)
		px = PixelFormat::ARGB_8;
	else
		px = PixelFormat::XRGB_8;

	if (!pFactory)
		pFactory = Base::activeContext()->surfaceFactory();

	bp.format = px;
	bp.palette = pPalette;
	bp.size = { pSDLSurf->w, pSDLSurf->h };

	auto pSurface = pFactory->createSurface(bp, (unsigned char*)pSDLSurf->pixels, pSDLSurf->pitch, &format);
	SDL_FreeSurface(pSDLSurf);
	return pSurface;
}


