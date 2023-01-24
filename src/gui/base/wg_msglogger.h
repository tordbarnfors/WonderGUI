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

#ifndef WG_MSGLOGGER_DOT_H
#define WG_MSGLOGGER_DOT_H
#pragma once

#include <iostream>
#include <wg_msgrouter.h>

namespace wg
{


	class MsgLogger;
	typedef	StrongPtr<MsgLogger>	MsgLogger_p;
	typedef	WeakPtr<MsgLogger>		MsgLogger_wp;

	class MsgLogger : public Receiver
	{
	public:
		//.____ Creation __________________________________________

		static MsgLogger_p		create( std::ostream& stream ) { return MsgLogger_p( new MsgLogger(stream)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _______________________________________________________

		void ignoreMsg( MsgType msg );
		void logMsg( MsgType msg );

		void logPointerMsgs();
		void ignorePointerMsgs();

		void logMouseButtonMsgs();
		void ignoreMouseButtonMsgs();

		void logKeyboardMsgs();
		void ignoreKeyboardMsgs();

		void logMouseMsgs();
		void ignoreMouseMsgs();

		void logInputMsgs();
		void ignoreInputMsgs();

		void logAllMsgs();
		void ignoreAllMsgs();

		//.____ Misc _______________________________________________________

		void receive(Msg * _pMsg) override;

		bool isMsgLogged( MsgType msg ) { return m_msgFilter[(int)msg]; }

	private:
		MsgLogger( std::ostream& stream );
		~MsgLogger();

		void	_onRouteAdded() override;
		void	_onRouteRemoved() override;

		std::string	_formatTimestamp( int64_t ms ) const;
		std::string _formatSource( Msg * _pMsg ) const;
		std::string _formatCopyTo( Msg * _pMsg ) const;
		std::string _formatModkeys( InputMsg * _pMsg ) const;
		std::string _formatPointerPos( InputMsg * _pMsg ) const;
		std::string _formatPointerStyle( PointerChangeMsg * _pMsg ) const;
		std::string _formatMouseButton( MouseButton button ) const;
		std::string _formatEditCommand( EditCmd command ) const;

		bool			m_msgFilter[MsgType_size];

		std::ostream&	m_out;

	};






} // namespace wg
#endif //WG_MSGLOGGER_DOT_H
