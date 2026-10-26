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
#ifndef WG_GLWARPMAPFACTORY_DOT_H
#define	WG_GLWARPMAPFACTORY_DOT_H
#pragma once

#include <wg_warpmapfactory.h>

namespace wg
{
	class GlWarpmapFactory;
	typedef	StrongPtr<GlWarpmapFactory>	GlWarpmapFactory_p;
	typedef	WeakPtr<GlWarpmapFactory>	GlWarpmapFactory_wp;

	//____ GlWarpmapFactory ___________________________________________________

	class GlWarpmapFactory : public WarpmapFactory
	{
	public:

		//.____ Creation __________________________________________

		static GlWarpmapFactory_p	create() { return GlWarpmapFactory_p(new GlWarpmapFactory()); }

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Warpmap_p createWarpmap( const Warpmap::Blueprint& blueprint ) override;
		Warpmap_p createWarpmap( const Warpmap::Blueprint& blueprint, const CoordF * pNodes, int pitch = 0 ) override;

	protected:
		GlWarpmapFactory() {}
		virtual ~GlWarpmapFactory() {}
	};

} // namespace wg
#endif // WG_GLWARPMAPFACTORY_DOT_H
