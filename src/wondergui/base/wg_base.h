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

#ifndef	WG_BASE_DOT_H
#define	WG_BASE_DOT_H
#pragma once

#include <wg_gfxbase.h>

#include <assert.h>

#include <wg_types.h>
#include <wg_hostbridge.h>
#include <wg_string.h>

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <atomic>



namespace wg
{
	class Font;
	class MsgRouter;
	class NumberLayout;
	class InputHandler;
	class TextLayout;
	class PackLayout;
	class Caret;
	class TextStyle;
	class Receiver;
	class SkinSlotManager;
	class Theme;

	typedef	StrongPtr<MsgRouter>		MsgRouter_p;
	typedef	StrongPtr<NumberLayout>		NumberLayout_p;
	typedef	StrongPtr<InputHandler>		InputHandler_p;
	typedef	StrongPtr<TextLayout>		TextLayout_p;
	typedef	StrongPtr<Caret>			Caret_p;
	typedef	StrongPtr<TextStyle>		TextStyle_p;
	typedef	StrongPtr<PackLayout>		PackLayout_p;
	typedef	StrongPtr<SkinSlotManager>	SkinSlotManager_p;
	typedef	StrongPtr<Theme>			Theme_p;


	class GUIContext : public Object
	{
		friend class Base;
		
		MsgRouter_p			pMsgRouter;
		InputHandler_p		pInputHandler;
		TextLayout_p		pDefaultTextLayout;
		Caret_p				pDefaultCaret;
		NumberLayout_p		pDefaultNumberLayout;
		TextStyle_p			pDefaultTextStyle;
		PackLayout_p		pDefaultPackLayout;
		SkinSlotManager_p	pSkinSlotManager;
		Theme_p				pDefaultTheme;

		HostBridge*		pHostBridge;

		int64_t			timestamp;

		std::vector<Receiver*>	updateReceivers;
		
		GfxContext_p		pGfxContext;
	};

	typedef	StrongPtr<GUIContext>	GUIContext_p;
	typedef	WeakPtr<GUIContext>		GUIContext_wp;


	/**
	 * @brief	Static base class for WonderGUI.
	 *
	 * A static base class for WonderGUI that handles initialization and general
	 * housekeeping and resource allocation.
	 *
	 * The first thing that you need to do when starting WonderGUI is to call Base::init()
	 * and the last thing you should do is to call Base::exit().
	 *
	 */

	class Base : public GfxBase
	{
//		friend class Object_wp;
		friend class Object;


	public:

		//.____ Creation __________________________________________

		static bool init( HostBridge * pHostBridge );
		static bool exit();

		//.____ Content _____________________________________________

		static GUIContext_p	 setContext( const GUIContext_p& pNewContext );
		static GUIContext_p	 context() { return s_pGUIContext; }

		static MsgRouter_p			msgRouter();
		static InputHandler_p		inputHandler();
		static SkinSlotManager_p	skinSlotManager();

		static void			setDefaultTheme(Theme* pTheme);
		static Theme_p		defaultTheme();

		static void			setDefaultTextLayout( TextLayout * pTextLayout );
		static TextLayout_p defaultTextLayout();

		static void			setDefaultCaret(Caret * pCaret);
		static Caret_p		defaultCaret();

		static void			setDefaultStyle( TextStyle * pStyle );
		static TextStyle_p 	defaultStyle();

		static void			setDefaultNumberLayout(NumberLayout * pFormatter);
		static NumberLayout_p defaultNumberLayout();

		static void			setDefaultPackLayout( PackLayout * pPackLayout );
		static PackLayout_p defaultPackLayout();
		
		static void			setClipboardText( const String& text );
		static String		getClipboardText();
		
		static void			setHostBridge( HostBridge * pHostBridge );
		static HostBridge *	hostBridge() { return s_pGUIContext->pHostBridge; }


		
		//.____ Misc ________________________________________________

		const static TypeInfo	TYPEINFO;

		static bool			isInitialized() { return s_guiInitCounter > 0; }

		static void			update( int64_t timestamp_microseconds );

		static int64_t 		timestamp() { return s_pGUIContext->timestamp; }

		//.____ Internal ____________________________________________________________

		static int64_t		_startReceiveUpdates(Receiver* pReceiver);
		static void			_stopReceiveUpdates(Receiver* pReceiver);

		static TextStyle*	_defaultStyle() { return s_pGUIContext->pDefaultTextStyle.rawPtr(); }

	protected:
		static	GUIContext_p	s_pGUIContext;

	private:

		static	int				s_guiInitCounter;		
		static	String			s_clipboardText;

	};



} // namespace wg
#endif //WG_BASE_DOT_H
