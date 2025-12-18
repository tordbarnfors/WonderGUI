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
#include <wg_bitmapcache.h>
#include <wg_gfxdevice.h>
#include <wg_gfxbase.h>

#include <algorithm>

namespace wg
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

	BitmapCache_p BitmapCache::create( int maxSize, SurfaceFactory * pFactory )
	{
		auto * pCache = new BitmapCache( maxSize, pFactory );
		return BitmapCache_p(pCache);
	}

	//____ constructor ____________________________________________________________

	BitmapCache::BitmapCache( int size, SurfaceFactory * pFactory )
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
			GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::InvalidParam, "You can not set cache to a negative size!", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
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
			m_cacheSurfaces[i].clear();

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

	std::tuple<Surface_p, CoordI> BitmapCache::getCacheSlot( SizeI size )
	{
		int category = size.h > 128 ? 9 : s_sizeToCategory[size.h];

		CacheSurf * pCacheSurf;

		if( category == 9  )
			pCacheSurf = _addCacheSurface( category, size.w, size.h );
		else if(m_cacheSurfaces[category].empty())
			pCacheSurf = _addCacheSurface(category, 1024, s_categoryHeight[category]);
		else
		{
			auto& ref = m_cacheSurfaces[category].back();
			
			if( ref.capacity - ref.used >= size.w )
				pCacheSurf = &ref;
			else
				pCacheSurf = _addCacheSurface( category, 1024, s_categoryHeight[category]);
		}
		
		int ofs = pCacheSurf->used;
		pCacheSurf->used += size.w;
		return std::make_tuple(pCacheSurf->pSurface, CoordI(ofs,0));
	}

	//____ getSurfaces() _________________________________________________________

	std::vector<Surface_p> BitmapCache::getSurfaces() const
	{
		std::vector<Surface_p>	output;
		
		for( const std::vector<CacheSurf>& vec : m_cacheSurfaces )
		{
			for( auto& entry : vec )
				output.push_back(entry.pSurface);
		}
		
		return output;
	}

	//____ _addCacheSurface() __________________________________________________

	BitmapCache::CacheSurf * BitmapCache::_addCacheSurface( int category, int width, int height )
	{
		auto pFactory = m_pFactory ? m_pFactory : GfxBase::defaultSurfaceFactory();
		
		Surface_p pSurf;
		
		if( category == 9 )
			pSurf = pFactory->createSurface( WGBP(Surface, _.size = {width,height}, _.format = PixelFormat::Alpha_8) );
		else
			pSurf = pFactory->createSurface( WGBP(Surface, _.size = {width,s_categoryHeight[category]}, _.format = PixelFormat::Alpha_8) );

		m_cacheSize += width * height;

		m_cacheSurfaces[category].emplace_back(pSurf,width,m_surfacesCreated++);
		return &m_cacheSurfaces[category].back();
	}

	//____ _truncateCache() ___________________________________________________

	void BitmapCache::_truncateCache(int maxSize)
	{
		Surface*	removedSurfaces[32];
		int			nRemovedSurfaces = 0;

		// Remove cache entries

		while (m_cacheSize > maxSize && nRemovedSurfaces < 32 )
		{
			// Find category with oldest cache entry

			uint32_t lowestCreationNb = m_surfacesCreated;
			int oldestCat = -1;
			for (int cat = 0; cat < 10; cat++)
			{
				if (!m_cacheSurfaces[cat].empty() && m_cacheSurfaces[cat].front().creationNb < lowestCreationNb)
					oldestCat = cat;
			}

			auto& toRemove = m_cacheSurfaces[oldestCat].front();

			// Decreaste s_cacheSize

			SizeI pixels = toRemove.pSurface->pixelSize();
			m_cacheSize -= pixels.w * pixels.h;

			// Remove surface and add to list of removed.

			removedSurfaces[nRemovedSurfaces++] = toRemove.pSurface;
			m_cacheSurfaces[oldestCat].erase(m_cacheSurfaces[oldestCat].begin());
		}

		// Remove references to bitmaps in removed cache surfaces

		for (auto pListener : m_listeners)
			pListener->_cacheSurfacesRemoved(nRemovedSurfaces, removedSurfaces);

		// Continue recursively if we need to remove more than we could in this round.

		if (m_cacheSize > maxSize)
			_truncateCache(maxSize);
	}

} // namespace wg
