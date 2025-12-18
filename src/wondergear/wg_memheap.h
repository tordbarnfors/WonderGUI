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
#ifndef WG_MEMHEAP_DOT_H
#define WG_MEMHEAP_DOT_H
#pragma once

#include <cstdint>


namespace wg 
{
	class Surface;


	class MemHeap
	{
	public:
		MemHeap( void * pHeap, int bytes );
		~MemHeap();

		void *		malloc( int bytes );
		void		free( void * pBuffer );

//		void		drawFragmentMap( int nSections, uint16_t * pSectionTable, Surface * pSurface );

		void		setDebugLevel( int level );
		
		int			capacity() const { return m_capacity; }
		int			reserved() const { return m_reserved; }
		int			largestAvailableBlock() const;
		
	private:

		void		_checkIntegrity();

		struct Header
		{
			uint32_t	boundsGuard;
			Header * pPrevBlock;
			Header * pNextBlock;
			Header * pNextFree;
			Header * pPrevFree;
		};

		void *	m_pBuffer;
		int		m_capacity;
		int		m_reserved;

		int		m_debugLevel = 0;		// 0 = No checks
										// 1 = Fast sanity checks on block being allocated/freed.
										// 2 = Sanity checks on all block headers for each alloc/free.
		
		Header * m_pFirstBlock;
		Header * m_pFirstFree;
	};



} // namespace wg
#endif //WG_MEMSTACK_DOT_H



