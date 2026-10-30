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
#ifndef WG_WARPMAPFACTORY_DOT_H
#define	WG_WARPMAPFACTORY_DOT_H
#pragma once

#include <wg_warpmap.h>

namespace wg
{

	class WarpmapFactory;
	typedef	StrongPtr<WarpmapFactory>	WarpmapFactory_p;
	typedef	WeakPtr<WarpmapFactory>	WarpmapFactory_wp;


	//____ WarpmapFactory _______________________________________________________
	/**
	 * @brief Factory class for creating warpmaps.
	 *
	 * WarpmapFactory is the base class for all warpmap factories. The warpmap factories
	 * are used by WonderGUI objects that needs to dynamically create warpmaps as
	 * part of their operation, such as CircularWarp.
	 *
	 **/
	class WarpmapFactory : public Object
	{
	public:

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		virtual Warpmap_p createWarpmap( const Warpmap::Blueprint& blueprint ) = 0;
		virtual Warpmap_p createWarpmap( const Warpmap::Blueprint& blueprint, const CoordSPX * pNodes, int pitch = 0 ) = 0;

	protected:
		WarpmapFactory() {}
		virtual ~WarpmapFactory() {}
	};

} // namespace wg
#endif // WG_WARPMAPFACTORY_DOT_H
