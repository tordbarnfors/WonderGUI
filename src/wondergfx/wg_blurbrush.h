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

#ifndef	WG_BLURBRUSH_DOT_H
#define	WG_BLURBRUSH_DOT_H
#pragma once

#include <wg_object.h>
#include <wg_gfxtypes.h>
#include <wg_geo.h>
#include <wg_color.h>

namespace wg
{


	class Blurbrush;
	typedef	StrongPtr<Blurbrush>	Blurbrush_p;
	typedef	WeakPtr<Blurbrush>		Blurbrush_wp;

	class Blurbrush : public Object
	{
	public:

		//.____ Blueprint _________________________________________________________

		struct Blueprint
		{
			float	blue[9] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
			Finalizer_p		finalizer = nullptr;
			float	green[9] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
			float	red[9] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };

			spx		size = 64 * 4;;
		};

		//.____ Creation __________________________________________________________

		static Blurbrush_p	create() { return create(Blueprint()); }
		static Blurbrush_p	create(const Blueprint& blueprint);


		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _________________________________________________

		spx		size() const { return m_radius; }

		const float* blue() const { return m_greenMtx; }
		const float* green() const { return m_greenMtx; }
		const float* red() const { return m_greenMtx; }

	protected:

		Blurbrush(const Blueprint& bp);
		virtual ~Blurbrush() {}

		spx		m_radius;

		float	m_redMtx[9];
		float	m_greenMtx[9];
		float	m_blueMtx[9];
	};
}



#endif //WG_BLURBRUSH_DOT_H
