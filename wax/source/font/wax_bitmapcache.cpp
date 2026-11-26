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
#include <wax_bitmapcache.h>
#include <wax_device.h>
#include <wax_base.h>

#include <algorithm>

namespace wax
{

	const TypeInfo BitmapCache::TYPEINFO = { "BitmapCache", &Object::TYPEINFO };

	const uint8_t		BitmapCache::s_categoryHeight[9] = { 8, 12, 16, 24, 32, 48, 64, 96, 128 };

	const uint8_t		BitmapCache::s_sizeToCategory[129] = { 0,0,0,0,0,0,0,0,0,
																1,1,1,1,
																2,2,2,2,
																3,3,3,3,3,3,3,3,
																4,4,4,4,4,4,4,4,
																5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
																6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
																7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
																8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8 };


	//____ Create _________________________________________________________________

	BitmapCache_p BitmapCache::create( int maxSize, BitmapFactory * pFactory )
	{
		auto * pCache = new BitmapCache( maxSize, pFactory );
		return BitmapCache_p(pCache);
	}

	//____ constructor ____________________________________________________________

	BitmapCache::BitmapCache( int size, BitmapFactory * pFactory )
	{
		m_cacheLimit	= size;
		m_pFactory		= pFactory;
	}

	//____ Destructor _____________________________________________________________

	BitmapCache::~BitmapCache()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BitmapCache::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setLimit() ____________________________________________________________

	void BitmapCache::setLimit(int maxBytes)
	{
		if (maxBytes < 0)
		{
			Base::reportError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "You can not set cache to a negative size!", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		m_cacheLimit = maxBytes;

		if (m_cacheLimit > 0 && m_cacheSize > m_cacheLimit)
			_truncateCache(m_cacheLimit);
	}

	//____ clear() _______________________________________________________________

	void BitmapCache::clear()
	{
		// Forget all cache references

		for (auto pListener : m_listeners)
			pListener->_cacheCleared();

		// Clear cache

		for (int i = 0; i < 10; i++)
			m_cacheBitmaps[i].clear();

		m_cacheSize = 0;
	}

	//____ addListener() ______________________________________________________

	void BitmapCache::addListener( BitmapCacheListener * pListener )
	{
		m_listeners.push_back(pListener);
	}

	//____ removeListener() ___________________________________________________

	void BitmapCache::removeListener( BitmapCacheListener * pListener )
	{
		if( pListener )
		{
			m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), pListener));
			if( m_listeners.empty() )
				clear();
		}
	}

	//____ getCacheSlot() _________________________________________________________

	std::tuple<Bitmap_p, CoordI> BitmapCache::getCacheSlot( SizeI size )
	{
		int category = size.h > 128 ? 9 : s_sizeToCategory[size.h];

		CacheBitmap * pCacheBitmap;

		if( category == 9  )
			pCacheBitmap = _addCacheBitmap( category, size.w, size.h );
		else if(m_cacheBitmaps[category].empty())
			pCacheBitmap = _addCacheBitmap(category, 1024, s_categoryHeight[category]);
		else
		{
			auto& ref = m_cacheBitmaps[category].back();
			
			if( ref.capacity - ref.used >= size.w )
				pCacheBitmap = &ref;
			else
				pCacheBitmap = _addCacheBitmap( category, 1024, s_categoryHeight[category]);
		}
		
		int ofs = pCacheBitmap->used;
		pCacheBitmap->used += size.w;
		return std::make_tuple(pCacheBitmap->pBitmap, CoordI(ofs,0));
	}

	//____ getBitmaps() _________________________________________________________

	std::vector<Bitmap_p> BitmapCache::getBitmaps() const
	{
		std::vector<Bitmap_p>	output;
		
		for( const std::vector<CacheBitmap>& vec : m_cacheBitmaps )
		{
			for( auto& entry : vec )
				output.push_back(entry.pBitmap);
		}
		
		return output;
	}

	//____ _addCacheBitmap() __________________________________________________

	BitmapCache::CacheBitmap * BitmapCache::_addCacheBitmap( int category, int width, int height )
	{
		auto pFactory = m_pFactory ? m_pFactory : Base::defaultBitmapFactory();
		
		Bitmap_p pBitmap;
		
		if( category == 9 )
			pBitmap = pFactory->createBitmap( WAXBP(Bitmap, _.size = {width,height}, _.format = PixelFormat::Alpha_8) );
		else
			pBitmap = pFactory->createBitmap( WAXBP(Bitmap, _.size = {width,s_categoryHeight[category]}, _.format = PixelFormat::Alpha_8) );

		m_cacheSize += width * height;

		m_cacheBitmaps[category].emplace_back(pBitmap,width,m_bitmapsCreated++);
		return &m_cacheBitmaps[category].back();
	}

	//____ _truncateCache() ___________________________________________________

	void BitmapCache::_truncateCache(int maxSize)
	{
		Bitmap*	removedBitmaps[32];
		int			nRemovedBitmaps = 0;

		// Remove cache entries

		while (m_cacheSize > maxSize && nRemovedBitmaps < 32 )
		{
			// Find category with oldest cache entry

			uint32_t lowestCreationNb = m_bitmapsCreated;
			int oldestCat = -1;
			for (int cat = 0; cat < 10; cat++)
			{
				if (!m_cacheBitmaps[cat].empty() && m_cacheBitmaps[cat].front().creationNb < lowestCreationNb)
					oldestCat = cat;
			}

			auto& toRemove = m_cacheBitmaps[oldestCat].front();

			// Decreaste s_cacheSize

			SizeI pixels = toRemove.pBitmap->pixelSize();
			m_cacheSize -= pixels.w * pixels.h;

			// Remove bitmap and add to list of removed.

			removedBitmaps[nRemovedBitmaps++] = toRemove.pBitmap;
			m_cacheBitmaps[oldestCat].erase(m_cacheBitmaps[oldestCat].begin());
		}

		// Remove references to bitmaps in removed cache bitmaps

		for (auto pListener : m_listeners)
			pListener->_cacheBitmapsRemoved(nRemovedBitmaps, removedBitmaps);

		// Continue recursively if we need to remove more than we could in this round.

		if (m_cacheSize > maxSize)
			_truncateCache(maxSize);
	}

} // namespace wax
