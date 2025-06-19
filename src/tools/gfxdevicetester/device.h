#pragma once


#include <wg_gfxdevice.h>
#include <wg_theme.h>

#include <wondergui.h>
#include <wondergfxstream.h>
//#include <wg_plugingfxdevice.h>

class Device;
typedef wg::StrongPtr<Device>   Device_p;
typedef wg::WeakPtr<Device>     Device_wp;


class Device : public wg::PackPanel
{
public:

    static Device_p create( const std::string& name, wg::GfxDevice * pDevice, wg::CanvasRef canvasRef, wg::Surface * pSurface, Theme * pTheme ) { return new Device(name, pDevice, canvasRef, pSurface, pTheme);};

    const std::string&   name() const { return m_name; }

	wg::GfxDevice_p		beginRender();
	void			endRender();

    wg::GfxDevice_p		gfxDevice() const { return m_pDevice; }
    wg::Surface_p		canvas() const { return m_pCanvas; }
	wg::Surface_p		displaySurface() const { return m_pDisplaySurface; }

	void			setNeedsRedraw() { m_bNeedsRedraw = true; };
	bool			needsRedraw() { return m_bNeedsRedraw; };

protected:
	Device( const std::string& name, wg::GfxDevice * pDevice, wg::CanvasRef canvasRef, wg::Surface * pSurface, Theme * pTheme );
    virtual ~Device() {};

    
    std::string          	m_name;
    wg::GfxDevice_p     	m_pDevice;
	wg::CanvasRef			m_canvasRef;
    wg::Surface_p       	m_pCanvas;
	wg::Surface_p			m_pDisplaySurface;
	wg::SurfaceDisplay_p	m_pDisplay;

	bool				m_bNeedsRedraw = true;
    
};

