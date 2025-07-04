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

#include <wg2_chart.h>
#include <wg2_pen.h>
#include <wg2_base.h>
#include <wg_texttool.h>
#include <wg2_util.h>
#include <wg_surfacefactory.h>

#include <wg_gfxdevice_gen2.h>
#include <wg_softbackend.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

static const char	c_widgetType[] = {"Chart"};

//____ Constructor ____________________________________________________________

WgChart::WgChart()
{
	m_defaultSize = WgSize(200,100);
	m_bOpaque = false;
	m_mode = WG_MODE_NORMAL;

	m_bDynamicValueRange = true;
	m_bDynamicSampleRange = true;

	m_topValue			= 1.f;
	m_bottomValue		= -1.f;

	m_firstSample		= 0.f;
	m_lastSample		= 0.f;

	m_sampleLabelStyle.alignment = WgOrigo::South;
	m_sampleLabelStyle.offset = { 0,0 };
	m_sampleLabelStyle.bLabelAtEnd = false;

	m_valueLabelStyle.alignment = WgOrigo::West;
	m_valueLabelStyle.offset = { 0,0 };
	m_valueLabelStyle.bLabelAtEnd = false;

	m_waveIdCounter = 1;
	m_renderSectionWidth = 64;			// Keep a high value until we have upgraded to new GfxDevice with multi-clip support.
}

//____ Destructor _____________________________________________________________

WgChart::~WgChart()
{
}

//____ Type() _________________________________________________________________

const char * WgChart::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgChart::GetClass()
{
	return c_widgetType;
}

//____ SetPreferredChartSize() _______________________________________________________

void WgChart::SetPreferredChartSize( const WgSize& size )
{
	if( size != m_defaultSize )
	{
		m_defaultSize = size;
		_requestResize();
	}
}

//____ PreferredPixelSize() __________________________________________________________

WgSize WgChart::PreferredPixelSize() const
{
	WgSize sz = (m_defaultSize * m_scale) / WG_SCALE_BASE;

	sz += m_pixelPadding;
	if (m_pSkin)
		sz = _skinSizeForContent( m_pSkin, sz, m_scale);

	return sz;
}

//____ SetSkin() ______________________________________________________________

void WgChart::SetSkin(wg::Skin * pSkin)
{
	//TODO: Might need resampling if content padding has changed.

	m_pSkin = pSkin;
	_requestResize();		// Just in case
	_requestRenderInCache();
}

//____ SetCanvasPadding() _____________________________________________________

void WgChart::SetCanvasPadding(WgBorders padding)
{
	m_pointPadding = padding;
	WgBorders pixelPadding( (padding.top * m_scale) >> WG_SCALE_BINALS, (padding.right * m_scale) >> WG_SCALE_BINALS,
							(padding.bottom * m_scale) >> WG_SCALE_BINALS, (padding.left * m_scale) >> WG_SCALE_BINALS );

	if (pixelPadding != m_pixelPadding)
	{
		m_pixelPadding = pixelPadding;
		_resampleAllWaves();
		_requestRenderInCache();
	}
}

//____ CanvasPadding() ________________________________________________________

WgBorders WgChart::CanvasPadding() const
{
	return m_pointPadding;
}

//____ ClearWaves() ___________________________________________________________

void WgChart::ClearWaves()
{
	m_waves.clear();
	_requestRenderInCache();
}

//____ AddWave() ______________________________________________________________

int WgChart::AddWave()
{
	m_waves.emplace_back();
	Wave&	w = m_waves.back();
	w.id = m_waveIdCounter++;
	return w.id;
}

//____ IsWaveDisplayed() ______________________________________________________

bool WgChart::IsWaveDisplayed(int waveId) const
{
	const Wave * p = _getWave(waveId);
	if (!p)
		return false;

	return !p->bHidden;
}

//____ HideWave() _____________________________________________________________

bool WgChart::HideWave(int waveId)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	if (p->bHidden == false)
	{
		p->bHidden = true;

		if( waveId >= m_cacheFirst && waveId <= m_cacheLast )
			_requestRenderInCache();
		else
			_requestRender();
	}
	return true;
}

//____ UnhideWave() ___________________________________________________________

bool WgChart::UnhideWave(int waveId)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	if (p->bHidden == true)
	{
		p->bHidden = false;
		if( waveId >= m_cacheFirst && waveId <= m_cacheLast )
			_requestRenderInCache();
		else
			_requestRender();
	}
	return true;
}

//____ HideAllWaves() _________________________________________________________

void WgChart::HideAllWaves()
{
	for (auto& wave : m_waves)
	{
		if (!wave.bHidden)
		{
			wave.bHidden = true;
			_requestRenderInCache();
		}
	}
}

//____ UnhideAllWaves() _______________________________________________________

void WgChart::UnhideAllWaves()
{
	for (auto& wave : m_waves)
	{
		if (wave.bHidden)
		{
			wave.bHidden = false;
			_requestRenderInCache();
		}
	}
}



//____ SetWaveStyle() _________________________________________________________

bool WgChart::SetWaveStyle(int waveId, WgColor fillColor, float topLineThickness, WgColor topLineColor, float bottomLineThickness, WgColor bottomLineColor, int transitionMs)
{
	Wave * p = _getWave(waveId);
	if (!p)
		return false;

	// HACK!
	// This avoids the problem of top/bottom lines switching color when passing each other in all currently used cases.

	if( bottomLineThickness == 0.f )
		bottomLineColor = topLineColor;

	bool startTransition = false;
	if(fillColor != p->fillColor)
	{
		p->fillEnd = fillColor;
		p->fillStart = p->fillColor;
		startTransition = true;
	}

	if(topLineColor != p->topLineColor)
	{
		p->topLineColorEnd = topLineColor;
		p->topLineColorStart = p->topLineColor;
		startTransition = true;
	}

	if(bottomLineColor != p->bottomLineColor)
	{
		p->bottomLineColorEnd = bottomLineColor;
		p->bottomLineColorStart = p->bottomLineColor;
		startTransition = true;
	}

	if(transitionMs == 0)
	{
	   p->fillColor = fillColor;
	   p->topLineColor = topLineColor;
	   p->bottomLineColor = bottomLineColor;
	}


	if(startTransition)
	{
		p->fadeTimeCounter = 0;
		_startReceiveTicks();
	}

	p->topLineThickness = topLineThickness;
	p->bottomLineThickness = bottomLineThickness;
	p->fadeTime = transitionMs;

	if( waveId >= m_cacheFirst && waveId <= m_cacheLast )
		_requestRenderInCache();
	else
		_requestRender();
	return true;
}

//____ SetWaveGradient() _________________________________________________________

