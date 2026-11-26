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
#ifndef	WAX_BITMAPCACHE_DOT_H
#define	WAX_BITMAPCACHE_DOT_H
#pragma once

#include <stddef.h>
#include <vector>

#include <wax_pointers.h>
#include <wax_types.h>
#include <wax_bitmap.h>
#include <wax_bitmapfactory.h>

namespace wax
{

	class BitmapCache;
	typedef	StrongPtr<BitmapCache>		BitmapCache_p;
	typedef	WeakPtr<BitmapCache>		BitmapCache_wp;


	class BitmapCacheListener
	{
	public:
		virtual void _cacheBitmapAdded(Bitmap* pBitmap) = 0;
		virtual void _cacheBitmapsRemoved(int nBitmaps, Bitmap* pBitmaps[]) = 0;
		virtual void _cacheCleared() = 0;
	};


	class BitmapCache : public Object
	{
	public:
		//.____ Creation __________________________________________

		static BitmapCache_p	create( int maxSize, BitmapFactory * pFactory = nullptr );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ___________________________________________________
		
		void 	setLimit(int maxBytes);
		int		limit() const { return m_cacheLimit; }

		int		size() const { return m_cacheSize; }

		void	clear();

		//.____ Misc ___________________________________________________________

		void		addListener( BitmapCacheListener * pListener );
		void		removeListener( BitmapCacheListener * pListener );
		
		std::tuple<Bitmap_p, CoordI> 	getCacheSlot( SizeI size );
		
		inline int						nbBitmaps() const { return int(m_cacheBitmaps->size()); }
		std::vector<Bitmap_p>			getBitmaps() const;
		
		
	protected:
		BitmapCache( int maxSize, BitmapFactory * pFactory );
		virtual ~BitmapCache();

		struct CacheBitmap
		{
			CacheBitmap( Bitmap * _pBitmap, int _capacity, uint32_t creationNb ) :
				pBitmap(_pBitmap),
				capacity(_capacity),
				used(0)
				{
					this->creationNb = creationNb;
				}
				
			
			Bitmap_p	pBitmap;
			int			capacity;
			int			used;
			
			uint32_t	creationNb;
		};

		CacheBitmap*		_addCacheBitmap(int category, int width, int height);
		void			_truncateCache(int maxSize);

		
		BitmapFactory_p		m_pFactory;
		
		std::vector<BitmapCacheListener*>	m_listeners;	// List of alive listeners.

		std::vector<CacheBitmap>	m_cacheBitmaps[10];		// The nine combined bitmaps + the tenth vector with all character-specific bitmaps.

		uint32_t				m_bitmapsCreated = 0;		// Increments and provides m_bitmapCreationNb for each new bitmap created.
		int						m_cacheSize = 0;			// Size in bytes of existing cacheBitmaps. Used and unused cache space.
		int						m_cacheLimit;				// Maximum size of cache in bytes. 0 = no limit.

		const static uint8_t	s_categoryHeight[9];		// Pixel heights for cache bitmaps in each category.
		const static uint8_t	s_sizeToCategory[129];		// Table for grouping sizes into categories.
	};


} // namespace wax
#endif //WAX_BITMAPCACHE_DOT_H
