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
#ifndef WG_ROOTPANEL_DOT_H
#define WG_ROOTPANEL_DOT_H
#pragma once

#include <wg_widget.h>
#include <wg_slotholder.h>
#include <wg_geo.h>
#include <wg_patches.h>
#include <wg_msgrouter.h>
#include <wg_gfxdevice.h>
#include <wg_slot.h>
#include <wg_root.h>

namespace wg
{

	class Widget;


	class RootPanel;
	typedef	StrongPtr<RootPanel>	RootPanel_p;
	typedef	WeakPtr<RootPanel>	RootPanel_wp;


	//____ RootPanel ____________________________________________________________

	/**
	 * @brief	Special widget, being the root of all widgets on screen.
	 *
	 * Special widget, being the root of all widgets on screen.
	 *
	 *
	 */


	class RootPanel : public Root, protected SlotHolder, protected SkinSlot::Holder
	{
		friend class Widget;
		friend class Container;
		friend class InputHandler;

	public:

		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			CanvasLayers_p	canvasLayers;
			CanvasRef		canvasRef = CanvasRef::None;
			Surface_p		canvasSurface;
			Skin_p			debugOverlay;
			int				debugAfterglow = 4;
			Finalizer_p		finalizer = nullptr;
			Rect			geo;
			GfxDevice_p		gfxDevice;
			PointerStyle	pointerStyle = PointerStyle::Undefined;
			int				scale = 0;
			Skin_p			skin;
			uintptr_t		windowRef = 0;
		};


		//.____ Creation __________________________________________

		static RootPanel_p	create();
		static RootPanel_p	create(const Blueprint& blueprint);
		static RootPanel_p	create(Surface* pCanvasSurface, GfxDevice* pDevice = nullptr);
		static RootPanel_p	create(CanvasRef canvasRef, GfxDevice* pDevice = nullptr);

		//.____ Components ____________________________________

		DynamicSlot			slot;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		bool				setScale(int scale);
		void				clearScale();
		int					scale() const override;
		inline bool			isScaleSet() const;

		bool				setGeo( const Rect& geo );
		Rect				geo() const override;

		//.____ State _________________________________________________

		bool				setVisible( bool bVisible );							//TODO: Implement!!!
		bool				isVisible() const { return m_bVisible; }

		inline Widget_p		focusedChild() const { return _focusedChild(); }

		//.____ Appearance ____________________________________________________

		virtual void		setSkin(Skin* pSkin);
		inline Skin_p		skin() const;

		void				setPointerStyle( PointerStyle style );
		inline PointerStyle pointerStyle() const override { return m_pointerStyle; }

		//.____ Rendering ________________________________________________


		bool				render();
		bool				render( const Rect& clip );

		bool				beginRender();
		bool				renderSection( const Rect& clip );
		bool				endRender();


		//.____ Debug __________________________________________________________

		void				setDebugMode( bool onOff );
		inline bool			isDebugMode() const;
		void 				setDebugOverlay(Skin * pOverlaySkin );
		void				setDebugAfterglow(int frames);
		inline Skin_p		debugOverlay() const;
		inline int			debugAfterglow() const;

		//.____ Misc ___________________________________________________________

		bool					setGfxDevice( GfxDevice * pDevice );
		inline GfxDevice_p 		gfxDevice() const;

		void					setWindowRef( uintptr_t windowRef );
		uintptr_t				windowRef() const { return m_windowRef; }


		bool					setCanvas(Surface* pCanvasSurface);
		bool					setCanvas(CanvasRef canvasRef);
		inline const CanvasInfo&canvasInfo() const;
		inline SizeI			canvasSize() const;

		void					setCanvasLayers( CanvasLayers * pLayers );
		inline CanvasLayers_p	canvasLayers() const;

		void					setDirtyRectAlignment( int alignment );
		int						dirtyRectAlignment() const { return m_dirtyRectAlignment; }

		Widget_p				findWidget(const Coord& ofs, SearchMode mode) override;

		inline int				nbDirtyRects() const;
		inline const RectSPX*	firstDirtyRect() const;

		inline int				nbUpdatedRects() const;
		inline const RectSPX*	firstUpdatedRect() const;

		inline void				addDirtyPatch(const RectSPX& rect) override;
		inline bool				addPreRenderCall(Widget* pWidget) override;


	protected:
		RootPanel();
		RootPanel(const Blueprint& bp);
		RootPanel(Surface* pCanvasSurface, GfxDevice* pGfxDevice );
		RootPanel(CanvasRef canvasRef, GfxDevice* pGfxDevice);
		~RootPanel();

		// SlotHolder methods

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		Container *  	_container() override;
		Root *			_root() override;
		int				_scale() const override;

		RectSPX			_slotGeo( const StaticSlot * pSlot ) const override;
		void			_childOverflowChanged( StaticSlot * pSlot, const BorderSPX& oldOverflow, const BorderSPX& newOverflow ) override;

		RectSPX			_childLocalToGlobal(const StaticSlot* pSlot, const RectSPX& rect) const override;
		RectSPX			_globalToChildLocal(const StaticSlot* pSlot, const RectSPX& rect) const override;
		RectSPX			_globalPtsToChildLocalSpx(const StaticSlot* pSlot, const Rect& rect) const override;
		Rect			_childLocalSpxToGlobalPts(const StaticSlot* pSlot, const RectSPX& rect) const override;

		bool			_isChildVisible( const StaticSlot * pSlot ) const override;
		RectSPX			_childWindowSection( const StaticSlot * pSlot ) const override;

		void			_childRequestRender( StaticSlot * pSlot, const RectSPX& rect ) override;
		void			_childRequestResize( StaticSlot * pSlot ) override;

