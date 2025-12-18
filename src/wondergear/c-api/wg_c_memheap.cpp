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
#include <new>

#include <wg_c_memheap.h>
#include <wg_memheap.h>
#include <wg_object.h>


using namespace wg;



wg_memHeap wg_initMemHeap( void * pHeap, int bytes )
{
	int offset = sizeof(MemHeap);
	
	char * areaStart = ((char*)pHeap) + offset;
	bytes -= offset;
	if( bytes < 0 )
		return 0;
	
	return new(pHeap) MemHeap(areaStart,bytes);
}

void wg_exitMemHeap( wg_memHeap heap )
{
	reinterpret_cast<MemHeap*>(heap)->~MemHeap();
}

void * wg_memHeapMalloc( wg_memHeap heap, int bytes )
{
	return reinterpret_cast<MemHeap*>(heap)->malloc(bytes);
}

void wg_memHeapFree( wg_memHeap heap, void * pBuffer )
{
	reinterpret_cast<MemHeap*>(heap)->free(pBuffer);
}

/*
void wg_drawMemHeapFragmentMap( wg_memHeap heap, int nSections, uint16_t * pSectionTable, wg_obj surface )
{
	reinterpret_cast<MemHeap*>(heap)->drawFragmentMap(nSections, pSectionTable, static_cast<Surface*>(reinterpret_cast<Object*>(surface)));
}
*/

void wg_setMemHeapDebugLevel( wg_memHeap heap, int level )
{
	reinterpret_cast<MemHeap*>(heap)->setDebugLevel(level);
}

int wg_memHeapCapacity( wg_memHeap heap )
{
	return reinterpret_cast<MemHeap*>(heap)->capacity();
}

int wg_memHeapReserved( wg_memHeap heap )
{
	return reinterpret_cast<MemHeap*>(heap)->reserved();
}

int wg_largestAvailableBlock( wg_memHeap heap )
{
	return reinterpret_cast<MemHeap*>(heap)->largestAvailableBlock();
}
