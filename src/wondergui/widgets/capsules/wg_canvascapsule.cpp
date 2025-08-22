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

#include <wg_canvascapsule.h>
#include <wg_canvasdisplay.h>

#include <wg_gfxdevice.h>
#include <wg_base.h>
#include <wg_util.h>

#include <wg_boxskin.h>
#include <wg_snapshottintmap.h>
#include <wg_gradyent.h>

namespace wg
{
	const TypeInfo CanvasCapsule::TYPEINFO = { "CanvasCapsule", &Capsule::TYPEINFO };

	using namespace Util;

	//____ constructor ____________________________________________________________

	CanvasCapsule::CanvasCapsule() : glow(this)
	{
		m_canvasSize = m_size;
	}

	//____ destructor _____________________________________________________________

	CanvasCapsule::~CanvasCapsule()
	{
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& CanvasCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setSkin() ___________________________________________________________

	void CanvasCapsule::setSkin(Skin* pSkin)
	{
		if( !m_bScaleCanvas )
		{
			SizeSPX oldBorderSize = m_skin.contentBorderSize(m_scale);
			SizeSPX newBorderSize = pSkin ? pSkin->_contentBorderSize(m_scale) : SizeSPX();

			if( oldBorderSize != newBorderSize )
			{
				_setCanvasSize( _contentRect() );
			}
		}

		Capsule::setSkin(pSkin);
	}


	//____ setTintColor() ______________________________________________________

	void CanvasCapsule::setTintColor(HiColor color, ColorTransition* pTransition)
	{
		if (color != m_tintColor)
		{
			if (pTransition)
			{
				if (!m_pTintColorTransition)
					_startReceiveUpdates();

				m_pTintColorTransition = pTransition;
				m_tintColorTransitionProgress = 0;
				m_startTintColor = m_tintColor;
				m_endTintColor = color;
			}
			else
			{
				if (m_pTintColorTransition)
					_stopReceiveUpdates();

				m_pTintColorTransition = nullptr;
				m_tintColor = color;
				_requestRender();
			}
		}
	}

	//____ setTintmap() __________________________________________________

	void CanvasCapsule::setTintmap(Tintmap* pTintmap, ColorTransition* pTransition)
	{
		if (pTintmap != m_pTintmap)
		{
			if (pTransition)
			{
				if (!m_pTintmapTransition)
					_startReceiveUpdates();

				m_pTintmapTransition = pTransition;
				m_tintmapTransitionProgress = 0;
				m_pStartTintmap = m_pTintmap ? m_pTintmap : Gradyent::create(HiColor::White, HiColor::White, HiColor::White, HiColor::White);
				m_pEndTintmap = pTintmap;
			}
			else
			{
				if (m_pTintmapTransition)
					_stopReceiveUpdates();

				m_pTintmapTransition = nullptr;
				m_pTintmap = pTintmap;
				_requestRender();
			}

		}
	}

	//____ setBlendMode() __________________________________________________________

	void CanvasCapsule::setBlendMode(BlendMode mode)
	{
		if (mode != m_blendMode)
		{
			m_blendMode = mode;
			_requestRender();
		}
	}

	//____ setSurfaceFactory() _________________________________________________

	void CanvasCapsule::setSurfaceFactory(SurfaceFactory* pFactory)
	{
		if (pFactory != m_pFactory)
		{
			m_pFactory = pFactory;
			m_pCanvas = nullptr;
			_requestRender();
		}
	}

	//____ setFormat() _____________________________________________________

	void CanvasCapsule::setFormat(PixelFormat format)
	{
		if (format != m_canvasFormat)
		{
			m_canvasFormat = format;
			m_pCanvas = nullptr;
			_requestRender();
		}
	}

	//____ setCanvasLayers() ___________________________________________________

	void CanvasCapsule::setCanvasLayers(CanvasLayers * pLayers)
	{
		if( pLayers != m_pCanvasLayers )
		{
			m_pCanvasLayers = pLayers;
			_requestRender();
		}
	}

	//____ setPlacement() _______________________________________________

	void CanvasCapsule::setPlacement(Placement placement)
	{
		if (placement != m_placement)
		{
			m_placement = placement;
			if( m_bScaleCanvas )
			_requestRender();
		}
	}


	//____ setRenderLayer() ___________________________________________________

	void CanvasCapsule::setRenderLayer(int renderLayer)
	{
		if (renderLayer != m_renderLayer)
		{
			m_renderLayer = renderLayer;
			_requestRender();
		}
	}

	//____ setScaleCanvas() ______________________________________________________

	void CanvasCapsule::setScaleCanvas(bool bScale)
	{
		if( bScale != m_bScaleCanvas )
		{
			m_bScaleCanvas = bScale;
			_resizeCanvasAndChild();
			_requestRender();
		}
	}

	//____ canvasGeo() ________________________________________________________

	Rect CanvasCapsule::canvasGeo() const
	{
		RectSPX contentRect = _contentRect(m_size);
		RectSPX canvasArea = m_bScaleCanvas ? _canvasWindow(contentRect) : contentRect;

		return spxToPts(canvasArea, m_scale);
	}

	//____ setClearColor() _______________________________________________________

	void CanvasCapsule::setClearColor( HiColor color )
	{
		if (color != m_clearColor)
		{
			m_clearColor = color;

			m_patches.clear();
			m_patches.push(m_canvasSize);
			_requestRender( _slotGeo(&slot) );
		}
	}

	//____ _matchingHeight() _____________________________________________________

	spx CanvasCapsule::_matchingHeight(spx width, int scale) const
	{
		if( m_bScaleCanvas && !slot.isEmpty() )
		{
			SizeSPX border = m_skin.contentBorderSize(scale);

			spx givenWidth = width - border.w;
			SizeSPX defSize = slot._widget()->_defaultSize(scale);

			spx wantedHeight = align(spx(int64_t(defSize.h * givenWidth) / defSize.w));
			return wantedHeight + border.h;
		}
		else
			return Capsule::_matchingHeight(width,scale);
	}

	//____ _matchingWidth() ______________________________________________________

	spx	CanvasCapsule::_matchingWidth(spx height, int scale) const
	{
		if( m_bScaleCanvas && !slot.isEmpty() )
		{
			SizeSPX border = m_skin.contentBorderSize(scale);

			spx givenHeight = height - border.h;
			SizeSPX defSize = slot._widget()->_defaultSize(scale);

			spx wantedWidth = align( spx(int64_t(defSize.w * givenHeight) / defSize.h));
			return wantedWidth + border.w;
		}
		else
			return Capsule::_matchingWidth(height,scale);
	}

	//____ _resizeCanvasAndChild() _______________________________________________

	void CanvasCapsule::_resizeCanvasAndChild()
	{
		SizeSPX newSize;
		if( m_bScaleCanvas )
		{
			if( slot._widget() )
				newSize = slot._widget()->_defaultSize(m_scale);
			else
				newSize.clear();
		}
		else
			newSize = _contentRect();

		if( newSize != m_canvasSize )
		{
			_setCanvasSize( newSize );
			if( slot._widget() )
				slot._widget()->_resize(newSize, m_scale);
		}
	}

	//____ _renderCanvas() ____________________________________________________

	Surface* CanvasCapsule::_renderCanvas(GfxDevice* pDevice)
	{
		// Possibly regenerate the canvas

		if (!m_pCanvas)
		{
			SurfaceFactory* pFactory = m_pFactory ? m_pFactory : Base::defaultSurfaceFactory();
			if (!pFactory)
			{
				//TODO: Error handling!
				return nullptr;
			}

			SizeI pixelSize = m_canvasSize / 64;
			m_pCanvas = pFactory->createSurface(WGBP(Surface, _.size = pixelSize, _.format = m_canvasFormat,
				_.canvas = true, _.scale = m_scale));
			m_patches.clear();
			m_patches.add(m_canvasSize);
		}

		// Render children into canvas surface

		if (!m_patches.isEmpty())
		{

			pDevice->beginCanvasUpdate(m_pCanvas, m_patches.size(), m_patches.begin(), m_pCanvasLayers, m_renderLayer);

			// Possibly clear the canvas before rendering

			HiColor clearColor = m_clearColor;

			if( clearColor == HiColor::Undefined )
			{
				auto canvasDesc = Util::pixelFormatToDescription(m_canvasFormat);
				if( canvasDesc.A_mask != 0 )
					clearColor = HiColor::Transparent;
			}

			if( clearColor != HiColor::Undefined )
			{
				pDevice->setBlendMode(BlendMode::Replace);
				pDevice->fill( clearColor );
				pDevice->setBlendMode(BlendMode::Blend);
			}

			//

			RectSPX canvas = { 0,0, m_canvasSize };
			slot._widget()->_render(pDevice, canvas, canvas);

			pDevice->endCanvasUpdate();

			m_patches.clear();
		}

		return m_pCanvas;
	}

	//____ _update() __________________________________________________________

	void CanvasCapsule::_update(int microPassed, int64_t microsecTimestamp)
	{
		glow._update(microPassed);

		if (m_pTintColorTransition)
		{
			int timestamp = m_tintColorTransitionProgress + microPassed;

			if (timestamp >= m_pTintColorTransition->duration())
			{
				m_tintColorTransitionProgress = 0;
				m_pTintColorTransition = nullptr;

				if (m_tintColor != m_endTintColor)
				{
					m_tintColor = m_endTintColor;
					_requestRender();
				}

				_stopReceiveUpdates();
			}
			else
			{
				m_tintColorTransitionProgress = timestamp;
				HiColor color = m_pTintColorTransition->snapshot(timestamp, m_startTintColor, m_endTintColor);

				if (color != m_tintColor)
				{
					m_tintColor = color;
					_requestRender();
				}
			}
		}

		if (m_pTintmapTransition)
		{
			int timestamp = m_tintmapTransitionProgress + microPassed;

			if (timestamp >= m_pTintmapTransition->duration())
			{
				m_tintmapTransitionProgress = 0;
				m_pTintmapTransition = nullptr;

				if (m_pTintmap != m_pEndTintmap)
				{
					m_pTintmap = m_pEndTintmap;
					_requestRender();
				}

				_stopReceiveUpdates();
			}
			else
			{
				m_tintmapTransitionProgress = timestamp;

				m_pTintmap = SnapshotTintmap::create( m_pStartTintmap, m_pEndTintmap, m_pTintmapTransition, timestamp );
				_requestRender();
			}
		}
	}

	//____ _render() _____________________________________________________________

	void CanvasCapsule::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		// Render our canvas content

		_renderCanvas(pDevice);

		//

		RectSPX contentRect = _contentRect(_canvas);
		RectSPX canvasArea = m_bScaleCanvas ? _canvasWindow(contentRect) : contentRect;
		RectSPX glowRect = m_bSkinAroundCanvas ? canvasArea : contentRect;

		if( m_bSkinAroundCanvas )
			m_skin.render(pDevice, canvasArea + m_skin.contentBorder(m_scale, m_state), m_scale, m_state);
		else
			m_skin.render(pDevice, _canvas, m_scale, m_state);


		// Blit our canvas

		int		rl = pDevice->renderLayer();
		BlendMode bm = pDevice->blendMode();
		HiColor c = pDevice->tintColor();

		if (m_renderLayer != -1)
			pDevice->setRenderLayer(m_renderLayer);


		glow._render(pDevice, glowRect, m_pCanvas, canvasArea);

		pDevice->setBlendMode(m_blendMode);

		pDevice->setTintColor(m_tintColor);

		if (m_pTintmap)
			pDevice->setTintmap(canvasArea, m_pTintmap);

		pDevice->setBlitSource(m_pCanvas);
		pDevice->stretchBlit(canvasArea);
/*
		// Debug code

		Skin_p pOutline1 = BoxSkin::create( WGBP(BoxSkin, _.outlineColor = Color::Green, _.outlineThickness = 1.f, _.color = Color::Transparent ));

		pOutline1->_render(pDevice, _canvas, m_scale, m_state );

		Skin_p pOutline2 = BoxSkin::create( WGBP(BoxSkin, _.outlineColor = Color::Red, _.outlineThickness = 1.f, _.color = Color::Transparent ));

		pOutline2->_render(pDevice, seedArea, m_scale, m_state );
*/
		if (m_pTintmap)
			pDevice->clearTintmap();

		pDevice->setTintColor(c);
		pDevice->setBlendMode(bm);
		pDevice->setRenderLayer(rl);
	}

