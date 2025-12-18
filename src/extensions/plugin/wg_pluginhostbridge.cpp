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
#include <wg_pluginhostbridge.h>
#include <wg_plugincalls.h>
#include <wg_root.h>

namespace wg
{

	//____ hidePointer() __________________________________________________________

	bool PluginHostBridge::hidePointer()
	{
		return PluginCalls::hostBridge->hidePointer(m_pHostBridge);
	}

	//____ showPointer() __________________________________________________________

	bool PluginHostBridge::showPointer()
	{
		return PluginCalls::hostBridge->showPointer(m_pHostBridge);
	}

	//____ setPointerStyle() ______________________________________________________

	bool PluginHostBridge::setPointerStyle( PointerStyle style )
	{
		return PluginCalls::hostBridge->setPointerStyle(m_pHostBridge, (wg_pointerStyle) style);
	}


	//____ getClipboardText() _____________________________________________________

	std::string PluginHostBridge::getClipboardText()
	{
		char temp[4096];
		
		int len = PluginCalls::hostBridge->getClipboardText(m_pHostBridge, 4096, temp );
		return temp;
	}

	//____ setClipboardText() _____________________________________________________

	bool PluginHostBridge::setClipboardText(const std::string& text)
	{
		return PluginCalls::hostBridge->setClipboardText(m_pHostBridge,text.c_str());
	}
	
	//____ requestFocus() _________________________________________________________

	bool PluginHostBridge::requestFocus(uintptr_t windowRef)
	{
		return PluginCalls::hostBridge->requestWindowFocus(m_pHostBridge, windowRef);
	}

	//____ yieldFocus() _________________________________________________________

	bool PluginHostBridge::yieldFocus(uintptr_t windowRef)
	{
		return PluginCalls::hostBridge->yieldWindowFocus(m_pHostBridge, windowRef);
	}

	//____ lockHidePointer() __________________________________________________

	bool PluginHostBridge::lockHidePointer()
	{
		return PluginCalls::hostBridge->lockHidePointer(m_pHostBridge);
	}

	//____ unlockShowPointer() __________________________________________________

	bool PluginHostBridge::unlockShowPointer()
	{
		return PluginCalls::hostBridge->unlockShowPointer(m_pHostBridge);
	}


} // namespace wg
