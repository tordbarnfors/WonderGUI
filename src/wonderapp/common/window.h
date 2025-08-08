/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/
#pragma once

#include <wondergui.h>

class Window;
typedef    wg::StrongPtr<Window>    Window_p;
typedef    wg::WeakPtr<Window>        Window_wp;

class Window : public wg::Object
{
public:

    //.____ Blueprint _________________________________________

    struct Blueprint
    {
        //NOTE: origo is only for initial positioning and is not maintained.
        // The origo-relative positioning of this blueprint will be recalculated
        // to a position in a coordinate system with origo at NorthWest.

        wg::Finalizer_p		finalizer = nullptr;
        wg::Size            maxSize;
        wg::Size            minSize;
        wg::Placement       origo = wg::Placement::Center;
        wg::Coord           pos;
        bool                resizable = true;
        wg::Size            size = { 640,480 };
        std::string         title = "WonderGUI Application";
    };

    //.____ Identification __________________________________________

    const wg::TypeInfo& typeInfo(void) const override;
    const static wg::TypeInfo    TYPEINFO;

    //.___ Content __________________________________________________

    wg::RootPanel_p rootPanel() const { return m_pRootPanel; }

	void			setContent(wg::Widget* pContent);

    //.____ Geometry ______________________________________________________

    wg::Rect        setGeo(const wg::Rect& geo);
    wg::Rect        geo() const { return m_geo; }

    wg::Coord        setPos(wg::Coord pos);
    wg::Coord        pos() const { return m_geo.pos(); }

    wg::Size        setSize(wg::Size size);
    wg::Size        size() const { return m_geo.size(); }

    //.____ Appearance ____________________________________________________

    virtual bool            setTitle(std::string& title) = 0;
    virtual std::string     title() const = 0;

    virtual bool            setIcon(wg::Surface * pIcon) = 0;

    //.____ Control _____________________________________________________________

    void    setCloseRequestHandler( std::function<bool(void)> handler );

    //.____ Internal ____________________________________________________________
    
    bool    _onCloseRequest();

    
    
protected:
    Window(wg::RootPanel* pRootPanel, const wg::Rect& geo);
    ~Window() {}

    virtual wg::Rect    _updateWindowGeo(const wg::Rect& geo) = 0;

    wg::RootPanel_p     m_pRootPanel;
	wg::Overlay_p		m_pLastOverlay;		// Overlay which we attach content to.

    wg::Rect            m_geo;
    std::function<bool(void)>    m_closeRequestHandler;

};
