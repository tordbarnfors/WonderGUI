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
#ifndef WAX_DX12WARPMAPFACTORY_DOT_H
#define	WAX_DX12WARPMAPFACTORY_DOT_H
#pragma once

#include <wax_warpmapfactory.h>

namespace wax
{

	class DX12WarpmapFactory;
	typedef	StrongPtr<DX12WarpmapFactory>	DX12WarpmapFactory_p;
	typedef	WeakPtr<DX12WarpmapFactory>	DX12WarpmapFactory_wp;


	//____ DX12WarpmapFactory _________________________________________________

	class DX12WarpmapFactory : public WarpmapFactory
	{
	public:

		//.____ Creation __________________________________________

		static DX12WarpmapFactory_p	create() { return DX12WarpmapFactory_p(new DX12WarpmapFactory()); }

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Warpmap_p createWarpmap(const Warpmap::Blueprint& blueprint) override;
		Warpmap_p createWarpmap(const Warpmap::Blueprint& blueprint, const CoordSPX * pNodes, int pitch = 0) override;

	protected:
		DX12WarpmapFactory() {}
		virtual ~DX12WarpmapFactory() {}
	};

} // namespace wax
#endif // WAX_DX12WARPMAPFACTORY_DOT_H
