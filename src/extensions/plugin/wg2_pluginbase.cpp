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
#include <wg2_pluginbase.h>
#include <wg_plugincalls.h>

using namespace wg;


WgPluginContext_p	WgPluginBase::s_pContext;




//___ init() ______________________________________________________________

bool WgPluginBase::init(wg_plugin_interface* pCallsCollection, void * pRealHostBridge,
						std::function<void(WgPluginContext *)> contextSwitchCallback )
{
	// Plugin initialization works differently from GearBase/GfxBase/Base in that a
	// new context is setup for each call to init.
	
	if( !PluginCalls::_init(pCallsCollection) )
		return false;								// Host has too old ABI!

	WgBase::Init(nullptr);
	WgBase::setContext(nullptr);

	auto pHostBridge = new PluginHostBridge(pRealHostBridge);
	s_pContext = new WgPluginContext();
	
	s_pContext->pBaseContext = WgBase::context();
	s_pContext->callsCollection = * pCallsCollection;
	s_pContext->pHostBridge = pHostBridge;
	s_pContext->contextSwitchCallback = contextSwitchCallback;
	
	return true;
}

//____ exit() __________________________________________________________________

bool WgPluginBase::exit()
{
	if( s_pContext == nullptr )
	{
		throwError(ErrorLevel::SilentError, ErrorCode::IllegalCall, "Call to WgPluginBase::exit() ignored, not initialized or already exited.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
		return false;
	}
	
	WgBase::Exit();

	s_pContext = nullptr;
	return true;
}

//____ setContext() ________________________________________________________

WgPluginContext_p WgPluginBase::setContext( const WgPluginContext_p& pNewContext )
{
	auto pOldContext = s_pContext;

	if (pNewContext == nullptr)
	{
		WgBase::setContext(nullptr);
		s_pContext->pBaseContext = WgBase::context();
		return pOldContext;
	}

	if( pNewContext != s_pContext )
    {
		s_pContext = pNewContext;
		PluginCalls::_init(&pNewContext->callsCollection);

		if( pNewContext->contextSwitchCallback )
		   pNewContext->contextSwitchCallback(pNewContext);
    }

	WgBase::setContext(pNewContext->pBaseContext);
	return pOldContext;
}
