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
#ifndef WG_WIRECAPSULE_DOT_H
#define WG_WIRECAPSULE_DOT_H
#pragma once

#include <wg_capsule.h>

//TODO: Need to support skin padding.

namespace wg
{

	class WireCapsule;
	typedef	StrongPtr<WireCapsule>		WireCapsule_p;
	typedef	WeakPtr<WireCapsule>		WireCapsule_wp;

	//____ WireCapsule ________________________________________________________

	class WireCapsule : public Capsule
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{

			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			bool			takesFocusFromChild = true;
			String			tooltip;
			bool			usePickHandles = false;

		};

		//.____ Creation __________________________________________

		static WireCapsule_p	create();
		static WireCapsule_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal _________________________________________________



	protected:
		WireCapsule();

		template<class BP> WireCapsule(const BP& bp) : Capsule(bp)
		{
			if (bp.child)
				slot.setWidget(bp.child);
		}

		virtual ~WireCapsule();

	private:

	};


} // namespace wg
#endif //WG_WIRECAPSULE_DOT_H
