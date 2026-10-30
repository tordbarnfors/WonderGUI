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
#ifndef WAX_STREAMWARPMAPFACTORY_DOT_H
#define	WAX_STREAMWARPMAPFACTORY_DOT_H
#pragma once

#include <wax_warpmapfactory.h>
#include <wax_streamencoder.h>

namespace wax
{
	class StreamWarpmapFactory;
	typedef	StrongPtr<StreamWarpmapFactory>	StreamWarpmapFactory_p;
	typedef	WeakPtr<StreamWarpmapFactory>	StreamWarpmapFactory_wp;

	//____ StreamWarpmapFactory ___________________________________________________

	class StreamWarpmapFactory : public WarpmapFactory
	{
	public:

		//.____ Creation __________________________________________

		static StreamWarpmapFactory_p	create( StreamEncoder * pEncoder ) { return StreamWarpmapFactory_p(new StreamWarpmapFactory(pEncoder)); }

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc _______________________________________________________

		Warpmap_p createWarpmap( const Warpmap::Blueprint& blueprint ) override;
		Warpmap_p createWarpmap( const Warpmap::Blueprint& blueprint, const CoordSPX * pNodes, int pitch = 0 ) override;

	protected:
		StreamWarpmapFactory( StreamEncoder * pEncoder ) : m_pEncoder(pEncoder) {}
		virtual ~StreamWarpmapFactory() {}

		StreamEncoder_p	m_pEncoder;
	};

} // namespace wax
#endif // WAX_STREAMWARPMAPFACTORY_DOT_H
