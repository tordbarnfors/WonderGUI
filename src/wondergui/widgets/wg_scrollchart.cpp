/*=========================================================================

                         >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI ScrollChartEntryics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

                            -----------

  The WonderGUI ScrollChartEntryics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                            -----------

  The WonderGUI ScrollChartEntryics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_scrollchart.h>
#include <wg_base.h>
#include <wg_gfxdevice.h>

#include <cstring>

namespace wg
{

    const TypeInfo ScrollChart::TYPEINFO = { "ScrollChart", &Widget::TYPEINFO };

    //____ constructor ____________________________________________________________

    ScrollChart::ScrollChart()
    {
    }

    //____ destructor _____________________________________________________________

    ScrollChart::~ScrollChart()
    {
    }

    //____ typeInfo() _________________________________________________________

    const TypeInfo& ScrollChart::typeInfo(void) const
    {
        return TYPEINFO;
    }

    //____ start() ____________________________________________________________

    void ScrollChart::start()
    {
        if (!m_bScrolling)
        {
            m_bScrolling = true;
            m_latestTimestamp = _startReceiveUpdates();
			
			_removeOutdatedSamples();
        }
    }

    //____ stop() _____________________________________________________________

    void ScrollChart::stop()
    {
        if (m_bScrolling)
        {
            m_bScrolling = false;
            _stopReceiveUpdates();
        }
    }

	//____ setDisplayTime() ____________________________________________________

	void ScrollChart::setDisplayTime(int microsec)
	{
		limit(microsec, 100000, m_maxDisplayTime);

		if (m_displayTime != microsec)
		{
			m_displayTime = microsec;
			m_bFullRedrawRequested = true;
		}
	}

    //____ _fullRefreshOfChart() _______________________________________

    void ScrollChart::_fullRefreshOfChart()
    {
        if (!m_pScrollSurface)
            return;

        SizeSPX wantedSize = m_size - _contentBorderSize(m_scale);

        if (m_pScrollSurface->pixelSize() * 64 != wantedSize)
            m_pScrollSurface = nullptr;
        else
            m_bFullRedrawRequested = true;
    }

    //____ _renderCharts() ____________________________________________________

    void ScrollChart::_renderCharts(GfxDevice* pDevice, const RectSPX& canvas)
    {
        // Make sure we have a canvas

        if (m_pScrollSurface == nullptr)
        {
            m_pScrollSurface = pDevice->surfaceFactory()->createSurface(WGBP(Surface, _.size = canvas.size() / 64, _.format = m_scrollSurfaceFormat, _.canvas = true));
			m_rightEdgeOfs = canvas.w;
			m_dirtLen = canvas.w;
        }

        // Render the charts

        if (m_dirtLen > 0)
        {
            int         nDirtyRects;
            RectSPX     dirtyRects[2];

            int overflow = m_dirtLen - m_rightEdgeOfs;
            if (overflow > 0)
            {
                dirtyRects[0] = { 0, 0, m_rightEdgeOfs, canvas.h };
                dirtyRects[1] = { canvas.w - overflow, 0, overflow, canvas.h };
                nDirtyRects = 2;
            }
            else
            {
                dirtyRects[0] = { m_rightEdgeOfs - m_dirtLen, 0, m_dirtLen, canvas.h };
                nDirtyRects = 1;
            }

            pDevice->beginCanvasUpdate(m_pScrollSurface, nDirtyRects, dirtyRects);

            pDevice->setBlendMode(BlendMode::Replace);
            pDevice->fill(m_scrollSurfaceBgColor);
            pDevice->setBlendMode(BlendMode::Blend);

            _renderOnScrollSurface(pDevice, canvas.size(), m_rightEdgeOfs, m_rightEdgeTimestamp, m_dirtLen);

            pDevice->endCanvasUpdate();

            m_dirtLen = 0;
        }

        // Copy scroll surface to our canvas

        pDevice->setBlitSource(m_pScrollSurface);
        pDevice->blit({ canvas.x, canvas.y }, { m_rightEdgeOfs, 0, canvas.w - m_rightEdgeOfs, canvas.h });
        pDevice->blit({ canvas.x + canvas.w - m_rightEdgeOfs,canvas.y }, { 0,0,m_rightEdgeOfs,canvas.h });
    }

    //____ _update() ____________________________________________________________

    void ScrollChart::_update(int microPassed, int64_t microsecTimestamp)
    {
        Chart::_update(microPassed, microsecTimestamp);

        if (!m_bScrolling)
		{
			if( m_bFullRedrawRequested )
			{
				_requestRenderChartArea();
				m_dirtLen = m_chartCanvas.w;
				m_bFullRedrawRequested = false;
			}
			return;
		}
		
        m_latestTimestamp = microsecTimestamp;
      
        if( !m_bPreRenderRequested )
            m_bPreRenderRequested = _requestPreRenderCall();

    }

    //____ _preRender() _______________________________________________________

    void ScrollChart::_preRender()
    {
		m_bPreRenderRequested = false;

        SizeSPX contentSize = m_chartCanvas;

        int64_t timestamp = m_latestTimestamp - m_latency;

        if (!m_pScrollSurface || m_bFullRedrawRequested)
        {
            m_rightEdgeOfs = contentSize.w;
            m_rightEdgeTimestamp = timestamp;
            m_dirtLen = contentSize.w;

            _requestRenderChartArea();
            m_bFullRedrawRequested = false;
            return;
        }

        // Calc new m_leftEdgeTimestamp, m_leftEdgeOfs, m_dirtOfs, m_dirtLen
        // Calc scrollSurface dirty rects.

        int microsecPerPixel = m_displayTime / (contentSize.w / 64);

        int timePassed = int(timestamp - m_rightEdgeTimestamp);

        if (timePassed > m_displayTime)
        {
            m_rightEdgeOfs = contentSize.w;
            m_rightEdgeTimestamp = timestamp;
            m_dirtLen = contentSize.w;
            _requestRenderChartArea();
        }
        else
        {
            int pixelsToScroll = (timePassed / microsecPerPixel);

            if (pixelsToScroll > 0)
            {
                m_rightEdgeOfs = (m_rightEdgeOfs + pixelsToScroll*64) % contentSize.w;
                m_rightEdgeTimestamp += pixelsToScroll * microsecPerPixel;
                m_dirtLen = pixelsToScroll * 64;
                _requestRenderChartArea();
            }
        }

    }

    //____ _requestFullRedraw() _______________________________________________

    void ScrollChart::_requestFullRedraw()
    {
        m_bFullRedrawRequested = true;
    }


}
