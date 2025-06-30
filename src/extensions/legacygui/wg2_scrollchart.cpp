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


#include <wg2_scrollchart.h>
#include <wg_surfacefactory.h>
#include <wg2_gfxdevice.h>
#include <wg2_pen.h>
#include <wg2_base.h>
#include <wg_texttool.h>
#include <wg2_util.h>
#include <wg2_eventhandler.h>

#include <wg_gradient.h>

#include <algorithm>
#include <limits>


#include <assert.h>

static const char	c_widgetType[] = { "ScrollChart" };

//____ Constructor ____________________________________________________________

WgScrollChart::WgScrollChart()
{
	m_defaultSize = WgSize(200, 100);
	m_bOpaque = false;
	m_mode = WG_MODE_NORMAL;

	m_bDynamicValueRange = true;

	m_setTopValue = 1.f;
	m_setBottomValue = -1.f;

	m_topValue = 1.f;
	m_bottomValue = -1.f;

	m_pCanvas = nullptr;
	m_pFactory = nullptr;

	m_bScrollFromStart = true;
	m_bStarted = false;
	m_bPaused = false;

	m_sampleTTL = 0;
	m_waveIdCounter = 1;

	m_windowBegin = 0;
	m_windowEnd = 0;

	m_scrollAmount = 0;
	m_scrollFraction = 0.f;

	m_sampleBeginTimestamp = 0;
	m_sampleEndTimestamp = 0;

	m_sampleLabelStyle.alignment = WgOrigo::South;
	m_sampleLabelStyle.offset = { 0,0 };
    m_sampleLabelStyle.bLabelAtEnd = false;

	m_valueLabelStyle.alignment = WgOrigo::West;
	m_valueLabelStyle.offset = { 0,0 };
    m_valueLabelStyle.bLabelAtEnd = false;

	m_chartColor = WgColor::White;
}

//____ Destructor _____________________________________________________________

WgScrollChart::~WgScrollChart()
{
	_stopReceiveTicks();
}

//____ Type() _________________________________________________________________

const char * WgScrollChart::Type(void) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgScrollChart::GetClass()
{
	return c_widgetType;
}

//____ SetPreferredChartSize() _______________________________________________________

void WgScrollChart::SetPreferredChartSize(const WgSize& size)
{
	if (size != m_defaultSize)
	{
		m_defaultSize = size;
		_requestResize();
	}
}

//____ PreferredPixelSize() __________________________________________________________

WgSize WgScrollChart::PreferredPixelSize() const
{
	WgSize sz = (m_defaultSize * m_scale) / WG_SCALE_BASE;

	sz += m_pixelPadding;
	if (m_pSkin)
		sz = _skinSizeForContent( m_pSkin, sz, m_scale);

	return sz;
}

//____ SetSurfaceFactory() ____________________________________________________

void WgScrollChart::SetSurfaceFactory(wg::SurfaceFactory * pFactory)
{
	m_pFactory = pFactory;
	_regenCanvas();
	_requestRender();
}


//____ SetSkin() ______________________________________________________________

void WgScrollChart::SetSkin(wg::Skin * pSkin)
{
	m_pSkin = pSkin;

	if (pSkin->isOpaque())
		m_bOpaque = true;
	else
		m_bOpaque = false;

	_requestResize();		// Just in case
	_requestRender();
}

//____ SetChartColor() ________________________________________________________

void WgScrollChart::SetChartColor(WgColor color)
{
	if (color != m_chartColor)
	{
		m_chartColor = color;
		m_bRefreshCanvas = true;
		_requestRender();
	}
}

//____ SetScrollFromStart() ___________________________________________________

void WgScrollChart::SetScrollFromStart(bool bScrollOnStart)
{
	m_bScrollFromStart = bScrollOnStart;
}


//____ SetCanvasPadding() _____________________________________________________

void WgScrollChart::SetCanvasPadding(WgBorders padding)
{
	m_pointPadding = padding;
	WgBorders pixelPadding( (padding.top * m_scale) >> WG_SCALE_BINALS, (padding.right * m_scale) >> WG_SCALE_BINALS,
							(padding.bottom * m_scale) >> WG_SCALE_BINALS, (padding.left * m_scale) >> WG_SCALE_BINALS );

	if (pixelPadding != m_pixelPadding)
	{
		m_pixelPadding = pixelPadding;
		_regenCanvas();
	}
}

//____ Start() ________________________________________________________________

bool WgScrollChart::Start(int sampleTTL)
{
	if (sampleTTL < 100)
		return false;

	sampleTTL *= 1000;		// Switch from millisec in input to microsec.
	
    if(m_sampleTTL == sampleTTL)
        return true;

    bool restart = false;

	if (m_bStarted)
    {
        restart = true;
		Stop();
    }

	m_sampleTTL = sampleTTL;
	m_bStarted = true;

	m_sampleBeginTimestamp = 0;
	m_sampleEndTimestamp = m_bScrollFromStart ? sampleTTL : 0;

	// Add start samples so that we start to draw the waves.
	for (auto& wave : m_waves)
    {
        if(restart)
        {
            // We need at least two samples to get going.
            wave.samples.push_back({ (int)(m_sampleEndTimestamp - m_sampleBeginTimestamp), wave.startTopSample, wave.startBottomSample });
        }

		wave.samples.push_front({ (int)m_sampleEndTimestamp, wave.startTopSample, wave.startBottomSample });
    }

	if (m_bDynamicValueRange && _updateDynamics() )
	{
		if (m_valueRangeResponder)
			m_valueRangeResponder(this, m_topValue, m_bottomValue);

		m_bRefreshCanvas = true;
		_requestRender();
	}

	_startReceiveTicks();
	return true;
}

//____ Stop() _________________________________________________________________

void WgScrollChart::Stop()
{
	m_bStarted = false;
	m_bPaused = false;
	m_bRefreshCanvas = true;
	m_sampleBeginTimestamp = 0;
	m_sampleEndTimestamp = 0;

	for (auto& wave : m_waves)
		wave.samples.clear();

	m_topValue = m_setTopValue;
	m_bottomValue = m_setBottomValue;

	_stopReceiveTicks();
}

