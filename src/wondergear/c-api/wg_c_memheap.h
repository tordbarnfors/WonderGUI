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
#ifndef WG_C_MEMHEAP_DOT_H
#define WG_C_MEMHEAP_DOT_H
#pragma once

#include <wg_c_geartypes.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef void * wg_memHeap;

	WG_EXPORT wg_memHeap	wg_initMemHeap( void * pHeap, int bytes );
	WG_EXPORT void			wg_exitMemHeap( wg_memHeap heap );

	WG_EXPORT void *		wg_malloc( wg_memHeap heap, int bytes );
	WG_EXPORT void  		wg_free( wg_memHeap heap, void * pBuffer );
/*
	void		wg_drawFragmentMap( wg_memHeap heap, int nSections, uint16_t * pSectionTable, wg_obj surface );
*/
	WG_EXPORT void			wg_setMemHeapDebugLevel( wg_memHeap heap, int level );

	WG_EXPORT int			wg_memHeapCapacity( wg_memHeap heap );
	WG_EXPORT int			wg_memHeapReserved( wg_memHeap heap );
	WG_EXPORT int			wg_largestAvailableBlock( wg_memHeap heap );

#ifdef __cplusplus
}
#endif

#endif //WG_C_MEMHEAP_DOT_H
