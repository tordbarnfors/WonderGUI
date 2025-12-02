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
#include <assert.h>
#include <wg_msg.h>
#include <wg_msgrouter.h>
#include <wg_base.h>
#include <wg_rootpanel.h>

namespace wg
{

	const TypeInfo MsgRouter::TYPEINFO = { "MsgRouter", &Object::TYPEINFO };


	//____ constructor ____________________________________________________________

	MsgRouter::MsgRouter()
	{
		m_bIsProcessing			= false;

		m_routeCounter = 1;				// We start on 1
	}

	//____ Destructor _____________________________________________________________

	MsgRouter::~MsgRouter()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MsgRouter::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ broadcastTo() ___________________________________________________________

	RouteId  MsgRouter::broadcastTo( Receiver * pReceiver )
	{
		Route * p = new ObjectRoute( pReceiver, MsgType::Dummy );
		p->m_handle = m_routeCounter++;
		m_broadcasts.pushBack( p );
		return p->m_handle;
	}

	RouteId  MsgRouter::broadcastTo(std::function<void(Msg * pMsg)> func)
	{
		Route * p = new FunctionRoute(func, MsgType::Dummy);
		p->m_handle = m_routeCounter++;
		m_broadcasts.pushBack(p);
		return p->m_handle;
	}

	//____ endBroadcast() __________________________________________________________

	bool  MsgRouter::endBroadcast( RouteId handle )
	{
		Route * p = m_broadcasts.first();
		while( p )
		{
			if( p->m_handle == handle )
			{
				delete p;
				return true;
			}

			p = p->next();
		}
		return false;
	}

	//____ addRoute() __________________________________________________________

	RouteId MsgRouter::addRoute( Object * pSource, Receiver * pReceiver )
	{
		Route * p = new ObjectRoute( pReceiver, MsgType::Dummy );
		return _addRoute( pSource, p );
	}

	RouteId MsgRouter::addRoute( Object * pSource, MsgType filter, Receiver * pReceiver )
	{
		Route * p = new ObjectRoute( pReceiver, filter );
		return _addRoute( pSource, p );
	}

	RouteId MsgRouter::addRoute( MsgType msgType, Receiver * pReceiver )
	{
		Route * p = new ObjectRoute( pReceiver, MsgType::Dummy );
		return _addRoute( msgType, p );
	}

	RouteId MsgRouter::addRoute(Object * pSource, std::function<void(Msg * pMsg)> func)
	{
		Route * p = new FunctionRoute(func, MsgType::Dummy);
		return _addRoute(pSource, p);
	}

	RouteId MsgRouter::addRoute(Object * pSource, MsgType filter, std::function<void(Msg * pMsg)> func)
	{
		Route * p = new FunctionRoute(func, filter);
		return _addRoute(pSource, p);
	}

	RouteId MsgRouter::addRoute(MsgType msgType, std::function<void(Msg * pMsg)> func)
	{
		Route * p = new FunctionRoute(func, MsgType::Dummy);
		return _addRoute(msgType, p);
	}


	//____ deleteRoutesTo() _______________________________________________________

	int MsgRouter::deleteRoutesTo( Receiver * _pReceiver )
	{
		Receiver * pReceiver = _pReceiver;
		int nDeleted = 0;

		// Delete from source routes

		for( auto it = m_sourceRoutes.begin() ; it != m_sourceRoutes.end() ; it++ )
		{
			Route * p = it->second.first();
			while( p )
			{
				if( p->receiver() == pReceiver )
				{
					Route * pNext = p->next();
					delete p;					// Receiver is dead, delete route.
					nDeleted++;
					p = pNext;
				}
				else
					p = p->next();

			}
		}

		// Delete from type routes

		for( auto it = m_typeRoutes.begin() ; it != m_typeRoutes.end() ; it++ )
		{
			Route * p = it->second.first();
			while( p )
			{
				if( p->receiver() == pReceiver )
				{
					Route * pNext = p->next();
					delete p;					// Receiver is dead, delete route.
					nDeleted++;
					p = pNext;
				}
				else
					p = p->next();
			}
		}

		return nDeleted;
	}

	//____ deleteRoutesFrom() _______________________________________________________

	int MsgRouter::deleteRoutesFrom( Object * pSource )
	{
		auto it = m_sourceRoutes.find(Object_wp(pSource) );

		if( it == m_sourceRoutes.end() )
			return 0;

		int nDeleted = it->second.size();
		m_sourceRoutes.erase(it);
		return nDeleted;
	}