		bool			_childRequestFocus( StaticSlot * pSlot, Widget * pWidget, bool bRaiseWindow ) override;
		bool			_childReleaseFocus( StaticSlot * pSlot, Widget * pWidget ) override;

		void			_childRequestInView( StaticSlot * pSlot ) override;
		void			_childRequestInView( StaticSlot * pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea ) override;

		Widget *		_prevChild( const StaticSlot * pSlot ) const override;
		Widget *		_nextChild( const StaticSlot * pSlot ) const override;

		void			_releaseChild( StaticSlot * pSlot ) override;
		void			_replaceChild(StaticSlot * pSlot, Widget * pNewChild) override;

		void			_didAddSlots(StaticSlot * pSlot, int nb) override;
		void			_didMoveSlots(StaticSlot * pFrom, StaticSlot * pTo, int nb) override;
		void			_willEraseSlots(StaticSlot * pSlot, int nb) override;

		// Methods for skin to access

		float			_skinValue(const SkinSlot* pSlot) const override;
		float			_skinValue2(const SkinSlot* pSlot) const override;

		State			_skinState(const SkinSlot* pSlot) const override;

		SizeSPX			_skinSize(const SkinSlot* pSlot) const override;

		void			_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect) override;


		Widget *			_findWidget( const CoordSPX& ofs, SearchMode mode );

//		void				_setFocusedChild( Widget * pWidget );

		Widget *			_child() const override;

		Widget *			_focusedChild() const override;

		PatchesSPX			m_dirtyPatches;		// Dirty patches that needs to be rendered.
		PatchesSPX			m_updatedPatches;	// Patches that were updated in last rendering session.

		std::vector<Widget_p>   m_preRenderCalls;

		bool				m_bDebugMode = false;
		bool				m_bRendering = false;
		Skin_p				m_pDebugOverlay;
		int					m_afterglowFrames;
		std::deque<PatchesSPX>	m_afterglowRects;	// Afterglow rects are placed in this queue.

		int					m_dirtyRectAlignment = 1;

		GfxDevice_p			m_pGfxDevice;
		CanvasInfo			m_canvas;			// Size of canvas in subpixels, when m_pCanvas is null.
		CanvasLayers_p		m_pCanvasLayers;

		RectSPX				m_geo;
		bool				m_bHasGeo = false;

		int					m_scale = 64;
		bool				m_bScaleSet = false;// Set when scale is explicitly specified and not taken from canvas.

		bool				m_bVisible = true;

		SkinSlot			m_skin;				//TODO: Padding is not respected yet.

		uintptr_t			m_windowRef = 0;

		Widget_wp			m_pFocusedChild;

		PointerStyle		m_pointerStyle = PointerStyle::Undefined;
	};

	//____ isScaleSet() _______________________________________________________

	bool RootPanel::isScaleSet() const
	{
		return m_bScaleSet;
	}

	//____ skin() _____________________________________________________________

	Skin_p RootPanel::skin() const
	{
		return m_skin.get();
	}

	//____ isDebugMode() ______________________________________________________

	bool RootPanel::isDebugMode() const
	{
		return m_bDebugMode;
	}

	//____ debugOverlay() _____________________________________________________

	inline Skin_p RootPanel::debugOverlay() const
	{
		return m_pDebugOverlay;
	}

	//____ debugAfterglow() ___________________________________________________

	int RootPanel::debugAfterglow() const
	{
		return m_afterglowFrames;
	}

	//____ gfxDevice() ___________________________________________________

	GfxDevice_p RootPanel::gfxDevice() const
	{
		return m_pGfxDevice;
	}

	//____ canvasInfo() ___________________________________________________

	const CanvasInfo& RootPanel::canvasInfo() const
	{
		return m_canvas;
	}

	//____ canvasSize() ___________________________________________________

	SizeI RootPanel::canvasSize() const
	{
		return m_canvas.size;
	}

	//____ canvasLayers() ___________________________________________________

	CanvasLayers_p RootPanel::canvasLayers() const
	{
		return m_pCanvasLayers;
	}

	//____ nbDirtyRects() _____________________________________________________

	int RootPanel::nbDirtyRects() const
	{
		return m_dirtyPatches.size();
	}

	//____ firstDirtyRect() ___________________________________________________

	const RectSPX* RootPanel::firstDirtyRect() const
	{
		return m_dirtyPatches.isEmpty() ? nullptr : m_dirtyPatches.begin();
	}

	//____ nbUpdateRects() ____________________________________________________

	int RootPanel::nbUpdatedRects() const
	{
		return m_updatedPatches.size();
	}

	//____ firstUpdateRect() __________________________________________________

	const RectSPX* RootPanel::firstUpdatedRect() const
	{
		return m_updatedPatches.isEmpty() ? nullptr : m_updatedPatches.begin();
	}

	//____ addDirtyPatch() ____________________________________________________

	void RootPanel::addDirtyPatch(const RectSPX& rect)
	{
		if( m_dirtyRectAlignment > 1 )
		{
			RectSPX aligned;

			int adder = (m_dirtyRectAlignment*64)-1;
			int mask = ~adder;

			aligned.x = rect.x & mask;
			aligned.y = rect.y;
			aligned.w = ((rect.x + rect.w + adder) & mask) - aligned.x;
			aligned.h = rect.h;

			m_dirtyPatches.add( aligned );
		}
		else
			m_dirtyPatches.add( rect );
	}

	//____ addPreRenderCall() _________________________________________________

	bool RootPanel::addPreRenderCall(Widget* pWidget)
	{
		m_preRenderCalls.push_back(pWidget);
		return true;
	}


} // namespace wg
#endif //WG_ROOTPANEL_DOT_H