bool WgChart::SetWaveGradient(int waveId, wg::Gradient gradient)
{
    Wave * p = _getWave(waveId);
    if (!p)
        return false;

    p->m_waveGradient = gradient;
    return true;
}

void WgChart::SetSampleLabelColor(WgColor col)
{
	auto bp = m_sampleLabelStyle.pTextStyle->blueprint();
	bp.color = col;
		m_sampleLabelStyle.pTextStyle = wg::TextStyle::create(bp);
}

void WgChart::SetSampleGridLineColor(WgColor col)
{
	for(auto &p : m_sampleGridLines)
	{
		p.color = col;
	}
}
void WgChart::SetValueLabelColor(WgColor col)
{
	auto bp = m_valueLabelStyle.pTextStyle->blueprint();
	bp.color = col;
	m_valueLabelStyle.pTextStyle = wg::TextStyle::create(bp);
}

void WgChart::SetValueGridLineColor(WgColor col)
{
	for(auto &p : m_valueGridLines)
	{
		p.color = col;
	}
}


//____ SetWaveSamples() _______________________________________________________

bool WgChart::SetWaveSamples(int waveId, int firstSample, int nSamples, float * pTopBorderSamples, float * pBottomBorderSamples)
{
	return _setWaveSamples(waveId, firstSample, nSamples, pTopBorderSamples, pBottomBorderSamples, 0.f);
}

bool WgChart::SetWaveSamples(int waveId, int firstSample, int nSamples, const float * pSamples, float floor)
{
	return _setWaveSamples(waveId, firstSample, nSamples, pSamples, nullptr, floor);
}


bool WgChart::_setWaveSamples(int waveId, int firstSample, int nSamples, const float * pTopBorderSamples, float * pBottomBorderSamples, float defaultSample )
{
	Wave * pWave = _getWave(waveId);
	if (!pWave)
		return false;

	if(m_useTemporalFiltering)
	{
		// m_msTime is not correct, but works ok, is updated on every TICK event.
		// Also, the attack/release filtering of the curve requires constant
		// updates by SetWaveSamples, which is not good, but at this point it's
		// too difficult to get this into the "resampled wave" data.
		//
		// Another caveat is the this is only supported by TopSamples (not BottomSamples)
		// at the moment.
		_filterWaveSamples(pTopBorderSamples, pWave, m_msTime);
	}

	// Check if waves are the same as already stored so that we don't trigger a re-render.
	bool earlyOut = false;
	if(pWave->nSamples == nSamples)
	{
		float sum = 0.0f;
		if (pTopBorderSamples) {
			for(int i=0; i<nSamples; i++)
			{
				sum += std::abs(pWave->orgTopSamples[i] - pTopBorderSamples[i]);
			}
		}
		if (pBottomBorderSamples) {
			for(int i=0; i<nSamples; i++)
			{
				sum += std::abs(pWave->orgBottomSamples[i] - pBottomBorderSamples[i]);
			}
		}
		if(std::abs(sum) < 0.001f)
			earlyOut = true;
	}
	else
		_requestRender();			// Hack! Possibly solves "staircase"-problem of C1 screens.

	if(earlyOut)
		return true;


	pWave->firstSample = firstSample;
	pWave->nSamples = nSamples;
	pWave->defaultSample = defaultSample;

	float max = -std::numeric_limits<float>::max();
	float min =  std::numeric_limits<float>::max();

	if (pTopBorderSamples)
	{
		pWave->orgTopSamples.resize(nSamples, m_defaultValue);

		for (int i = 0; i < nSamples; i++)
		{
			float sample = pTopBorderSamples[i];
			if(!m_useTemporalFiltering)
				pWave->orgTopSamples[i] = sample;

			if (max < sample) max = sample;
			if (min > sample) min = sample;
		}
	}
	else
	{
		pWave->orgTopSamples.clear();
		max = defaultSample;
		min = defaultSample;
	}
	if (pBottomBorderSamples)
	{
		pWave->orgBottomSamples.resize(nSamples, m_defaultValue);
		for (int i = 0; i < nSamples; i++)
		{
			float sample = pBottomBorderSamples[i];
			pWave->orgBottomSamples[i] = sample;

			if (max < sample) max = sample;
			if (min > sample) min = sample;
		}
	}
	else
	{
		pWave->orgBottomSamples.clear();
		if (max < defaultSample) max = defaultSample;
		if (min > defaultSample) min = defaultSample;
	}

	pWave->maxSample = max;
	pWave->minSample = min;

	bool bResampleAll = _updateDynamics();
	if ( bResampleAll )
	{
		_resampleAllWaves();
		_requestRenderInCache();
	}
	else
		_resampleWave(pWave, true);

	return true;
}

//____ _filterWaveSamples() ________________________________________________________

void WgChart::_filterWaveSamples(const float* in, Wave* p, int ms)
{
	int N = p->nSamples;
	if(p->orgTopSamples.size() != N)
		return;

	// Lambda filtering on linear values means linear filtering in dB space
	// x dB/second avg (in log space)
	// ms = milliseconds since last call
	// delta = x * ms / 1000

	float delta_release = m_releaseTime * (float)ms * 0.001f;
	float delta_attack = m_attackTime * (float)ms * 0.001f;

	for(int i=0; i<N; i++)
	{
		if(in[i] < p->orgTopSamples[i] - delta_release)
			p->orgTopSamples[i] -= delta_release;
		else if(in[i] > p->orgTopSamples[i] + delta_attack)
			p->orgTopSamples[i] += delta_attack;
		else
			p->orgTopSamples[i] = in[i];
	}
}


//____ SetFixedValueRange() ________________________________________________________

bool WgChart::SetFixedValueRange(float topValue, float bottomValue)
{
	if (topValue == bottomValue)
		return false;

	if (m_bDynamicValueRange || topValue != m_topValue || bottomValue != m_bottomValue)
	{
		m_bDynamicValueRange = false;
		m_topValue = topValue;
		m_bottomValue = bottomValue;
		_resampleAllWaves();
		if (m_valueRangeResponder)
			m_valueRangeResponder(this, topValue, bottomValue);
		_requestRenderInCache();
	}

	return true;
}

//____ SetDynamicValueRange() ________________________________________________________

void WgChart::SetDynamicValueRange()
{
	if (!m_bDynamicValueRange)
	{
		m_bDynamicValueRange = true;
		_updateDynamics();
		_resampleAllWaves();
		_requestRenderInCache();
//		if (m_valueRangeResponder)
//			m_valueRangeResponder(this, m_topValue, m_bottomValue);
	}
}


void WgChart::IgnoreDynamicScaling(int waveId, bool ignore)
{
	Wave * pWave = _getWave(waveId);
	if (!pWave)
		return;

	pWave->ignoreDynamicScaling = ignore;
}

