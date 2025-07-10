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
#ifndef WG_PADDINGCAPSULE_DOT_H
#define WG_PADDINGCAPSULE_DOT_H
#pragma once

#include <wg_capsule.h>

namespace wg
{
	class PaddingCapsule;
	typedef	StrongPtr<PaddingCapsule>	PaddingCapsule_p;
	typedef	WeakPtr<PaddingCapsule>		PaddingCapsule_wp;

	//____ PaddingCapsule ______________________________________________________

	class PaddingCapsule : public Capsule
	{
	public:
		//____ Blueprint __________________________________________

		struct Blueprint
		{

			Object_p		baggage;
			Widget_p		child;
			bool			disabled		= false;
			bool			dropTarget		= false;
			Finalizer_p		finalizer		= nullptr;
			int				id				= 0;
			MarkPolicy		markPolicy		= MarkPolicy::AlphaTest;
			bool			pickable		= false;
			uint8_t			pickCategory	= 0;
			bool			pickHandle		= false;
			Border			padding;
			PointerStyle	pointer			= PointerStyle::Undefined;
			bool			selectable		= false;
			Skin_p			skin;
			bool			stickyFocus		= false;
			bool			tabLock			= false;
			String			tooltip;
			bool			usePickHandles	= false;
		};


		//.____ Creation __________________________________________

		static PaddingCapsule_p	create();
		static PaddingCapsule_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal ______________________________________________________

		virtual spx		_matchingHeight(spx width, int scale) const override;
		virtual spx		_matchingWidth(spx height, int scale) const override;

		SizeSPX			_defaultSize(int scale) const override;
		SizeSPX			_minSize(int scale) const override;
		SizeSPX			_maxSize(int scale) const override;


	protected:
		PaddingCapsule();

		template<class BP> PaddingCapsule( const BP& bp ) : Capsule(bp)
		{
			 m_padding = bp.padding;

			 if (bp.child)
				 slot.setWidget(bp.child);
		}

		virtual ~PaddingCapsule();

		void		_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip) override;
		void		_resize(const SizeSPX& size, int scale) override;
		RectSPX		_slotGeo(const StaticSlot* pSlot) const override;

		void		_firstSlotWithGeo(SlotWithGeo& package) const override;


		Border		m_padding;
	};


} // namespace wg
#endif //WG_PADDINGCAPSULE_DOT_H
