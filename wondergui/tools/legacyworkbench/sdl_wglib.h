
#include <SDL3/SDL.h>

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

	void		ConvertPixelFormat( WgPixelFormat * pWGFormat, SDL_PixelFormat pSDLFormat );
	SDL_Surface *	loadImage( const char * pPath );
	
};
