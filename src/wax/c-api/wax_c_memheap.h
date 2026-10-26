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
#ifndef WAX_C_MEMHEAP_DOT_H
#define WAX_C_MEMHEAP_DOT_H
#pragma once

#include <wax_c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef void * wax_memHeap;

	WAX_EXPORT wax_memHeap	wax_initMemHeap( void * pHeap, int bytes );
	WAX_EXPORT void			wax_exitMemHeap( wax_memHeap heap );

	WAX_EXPORT void *		wax_malloc( wax_memHeap heap, int bytes );
	WAX_EXPORT void  		wax_free( wax_memHeap heap, void * pBuffer );
/*
	void		wax_drawFragmentMap( wax_memHeap heap, int nSections, uint16_t * pSectionTable, wax_obj surface );
*/
	WAX_EXPORT void			wax_setMemHeapDebugLevel( wax_memHeap heap, int level );

	WAX_EXPORT int			wax_memHeapCapacity( wax_memHeap heap );
	WAX_EXPORT int			wax_memHeapReserved( wax_memHeap heap );
	WAX_EXPORT int			wax_largestAvailableBlock( wax_memHeap heap );

#ifdef __cplusplus
}
#endif

#endif //WAX_C_MEMHEAP_DOT_H