//____ SetRenderSectionWidth() _________________________________________________________

bool WgChart::SetRenderSectionWidth( int pointWidth )
{
	if( pointWidth < 16 || pointWidth > 4096 )
		return false;

	m_renderSectionWidth = pointWidth;
	return true;
}


//____ SetFixedSampleRange() __________________________________________________

bool WgChart::SetFixedSampleRange(float firstSample, float lastSample)
{
	if (firstSample == lastSample)
		return false;

	if (m_bDynamicSampleRange || firstSample != m_firstSample || lastSample != m_lastSample)
	{

		m_bDynamicSampleRange = false;
		m_firstSample = firstSample;
		m_lastSample = lastSample;
		_resampleAllWaves();				//TODO: Optimize! Can skip if fractional start offset and width is same.
		if (m_sampleRangeResponder)
			m_sampleRangeResponder(this, firstSample, lastSample);
		_requestRenderInCache();
	}
	return true;
}

//____ NativeSampleRange() __________________________________________________

int WgChart::NativeSampleRange() const
{
	int width = PixelSize().w;

	if( m_pSkin )
		width -= _skinContentPadding( m_pSkin, m_scale).w;

	width -= m_pixelPadding.width();

	return width > 0 ? width + 1 : 0;
}


//____ SetDynamicSampleRange() ________________________________________________________

void WgChart::SetDynamicSampleRange()
{
	if (!m_bDynamicSampleRange)
	{
		m_bDynamicSampleRange = true;
		_updateDynamics();
		_resampleAllWaves();
		_requestRenderInCache();

//		if (m_sampleRangeResponder)
//			m_sampleRangeResponder(this, m_firstSample, m_lastValue);
	}
}

//____ SetSampleLabelStyle() __________________________________________________

void WgChart::SetSampleLabelStyle(WgOrigo alignment, WgCoord offset, wg::Skin * pSkin, wg::TextStyle * pStyle, bool bLabelsOnTop )
{
	m_sampleLabelStyle.alignment = alignment;
	m_sampleLabelStyle.offset = offset;
	m_sampleLabelStyle.pSkin = pSkin;
	m_sampleLabelStyle.pTextStyle = pStyle;
	m_sampleLabelStyle.bLabelAtEnd = bLabelsOnTop;
	_requestRender();
}

//____ SetSampleGridLines() ____________________________________________________

void WgChart::SetSampleGridLines(int nLines, GridLine * pLines)
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

void WgChart::SetValueLabelStyle(WgOrigo alignment, WgCoord offset, wg::Skin * pSkin, wg::TextStyle * pStyle, bool bLabelsOnRight )
{
	m_valueLabelStyle.alignment = alignment;
	m_valueLabelStyle.offset = offset;
	m_valueLabelStyle.pSkin = pSkin;
	m_valueLabelStyle.pTextStyle = pStyle;
	m_valueLabelStyle.bLabelAtEnd = bLabelsOnRight;
	_requestRender();
}

//____ SetValueGridLines() ____________________________________________________

void WgChart::SetValueGridLines(int nLines, GridLine * pLines)
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

void WgChart::SetResizeResponder(std::function<void(WgChart * pWidget, WgSize newSize)> func)
{
	m_resizeResponder = func;
	if (func)
		func(this, PixelSize());
}

//____ SetSampleRangeResponder() _____________________________________________________

void WgChart::SetSampleRangeResponder(std::function<void(WgChart * pWidget, float firstSample, float lastSample)> func)
{
	m_sampleRangeResponder = func;
	if (func)
		func(this, m_firstSample, m_lastSample);
}

//____ SetValueRangeResponder() _____________________________________________________

void WgChart::SetValueRangeResponder(std::function<void(WgChart * pWidget, float topValue, float bottomValue)> func)
{
	m_valueRangeResponder = func;
	if (func)
		func(this, m_topValue, m_bottomValue);
}

//____ SetBitmapCaching() ____________________________________________________________

bool WgChart::SetBitmapCaching( int firstWaveId, int lastWaveId, wg::SurfaceFactory * pFactory )
{
	if( firstWaveId < 1 || firstWaveId > lastWaveId || lastWaveId >= m_waveIdCounter || pFactory == nullptr )
		return false;

	m_cacheFirst = firstWaveId;
	m_cacheLast = lastWaveId;

	if( pFactory != m_pSurfaceFactory )
	{
		if( m_pCacheBitmap )
		{
			m_pCacheBitmap = nullptr;
			m_cacheDirt.clear();
		}

		m_pSurfaceFactory = pFactory;
	}

	return true;
}

//____ ClearBitmapCaching() __________________________________________________________

void WgChart::ClearBitmapCaching()
{
	m_cacheFirst    = 0;
	m_cacheLast     = 0;

	if( m_pCacheBitmap )
		m_pCacheBitmap = nullptr;

	m_pSurfaceFactory = nullptr;
	m_cacheDirt.clear();
}

//____ _onEvent() _____________________________________________________________

void WgChart::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	switch( pEvent->Type() )
	{
		case WG_EVENT_TICK:
		{
			const WgEvent::Tick * pTick = static_cast<const WgEvent::Tick*>(pEvent);

			int ms = pTick->Millisec();
			m_msTime = ms; // Used by _filterWaveSamples

			bool isDirty = false;
			for( int i = 0 ; i < m_waves.size() ; i++ )
			{
				Wave * p = &m_waves[i];
				if(!p)
					break;

				p->fadeTimeCounter += pTick->Millisec();
				if(p->fadeTimeCounter < p->fadeTime)
				{
					float progress = p->fadeTimeCounter / (float) p->fadeTime;

					p->fillColor.r = p->fillStart.r + (int)((((int)p->fillEnd.r) - p->fillStart.r) * progress);
					p->fillColor.g = p->fillStart.g + (int)((((int)p->fillEnd.g) - p->fillStart.g) * progress);
					p->fillColor.b = p->fillStart.b + (int)((((int)p->fillEnd.b) - p->fillStart.b) * progress);
					p->fillColor.a = p->fillStart.a + (int)((((int)p->fillEnd.a) - p->fillStart.a) * progress);

					p->topLineColor.r = p->topLineColorStart.r + (int)((((int)p->topLineColorEnd.r) - p->topLineColorStart.r) * progress);
					p->topLineColor.g = p->topLineColorStart.g + (int)((((int)p->topLineColorEnd.g) - p->topLineColorStart.g) * progress);
					p->topLineColor.b = p->topLineColorStart.b + (int)((((int)p->topLineColorEnd.b) - p->topLineColorStart.b) * progress);
					p->topLineColor.a = p->topLineColorStart.a + (int)((((int)p->topLineColorEnd.a) - p->topLineColorStart.a) * progress);

					p->bottomLineColor.r = p->bottomLineColorStart.r + (int)((((int)p->bottomLineColorEnd.r) - p->bottomLineColorStart.r) * progress);
					p->bottomLineColor.g = p->bottomLineColorStart.g + (int)((((int)p->bottomLineColorEnd.g) - p->bottomLineColorStart.g) * progress);
					p->bottomLineColor.b = p->bottomLineColorStart.b + (int)((((int)p->bottomLineColorEnd.b) - p->bottomLineColorStart.b) * progress);
					p->bottomLineColor.a = p->bottomLineColorStart.a + (int)((((int)p->bottomLineColorEnd.a) - p->bottomLineColorStart.a) * progress);

					isDirty = true;
				}
				else
				{
					p->fillColor = p->fillEnd;
					p->topLineColor = p->topLineColorEnd;
					p->bottomLineColor = p->bottomLineColorEnd;
				}
			}

			if(isDirty)
			{
				_requestRender();
			}
			else
			{
				_requestRender();
				if(!m_useTemporalFiltering)
					_stopReceiveTicks();
			}

			break;
		}
		default:
			WgWidget::_onEvent(pEvent,pHandler);
			break;
	}
}
//____ _onCloneContent() _______________________________________________________

