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
#include <wg_c_bitmapcache.h>
#include <wg_bitmapcache.h>
#include <wg_gfxbase.h>

using namespace wg;

inline BitmapCache* getPtr(wg_obj obj) {
	return static_cast<BitmapCache*>(reinterpret_cast<Object*>(obj));
}

class Listener : public BitmapCacheListener
{
public:
	Listener() {};
	virtual ~Listener() = default;

	void _cacheSurfaceAdded(Surface* pSurface) override { m_data.addFunc(static_cast<Object*>(pSurface),m_data.addPtr,m_data.addInt); }
	void _cacheSurfacesRemoved(int nSurfaces, Surface* pSurfaces[]) override
	{
		wg_obj * pObjects = (wg_obj*) GfxBase::memStackAlloc(sizeof(wg_obj)*nSurfaces);
		
		for( int i = 0 ; i < nSurfaces ; i++ )
			pObjects[i] = static_cast<Object*>(pSurfaces[i]);
		
		m_data.removeFunc(nSurfaces,pObjects,m_data.removePtr,m_data.removeInt);
		
		GfxBase::memStackFree(sizeof(wg_obj)*nSurfaces);
	}
	void _cacheCleared() override { m_data.clearFunc( m_data.addPtr, m_data.addInt ); }
	
	wg_cacheListenerBP m_data;
																	
};


wg_obj wg_createBitmapCache( int maxSize, wg_obj surfaceFactory )
{
	SurfaceFactory * pFactory = surfaceFactory == 0 ? nullptr : static_cast<SurfaceFactory*>(reinterpret_cast<Object*>(surfaceFactory));

	auto p = BitmapCache::create(maxSize,pFactory);
	p->retain();
	return static_cast<Object*>(p.rawPtr());
}

void wg_setCacheLimit(wg_obj bitmapCache, int maxBytes)
{
	getPtr(bitmapCache)->setLimit(maxBytes);
}

int wg_cacheLimit(wg_obj bitmapCache)
{
	return getPtr(bitmapCache)->limit();
}

int wg_cacheSize(wg_obj bitmapCache)
{
	return getPtr(bitmapCache)->size();
}

void wg_clearCache(wg_obj bitmapCache)
{
	getPtr(bitmapCache)->clear();
}

wg_cacheListener wg_addCacheListener(wg_obj bitmapCache, wg_cacheListenerBP blueprint )
{
	auto pListener = new Listener();
	pListener->m_data = blueprint;
	getPtr(bitmapCache)->addListener(pListener);

	return pListener;
}

void wg_removeCacheListener(wg_obj bitmapCache, wg_cacheListener listener )
{
	auto pListener = static_cast<Listener*>(listener);
	
	getPtr(bitmapCache)->removeListener(pListener);
	delete pListener;
}

wg_cacheSlot wg_getCacheSlot(wg_obj bitmapCache, wg_sizeI size)
{
	Surface_p	pSurf;
	CoordI		coord;
	
	std::tie(pSurf,coord) = getPtr(bitmapCache)->getCacheSlot({size.w,size.h});

	return {static_cast<Object*>(pSurf.rawPtr()),{coord.x,coord.y}};
}

int wg_getNbCacheSurfaces(wg_obj bitmapCache)
{
	return getPtr(bitmapCache)->nbSurfaces();
}


int wg_getCacheSurfaces(wg_obj bitmapCache, int maxSurfaces, wg_obj * pArray )
{
	auto surfVector = getPtr(bitmapCache)->getSurfaces();
	int amount = std::min(maxSurfaces, (int)surfVector.size());
	
	for( int i = 0 ; i < amount ; i++ )
		pArray[i] = surfVector[i].rawPtr();
	
	return amount;
}
