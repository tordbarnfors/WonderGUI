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
#include <wax_c_bitmapcache.h>
#include <wax_bitmapcache.h>
#include <wax_base.h>

using namespace wax;

inline BitmapCache* getPtr(wax_obj obj) {
	return static_cast<BitmapCache*>(reinterpret_cast<Object*>(obj));
}

class Listener : public BitmapCacheListener
{
public:
	Listener() {};
	virtual ~Listener() = default;

	void _cacheBitmapAdded(Bitmap* pBitmap) override { m_data.addFunc(static_cast<Object*>(pBitmap),m_data.addPtr,m_data.addInt); }
	void _cacheBitmapsRemoved(int nBitmaps, Bitmap* pBitmaps[]) override
	{
		wax_obj * pObjects = (wax_obj*) Base::memStackAlloc(sizeof(wax_obj)*nBitmaps);
		
		for( int i = 0 ; i < nBitmaps ; i++ )
			pObjects[i] = static_cast<Object*>(pBitmaps[i]);
		
		m_data.removeFunc(nBitmaps,pObjects,m_data.removePtr,m_data.removeInt);
		
		Base::memStackFree(sizeof(wax_obj)*nBitmaps);
	}
	void _cacheCleared() override { m_data.clearFunc( m_data.addPtr, m_data.addInt ); }
	
	wax_cacheListenerBP m_data;
																	
};


wax_obj wax_createBitmapCache( int maxSize, wax_obj bitmapFactory )
{
	BitmapFactory * pFactory = bitmapFactory == 0 ? nullptr : static_cast<BitmapFactory*>(reinterpret_cast<Object*>(bitmapFactory));

	auto p = BitmapCache::create(maxSize,pFactory);
	p->retain();
	return static_cast<Object*>(p.rawPtr());
}

void wax_setCacheLimit(wax_obj bitmapCache, int maxBytes)
{
	getPtr(bitmapCache)->setLimit(maxBytes);
}

int wax_cacheLimit(wax_obj bitmapCache)
{
	return getPtr(bitmapCache)->limit();
}

int wax_cacheSize(wax_obj bitmapCache)
{
	return getPtr(bitmapCache)->size();
}

void wax_clearCache(wax_obj bitmapCache)
{
	getPtr(bitmapCache)->clear();
}

wax_cacheListener wax_addCacheListener(wax_obj bitmapCache, wax_cacheListenerBP blueprint )
{
	auto pListener = new Listener();
	pListener->m_data = blueprint;
	getPtr(bitmapCache)->addListener(pListener);

	return pListener;
}

void wax_removeCacheListener(wax_obj bitmapCache, wax_cacheListener listener )
{
	auto pListener = static_cast<Listener*>(listener);
	
	getPtr(bitmapCache)->removeListener(pListener);
	delete pListener;
}

wax_cacheSlot wax_getCacheSlot(wax_obj bitmapCache, wax_sizeI size)
{
	Bitmap_p	pBitmap;
	CoordI		coord;
	
	std::tie(pBitmap,coord) = getPtr(bitmapCache)->getCacheSlot({size.w,size.h});

	return {static_cast<Object*>(pBitmap.rawPtr()),{coord.x,coord.y}};
}

int wax_getNbCacheBitmaps(wax_obj bitmapCache)
{
	return getPtr(bitmapCache)->nbBitmaps();
}


int wax_getCacheBitmaps(wax_obj bitmapCache, int maxBitmaps, wax_obj * pArray )
{
	auto bitmapVector = getPtr(bitmapCache)->getBitmaps();
	int amount = std::min(maxBitmaps, (int)bitmapVector.size());
	
	for( int i = 0 ; i < amount ; i++ )
		pArray[i] = bitmapVector[i].rawPtr();
	
	return amount;
}
