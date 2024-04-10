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

#ifndef	WG2_BASE_DOT_H
#define	WG2_BASE_DOT_H

/*
#ifndef WG2_TEXTMGR_DOT_H
#	include <wg2_textmgr.h>
#endif
*/

#include <assert.h>
#include <map>
#include <functional>
#include <string>

#ifndef WG2_USERDEFINES_DOT_H
#	include <wg2_userdefines.h>
#endif

#ifndef WG2_KEY_DOT_H
#	include <wg2_key.h>
#endif

#include <wg_gfxbase.h>
#include <wg_textstyle.h>
#include <wg_hostbridge.h>


class WgCursor;

namespace wg
{
	class Object;
	class MemStack;
    class HostBridge;
};


class WgError
{
public:
	wg::ErrorCode   code;
	std::string     message;
	const wg::Object *  pObject;
	const char *    classname;
	const char *    function;
	const char *    file;
	int             line;
};

class WgContext : public wg::Object
{
	friend class WgBase;
	
	WgCursor *			pDefaultCursor = nullptr;

	// Settings for keyboard/pointer input

	int					doubleClickTimeTreshold = 250;		// Maximum millseconds between first and second press to count as a doubleclick.
	int					doubleClickDistanceTreshold = 2;	// Maximum distance the pointer may move between first and second press to count as a doubleclick.

	int					buttonRepeatDelay = 300;
	int					buttonRepeatRate = 200;

	int					keyRepeatDelay = 300;
	int					keyRepeatRate = 150;

	std::map<int,WgKey>	keycodeMap;		// Maps native keycodes to WgKey.
	
	wg::HostBridge *	pHostBridge = nullptr;
	
	wg::GfxContext_p	pGfxContext;
};

typedef	wg::StrongPtr<WgContext>	WgContext_p;
typedef	wg::WeakPtr<WgContext>		WgContext_wp;


class WgBase : public wg::GfxBase
{
public:
	static void Init( wg::HostBridge * pHostBridge );
	static void Exit();

	static WgContext_p	 switchContext( const WgContext_p& pNewContext );

	
	static void 	SetDefaultCursor( WgCursor * pCursor );
	static WgCursor * GetDefaultCursor() { assert(s_pContext!=nullptr); return s_pContext->pDefaultCursor; }

	static void		MapKey( WgKey translated_keycode, int native_keycode );
	static void		UnmapKey( WgKey translated_keycode );
	static void		ClearKeyMap();
	static WgKey	TranslateKey( int native_keycode );

	static bool		SetMouseButtonRepeat( int delay, int rate );
	static bool		SetKeyRepeat( int delay, int rate );

	static int		MouseButtonRepeatDelay() { assert(s_pContext!=0); return s_pContext->buttonRepeatDelay; }
	static int		MouseButtonRepeatRate() { assert(s_pContext!=0); return s_pContext->buttonRepeatRate; }

	static int		KeyRepeatDelay() { assert(s_pContext!=0); return s_pContext->keyRepeatDelay; }
	static int		KeyRepeatRate() { assert(s_pContext!=0); return s_pContext->keyRepeatRate; }

	static bool		SetDoubleClickTresholds( int time, int distance );
	static int		DoubleClickTimeTreshold() { assert(s_pContext!=0); return s_pContext->doubleClickTimeTreshold; }
	static int		DoubleClickDistanceTreshold() { assert(s_pContext!=0); return s_pContext->doubleClickDistanceTreshold; }

	static wg::HostBridge * hostBridge() { return s_pContext->pHostBridge; }
	
	static void		setDefaultStyle( wg::TextStyle* pStyle );
	static wg::TextStyle_p	defaultStyle() { return wg::TextStyle::s_pDefaultStyle; }

private:

	static int                              s_iSoftubeNumberOfInstances;
	static WgContext_p						s_pContext;
		
};

#endif //WG2_BASE_DOT_H
