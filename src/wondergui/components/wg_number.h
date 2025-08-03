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
#ifndef	WG_NUMBER_DOT_H
#define WG_NUMBER_DOT_H
#pragma once

#include <limits>
#include <wg_staticnumber.h>

namespace wg
{


	//____ Number ____________________________________________________________

	class Number : public StaticNumber
	{
	public:
		Number(Widget * pHolder);
		virtual ~Number() {}

		//.____ Blueprint ______________________________________________________

		struct Blueprint
		{
			NumberLayout_p	layout;
			double			maxValue = std::numeric_limits<double>::max();
			double			minValue = std::numeric_limits<double>::lowest();
			double			value = 0.0;
		};

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _____________________________________________

		void				clear();
		bool				set( double value );

		bool				setRange( double min, double max );
		inline double		min() const { return m_minValue; }
		inline double		max() const { return m_maxValue; }

		//.____ Internal __________________________________________________

		void				_initFromBlueprint(const Blueprint& blueprint);

	protected:

		bool				_set(double value) override;

		double				m_minValue;
		double				m_maxValue;
	};



} // namespace wg
#endif //WG_CNUMBERDISPLAY_DOT_H
