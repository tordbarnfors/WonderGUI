/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

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

		void exportHorizontalColors(spx length, HiColor* pOutput) override;
		void exportVerticalColors(spx length, HiColor* pOutput) override;

		Gradient exportGradient() override;

	protected:

		Gradyent(HiColor top, HiColor bottom, HiColor left, HiColor right);
		virtual ~Gradyent() {};

		void	_setFlags();

		HiColor		m_top;
		HiColor		m_bottom;
		HiColor		m_left;
		HiColor		m_right;
	};

}



#endif //WG_TINTMAP_DOT_H
