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

#ifndef	WG_TINTMAP_DOT_H
#define	WG_TINTMAP_DOT_H
#pragma once

#include <wg_object.h>
#include <wg_gfxtypes.h>
#include <wg_geo.h>
#include <wg_color.h>
#include <wg_gradient.h>

namespace wg
{


	class Tintmap;
	typedef	StrongPtr<Tintmap>	Tintmap_p;
	typedef	WeakPtr<Tintmap>	Tintmap_wp;

	class Tintmap : public Object
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _________________________________________________

		inline bool isOpaque() const { return m_bOpaque; }
		inline bool isVertical() const { return m_bVertical; }
		inline bool isHorizontal() const { return m_bHorizontal; }

		//.____ Render ________________________________________________

		virtual void exportHorizontalColors(spx length, HiColor* pOutput) = 0;
		virtual void exportVerticalColors(spx length, HiColor* pOutput) = 0;

		virtual Gradient exportGradient() = 0;			// To support legacy code.
		
	protected:

		void _exportDummyColors(spx length, HiColor * pOutput);

		
		bool	m_bOpaque;
		bool	m_bVertical;
		bool	m_bHorizontal;

	};

}



#endif //WG_TINTMAP_DOT_H
