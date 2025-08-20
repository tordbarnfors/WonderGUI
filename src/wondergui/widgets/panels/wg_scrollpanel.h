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

#ifndef	WG_SCROLLPANEL_DOT_H
#define	WG_SCROLLPANEL_DOT_H
#pragma once



#include <wg_container.h>
#include <wg_slot.h>
#include <wg_scroller.h>
#include <wg_transitions.h>

namespace wg
{

	class ScrollPanel;
	typedef	StrongPtr<ScrollPanel>	ScrollPanel_p;
	typedef	WeakPtr<ScrollPanel>	ScrollPanel_wp;


	//____ ScrollPanel ___________________________________________________________

	class ScrollPanel : public Container, protected Scroller::Holder
	{
	public:

		//.____ Blueprint _____________________________________________________
		
		struct Blueprint
		{
			bool				autohideScrollbars 	= true;
			bool				autoscroll 			= false;
			Object_p			baggage;
			Widget_p			child;
			Placement			childPlacement 		= Placement::NorthWest;
			SizeConstraint		childConstraintX 	= SizeConstraint::None;
			SizeConstraint		childConstraintY 	= SizeConstraint::None;
			bool				disabled 			= false;
			bool				dropTarget 			= false;
			Finalizer_p			finalizer 			= nullptr;
			int					id 					= 0;
			MarkPolicy			markPolicy 			= MarkPolicy::AlphaTest;
			MaskOp				maskOp				= MaskOp::Recurse;
			bool				overlayScrollbars 	= false;
			pts					pageOverlap 		= 8;
			bool				pickable 			= false;
			uint8_t				pickCategory 		= 0;
			bool				pickHandle			= false;
			PointerStyle		pointer 			= PointerStyle::Undefined;
			Scroller::Blueprint	scrollbarX;
			Scroller::Blueprint	scrollbarY;
			bool				selectable 			= false;
			Skin_p				skin;
			bool				stealWheelFromScrollbars = true;
			pts					stepSize 			= 8;
			bool				stickyFocus			= false;
			bool				tabLock 			= false;
			String				tooltip;
			CoordTransition_p	transition;
			bool				usePickHandles		= false;
			Axis				wheelOneAxis 		= Axis::Y;
			Axis				wheelTwoAxis		= Axis::X;
			ModKeys				wheelShift 			= ModKeys::Alt;
			pts					wheelStepSize 		= 1*3;
		};
		
		//.____ Creation __________________________________________

		static ScrollPanel_p	create() { return ScrollPanel_p(new ScrollPanel()); }
		static ScrollPanel_p	create( const Blueprint& blueprint ) { return ScrollPanel_p(new ScrollPanel(blueprint)); }

		//.____ Components _______________________________________

		DynamicSlot		slot;
		Scroller		scrollbarX;
		Scroller		scrollbarY;

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Behavior _______________________________________________________

		void			setAutohideScrollbars(bool scrollbarX, bool scrollbarY);
		bool			autohideScrollbarX() const { return m_bAutohideScrollbarX; }
		bool			autohideScrollbarY() const { return m_bAutohideScrollbarY; }

		void			setOverlayScrollbars(bool scrollbarX, bool scrollbarY);
		bool			overlayScrollbarX() const { return m_bOverlayScrollbarX; }
		bool			overlayScrollbarY() const { return m_bOverlayScrollbarY; }


		void			setStepSize(pts stepSize);
		void			setStepSize(pts x, pts y);
		pts				stepSizeX() const { return m_stepSizeX; }
		pts				stepSizeY() const { return m_stepSizeY; }

		void			setWheelStepSize(pts stepSize);
		void			setWheelStepSize(pts x, pts y);
		pts				wheelStepSizeX() const { return m_wheelStepSizeX; }
		pts				wheelStepSizeY() const { return m_wheelStepSizeY; }