//____ Pause() ________________________________________________________________

void WgScrollChart::Pause()
{
	if (m_bStarted && !m_bPaused)
	{
		m_bPaused = true;
		_stopReceiveTicks();
	}
}

//____ Continue() _____________________________________________________________

void WgScrollChart::Continue()
{
	if (m_bStarted && m_bPaused)
	{
		m_bPaused = false;
		_startReceiveTicks();
	}
}

//____ SetValueRange() ________________________________________________________

bool WgScrollChart::SetValueRange(float topValue, float bottomValue)
{
	if (topValue == bottomValue)
		return false;

	if (topValue != m_setTopValue || bottomValue != m_setBottomValue)
	{
		m_setTopValue = topValue;
		m_setBottomValue = bottomValue;

		bool	bRangeModified;

		if( m_bDynamicValueRange )
			bRangeModified = _updateDynamics();
		else
		{
			m_topValue = topValue;
			m_bottomValue = bottomValue;
			bRangeModified = true;
		}

		if (bRangeModified)
		{
			if (m_valueRangeResponder)
				m_valueRangeResponder(this, m_topValue, m_bottomValue);

			m_bRefreshCanvas = true;
			_requestRender();
		}
	}

	return true;
}

//____ SetDynamicValueRange() ________________________________________________________

void WgScrollChart::SetDynamicValueRange( bool bDynamic )
{
	if (bDynamic == m_bDynamicValueRange)
		return;

	m_bDynamicValueRange = bDynamic;

	bool	bRangeModified = false;

	if (m_bDynamicValueRange)
	{
		bRangeModified = _updateDynamics();
	}
	else if (m_topValue != m_setTopValue || m_bottomValue != m_setBottomValue)
	{
			m_topValue = m_setTopValue;
			m_bottomValue = m_setBottomValue;
			bRangeModified = true;
	}

	if (bRangeModified)
	{
		if (m_valueRangeResponder)
			m_valueRangeResponder(this, m_topValue, m_bottomValue);

		m_bRefreshCanvas = true;
		_requestRender();
	}
}

//____ SetSampleLabelStyle() __________________________________________________

void WgScrollChart::SetSampleLabelStyle(WgOrigo alignment, WgCoord offset, wg::Skin * pSkin, wg::TextStyle * pStyle, bool bLabelsOnTop)
{
	m_sampleLabelStyle.alignment = alignment;
	m_sampleLabelStyle.offset = offset;
	m_sampleLabelStyle.pSkin = pSkin;
	m_sampleLabelStyle.pTextStyle = pStyle;
    m_sampleLabelStyle.bLabelAtEnd = bLabelsOnTop;
	_requestRender();
}

//____ SetSampleGridLines() ____________________________________________________

void WgScrollChart::SetSampleGridLines(int nLines, GridLine * pLines)
{
	if (nLines == 0 || pLines == nullptr)
	{
		m_sampleGridLines.clear();
	}
	else
	{
		m_sampleGridLines.resize(nLines);

		for (int i = 0; i < nLines; i++)
			m_sampleGridLines[i] = pLines[i];
	}
	_requestRender();

}

//____ SetValueLabelStyle() ___________________________________________________

void WgScrollChart::SetValueLabelStyle(WgOrigo alignment, WgCoord offset, wg::Skin * pSkin, wg::TextStyle * pStyle, bool bLabelsOnRight)
{
	m_valueLabelStyle.alignment = alignment;
	m_valueLabelStyle.offset = offset;
	m_valueLabelStyle.pSkin = pSkin;
	m_valueLabelStyle.pTextStyle = pStyle;
    m_valueLabelStyle.bLabelAtEnd = bLabelsOnRight;
	_requestRender();
}

//____ SetValueGridLines() ____________________________________________________

void WgScrollChart::SetValueGridLines(int nLines, GridLine * pLines)
{
	if (nLines == 0 || pLines == nullptr)
	{
		m_valueGridLines.clear();
	}
	else
	{
		m_valueGridLines.resize(nLines);

		for (int i = 0; i < nLines; i++)
			m_valueGridLines[i] = pLines[i];
	}
	_requestRender();
}

//____ SetResizeResponder() _____________________________________________________

void WgScrollChart::SetResizeResponder(std::function<void(WgScrollChart * pWidget, WgSize newSize)> func)
{
	m_resizeResponder = func;
	if (func)
		func(this, PixelSize());
}

//____ SetValueRangeResponder() _____________________________________________________

void WgScrollChart::SetValueRangeResponder(std::function<void(WgScrollChart * pWidget, float topValue, float bottomValue)> func)
{
	m_valueRangeResponder = func;
	if (func)
		func(this, m_topValue, m_bottomValue);
}


//____ StartLineWave() ____________________________________________________________

int	WgScrollChart::StartLineWave(float startSample, float thickness, WgColor color, std::function<float(uint64_t timeCode)> sampleFeeder)
{
	m_waves.emplace_back();
	Wave&	w = m_waves.back();

	w.startTimestamp = m_windowEnd;

	w.id = m_waveIdCounter++;
	w.type = WaveType::Line;
	w.startTopSample = startSample;
	w.startBottomSample = 0.f;
	w.topLineThickness = thickness;
	w.topLineColor = color;
	w.simpleSampleFeeder = sampleFeeder;
	w.samples.push_back({ (int) (m_sampleEndTimestamp - m_sampleBeginTimestamp),startSample,0 });

	w.nextTopSample = startSample;
	w.nextBottomSample = 0.f;

	return w.id;
}

//____ StartSimpleWave() ______________________________________________________

