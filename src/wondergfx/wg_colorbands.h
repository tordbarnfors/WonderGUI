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
#ifndef	WG_COLORBANDS_DOT_H
#define	WG_COLORBANDS_DOT_H
#pragma once

#include <wg_tintmap.h>

#include <vector>

namespace wg
{


	class Colorbands;
	typedef	StrongPtr<Colorbands>	Colorbands_p;
	typedef	WeakPtr<Colorbands>	Colorbands_wp;

	class Colorbands : public Tintmap
	{
	public:

		struct Band
		{
			HiColor		color = HiColor::White;
			float		end = 0.f;
		};

		struct Blueprint
		{
			Axis				axis =	Axis::X;
			std::vector<Band>	bands;
			Finalizer_p			finalizer;
		};

		//.____ Creation __________________________________________

		static Colorbands_p	create(const Blueprint& blueprint);
		static Colorbands_p	create(Axis axis, const std::initializer_list<Band> bands);
		static Colorbands_p	create(Axis axis, const Band * begin, const Band * end);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Render ________________________________________________

		void exportColors(SizeI tintmapSize, HiColor* pOutputX, HiColor* pOutputY) override;

		//.____ Misc ___________________________________________________

		int alpha(const CoordSPX& pos, const RectSPX& area) override;


	protected:

		Colorbands(Axis axis, const Band * begin, const Band * end);
		virtual ~Colorbands() {};

		void	_export(int entries, HiColor* pDest);
		void	_setFlags();

		Axis				m_axis;
		std::vector<Band>	m_bands;
	};

}



#endif //WG_COLORBANDS_DOT_H
