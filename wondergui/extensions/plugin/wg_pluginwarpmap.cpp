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
#include <wg_plugincalls.h>
#include <wg_pluginwarpmap.h>

#include <algorithm>

namespace wg
{
	const TypeInfo PluginWarpmap::TYPEINFO = { "PluginWarpmap", &Warpmap::TYPEINFO };

	//____ create() ___________________________________________________________

	PluginWarpmap_p PluginWarpmap::create( wax_obj object )
	{
		if (!object)
			return nullptr;

		return PluginWarpmap_p(new PluginWarpmap(object));
	}

	//____ constructor ________________________________________________________
	// Initialized with an empty Blueprint, then given the geometry and nodes of the host's Warpmap.

	PluginWarpmap::PluginWarpmap( wax_obj object )
	{
		PluginCalls::object->retain(object);
		m_cWarpmap = object;

		wax_sizeI size	= PluginCalls::warpmap->warpmapPixelSize(object);
		m_size			= { size.w, size.h };
		m_cellSize		= std::max(1, PluginCalls::warpmap->warpmapCellSize(object));
		m_uPeriod		= PluginCalls::warpmap->warpmapUPeriod(object);
		m_vPeriod		= PluginCalls::warpmap->warpmapVPeriod(object);

		m_nodes.w = (m_size.w + m_cellSize - 1) / m_cellSize + 1;
		m_nodes.h = (m_size.h + m_cellSize - 1) / m_cellSize + 1;

		m_pNodes = new CoordSPX[m_nodes.w * m_nodes.h];

		wax_rectI all = { 0, 0, m_nodes.w, m_nodes.h };
		PluginCalls::warpmap->exportWarpmapNodes(object, &all, (wax_coordSPX*) m_pNodes, 0);

		m_bConstructed = true;
	}

	//____ destructor _________________________________________________________

	PluginWarpmap::~PluginWarpmap()
	{
		PluginCalls::object->release(m_cWarpmap);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginWarpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setPeriods() _______________________________________________________
	// The host gets the periods before the nodes normalized into them.

	bool PluginWarpmap::setPeriods( spx uPeriod, spx vPeriod )
	{
		if (!PluginCalls::warpmap->setWarpmapPeriods(m_cWarpmap, uPeriod, vPeriod))
			return false;

		return Warpmap::setPeriods(uPeriod, vPeriod);
	}

	//____ _nodesUpdated() ____________________________________________________

	void PluginWarpmap::_nodesUpdated( const RectI& region )
	{
		if (!m_bConstructed)
			return;

		wax_rectI cRegion = { region.x, region.y, region.w, region.h };
		PluginCalls::warpmap->importWarpmapNodes(m_cWarpmap, &cRegion, (const wax_coordSPX*) (m_pNodes + region.y * m_nodes.w + region.x), m_nodes.w);
	}

} // namespace wg