int WgScrollChart::StartSimpleWave(float startSample, float floor, float topLineThickness, WgColor topLineColor,
	float floorLineThickness, WgColor floorLineColor, WgColor fillColor, std::function<float(uint64_t timeCode)> sampleFeeder)
{
	m_waves.emplace_back();
	Wave&	w = m_waves.back();

	w.startTimestamp = m_windowEnd;

	w.id = m_waveIdCounter++;
	w.type = WaveType::Simple;
	w.startTopSample = startSample;
	w.startBottomSample = 0.f;
	w.floorValue = floor;
	w.topLineThickness = topLineThickness;
	w.topLineColor = topLineColor;
	w.bottomLineThickness = floorLineThickness;
	w.bottomLineColor = floorLineColor;
	w.fillColor = fillColor;
	w.simpleSampleFeeder = sampleFeeder;
	w.samples.push_back({ (int)(m_sampleEndTimestamp - m_sampleBeginTimestamp),startSample,0 });

	w.nextTopSample = startSample;
	w.nextBottomSample = 0.f;

	return w.id;
}

//____ StartComplexWave() _____________________________________________________

int WgScrollChart::StartComplexWave(SamplePair startSample, float topLineThickness, WgColor topLineColor,
	float bottomLineThickness, WgColor bottomLineColor, WgColor fillColor, std::function<SamplePair(uint64_t timeCode)> sampleFeeder)
{
	m_waves.emplace_back();
	Wave&	w = m_waves.back();

	w.startTimestamp = m_windowEnd;

	w.id = m_waveIdCounter++;
	w.type = WaveType::Complex;
	w.startTopSample = startSample.top;
	w.startBottomSample = startSample.bottom;
	w.floorValue = startSample.bottom;
	w.topLineThickness = topLineThickness;
	w.topLineColor = topLineColor;
	w.bottomLineThickness = bottomLineThickness;
	w.bottomLineColor = bottomLineColor;
	w.fillColor = fillColor;
	w.complexSampleFeeder = sampleFeeder;
	w.samples.push_back({(int)(m_sampleEndTimestamp - m_sampleBeginTimestamp),startSample.top,startSample.bottom });

	w.nextTopSample = startSample.top;
	w.nextBottomSample = startSample.bottom;

	return w.id;
}

//____ ClearWave() _____________________________________________________________

bool WgScrollChart::ClearWave(int waveId, float topSample, float bottomSample)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	p->samples.clear();
	p->samples.push_back({(int)(m_sampleEndTimestamp - m_sampleBeginTimestamp),topSample,bottomSample });
	p->samples.push_back({(int)(1),topSample,bottomSample });

	p->startTopSample = topSample;
	p->startBottomSample = bottomSample;

	p->nextTopSample = topSample;
	p->nextBottomSample = bottomSample;

	m_bRefreshCanvas = true;
	return true;
}



//____ StopWave() _____________________________________________________________

bool WgScrollChart::StopWave(int waveId)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	p->bHidden = true;
	return true;
}

//____ StopAllWaves() _________________________________________________________

void WgScrollChart::StopAllWaves()
{
	for (auto& wave : m_waves)
		wave.bHidden = true;
}

//____ SetWaveColors() ________________________________________________________

bool WgScrollChart::SetWaveColors( int waveId, WgColor topLineColor, WgColor bottomLineColor, WgColor fillColor )
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	p->topLineColor = topLineColor;
	p->bottomLineColor = bottomLineColor;
	p->fillColor = fillColor;

	m_bRefreshCanvas = true;
	return true;
}

//____ IsWaveDisplayed() ______________________________________________________

bool WgScrollChart::IsWaveDisplayed(int waveId) const
{
	const Wave * p = _getWave(waveId);
	if (!p)
		return false;

	return !p->bHidden;
}

//____ HideWave() _____________________________________________________________

bool WgScrollChart::HideWave(int waveId)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	if (p->bHidden == false)
	{
		p->bHidden = true;
		m_bRefreshCanvas = true;
	}
	return true;
}

//____ UnhideWave() ___________________________________________________________

bool WgScrollChart::UnhideWave(int waveId)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	if (p->bHidden == true)
	{
		p->bHidden = false;
		m_bRefreshCanvas = true;
	}
	return true;
}

//____ HideAllWaves() _________________________________________________________

void WgScrollChart::HideAllWaves()
{
	for (auto& wave : m_waves)
	{
		if (!wave.bHidden)
		{
			wave.bHidden = true;
			m_bRefreshCanvas = true;
		}
	}
}

//____ UnhideAllWaves() _______________________________________________________

void WgScrollChart::UnhideAllWaves()
{
	for (auto& wave : m_waves)
	{
		if (wave.bHidden)
		{
			wave.bHidden = false;
			m_bRefreshCanvas = true;
		}
	}
}

//____ FeedSample() ___________________________________________________________

bool WgScrollChart::FeedSample(int waveId, float sample)
{
	Wave * p = _getWave(waveId);
	if (!p || p->type == WaveType::Complex )
		return false;

	p->nextTopSample = sample;
	return true;
}

bool WgScrollChart::FeedSample(int waveId, SamplePair sample)
{
	Wave * p = _getWave(waveId);
	if (!p || p->type != WaveType::Complex)
		return false;

	p->nextTopSample = sample.top;
	p->nextBottomSample = sample.bottom;
	return true;
}

//____ SetStaticMode() ________________________________________________________

void WgScrollChart::SetStaticMode( bool bStatic )
{
	m_bStaticMode = bStatic;
	_requestRender();
}

//____ SetStaticModeFadeOutLength() ___________________________________________

void WgScrollChart::SetStaticModeFadeOutLength( int length )
{
	m_fadeTailLength = length;
	_requestRender();
}

//____ SetStaticModeGradient() ________________________________________________

void WgScrollChart::SetStaticModeGradient( const wg::Gradient& gradient )
{
	m_fadeTailGradient = gradient;
	_requestRender();
}



//____ _onEvent() _____________________________________________________________

