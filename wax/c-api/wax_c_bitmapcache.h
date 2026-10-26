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
#ifndef WAX_C_BITMAPCACHE_DOT_H
#define WAX_C_BITMAPCACHE_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct wax_cacheSlot_struct
{
	wax_obj		surface;
	wax_coordI	coord;
} wax_cacheSlot;


typedef void(*wax_cacheAddedSurfaceFunc)(wax_obj surface, void * pData, int data);
typedef void(*wax_cacheRemovedSurfacesFunc)(int nSurfaces, wax_obj* pSurfaces, void * pData, int data);
typedef void(*wax_cacheClearedFunc)(void * pData, int data);

typedef void* wax_cacheListener;


typedef struct wax_cacheListenerBP_struct
{
	wax_cacheAddedSurfaceFunc	addFunc;
	void *						addPtr;
	int							addInt;
	
	wax_cacheRemovedSurfacesFunc	removeFunc;
	void *						removePtr;
	int							removeInt;
	
	wax_cacheClearedFunc			clearFunc;
	void *						clearPtr;
	int							clearInt;
} wax_cacheListenerBP;


WAX_EXPORT wax_obj			wax_createBitmapCache( int maxSize, wax_obj surfaceFactory );

WAX_EXPORT void 				wax_setCacheLimit(wax_obj bitmapCache, int maxBytes);
WAX_EXPORT int				wax_cacheLimit(wax_obj bitmapCache);
WAX_EXPORT int				wax_cacheSize(wax_obj bitmapCache);
WAX_EXPORT void				wax_clearCache(wax_obj bitmapCache);

WAX_EXPORT wax_cacheListener	wax_addCacheListener(wax_obj bitmapCache, wax_cacheListenerBP blueprint );
WAX_EXPORT void				wax_removeCacheListener(wax_obj bitmapCache, wax_cacheListener listener );

WAX_EXPORT wax_cacheSlot 		wax_getCacheSlot(wax_obj bitmapCache, wax_sizeI size);
WAX_EXPORT int				wax_getNbCacheSurfaces(wax_obj bitmapCache);
WAX_EXPORT int				wax_getCacheSurfaces(wax_obj bitmapCache, int maxSurfaces, wax_obj * pArray );


#ifdef __cplusplus
}
#endif


#endif