	//____ _resize() _____________________________________________________________

	void CanvasCapsule::_resize(const SizeSPX& size, int scale)
	{
		if( !m_bScaleCanvas )
		{
			_setCanvasSize( _contentRect(size) );
			Capsule::_resize(size, scale);
		}
		else
		{
			if( scale != m_scale && slot._widget())
			{
				auto sz = slot._widget()->_defaultSize(scale);
				slot._widget()->_resize(sz, scale);
				_setCanvasSize(sz);
			}
			m_size = size;
			m_scale = scale;
		}

		glow._setSize(size,scale);
	}

	//____ _maskPatches() ________________________________________________________

	void CanvasCapsule::_maskPatches( PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip )
	{
		RectSPX skinRect = geo;

		RectSPX contentRect = _contentRect(geo);
		RectSPX canvasArea = m_bScaleCanvas ? _canvasWindow(contentRect) : contentRect;

		if( m_bScaleCanvas && m_bSkinAroundCanvas )
			skinRect = canvasArea + m_skin.contentBorder(m_scale, m_state);

		RectSPX coverage = m_skin.coverage(skinRect, m_scale, m_state);

		patches.sub( RectSPX::overlap(coverage,clip) );

		if( coverage.contains(_contentRect(geo)) )
			return;										// No need to loop through children, skins coverage contains them all.


		// We can't mask against canvas content if canvas is applied with some transparency.

		if( m_tintColor.a != 4096 || (m_pTintmap && !m_pTintmap->isOpaque()) )
			return;

		//

		if( slot._widget() )
		{
			if( Util::pixelFormatToDescription(m_canvasFormat).A_mask == 0 )
				patches.sub(RectSPX::overlap(canvasArea, clip));
			else
				slot._widget()->_maskPatches(patches, canvasArea, clip);
		}
	}