void WgScrollChart::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	if (pEvent->Type() == WG_EVENT_TICK)
	{
		if( !m_pCanvas )
			return;
		
		// Update timestamps

		int ticks = static_cast<const WgEvent::Tick*>(pEvent)->Microsec();
		m_sampleEndTimestamp += ticks;

		// Set length of last sample and add our new sample to end of each running wave

		for (auto& wave : m_waves)
		{
			if (!wave.bStopped)
			{
/*				if (m_bDynamicValueRange)
				{
					if (wave.nextTopSample >= wave.maxSample)
					{
						if (wave.nextTopSample == wave.maxSample)
							wave.maxSampleCount++;
						else
						{
							wave.maxSample = wave.nextTopSample;
							wave.maxSampleCount = 1;
						}

					}
					if (wave.nextTopSample <= wave.minSample)
					{
						wave.minSample = wave.nextTopSample;
						if (wave.nextTopSample == wave.minSample)
							wave.minSampleCount++;
						else
						{
							wave.minSample = wave.nextTopSample;
							wave.minSampleCount = 1;
						}
					}

					if (wave.type == WaveType::Complex)
					{
						if (wave.nextBottomSample >= wave.maxSample)
						{
							if (wave.nextBottomSample == wave.maxSample)
								wave.maxSampleCount++;
							else
							{
								wave.maxSample = wave.nextBottomSample;
								wave.maxSampleCount = 1;
							}

						}
						if (wave.nextBottomSample <= wave.minSample)
						{
							wave.minSample = wave.nextBottomSample;
							if (wave.nextBottomSample == wave.minSample)
								wave.minSampleCount++;
							else
							{
								wave.minSample = wave.nextBottomSample;
								wave.minSampleCount = 1;
							}
						}
					}
				}
*/
				wave.samples.back().length = ticks;
				wave.samples.push_back({ 0, wave.nextTopSample, wave.nextBottomSample });
			}
		}

		int cutAmount  = ((int)(m_sampleEndTimestamp - m_sampleBeginTimestamp)) - m_sampleTTL - 2;

		// Update m_sampleBeginTimestamp and cut samples not within window anymore from beginning of waves

		if (cutAmount > 0)
		{
			m_sampleBeginTimestamp += cutAmount;

			for (auto& wave : m_waves)
			{
				int waveCutAmount = cutAmount;
//				bool bRefreshMinMax = false;

				while (wave.samples.size() > 2 && wave.samples[0].length <= waveCutAmount )
				{
					const Sample& s = wave.samples[0];

					waveCutAmount -= s.length;


//				if( s.top == wave.maxSample || s.top == wave.minSample || (wave.type == WaveType::Complex && (s.bottom == wave.maxSample )

					wave.samples.pop_front();
				}


				// Update first sample so that it begins at window start.

				float fractionRemoved = waveCutAmount / (float) wave.samples[0].length;

				wave.samples[0].top += (wave.samples[1].top - wave.samples[0].top) * fractionRemoved;
				wave.samples[0].bottom += (wave.samples[1].bottom - wave.samples[0].bottom) * fractionRemoved;

				wave.samples[0].length -= waveCutAmount;
			}
		}



		// Calculate scroll amount and update scrollFraction.

		m_scrollFraction += ticks;

		float samplesPerPixel = m_sampleTTL / (float) m_pCanvas->pixelSize().w;

		int scrollAmount =  (int) ((m_scrollFraction-1) / samplesPerPixel);

		m_scrollAmount += scrollAmount;
		m_scrollFraction -= scrollAmount * samplesPerPixel;

		if (scrollAmount > 0)
		{
			if( m_bStaticMode )
			{
				auto canvas = _skinContentRect( m_pSkin, {0,0,PixelSize()}, WgStateEnum::Default, m_scale);

				int dirtyLength = m_scrollAmount + (m_fadeTailLength*m_scale/WG_SCALE_BASE);
				
				if( m_canvasOfs + dirtyLength <= canvas.w )
				{
					_requestRender(  {canvas.x + m_canvasOfs, canvas.y, dirtyLength, canvas.h} );
				}
				else
				{
					_requestRender( {canvas.x + m_canvasOfs, canvas.y, canvas.w - m_canvasOfs, canvas.h} );
					_requestRender( {canvas.x, canvas.y, dirtyLength - (canvas.w - m_canvasOfs), canvas.h} );
				}
			}
			else
				_requestRender();

			
			
			// Update window end and window start

			m_windowEnd = m_sampleEndTimestamp - m_scrollFraction;

			if (m_windowEnd > m_sampleTTL)
				m_windowBegin = m_windowEnd - m_sampleTTL;
			else
				m_windowBegin = 0;
		}
	}
	else
		WgWidget::_onEvent(pEvent,pHandler);

}

//____ _onCloneContent() ______________________________________________________

void WgScrollChart::_onCloneContent(const WgWidget * _pOrg)
{
	WgScrollChart * pOrg = (WgScrollChart*)_pOrg;

	m_defaultSize = pOrg->m_defaultSize;
}

//____ _renderPatches() _______________________________________________________

