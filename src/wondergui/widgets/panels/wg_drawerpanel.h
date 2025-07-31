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

#ifndef	WG_DRAWERPANEL_DOT_H
#define	WG_DRAWERPANEL_DOT_H
#pragma once



#include <wg_container.h>
#include <wg_slot.h>
#include <wg_slotarray.h>
#include <wg_transitions.h>


namespace wg
{

	//____ DrawerPanel ___________________________________________________________

	class DrawerPanel;
	typedef	StrongPtr<DrawerPanel>	DrawerPanel_p;
	typedef	WeakPtr<DrawerPanel>	DrawerPanel_wp;

	class DrawerPanel : public Container
	{

	public:

		//.____ Blueprint ___________________________________________________________

		struct Blueprint
		{
			Object_p			baggage;
			Coord				buttonOfs;
			Placement			buttonPlacement = Placement::West;
			Size				buttonSize;
			Skin_p				buttonSkin;
			bool				disabled		= false;
			Direction			direction		= Direction::Down;
			bool				dropTarget		= false;
			Finalizer_p			finalizer		= nullptr;
			int					id				= 0;
			MarkPolicy			markPolicy		= MarkPolicy::AlphaTest;
			MaskOp				maskOp			= MaskOp::Recurse;
			bool				pickable		= false;
			uint8_t				pickCategory	= 0;
			bool				pickHandle		= false;
			PointerStyle		pointer			= PointerStyle::Undefined;
			bool				selectable		= true;
			Skin_p				skin;
			bool				stickyFocus		= false;
			bool				tabLock			= false;
			String				tooltip;
			ValueTransition_p	transition;
			bool				usePickHandles = false;
		};
		
		//.____ Creation __________________________________________

		static DrawerPanel_p	create() { return DrawerPanel_p(new DrawerPanel()); }
		static DrawerPanel_p	create(const Blueprint& blueprint) { return DrawerPanel_p(new DrawerPanel(blueprint)); }

		//.____ Components _______________________________________

		SlotArray<DynamicSlot, 2>		slots;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance ____________________________________________________

		void				setButton(Skin* pSkin, Placement origo = Placement::West, Coord ofs = Coord(), Size size = Size() );

		//.____ Behaviour ____________________________________________

		void				setDirection(Direction direction);
		Direction			direction() const { return m_direction; }

		void				setTransition(ValueTransition * pTransition);
		ValueTransition_p	transition() const { return m_pTransition; }

		//.____ Control ________________________________________________________

		void				setOpen(bool bOpen);
		bool				isOpen() const { return (m_foldState == FoldState::OPEN || m_foldState == FoldState::OPENING); }

		void				open() { setOpen(true); }
		void				close() { setOpen(false); }

		void				openImmediately();			// Open without transition
		void				closeImmediately();			// Close without transition


		//.____ Internal _______________________________________________________

		spx					_matchingHeight(spx width, int scale) const override;
		spx					_matchingWidth(spx height, int scale) const override;

		SizeSPX				_defaultSize(int scale) const override;
		SizeSPX				_minSize(int scale) const override;
		SizeSPX				_maxSize(int scale) const override;

	protected:
		DrawerPanel();
		template<class BP> DrawerPanel(const BP& bp) : slots(this), m_buttonSkin(this), Container(bp)
		{
			m_direction		= bp.direction;
			m_pTransition	= bp.transition;

			m_buttonGeo = { bp.buttonOfs, bp.buttonSize };
			m_buttonPlacement = bp.buttonPlacement;

			if( bp.buttonSkin )
				m_buttonSkin.set(bp.buttonSkin);

			m_bSiblingsOverlap = false;
		}
		
		virtual ~DrawerPanel();

		enum FoldState
		{
			CLOSED,
			OPEN,
			CLOSING,
			OPENING
		};

		std::tuple<SizeSPX,SizeSPX>	_calcDefaultSize(int scale) const;
		void				_updateGeo();

		inline bool			_isVertical() const { return (m_direction == Direction::Up || m_direction == Direction::Down); }
		RectSPX				_buttonGeo() const;


		// Overloaded from Widget

		void		_resize(const SizeSPX& size, int scale) override;
		void		_receive(Msg * pMsg) override;

		void		_update(int microPassed, int64_t microsecTimestamp) override;
		void		_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;

		bool		_alphaTest(const CoordSPX& ofs) override;

//		void		_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip) override;


		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		Widget *	_firstChild() const override;
		Widget *	_lastChild() const override;

		BorderSPX	_calcOverflow() override;
		
		void		_firstSlotWithGeo(SlotWithGeo& package) const override;
		void		_nextSlotWithGeo(SlotWithGeo& package) const override;

		RectSPX		_slotGeo( const StaticSlot * pSlot) const override;

		void		_childRequestRender(StaticSlot * pSlot, const RectSPX& rect) override;
		void		_childRequestResize(StaticSlot * pSlot) override;

		void		_childRequestInView(StaticSlot* pSlot) override;
		void		_childRequestInView(StaticSlot* pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea) override;


		Widget *	_prevChild(const StaticSlot * pSlot) const override;
		Widget *	_nextChild(const StaticSlot * pSlot) const override;

		void		_releaseChild(StaticSlot * pSlot) override;
		void		_replaceChild(StaticSlot * pSlot, Widget * pNewChild) override;

		//


		Direction			m_direction = Direction::Down;
		SizeSPX				m_defaultSizeOpen;
		SizeSPX				m_defaultSizeClosed;

		RectSPX				m_frontGeo;
		RectSPX				m_backCanvas;
		RectSPX				m_backWindow;

		FoldState			m_foldState = FoldState::CLOSED;

		ValueTransition_p	m_pTransition;
		int					m_transitionProgress = -1;

		SkinSlot			m_buttonSkin;
		Placement			m_buttonPlacement = Placement::Undefined;
		Rect				m_buttonGeo;
		bool				m_bButtonHovered = false;

	};

}

#endif //WG_DRAWERPANEL_DOT_H
