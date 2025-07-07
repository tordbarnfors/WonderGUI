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

#ifndef WG_STACKPANEL_DOT_H
#define WG_STACKPANEL_DOT_H
#pragma once

#include <wg_panel.h>
#include <wg_slot.h>

namespace wg
{

	class StackPanel;

	typedef	StrongPtr<StackPanel>	StackPanel_p;
	typedef	WeakPtr<StackPanel>		StackPanel_wp;


	//____ StackPanelSlot ____________________________________________________________

	class StackPanelSlot : public PanelSlot
	{
		friend class StackPanel;
		friend class DynamicSlotVector<StackPanelSlot>;

	public:
		
		//.____ Blueprint _______________________________________________________

		struct Blueprint
		{
			Border			margin = 0;
			Placement		placement = Placement::Center;
			SizePolicy2D	sizePolicy = SizePolicy2D::Original;
			bool			visible = true;
		};

		//.____ Identification ________________________________________________

		const static TypeInfo	TYPEINFO;

		//.____ Geometry ______________________________________________________

		inline void		setMargin(Border padding);
		inline Border	margin() const { return m_margin; }

		//.____ Properties _________________________________________________

		void				setSizePolicy(SizePolicy2D policy);
		inline SizePolicy2D sizePolicy() const { return m_sizePolicy; }

		void				setPlacement(Placement placement);
		inline Placement	placement() const { return m_placement; }

		//.____ Operators __________________________________________

		inline void operator=(Widget * pWidget) { setWidget(pWidget); }

		//.____ Misc _______________________________________________

		using Link = DynamicSlotVector<StackPanelSlot>::SlotLink;

	protected:

		StackPanelSlot(SlotHolder * pHolder) : PanelSlot(pHolder) {}
		StackPanelSlot(StackPanelSlot&& o) = default;
		StackPanelSlot& operator=(StackPanelSlot&& o) = default;

		bool _setBlueprint( const Blueprint& bp );

		SizeSPX		_defaultSizeWithMargin(int scale) const { return m_pWidget->_defaultSize(scale) + Util::align(Util::ptsToSpx(m_margin, scale)); }
		
		Border				m_margin;

		Placement			m_placement = Placement::Center;
		SizePolicy2D		m_sizePolicy = SizePolicy2D::Original;
		bool				m_bResizeRequired = false;
	};


	//____ StackPanel ___________________________________________________________

	/**
	*/

	class StackPanel : public Panel<StackPanelSlot>
	{ 

	public:

		friend class StackPanelSlot;

		//.____ Blueprint ___________________________________________________________
		
		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled		= false;
			bool			dropTarget 		= false;
			Finalizer_p		finalizer		= nullptr;
			int				id				= 0;
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

		static StackPanel_p	create() { return StackPanel_p(new StackPanel()); }
		static StackPanel_p	create(const Blueprint& blueprint) { return StackPanel_p(new StackPanel(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Layout _____________________________________________________
		
		bool			setSlotMargin(int index, int amount, Border padding);
		bool			setSlotMargin(iterator beg, iterator end, Border padding);
		bool			setSlotMargin(int index, int amount, std::initializer_list<Border> padding);
		bool			setSlotMargin(iterator beg, iterator end, std::initializer_list<Border> padding);
		
		//.____ Internal _________________________________________________

		spx			_matchingHeight(spx width, int scale) const override;
		spx			_matchingWidth(spx height, int scale) const override;

		SizeSPX		_defaultSize(int scale) const override;

	protected:
		StackPanel();
		template<class BP> StackPanel(const BP& bp) : Panel(bp)
		{
			m_bSiblingsOverlap = true;
		}

		virtual ~StackPanel();

		// Overloaded from Widget

		void		_resize( const SizeSPX& size, int scale ) override;

		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		void		_childRequestRender( StaticSlot * pSlot, const RectSPX& rect ) override;
		void		_childRequestResize( StaticSlot * pSlot ) override;

		void		_releaseChild(StaticSlot * pSlot) override;
		void		_replaceChild(StaticSlot * pSlot, Widget * pNewChild ) override;

		void		_didAddSlots(StaticSlot * pSlot, int nb) override;
		void		_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb) override;
		void		_willEraseSlots(StaticSlot * pSlot, int nb) override;
		void		_hideSlots(StaticSlot *, int nb) override;
		void		_unhideSlots(StaticSlot *, int nb) override;
		void		_setSlotMargins(StaticSlot *, int nb, Border padding);
		void		_setSlotMargins(StaticSlot *, int nb, const Border * pPaddings);


		// Internal to StackPanel

		SizeSPX _calcDefaultSize(int scale) const;
		void	_updateChildGeo(StackPanelSlot * pBegin, StackPanelSlot * pEnd, bool bRender);

		void	_hideChildren( StackPanelSlot * pBegin, StackPanelSlot * pEnd );
		void	_unhideChildren( StackPanelSlot * pBegin, StackPanelSlot * pEnd );

		bool 	_makeWidgetAppear( StackPanelSlot * pSlot );

		RectSPX	_childGeo( const StackPanelSlot * pSlot ) const;

		SizeSPX	m_defaultSize;
	};


} // namespace wg
#endif //WG_STACKPANEL_DOT_H