void WgScrollChart::_renderPatches(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches)
{

	if (m_pCanvas)
	{
		WgSize sz = m_pCanvas->pixelSize();
		double	timestampInc = m_sampleTTL / (double)sz.w;

		if (m_bRefreshCanvas || m_scrollAmount >= sz.w)
		{
            pDevice->beginCanvasUpdate(m_pCanvas);
			pDevice->setBlendMode(wg::BlendMode::Replace);
			pDevice->fill(sz*64, m_chartColor);
			pDevice->setBlendMode(wg::BlendMode::Blend);

			_renderWaveSegment(pDevice, sz, m_windowBegin, m_windowEnd, timestampInc);

 			pDevice->endCanvasUpdate();

			if (m_windowEnd - m_windowBegin < m_sampleTTL)
			{
				m_canvasOfs = (int) ((m_windowEnd - m_windowBegin) / timestampInc);
			}
			else
				m_canvasOfs = 0; // m_scrollAmount % m_pCanvas->Size().w;

			m_scrollAmount = 0;

			m_bRefreshCanvas = false;
		}
		else if (m_scrollAmount > 0)
		{
			// Adjust our windows to account for thicker lines

			int margin = ((int)(_thickestLine() / 2)) + 1;

			m_canvasOfs = (m_canvasOfs + sz.w - margin) % sz.w;
			m_scrollAmount += margin;

			//

			if (m_canvasOfs + m_scrollAmount <= sz.w )
			{
				double windowBegin = m_windowEnd - m_scrollAmount*timestampInc;
				double windowEnd = m_windowEnd;

				WgRect canvas(m_canvasOfs, 0, m_scrollAmount, sz.h);
				WgRect canvasClip = canvas * 64;

				pDevice->beginCanvasUpdate(m_pCanvas, 1, &canvasClip);
				pDevice->setBlendMode(wg::BlendMode::Replace);
				pDevice->fill(m_chartColor);
				pDevice->setBlendMode(wg::BlendMode::Blend);
				_renderWaveSegment(pDevice, canvas, windowBegin, windowEnd, timestampInc);
			}
			else
			{
				int width1 = sz.w - m_canvasOfs;
				int width2 = m_scrollAmount - width1;

				WgRect canvas1(m_canvasOfs, 0, width1, sz.h);
				WgRect canvas2(0, 0, width2, sz.h);

				WgRect cliplist[2] = { canvas1*64, canvas2*64 };

				pDevice->beginCanvasUpdate(m_pCanvas, 2, cliplist);
				pDevice->setBlendMode(wg::BlendMode::Replace);
				pDevice->fill(m_chartColor);
				pDevice->setBlendMode(wg::BlendMode::Blend);

				double windowBegin = m_windowEnd - m_scrollAmount*timestampInc;
				double windowEnd = m_windowEnd - width2*timestampInc;
				_renderWaveSegment(pDevice, canvas1, windowBegin, windowEnd, timestampInc);

				windowBegin = m_windowEnd - width2*timestampInc;
				windowEnd = m_windowEnd;
				_renderWaveSegment(pDevice, canvas2, windowBegin, windowEnd, timestampInc);
			}

            pDevice->endCanvasUpdate();

			m_canvasOfs = (m_canvasOfs + m_scrollAmount) % sz.w;
			m_scrollAmount = 0;
		}
	}

	WgWidget::_renderPatches(pDevice, _canvas, _window, _pPatches);
}

//____ _thickestLine() ________________________________________________________

float WgScrollChart::_thickestLine() const
{
	float thickness = 0.f;

	for (auto& w : m_waves)
	{
		if (w.topLineThickness > thickness)
			thickness = w.topLineThickness;

		if (w.bottomLineThickness > thickness)
			thickness = w.bottomLineThickness;
	}
	return thickness;
}


//____ _renderWaveSegment() ___________________________________________________

void WgScrollChart::_renderWaveSegment(wg::GfxDevice * pDevice, const WgRect& _canvas, double startTimestamp, double endTimestamp, float timestampInc)
{
	// Get margin and resample start/end timestamps that take thickest line into account

	int margin = ((int)(_thickestLine() / 2)) + 1;
	double  resampleStartTimestamp = startTimestamp - timestampInc*margin;
	double  resampleEndTimestamp = endTimestamp + timestampInc*margin;

	// Setup resample buffer

	int nSamples = (_canvas.w+1) + margin*2;
	int bufferSize = nSamples * sizeof(int) * 2;

	int * pTopBuffer = (int*) wg::GfxBase::memStackAlloc(bufferSize);
	int * pBottomBuffer = pTopBuffer + nSamples;

	// Calculate yOfs, chart floor and valueFactor

	int canvasHeight = m_pCanvas->pixelSize().h;
	float valueFactor = canvasHeight / (m_bottomValue - m_topValue);

	float graphFloor;
	float yOfs;
	if (valueFactor < 0)
	{
		graphFloor = m_bottomValue;
		yOfs = canvasHeight * 64;
	}
	else
	{
		graphFloor = m_topValue;
		yOfs = 0;
	}

	//

	for (auto& wave : m_waves)
	{
		if (!wave.bHidden)
		{
			int waveSamples = nSamples;
			int waveSamplesOffset = 0;

			_resampleWavePortion(waveSamplesOffset, waveSamples, pTopBuffer, pBottomBuffer, wave, resampleStartTimestamp, resampleEndTimestamp, timestampInc);

			int floor = yOfs + (int)((wave.floorValue - graphFloor) * valueFactor * 64);

			WgWaveLine topLine = { waveSamples, wave.topLineThickness*64, wave.topLineColor, pTopBuffer, 0 };
			WgWaveLine bottomLine = { waveSamples, wave.bottomLineThickness*64, wave.bottomLineColor, pBottomBuffer, floor };

			switch (wave.type)
			{
			case WaveType::Line:
				//TODO: Implement!
				break;
			case WaveType::Simple:
				bottomLine.length = 0;
				bottomLine.pWave = nullptr;
				pDevice->drawWave( WgRect( _canvas.x - margin + waveSamplesOffset,_canvas.y, waveSamples-1,_canvas.h )*64, &topLine, &bottomLine, wave.fillColor);
				break;
			case WaveType::Complex:
				pDevice->drawWave( WgRect( _canvas.x - margin + waveSamplesOffset,_canvas.y, waveSamples-1,_canvas.h )*64, &topLine, &bottomLine, wave.fillColor);
				break;
			}
		}
	}

	wg::GfxBase::memStackFree(bufferSize);
}

//____ _resampleWavePortion() _________________________________________________

