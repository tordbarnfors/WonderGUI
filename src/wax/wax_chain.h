//-----------------------------------------------------------------------------
//
//								>>> wg_chain.h <<<
//
//					(c) 2002 Tord Jansson (tord.jansson@gmail.com)
//
//	Simple double-linked list implementation, done _MY WAY_!
//
//	This source code may be freely copied, modifed and used in any software
//  project, both free (as in speech) and proprietary. Commercial use and/or
//  distribution does not require any license.
//
//-----------------------------------------------------------------------------
#ifndef WG_CHAIN_DOT_H
#define	WG_CHAIN_DOT_H
#pragma once

#include <wg_geartypes.h>

namespace wg
{

	class Link;

	//____ Class ChainImp _________________________________________________________

	class ChainImp		/** @private */
	{
		friend	class Link;

	protected:
		ChainImp();
		virtual ~ChainImp();

		bool	isMemberOf( Link * pLink ) const;		///< Check if specified link is a member.
		void	clear();

		void	pushFront( Link * pLink );		///< Push link onto top of chain.
		Link* popFront();									///< Retrieve link from top of chain (disconnects it).

		void	pushBack( Link * pLink );		///< Add link to end of chain.
		Link* popBack();									///< Retrieve link from end of chain (disconnects it).

		Link* get( int index ) const;

		/*
		void	pushBack( ChainImp * pChain );	///< Push all links from specified chain to back of us.
		void	pushFront( ChainImp * pChain );	///< Push all links from specified chain to our front.

		void	pushBack( ChainImp * pChain, Link * pBeg, Link * pEnd );		///< Push all links from specified chain to back of us.
		void	pushFront( ChainImp * pChain, Link * pBeg, Link * pEnd );	///< Push all links from specified chain to our front.
	*/

	protected:
		Link *	m_pFirst;
		Link *	m_pLast;
		int			m_size;
	};



	//____ Class Chain ____________________________________________________________

	template<class T>
	class Chain : public ChainImp				/** @private */
	{
	public:
		inline bool	isMemberOf( T* pLink ) const { return ChainImp::isMemberOf(pLink); };
		inline void	clear() { ChainImp::clear(); };

		inline T*		first() const { return (T*) m_pFirst; };
		inline T*		last() const { return (T*) m_pLast; };

		inline void		pushFront( T* pLink ) { ChainImp::pushFront( pLink ); };
		inline T*		popFront() { return (T*) ChainImp::popFront(); };

		inline void		pushBack( T* pLink ) { ChainImp::pushBack( pLink ); };
		inline T*		popBack() { return (T*) ChainImp::popBack(); };

		inline T*		get( int index ) const { return (T*) ChainImp::get(index); };

		inline int		size() const { return m_size; }

		inline int		isEmpty() const { return (m_size==0); }
	};



	//____ Class Link _____________________________________________________________

	#define	LINK_METHODS( type ) \
		inline bool	moveUp() { return wg::Link::_moveUp(); }; \
		inline bool	moveDown() { return wg::Link::_moveDown(); }; \
		inline bool	moveFirst( wg::Chain<type> * pChain = 0 ) { return wg::Link::_moveFirst( pChain ); }; \
		inline bool	moveLast( wg::Chain<type> * pChain = 0 ) { return wg::Link::_moveLast( pChain ); }; \
		inline bool	moveAfter( type * pLink ) { return wg::Link::_moveAfter( pLink ); }; \
		inline bool	moveBefore( type * pLink) { return wg::Link::_moveBefore( pLink ); }; \
		inline bool	disconnect() {return wg::Link::_disconnect(); }; \
		inline type *	prev() const { return (type *) m_pPrev; }; \
		inline type *	next() const { return (type *) m_pNext; }; \
		inline wg::Chain<type> * chain() const { return (wg::Chain<type> *) m_pChain; }; \
		inline int		index() const { return wg::Link::_index(); };

	#define	PROTECTED_LINK_METHODS( type ) \
		inline bool	_moveUp() { return wg::Link::_moveUp(); }; \
		inline bool	_moveDown() { return wg::Link::_moveDown(); }; \
		inline bool	_moveFirst( wg::Chain<type> * pChain = 0 ) { return wg::Link::_moveFirst( pChain ); }; \
		inline bool	_moveLast( wg::Chain<type> * pChain = 0 ) { return wg::Link::_moveLast( pChain ); }; \
		inline bool	_moveAfter( type * pLink ) { return wg::Link::_moveAfter( pLink ); }; \
		inline bool	_moveBefore( type * pLink) { return wg::Link::_moveBefore( pLink ); }; \
		inline bool	_disconnect() {return wg::Link::_disconnect(); }; \
		inline type *	_prev() const { return (type *) m_pPrev; }; \
		inline type *	_next() const { return (type *) m_pNext; }; \
		inline wg::Chain<type> * _chain() const { return (wg::Chain<type> *) m_pChain; }; \
		inline int		_index() const { return wg::Link::_index(); };



	class Link		/** @private */
	{
		friend	class ChainImp;
	//	friend	class Chain;

	protected:
		Link( ChainImp * pChain = 0 );
		virtual ~Link();

		bool		_moveUp();						///< Move link one step closer to top.
		bool		_moveDown();						///< Move link one step closer to bottom.
		bool		_moveFirst( ChainImp * pChain = 0 );///< Move link to top of current or specified chain.
		bool		_moveLast( ChainImp * pChain = 0 ); ///< Move link to bottom of current or specified chain.
		bool		_moveAfter( Link * pLink );		///< Move link to position right after specified link in specified links chain.
		bool		_moveBefore( Link * pLink );		///< Move link to position right before specified link in specified links chain.

		int			_index() const;					///< Get position in chain (0+), -1 if not in a chain.
		bool		_disconnect();					///< Disconnects Link from its chain.

	protected:
		ChainImp *	m_pChain;

		Link *		m_pNext;
		Link *		m_pPrev;

	private:
		void		_reset();
		void		_unlink();

	};


	//____ ChainImp::pushFront() _______________________________________________

	inline void	ChainImp::pushFront( Link * pLink )
	{
		pLink->_moveFirst( this );
	}

	//____ ChainImp::popFront() ________________________________________________

	inline Link * ChainImp::popFront()
	{
		Link * pLink = m_pFirst;
		if( pLink )
			pLink->_disconnect();
		return pLink;
	}

	//____ ChainImp::pushBack() ________________________________________________

	inline void	ChainImp::pushBack( Link * pLink )
	{
		pLink->_moveLast( this );
	}

	//____ ChainImp::popBack() _________________________________________________

	inline Link * ChainImp::popBack()
	{
		Link * pLink = m_pLast;
		if( pLink )
			pLink->_disconnect();
		return pLink;
	}


	//____ Link::_unlink() _________________________________________________________

	inline void Link::_unlink()
	{
		if( m_pPrev )
			m_pPrev->m_pNext = m_pNext;
		else
			m_pChain->m_pFirst = m_pNext;

		if( m_pNext )
			m_pNext->m_pPrev = m_pPrev;
		else
			m_pChain->m_pLast = m_pPrev;

		m_pChain->m_size--;
	}

	//____ Link::_reset() __________________________________________________________

	inline void Link::_reset()
	{
		m_pChain	= 0;
		m_pPrev		= 0;
		m_pNext		= 0;
	}

	//______________________________________________________________________________

} // namespace wg
#endif // WG_CHAIN_DOT_H
