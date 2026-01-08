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
#ifndef	WG_STATICTINTMAP_DOT_H
#define	WG_STATICTINTMAP_DOT_H
#pragma once

#include <wg_tintmap.h>

namespace wg
{


	class StaticTintmap;
	typedef	StrongPtr<StaticTintmap>	StaticTintmap_p;
	typedef	WeakPtr<StaticTintmap>	StaticTintmap_wp;

	class StaticTintmap : public Tintmap
	{
	public:


		//.____ Creation __________________________________________

		static StaticTintmap_p	create(SizeI size, const HiColor * pColorstripX, const HiColor * pColorstripY);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Render ________________________________________________

		void exportColors(SizeI tintmapSize, HiColor* pOutputX, HiColor* pOutputY ) override;

		//.____ Misc ___________________________________________________

		int alpha( const CoordSPX& pos, const RectSPX& area ) override;


	protected:

		StaticTintmap(SizeI size, const HiColor * pColorstripX, const HiColor * pColorstripY);
		virtual ~StaticTintmap();

		HiColor *	m_pColors;
		SizeI		m_size;

	};

}



#endif //WG_STATICTINTMAP_DOT_H
