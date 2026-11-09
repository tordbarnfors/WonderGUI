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
#ifndef WAX_VulkanWARPMAPFACTORY_DOT_H
#define	WAX_VulkanWARPMAPFACTORY_DOT_H
#pragma once

#include <wax_warpmapfactory.h>

namespace wax
{

	class VulkanWarpmapFactory;
	typedef	StrongPtr<VulkanWarpmapFactory>	VulkanWarpmapFactory_p;
	typedef	WeakPtr<VulkanWarpmapFactory>	VulkanWarpmapFactory_wp;


	//____ VulkanWarpmapFactory _________________________________________________

	class VulkanWarpmapFactory : public WarpmapFactory
	{
	public:

		//.____ Creation __________________________________________

		static VulkanWarpmapFactory_p	create() { return VulkanWarpmapFactory_p(new VulkanWarpmapFactory()); }

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Warpmap_p createWarpmap(const Warpmap::Blueprint& blueprint) override;
		Warpmap_p createWarpmap(const Warpmap::Blueprint& blueprint, const Warpmap::Vertex * pVertices, const Warpmap::Triangle * pTriangles) override;

	protected:
		VulkanWarpmapFactory() {}
		virtual ~VulkanWarpmapFactory() {}
	};

} // namespace wax
#endif // WAX_VulkanWARPMAPFACTORY_DOT_H
