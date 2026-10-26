
#ifdef WIN32
#include <SDL.h>
#include <SDL_image.h>
#else
#	ifdef __APPLE__
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wdocumentation"
#		include <SDL2/SDL.h>
#		include <SDL2_image/SDL_image.h>
#       pragma clang diagnostic pop
#	else
#		include <SDL2/SDL.h>
#		include <SDL2/SDL_image.h>
#	endif
#endif

#include <map>

#include <wg2_types.h>
#include <wg2_widget.h>

#include <wax_surface.h>
#include <wax_font.h>

namespace wax
{
	class SurfaceFactory;
}

namespace wg
{
};


class WgEventHandler;
class WgResDB;
class WgFont;

namespace sdl_wglib
{
	void 		MapKeys();
	void 		BeginEvents( WgEventHandler * pHandler );
	void 		TranslateEvent( SDL_Event& event );
	void 		EndEvents();

wg::Surface_p LoadSurface( const char * path, wg::SurfaceFactory * factory, const wg::Surface::Blueprint& bp = {} );
    bool		SavePNG( wg::Surface * pSurface, const char * path);

    std::map<std::string,WgWidget*> *	LoadStdWidgets( const char * pImagePath, const char * pImagePathX2, const char * pImagePathX4, wg::SurfaceFactory * factory );

    wg::Font_p	LoadBitmapFont( const char * pImgPath, const char * pSpecPath, wg::SurfaceFactory * factory );
	
	int 		FileSize( const char * pPath );
	void * 		LoadFile( const char * pPath );

	void		ConvertPixelFormat( WgPixelFormat * pWGFormat, const SDL_PixelFormat * pSDLFormat );
	
};
