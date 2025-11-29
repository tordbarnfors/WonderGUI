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

	//____ setFlip() _____________________________________________________________

	void ScrollChart::setFlip( GfxFlip flip )
	{
		m_flip = flip;
		m_bAxisSwapped = ( flip == GfxFlip::Rot90 || flip == GfxFlip::Rot90FlipX || flip == GfxFlip::Rot90FlipY ||
						  flip == GfxFlip::Rot270 || flip == GfxFlip::Rot270FlipX || flip == GfxFlip::Rot270FlipY );

		_fullRefreshOfChart();
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
            _requestFullRedraw();
            _requestRender();
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

		if( m_bAxisSwapped )
			std::swap(wantedSize.w, wantedSize.h);

        if (m_pScrollSurface->pixelSize() * 64 != wantedSize)
            m_pScrollSurface = nullptr;
        else
            m_bFullRedrawRequested = true;
    }

    //____ _renderCharts() ____________________________________________________

    void ScrollChart::_renderCharts(GfxDevice* pDevice, const RectSPX& canvas)
    {
		SizeSPX surfaceSize = canvas.size();
		if( m_bAxisSwapped )
			std::swap(surfaceSize.w,surfaceSize.h);

		// Make sure we have a canvas

        if (m_pScrollSurface == nullptr)
        {
            m_pScrollSurface = pDevice->surfaceFactory()->createSurface(WGBP(Surface, _.size = surfaceSize / 64, _.format = m_scrollSurfaceFormat, _.canvas = true));
			m_rightEdgeOfs = surfaceSize.w;
			m_dirtLen = surfaceSize.w;
        }

        // Render the charts

        if (m_dirtLen > 0)
        {
            int         nDirtyRects;
            RectSPX     dirtyRects[2];

            int overflow = m_dirtLen - m_rightEdgeOfs;
            if (overflow > 0)
            {
                dirtyRects[0] = { 0, 0, m_rightEdgeOfs, surfaceSize.h };
                dirtyRects[1] = { surfaceSize.w - overflow, 0, overflow, surfaceSize.h };
                nDirtyRects = 2;
            }
            else
            {
                dirtyRects[0] = { m_rightEdgeOfs - m_dirtLen, 0, m_dirtLen, surfaceSize.h };
                nDirtyRects = 1;
            }

            pDevice->beginCanvasUpdate(m_pScrollSurface, nDirtyRects, dirtyRects);

            pDevice->setBlendMode(BlendMode::Replace);
            pDevice->fill(m_scrollSurfaceBgColor);
            pDevice->setBlendMode(BlendMode::Blend);

            _renderOnScrollSurface(pDevice, surfaceSize, m_rightEdgeOfs, m_rightEdgeTimestamp, m_dirtLen);

            pDevice->endCanvasUpdate();

            m_dirtLen = 0;
        }

        // Copy scroll surface to our canvas

        pDevice->setBlitSource(m_pScrollSurface);


		switch( m_flip )
		{
			case GfxFlip::None:
			case GfxFlip::FlipY:
			case GfxFlip::Rot180FlipX:
				pDevice->flipBlit( {canvas.x, canvas.y}, { m_rightEdgeOfs, 0, surfaceSize.w - m_rightEdgeOfs, surfaceSize.h }, m_flip );
				pDevice->flipBlit({ canvas.x + canvas.w - m_rightEdgeOfs,canvas.y }, { 0,0,m_rightEdgeOfs,surfaceSize.h }, m_flip );
				break;

			case GfxFlip::FlipX:
			case GfxFlip::Rot180:
			case GfxFlip::Rot180FlipY:
				pDevice->flipBlit({canvas.x, canvas.y}, { 0,0,m_rightEdgeOfs,surfaceSize.h }, m_flip );
				pDevice->flipBlit( { canvas.x + m_rightEdgeOfs,canvas.y }, { m_rightEdgeOfs, 0, surfaceSize.w - m_rightEdgeOfs, surfaceSize.h }, m_flip );
				break;

			case GfxFlip::Rot90:
			case GfxFlip::Rot90FlipX:
			case GfxFlip::Rot270FlipY:
				pDevice->flipBlit( {canvas.x, canvas.y}, { m_rightEdgeOfs, 0, surfaceSize.w - m_rightEdgeOfs, surfaceSize.h }, m_flip );
				pDevice->flipBlit({ canvas.x, canvas.y + canvas.h - m_rightEdgeOfs }, { 0, 0, m_rightEdgeOfs, surfaceSize.h }, m_flip );
				break;


			case GfxFlip::Rot90FlipY:
			case GfxFlip::Rot270:
			case GfxFlip::Rot270FlipX:
				pDevice->flipBlit( {canvas.x, canvas.y}, { 0, 0, m_rightEdgeOfs, surfaceSize.h }, m_flip );
				pDevice->flipBlit({ canvas.x, canvas.y + m_rightEdgeOfs }, { m_rightEdgeOfs, 0, surfaceSize.w - m_rightEdgeOfs, surfaceSize.h }, m_flip );
				break;
		}
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
				m_dirtLen = _canvasSize().w;
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
		if( m_bAxisSwapped )
			std::swap(contentSize.w,contentSize.h);

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
