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
#ifndef WG_LABELCAPSULE_DOT_H
#define WG_LABELCAPSULE_DOT_H
#pragma once

#include <wg_capsule.h>
#include <wg_dynamictext.h>

namespace wg
{
	class LabelCapsule;
	typedef	StrongPtr<LabelCapsule>	LabelCapsule_p;
	typedef	WeakPtr<LabelCapsule>		LabelCapsule_wp;

	//____ LabelCapsule ______________________________________________________

	class LabelCapsule : public Capsule
	{
	public:
		//____ Blueprint __________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = true;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			DynamicText::Blueprint	label;
			Placement		labelPlacement = Placement::NorthWest;
			Skin_p			labelSkin;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = true;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
			bool			usePickHandles = false;

		};

		//.____ Creation __________________________________________

		static LabelCapsule_p	create();
		static LabelCapsule_p	create(const Blueprint& blueprint);

		//.____ Components ____________________________________

		DynamicText			label;

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal _________________________________________________

		spx				_matchingWidth(spx height, int scale) const override;
		spx				_matchingHeight(spx width, int scale) const override;
		SizeSPX			_defaultSize(int scale) const override;



	protected:
		LabelCapsule() : label(this), m_labelSkin(this) {};

		template<class BP> LabelCapsule(const BP& bp) : label(this), m_labelSkin(this), Capsule(bp)
		{
			m_labelPlacement = bp.labelPlacement;

			label._initFromBlueprint(bp.label);

			if (bp.labelSkin)
			{
				//TODO: Support overflowing labelSkin.

				m_labelSkin.set(bp.labelSkin);
//				m_bOverflow = bp.skin->_overflowsGeo();
			}

		}

		virtual ~LabelCapsule() {};

		RectSPX _labelGeo(RectSPX widgetSize, int scale) const;


		void			_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		bool			_alphaTest(const CoordSPX& ofs) override;
		void			_resize(const SizeSPX& size, int scale) override;
		void			_setState(State state) override;

		CoordSPX		_componentPos(const Component* pComponent) const override;
		SizeSPX			_componentSize(const Component* pComponent) const override;
		RectSPX			_componentGeo(const Component* pComponent) const override;

		SizeSPX			_skinSize(const SkinSlot* pSlot) const override;
		void			_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect) override;


		SkinSlot		m_labelSkin;
		Placement		m_labelPlacement = Placement::NorthWest;

	};


} // namespace wg
#endif //WG_LABELCAPSULE_DOT_H