void WgChart::_onCloneContent( const WgWidget * _pOrg )
{
	WgChart * pOrg = (WgChart*) _pOrg;

	m_defaultSize = pOrg->m_defaultSize;
}

//____ _renderPatches() ________________________________________________________

void WgChart::_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{
	if( m_cacheFirst > 0 )
	{
		if( !m_pCacheBitmap )
		{
			m_pCacheBitmap = m_pSurfaceFactory->createSurface( WGBP(Surface,
																_.size = _canvas.size(),
																_.format = wg::PixelFormat::BGRA_8) );
			m_cacheDirt.add( _canvas.size()*64 );
		}

		if( !m_cacheDirt.isEmpty() )
			_updateBitmapCache( pDevice );
	}

	WgWidget::_renderPatches( pDevice, _canvas, _window, _pPatches );
}

//____ _updateBitmapCache() ____________________________________________________

void WgChart::_updateBitmapCache( wg::GfxDevice * pDevice )
{
	auto pOldCanvas = pDevice->canvas();

	WgColor oldTint = pDevice->tintColor();
	WgBlendMode oldBlend = pDevice->blendMode();

	pDevice->beginCanvasUpdate(m_pCacheBitmap, m_cacheDirt.size(), m_cacheDirt.begin());
	pDevice->setBlendMode(WgBlendMode::Replace);
	pDevice->setTintColor(WgColor::White);

	pDevice->fill(WgColor::Transparent);

	pDevice->setBlendMode(WgBlendMode::Blend);

	WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, m_pCacheBitmap->pixelSize(), wg::State::Default, m_scale) : WgRect(0,0,m_pCacheBitmap->pixelSize());
	WgRect waveCanvas = canvas - m_pixelPadding;


	m_cacheDirt.clip( WgRect( waveCanvas.x, canvas.y, waveCanvas.w, canvas.h )*64 );

	for( int i = m_cacheFirst-1 ; i <= m_cacheLast-1 ; i++ )
		_renderWave( m_waves[i], pDevice, waveCanvas );

/*
	pDirt = m_cacheDirt.Begin();
	while( pDirt < m_cacheDirt.End() )
	{
		WgRect waveClip(*pDirt, { waveCanvas.x, canvas.y, waveCanvas.w, canvas.h });        // Samples stay within padding, values may stretch outside.
		for( int i = m_cacheFirst-1 ; i <= m_cacheLast-1 ; i++ )
			_renderWave( m_waves[i], pDevice, waveCanvas, waveClip );
		pDirt++;
	}
*/
	m_cacheDirt.clear();
	pDevice->endCanvasUpdate();
	pDevice->setTintColor(oldTint);
	pDevice->setBlendMode(oldBlend);
}

//____ _onRender() _____________________________________________________________

