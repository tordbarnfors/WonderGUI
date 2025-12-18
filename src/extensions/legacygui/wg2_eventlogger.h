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
#ifndef WG_EVENTLOGGER_DOT_H
#define WG_EVENTLOGGER_DOT_H

#include <iostream>
#include <functional>

#include <wg2_eventhandler.h>


class WgEventLogger : public WgEventListener
{
public:

	WgEventLogger( std::ostream& stream );
	WgEventLogger( const std::function<void(const char*)> func );

	~WgEventLogger();

	void OnEvent( const WgEvent::Event * _pEvent );

	void IgnoreEvent( WgEventType event );
	void LogEvent( WgEventType event );

	void LogPointerEvents();
	void IgnorePointerEvents();

	void LogMouseButtonEvents();
	void IgnoreMouseButtonEvents();

	void LogKeyboardEvents();
	void IgnoreKeyboardEvents();

	void LogMouseEvents();
	void IgnoreMouseEvents();

	void LogInputEvents();
	void IgnoreInputEvents();

	void LogAllEvents();
	void IgnoreAllEvents();

	bool IsEventLogged( WgEventType event ) { return m_eventFilter[event]; }

private:
	std::string	_formatTimestamp( int64_t ms );
	std::string _formatWidget( const WgEvent::Event * _pEvent );
	std::string _formatModkeys( const WgEvent::Event * _pEvent );
	std::string _formatPointerPos( const WgEvent::Event * _pEvent );
	std::string _formatPointerStyle( const WgEvent::PointerChange * _pEvent );


	bool			m_eventFilter[WG_EVENT_MAX];

	std::ostream *	m_pStream = nullptr;
	std::function<void(const char*)>	m_func;

};





#endif //WG_EVENTLOGGER_DOT_H