		void			setPageOverlap(pts overlap);
		void			setPageOverlap(pts x, pts y);
		pts				pageOverlapX() const { return m_pageOverlapX; }
		pts				pageOverlapY() const { return m_pageOverlapY; }

		void			setWheelAxis(Axis wheelOneAxis, Axis wheelTwoAxis);
		Axis			wheelOneAxis() const { return m_wheelOneAxis; }
		Axis			wheelTwoAxis() const { return m_wheelTwoAxis; }

		void			setWheelAxisShift(ModKeys axisShift);
		ModKeys			wheelAxisShift() const { return m_wheelAxisShiftCombo; }

		void			setStealWheelFromScrollbars(bool bSteal);
		bool			stealWheelFromScrollbars() const { return m_bStealWheelFromScrollbars; }

		void			setAutoscroll(bool autoscrollX, bool autoscrollY);
		bool			autoscrollX() const { return m_bAutoscrollX; }
		bool			autoscrollY() const { return m_bAutoscrollY; }

		void			setChildPlacement(Placement placement);
		Placement		childPlacement() const { return m_smallChildPlacement; }

		bool			setSizeConstraints(SizeConstraint width, SizeConstraint height);
		SizeConstraint	widthConstraint() const { return m_widthConstraint; }
		SizeConstraint	heightConstraint() const { return m_heightConstraint; }


		//.____ Control _____________________________________________________

		void			setRelativeViewOffset(float x, float y, CoordTransition* pTransition = nullptr);

		bool			setViewOffset(Coord offset, CoordTransition* pTransition = nullptr);
		Coord			viewOffset() const;

		void			setTransition(CoordTransition* pTransition);
		CoordTransition_p	transition() const { return m_pDefaultTransition; }

		bool			isTransitioning() const { return m_pTransitionInUse; }

		//.____ Misc ____________________________________________________

		Size			viewSize() const;
		Size			contentSize() const;
				
		//void			setDragCombo(MouseButton button, ModKeys modkeys); // NOT POSSIBLE YET, NEEDS TO BE ABLE TO INTERCEPT MESSAGES.

		//.____ Internal ______________________________________________________

		spx				_matchingHeight(spx width, int scale) const override;
		spx				_matchingWidth(spx height, int scale) const override;

		SizeSPX			_defaultSize(int scale) const override;

		bool			_setViewOffset( CoordSPX offset );
		CoordSPX		_viewOffset() const { return m_childWindow.pos() - m_childCanvas.pos(); } // Can be negative if content is smaller than view and placement is not NorthWest.
		
		bool			_setViewTransition( CoordSPX offset, CoordTransition * pTransition );

		SizeSPX			_viewSize() const { return m_childWindow.size(); }
		SizeSPX			_contentSize() const { return m_childCanvas.size(); }
;

		
	protected:
		ScrollPanel();
		template<class BP> ScrollPanel(const BP& bp) : slot(this), scrollbarX(this, this, Axis::X), scrollbarY(this, this, Axis::Y), Container(bp)
		{
			scrollbarX._initFromBlueprint(bp.scrollbarX);
			scrollbarY._initFromBlueprint(bp.scrollbarY);

			m_smallChildPlacement	= bp.childPlacement;
			m_widthConstraint		= bp.childConstraintX;
			m_heightConstraint		= bp.childConstraintY;

			m_bAutoscrollX			= bp.autoscroll;
			m_bAutoscrollY			= bp.autoscroll;

			m_bAutohideScrollbarX	= bp.autohideScrollbars;
			m_bAutohideScrollbarY	= bp.autohideScrollbars;
			
			m_bOverlayScrollbarX	= bp.overlayScrollbars;
			m_bOverlayScrollbarY	= bp.overlayScrollbars;

			m_stepSizeX 			= bp.stepSize;
			m_stepSizeY				= bp.stepSize;

			m_wheelStepSizeX		= bp.wheelStepSize;
			m_wheelStepSizeY		= bp.wheelStepSize;

			m_pageOverlapX			= bp.pageOverlap;
			m_pageOverlapY			= bp.pageOverlap;

			m_wheelOneAxis			= bp.wheelOneAxis;
			m_wheelTwoAxis			= bp.wheelTwoAxis;

			m_wheelAxisShiftCombo	= bp.wheelShift;
			m_bStealWheelFromScrollbars = bp.stealWheelFromScrollbars;

			m_pDefaultTransition	= bp.transition;
			
			_updateRegions();
			_updateCanvasSize();
			
			if( bp.child )
			slot._setWidget(bp.child);

			_childWindowCorrection();
			_updateScrollbars();
		}
		
