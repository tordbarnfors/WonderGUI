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
#ifndef WG_MSGROUTER_DOT_H
#define WG_MSGROUTER_DOT_H
#pragma once

#include <deque>
#include <map>
#include <vector>
#include <functional>

#include <wg_msg.h>
#include <wg_chain.h>


#include <wg_key.h>

namespace wg
{

	class RootPanel;


	class MsgRouter;
	typedef	StrongPtr<MsgRouter>		MsgRouter_p;
	typedef	WeakPtr<MsgRouter>	MsgRouter_wp;

	class MsgRouter : public Object
	{
	friend class Widget;
	friend class RootPanel;

	public:
		//.____ Creation __________________________________________

		static MsgRouter_p	create() { return MsgRouter_p(new MsgRouter()); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _________________________________________________

		bool		post( Msg * pMsg );
		void		dispatch();


		RouteId		broadcastTo( Receiver * pReceiver );
		RouteId		broadcastTo(std::function<void(Msg * pMsg)> func );
		bool		endBroadcast( RouteId handle );

		RouteId		addRoute( Object * pSource, Receiver * pReceiver );
		RouteId		addRoute( Object * pSource, MsgType filter, Receiver * pReceiver );
		RouteId		addRoute( MsgType type, Receiver * pReceiver );

		RouteId		addRoute(Object * pSource, std::function<void(Msg * pMsg)> func);
		RouteId		addRoute(Object * pSource, MsgType filter, std::function<void(Msg * pMsg)> func);
		RouteId		addRoute(MsgType type, std::function<void(Msg * pMsg)> func);


		bool		deleteRoute( RouteId handle );
		int			deleteRoutesTo( Receiver * pReceiver );
		int			deleteRoutesFrom( Object * pSource );
		int			deleteRoutesFrom( MsgType type );

		int			clearMsgQueue();		// Deletes all messages in queue without dispatching them.
		int			clearRoutes();
		int			garbageCollectRoutes();

		//----

	private:
		MsgRouter();
		~MsgRouter();

		class		Route;

		void 		_dispatchQueued();


		void		_broadcast( Msg * pMsg );
		void		_dispatchToSourceRoutes( Msg * pMsg );
		void		_dispatchToTypeRoutes( Msg * pMsg );

		RouteId		_addRoute( Object * pSource, Route * pRoute );
		RouteId		_addRoute( MsgType type, Route * pRoute );

		//

		class Route : public Link
		{
		friend class MsgRouter;
		public:
			Route( MsgType filter );
			virtual ~Route();

			LINK_METHODS(Route);

			virtual void 		dispatch( Msg * pMsg ) = 0;
			virtual bool 		isAlive() const = 0;
			virtual Receiver *	receiver() const = 0;

		protected:
			RouteId				m_handle;
			MsgType				m_filter;					// Filter for dispatching. Message needs to be the same if filter != Dummy
		};

		//

		class ObjectRoute : public Route
		{
		public:
			ObjectRoute( Receiver * pReceiver, MsgType filter);
			virtual ~ObjectRoute();

			void 		dispatch(Msg * pMsg) override;
			bool 		isAlive() const override;
			Receiver *	receiver() const override;

		protected:
			Receiver_wp			m_pReceiver;
		};

		//

		class FunctionRoute : public Route
		{
		public:
			FunctionRoute(std::function<void(Msg * pMsg)> func, MsgType filter);
			virtual ~FunctionRoute();

			void 		dispatch(Msg * pMsg) override;
			bool 		isAlive() const override;
			Receiver *	receiver() const override;

		protected:
			std::function<void( Msg * pMsg )> m_func;
		};




		std::deque<Msg_p>			m_msgQueue;
		bool						m_bIsProcessing;		// Set when we are inside dispatch().
		std::deque<Msg_p>::iterator	m_insertPos;			// Position where we insert messages being queued when processing.

		RouteId					m_routeCounter;				// Increment by one for each new callbackHandle, gives unique IDs.
		Chain<Route>			m_broadcasts;

		std::map<Object_wp,Chain<Route> >	m_sourceRoutes;
		std::map<MsgType,Chain<Route> >		m_typeRoutes;
	};



} // namespace wg
#endif //WG_MSGROUTER_DOT_H