	int MsgRouter::deleteRoutesFrom( MsgType msgType )
	{
		auto it = m_typeRoutes.find(msgType);

		if( it == m_typeRoutes.end() )
			return 0;

		int nDeleted = it->second.size();
		m_typeRoutes.erase(it);
		return nDeleted;
	}


	//____ deleteRoute() ______________________________________________________

	bool MsgRouter::deleteRoute( RouteId id )
	{
		for( auto it = m_typeRoutes.begin() ; it != m_typeRoutes.end() ; ++it )
		{
			Route * p = it->second.first();
			while( p )
			{
				if( p->m_handle == id )
				{
					delete p;
					return true;
				}
				p = p->next();
			}
		}

		for( auto it = m_sourceRoutes.begin() ; it != m_sourceRoutes.end() ; ++it )
		{
			Route * p = it->second.first();
			while( p )
			{
				if( p->m_handle == id )
				{
					delete p;
					return true;
				}
				p = p->next();
			}
		}

		return false;
	}

	//____ clearMsgQueue() _______________________________________________________

	int MsgRouter::clearMsgQueue()
	{
		int nMessages = (int) m_msgQueue.size();
		m_msgQueue.clear();
		return nMessages;
	}

	//____ clearRoutes() _______________________________________________________

	int MsgRouter::clearRoutes()
	{
		m_sourceRoutes.clear();
		m_typeRoutes.clear();
		return false;
	}

	//____ garbageCollectRoutes() __________________________________________________

	int MsgRouter::garbageCollectRoutes()
	{
		int nDeleted = 0;

		// Delete any dead global callbacks

		Route * p = m_broadcasts.first();
		while( p )
		{
			if( p->isAlive() )
				p = p->next();
			else
			{
				Route * pNext = p->next();
				delete p;
				nDeleted++;
				p = pNext;
			}
		}

		// Delete any dead source routes.
		// These can be dead by either source or receiver having been deleted.

		{
			auto it = m_sourceRoutes.begin();

			while( it != m_sourceRoutes.end() )
			{
				if( !it->first )
				{
					nDeleted += it->second.size();
					it = m_sourceRoutes.erase(it);		// Sender is dead, delete whole branch of callbacks.
				}
				else
				{
					Route * p = it->second.first();
					while( p )
					{
						if( p->isAlive() )
							p = p->next();
						else
						{
							Route * pNext = p->next();
							delete p;					// Receiver is dead, delete callback.
							nDeleted++;
							p = pNext;
						}
					}
					++it;
				}
			}
		}
		// Delete any dead type routes.
		// These can be dead by receiver having been deleted.

		for( auto it = m_typeRoutes.begin(); it != m_typeRoutes.end() ; it++ )
		{
			Route * p = it->second.first();
			while( p )
			{
				if( p->isAlive() )
					p = p->next();
				else
				{
					Route * pNext = p->next();
					delete p;					// Receiver is dead, delete callback.
					nDeleted++;
					p = pNext;
				}
			}
		}

		return nDeleted;
	}

	//____ _addRoute() _________________________________________________________

	RouteId MsgRouter::_addRoute( Object * pSource, Route * pRoute )
	{
		if( !pSource )
			return 0;

		Chain<Route>& chain = m_sourceRoutes[pSource];
		chain.pushBack(pRoute);
		pRoute->m_handle = m_routeCounter++;
		return pRoute->m_handle;
	}

	//____ _addRoute() _________________________________________________________

	RouteId MsgRouter::_addRoute( MsgType type, Route * pRoute )
	{
		if( type == MsgType::Dummy || type > MsgType_max )
			return 0;

		Chain<Route>& chain = m_typeRoutes[type];
		chain.pushBack(pRoute);
		pRoute->m_handle = m_routeCounter++;
		return pRoute->m_handle;
	}


	//____ post() ___________________________________________________________

	bool MsgRouter::post( Msg * pMsg )
	{

		if( m_bIsProcessing )
		{
			// Msgs that are posted as a direct result of another event being processed
			// are inserted before the next event in the queue.
			// If two or more events are posted by the same event being processed,
			// they need to be queued in the order of posting.

			m_insertPos = m_msgQueue.insert( m_insertPos, pMsg );
			m_insertPos++;
		}
		else
		{
			// Msgs being posted outside processing loop are simply added to the
			// queue.

			m_msgQueue.push_back( pMsg );
		}

		return true;
	}

	//____ dispatch() ________________________________________________________

