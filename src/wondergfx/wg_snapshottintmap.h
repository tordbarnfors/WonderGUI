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

#ifndef	WG_SNAPSHOTTINTMAP_DOT_H
#define	WG_SNAPSHOTTINTMAP_DOT_H
#pragma once

#include <wg_tintmap.h>
#include <wg_transitions.h>

namespace wg
{


	class SnapshotTintmap;
	typedef	StrongPtr<SnapshotTintmap>	SnapshotTintmap_p;
	typedef	WeakPtr<SnapshotTintmap>	SnapshotTintmap_wp;

	class SnapshotTintmap : public Tintmap
	{
	public:

		//.____ Blueprint _________________________________________

		struct Blueprint
		{
			// All parameters are mandatory.

			Tintmap_p			from;
			Tintmap_p			to;
			ColorTransition_p	transition;
			int					timestamp = -1;
		};

		//.____ Creation __________________________________________

		static SnapshotTintmap_p	create( const Blueprint& blueprint );
		static SnapshotTintmap_p	create(Tintmap * pFrom, Tintmap * pTo, ColorTransition * pTransition, int timestamp);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Render ________________________________________________

		void exportHorizontalColors(spx length, HiColor* pOutput) override;
		void exportVerticalColors(spx length, HiColor* pOutput) override;

		Gradient exportGradient() override;

	protected:

		SnapshotTintmap(Tintmap * pFrom, Tintmap * pTo, ColorTransition * pTransition, int timestamp);
		virtual ~SnapshotTintmap() {};

		void	_setFlags();

		Tintmap_p			m_pFrom;
		Tintmap_p			m_pTo;
		ColorTransition_p 	m_pTransition;
		int					m_timestamp;
	};
}



#endif //WG_SNAPSHOTTINTMAP_DOT_H
