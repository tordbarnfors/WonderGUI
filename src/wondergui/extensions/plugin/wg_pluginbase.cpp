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
#include <wg_pluginbase.h>
#include <wg_plugincalls.h>

namespace wg
{

	const TypeInfo	PluginBase::TYPEINFO = { "PluginBase", &Base::TYPEINFO };

	PluginContext_p		PluginBase::s_pPluginContext;


	//___ init() ______________________________________________________________

	bool PluginBase::init(wg_plugin_interface* pCallsCollection, void * pRealHostBridge)
	{
		if( !PluginCalls::_init(pCallsCollection) )
			return false;								// Host has too old ABI.

		Base::init(nullptr);
		Base::setContext(nullptr);

		auto pHostBridge = new PluginHostBridge(pRealHostBridge);
		Base::setHostBridge(pHostBridge);

		s_pPluginContext = new PluginContext();

		s_pPluginContext->pHostBridge = pHostBridge;
		s_pPluginContext->pluginInterface = * pCallsCollection;
		s_pPluginContext->pGUIContext = Base::context();

		return true;
	}

	//____ exit() __________________________________________________________________

	bool PluginBase::exit()
	{
		bool res = Base::exit();

		delete s_pPluginContext->pHostBridge;

		s_pPluginContext = nullptr;
		return res;
	}

	//____ setContext() __________________________________________________________

	PluginContext_p PluginBase::setContext( const PluginContext_p& pNewContext )
	{
		auto pOld = s_pPluginContext;

		if (pNewContext == nullptr)
		{
			Base::setContext(nullptr);
			s_pPluginContext->pGUIContext = Base::context();
			return pOld;
		}

		if( pNewContext != s_pPluginContext )
		{
			s_pPluginContext = pNewContext;
			PluginCalls::_init(&pNewContext->pluginInterface);
		}

		Base::setContext(pNewContext->pGUIContext);

		return pOld;
	}

	//____ setHostBridge() _______________________________________________________

	void PluginBase::setHostBridge( HostBridge * pHostBridge )
	{
		delete s_pPluginContext->pHostBridge;

		s_pPluginContext->pHostBridge = new PluginHostBridge(pHostBridge);

		Base::setHostBridge(pHostBridge);
	}
}


