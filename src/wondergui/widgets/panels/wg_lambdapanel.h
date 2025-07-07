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

#ifndef WG_LAMBDAPANEL_DOT_H
#define WG_LAMBDAPANEL_DOT_H
#pragma once


#include <wg_panel.h>

namespace wg
{

	class LambdaPanel;
	typedef	StrongPtr<LambdaPanel>		LambdaPanel_p;
	typedef	WeakPtr<LambdaPanel>		LambdaPanel_wp;


	//____ LambdaPanelSlot ____________________________________________________________

	class LambdaPanelSlot : public PanelSlot
	{
		friend class LambdaPanel;
		friend class DynamicSlotVector<LambdaPanelSlot>;

	public:

		//.____ Blueprint _______________________________________________________

		struct Blueprint
		{
			std::function<Rect(Widget * pWidget, Size parentSize)> func;
		};
		
		//.____ Identification ________________________________________________

		const static TypeInfo	TYPEINFO;

		//.____ Properties _________________________________________________

		void	setFunction(const std::function<Rect(Widget * pWidget, Size parentSize)>& func);
		inline const std::function<Rect(Widget * pWidget, Size parentSize)>& function() const { return m_func; }

		//.____ Operators __________________________________________

		inline void operator=(Widget * pWidget) { setWidget(pWidget); }

		//.____ Misc _______________________________________________

		using Link = DynamicSlotVector<LambdaPanelSlot>::SlotLink;

	protected:

		LambdaPanelSlot(SlotHolder * pHolder) : PanelSlot(pHolder) {}
	//			Slot(Slot&& o) noexcept = default;
	//			Slot& operator=(Slot&& o) = default;

		const static bool safe_to_relocate = false;

		bool _setBlueprint( const Blueprint& bp );

		std::function<Rect(Widget * pWidget, Size parentSize)>	m_func = nullptr;
	};


	//____ LambdaPanel _________________________________________________________

	class LambdaPanel : public Panel<LambdaPanelSlot>
	{

	public:

		friend class LambdaPanelSlot;
		
		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			Size			defaultSize;
			bool			disabled = false;
			bool			dropTarget = false;
			EdgePolicy		edgePolicy = EdgePolicy::Confine;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::Undefined;
			MaskOp			maskOp = MaskOp::Recurse;
			Size			maxSize;
			Size			minSize;
			bool			pickable = false;
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

		static LambdaPanel_p	create() { return LambdaPanel_p(new LambdaPanel()); }
		static LambdaPanel_p	create( const Blueprint& blueprint ) { return LambdaPanel_p(new LambdaPanel(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________

		void			setEdgePolicy(EdgePolicy policy);
		EdgePolicy		edgePolicy() const { return m_edgePolicy; }

		bool			setMinSize(Size minSize);
		bool			setMaxSize(Size maxSize);
		bool			setSizeLimits( Size minSize, Size maxSize );
		bool			setDefaultSize(Size defaultSize);
		
	protected:
		LambdaPanel();
		template<class BP> LambdaPanel(const BP& bp) : Panel(bp)
		{
			m_bSiblingsOverlap	= true;
			m_edgePolicy		= bp.edgePolicy;
			m_defaultSize		= bp.defaultSize;
			m_minSize			= bp.minSize;
			m_maxSize			= bp.maxSize;
		}

		virtual ~LambdaPanel();

		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		SizeSPX		_defaultSize(int scale) const override;
		SizeSPX		_minSize(int scale) const override;
		SizeSPX		_maxSize(int scale) const override;

		void		_childRequestRender( StaticSlot * pSlot, const RectSPX& rect ) override;
		void		_childRequestResize( StaticSlot * pSlot ) override;

		void		_releaseChild(StaticSlot * pSlot) override;
		void		_replaceChild(StaticSlot * pSlot, Widget * pNewChild) override;

		void		_didAddSlots(StaticSlot * pSlot, int nb) override;
		void		_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb) override;
		void		_willEraseSlots(StaticSlot * pSlot, int nb) override;
		void		_hideSlots(StaticSlot * pSlot, int nb) override;
		void		_unhideSlots(StaticSlot * pSlot, int nb) override;

	private:
		void		_updateSlotGeo(StaticSlot * pSlot, int nb);

		void		_resize( const SizeSPX& size, int scale ) override;
		void		_updateGeo(LambdaPanelSlot * pSlot, bool bForceResize = false, bool bUpdateInfluence = true);
		void		_updateAllSlotsGeo();

		void		_onRequestRender( const RectSPX& rect, const LambdaPanelSlot * pSlot );

		Size		m_minSize;
		Size		m_maxSize;
		Size		m_defaultSize;

		EdgePolicy	m_edgePolicy = EdgePolicy::Confine;
	};



} // namespace wg
#endif //WG_LAMBDAPANEL_DOT_H