	//____ _releaseChild() _______________________________________________________

	void CanvasCapsule::_releaseChild( StaticSlot * pSlot )
	{
		Capsule::_releaseChild(pSlot);
		_setCanvasSize( { 0,0 } );
	}

	//____ _replaceChild() _______________________________________________________

	void CanvasCapsule::_replaceChild(StaticSlot * pSlot, Widget * pWidget)
	{
		Capsule::_replaceChild(pSlot, pWidget);
		_resizeCanvasAndChild();
	}

	//____ _root() ___________________________________________________________________

	Root * CanvasCapsule::_root()
	{
		Root * pRoot = Capsule::_root();

		if( !pRoot )
		{
			// We don't have any root, but we might have one through a CanvasDisplay.

			for( auto pSideDisplay : m_sideDisplays )
			{
				Container * pParent = pSideDisplay->_parent();
				if( pParent )
				{
					pRoot = pParent->_root();
					if( pRoot )
						return pRoot;
				}
			}
		}

		return pRoot;
	}
	//____ _findWidget() ____________________________________________________________

	Widget * CanvasCapsule::_findWidget( const CoordSPX& ofs, SearchMode mode )
	{
		if( m_bScaleCanvas )
		{
			if( !slot.isEmpty() )
			{
				RectSPX content = _contentRect();
				RectSPX geo = _canvasWindow(_contentRect());
				if( geo.contains(ofs) )
				{
					Widget * pWidget = slot._widget();

					CoordSPX ofsInCanvas = (ofs - geo.pos()) * (m_canvasSize.w / (float) geo.w);

					if (pWidget->isContainer())
					{
						Widget * pRes = static_cast<Container*>(pWidget)->_findWidget(ofsInCanvas, mode);
						if (pRes)
							return pRes;
					}
					else if( mode == SearchMode::Geometry || slot._widget()->_markTest( ofsInCanvas ) )
						return pWidget;
				}
			}

			// Check against ourselves

			if( mode == SearchMode::Geometry || _markTest(ofs) )
				return this;

			return nullptr;
		}
		else
			return Capsule::_findWidget(ofs, mode);
	}

