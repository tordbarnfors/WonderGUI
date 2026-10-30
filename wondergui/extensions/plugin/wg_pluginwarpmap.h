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
#ifndef	WG_PLUGINWARPMAP_DOT_H
#define	WG_PLUGINWARPMAP_DOT_H
#pragma once

#include <wax_warpmap.h>
#include <wg_types.h>

typedef void* wax_obj;

namespace wg
{
	class PluginWarpmap;
	typedef	StrongPtr<PluginWarpmap>	PluginWarpmap_p;
	typedef	WeakPtr<PluginWarpmap>		PluginWarpmap_wp;

	//____ PluginWarpmap _______________________________________________________
	/**
	 * @brief Warpmap of the host, used by a plugin.
	 *
	 * Nodes are kept on both sides, the host's for drawing and ours for the API. Changes
	 * are passed on to the host.
	 */

	class PluginWarpmap : public Warpmap
	{
	public:

		//.____ Creation __________________________________________

		static PluginWarpmap_p	create( wax_obj object );

		//.____ Identification __________________________________________

		const TypeInfo&	typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _______________________________________________________

		bool			setPeriods( spx uPeriod, spx vPeriod ) override;

		//.____ Misc _________________________________________________________

		inline wax_obj	cObject() { return m_cWarpmap; }

	private:
		PluginWarpmap( wax_obj object );
		~PluginWarpmap();

		void	_nodesUpdated( const RectI& region ) override;

		wax_obj	m_cWarpmap;
		bool	m_bConstructed = false;
	};

} // namespace wg
#endif //WG_PLUGINWARPMAP_DOT_H
