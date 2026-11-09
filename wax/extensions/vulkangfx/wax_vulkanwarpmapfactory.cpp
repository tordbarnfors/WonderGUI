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

#include <wax_vulkanwarpmapfactory.h>
#include <wax_vulkanwarpmap.h>

namespace wax
{

	const TypeInfo VulkanWarpmapFactory::TYPEINFO = { "VulkanWarpmapFactory", &WarpmapFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& VulkanWarpmapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createWarpmap() ____________________________________________________

	Warpmap_p VulkanWarpmapFactory::createWarpmap(const Warpmap::Blueprint& blueprint)
	{
		return VulkanWarpmap::create(blueprint);
	}

	Warpmap_p VulkanWarpmapFactory::createWarpmap(const Warpmap::Blueprint& blueprint, const Warpmap::Vertex * pVertices, const Warpmap::Triangle * pTriangles)
	{
		return VulkanWarpmap::create(blueprint, pVertices, pTriangles);
	}

} // namespace wax
