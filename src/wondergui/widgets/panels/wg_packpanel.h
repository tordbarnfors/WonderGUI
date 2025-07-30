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
#ifndef WG_PACKPANEL_DOT_H
#define WG_PACKPANEL_DOT_H
#pragma once

#include <wg_packlayout.h>
#include <wg_panel.h>


namespace wg
{

	class PackPanel;
	typedef	StrongPtr<PackPanel>		PackPanel_p;
	typedef	WeakPtr<PackPanel>			PackPanel_wp;


	//____ PackPanelSlot ____________________________________________________________

	class PackPanelSlot : public PanelSlot
	{
		friend class PackPanel;
		template<class S> friend class SlotVector;
		friend class DynamicSlotVector<PackPanelSlot>;

	public:

		//.____ Blueprint _______________________________________________________

		struct Blueprint
		{
			float	baseline = 0.f;
			bool	visible = true;
			float	weight = 1;
		};
		
		//.____ Identification ________________________________________________

		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		void			setWeight(float weight);
		inline float	weight() const { return m_weight; }

		void			setBaseline(float baseline);
		inline float	baseline() const { return m_baseline; }
		
		//.____ Operators __________________________________________

		inline void operator=(Widget * pWidget) { setWidget(pWidget); }

		//.____ Misc _______________________________________________

		using Link = DynamicSlotVector<PackPanelSlot>::SlotLink;

	protected:

		PackPanelSlot(SlotHolder *pHolder) : PanelSlot(pHolder) {}
		PackPanelSlot(PackPanelSlot&& o) = default;
		PackPanelSlot& operator=(PackPanelSlot&& o) = default;

		bool _setBlueprint( const Blueprint& bp );

		bool		m_bResizeRequired = false;
		float		m_weight = 1.f;				// Weight for space allocation.
		float		m_baseline = 0.f;
		SizeSPX		m_defaultSize;				// Cached padded default size from the child.
	};


	//____ PackPanel ________________________________________________________________
	/**
	 * @brief	A widget for arranging children horizontally or vertically.
	 *
	 * A widget for arranging children horizontally or vertically.
	 */

	class PackPanel : public PanelTemplate<PackPanelSlot>
	{
		friend class PackPanelSlot;
	public:


		//.____ Blueprint _____________________________________________________
		
		struct Blueprint
		{
			Axis			axis			= Axis::X;
			Object_p		baggage;
			bool			dropTarget 		= false;
			bool			disabled 		= false;
			Finalizer_p		finalizer 		= nullptr;
			int				id 				= 0;
			PackLayout_p	layout;
			MarkPolicy		markPolicy 		= MarkPolicy::AlphaTest;
			MaskOp			maskOp			= MaskOp::Recurse;
			bool			pickable 		= false;
			uint8_t			pickCategory	= 0;
			bool			pickHandle		= false;
			PointerStyle	pointer 		= PointerStyle::Undefined;
			bool			selectable 		= false;
			Skin_p			skin;
			Alignment		slotAlignment	= Alignment::Justify;
			pts				spacing			= 0;
			pts				spacingAfter	= 0;
			pts				spacingBefore	= 0;
			bool			stickyFocus		= false;
			bool			tabLock 		= false;
			String			tooltip;
			bool			usePickHandles	= false;
		};
		
		//.____ Creation __________________________________________

		static PackPanel_p	create() { return PackPanel_p(new PackPanel()); }
		static PackPanel_p	create(const Blueprint& blueprint) { return PackPanel_p(new PackPanel(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;
		
		//.____ Layout ________________________________________________________

		void			setAxis(Axis orientaiton);
		Axis			axis() const { return m_axis; }

		void			setLayout(PackLayout* pLayout);
		PackLayout_p	layout() const { return m_pLayout; }

		void			setSpacing(pts before, pts between, pts after);
		void			setSpacing(pts between);

		void			setSlotAlignment( Alignment alignment );
		Alignment		slotAlignment() const { return m_slotAlignment; }
		
		bool			setSlotWeight(int index, int amount, float weight);
		bool			setSlotWeight(iterator  beg, iterator end, float weight);
		bool			setSlotWeight(int index, int amount, std::initializer_list<float> weights);
		bool			setSlotWeight(iterator beg, iterator end, std::initializer_list<float> weights);

		bool			setSlotBaseline(int index, int amount, float weight);
		bool			setSlotBaseline(iterator  beg, iterator end, float weight);
		bool			setSlotBaseline(int index, int amount, std::initializer_list<float> weights);
		bool			setSlotBaseline(iterator beg, iterator end, std::initializer_list<float> weights);

		
		//.____ Internal ______________________________________________________

		spx				_matchingHeight(spx width, int scale) const override;
		spx				_matchingWidth(spx height, int scale) const override;

		SizeSPX			_defaultSize(int scale) const override;


	protected:
		PackPanel();
		template< class BP> PackPanel(const BP& bp) : PanelTemplate(bp)
		{
			m_bSiblingsOverlap	= false;
			m_axis				= bp.axis;
			m_pLayout			= bp.layout;
			m_spacingBefore		= bp.spacingBefore;
			m_spacingBetween	= bp.spacing;
			m_spacingAfter		= bp.spacingAfter;
			m_slotAlignment		= bp.slotAlignment;
		}
		
		virtual ~PackPanel();

		// Overloaded from Widget

		void		_resize( const SizeSPX& size, int scale ) override;


		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		void		_childRequestResize(StaticSlot * pSlot) override;

		void		_releaseChild(StaticSlot * pSlot) override;
		void		_replaceChild(StaticSlot * pSlot, Widget * pNewWidget) override;

		void		_didAddSlots(StaticSlot * pSlot, int nb) override;
		void		_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb) override;
		void		_willEraseSlots(StaticSlot * pSlot, int nb) override;

		void		_hideSlots(StaticSlot *, int nb) override;
		void		_unhideSlots(StaticSlot *, int nb) override;

		void		_reweightSlots(PackPanelSlot * pSlot, int nb, float weight);
		void		_reweightSlots(PackPanelSlot * pSlot, int nb, const float * pWeights);

		void		_setBaselines(PackPanelSlot * pSlot, int nb, float weight);
		void		_setBaselines(PackPanelSlot * pSlot, int nb, const float * pWeights);
		
		//		void		_refreshChildGeo() { _refreshChildGeo(true); }

		//

		void		_refreshChildGeo(bool bRequestRender);
		void		_realignChildGeo(bool bRequestRender);

		void		_hideChildren(PackPanelSlot * pSlot, int nb);
		void		_unhideChildren(PackPanelSlot * pSlot, int nb);

		void		_refreshGeometries();
		SizeSPX		_calcDefaultContentSize( int scale, spx& maxAscend, spx& maxDescend ) const;
		spx 		_calcBaselineOffset(spx givenBreadth);
		spx			_calcTotalSpacing( int scale ) const;

		int			_populateLayoutArray( PackLayout::Item * pArray, int scale ) const;
		int			_populateLayoutArray(PackLayout::Item* pArray, int scale, spx forcedBreadth ) const;

		Axis			m_axis = Axis::X;
		PackLayout_p	m_pLayout;
		SizeSPX			m_defaultContentSize;

		pts				m_spacingBefore = 0;
		pts				m_spacingBetween = 0;
		pts				m_spacingAfter = 0;

		Alignment		m_slotAlignment = Alignment::Justify;
		spx				m_maxAscend = 0;
		spx				m_maxDescend = 0;
		
		spx				m_totalSpacing = 0;
		
		int				m_nbVisibleSlots = 0;
	};



} // namespace wg
#endif //WG_PACKPANEL_DOT_H
