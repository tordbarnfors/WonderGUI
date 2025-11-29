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
#ifndef	WG_BITMAPCACHE_DOT_H
#define	WG_BITMAPCACHE_DOT_H
#pragma once

#include <stddef.h>
#include <vector>

#include <wg_pointers.h>
#include <wg_gfxtypes.h>
#include <wg_surface.h>
#include <wg_surfacefactory.h>

namespace wg
{

	class BitmapCache;
	typedef	StrongPtr<BitmapCache>		BitmapCache_p;
	typedef	WeakPtr<BitmapCache>		BitmapCache_wp;


	class BitmapCacheListener
	{
	public:
		virtual void _cacheSurfaceAdded(Surface* pSurface) = 0;
		virtual void _cacheSurfacesRemoved(int nSurfaces, Surface* pSurfaces[]) = 0;
		virtual void _cacheCleared() = 0;
	};


	class BitmapCache : public Object
	{
	public:
		//.____ Creation __________________________________________

		static BitmapCache_p	create( int maxSize, SurfaceFactory * pFactory = nullptr );

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
		
		std::tuple<Surface_p, CoordI> 	getCacheSlot( SizeI size );
		
		inline int						nbSurfaces() const { return int(m_cacheSurfaces->size()); }
		std::vector<Surface_p>			getSurfaces() const;
		
		
	protected:
		BitmapCache( int maxSize, SurfaceFactory * pFactory );
		virtual ~BitmapCache();

		struct CacheSurf
		{
			CacheSurf( Surface * _pSurface, int _capacity, uint32_t creationNb ) :
				pSurface(_pSurface),
				capacity(_capacity),
				used(0)
				{
					this->creationNb = creationNb;
				}
				
			
			Surface_p	pSurface;
			int			capacity;
			int			used;
			
			uint32_t	creationNb;
		};

		CacheSurf*		_addCacheSurface(int category, int width, int height);
		void			_truncateCache(int maxSize);

		
		SurfaceFactory_p		m_pFactory;
		
		std::vector<BitmapCacheListener*>	m_listeners;	// List of alive listeners.

		std::vector<CacheSurf>	m_cacheSurfaces[10];		// The nine combined surfaces + the tenth vector with all character-specific surfaces.

		uint32_t				m_surfacesCreated = 0;		// Increments and provides m_surfCreationNb for each new surface created.
		int						m_cacheSize = 0;			// Size in bytes of existing cacheSurfaces. Used and unused cache space.
		int						m_cacheLimit;				// Maximum size of cache in bytes. 0 = no limit.

		const static uint8_t	s_categoryHeight[9];		// Pixel heights for cache surfaces in each category.
		const static uint8_t	s_sizeToCategory[129];		// Table for grouping sizes into categories.
	};


} // namespace wg
#endif //WG_BITMAPCACHE_DOT_H
