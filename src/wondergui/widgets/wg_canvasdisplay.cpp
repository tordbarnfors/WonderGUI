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

#include <wg_canvasdisplay.h>
#include <wg_canvascapsule.h>
#include <wg_gfxdevice.h>
#include <wg_snapshottintmap.h>

namespace wg
{
	using namespace Util;

	const TypeInfo CanvasDisplay::TYPEINFO = { "CanvasDisplay", &Widget::TYPEINFO };

	//____ constructor ____________________________________________________________

	CanvasDisplay::CanvasDisplay()
	{
		m_defaultSize = Size(-1,-1);
	}

	//____ destructor _____________________________________________________________

	CanvasDisplay::~CanvasDisplay()
	{
		if (m_pCanvas)
			m_pCanvas->_removeSideDisplay(this);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& CanvasDisplay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setCanvas() ________________________________________________________

	void CanvasDisplay::setCanvas(CanvasCapsule* pCanvas)
	{
		if (pCanvas == m_pCanvas)
			return;

		if (m_pCanvas)
		{
			m_pCanvas->_removeSideDisplay(this);
		}

		m_pCanvas = pCanvas;

		SizeSPX newSize;
		if (pCanvas)
		{
			pCanvas->_addSideDisplay(this);
			newSize = pCanvas->_canvasSize();
		}

		_requestRender();

		if (newSize != m_canvasSize)
		{
			m_canvasSize = newSize;
			_requestResize();
		}
	}

	//____ setPlacement() _____________________________________________________

	void CanvasDisplay::setPlacement(Placement placement)
	{
		if (placement != m_placement)
		{
			m_placement = placement;
			_requestRender();
		}
	}

	//____ setDefaultSize() _______________________________________________________

	void CanvasDisplay::setDefaultSize( const Size& size )
	{
		if( size != m_defaultSize )
		{
			m_defaultSize = size;
			_requestResize();
		}
	}

	//____ setTintColor() ______________________________________________________

	void CanvasDisplay::setTintColor(HiColor color, ColorTransition* pTransition)
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

	//____ setTintmap() _______________________________________________

	void CanvasDisplay::setTintmap(Tintmap * pTintmap, ColorTransition* pTransition)
	{
		if (pTintmap != m_pTintmap)
		{
			if (pTransition)
			{
				if (!m_pTintmapTransition)
					_startReceiveUpdates();

				m_pTintmapTransition = pTransition;
				m_tintmapTransitionProgress = 0;
				m_pStartTintmap = m_pTintmap;
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

	void CanvasDisplay::setBlendMode(BlendMode mode)
	{
		if (mode != m_blendMode)
		{
			m_blendMode = mode;
			_requestRender();
		}
	}

	//____ defaultSize() __________________________________________________________

	SizeSPX CanvasDisplay::_defaultSize(int scale) const
	{
		if (m_defaultSize.w >= 0 && m_defaultSize.h >= 0)
		{
			return SizeSPX(m_defaultSize*scale);
		}
		else if (m_pCanvas)
		{
			SizeSPX size = alignUp(m_pCanvas->_canvasSize() / 64 * scale);
			size += m_skin.contentBorderSize(scale);
			return size;
		}
		else
			return Widget::_defaultSize(scale);
	}

	//____ _matchingHeight() ___________________________________________________

	spx CanvasDisplay::_matchingHeight(spx width, int scale) const
	{
		if (m_defaultSize.w >= 0 && m_defaultSize.h >= 0)
		{
			float scaleFactor = width / (float) m_defaultSize.w;

			return alignUp(m_defaultSize.h * scaleFactor);
		}
		else if (m_pCanvas)
		{
			SizeSPX borderSize = m_skin.contentBorderSize(scale);
			width -= borderSize.w;

			SizeSPX canvasSize = m_pCanvas->_canvasSize();

			float scaleFactor = width / (float)canvasSize.w;
			if( scaleFactor < 1.f )
				canvasSize.h = alignUp(canvasSize.h * scaleFactor);

			return canvasSize.h + borderSize.h;
		}
		else
			return Widget::_matchingHeight(width, scale);
	}

	//____ _matchingWidth() ____________________________________________________

	spx CanvasDisplay::_matchingWidth(spx height, int scale) const
	{
		if (m_defaultSize.w >= 0 && m_defaultSize.h >= 0)
		{
			float scaleFactor = height / (float)m_defaultSize.h;

			return alignUp(m_defaultSize.w * scaleFactor);
		}
		else if (m_pCanvas)
		{
			SizeSPX borderSize = m_skin.contentBorderSize(scale);
			height -= borderSize.h;

			SizeSPX canvasSize = m_pCanvas->_canvasSize();

			float scaleFactor = height / (float)canvasSize.h;
			if( scaleFactor < 1.f )
				canvasSize.w = alignUp(canvasSize.w * scaleFactor);

			return canvasSize.w + borderSize.w;
		}
		else
			return Widget::_matchingWidth(height, scale);
	}

	//____ _update() __________________________________________________________

	void CanvasDisplay::_update(int microPassed, int64_t microsecTimestamp)
	{
		if (m_pTintColorTransition)
		{
			int timestamp = m_tintColorTransitionProgress + microPassed;

			if (timestamp >= m_pTintColorTransition->duration())
			{
				m_tintColorTransitionProgress = 0;
				m_pTintColorTransition = nullptr;
				m_tintColor = m_endTintColor;

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

				m_pTintmap = m_pEndTintmap;
				m_pEndTintmap = nullptr;
				m_pStartTintmap = nullptr;
				_requestRender();

				_stopReceiveUpdates();
			}
			else
			{
				m_tintmapTransitionProgress = timestamp;

				m_pTintmap = SnapshotTintmap::create(m_pStartTintmap, m_pEndTintmap, m_pTintmapTransition, timestamp);
				_requestRender();
			}
		}
	}


	//____ render() ___________________________________________________________

	void CanvasDisplay::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		if (m_bSkinAroundCanvas && m_pCanvas )
			m_skin.render(pDevice, _canvasWindow(_contentRect(_canvas)) + m_skin.contentBorder(m_scale, m_state), m_scale, m_state);
		else
			m_skin.render(pDevice, _canvas, m_scale, m_state);

		if (m_pCanvas)
		{
			auto dest = _contentRect(_canvas);

			if ( dest.w > 0 && dest.h > 0)
			{
				auto pSurface = m_pCanvas->_renderCanvas(pDevice);

				auto canvasArea = _canvasWindow(dest);
				
				// Blit our canvas

//				int		rl = pDevice->renderLayer();
				BlendMode bm = pDevice->blendMode();
				HiColor c = pDevice->tintColor();

//				if (m_renderLayer != -1)
//					pDevice->setRenderLayer(m_renderLayer);

				pDevice->setBlendMode(m_blendMode);

				pDevice->setTintColor(m_tintColor);

				if (m_pTintmap )
					pDevice->setTintmap(canvasArea, m_pTintmap);

				pDevice->setBlitSource(pSurface);
				pDevice->stretchBlit(canvasArea);

				if (m_pTintmap)
					pDevice->clearTintmap();

				pDevice->setTintColor(c);
				pDevice->setBlendMode(bm);
//				pDevice->setRenderLayer(rl);
			}
		}
	}

	//____ _canvasReset() _____________________________________________________

	void CanvasDisplay::_canvasReset(SizeSPX canvasSize)
	{
		_requestRender();
		if (canvasSize != m_canvasSize)
		{
			m_canvasSize = canvasSize;
			if( m_defaultSize.w < 0 || m_defaultSize.h < 0 )
				_requestResize();
		}
	}

	//____ _canvasDirtyRect() _________________________________________________

	void CanvasDisplay::_canvasDirtyRect(const RectSPX rect)
	{
		RectSPX contentRect = _contentRect();
		RectSPX dirt;

		float scaleFactor = std::min({ 1.f, contentRect.w / (float)m_canvasSize.w, contentRect.h / (float)m_canvasSize.h });
		dirt = alignUp(rect * scaleFactor);
		dirt += _canvasWindow(contentRect).pos();

		_requestRender(dirt);
	}

	//____ _canvasWindow() _______________________________________________________

	RectSPX CanvasDisplay::_canvasWindow(RectSPX window) const
	{
		float scaleFactor = std::min({ 1.f, window.w / (float)m_canvasSize.w, window.h / (float)m_canvasSize.h });

		SizeSPX canvasInWindowSize = align(SizeSPX(m_canvasSize * scaleFactor));

		return Util::placementToRect(m_placement, window.size(), canvasInWindowSize) + window.pos();
	}

	//____ _alphaTest() ________________________________________________________

	bool CanvasDisplay::_alphaTest(const CoordSPX& ofs)
	{
		RectSPX canvasGeo = _canvasWindow(_contentRect(m_size));

		if (canvasGeo.contains(ofs) && m_pCanvas)
		{
			float scale = m_canvasSize.w / float(canvasGeo.w);

			CoordSPX surfOfs = (ofs - canvasGeo.pos()) * scale;

			auto pSurface = m_pCanvas->_canvasSurface();
			if (pSurface)
			{
				if (pSurface->alpha(surfOfs) > 0)		// Any alpha gives a hit. We could add a variable for this if needed.
					return true;
			}
		}

		if (m_bSkinAroundCanvas)
		{
			return m_skin.markTest(ofs, canvasGeo + m_skin.contentBorder(m_scale, m_state), m_scale, m_state);
		}
		else
			return m_skin.markTest(ofs, RectSPX(m_size), m_scale, m_state);
	}



} // namespace wg
