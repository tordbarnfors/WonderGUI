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
#include <wg_memheap.h>
#include <wg_gearbase.h>
#include <cstddef>
#include <algorithm>

namespace wg
{
	
  //____ constructor _______________________________________________________________

  MemHeap::MemHeap( void * pHeap, int bytes )
  {
	  
    m_pBuffer = pHeap;

	// Make sure we are aligned

	char * pStart = (char*) ((((size_t)pHeap)+3) & (size_t) 0xFFFFFFFFFFFFFFFC);
	char * pEnd = (char*) ((((size_t)pHeap)+bytes) & (size_t) 0xFFFFFFFFFFFFFFFC);

	int capacity = int(pEnd - pStart) - sizeof(Header)*2;

    m_capacity = capacity;
    m_reserved = 0;



    Header * pHeader = (Header*) pStart;

    Header * pFooter = (Header*) (pEnd-sizeof(Header));

	pFooter->boundsGuard = 0xDEADBEEF;
	pFooter->pPrevBlock = pHeader;
	pFooter->pNextBlock = nullptr;
	pFooter->pPrevFree = nullptr;
	pFooter->pNextFree = nullptr;


	pHeader->boundsGuard = 0xDEADBEEF;
    pHeader->pPrevBlock = nullptr;
    pHeader->pNextBlock = pFooter;
    pHeader->pNextFree = nullptr;
    pHeader->pPrevFree = nullptr;
	
    m_pFirstBlock = pHeader;
    m_pFirstFree = pHeader;
  }

  //____ destructor _______________________________________________________________

  MemHeap::~MemHeap()
  {
  }

  //____ malloc() _____________________________________________________________

  void * MemHeap::malloc( int bytes )
  {
	  if( m_debugLevel > 1 )
	  {
		  _checkIntegrity();
	  }
	  
    bytes =(bytes+3) & 0xFFFFFFFC;

    Header * pHead = m_pFirstFree;

    while( pHead && bytes >= ((char*)pHead->pNextBlock) - ((char*)&pHead[1]) )
      pHead = pHead->pNextFree;

    if( !pHead )
	{
		GearBase::throwError(ErrorLevel::Error, ErrorCode::ResourceExhausted, "Failed to allocate memory. This MemHeap can not provide a continous area of that many bytes.", nullptr, nullptr, __func__, __FILE__, __LINE__);
		return nullptr;
	}

    int blockCapacity = (int) (((char*)pHead->pNextBlock) - ((char*)&pHead[1]));

    if( blockCapacity > bytes + sizeof(Header) )
    {
      Header * pNew = (Header*) (((char*)&pHead[1]) + bytes);

		pNew->boundsGuard = 0xDEADBEEF;

      pNew->pPrevBlock = pHead;
      pNew->pNextBlock = pHead->pNextBlock;
      pNew->pPrevFree = pHead->pPrevFree;
      pNew->pNextFree = pHead->pNextFree;

      if( pNew->pPrevFree )
        pNew->pPrevFree->pNextFree = pNew;	// Needed temporarily
      if( pNew->pNextFree )
        pNew->pNextFree->pPrevFree = pNew;
      if( pNew->pNextBlock )
        pNew->pNextBlock->pPrevBlock = pNew;

      pHead->pNextBlock = pNew;
      pHead->pNextFree = pNew;	// Needed temporarily.
    }
    else
    {
      if( pHead->pPrevFree )
        pHead->pPrevFree->pNextFree = pHead->pNextFree;
      if( pHead->pNextFree )
        pHead->pNextFree->pPrevFree = pHead->pPrevFree;
    }

    m_reserved += ((char*)pHead->pNextBlock) - ((char*)pHead);

    if( m_pFirstFree == pHead )
      m_pFirstFree = pHead->pNextFree;

    pHead->pPrevFree = nullptr;
    pHead->pNextFree = nullptr;

	if( m_debugLevel > 1 )
	{
		if( ((char*)pHead) < m_pBuffer || ((char*)&pHead[1]) + bytes > ((char*)m_pBuffer) + m_capacity + sizeof(Header) )
		{
			GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Reserved memory outside buffer", nullptr, nullptr, __func__, __FILE__, __LINE__);
		}
		  
		_checkIntegrity();
	}

	return &(pHead[1]);
  }

  //____ free() _______________________________________________________________