void WgScrollChart::_resampleWavePortion(int& ofs, int& nSamples, int * pOutTop, int * pOutBottom, const WgScrollChart::Wave& wave, double startTimeStamp, double endTimeStamp, float timestampInc)
{
	double	pixelTimestamp = startTimeStamp;

	uint64_t	sampleTimestamp = m_sampleBeginTimestamp;

	if (sampleTimestamp < wave.startTimestamp)
		sampleTimestamp += wave.startTimestamp + sampleTimestamp;

	// Forward pixels until we have correct prev and next iterators.

	pixelTimestamp += ofs * timestampInc;

	while (nSamples > 0 && sampleTimestamp > pixelTimestamp)
	{
		ofs++;
		nSamples--;
		pixelTimestamp += timestampInc;
	}

	auto itPrev = wave.samples.begin();
	auto itNext = itPrev; itNext++;

	//

	int canvasHeight = m_pCanvas->pixelSize().h;
	float valueFactor = canvasHeight / (m_bottomValue - m_topValue);

	float floor;
	float yOfs;
	if (valueFactor < 0)
	{
		floor = m_bottomValue;
		yOfs = canvasHeight * 64;
	}
	else
	{
		floor = m_topValue;
		yOfs = 0;
	}

	//

	int i = 0;
	while( i < nSamples && itNext != wave.samples.end() )
	{
		while (sampleTimestamp + itPrev->length < pixelTimestamp)
		{
			sampleTimestamp += itPrev->length;
			itPrev = itNext++;
			if (itNext == wave.samples.end())
			{
				nSamples = i;
				return;
			}
		}

		float fraction = (pixelTimestamp - sampleTimestamp) / itPrev->length;


		float top = itPrev->top * (1-fraction) + itNext->top * fraction;
		*pOutTop++ = yOfs + (int) ((top-floor) * valueFactor * 64);


		if (wave.type == WaveType::Complex)
		{
			float bottom = itPrev->bottom * (1-fraction) + itNext->bottom * fraction;
			*pOutBottom++ = yOfs + (int) ((bottom - floor) * valueFactor * 64);
		}
		pixelTimestamp += timestampInc;
		i++;
	}

}

//____ _renderValueGridLines() _____________________________________________________

void WgScrollChart::_renderValueGridLines(wg::GfxDevice * pDevice, const WgRect& _canvas)
{
	// Draw value grid lines

	float top, bottom, mul, startOfs;

	if (!m_valueGridLines.empty())
	{
		// Preparations for both lines and labels

		top = std::min(m_topValue, m_bottomValue);
		bottom = std::max(m_topValue, m_bottomValue);

		mul = _canvas.h / (m_bottomValue - m_topValue);
		startOfs = mul > 0 ? _canvas.y : _canvas.y + _canvas.h;

		// Draw lines

		for (auto& line : m_valueGridLines)
		{
			int yOfs = int(startOfs) + (int)((line.pos - top) * mul + 0.5f);
			pDevice->drawLine( WgCoord( _canvas.x, yOfs )*64, WgDirection::Right, _canvas.w*64, line.color, line.thickness*64 * m_scale / WG_SCALE_BASE);
		}
	}
}

//____ _renderSampleGridLines() _____________________________________________________

void WgScrollChart::_renderSampleGridLines(wg::GfxDevice* pDevice, const WgRect& canvas)
{
	if (!m_sampleGridLines.empty())
	{
		for (auto& line : m_sampleGridLines)
		{
			int xOfs = canvas.x + (int) (line.pos * canvas.w);
			pDevice->drawLine( WgCoord( xOfs, canvas.y )*64, WgDirection::Down, canvas.h*64, line.color, line.thickness*64 * m_scale / WG_SCALE_BASE);
		}
	}
}

//____ _onRender() ____________________________________________________________

