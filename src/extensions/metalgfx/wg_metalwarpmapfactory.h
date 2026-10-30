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
#ifndef WG_METALWARPMAPFACTORY_DOT_H
#define	WG_METALWARPMAPFACTORY_DOT_H
#pragma once

#include <wg_warpmapfactory.h>

namespace wg
{

	class MetalWarpmapFactory;
	typedef	StrongPtr<MetalWarpmapFactory>	MetalWarpmapFactory_p;
	typedef	WeakPtr<MetalWarpmapFactory>	MetalWarpmapFactory_wp;


	//____ MetalWarpmapFactory ________________________________________________

	class MetalWarpmapFactory : public WarpmapFactory
	{
	public:

		//.____ Creation __________________________________________

		static MetalWarpmapFactory_p	create() { return MetalWarpmapFactory_p(new MetalWarpmapFactory()); }

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Warpmap_p createWarpmap( const Warpmap::Blueprint& blueprint ) override;
		Warpmap_p createWarpmap( const Warpmap::Blueprint& blueprint, const CoordSPX * pNodes, int pitch = 0 ) override;

	protected:
		MetalWarpmapFactory() {}
		virtual ~MetalWarpmapFactory() {}
	};

} // namespace wg
#endif // WG_METALWARPMAPFACTORY_DOT_H
