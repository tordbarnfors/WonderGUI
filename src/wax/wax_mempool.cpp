
#include <wg_mempool.h>
#include <stdlib.h>

namespace wg
{

	//____ constructor ____________________________________________________________

	MemPool::MemPool( int entriesPerBlock, int entrySize )
	{
		m_nEntriesPerBlock	= entriesPerBlock;
		m_entrySize			= entrySize;
		m_nAllocEntries		= 0;
	}

	//____ Destructor _____________________________________________________________

	MemPool::~MemPool()
	{
	}

	//____ allocEntry() ___________________________________________________________

	void * MemPool::allocEntry()
	{
		m_nAllocEntries++;

		Block * pBlock = m_blocks.first();
		if(pBlock == 0)
			pBlock = _addBlock();

		if( pBlock->nAllocEntries == pBlock->maxEntries )
		{
			m_blocks.pushBack(pBlock);			// This block is full so we put it in the back.

			pBlock = m_blocks.first();
			if( pBlock->nAllocEntries == pBlock->maxEntries )
			{
				_addBlock();					// We don't have any free entries left in any block.
												// so we need to create a new one.
				pBlock = m_blocks.first();
			}
		}

		return pBlock->allocEntry();
	}

	//____ freeEntry() ____________________________________________________________

	void MemPool::freeEntry( void * pEntry )
	{
		if( pEntry == 0 )
			return;

		Block * pBlock = m_blocks.first();

		while( pBlock && ((pEntry < pBlock->pMemBlock) || (pEntry >= ((uint8_t*)pBlock->pMemBlock) + pBlock->blockSize)) )
		{
			pBlock = pBlock->next();
		}

		if( !pBlock )
		{
			return;								// ERROR!!! ENTRY HAS NOT BEEN RESERVED THROUGH US!!!!!!!!!!
		}

		if( pBlock->nAllocEntries == pBlock->maxEntries )
			m_blocks.pushFront(pBlock);			// Full block will get an entry free, needs to be among the free ones...

		pBlock->freeEntry(pEntry);

		if( pBlock->nAllocEntries == 0 )
			delete pBlock;

		m_nAllocEntries--;
	}


	//____ _addBlock() _____________________________________________________________

	MemPool::Block *MemPool::_addBlock()
	{
		Block * pBlock = new Block( m_nEntriesPerBlock, m_entrySize );
		m_blocks.pushFront( pBlock );
		return pBlock;
	}

	//____ Block::Constructor _____________________________________________________

	MemPool::Block::Block( int _nEntries, int _entrySize )
	{
		pMemBlock		= malloc( _nEntries*_entrySize );
		blockSize		= _nEntries*_entrySize;
		nAllocEntries	= 0;
		nCleanEntries	= 0;
		maxEntries		= _nEntries;
		firstFreeEntry	= 0;
		entrySize		= _entrySize;
	}

	//____ Block::Destructor ______________________________________________________

	MemPool::Block::~Block()
	{
		free( pMemBlock );
	}

	//____ Block::allocEntry() ____________________________________________________

	void * MemPool::Block::allocEntry()
	{
		if( nAllocEntries == maxEntries )
			return 0;

		void * p = ((uint8_t*)pMemBlock) + firstFreeEntry*entrySize;
		nAllocEntries++;

		if( firstFreeEntry == nCleanEntries )
		{
			firstFreeEntry++;
			nCleanEntries++;
		}
		else
		{
			firstFreeEntry = * ((int*)p);
		}
		return p;
	}

	//____ Block::freeEntry() _____________________________________________________

	bool MemPool::Block::freeEntry( void * pEntry )
	{
		if( pEntry < pMemBlock || pEntry >= ((uint8_t*)pMemBlock) + blockSize )
			return false;

		* ((int*)pEntry) = firstFreeEntry;
		firstFreeEntry = ((int) (((char*)pEntry) - ((char*)pMemBlock))) / entrySize;

		nAllocEntries--;
		return true;
	}



} // namespace wg
