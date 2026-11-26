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
#include <wg_base.h>

#include <wg_pluginbitmapcache.h>
#include <wg_plugincalls.h>

#include <cassert>
#include <algorithm>


namespace wg
{

	const TypeInfo	PluginBitmapCache::TYPEINFO = { "PluginBitmapCache", &Object::TYPEINFO };


	//____ constructor ____________________________________________________________

	PluginBitmapCache::PluginBitmapCache( wax_obj bitmapCache )
	{
		m_cCache = bitmapCache;
		PluginCalls::object->retain(bitmapCache);

		// Retrieve and wrap existing cache bitmaps

		int nbBitmaps = PluginCalls::bitmapCache->getNbCacheBitmaps(bitmapCache);

		if (nbBitmaps > 0)
		{
			// Reserve buffer, get bitmap objects

			int mem = nbBitmaps * sizeof(wax_obj);

			wax_obj * pBitmaps = (wax_obj*) Base::memStackAlloc(mem);
			PluginCalls::bitmapCache->getCacheBitmaps(bitmapCache, nbBitmaps, pBitmaps);


			int highestNumber = 0;

			if (PluginCalls::bitmap->getBitmapIdentity(pBitmaps[0]) == 0)
			{
				// We are first PluginBitmapCache, we start numbering existing ones

				for (int i = 0; i < nbBitmaps; i++)
					PluginCalls::bitmap->setBitmapIdentity(pBitmaps[i], i + 1);

				highestNumber = nbBitmaps;
			}
			else
			{
				// Already numbered, we need to find highest number for stack size.

				for (int i = 0; i < nbBitmaps; i++)
					highestNumber = std::max( PluginCalls::bitmap->getBitmapIdentity(pBitmaps[i]), highestNumber);
			}

			// Resize vector, create and insert wrappers

			m_bitmaps.resize(highestNumber);

			for (int i = 0; i < nbBitmaps; i++)
			{
				wax_obj obj = pBitmaps[i];

				int ofs = PluginCalls::bitmap->getBitmapIdentity(obj) - 1;
				m_bitmaps[ofs] = PluginBitmap::create(obj);
			}

			// Release buffer

			Base::memStackFree(mem);
		}

		// Set up listener.

		wax_cacheListenerBP	bp;


		bp.addFunc = [](wax_obj bitmap, void* pData, int data) { ((PluginBitmapCache*)pData)->_bitmapAdded(bitmap); };
		bp.addPtr = this;

		bp.removeFunc = [](int nBitmaps, wax_obj* pBitmaps, void* pData, int data) { ((PluginBitmapCache*)pData)->_bitmapsRemoved(nBitmaps, pBitmaps); };
		bp.removePtr = this;

		bp.clearFunc = [](void* pData, int data) { ((PluginBitmapCache*)pData)->_cleared(); };
		bp.clearPtr = this;

		m_listenerId = PluginCalls::bitmapCache->addCacheListener(bitmapCache, bp);
	}

	//____ Destructor _____________________________________________________________

	PluginBitmapCache::~PluginBitmapCache()
	{
		PluginCalls::bitmapCache->removeCacheListener(m_cCache,m_listenerId);

		PluginCalls::object->release(m_cCache);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginBitmapCache::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setLimit() ____________________________________________________________

	void PluginBitmapCache::setLimit(int maxBytes)
	{
		PluginCalls::bitmapCache->setCacheLimit(m_cCache, maxBytes);
	}

	//____ limit() ____________________________________________________________

	int PluginBitmapCache::limit() const
	{
		return PluginCalls::bitmapCache->cacheLimit(m_cCache);
	}

	//____ size() _____________________________________________________________

	int PluginBitmapCache::size() const
	{
		return PluginCalls::bitmapCache->cacheSize(m_cCache);
	}

	//____ clear() _______________________________________________________________

	void PluginBitmapCache::clear()
	{
		PluginCalls::bitmapCache->clearCache(m_cCache);
	}

	//____ addListener() ______________________________________________________

	void PluginBitmapCache::addListener(BitmapCacheListener* pListener)
	{
		m_listeners.push_back(pListener);
	}

	//____ removeListener() ___________________________________________________

	void PluginBitmapCache::removeListener(BitmapCacheListener* pListener)
	{
		if (pListener)
		{
			m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), pListener));
			if (m_listeners.empty())
				clear();
		}
	}

	//____ getCacheSlot() _____________________________________________________

	std::tuple<Bitmap_p, CoordI> PluginBitmapCache::getCacheSlot(SizeI size)
	{
		auto slot = PluginCalls::bitmapCache->getCacheSlot(m_cCache, { size.w,size.h });

		int ofs = PluginCalls::bitmap->getBitmapIdentity(slot.bitmap) - 1;

		return std::make_tuple(m_bitmaps[ofs], CoordI( slot.coord.x, slot.coord.y ) );
	}

	//____ getBitmaps() ______________________________________________________

	std::vector<Bitmap_p> PluginBitmapCache::getBitmaps() const
	{
		std::vector<Bitmap_p> vec;

		for ( auto& bitmap : m_bitmaps )
		{
			if (bitmap)
				vec.push_back(bitmap);
		}

		return vec;
	}

	//____ wrapperBitmap() ___________________________________________________

	PluginBitmap* PluginBitmapCache::wrapperBitmap(wax_obj bitmapObject)
	{
		int offset = PluginCalls::bitmap->getBitmapIdentity(bitmapObject) -1;

		assert(offset > 0 && offset < m_bitmaps.size() && m_bitmaps[offset] != nullptr );

		return m_bitmaps[offset];
	}

	//____ _bitmapAdded() ____________________________________________________

	void PluginBitmapCache::_bitmapAdded(wax_obj bitmap)
	{
		int ofs = PluginCalls::bitmap->getBitmapIdentity(bitmap) - 1;
		if (ofs < 0)
		{
			// We are the first PluginBitmapCache receiving the callback. Let's decide and offset.

			int ofs = 0;
			for ( auto& bitmap : m_bitmaps )
			{
				if (!bitmap)
					break;
				ofs++;
			}

			PluginCalls::bitmap->setBitmapIdentity(bitmap, ofs + 1);
		}

		if (ofs >= m_bitmaps.size())
			m_bitmaps.resize(ofs + 1);

		m_bitmaps[ofs] = PluginBitmap::create(bitmap);
	}

	//____ _bitmapsRemoved() _________________________________________________

	void PluginBitmapCache::_bitmapsRemoved(int nBitmaps, wax_obj* pBitmaps)
	{
		for (int i = 0; i < nBitmaps; i++)
		{
			int ofs = PluginCalls::bitmap->getBitmapIdentity(pBitmaps[i]) - 1;
			m_bitmaps[ofs] = nullptr;
		}
	}

	//_____ _cleared() ________________________________________________________

	void PluginBitmapCache::_cleared()
	{
		m_bitmaps.clear();
	}




} // namespace wg