void WgChart::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	WgWidget::_onRender(pDevice, _canvas, _window);

	WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, _canvas, wg::State::Default, m_scale) : _canvas;

	// Preparations for both grid and wave drawing

	WgRect waveCanvas = canvas - m_pixelPadding;
    float sampleScale = (m_lastSample - m_firstSample == 0) ? 1.0f : waveCanvas.w / (m_lastSample - m_firstSample);

	int   yOfs = m_valueLabelStyle.bLabelAtEnd ? canvas.y : canvas.y + canvas.h;
	int   yOfs2 = m_sampleLabelStyle.bLabelAtEnd ? canvas.y : canvas.y + canvas.h;

	// Draw sample grid lines


	if (!m_sampleGridLines.empty() && m_bShowGrid)
	{
		for (auto& line : m_sampleGridLines)
		{
			int xOfs = waveCanvas.x + (int) ((line.pos - m_firstSample) * sampleScale);
			pDevice->drawLine( WgCoord( xOfs, canvas.y )*64, WgDirection::Down, canvas.h*64, line.color, line.thickness*64 * m_scale / WG_SCALE_BASE);
		}

		for (auto &line : m_sampleGridLines)
		{
			if (!line.label.isEmpty())
			{
				int xOfs = waveCanvas.x + (int)((line.pos - m_firstSample) * sampleScale);

				WgPen	pen(pDevice, _canvas);
				wg::TextAttr attr;

				WgBase::defaultStyle()->exportAttr(wg::State::Default, &attr, m_scale >> 6);
				if( m_sampleLabelStyle.pTextStyle )
					m_sampleLabelStyle.pTextStyle->addToAttr(wg::State::Default, &attr, m_scale >> 6);

				pen.SetScale(m_scale);
				pen.SetAttributes(attr);

				WgSize labelSize;
				labelSize.w = WgUtil::lineWidth(nullptr, attr, wg::StateEnum::Default, line.label.chars(),m_scale);
				labelSize.h = pen.GetLineHeight();

				WgCoord textOfs;
				if (m_sampleLabelStyle.pSkin)
				{
					labelSize = _skinSizeForContent( m_sampleLabelStyle.pSkin, labelSize, m_scale);
					textOfs = _skinContentRect( m_sampleLabelStyle.pSkin, labelSize, wg::State::Default, m_scale ).pos();
				}

				WgCoord labelPos = _placeLabel({ xOfs, yOfs2 }, m_sampleLabelStyle.alignment, m_sampleLabelStyle.offset, labelSize);

				if (m_sampleLabelStyle.pSkin)
					_renderSkin( m_sampleLabelStyle.pSkin, pDevice, wg::State::Default, { labelPos,labelSize }, m_scale);

				pen.SetPos(labelPos + textOfs);
				WgGfxDevice::PrintLine(pDevice, pen, attr, line.label.chars());
			}
		}
	}

	// Draw value grid lines

	if (!m_valueGridLines.empty() && m_bShowGrid)
	{
		float top = std::min(m_topValue, m_bottomValue);
		float bottom = std::max(m_topValue, m_bottomValue);

		float mul = waveCanvas.h / (m_bottomValue - m_topValue);
		int	  startOfs = mul > 0 ? waveCanvas.y : waveCanvas.y + waveCanvas.h;

		int   xOfs = m_valueLabelStyle.bLabelAtEnd ? canvas.x + canvas.w : canvas.x;

		for (auto& line : m_valueGridLines)
		{
			int yOfs = 0;

			if(std::isfinite((line.pos - top) * mul))
				yOfs = startOfs + (int)((line.pos - top) * mul); // +0.5f);
			pDevice->drawLine( WgCoord( canvas.x, yOfs )*64, WgDirection::Right, canvas.w*64, line.color, line.thickness*64 * m_scale / WG_SCALE_BASE );
		}

		for (auto &line : m_valueGridLines)
		{
			if (!line.label.isEmpty())
			{
				int yOfs = 0;

				if(std::isfinite((line.pos - top) * mul))
					yOfs = startOfs + (int)((line.pos - top) * mul); // +0.5f);

				WgPen pen(pDevice, _canvas);
				wg::TextAttr attr;

				WgBase::defaultStyle()->exportAttr(wg::State::Default, &attr, m_scale >> 6);
				if( m_valueLabelStyle.pTextStyle )
					m_valueLabelStyle.pTextStyle->addToAttr(wg::State::Default, &attr, m_scale >> 6);

				pen.SetScale(m_scale);
				pen.SetAttributes(attr);


				WgSize labelSize;
				labelSize.w = WgUtil::lineWidth(nullptr, attr, wg::StateEnum::Default, line.label.chars(),m_scale);
				labelSize.h = pen.GetLineHeight();

				WgCoord textOfs;
				if (m_valueLabelStyle.pSkin)
				{
					labelSize = _skinSizeForContent( m_valueLabelStyle.pSkin, labelSize, m_scale);
					textOfs = _skinContentRect( m_valueLabelStyle.pSkin, labelSize, wg::State::Default, m_scale).pos();
				}

				WgCoord labelPos = _placeLabel({ xOfs, yOfs }, m_valueLabelStyle.alignment, m_valueLabelStyle.offset, labelSize);

				if (m_valueLabelStyle.pSkin)
					_renderSkin( m_valueLabelStyle.pSkin, pDevice, wg::State::Default, { labelPos,labelSize }, m_scale);

				pen.SetPos(labelPos + textOfs);
				WgGfxDevice::PrintLine(pDevice, pen, attr, line.label.chars());
			}
		}
	}

	// Render waves

	const WgRect * pOldClipList = pDevice->clipList();
	int     oldClipListSize = pDevice->clipListSize();

	WgRect waveClip = WgRect( waveCanvas.x, canvas.y, waveCanvas.w, canvas.h )*64;	// Samples stay within padding, values may stretch outside.

	int allocSize = oldClipListSize*sizeof(WgRect);
	WgRect * pRects = (WgRect*) wg::GfxBase::memStackAlloc( allocSize );
	int nRects = 0;

	for( int i = 0 ; i < oldClipListSize ; i++ )
	{
		pRects[nRects] = WgRect::overlap(pOldClipList[i], waveClip );
		if( !pRects[nRects].isEmpty() )
			nRects++;
	}

	pDevice->setClipList(nRects, pRects);

	for( int i = 0 ; i < (int) m_waves.size() ; i++ )
	{
		if( i == m_cacheFirst-1 )
		{
			pDevice->setBlitSource(m_pCacheBitmap);
			pDevice->blit( WgCoord(_canvas.x, _canvas.y)*64, _canvas.size()*64 );
			i = m_cacheLast-1;
		}
		else
			_renderWave( m_waves[i], pDevice, waveCanvas );
	}

	pDevice->setClipList(oldClipListSize, pOldClipList);
	wg::GfxBase::memStackFree(allocSize);
}

//____ _renderWave() __________________________________________________________

void WgChart::_renderWave( Wave& wave, wg::GfxDevice * pDevice, const WgRect& waveCanvas )
{
	if (wave.bHidden || wave.nSamples == 0)
	   return;

	int xOfs = wave.resampledFirst;

	WgWaveLine top, bottom;

	top.color = wave.topLineColor;
	top.thickness = wave.topLineThickness*64*m_scale/WG_SCALE_BASE;
	top.length = (int) wave.resampledTop.size();
	top.pWave = wave.resampledTop.data();
	top.hold = wave.resampledDefault;

	bottom.color = wave.bottomLineColor;
	bottom.thickness = wave.bottomLineThickness*64*m_scale/WG_SCALE_BASE;
	bottom.length = (int)wave.resampledBottom.size();
	bottom.pWave = wave.resampledBottom.data();
	bottom.hold = wave.resampledDefault;

	int length = std::max(top.length, bottom.length)-1;

    // Check if gradient is set by SetWaveGradient
    bool useGradient = wave.m_waveGradient.isValid();

	// Don't use gradient on "slow" devices
	if( pDevice->typeInfo() == wg::GfxDeviceGen2::TYPEINFO )
	{
		auto pBackend = static_cast<wg::GfxDeviceGen2*>(pDevice)->backend();
		if( pBackend->typeInfo()  == wg::SoftBackend::TYPEINFO )
			useGradient = false;
	}

	if(length >= 1)
	{
        if(useGradient)
        {
            pDevice->setTintGradient(WgRect(waveCanvas.x + xOfs, waveCanvas.y, length, waveCanvas.h)*64, wave.m_waveGradient);
        }

		pDevice->drawWave(WgRect(waveCanvas.x + xOfs, waveCanvas.y, length, waveCanvas.h)*64, &top, &bottom, wave.fillColor);

        if(useGradient)
        {
            pDevice->clearTintGradient();
        }
//        pDevice->ClipDrawHorrWave(waveClip, WgCoord(waveCanvas.x + xOfs, waveCanvas.y), length, top, bottom, wave.fillColor, wave.fillColor);
	}
}

//____ _placeLabel() __________________________________________________________

WgCoord	WgChart::_placeLabel(WgCoord startPoint, WgOrigo alignment, WgCoord labelOffset, WgSize labelSize) const
{
	return startPoint + (labelOffset*m_scale/WG_SCALE_BASE) -WgCoord(labelSize.w,labelSize.h) + WgUtil::OrigoToOfs(alignment, labelSize);
}

//____ _getWave() _____________________________________________________________

