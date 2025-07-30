
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


#include <wg_base.h>

#include <wg_msgrouter.h>
#include <wg_basictextlayout.h>
#include <wg_basicnumberlayout.h>
#include <wg_inputhandler.h>
#include <wg_theme.h>

#include <wg_dummyfont.h>
#include <wg_memstack.h>
#include <wg_mempool.h>
#include <wg_textstyle.h>
#include <wg_texttool.h>
#include <wg_textstylemanager.h>
#include <wg_skinslotmanager.h>
#include <wg_bitmapcache.h>
#include <wg_packlayout.h>

#include <iostream>
#include <algorithm>

namespace wg
{
	const TypeInfo	Base::TYPEINFO = { "Base", &GfxBase::TYPEINFO };

	int				Base::s_guiInitCounter = 0;

	GUIContext_p	Base::s_pGUIContext;

	String			Base::s_clipboardText;

	//____ init() __________________________________________________________________

	bool Base::init( HostBridge * pHostBridge )
	{
		if( s_guiInitCounter > 0 )
		{
			s_guiInitCounter++;
			return true;
		}

		if( !GfxBase::init() )
			return false;

		s_pGUIContext = new GUIContext();
		s_pGUIContext->pGfxContext = GfxBase::context();

		s_pGUIContext->pHostBridge = pHostBridge;

		TextTool::setDefaultBreakRules();

		TextStyleManager::init();

		s_pGUIContext->pDefaultCaret = Caret::create();

		s_pGUIContext->pDefaultTextLayout = BasicTextLayout::create({});

		TextStyle::Blueprint textStyleBP;
		textStyleBP.font = DummyFont::create();
		s_pGUIContext->pDefaultTextStyle = TextStyle::create( textStyleBP );

		s_pGUIContext->pDefaultNumberLayout = BasicNumberLayout::create( BasicNumberLayout::Blueprint() );

		s_pGUIContext->pDefaultPackLayout = PackLayout::create( WGBP(PackLayout,
														_.expandFactor = PackLayout::Factor::Weight,
														_.shrinkFactor = PackLayout::Factor::Weight
														));

		s_pGUIContext->pMsgRouter = MsgRouter::create();
		s_pGUIContext->pInputHandler = InputHandler::create();
		s_pGUIContext->pSkinSlotManager = SkinSlotManager::create();

		s_pGUIContext->timestamp = 0;

		TextStyle::s_pDefaultStyle = s_pGUIContext->pDefaultTextStyle;

		s_guiInitCounter = 1;
		return true;
	}

	//____ exit() __________________________________________________________________

