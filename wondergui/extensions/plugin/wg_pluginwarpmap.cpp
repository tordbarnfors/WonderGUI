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
	// Initialized with an empty Blueprint, then given the geometry and content of the host's Warpmap.

	PluginWarpmap::PluginWarpmap( wax_obj object )
	{
		PluginCalls::object->retain(object);
		m_cWarpmap = object;

		auto pCalls = PluginCalls::warpmap;

		wax_sizeI size	= pCalls->warpmapPixelSize(object);
		m_size			= { size.w, size.h };
		m_nVertices		= std::max(0, pCalls->warpmapVertexCount(object));
		m_nTriangles	= std::max(0, pCalls->warpmapTriangleCount(object));
		m_uPeriod		= pCalls->warpmapUPeriod(object);
		m_vPeriod		= pCalls->warpmapVPeriod(object);

		m_pVertices = new Vertex[m_nVertices];
		m_pTriangles = new Triangle[m_nTriangles];

		pCalls->exportWarpmapVertices(object, 0, m_nVertices, (wax_warpVertex*) m_pVertices);
		pCalls->exportWarpmapTriangles(object, 0, m_nTriangles, (wax_warpTriangle*) m_pTriangles);

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

	//____ _updated() _________________________________________________________
	// Everything at once goes to the host in one call, as do periods alone.

	void PluginWarpmap::_updated( int firstVertex, int nVertices, int firstTriangle, int nTriangles )
	{
		if (!m_bConstructed)
			return;

		auto pCalls = PluginCalls::warpmap;

		bool bAllVertices = firstVertex == 0 && nVertices == m_nVertices;
		bool bAllTriangles = firstTriangle == 0 && nTriangles == m_nTriangles;

		if ((bAllVertices || nVertices == 0) && (bAllTriangles || nTriangles == 0))
		{
			pCalls->importWarpmapMesh(m_cWarpmap, nVertices > 0 ? (const wax_warpVertex*) m_pVertices : nullptr,
									  nTriangles > 0 ? (const wax_warpTriangle*) m_pTriangles : nullptr, m_uPeriod, m_vPeriod);
			return;
		}

		if (nVertices > 0)
			pCalls->importWarpmapVertices(m_cWarpmap, firstVertex, nVertices, (const wax_warpVertex*) (m_pVertices + firstVertex));

		if (nTriangles > 0)
			pCalls->importWarpmapTriangles(m_cWarpmap, firstTriangle, nTriangles, (const wax_warpTriangle*) (m_pTriangles + firstTriangle));

		if (m_uPeriod != pCalls->warpmapUPeriod(m_cWarpmap) || m_vPeriod != pCalls->warpmapVPeriod(m_cWarpmap))
			pCalls->setWarpmapPeriods(m_cWarpmap, m_uPeriod, m_vPeriod);
	}

} // namespace wg