WgChart::Wave * WgChart::_getWave(int waveId)
{
	for (auto& wave : m_waves)
		if (wave.id == waveId)
			return &wave;
	return nullptr;
}

const WgChart::Wave * WgChart::_getWave(int waveId) const
{
	for (auto& wave : m_waves)
		if (wave.id == waveId)
			return &wave;
	return nullptr;
}



//____ _onAlphaTest() ___________________________________________________________

bool WgChart::_onAlphaTest( const WgCoord& ofs )
{
	//TODO: Implement?

	return true;
}


//____ _onNewSize() ___________________________________________________________

void WgChart::_onNewSize(const WgSize& size)
{
	if (m_resizeResponder)
		m_resizeResponder(this, size);

	_resampleAllWaves();
	WgWidget::_onNewSize(size);
}

//____ _setScale() ____________________________________________________________

void WgChart::_setScale(int scale)
{
	WgWidget::_setScale(scale);

	SetCanvasPadding(m_pointPadding);		// Update m_pixelPadding and resample if needed.
	_requestResize();
	_requestRenderInCache();
}


//____ _updateDynamics() ______________________________________________________

bool WgChart::_updateDynamics()
{
	bool bNeedRefresh = false;

	if (m_bDynamicSampleRange)
	{
		int first = INT_MAX, last = INT_MIN;

		for (auto& it : m_waves)
		{
			if (first > it.firstSample)
				first = it.firstSample;

			if (last < it.firstSample + it.nSamples)
				last = it.firstSample + it.nSamples;
		}

		if ((float)first != m_firstSample || (float)last != m_lastSample)
		{
			m_firstSample = (float) first;
			m_lastSample = (float) last;
			bNeedRefresh = true;
		}
	}

	float min = std::numeric_limits<float>::max();
	float max = std::numeric_limits<float>::min();

	if (m_bDynamicValueRange)
	{
		for (auto& it : m_waves)
		{
			// Need to be able to set this per curve
			if(it.ignoreDynamicScaling)
			{
			}
			else
			{
				if (min > it.minSample)
					min = it.minSample;

				if (max < it.maxSample)
					max = it.maxSample;
			}
		}

		if (!m_bForceSymmetricAndQuantized && ( max != m_topValue || min != m_bottomValue))
		{
			m_topValue = max;
			m_bottomValue = min;
			bNeedRefresh = true;
		}
	}

	// This assumes that the range has 0 in the middle.
	if (m_bDynamicValueRange && m_bForceSymmetricAndQuantized)
	{
		// Quantize
//        m_fDynamicMinSize = 0.1f;
		float top = std::ceil(max/m_fDynamicStepSize) * m_fDynamicStepSize;
		float bot = std::floor(min/m_fDynamicStepSize) * m_fDynamicStepSize;

		// Make symmetric around 0
		float max = std::max(top, -bot);
//        max += 3; m_fDynamicStepSize*2; // Start to increase when at the step size.
		max+= m_fDynamicMinSize/2; // Start to increase when at minSize/2
		max = std::max(m_fDynamicMinSize, max);

		if ( max != m_topValue )
		{
			m_topValue = max;
			bNeedRefresh = true;
		}
		else
		{
			bNeedRefresh = false;
		}

		m_topValue = max;
		m_bottomValue = -max;
	}

	return bNeedRefresh;
}

//____ _resampleAllWaves() ____________________________________________________

void WgChart::_resampleAllWaves()
{
	for (auto& wave : m_waves)
		_resampleWave(&wave);
}

//____ _resampleWave() ________________________________________________________