  void MemHeap::free( void * pBuffer )
  {
	  if( pBuffer == nullptr )
		return;

	  Header * pHead = &((Header*)pBuffer)[-1];

	  if( m_debugLevel > 0 )
	  {
		  if( pBuffer < m_pBuffer || pBuffer > (char*)m_pBuffer + m_capacity )
		  {
			  GearBase::throwError(ErrorLevel::Critical, ErrorCode::InvalidParam, "Pointer to memory to free is outside bounds and therefre do not point to memory allocated by this MemHeap object.", nullptr, nullptr, __func__, __FILE__, __LINE__);
		  }
		  
		  if( pHead->boundsGuard != 0xDEADBEEF )
		  {
			  GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Header of block being freed is corrupt. Either provided pointer is wrong or we have an out-of-bounds write somewhere.", nullptr, nullptr, __func__, __FILE__, __LINE__);
		  }

		  if( pHead->pNextBlock->boundsGuard != 0xDEADBEEF )
		  {
			  GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Bytes immediately following block being freed have been overwritten. Have we written too far?", nullptr, nullptr, __func__, __FILE__, __LINE__);
		  }

		  if( m_debugLevel > 1 )
			  _checkIntegrity();
	  }


      m_reserved -= ((char*)pHead->pNextBlock) - ((char*)&pHead[1]);


      Header * pPrev = pHead->pPrevBlock;
      Header * pNext = pHead->pNextBlock;

      bool  bEmptyPrevBlock = pPrev && (pPrev->pPrevFree || pPrev->pNextFree || pPrev == m_pFirstFree);
      bool  bEmptyNextBlock = pNext && (pNext->pPrevFree || pNext->pNextFree || pNext == m_pFirstFree);

      if( !bEmptyPrevBlock && !bEmptyNextBlock )
      {
        if( m_pFirstFree )
          m_pFirstFree->pPrevFree = pHead;

        pHead->pNextFree = m_pFirstFree;
        m_pFirstFree = pHead;

      }
      else
      {
        if( bEmptyPrevBlock )
        {
          pPrev->pNextBlock = pHead->pNextBlock;
          if( pHead->pNextBlock )
            pHead->pNextBlock->pPrevBlock = pPrev;

          pHead = pPrev;

          m_reserved -= sizeof(pHead);                          // Removed a block header
        }

        if( bEmptyNextBlock )
        {
          pHead->pNextBlock = pNext->pNextBlock;
          if( pNext->pNextBlock )
            pNext->pNextBlock->pPrevBlock = pHead;

          if( pHead == pPrev )
          {
            // pHead is already in the free linked list,
            // so we just remove pNext from the link.

            if( pNext->pPrevFree )
              pNext->pPrevFree->pNextFree = pNext->pNextFree;
            else
              m_pFirstFree = pNext->pNextFree;

            if( pNext->pNextFree )
              pNext->pNextFree->pPrevFree = pNext->pPrevFree;
          }
          else
          {
            // pHead is not in the free linked list,
            // so we let pHead take over pNext's place.

            pHead->pPrevFree = pNext->pPrevFree;
            pHead->pNextFree = pNext->pNextFree;

            if( pHead->pPrevFree )
              pHead->pPrevFree->pNextFree = pHead;
            else
              m_pFirstFree = pHead;

            if( pHead->pNextFree )
              pHead->pNextFree->pPrevFree = pHead;
          }

          m_reserved -= sizeof(pHead);                          // Removed a block header
        }
      }
	  
	  if( m_debugLevel > 1 )
		  _checkIntegrity();
  }

//____ drawFragmentMap() ______________________________________________________
/*
void MemHeap::drawFragmentMap( int nSectors, uint16_t * pSectorTable, Surface * pSurface16bits )
{
	for( int i = 0 ; i < nSectors ; i++ )
		pSectorTable[i] = 0;
	
	int bufferSize = m_capacity + sizeof(Header)*2;
	int bytesPerSector = bufferSize / nSectors;
	
	Header * pBlock = m_pFirstBlock;

	// Generate bytes/section table
		
	while( pBlock && pBlock->pNextBlock )
	{
		if( pBlock != m_pFirstFree && !pBlock->pPrevFree && !pBlock->pNextFree )
		{
			int blockStartByte = (char*)pBlock - (char*)m_pBuffer;
			int blockEndByte = (char*)pBlock->pNextBlock - (char*)m_pBuffer;

			int blockBytes = blockEndByte - blockStartByte;
			int sector = blockStartByte / bytesPerSector;
			
			int blockOfsInFirstSection = blockStartByte % bytesPerSector;
			
			int bytesInFirstSector = std::min( bytesPerSector - blockOfsInFirstSection, blockBytes );
			
			assert( sector >= 0 && sector < nSectors );
			pSectorTable[sector++] += bytesInFirstSector;
			blockBytes -= bytesInFirstSector;
			
			while( blockBytes > 0 )
			{
				int bytesInSector = std::min( blockBytes, bytesPerSector );
				assert( sector >= 0 && sector < nSectors );
				pSectorTable[sector++] += bytesInSector;
				blockBytes -= bytesInSector;
			}
		}
		
		pBlock = pBlock->pNextBlock;
	}
	
	// Start drawing the fragment map.

	PixelBuffer buffer = pSurface16bits->allocPixelBuffer();

	// Try out pixelsize for our sectors
	
	int secSize = 1;
	
	while( (buffer.rect.w / (secSize+1)) * (buffer.rect.h / (secSize+1)) > nSectors )
		secSize++;
	
	int sectorsPerRow = buffer.rect.w / secSize;

	// Generate pixel values for the colors we want.
	
	uint16_t colEmpty 	= pSurface16bits->colorToPixel( Color8::LightGreen );
	uint16_t colFrag 	= pSurface16bits->colorToPixel( Color8::Yellow );
	uint16_t colFull 	= pSurface16bits->colorToPixel( Color8::Red );

	// Clear surface
	
	pSurface16bits->fill(Color8::Black);
	
	// Draw fragment map.

	uint16_t * pPixels = (uint16_t*) buffer.pixels;

	for( int sector = 0 ; sector < nSectors ; sector++ )
	{
		// Decide color for sector
		
		uint16_t color;
		
		float fillFraction = pSectorTable[sector] / float(bytesPerSector);

		if( fillFraction == 0.f )
			color = colEmpty;
		else if( fillFraction == 1.f )
			color = colFull;
		else
			color = colFrag;
		
		// Draw the sector
		
		for( int y = 0 ; y < secSize ; y++ )
		{
			for( int x = 0 ; x < secSize ; x++ )
				pPixels[y*buffer.pitch/2+x] = color;
		}
		
			
		// Step forward pixelpointer for drawing next sector
		
		if( (sector+1) % sectorsPerRow == 0 )
		{
			int newRow = (sector+1) / sectorsPerRow;
			pPixels = (uint16_t*) (buffer.pixels+newRow*secSize*buffer.pitch);
		}
		else
			pPixels += secSize;
	}
	
	pSurface16bits->pullPixels(buffer);
	pSurface16bits->freePixelBuffer(buffer);
}
*/

//____ setDebugLevel() ________________________________________________________

void MemHeap::setDebugLevel( int level )
{
	m_debugLevel = level;
}

//____ largestAvailableBlock() ________________________________________________

int MemHeap::largestAvailableBlock() const
{
	int largest = 0;
	
	Header * p = m_pFirstFree;
	while( p )
	{
		if( p->pNextBlock )
		{
			int sz = (int) (((char*)p->pNextBlock) - ((char*)p) - sizeof(Header));
			if( sz > largest )
				largest = sz;
		}
		p = p->pNextFree;
	}
	return largest;
}


//____ _checkIntegrity() ______________________________________________________

void MemHeap::_checkIntegrity()
{
	
	for( auto pBlock = m_pFirstBlock ; pBlock != nullptr ; pBlock = pBlock->pNextBlock )
	{
		if( ((char*)pBlock > (char*)m_pBuffer + m_capacity + sizeof(Header)) || (char*)pBlock < m_pBuffer )
		{
			GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Memory Block outside bounds", nullptr, nullptr, __func__, __FILE__, __LINE__);
		}
		
		if( pBlock->boundsGuard != 0xDEADBEEF )
		{
			GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Write outside allocated memory detected", nullptr, nullptr, __func__, __FILE__, __LINE__);
		}
		
		if( pBlock->pNextBlock < &pBlock[1] && pBlock->pNextBlock != nullptr )
		{
			GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Header::pNextBlock has illegal value", nullptr, nullptr, __func__, __FILE__, __LINE__);
		}
		
		if( pBlock->pNextBlock && pBlock->pNextBlock->pPrevBlock != pBlock )
		{
			GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Block chain broken. Next block does not point back at us", nullptr, nullptr, __func__, __FILE__, __LINE__);
		}
		
		if( pBlock->pPrevBlock && pBlock->pPrevBlock->pNextBlock != pBlock )
		{
			GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Block chain broken. Prev block does not point to us", nullptr, nullptr, __func__, __FILE__, __LINE__);
		}
		
		if( pBlock->pNextFree && pBlock->pNextFree->pPrevFree != pBlock )
		{
			GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Block chain for free blocks broken. Next block does not point back to us", nullptr, nullptr, __func__, __FILE__, __LINE__);
		}

		if( pBlock->pPrevFree && pBlock->pPrevFree->pNextFree != pBlock )
		{
			GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Block chain for free blocks broken. Prev block does not point to us", nullptr, nullptr, __func__, __FILE__, __LINE__);
		}
		
		if( pBlock->pNextFree && !pBlock->pPrevFree && pBlock != m_pFirstFree )
		{
			GearBase::throwError(ErrorLevel::Critical, ErrorCode::Internal, "Block chain for free blocks broken. Free block not really part of the chain", nullptr, nullptr, __func__, __FILE__, __LINE__);
		}
	}
	
	
	
}


} // namespace wg
