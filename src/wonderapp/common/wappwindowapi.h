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

#include <string>

namespace wapp
{


	class WindowAPI
	{

	public:

		class SysCalls
		{
		public:
			virtual void		destroy() = 0;
			virtual wg::Rect	setGeo(const wg::Rect& geo) = 0;
			virtual bool		requestFocus() = 0;
			virtual bool		releaseFocus() = 0;
			virtual bool		minimize() = 0;
			virtual bool		restore() = 0;
		};


		class Result
		{
		public:
			bool			success;
			std::string		errorMsg;
			wg::Rect		geo;
			wg::RootPanel_p	root;
			SysCalls *		pSysCalls;
		};


		virtual				~WindowAPI() = default;
		virtual bool		onClose() = 0;
		virtual wg::Size	adjustSize(wg::Size size) = 0;
		virtual void		onResize(wg::Size size) = 0;
		virtual void		onMove(wg::Coord pos) = 0;
		virtual void		onFocusGained() = 0;
		virtual void		onFocusLost() = 0;
		virtual void		onMinimized() = 0;
		virtual void		onRestored() = 0;
	};

} // namespace wapp