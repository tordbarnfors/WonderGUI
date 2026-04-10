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
#ifndef WG_SCROLLCAPSULE_DOT_H
#define WG_SCROLLCAPSULE_DOT_H
#pragma once


#include <wg_capsule.h>
#include <wg_scroller.h>

namespace wg
{
	class ScrollCapsule;
	typedef	StrongPtr<ScrollCapsule>	ScrollCapsule_p;
	typedef	WeakPtr<ScrollCapsule>		ScrollCapsule_wp;


	class ScrollCapsule : public Capsule, protected Scroller::Holder
	{
	public:

		//____ Blueprint ____________________________________________________________

		struct Blueprint
		{
			bool				autoHideScrollbars = true;
			Object_p			baggage;
			Widget_p			child;
			Skin_p				cornerSkin;
			bool				disabled = false;
			bool				dropTarget = false;
			Finalizer_p			finalizer = nullptr;
			int					id = 0;
			MarkPolicy			markPolicy = MarkPolicy::AlphaTest;
			bool				overlayScrollbars = false;
			bool				pickable = false;
			uint8_t				pickCategory = 0;
			bool				pickHandle = false;
			PointerStyle		pointer = PointerStyle::Undefined;
			Scroller::Blueprint	scrollbarX;
			Scroller::Blueprint	scrollbarY;
			bool				scrollX = true;
			bool				scrollY = true;
			bool				selectable = false;
			Skin_p				skin;
			bool				stickyFocus = false;
			bool				tabLock = false;
			bool				takesFocusFromChild = true;
			String				tooltip;
			bool				usePickHandles = false;
		};

/*
		bool scrollX;
		bool scrollY;
		bool overlayScrollbars;
		bool autoHideScrollbars;
*/

		/*	
		*	corner skin
		*	wheel handling
		*	step size
		*	page overlap
		* 
		*/


		//.____ Creation ______________________________________________________

		static ScrollCapsule_p	create();
		static ScrollCapsule_p	create(const Blueprint& blueprint);

		//.____ Components _______________________________________

		Scroller		scrollbarX;
		Scroller		scrollbarY;


		//.____ Identification ________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		ScrollCapsule();

		template<class BP> ScrollCapsule(const BP& bp) : Capsule(bp), scrollbarX(this, this, Axis::X), scrollbarY(this, this, Axis::Y), m_cornerSkin(this)
		{
			scrollbarX._initFromBlueprint(bp.scrollbarX);
			scrollbarY._initFromBlueprint(bp.scrollbarY);

			m_bAutoHideScrollbars = bp.autoHideScrollbars;
			m_bOverlayScrollbars = bp.overlayScrollbars;
			m_bScrollX = bp.scrollX;
			m_bScrollY = bp.scrollY;

			m_cornerSkin.set( bp.cornerSkin );

			if (bp.child)
				slot.setWidget(bp.child);
		}

		virtual ~ScrollCapsule();

		void		_receive(Msg * pMsg) override;
		void		_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip) override;

		void		_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		bool		_alphaTest(const CoordSPX& ofs) override;
		void		_resize(const SizeSPX& size, int scale) override;
		void		_setState(State state) override;


		spx			_matchingHeight(spx width, int scale) const override;
		spx			_matchingWidth(spx height, int scale) const override;

		SizeSPX		_defaultSize(int scale) const override;
		SizeSPX		_minSize(int scale) const override;
		SizeSPX		_maxSize(int scale) const override;

		BorderSPX	_calcOverflow() override;

		void		_updateRegions();
		bool		_setViewOffset( CoordSPX offset );
		void		_childCanvasCorrection();
		void		_updateScrollbars( const RectSPX& oldCanvas, const RectSPX& oldView );

		// Overloaded from Container

		Widget *	_findWidget(const CoordSPX& pos, SearchMode mode) override;

		void		_firstSlotWithGeo(SlotWithGeo& package) const override;

		RectSPX		_slotGeo(const StaticSlot * pSlot) const override;
		void		_childOverflowChanged( StaticSlot * pSlot, const BorderSPX& oldOverflow, const BorderSPX& newOverflow ) override;

		RectSPX		_childWindowSection(const StaticSlot * pSlot) const override;

		RectSPX		_childLocalToGlobal(const StaticSlot* pSlot, const RectSPX& rect) const override;
		RectSPX		_globalToChildLocal(const StaticSlot* pSlot, const RectSPX& rect) const override;
		RectSPX		_globalPtsToChildLocalSpx(const StaticSlot* pSlot, const Rect& rect) const override;
		Rect		_childLocalSpxToGlobalPts(const StaticSlot* pSlot, const RectSPX& rect) const override;

		void		_childRequestRender(StaticSlot * pSlot, const RectSPX& rect) override;
		void		_childRequestResize(StaticSlot * pSlot) override;

		void		_childRequestInView(StaticSlot * pSlot) override;
		void		_childRequestInView(StaticSlot * pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea) override;

		void		_releaseChild(StaticSlot * pSlot) override;
		void		_replaceChild(StaticSlot * pSlot, Widget * pWidget) override;


		//

		CoordSPX		_componentPos(const Component* pComponent) const override;
		SizeSPX			_componentSize(const Component* pComponent) const override;
		RectSPX			_componentGeo(const Component* pComponent) const override;


		// Needed for Scroller

		void	_scrollbarStep(const Scroller* pComponent, int dir) override;
		void	_scrollbarPage(const Scroller* pComponent, int dir) override;
		void	_scrollbarWheel(const Scroller* pComponent, int dir) override;
		spx		_scrollbarMove(const Scroller* pComponent, spx pos) override;
		std::tuple<spx, spx, spx> _scrollbarOfsLenContent(const Scroller* pComponent) override;



		bool		m_bScrollX = true;
		bool		m_bScrollY = true;
		bool		m_bOverlayScrollbars = false;
		bool		m_bAutoHideScrollbars = true;

		SkinSlot		m_cornerSkin;

		RectSPX			m_viewRegion;
		RectSPX			m_scrollbarXRegion;
		RectSPX			m_scrollbarYRegion;

		RectSPX			m_childCanvas;								// Child canvas in our coordinate system


	};

} // namespace wg

#endif //WG_SCROLLCAPSULE_DOT_H
