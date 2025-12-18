/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#pragma once

#include <wg_object.h>
#include <wg_blob.h>
#include <wg_surface.h>
#include <wg_rootpanel.h>
#include <wg_theme.h>

#include <wappapi.h>
#include <wappwindow.h>


class WonderApp;
typedef	wg::StrongPtr<WonderApp>	WonderApp_p;
typedef	wg::WeakPtr<WonderApp>		WonderApp_wp;


class WonderApp : public wg::Object
{
public:


	static WonderApp_p create();

	virtual bool	init(wapp::API* pAPI) = 0;
	virtual bool	update() = 0;
	virtual void	exit() = 0;

	virtual void	closeWindow(wapp::Window* pWindow) = 0;
		
protected:

	~WonderApp() {}
	
};




