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
#ifndef	WG_PLUGINBASE_DOT_H
#define	WG_PLUGINBASE_DOT_H
#pragma once

#include <wg_plugininterface.h>
#include <wg_pluginhostbridge.h>
#include <wg_base.h>

namespace wg
{

	class PluginContext : public Object
	{
		friend class PluginBase;

		wg_plugin_interface		pluginInterface;

		PluginHostBridge *		pHostBridge;	// Needs a local copy so we can delete it with the context.
		GUIContext_p			pGUIContext;
	};

	typedef	StrongPtr<PluginContext>	PluginContext_p;
	typedef	WeakPtr<PluginContext>		PluginContext_wp;


	class PluginBase : public Base
	{
	public:

		//.____ Creation __________________________________________

		static bool init( wg_plugin_interface* pInterface, void * pRealHostBridge );
		static bool exit();

		//.____ Content _____________________________________________

		static PluginContext_p	setContext( const PluginContext_p& pNewContext );
		static PluginContext_p	context() { return s_pPluginContext; }

		static void				setHostBridge( HostBridge * pHostBridge );


		//.____ Misc ________________________________________________

		const static TypeInfo	TYPEINFO;

	private:

		static	PluginContext_p	s_pPluginContext;

	};


} // namespace wg
#endif //WG_PLUGINBASE_DOT_H