void WgScrollChart::_onRender(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window)
{
	WgRect canvas;
	
	if (m_pSkin)
	{
		canvas = _skinContentRect( m_pSkin, _canvas, WgStateEnum::Default, m_scale);
		if (m_chartColor.a == 255)
		{
			WgRect& coverage = canvas;
			
			if ( !coverage.contains( pDevice->clipBounds()/64 ) )
			{
				WgRect topSection(canvas.x, canvas.y, canvas.w, coverage.y - canvas.y);
				WgRect leftSection(canvas.x, coverage.y, coverage.x - canvas.x, coverage.h);
				WgRect rightSection(coverage.x + coverage.w, coverage.y, canvas.x + canvas.w - (coverage.x + coverage.w), coverage.h);
				WgRect bottomSection(canvas.x, coverage.y + coverage.h, canvas.w, canvas.y + canvas.h - (coverage.y + coverage.h));
				
				topSection *= 64;
				leftSection *= 64;
				rightSection *= 64;
				bottomSection *= 64;
				
				const WgRect * pOldClipList = pDevice->clipList();
				int     oldClipListSize = pDevice->clipListSize();
				
				int allocSize = oldClipListSize*sizeof(WgRect)*4;
				WgRect * pRects = (WgRect*) wg::GfxBase::memStackAlloc( allocSize );
				int nRects = 0;
				
				for( int i = 0 ; i < oldClipListSize ; i++ )
				{
					if( pOldClipList[i].isOverlapping(topSection) )
						pRects[nRects++] = WgRect::overlap(pOldClipList[i], topSection );
					
					if( pOldClipList[i].isOverlapping(leftSection) )
						pRects[nRects++] = WgRect::overlap(pOldClipList[i], leftSection );
					
					if( pOldClipList[i].isOverlapping(bottomSection) )
						pRects[nRects++] = WgRect::overlap(pOldClipList[i], bottomSection );
					
					if( pOldClipList[i].isOverlapping(rightSection) )
						pRects[nRects++] = WgRect::overlap(pOldClipList[i], rightSection );
				}
				
				pDevice->setClipList(nRects, pRects);
				
				_renderSkin( m_pSkin, pDevice, WgStateEnum::Default, _canvas, m_scale);
				
				pDevice->setClipList(oldClipListSize, pOldClipList);
				wg::GfxBase::memStackFree(allocSize);
			}
		}
		else
			_renderSkin( m_pSkin, pDevice, WgStateEnum::Default, _canvas, m_scale);
		
	}
	else
		canvas = _canvas;
	
	// Preparations for both grid and wave drawing
	
	WgRect scrollCanvas = canvas - m_pixelPadding;
	
	// Draw background grid lines (note: opaque chart color will cover backround grid lines!)
	
	if (!m_bForegroundGrid)
	{
		_renderSampleGridLines(pDevice, scrollCanvas);
		_renderValueGridLines(pDevice, scrollCanvas);
	}

	// Copy wave from backbuffer

	if (m_pCanvas)
	{
		assert(m_pCanvas->pixelSize() == scrollCanvas.size());
		
		auto oldTintColor = pDevice->tintColor();
		auto oldBlendMode = pDevice->blendMode();
		
		if( m_combinedWavesTintGradient.isValid() )
			pDevice->setTintGradient(scrollCanvas*64, m_combinedWavesTintGradient );
		
		pDevice->setBlitSource(m_pCanvas);
		pDevice->setTintColor(m_combinedWavesTintColor);
		pDevice->setBlendMode(m_combinedWavesBlendMode);
		
		if( m_bStaticMode)
		{
			int canvasLen = m_pCanvas->pixelSize().w;

			int tailLength = m_fadeTailLength*m_scale/WG_SCALE_BASE;
			int tailBegin = m_canvasOfs;
			int tailEnd = (tailBegin + tailLength) % canvasLen;

			if( tailLength > 0 )
			{
				if( tailEnd > tailBegin )
				{
					pDevice->blit(WgCoord(scrollCanvas.x, scrollCanvas.y)*64, WgRect( 0, 0, tailBegin, scrollCanvas.h )*64 );
					pDevice->setTintGradient( WgRect(scrollCanvas.x + tailBegin, scrollCanvas.y, tailLength, scrollCanvas.h)*64, m_fadeTailGradient );
					pDevice->blit(WgCoord(scrollCanvas.x + tailBegin, scrollCanvas.y)*64, WgRect( tailBegin, 0, tailLength, scrollCanvas.h )*64 );
					pDevice->clearTintGradient();
					pDevice->blit(WgCoord(scrollCanvas.x + tailEnd, scrollCanvas.y)*64, WgRect( tailEnd, 0, canvasLen - tailEnd, scrollCanvas.h )*64 );
				}
				else
				{
					pDevice->blit(WgCoord(scrollCanvas.x + tailEnd, scrollCanvas.y)*64, WgRect( tailEnd, 0, tailBegin - tailEnd, scrollCanvas.h )*64 );
					
					pDevice->setTintGradient( WgRect(scrollCanvas.x + tailBegin, scrollCanvas.y, tailLength, scrollCanvas.h)*64, m_fadeTailGradient );
					pDevice->blit(WgCoord(scrollCanvas.x + tailBegin, scrollCanvas.y)*64, WgRect( tailBegin, 0, tailLength, scrollCanvas.h )*64 );
					
					pDevice->setTintGradient( WgRect(scrollCanvas.x - (tailLength - tailEnd), scrollCanvas.y, tailLength, scrollCanvas.h)*64, m_fadeTailGradient );
					pDevice->blit(WgCoord(scrollCanvas.x, scrollCanvas.y)*64, WgRect( 0, 0, tailEnd, scrollCanvas.h )*64 );
					pDevice->clearTintGradient();
				}
			}
			else
				pDevice->blit(WgCoord(scrollCanvas.x, scrollCanvas.y)*64 );
		}
		else
		{
			if( m_windowBegin == 0 )
				pDevice->blit( WgCoord(scrollCanvas.x, scrollCanvas.y)*64, WgRect( 0, 0, scrollCanvas.w, scrollCanvas.h )*64 );
			else
			{
				int firstPartLen = m_pCanvas->pixelSize().w - m_canvasOfs;
				
				pDevice->blit(WgCoord(scrollCanvas.x, scrollCanvas.y)*64, WgRect( m_canvasOfs, 0, firstPartLen, scrollCanvas.h )*64 );
				pDevice->blit(WgCoord(scrollCanvas.x + firstPartLen, scrollCanvas.y)*64, WgRect( 0, 0, scrollCanvas.w - firstPartLen, scrollCanvas.h )*64 );
			}
		}

			
		pDevice->setTintColor(oldTintColor);
		pDevice->setBlendMode(oldBlendMode);
		
		if( m_combinedWavesTintGradient.isValid() )
			pDevice->clearTintGradient();
	}
	else if (m_chartColor.a != 0)
	{
		pDevice->fill(scrollCanvas*64, m_chartColor);
	}

	// Draw foreground sample grid lines

	if (m_bForegroundGrid)
	{
		_renderSampleGridLines(pDevice, scrollCanvas);
		_renderValueGridLines(pDevice, scrollCanvas);
	}

	// Draw value grid labels

	if (!m_valueGridLines.empty())
	{
		float top, bottom, mul, startOfs;

		top = std::min(m_topValue, m_bottomValue);
		bottom = std::max(m_topValue, m_bottomValue);

		mul = scrollCanvas.h / (m_bottomValue - m_topValue);
		startOfs = float(mul > 0 ? scrollCanvas.y : scrollCanvas.y + scrollCanvas.h);

        int xOfs = m_valueLabelStyle.bLabelAtEnd ? canvas.x + canvas.w : canvas.x;

        WgPen pen(pDevice, _canvas);
        wg::TextAttr attr;

        WgBase::defaultStyle()->exportAttr(WgStateEnum::Default, &attr, m_scale >> 6);

        if(m_valueLabelStyle.pTextStyle)
        {
            m_valueLabelStyle.pTextStyle->addToAttr(WgStateEnum::Default, &attr, m_scale >> 6);
        }

        pen.SetScale(m_scale);
        pen.SetAttributes(attr);

        int width = -1;

        // Find the max width of the labels.
        for(const auto& line: m_valueGridLines)
        {
            width = std::max(width, static_cast<int>(WgUtil::lineWidth(nullptr, attr, wg::StateEnum::Default, line.label.chars(), m_scale)));
        }

		for (auto& line : m_valueGridLines)
		{
			int yOfs = int(startOfs) + (int)((line.pos - top) * mul + 0.5f);

			if (!line.label.isEmpty())
			{
				WgSize labelSize;
				labelSize.w = width;
				labelSize.h = pen.GetLineHeight();

				WgCoord textOfs;
				if (m_valueLabelStyle.pSkin)
				{
					labelSize = _skinSizeForContent( m_valueLabelStyle.pSkin, labelSize, m_scale);
					textOfs = _skinContentRect( m_valueLabelStyle.pSkin, labelSize, WgStateEnum::Default, m_scale).pos();
				}

				WgCoord labelPos = _placeLabel({ xOfs, yOfs }, m_valueLabelStyle.alignment, m_valueLabelStyle.offset, labelSize);

				if (m_valueLabelStyle.pSkin)
					_renderSkin(m_valueLabelStyle.pSkin, pDevice, WgStateEnum::Default, { labelPos,labelSize }, m_scale);

				pen.SetPos(labelPos + textOfs);
				WgGfxDevice::PrintLine(pDevice, pen, attr, line.label.chars());
			}
		}
	}
	
	// Draw sample gridline labels
	
	if (!m_sampleGridLines.empty() )
	{
		int   yOfs2 = m_sampleLabelStyle.bLabelAtEnd ? canvas.y : canvas.y + canvas.h;

		for (auto &line : m_sampleGridLines)
		{
			if (!line.label.isEmpty())
			{
				int xOfs = scrollCanvas.x + (int)(line.pos * scrollCanvas.w);

				WgPen	pen(pDevice, _canvas);
				wg::TextAttr attr;

				WgBase::defaultStyle()->exportAttr(WgStateEnum::Default, &attr, m_scale >> 6);
				if( m_sampleLabelStyle.pTextStyle )
					m_sampleLabelStyle.pTextStyle->addToAttr(WgStateEnum::Default, &attr, m_scale >> 6);

				pen.SetScale(m_scale);
				pen.SetAttributes(attr);

				WgSize labelSize;
				labelSize.w = WgUtil::lineWidth(nullptr, attr, wg::StateEnum::Default, line.label.chars(),m_scale);
				labelSize.h = pen.GetLineHeight();

				WgCoord textOfs;
				if (m_sampleLabelStyle.pSkin)
				{
					labelSize = _skinSizeForContent( m_sampleLabelStyle.pSkin, labelSize, m_scale);
					textOfs = _skinContentRect( m_sampleLabelStyle.pSkin, labelSize, WgStateEnum::Default, m_scale ).pos();
				}

				WgCoord labelPos = _placeLabel({ xOfs, yOfs2 }, m_sampleLabelStyle.alignment, m_sampleLabelStyle.offset, labelSize);

				if (m_sampleLabelStyle.pSkin)
					_renderSkin( m_sampleLabelStyle.pSkin, pDevice, WgStateEnum::Default, { labelPos,labelSize }, m_scale);

				pen.SetPos(labelPos + textOfs);
				WgGfxDevice::PrintLine(pDevice, pen, attr, line.label.chars());
			}
		}
	}

	
}