void WgChart::_resampleWave(Wave * pWave, bool bRequestRenderOnChanges )
{
	WgSize	canvas = PixelSize();

	if (m_pSkin)
		canvas = _skinContentRect( m_pSkin, canvas, wg::State::Default, m_scale);


	canvas -= m_pixelPadding;

    // sampleScale < 1 means upsampling
	float sampleScale = (m_lastSample - m_firstSample) / (canvas.w + 1);

	int nResampled = 0;

	if(sampleScale != 0)
		nResampled = (int) (pWave->nSamples / sampleScale);

	if (nResampled < 0)
		nResampled = 0;

	float valueFactor = canvas.h / (m_bottomValue - m_topValue);

	float floor;
	float yOfs;
	if (valueFactor < 0)
	{
		floor = m_bottomValue;
		yOfs = (float) canvas.h*64;
	}
	else
	{
		floor = m_topValue;
		yOfs = 0.f;
	}

    floor = !std::isfinite(floor) ? 1.0f : floor;

	int newDefault = 0;
	int newFirst = 0;

	if(std::isfinite((pWave->defaultSample - floor) * valueFactor * 64))
		newDefault = int(yOfs) + (int)((pWave->defaultSample - floor) * valueFactor * 64);

	if(std::isfinite((pWave->firstSample - m_firstSample) * sampleScale))
		newFirst = (int)((pWave->firstSample - m_firstSample) * sampleScale);


	int *	pNewTopSamples = nullptr;
	int *	pNewBottomSamples = nullptr;
	int		nbNewTopSamples = 0;
	int		nbNewBottomSamples = 0;

	if (pWave->orgTopSamples.size() > 0)
	{
		nbNewTopSamples = nResampled;

		if( bRequestRenderOnChanges )
			pNewTopSamples = (int*) wg::GfxBase::memStackAlloc( nResampled * sizeof(int) );
		else
		{
			pWave->resampledTop.resize(nResampled);
			pNewTopSamples = &pWave->resampledTop[0];
		}

		if (nResampled == pWave->nSamples)
		{
			for (int i = 0; i < nResampled; i++)
				pNewTopSamples[i] = int(yOfs) + (int)((pWave->orgTopSamples[i] - floor) * valueFactor * 64);
		}
		else
		{
			if(nResampled > 0.0f)
			{
                if(sampleScale < 1.0f) // upsampling
                {
                    float stepFactor = (pWave->orgTopSamples.size() - 1) / (float) nResampled;

                    for (int i = 0; i < nResampled; i++)
                    {
                        float sample = stepFactor*i;
                        int ofs = (int)sample;
                        int frac2 = ((int)(sample * 64)) & 0x3F;
                        int frac1 = 64 - frac2;

                        float val1 = (pWave->orgTopSamples[ofs] - floor) * valueFactor * 64;
                        float val2 = (pWave->orgTopSamples[ofs+1] - floor) * valueFactor * 64;
                        if(std::isfinite(val1) && std::isfinite(val2))
                            pNewTopSamples[i] = int(yOfs) + ((((int)val1)*frac1 + ((int)val2)*frac2) >> 6);
                        else
                            pNewTopSamples[i] = 0;
                    }
                }
                else // downsampling
                {
                    // stepFactor is > 1
                    float stepFactor = (pWave->orgTopSamples.size() - 1) / (float) nResampled;

                    int j = 0;
                    for (int i = 0; i < nResampled; i++)
                    {
                        // For each new sample, check which sample is the furthest
                        // away from 0. Use that one. This mode was designed for EQ curves,
                        // but will probably work for most curves.

                        float val = 0.f;
                        // TODO: Can we overrun here? j = stepFactor*i = (orgSize-1) * (n_Resampled-1) / nResampled. NOPE
                        while(j <= stepFactor*i)
                        {
                            float cur = pWave->orgTopSamples[j];
                            val = (std::abs(cur) > std::abs(val)) ? cur : val;
                            j++;
                        }
                        // Just a test to see if sanitizer accepts our code.
                        if(!std::isfinite(yOfs))
                            yOfs = 0.0f;
                        if(std::isfinite(val))
                            pNewTopSamples[i] = int(yOfs) + (int)((val-floor) * valueFactor * 64);
                        else
                            pNewTopSamples[i] = 0;
                    }
                }
			}
		}
	}

	if (pWave->orgBottomSamples.size() > 0)
	{
		nbNewBottomSamples = nResampled;

		if (bRequestRenderOnChanges)
			pNewBottomSamples = (int*) wg::GfxBase::memStackAlloc(nResampled * sizeof(int));
		else
		{
			pWave->resampledBottom.resize(nResampled);
			pNewBottomSamples = &pWave->resampledBottom[0];
		}

		if (nResampled == pWave->nSamples)
		{
			for (int i = 0; i < nResampled; i++) {
				if(std::isfinite(pWave->orgBottomSamples[i]))
					pNewBottomSamples[i] = int(yOfs) + (int)((pWave->orgBottomSamples[i] - floor) * valueFactor * 64);
				else
					pNewBottomSamples[i] = 0;
			}
		}
		else
		{
			if(nResampled > 0.0f)
			{
                if(sampleScale < 1.0f) // upsampling
                {

                    float stepFactor = (pWave->orgTopSamples.size() - 1) / (float)nResampled;

                    for (int i = 0; i < nResampled; i++)
                    {
                        float sample = stepFactor*i;
                        int ofs = (int)sample;
                        int frac2 = ((int)(sample * 64)) & 0x3F;
                        int frac1 = 64 - frac2;

                        float val1 = (pWave->orgBottomSamples[ofs] - floor) * valueFactor * 64;
                        float val2 = (pWave->orgBottomSamples[ofs + 1] - floor) * valueFactor * 64;

                        if(std::isfinite(val1) && std::isfinite(val2))
                            pNewBottomSamples[i] = int(yOfs) + ((((int)val1)*frac1 + ((int)val2)*frac2) >> 6);
                        else
                            pNewBottomSamples[i] = 0;
                    }
                }
                else // downsampling
                {
                    // stepFactor is > 1
                    float stepFactor = (pWave->orgTopSamples.size() - 1) / (float) nResampled;

                    int j = 0;
                    for (int i = 0; i < nResampled; i++)
                    {
                        // For each new sample, check which sample is the furthest
                        // away from 0. Use that one. This mode was designed for EQ curves,
                        // but will probably work for most curves.

                        float val = 0.f;
                        while(j <= stepFactor*i)
                        {
                            float cur = pWave->orgBottomSamples[j];
                            val = (std::abs(cur) > std::abs(val)) ? cur : val;
                            j++;
                        }

                        if(std::isfinite(val))
                            pNewBottomSamples[i] = int(yOfs) + (int)((val-floor) * valueFactor * 64);
                        else
                            pNewBottomSamples[i] = 0;
                    }
                }
			}
		}
	}

	// Request render on the changes

	if (bRequestRenderOnChanges)
	{
		int begOrgSamples = pWave->resampledFirst;
		int begNewSamples = newFirst;
		float maxLineThickness = std::max(pWave->topLineThickness, pWave->bottomLineThickness)*m_scale / WG_SCALE_BASE;

		bool bInCache = (pWave >= _getWave(m_cacheFirst) && pWave < _getWave(m_cacheLast) );

		auto topPointer    = pWave->resampledTop.size() > 0    ? &pWave->resampledTop[0] : nullptr;
		auto bottomPointer = pWave->resampledBottom.size() > 0 ? &pWave->resampledBottom[0] : nullptr;
		_requestRenderOnNewSamples(	begOrgSamples, int(pWave->resampledTop.size()), topPointer, int(pWave->resampledBottom.size()), bottomPointer,
									begNewSamples, nbNewTopSamples, pNewTopSamples, nbNewBottomSamples, pNewBottomSamples,
									pWave->resampledDefault, newDefault, maxLineThickness, bInCache );

		// Replace old samples with new. Reversed order of Bottom/Top samples so that we free stack memory in right order.

		if( nbNewBottomSamples > 0 )
		{
			pWave->resampledBottom.resize(nResampled);
			memcpy(&pWave->resampledBottom[0], pNewBottomSamples, nbNewBottomSamples * sizeof(int));
			wg::GfxBase::memStackFree(nbNewBottomSamples * sizeof(int));
		}

		if( nbNewTopSamples > 0 )
		{
			pWave->resampledTop.resize(nResampled);
			memcpy(&pWave->resampledTop[0], pNewTopSamples, nbNewTopSamples * sizeof(int));
			wg::GfxBase::memStackFree(nbNewTopSamples * sizeof(int));
		}
	}

	// Finish up

	pWave->resampledDefault = newDefault;
	pWave->resampledFirst = newFirst;

	if (nbNewBottomSamples == 0)
		pWave->resampledBottom.clear();

	if (nbNewTopSamples == 0)
		pWave->resampledTop.clear();
}

//____ _requestRenderOnNewSamples() ___________________________________________