	void CanvasCapsule::_childRequestRender(StaticSlot* pSlot, const RectSPX& _rect)
	{
//		assert( RectSPX(m_canvasSize).contains(_rect));

		auto rect = RectSPX::overlap(_rect,RectSPX(m_canvasSize));

		m_patches.add(rect);

		RectSPX dirt;

		RectSPX contentRect = _contentRect();

		if( m_bScaleCanvas )
		{
			float scaleFactor = std::min({contentRect.w / (float)m_canvasSize.w, contentRect.h / (float)m_canvasSize.h});

			dirt = alignUp(rect*scaleFactor);

			dirt += _canvasWindow(contentRect).pos();
		}
		else
			dirt = rect + contentRect.pos();

		_requestRender(dirt);

		for (auto pDisplay : m_sideDisplays)
			pDisplay->_canvasDirtyRect(rect);

	}

	//____ _childRequestResize() _________________________________________________

	void CanvasCapsule::_childRequestResize( StaticSlot * pSlot )
	{
		if( m_bScaleCanvas )
		{
			auto def = pSlot->widget()->_defaultSize(m_scale);
			if( def != m_canvasSize )
			{
				_setCanvasSize( def );
				pSlot->widget()->_resize(def, m_scale);
				_requestRender();
			}
		}
		else
			Capsule::_requestResize();
	}

