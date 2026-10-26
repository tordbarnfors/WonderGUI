
#pragma once

#include <wonderapp.h>

#include <wondergui.h>
#include <wax_freetypefont.h>

#include <wax_softsurface.h>

#include <initializer_list>
#include <string>

#include <wax_c.h>

using namespace wg;

class MyApp : public WonderApp
{
public:

	bool		init(wapp::API* pAPI) override;
	bool		update() override;
	void		exit() override;

	void		closeWindow(wapp::Window* pWindow) override;


private:

	bool			_setupGUI(wapp::API* pAPI);
	bool			_loadStream(std::string path);

	bool			_setupStreamPlaying();


	wapp::API * m_pAppAPI = nullptr;
	wapp::Window_p	m_pWindow = nullptr;
	
	TextStyle_p		m_pTextStyle;
	TextStyle_p		m_pLabelStyle;

	TextLayout_p	m_pTextLayoutCentered;

	SurfaceDisplay_p	m_pDisplay;

	Blob_p			m_pStreamBlob;


	wax_obj 			m_wrapper = nullptr;
	wax_obj 			m_player = nullptr;
	wax_obj 			m_pump = nullptr;
	wax_obj			m_streamGfxDevice = nullptr;

	wax_obj			m_streamFrontCanvas = nullptr;
	wax_obj			m_streamBackCanvas = nullptr;

	wax_patches		m_streamFrontPatches = nullptr;
	wax_patches		m_streamBackPatches = nullptr;
};
