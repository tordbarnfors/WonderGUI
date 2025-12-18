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
#include <wg_rootpanel.h>
#include <wg_base.h>
#include <wg_container.h>
#include <wg_boxskin.h>
#include <wg_inputhandler.h>
#include <wg_util.h>

#include <new>


#include <wg_msgrouter.h>


namespace wg
{
	using namespace Util;

	const TypeInfo RootPanel::TYPEINFO = { "RootPanel", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	RootPanel_p	RootPanel::create() 
	{ 
		return RootPanel_p(new RootPanel()); 
	}

	RootPanel_p	RootPanel::create(const Blueprint& bp)
	{
		return RootPanel_p(new RootPanel(bp));
	}

	RootPanel_p	RootPanel::create( Surface* pCanvas, GfxDevice* pDevice )
	{ 
		if (!pCanvas)
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Provided canvas is null.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return RootPanel_p(new RootPanel(pCanvas, pDevice)); 
	}

	RootPanel_p	RootPanel::create( CanvasRef ref, GfxDevice* pDevice )
	{
		if( ref < CanvasRef_min || ref > CanvasRef_max || ref == CanvasRef::None )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Provided CanvasRef is invalid.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);	
			return nullptr;
		}

		GfxDevice* pUseDevice = pDevice ? pDevice : Base::defaultGfxDevice().rawPtr();
		if (pUseDevice == nullptr)
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "No GfxDevice provided and no default GfxDevice set.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		if(pUseDevice->canvas(ref).size.isEmpty() )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Provided CanvasRef does not refer to a valid canvas.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return RootPanel_p(new RootPanel(ref, pDevice));
	}


	//____ Constructor ____________________________________________________________

	RootPanel::RootPanel() : slot(this), m_skin(this)
	{
		BoxSkin::Blueprint bp;
		bp.color = HiColor(4096, 0, 0, 2048);
		bp.outlineThickness = 1;
		bp.outlineColor = HiColor(4096, 0, 0, 2048);
		
		BoxSkin::StateBP sbp;
		sbp.state = State::Focused;
		sbp.data.color = HiColor(4096, 0, 0, 2048);
		sbp.data.outlineColor = HiColor(4096, 0, 0, 4096);
		bp.states.push_back(sbp);

		m_pDebugOverlay = BoxSkin::create(bp);
		m_afterglowFrames = 4;
	}

	RootPanel::RootPanel(const Blueprint& bp) : slot(this), m_skin(this)
	{
		m_afterglowFrames = bp.debugAfterglow;
		m_pGfxDevice = bp.gfxDevice;
		m_pCanvasLayers = bp.canvasLayers;
		m_windowRef = bp.windowRef;
				
		GfxDevice* pUseGfxDevice = bp.gfxDevice ? bp.gfxDevice.rawPtr() : Base::defaultGfxDevice().rawPtr();

		if( bp.canvasRef != CanvasRef::None )
		{
			m_canvas = pUseGfxDevice->canvas(bp.canvasRef);
		}
		else if( bp.canvasSurface )
		{
			m_canvas.ref = CanvasRef::None;
			m_canvas.pSurface = bp.canvasSurface;
			m_canvas.size = bp.canvasSurface->pixelSize()*64;
			m_canvas.scale = bp.canvasSurface->scale();
		}

		m_geo = m_canvas.size;

		if( bp.scale > 0 )
			m_scale = bp.scale;
		else
			m_scale = m_canvas.scale;
		
		if( !bp.geo.isEmpty() )
		{
			RectSPX geoSpx = RectSPX::overlap(m_canvas.size, align(ptsToSpx(bp.geo,m_scale)));
			if( !geoSpx.isEmpty() )
			{
				m_bHasGeo = true;
				m_geo = geoSpx;
			}
		}
	
		if( bp.finalizer )
			setFinalizer(bp.finalizer);

		if (bp.skin)
			m_skin.set(bp.skin);

		if( bp.debugOverlay )
			m_pDebugOverlay = bp.debugOverlay;
		else
		{
			BoxSkin::Blueprint bp;
			bp.color = HiColor(4096, 0, 0, 2048);
			bp.outlineThickness = 1;
			bp.outlineColor = HiColor(4096, 0, 0, 2048);
			
			BoxSkin::StateBP sbp;
			sbp.state = State::Focused;
			sbp.data.color = HiColor(4096, 0, 0, 2048);
			sbp.data.outlineColor = HiColor(4096, 0, 0, 4096);
			bp.states.push_back(sbp);

			m_pDebugOverlay = BoxSkin::create(bp);
		}
	}

	RootPanel::RootPanel(Surface* pSurface, GfxDevice * pGfxDevice ) : RootPanel()
	{
		m_canvas.ref = CanvasRef::None;
		m_canvas.pSurface = pSurface;
		m_canvas.size = pSurface->pixelSize()*64;
		m_canvas.scale = pSurface->scale();

		m_geo = m_canvas.size;
		m_pGfxDevice = pGfxDevice;
		m_scale = m_canvas.scale;
	}

	RootPanel::RootPanel(CanvasRef ref, GfxDevice* pGfxDevice) : RootPanel()
	{
		GfxDevice* pUseGfxDevice = pGfxDevice ? pGfxDevice : Base::defaultGfxDevice().rawPtr();

		m_canvas = pUseGfxDevice->canvas(ref);

		m_geo = m_canvas.size;
		m_pGfxDevice = pGfxDevice;
		m_scale = m_canvas.scale;
	}


	//____ Destructor _____________________________________________________________

	RootPanel::~RootPanel()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& RootPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setGfxDevice() _________________________________________________________

	bool RootPanel::setGfxDevice(GfxDevice* pDevice)
	{
		m_pGfxDevice = pDevice;

		m_dirtyPatches.add( m_geo );
		return true;
	}

	//____setWindowRef() _________________________________________________________

	void RootPanel::setWindowRef( uintptr_t windowRef )
	{
		m_windowRef = windowRef;
	}

	//____ setCanvas() ________________________________________________________

	bool RootPanel::setCanvas(Surface* pSurface)
	{
		if (!pSurface)
		{
			return false;
		}

		if( pSurface == m_canvas.pSurface )
		{
			return true;
		}
		
		int oldScale = m_scale;
		SizeSPX oldSize = m_geo.size();

		m_canvas.ref = CanvasRef::None;
		m_canvas.pSurface = pSurface;
		m_canvas.size = pSurface->pixelSize()*64;
		m_canvas.scale = pSurface->scale();

		if (!m_bScaleSet)
			m_scale = m_canvas.scale;

		if (!m_bHasGeo)
			m_geo = m_canvas.size;

		if ((m_scale != oldScale || oldSize != m_geo.size()) && slot._widget())
			slot._setSize(m_geo.size(),m_scale);

		m_dirtyPatches.add(m_geo);
		return true;
	}

	bool RootPanel::setCanvas(CanvasRef canvasRef)
	{
        GfxDevice* pGfxDevice = m_pGfxDevice ? m_pGfxDevice : Base::defaultGfxDevice();

        if( !pGfxDevice )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "No GfxDevice provided and no default GfxDevice set.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}
			
		if( pGfxDevice->canvas(canvasRef).size.isEmpty() )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Provided CanvasRef does not refer to a valid canvas.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		int oldScale = m_scale;
		SizeSPX oldSize = m_geo.size();

		m_canvas = pGfxDevice->canvas(canvasRef);

		if (!m_bScaleSet)
			m_scale = m_canvas.scale;

		if (!m_bHasGeo)
			m_geo = m_canvas.size;

		if ((m_scale != oldScale || oldSize != m_geo.size()) && slot._widget())
			slot._setSize(m_geo.size(), m_scale);

		m_dirtyPatches.add(m_geo);
		return true;
	}

	//____ setCanvasLayers() ___________________________________________________

	void RootPanel::setCanvasLayers( CanvasLayers * pLayers )
	{
		if( pLayers != m_pCanvasLayers )
		{
			m_pCanvasLayers = pLayers;
			m_dirtyPatches.add(m_geo);
		}
	}

	//____ setScale() __________________________________________________________

	bool RootPanel::setScale(int scale)
	{
		if (scale != m_scale)
		{
			m_scale = scale;
			m_bScaleSet = true;
			if (slot._widget())
			{
				slot._setSize(m_geo.size(),m_scale);
				m_dirtyPatches.add(m_geo);
			}
		}

		return true;
	}

	//____ clearScale() _________________________________________________________

	void RootPanel::clearScale()
	{
		if (m_bScaleSet)
		{
			m_bScaleSet = false;
			m_scale = m_canvas.scale;
			if (slot._widget())
			{
				slot._setSize(m_geo.size(), m_scale);
				m_dirtyPatches.add(m_geo);
			}
		}
	}

	//____ scale() ____________________________________________________________

	int RootPanel::scale() const
	{
		return m_scale;
	}

	//_____ setGeo() _____________________________________________________________

	bool RootPanel::setGeo(const Rect& _geo)
	{
		RectSPX geoSpx = RectSPX::overlap(m_canvas.size, align(ptsToSpx(_geo,m_scale)));

		m_bHasGeo = !_geo.isEmpty();

		if (m_geo != geoSpx)
		{
			m_geo = geoSpx;
			if (slot._widget())
				slot._setSize(m_geo.size(), m_scale);
		}

		return true;
	}

	//____ geo() __________________________________________________________________

	Rect RootPanel::geo() const
	{
		if (m_bHasGeo)
			return spxToPts(m_geo,m_scale);
		else
			return spxToPts(m_canvas.size,m_scale);
	}

	//____ setVisible() ___________________________________________________________

	bool RootPanel::setVisible(bool bVisible)
	{
		if (bVisible != m_bVisible)
		{
			m_bVisible = bVisible;
			m_dirtyPatches.add(m_geo);
		}
		return true;
	}

	//____ setSkin() __________________________________________________________

	void RootPanel::setSkin(Skin* pSkin)
	{
		//TODO: Possibly resize child.

		m_skin.set(pSkin);
		m_dirtyPatches.add(m_geo);
	}

	//____ setPointerStyle() _____________________________________________________

	void RootPanel::setPointerStyle( PointerStyle style )
	{
		m_pointerStyle = style;
	}

	//____ setDebugMode() ______________________________________________________

	void RootPanel::setDebugMode(bool onOff)
	{
		m_bDebugMode = onOff;

		// Make sure to clean up on the screen.

		if (m_bDebugMode == false)
		{
			for (auto it = m_afterglowRects.begin(); it != m_afterglowRects.end(); it++)
				m_dirtyPatches.add(&(*it));

			m_afterglowRects.clear();
		}
	}


	//____ setDebugOverlay() ____________________________________________________

	void RootPanel::setDebugOverlay(Skin* pOverlaySkin)
	{
		m_pDebugOverlay = pOverlaySkin;
	}

	//____ setDebugAfterglow() ____________________________________________________

	void RootPanel::setDebugAfterglow(int frames)
	{
		limit(frames, 0, 100);
		m_afterglowFrames = frames;
	}

	//____ render() _______________________________________________________________

	bool RootPanel::render()
	{
		return render(geo());
	}

	bool RootPanel::render(const Rect& clip)
	{
		if( !beginRender() )
			return false;

		if (!renderSection(clip))
		{
			endRender();
			return false;
		}

		if (!endRender())
			return false;

		return true;
	}

	//____ beginRender() __________________________________________________________

	bool RootPanel::beginRender()
	{
		GfxDevice* pGfxDevice = m_pGfxDevice ? m_pGfxDevice : Base::defaultGfxDevice();

		if( !pGfxDevice )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No GfxDevice provided and no default GfxDevice set.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;						// No GFX-device or no widgets to render.
		}

		if( m_canvas.size.isEmpty() )
		{
			//TODO: Error handling!
			return false;						// No GFX-device or no widgets to render.
		}

		// Handle preRender calls.

		for (auto& pWidget : m_preRenderCalls)
			pWidget->_preRender();

		m_preRenderCalls.clear();

		// Handle debug overlays.

		if (m_bDebugMode)
		{
			// Remove from afterglow queue patches that are overlapped by our new dirty patches.

			for (std::deque<PatchesSPX>::iterator it = m_afterglowRects.begin(); it != m_afterglowRects.end(); ++it)
				it->sub(&m_dirtyPatches);

			// Add our new dirty patches to the top of the afterglow queue.

			m_afterglowRects.push_front(PatchesSPX());
			m_afterglowRects.front().add(&m_dirtyPatches);

			// Possibly remove overlays from the back, put them into dirty rects for re-render

			while ((int)m_afterglowRects.size() > m_afterglowFrames + 1)
			{
				m_dirtyPatches.add(&m_afterglowRects.back());
				m_afterglowRects.pop_back();
			}

			// Re-render graphics behind overlays that go from state FOCUSED to NORMAL

			if (m_afterglowRects.size() > 1)
			{
				m_dirtyPatches.add(&m_afterglowRects[1]);
			}
		}

		// Initialize GFX-device.

		if (pGfxDevice->beginRender())
			m_bRendering = true;

		return m_bRendering;
	}


	//____ renderSection() __________________________________________________________

	bool RootPanel::renderSection(const Rect& _clip)
	{
		GfxDevice* pGfxDevice = m_pGfxDevice ? m_pGfxDevice : Base::defaultGfxDevice();

		if (!m_bRendering)
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "GfxDevice not in rendering state. Did you forget to call beginRender()?", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}


		if( m_canvas.size.isEmpty() )
		{
			//TODO: Error handling!
			return false;						// Missing GfxDevice or Canvas or widgets to render.
		}
		
		// Make sure we have a vaild clip rectangle (doesn't go outside our geometry and has an area)

		RectSPX geo = m_geo;
		RectSPX clip = RectSPX::overlap(align(ptsToSpx(_clip,m_scale)), geo);
		if( clip.w == 0 || clip.h == 0 )
			return false;						// Invalid rect area.

		// Nothing to render if we are hidden

		if (!m_bVisible)
			return true;						// Not an error, just hidden.

		// Copy and clip our dirty patches

		PatchesSPX dirtyPatches(m_dirtyPatches, clip);

		// Render the dirty patches recursively

		if (dirtyPatches.size() > 0)
		{
			int nRects = dirtyPatches.size();
			const RectSPX* pRects = dirtyPatches.begin();

			if (m_canvas.ref == CanvasRef::None )
				pGfxDevice->beginCanvasUpdate(m_canvas.pSurface, nRects, pRects, m_pCanvasLayers);
			else
				pGfxDevice->beginCanvasUpdate(m_canvas.ref, nRects, pRects, m_pCanvasLayers);

			m_skin.render(pGfxDevice, geo, m_scale, State::Default);

			if( !slot.isEmpty() )
				slot._widget()->_render( pGfxDevice, geo, geo );

			pGfxDevice->endCanvasUpdate();
		}

		// Handle updated rect overlays

		if (m_bDebugMode && m_pDebugOverlay)
		{
			// Set clipping rectangle.

			if (m_canvas.ref == CanvasRef::None )
				pGfxDevice->beginCanvasUpdate(m_canvas.pSurface, 1, &clip, nullptr, 0);
			else
				pGfxDevice->beginCanvasUpdate(m_canvas.ref, 1, &clip, nullptr, 0);

			// Render our new overlays

			for (const RectSPX* pRect = m_afterglowRects[0].begin(); pRect != m_afterglowRects[0].end(); pRect++)
			{
				m_pDebugOverlay->_render(pGfxDevice, *pRect, m_scale, State::Focused);
			}

			// Render overlays that have turned into afterglow

			if (m_afterglowRects.size() > 1)
			{
				for (const RectSPX* pRect = m_afterglowRects[1].begin(); pRect != m_afterglowRects[1].end(); pRect++)
				{
					m_pDebugOverlay->_render(pGfxDevice, *pRect, m_scale, State::Default);
				}
			}

			pGfxDevice->endCanvasUpdate();
		}

		return true;
	}

	//____ endRender() ____________________________________________________________

	bool RootPanel::endRender(void)
	{
		GfxDevice* pGfxDevice = m_pGfxDevice ? m_pGfxDevice : Base::defaultGfxDevice();

		if (!m_bRendering)
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "GfxDevice not in rendering state. Did you forget to call beginRender()?", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		
		// Turn dirty patches into update patches
		//TODO: Optimize by just making a swap.

		m_updatedPatches.clear();
		m_updatedPatches.add(&m_dirtyPatches);
		m_dirtyPatches.clear();

		m_bRendering = false;
		return pGfxDevice->endRender();
	}

	//____ findWidget() _______________________________________________________

	Widget_p RootPanel::findWidget(const Coord& ofs, SearchMode mode)
	{
		return Widget_p(_findWidget(Util::ptsToSpx(ofs, m_scale) - m_geo.pos(), mode));
	}

	//____ _findWidget() _____________________________________________________________

	Widget* RootPanel::_findWidget(const CoordSPX& ofs, SearchMode mode)
	{
		if (!m_geo.contains(ofs) || !slot._widget())
			return nullptr;

		if (slot._widget() && slot._widget()->isContainer())
			return static_cast<Container*>(slot._widget())->_findWidget(ofs, mode);

		if (mode == SearchMode::Geometry || slot._widget()->_markTest(ofs))
			return slot._widget();

		return nullptr;
	}

	//____ _child() ______________________________________________________________

	Widget * RootPanel::_child() const
	{
		return slot._widget();
	}


	//____ _focusedChild() ______________________________________________________

	Widget* RootPanel::_focusedChild() const
	{
		if (!m_pFocusedChild)
			return slot._widget();

		return m_pFocusedChild;
	}


	//____ _slotGeo() ________________________________________________________

	RectSPX RootPanel::_slotGeo(const StaticSlot* pSlot) const
	{
		return m_geo;
	}

	//____ _childOverflowChanged() _______________________________________________

	void RootPanel::_childOverflowChanged( StaticSlot * pSlot, const BorderSPX& oldOverflow, const BorderSPX& newOverflow )
	{
		
	}


	//____ _childLocalToGlobal() _______________________________________________

	RectSPX RootPanel::_childLocalToGlobal(const StaticSlot* pSlot, const RectSPX& rect) const
	{
		return rect + m_geo.pos();
	}

	//____ _globalToChildLocal() ________________________________________________

	RectSPX RootPanel::_globalToChildLocal(const StaticSlot* pSlot, const RectSPX& rect) const
	{
		return rect - m_geo.pos();
	}

	//____ _globalPtsToChildLocalSpx() ___________________________________________

	RectSPX RootPanel::_globalPtsToChildLocalSpx(const StaticSlot* pSlot, const Rect& rect) const
	{
		return Util::ptsToSpx(rect, m_scale) - m_geo.pos();
	}

	//____ _childLocalSpxToGlobalPts() ___________________________________________

	Rect RootPanel::_childLocalSpxToGlobalPts(const StaticSlot* pSlot, const RectSPX& rect) const
	{
		return Util::spxToPts( rect + m_geo.pos(), m_scale );
	}

	//____ _isChildVisible() __________________________________________________

	bool RootPanel::_isChildVisible(const StaticSlot* pSlot) const
	{
		return true;
	}

	//____ _childWindowSection() __________________________________________________

	RectSPX RootPanel::_childWindowSection(const StaticSlot* pSlot) const
	{
		return m_geo;
	}

	//____ _slotTypeInfo() ________________________________________________________

	const TypeInfo& RootPanel::_slotTypeInfo(const StaticSlot* pSlot) const
	{
		return DynamicSlot::TYPEINFO;
	}

	//____ _container() __________________________________________________

	Container* RootPanel::_container()
	{
		return nullptr;
	}

	//____ _root() __________________________________________________

	Root* RootPanel::_root()
	{
		return this;
	}

	//____ _scale() _______________________________________________

	int RootPanel::_scale() const
	{
		return m_scale;
	}

	void RootPanel::_childRequestRender(StaticSlot* pSlot, const RectSPX& rect)
	{
		if (m_bVisible)
			addDirtyPatch(rect + m_geo.pos());
	}

	//____ _childRequestResize() __________________________________________________

	void RootPanel::_childRequestResize(StaticSlot* pSlot)
	{
		slot._setSize(m_geo.size(), m_scale);
	}

	//____ _childRequestFocus() __________________________________________________

	bool RootPanel::_childRequestFocus(StaticSlot* pSlot, Widget* pWidget, bool bRaiseWindow)
	{
		// Set new focused widget for this Root
		
		Widget* pOldFocus = m_pFocusedChild.rawPtr();
		m_pFocusedChild = pWidget;

		// Check if we need to request window focus.

		if (bRaiseWindow && Base::inputHandler()->focusedWindow() != this)
		{
			if (Base::hostBridge() && !Base::hostBridge()->requestFocus(m_windowRef))
				return false;

			Base::inputHandler()->setFocusedWindow(this);
		}
		
		//

		if( pOldFocus == pWidget )
			return true;
		
		return Base::inputHandler()->_focusChanged(this, pOldFocus, pWidget);
	}

	//____ _childReleaseFocus() __________________________________________________

	bool RootPanel::_childReleaseFocus(StaticSlot* pSlot, Widget* pWidget)
	{
		if (pWidget != m_pFocusedChild.rawPtr())
			return true;					// Didn't have focus, although widget seems to believe it.

		Widget* pNewFocus = slot._widget();	// We revert keyboard focus to our base widget.

		if (Base::hostBridge() && Base::hostBridge()->yieldFocus(m_windowRef)) // Let OS know that we don't need focus anymore.
			Base::inputHandler()->setFocusedWindow(nullptr);
		else if( m_pFocusedChild != pNewFocus )
			Base::inputHandler()->_focusChanged(this, m_pFocusedChild, pNewFocus );

		m_pFocusedChild = pNewFocus;
		
		return (Base::inputHandler()->focusedWidget() != pWidget);
	}

	//____ _childRequestInView() __________________________________________________

	void RootPanel::_childRequestInView(StaticSlot* pSlot)
	{
		// Do nothing, root ignores inView requests.
	}
	void RootPanel::_childRequestInView(StaticSlot* pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea)
	{
		// Do nothing, root ignores inView requests.
	}

	//____ _prevChild() __________________________________________________

	Widget* RootPanel::_prevChild(const StaticSlot* pSlot) const
	{
		return nullptr;
	}

	//____ _nextChild() __________________________________________________

	Widget* RootPanel::_nextChild(const StaticSlot* pSlot) const
	{
		return nullptr;
	}

	//____ _releaseChild() ____________________________________________________

	void RootPanel::_releaseChild(StaticSlot* pSlot)
	{
		_replaceChild(pSlot, nullptr);
	}

	//____ _replaceChild() ____________________________________________________________

	void RootPanel::_replaceChild(StaticSlot* pSlot, Widget* pNewWidget)
	{
		slot._setWidget(pNewWidget);

		if (pNewWidget)
			pNewWidget->_resize(m_geo.size(),m_scale);

		m_dirtyPatches.clear();
		m_dirtyPatches.add(m_geo);
	}

	//____ _didAddSlots() _____________________________________________________

	void RootPanel::_didAddSlots(StaticSlot* pSlot, int nb)
	{
		return;				// RootPanel doesn't support adding/moving/erasing slots
	}

	//____ _didMoveSlots() ____________________________________________________

	void RootPanel::_didMoveSlots(StaticSlot* pFrom, StaticSlot* pTo, int nb)
	{
		return;				// RootPanel doesn't support adding/moving/erasing slots
	}

	//____ _willEraseSlots() __________________________________________________

	void RootPanel::_willEraseSlots(StaticSlot* pSlot, int nb)
	{
		return;				// RootPanel doesn't support adding/moving/erasing slots
	}

	//____ _skinValue() _______________________________________________________

	float RootPanel::_skinValue(const SkinSlot* pSlot) const
	{
		return 1.f;
	}

	//____ _skinValue2() ______________________________________________________

	float RootPanel::_skinValue2(const SkinSlot* pSlot) const
	{
		return -1.f;
	}

	//____ _skinState() _______________________________________________________

	State RootPanel::_skinState(const SkinSlot* pSlot) const
	{
		return State::Default;
	}

	//____ _skinSize() ________________________________________________________

	SizeSPX RootPanel::_skinSize(const SkinSlot* pSlot) const
	{
		return m_geo;
	}

	//____ _skinRequestRender() _______________________________________________

	void RootPanel::_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect)
	{
		m_dirtyPatches.push(rect);
	}


} // namespace wg
