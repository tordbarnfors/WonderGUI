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
#ifndef	WG_GRADYENT_DOT_H
#define	WG_GRADYENT_DOT_H
#pragma once

#include <wg_tintmap.h>
#include <wg_gradient.h>

namespace wg
{


	class Gradyent;
	typedef	StrongPtr<Gradyent>	Gradyent_p;
	typedef	WeakPtr<Gradyent>	Gradyent_wp;

	class Gradyent : public Tintmap
	{
	public:


		//.____ Creation __________________________________________

		static Gradyent_p	create(HiColor top, HiColor bottom, HiColor left, HiColor right);
		static Gradyent_p	create( const Gradient& gradient );									//NOTE! Only here for legacy reasons. Conversion from corner to side colors performed and only one direction supported.

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Render ________________________________________________

		void exportColors(SizeI tintmapSize, HiColor* pOutputX, HiColor* pOutputY ) override;

		Gradient exportGradient() override;

		//.____ Misc ___________________________________________________

		int alpha( const CoordSPX& pos, const RectSPX& area ) override;


	protected:

		Gradyent(HiColor top, HiColor bottom, HiColor left, HiColor right);
		virtual ~Gradyent() {};

		void	_export(int entries, HiColor * pDest, const HiColor& from, const HiColor& to);
		void	_setFlags();

		HiColor		m_top;
		HiColor		m_bottom;
		HiColor		m_left;
		HiColor		m_right;
	};

}



#endif //WG_TINTMAP_DOT_H
