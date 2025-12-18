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
#include <wg_c_hostbridge.h>
#include <wg_hostbridge.h>
#include <wg_root.h>

#include <algorithm>
#include <cstring>

using namespace wg;

inline HostBridge* getPtr(wg_obj obj) {
	return reinterpret_cast<HostBridge*>(obj);
}

int wg_hidePointer( void * pHostBridge )
{
	return getPtr(pHostBridge)->hidePointer();
}

int wg_showPointer( void * pHostBridge )
{
	return getPtr(pHostBridge)->showPointer();
}

int wg_setPointerStyle( void * pHostBridge, wg_pointerStyle style )
{
	return getPtr(pHostBridge)->setPointerStyle( (PointerStyle) style );
}

int wg_getClipboardText( void * pHostBridge, int maxChars, char * pDest )
{
	std::string str = getPtr(pHostBridge)->getClipboardText();
	
	int nBytes = std::min(int(str.length()), maxChars);
	memcpy( pDest, str.c_str(), nBytes );
	
	if( nBytes < maxChars )
		pDest[nBytes] = 0;
	
	return nBytes;
}

int wg_setClipboardText( void * pHostBridge, const char * pText )
{
	return getPtr(pHostBridge)->setClipboardText(pText);
}

int wg_requestWindowFocus( void * pHostBridge, uintptr_t windowRef)
{
	return getPtr(pHostBridge)->requestFocus( windowRef );
}

int wg_yieldWindowFocus(void* pHostBridge, uintptr_t windowRef)
{
	return getPtr(pHostBridge)->yieldFocus(windowRef);
}

int wg_lockHidePointer(void* pHostBridge)
{
	return getPtr(pHostBridge)->lockHidePointer();
}

int wg_unlockShowPointer(void* pHostBridge)
{
	return getPtr(pHostBridge)->unlockShowPointer();
}