//____ _onAlphaTest() _________________________________________________________

bool WgScrollChart::_onAlphaTest(const WgCoord& ofs)
{
	//TODO: Implement?

	return true;
}

//____ _onNewSize() ___________________________________________________________

void WgScrollChart::_onNewSize(const WgSize& size)
{
	if (m_resizeResponder)
		m_resizeResponder(this, size);

	_regenCanvas();
	WgWidget::_onNewSize(size);
}

//____ _setScale() ____________________________________________________________

void WgScrollChart::_setScale(int scale)
{
	WgWidget::_setScale(scale);

	SetCanvasPadding(m_pointPadding);		// Update m_pixelPadding and resample if needed.
	_requestResize();
	_requestRender();
}

//____ _regenCanvas() _________________________________________________________

void WgScrollChart::_regenCanvas()
{
		m_pCanvas = nullptr;

	if (m_pFactory)
	{
		WgSize sz = _getScrollWindow().size();

		if (sz.w <= 0 && sz.h <= 0)
			return;

		m_pCanvas = m_pFactory->createSurface( WGBP(Surface,
													_.size = sz,
													_.format = m_pixelType,
													_.canvas = true ));
//		m_pCanvas->Fill(m_chartColor);
		m_canvasOfs = 0;
		m_bRefreshCanvas = true;
	}
}

//____ _updateDynamics() ______________________________________________________

bool WgScrollChart::_updateDynamics()
{
	bool bNeedRefresh = false;

	if (m_bDynamicValueRange)
	{
		float min = std::numeric_limits<float>::max();
		float max = std::numeric_limits<float>::min();

		for (auto& it : m_waves)
		{
			if (min > it.minSample)
				min = it.minSample;

			if (max < it.maxSample)
				max = it.maxSample;
		}

		if (max < m_setTopValue)
			max = m_setTopValue;

		if (min > m_setBottomValue)
			min = m_setBottomValue;

		if (max != m_topValue || min != m_bottomValue)
		{
			m_topValue = max;
			m_bottomValue = min;
			bNeedRefresh = true;
		}
	}

	return bNeedRefresh;
}

//____ _placeLabel() __________________________________________________________

WgCoord	WgScrollChart::_placeLabel(WgCoord startPoint, WgOrigo alignment, WgCoord labelOffset, WgSize labelSize) const
{
	return startPoint + (labelOffset*m_scale/WG_SCALE_BASE) - WgCoord(0, labelSize.h) + WgUtil::OrigoToOfs(alignment, labelSize);
}

//____ _getWave() _____________________________________________________________

WgScrollChart::Wave * WgScrollChart::_getWave(int waveId)
{
	for (auto& wave : m_waves)
		if (wave.id == waveId)
			return &wave;
	return nullptr;
}

const WgScrollChart::Wave * WgScrollChart::_getWave(int waveId) const
{
	for (auto& wave : m_waves)
		if (wave.id == waveId)
			return &wave;
	return nullptr;
}

//____ _getScrollWindow() _____________________________________________________

WgRect WgScrollChart::_getScrollWindow() const
{
	WgRect r = PixelSize();

	if (m_pSkin)
		r = _skinContentRect( m_pSkin, r, WgStateEnum::Default, m_scale).size();

	r -= m_pixelPadding;

	if (r.w < 0) r.w = 0;
	if (r.h < 0) r.h = 0;

	return r;
}
