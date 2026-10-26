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
#ifndef	WG_PLUGINBITMAPCACHE_DOT_H
#define	WG_PLUGINBITMAPCACHE_DOT_H
#pragma once

#include <wax_bitmapcache.h>
#include <wg_pluginsurface.h>
#include <wax_c_bitmapcache.h>
#include <wg_types.h>
#include <wg_c_types.h>

namespace wg
{

	class PluginBitmapCache;
	typedef	StrongPtr<PluginBitmapCache>		PluginBitmapCache_p;
	typedef	WeakPtr<PluginBitmapCache>		PluginBitmapCache_wp;



	class PluginBitmapCache : public Object
	{
	public:
		//.____ Creation __________________________________________

		static PluginBitmapCache_p	create(wax_obj bitmapCache);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ___________________________________________________

		void 	setLimit(int maxBytes);
		int		limit() const;

		int		size() const;

		void	clear();

		//.____ Misc ___________________________________________________________

		void		addListener(BitmapCacheListener* pListener);
		void		removeListener(BitmapCacheListener* pListener);

		std::tuple<Surface_p, CoordI> 	getCacheSlot(SizeI size);
		std::vector<Surface_p>			getSurfaces() const;

		PluginSurface *  wrapperSurface(wax_obj surfaceObject);


	protected:
		PluginBitmapCache(wax_obj bitmapCache);
		virtual ~PluginBitmapCache();

		void _surfaceAdded(wax_obj surface);
		void _surfacesRemoved(int nSurfaces, wax_obj * pSurfaces);
		void _cleared();


		wax_obj				m_cCache;
		wax_cacheListener	m_listenerId;


		std::vector<BitmapCacheListener*>	m_listeners;	// List of alive listeners.

		std::vector<PluginSurface_p>			m_surfaces;

	};

} // namespace wg
#endif //WG_PLUGINBITMAPCACHE_DOT_H
