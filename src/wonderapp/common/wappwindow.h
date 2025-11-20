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
#include <wappwindowapi.h>
//#include <wappapi.h>

namespace wg
{
class DebugFrontend;
}

namespace wapp
{
	class API;

	class Window;
	typedef    wg::StrongPtr<Window>    Window_p;
	typedef    wg::WeakPtr<Window>        Window_wp;

	class Window : public wg::Object, public WindowAPI
	{
	public:

		//.____ Blueprint _________________________________________

		struct Blueprint
		{
			//NOTE: origin is only for initial positioning and is not maintained.
			// The origin-relative positioning of this blueprint will be recalculated
			// to a position in a coordinate system with origin at NorthWest.

			bool				debug = true;
			wg::Finalizer_p		finalizer = nullptr;
			wg::Placement       origin = wg::Placement::Center;
			wg::Coord           pos;
			bool                open = true;
			bool                resizable = true;
			wg::Size            size = { 640,480 };
			std::string         title = "WonderGUI Application";

			bool				dndOverlay = true;
			bool				popupOverlay = true;
			bool				tooltipOverlay = true;
			bool				modalOverlay = true;
		};

		//.____ Creation __________________________________________

		static Window_p    create(API* pAPI, const Blueprint& blueprint);


		//.____ Identification __________________________________________

		const wg::TypeInfo& typeInfo(void) const override;
		const static wg::TypeInfo    TYPEINFO;

		//.___ Content __________________________________________________

		wg::Capsule_p	mainCapsule() const { return m_pMainCapsule; }

		//.____ Geometry ______________________________________________________

		wg::Rect        setGeo(const wg::Rect& geo);
		wg::Rect        geo() const { return m_geo; }

		wg::Coord       setPos(wg::Coord pos);
		wg::Coord       pos() const { return m_geo.pos(); }

		wg::Size        setSize(wg::Size size);
		wg::Size        size() const { return m_geo.size(); }

		//.____ Appearance ____________________________________________________

		bool            setTitle(std::string& title);
		std::string     title() const;

		bool            setIcon(wg::Surface* pIcon);

		//.____ Internal ____________________________________________________________

		bool		onClose() override;
		wg::Size	adjustSize(wg::Size size) override;
		void		onResize(wg::Size size) override;
		void		onMove(wg::Coord pos) override;
		void		onFocusGained() override;
		void		onFocusLost() override;
		void		onMinimized() override;
		void		onRestored() override;



	protected:
		Window(API* pAPI, const Blueprint& blueprint);
		~Window();

		wg::Rect    _updateWindowGeo(const wg::Rect& geo);

		SysCalls *			m_pSysCalls = nullptr;

		wg::RootPanel_p		m_pRoot;
		wg::Capsule_p		m_pMainCapsule;

		wg::Rect            m_geo;

	};

}