void WgChart::_requestRenderOnNewSamples(   int begOrgSamples, int nbOrgTopSamples, int * pOrgTopSamples, int nbOrgBottomSamples, int * pOrgBottomSamples,
											int begNewSamples, int nbNewTopSamples, int * pNewTopSamples, int nbNewBottomSamples, int * pNewBottomSamples,
											int orgDefaultSample, int newDefaultSample, float maxLineThickness, bool bInCache )
{
	// Calculate size of our sample canvas

	WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, PixelSize(), wg::State::Default, m_scale).size() : PixelSize();
	canvas -= m_pixelPadding;

	// Calculate needed margin (in pixels) for line thickness

	int margin = int(maxLineThickness/2) + 2;

	// Calculate what section we need to start in


	int sectionBeg = (std::min(begOrgSamples,begNewSamples) / m_renderSectionWidth) * m_renderSectionWidth;



	while (sectionBeg < canvas.w)
	{
		int topDirtOfs = 0, topDirtHeight = 0;
		int bottomDirtOfs = 0, bottomDirtHeight = 0;

		int sectionWidth = std::min(m_renderSectionWidth, canvas.w - sectionBeg);

		int orgSampleOfs = sectionBeg - begOrgSamples - margin;
		int newSampleOfs = sectionBeg - begNewSamples - margin;
		int nSamples = sectionWidth + margin * 2;

		int min1=0;
		int max1=0;
		int min2=0;
		int max2=0;

		// Create dirty rect covering all old and new top samples

		if (nbOrgTopSamples > 0 || nbNewTopSamples > 0 || orgDefaultSample != newDefaultSample)
		{
			if (_lineFragmentMinMax(orgSampleOfs, nSamples, nbOrgTopSamples, pOrgTopSamples, orgDefaultSample, &min1, &max1))
			{
				topDirtOfs = min1 / 64 - margin;
				topDirtHeight = max1 / 64 + 2 + margin*2 - topDirtOfs;
			}

			if (_lineFragmentMinMax(newSampleOfs, nSamples, nbNewTopSamples, pNewTopSamples, newDefaultSample, &min2, &max2))
			{
				if (min1 == max1 && min2 == max2 && min1 == min2)
				{
					// Totally flat line section with no movement, no need to render it

					topDirtOfs = 0;
					topDirtHeight = 0;
				}
				else
				{
					int newOfs = min2 / 64 - margin;
					int newHeight = max2 / 64 + 2 + margin*2 - newOfs;

					if (newOfs < topDirtOfs)
					{
						topDirtHeight += topDirtOfs - newOfs;
						topDirtOfs = newOfs;
					}

					if (newOfs + newHeight > topDirtOfs + topDirtHeight)
						topDirtHeight = newOfs + newHeight - topDirtOfs;

				}
			}
		}

		// Calculate dirty height covering all old and new bottom samples

		if (nbOrgBottomSamples > 0 || nbNewBottomSamples > 0 || orgDefaultSample != newDefaultSample)
		{
			if (_lineFragmentMinMax(orgSampleOfs, nSamples, nbOrgBottomSamples, pOrgBottomSamples, orgDefaultSample, &min1, &max1))
			{
				bottomDirtOfs = min1 / 64 - margin;
				bottomDirtHeight = max1 / 64 + 2 + margin*2 - bottomDirtOfs;
			}

			if (_lineFragmentMinMax(newSampleOfs, nSamples, nbNewBottomSamples, pNewBottomSamples, newDefaultSample, &min2, &max2))
			{
				if (min1 == max1 && min2 == max2 && min1 == min2)
				{
					// Totally flat line section with no movement, no need to render it

					bottomDirtOfs = 0;
					bottomDirtHeight = 0;
				}
				else
				{
					int newOfs = min2 / 64 - margin;
					int newHeight = max2 / 64 + 2 + margin*2 - newOfs;

					if (newOfs < bottomDirtOfs)
					{
						bottomDirtHeight += bottomDirtOfs - newOfs;
						bottomDirtOfs = newOfs;
					}

					if (newOfs + newHeight > bottomDirtOfs + bottomDirtHeight)
						bottomDirtHeight = newOfs + newHeight - bottomDirtOfs;
				}
			}
		}

		// Do some clipping on height

		if (topDirtOfs >= canvas.h)
		{
			topDirtOfs = 0;
			topDirtHeight = 0;
		}

		if (topDirtOfs < 0)
		{
			topDirtHeight = std::max( topDirtHeight + topDirtOfs, 0 );
			topDirtOfs = 0;
		}

		if (topDirtOfs + topDirtHeight > canvas.h)
			topDirtHeight = canvas.h - topDirtOfs;

		if (bottomDirtOfs >= canvas.h)
		{
			bottomDirtOfs = 0;
			bottomDirtHeight = 0;
		}

		if (bottomDirtOfs < 0)
		{
			bottomDirtHeight = std::max(bottomDirtHeight + bottomDirtOfs, 0);
			bottomDirtOfs = 0;
		}

		if (bottomDirtOfs + bottomDirtHeight > canvas.h)
			bottomDirtHeight = canvas.h - bottomDirtOfs;


		// Generate dirty rect(s) and request render as needed

		if (bottomDirtOfs < topDirtOfs + topDirtHeight && bottomDirtOfs + bottomDirtHeight > topDirtOfs)
		{
			int dirtTop = std::min(topDirtOfs, bottomDirtOfs);
			int dirtHeight = std::max(topDirtOfs + topDirtHeight, bottomDirtOfs + bottomDirtHeight) - dirtTop;

			if (dirtHeight > 0)
			{
				WgRect rect( canvas.x + sectionBeg, canvas.y + dirtTop, sectionWidth, dirtHeight );

				if( bInCache )
					_requestRenderInCache(rect);
				else
					_requestRender(rect);
			}
		}
		else
		{
			if (topDirtHeight > 0)
			{
				WgRect rect( canvas.x + sectionBeg, canvas.y + topDirtOfs, sectionWidth, topDirtHeight );

				if( bInCache )
					_requestRenderInCache(rect);
				else
					_requestRender(rect);
			}

			if (bottomDirtHeight > 0)
			{
				WgRect rect( canvas.x + sectionBeg, canvas.y + bottomDirtOfs, sectionWidth, bottomDirtHeight );

				if( bInCache )
					_requestRenderInCache(rect);
				else
					_requestRender(rect);
			}
		}


		//

		sectionBeg += sectionWidth;
	}

}

//____ _requestRenderInCache() ___________________________________________________

void WgChart::_requestRenderInCache()
{
	if( m_pCacheBitmap )
		m_cacheDirt.add( m_pCacheBitmap->pixelSize()*64 );
	_requestRender();
}

void WgChart::_requestRenderInCache( const WgRect& rect )
{
	if( m_pCacheBitmap )
		m_cacheDirt.add( rect*64 );
	_requestRender(rect);
}


//____ _lineFragmentMinMax() ______________________________________________________

bool WgChart::_lineFragmentMinMax(int begin, int length, int nbSamples, int * pSamples, int defaultSample, int * wpMin, int * wpMax)
{
	int min, max;

	// Special case: Having 0 samples means that default sample is used.

	if (length == 0)
	{
		*wpMin = defaultSample;
		*wpMax = defaultSample;
		return true;
	}

	//

	if (begin < 0)
	{
		length += begin;
		begin = 0;
	}

	if (begin + length > nbSamples)
		length = nbSamples - begin;

	if (length <= 0)
		return false;

	max = min = pSamples[begin];

	for (int i = begin+1; i < begin + length; i++)
	{
		int s = pSamples[i];
		min = std::min(s, min);
		max = std::max(s, max);
	}

	*wpMin = min;
	*wpMax = max;
	return true;
}