	void MsgRouter::dispatch()
	{
		if( m_bIsProcessing )
		{
			Base::throwError(ErrorLevel::Warning, ErrorCode::IllegalCall, "dispatch() called from within a dispatch callback (i.e. message being processed calls dispatch). Call ignored.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		m_bIsProcessing = true;

		m_insertPos = m_msgQueue.begin();	// Insert any POINTER_ENTER/EXIT right at beginning.

		_dispatchQueued();

		m_bIsProcessing = false;
	}


	//____ _dispatchQueued() ___________________________________________________

	void MsgRouter::_dispatchQueued()
	{
		while( !m_msgQueue.empty() )
		{
			Msg_p pMsg = m_msgQueue.front();
			m_insertPos = m_msgQueue.begin()+1;	// Insert position set to right after current event.

			do
			{
				if( pMsg->hasCopyTo()  )
					pMsg->getCopyTo()->receive( pMsg );

				if( pMsg->hasSource() )
					_dispatchToSourceRoutes( pMsg );

				if( !pMsg->reposted() )
					_dispatchToTypeRoutes( pMsg );

				_broadcast( pMsg );
			}
			while( pMsg->doRepost() );

			if( pMsg->hasFinalRecipient() )
				pMsg->finalRecipient()->receive( pMsg );

			m_msgQueue.pop_front();
		}

		m_insertPos = m_msgQueue.begin();		// Insert position set right to start.
	}


	//____ _broadcast() ________________________________________________

	void MsgRouter::_broadcast( Msg * pMsg )
	{
		Route * pRoute = m_broadcasts.first();

		while( pRoute )
		{
			auto pNext = pRoute->next();
			pRoute->dispatch( pMsg );
			pRoute = pNext;
		}
	}


	//____ _dispatchToTypeRoutes() __________________________________________________

	void MsgRouter::_dispatchToTypeRoutes( Msg * pMsg )
	{
		auto it = m_typeRoutes.find(pMsg->type());
		if( it != m_typeRoutes.end() )
		{
			Route * pRoute = it->second.first();

			while( pRoute )
			{
				auto pNext = pRoute->next();
				pRoute->dispatch( pMsg );
				pRoute = pNext;
			}
		}
	}

	//____ _dispatchToSourceRoutes() ________________________________________________

	void MsgRouter::_dispatchToSourceRoutes( Msg * pMsg )
	{
		Object * pSource = pMsg->sourceRawPtr();

		if( pSource )
		{
			Object_wp src = Object_wp(pSource);
			auto it = m_sourceRoutes.find(Object_wp(pSource));
			if( it != m_sourceRoutes.end() )
			{
				Route * pRoute = it->second.first();
				while( pRoute )
				{
					auto pNext = pRoute->next();
					if( pRoute->m_filter == MsgType::Dummy || pRoute->m_filter == pMsg->type() )
						pRoute->dispatch( pMsg );
					pRoute = pNext;
				}
			}
		}
	}


	MsgRouter::Route::Route(MsgType filter)
	{
		m_filter = filter;
	}

	MsgRouter::Route::~Route()
	{
	}

	MsgRouter::ObjectRoute::ObjectRoute( Receiver * pReceiver, MsgType filter ) : Route(filter)
	{
		m_pReceiver = pReceiver;
		pReceiver->_onRouteAdded();
	}

	MsgRouter::ObjectRoute::~ObjectRoute()
	{
		if( m_pReceiver )
			m_pReceiver->_onRouteRemoved();
	}

	void MsgRouter::ObjectRoute::dispatch( Msg * pMsg )
	{
		m_pReceiver->receive( pMsg );
	}

	bool MsgRouter::ObjectRoute::isAlive() const
	{
		return m_pReceiver != nullptr;
	}

	Receiver * MsgRouter::ObjectRoute::receiver() const
	{
		return m_pReceiver.rawPtr();
	}

	MsgRouter::FunctionRoute::FunctionRoute(std::function<void(Msg * pMsg)> func, MsgType filter) : Route(filter)
	{
		m_func = func;
	}

	MsgRouter::FunctionRoute::~FunctionRoute()
	{
	}

	void MsgRouter::FunctionRoute::dispatch(Msg * pMsg)
	{
		m_func(pMsg);
	}

	bool MsgRouter::FunctionRoute::isAlive() const
	{
		return true;
	}

	Receiver * MsgRouter::FunctionRoute::receiver() const
	{
		return nullptr;
	}



} // namespace wg
