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

#ifndef	WG_TWOSLOTPANEL_DOT_H
#define	WG_TWOSLOTPANEL_DOT_H
#pragma once



#include <wg_container.h>
#include <wg_slot.h>
#include <wg_slotarray.h>
#include <wg_packlayout.h>


namespace wg
{

	//____ TwoSlotPanel ___________________________________________________________

	class TwoSlotPanel;
	typedef	StrongPtr<TwoSlotPanel>	TwoSlotPanel_p;
	typedef	WeakPtr<TwoSlotPanel>	TwoSlotPanel_wp;

	class TwoSlotPanel : public Container
	{

	public:

		//____ Slot _________________________________________________________

		class Slot : public DynamicSlot
		{
			friend class TwoSlotPanel;
			template<class S, int X> friend class SlotArray;

		public: 

			//.____ Blueprint _______________________________________________________

			struct Blueprint
			{
				float	weight = 1;
			};
			
			//.____ Identification ________________________________________________

			const static TypeInfo	TYPEINFO;

			//.____ Geometry _________________________________________________

			void			setWeight(float weight) { m_weight = int(weight * 65536); };
			inline float	weight() const { return float(m_weight)/65536; }


			//.____ Operators __________________________________________

			inline void operator=(Widget * pWidget) { setWidget(pWidget); }

		protected:
			Slot() : DynamicSlot(nullptr) {}

			bool _setBlueprint( const Blueprint& bp );

			
			int		m_weight = 65536;			// Float scaled down to 16.16 format.
			int		m_length = 0;
		};

		//.____ Blueprint _________________________________________
		
		struct Blueprint
		{
			Axis			axis = Axis::X;
			Object_p		baggage;
			bool			disabled		= false;
			bool			dropTarget		= false;
			Finalizer_p		finalizer		= nullptr;
			int				id				= 0;
			PackLayout_p	layout;
			MarkPolicy		markPolicy		= MarkPolicy::AlphaTest;
			MaskOp			maskOp			= MaskOp::Recurse;
			bool			pickable		= false;
			uint8_t			pickCategory	= 0;
			bool			pickHandle		= false;
			PointerStyle	pointer			= PointerStyle::Undefined;
			bool			selectable		= false;
			Skin_p			skin;
			bool			stickyFocus		= false;
			bool			tabLock			= false;
			String			tooltip;
			bool			usePickHandles = false;

		};
		
		//.____ Creation __________________________________________

		static TwoSlotPanel_p	create() { return TwoSlotPanel_p(new TwoSlotPanel()); }
		static TwoSlotPanel_p	create(const Blueprint& blueprint) { return TwoSlotPanel_p(new TwoSlotPanel(blueprint)); }

		//.____ Components _______________________________________

		SlotArray<Slot, 2>		slots;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Layout _______________________________________________________

		void			setAxis(Axis orientaiton);
		Axis			axis() const { return m_bHorizontal ? Axis::X : Axis::Y; }

		void			setLayout(PackLayout* pLayout);
		PackLayout_p	layout() const { return m_pLayout; }

		//.____ Internal _______________________________________________________

		SizeSPX			_defaultSize(int scale) const override;


	protected:
		TwoSlotPanel();
		template<class BP> TwoSlotPanel(const BP& bp) : slots(this), Container(bp)
		{
			m_bHorizontal = (bp.axis == Axis::X);
			m_pLayout = bp.layout;

			m_bSiblingsOverlap = false;
		};
		
		virtual ~TwoSlotPanel();
		
		void		_updateDefaultSize();
		SizeSPX		_calcDefaultSize(int scale) const;
		bool		_updateGeo( bool bForce );

		// Overloaded from Widget

		void		_resize(const SizeSPX& size, int scale) override;

		void		_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip) override;

		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		Widget *	_firstChild() const override;
		Widget *	_lastChild() const override;

		void		_firstSlotWithGeo(SlotWithGeo& package) const override;
		void		_nextSlotWithGeo(SlotWithGeo& package) const override;

		RectSPX		_slotGeo( const StaticSlot * pSlot) const override;

		void		_childRequestRender(StaticSlot * pSlot, const RectSPX& rect) override;
		void		_childRequestResize(StaticSlot * pSlot) override;

		Widget *	_prevChild(const StaticSlot * pSlot) const override;
		Widget *	_nextChild(const StaticSlot * pSlot) const override;

		void		_releaseChild(StaticSlot * pSlot) override;
		void		_replaceChild(StaticSlot * pSlot, Widget * pNewChild) override;

		//

		inline RectSPX	_slotOneRect(const RectSPX& contentRect) const
		{
			if (m_bHorizontal)
				return { contentRect.x, contentRect.y, slots[0].m_length, contentRect.h };
			else
				return { contentRect.x, contentRect.y, contentRect.w, slots[0].m_length };
		}

		inline RectSPX	_slotTwoRect(const RectSPX& contentRect) const
		{
			if (m_bHorizontal)
				return { contentRect.x + slots[0].m_length, contentRect.y, slots[1].m_length, contentRect.h };
			else
				return { contentRect.x, contentRect.y + slots[0].m_length, contentRect.w, slots[1].m_length };
		}


		bool			m_bHorizontal;
		SizeSPX			m_defaultSize;
		PackLayout_p	m_pLayout;
	};

}

#endif //WG_TWOSLOTPANEL_DOT_H
