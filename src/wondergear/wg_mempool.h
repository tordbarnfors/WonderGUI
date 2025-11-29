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
#ifndef WG_MEMPOOL_DOT_H
#define WG_MEMPOOL_DOT_H
#pragma once


#include <wg_geartypes.h>
#include <wg_chain.h>

namespace wg
{

	class MemPool
	{
	public:
		//.____ Creation __________________________________________

		MemPool( int entriesPerBlock, int entrySize );
		virtual ~MemPool();

		//.____ Misc _______________________________________________________

		void *	allocEntry();
		void	freeEntry( void * pEntry );

		inline int		entriesAllocated() const { return m_nAllocEntries; }
		inline int		entriesAvailable() const { return m_blocks.size()*m_nEntriesPerBlock - m_nAllocEntries; }
		inline int		capacity() const { return m_blocks.size()*m_nEntriesPerBlock; }
		inline bool		isEmpty() const { return (m_nAllocEntries == 0); }


	private:

		class Block;

		Block *	_addBlock();



		// Block of entries.

		class Block : public Link
		{
		public:
			Block( int nEntries, int entrySize );
			~Block();

			LINK_METHODS( Block );


			void * allocEntry();
			bool freeEntry( void * pEntry );

			void *		pMemBlock;			// Memory area containing our entries.
			int			blockSize;			// SizeI of memory area containing our entries.
			int			nAllocEntries;		// Number of entires currently in used.
			int			nCleanEntries;		// Number of clean entries, all entries after this
											// are free AND uninitzialised.
			int			maxEntries;			// Number of entries in block.
			int			firstFreeEntry;		// Number of first free entry, entry contains number of next
											// unless firstFreeEntry==nCleanEntries.
			int			entrySize;			// SizeI of each individual entry.

		};

		Chain<Block>	m_blocks;
		int				m_nEntriesPerBlock;
		int				m_entrySize;
		int				m_nAllocEntries;
	};





} // namespace wg
#endif //WG_MEMPOOL_DOT_H