	bool Base::exit()
	{
		if( s_guiInitCounter <= 0 )
		{
			throwError(ErrorLevel::SilentError, ErrorCode::IllegalCall, "Call to Base::exit() ignored, not initialized or already exited.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		if( s_guiInitCounter > 1 )
		{
			s_guiInitCounter--;			// This belongs to GfxBase, but we do like this anyway.
			return true;
		}

		// We need to make sure our objects are destroyed before continuing.

		s_pGUIContext->pMsgRouter = nullptr;
		s_pGUIContext->pInputHandler = nullptr;
		s_pGUIContext->pSkinSlotManager = nullptr;
		s_pGUIContext->pDefaultTheme = nullptr;

		s_pGUIContext->pDefaultCaret = nullptr;
		s_pGUIContext->pDefaultTextLayout = nullptr;
		s_pGUIContext->pDefaultNumberLayout = nullptr;
		s_pGUIContext->pDefaultPackLayout = nullptr;

		s_pGUIContext = nullptr;

		s_clipboardText.clear();

		TextStyleManager::exit();

		s_guiInitCounter = 0;
		return GfxBase::exit();
	}

	//____ setContext() _______________________________________________________

	GUIContext_p Base::setContext( const GUIContext_p& pNewContext )
	{
		auto pOld = s_pGUIContext;

		if( pNewContext )
		{
			s_pGUIContext = pNewContext;
			GfxBase::setContext(pNewContext->pGfxContext);
		}
		else
		{
			auto p = new GUIContext();

			GfxBase::setContext(nullptr);
			p->pGfxContext = GfxBase::context();

			p->pDefaultCaret = Caret::create();

			p->pDefaultTextLayout = BasicTextLayout::create({});
			
			TextStyle::Blueprint textStyleBP;
			textStyleBP.font = DummyFont::create();
			p->pDefaultTextStyle = TextStyle::create( textStyleBP );

			p->pDefaultNumberLayout = BasicNumberLayout::create( BasicNumberLayout::Blueprint() );

			p->pDefaultPackLayout = PackLayout::create( WGBP(PackLayout,
																		 _.expandFactor = PackLayout::Factor::Weight,
																		 _.shrinkFactor = PackLayout::Factor::Weight
																		 ));
			p->pMsgRouter = MsgRouter::create();
			p->pInputHandler = InputHandler::create();
			p->pSkinSlotManager = SkinSlotManager::create();

			p->timestamp = 0;

			s_pGUIContext = p;

		}

		TextStyle::s_pDefaultStyle = s_pGUIContext->pDefaultTextStyle;
		return pOld;
	}

	//____ msgRouter() _________________________________________________________

	MsgRouter_p	Base::msgRouter()
	{
		return s_pGUIContext->pMsgRouter;
	}

	//____ inputHandler() ______________________________________________________

	InputHandler_p Base::inputHandler()
	{
		return s_pGUIContext->pInputHandler;
	}

	//____ skinSlotManager() _____________________________________________________

	SkinSlotManager_p Base::skinSlotManager()
	{
		return s_pGUIContext->pSkinSlotManager;
	}

	//____ defaultTheme() ______________________________________________________

	Theme_p Base::defaultTheme()
	{
		return s_pGUIContext->pDefaultTheme;
	}

	//____ setDefaultTheme() ___________________________________________________

	void Base::setDefaultTheme(Theme* pTheme)
	{
		s_pGUIContext->pDefaultTheme = pTheme;
	}


	//____ defaultCaret() ______________________________________________________

	Caret_p Base::defaultCaret()
	{
		return s_pGUIContext->pDefaultCaret;
	}

	//____ setDefaultCaret() ___________________________________________________

	void Base::setDefaultCaret( Caret * pCaret )
	{
		s_pGUIContext->pDefaultCaret = pCaret;
	}

	//_____ defaultTextLayout() ________________________________________________

	TextLayout_p Base::defaultTextLayout()
	{
		return s_pGUIContext->pDefaultTextLayout;
	}

	//____ setDefaultTextLayout() ___________________________________________________

	void Base::setDefaultTextLayout( TextLayout * pTextLayout )
	{
		s_pGUIContext->pDefaultTextLayout = pTextLayout;
	}

	//____ defaultNumberLayout() _____________________________________________

	NumberLayout_p Base::defaultNumberLayout()
	{
		return s_pGUIContext->pDefaultNumberLayout;
	}

	//____ setDefaultNumberLayout() _______________________________________________________

	void Base::setDefaultNumberLayout(NumberLayout * pFormatter)
	{
		s_pGUIContext->pDefaultNumberLayout = pFormatter;
	}

	//_____ defaultPackLayout() ________________________________________________

	PackLayout_p Base::defaultPackLayout()
	{
		return s_pGUIContext->pDefaultPackLayout;
	}

	//____ setDefaultPackLayout() ___________________________________________________

	void Base::setDefaultPackLayout( PackLayout * pTextLayout )
	{
		s_pGUIContext->pDefaultPackLayout = pTextLayout;
	}

	//____ defaultStyle() ______________________________________________________

	TextStyle_p Base::defaultStyle()
	{
		return s_pGUIContext->pDefaultTextStyle;
	}

	//____ setDefaultStyle() _______________________________________________________

	void Base::setDefaultStyle( TextStyle * pStyle )
	{
		s_pGUIContext->pDefaultTextStyle = pStyle;
		TextStyle::s_pDefaultStyle = pStyle;
	}

	//____ setClipboardText() ____________________________________________________

	void Base::setClipboardText( const String& text )
	{
		s_clipboardText = text;
		if( s_pGUIContext->pHostBridge )
		{
			auto stdString = CharSeq(text).getStdString();

			s_pGUIContext->pHostBridge->setClipboardText(stdString);
		}
	}

	//____ setHostBridge() _______________________________________________________

	void Base::setHostBridge( HostBridge * pHostBridge )
	{
		s_pGUIContext->pHostBridge = pHostBridge;
	}


	//____ getClipboardText() ____________________________________________________

	String Base::getClipboardText()
	{
		if( s_pGUIContext->pHostBridge )
		{
			auto stdString = s_pGUIContext->pHostBridge->getClipboardText();

			return String(stdString);
		}
		else
		{
			return s_clipboardText;
		}
	}

	//____ update() ______________________________________________________________

	void Base::update( int64_t timestamp )
	{
		int64_t microPassed = timestamp - s_pGUIContext->timestamp;
		s_pGUIContext->timestamp = timestamp;

		// Any too large delay is transformed into a minor update.
		// This takes care of first call and freezes not messing
		// up things too much.

		if( microPassed > 2000000 )
			microPassed = 1000;

		// Update wondergui systems

		s_pGUIContext->pInputHandler->_update(timestamp/1000);
		s_pGUIContext->pSkinSlotManager->update(int(microPassed/1000));

		// Update widgets.

		bool bEmptyFound = false;

		for (auto pReceiver : s_pGUIContext->updateReceivers)
		{
			if( pReceiver )
				pReceiver->_update(int(microPassed), timestamp);
			else
				bEmptyFound = true;
		}

		if( bEmptyFound )
			s_pGUIContext->updateReceivers.erase(std::remove(s_pGUIContext->updateReceivers.begin(), s_pGUIContext->updateReceivers.end(), nullptr), s_pGUIContext->updateReceivers.end() );
	}


	//____ _startReceiveUpdates() ________________________________________________

	int64_t Base::_startReceiveUpdates(Receiver* pReceiver)
	{
		s_pGUIContext->updateReceivers.push_back(pReceiver);
		return s_pGUIContext->timestamp;
	}

	//____ _stopReceiveUpdates() _________________________________________________

	void Base::_stopReceiveUpdates(Receiver* pReceiver)
	{
		auto it = std::find(s_pGUIContext->updateReceivers.begin(), s_pGUIContext->updateReceivers.end(), pReceiver);
		if( it != s_pGUIContext->updateReceivers.end() )
			* it = nullptr;
		else
		{
			Base::throwError(ErrorLevel::Warning, ErrorCode::SystemIntegrity, "Base::_stopReceiveUpdates() called on object not receiving updates.", pReceiver, nullptr, __func__, __FILE__, __LINE__);
		}
	}


} // namespace wg