	//____ _childRequestInView() _________________________________________________

	void CanvasCapsule::_childRequestInView( StaticSlot * pSlot )
	{
		if( m_pHolder )
		{
			RectSPX area = _slotGeo( pSlot );
			m_pHolder->_childRequestInView( m_pSlot, area, area );
		}
	}

	void CanvasCapsule::_childRequestInView( StaticSlot * pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea )
	{
		if( m_pHolder )
		{
			if( m_bScaleCanvas )
			{
				RectSPX window = _canvasWindow(_contentRect(m_size));
				float scaleFactor = window.w / (float) m_canvasSize.w;

				m_pHolder->_childRequestInView( m_pSlot, Util::alignUp(mustHaveArea*scaleFactor) + window.pos(), Util::alignUp(niceToHaveArea*scaleFactor) + window.pos() );
			}
			else
			{
				CoordSPX pos( _slotGeo( pSlot ).pos() );
				m_pHolder->_childRequestInView( m_pSlot, mustHaveArea + pos, niceToHaveArea + pos );
			}
		}
	}


	//____ _slotGeo() ___________________________________________________________

	RectSPX CanvasCapsule::_slotGeo(const StaticSlot* pSlot) const
	{
		if (m_bScaleCanvas)
		{
			RectSPX contentRect = _contentRect(m_size);
			return _canvasWindow(contentRect);
		}
		else
			return m_skin.contentRect(m_size, m_scale, m_state);
	}

	//____ _childLocalToGlobal() _______________________________________________