		virtual ~ScrollPanel();

		void		_updateRegions();

		void		_childWindowCorrection();

		void		_updateCanvasSize();
		void		_updateScrollbars();

		SizeSPX		_calcCanvasSize( SizeSPX viewMinSize, SizeSPX viewMaxSize );


		// Overloaded from Widget

		virtual void _resize(const SizeSPX& size, int scale) override;

		void		_update(int microPassed, int64_t microsecTimestamp) override;

		void		_receive(Msg * pMsg) override;
		void		_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		void		_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip) override;

		bool		_alphaTest(const CoordSPX& ofs) override;

		BorderSPX	_calcOverflow() override;


		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		Widget *	_findWidget(const CoordSPX& pos, SearchMode mode) override;

		Widget *	_firstChild() const override;
		Widget *	_lastChild() const override;

		void		_firstSlotWithGeo(SlotWithGeo& package) const override;
		void		_nextSlotWithGeo(SlotWithGeo& package) const override;

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

		Widget *	_prevChild(const StaticSlot * pSlot) const override;
		Widget *	_nextChild(const StaticSlot * pSlot) const override;

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

		RectSPX			m_viewRegion;
		RectSPX			m_scrollbarXRegion;
		RectSPX			m_scrollbarYRegion;

		RectSPX			m_childCanvas;								// Child canvas in our coordinate system
		RectSPX			m_childWindow;								// Child view in our coordinate system.
		SizeSPX			m_childDefaultSize;							// Default size from our child.

		Placement		m_smallChildPlacement = Placement::NorthWest;	// Child placement within view if smaller than view.
		SizeConstraint	m_widthConstraint = SizeConstraint::None;		// Constraint of child width.
		SizeConstraint	m_heightConstraint = SizeConstraint::None;		// Constraint of child height.

		bool			m_bAutohideScrollbarX = true;
		bool			m_bAutohideScrollbarY = true;

		bool			m_bOverlayScrollbarX = false;
		bool			m_bOverlayScrollbarY = false;

		spx				m_viewXOfs = 0, m_viewXLen = 0, m_canvasXLen = 0;
		spx				m_viewYOfs = 0, m_viewYLen = 0, m_canvasYLen = 0;

		pts				m_stepSizeX = 8, m_stepSizeY = 8;
		pts				m_wheelStepSizeX = 1*3, m_wheelStepSizeY = 1*3;
		pts				m_pageOverlapX = 8, m_pageOverlapY = 8;

		Axis			m_wheelOneAxis = Axis::Y;		// What wheel should be used for horizontal scrolling. (0=none)
		Axis			m_wheelTwoAxis = Axis::X;		// What wheel should be used for vertical scrolling. (0=none)
		ModKeys			m_wheelAxisShiftCombo = ModKeys::Alt;
		bool			m_bStealWheelFromScrollbars = false;

		bool			m_bAutoscrollX = false;
		bool			m_bAutoscrollY = false;

		bool			m_bChildRequestedResize = false;
		
		// Transition related
		
		CoordTransition_p	m_pDefaultTransition;
		CoordTransition_p	m_pTransitionInUse;
		int					m_transitionProgress = 0;
		
		CoordSPX			m_startViewOfs;
		CoordSPX			m_endViewOfs;
		
		
	};


} // namespace wg
#endif //WG_SCROLLPANEL_DOT_H