	RectSPX CanvasCapsule::_childLocalToGlobal(const StaticSlot* pSlot, const RectSPX& rect) const
	{
		if (m_bScaleCanvas)
		{
			RectSPX window = _canvasWindow(_contentRect(m_size));

			float scaleFactor = window.w / (float) m_canvasSize.w;

			auto global = _toGlobal((rect * scaleFactor) + window.pos());
			return global;
		}
		else
			return _toGlobal(rect + _slotGeo(pSlot).pos());

	}

	//____ _globalToChildLocal() ________________________________________________

	RectSPX CanvasCapsule::_globalToChildLocal(const StaticSlot* pSlot, const RectSPX& rect) const
	{
		if (m_bScaleCanvas)
		{
			RectSPX window = _canvasWindow(_contentRect(m_size));

			float scaleFactor = (float)m_canvasSize.w / window.w;

			auto local = (_toLocal(rect) - window.pos()) * scaleFactor;
			return local;
		}
		else
			return _toLocal(rect - _slotGeo(pSlot).pos());
	}

	//____ _globalPtsToChildLocalSpx() ___________________________________________

	RectSPX CanvasCapsule::_globalPtsToChildLocalSpx(const StaticSlot* pSlot, const Rect& rect) const
	{
		RectSPX rectSPX = m_pHolder ? m_pHolder->_globalPtsToChildLocalSpx(m_pSlot, rect) : Util::align(Util::ptsToSpx(rect, m_scale));

		if (m_bScaleCanvas)
		{
			RectSPX window = _canvasWindow(_contentRect(m_size));

			float scaleFactor = (float)m_canvasSize.w / window.w;

			auto local = (rectSPX - window.pos()) * scaleFactor;
			return local;
		}
		else
		{
			return rectSPX - _slotGeo(pSlot).pos();
		}
	}

	//____ _childLocalSpxToGlobalPts() ___________________________________________

	Rect CanvasCapsule::_childLocalSpxToGlobalPts(const StaticSlot* pSlot, const RectSPX& _rect) const
	{
		RectSPX rect;

		if (m_bScaleCanvas)
		{
			RectSPX window = _canvasWindow(_contentRect(m_size));

			float scaleFactor = (float)m_canvasSize.w / window.w;

			rect = (_rect + window.pos()) * scaleFactor;
		}
		else
			rect = _rect + _slotGeo(pSlot).pos();

		if( m_pHolder )
			return m_pHolder->_childLocalSpxToGlobalPts( m_pSlot, rect );
		else
			return Util::spxToPts(rect, m_scale);
	}

	//____ _canvasWindow() _______________________________________________________

	RectSPX CanvasCapsule::_canvasWindow( RectSPX window ) const
	{
		float scaleFactor = std::min( { window.w / (float) m_canvasSize.w, window.h / (float) m_canvasSize.h } );

		SizeSPX canvasInWindowSize = align(SizeSPX( m_canvasSize * scaleFactor ));

		return Util::placementToRect(m_placement, window.size(), canvasInWindowSize) + window.pos();
	}

	//____ _addSideDisplay() __________________________________________________

	void CanvasCapsule::_addSideDisplay(CanvasDisplay* pSideDisplay)
	{
		m_sideDisplays.push_back(pSideDisplay);
	}

	//____ _removeSideDisplay() _______________________________________________

	void CanvasCapsule::_removeSideDisplay(CanvasDisplay* pSideDisplay)
	{
		m_sideDisplays.erase(std::remove(m_sideDisplays.begin(), m_sideDisplays.end(), pSideDisplay));
	}

	//____ _setCanvasSize() ______________________________________________

	void CanvasCapsule::_setCanvasSize(SizeSPX canvasSize)
	{
		m_canvasSize = canvasSize;
		m_pCanvas = nullptr;

		for (auto pDisplay : m_sideDisplays)
			pDisplay->_canvasReset(canvasSize);
	}
}
