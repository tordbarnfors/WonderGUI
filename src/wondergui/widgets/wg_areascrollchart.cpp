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
#include <wg_base.h>
#include <wg_gfxdevice.h>
#include <wg_areascrollchart.h>

#include <cstring>

namespace wg
{

	const TypeInfo AreaScrollChart::TYPEINFO = {"AreaScrollChart", &Widget::TYPEINFO};

	//____ constructor ____________________________________________________________

	AreaScrollChart::AreaScrollChart() : entries(this)
	{
	}

	//____ destructor _____________________________________________________________

	AreaScrollChart::~AreaScrollChart()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& AreaScrollChart::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setPadWithLastSample() _______________________________________________

	void AreaScrollChart::setPadWithLastSample(bool bPadWithLast)
	{
		m_bPadWithLastSample = bPadWithLast;
	}

	//____ _update() ____________________________________________________________

	void AreaScrollChart::_update(int microPassed, int64_t microsecTimestamp)
	{
		ScrollChart::_update(microPassed, microsecTimestamp);

		// Remove samples not needed anymore

		_removeOutdatedSamples();

		// Update color/tint transitions.

		bool	transitionsActive = false;

		// Update all transitions

		for (auto& graph : entries)
		{
			if (graph.m_pColorTransition)
			{
				int timestamp = graph.m_colorTransitionProgress + microPassed;

				if (timestamp >= graph.m_pColorTransition->duration())
				{
					graph.m_colorTransitionProgress = 0;
					graph.m_pColorTransition = nullptr;

					graph.m_fillColor = graph.m_endFillColor;
					graph.m_outlineColor = graph.m_endOutlineColor;

					graph.m_fillGradient = graph.m_endFillGradient;
					graph.m_outlineGradient = graph.m_endOutlineGradient;
				}
				else
				{
					graph.m_colorTransitionProgress = timestamp;

					graph.m_fillColor = graph.m_pColorTransition->snapshot(timestamp, graph.m_startFillColor, graph.m_endFillColor);
					graph.m_outlineColor = graph.m_pColorTransition->snapshot(timestamp, graph.m_startOutlineColor, graph.m_endOutlineColor);

					if (!graph.m_fillGradient.isUndefined())
					{
						graph.m_fillGradient.topLeft = graph.m_pColorTransition->snapshot(timestamp, graph.m_startFillGradient.topLeft, graph.m_endFillGradient.topLeft);
						graph.m_fillGradient.topRight = graph.m_pColorTransition->snapshot(timestamp, graph.m_startFillGradient.topRight, graph.m_endFillGradient.topRight);
						graph.m_fillGradient.bottomLeft = graph.m_pColorTransition->snapshot(timestamp, graph.m_startFillGradient.bottomLeft, graph.m_endFillGradient.bottomLeft);
						graph.m_fillGradient.bottomRight = graph.m_pColorTransition->snapshot(timestamp, graph.m_startFillGradient.bottomRight, graph.m_endFillGradient.bottomRight);

						graph.m_outlineGradient.topLeft = graph.m_pColorTransition->snapshot(timestamp, graph.m_startOutlineGradient.topLeft, graph.m_endOutlineGradient.topLeft);
						graph.m_outlineGradient.topRight = graph.m_pColorTransition->snapshot(timestamp, graph.m_startOutlineGradient.topRight, graph.m_endOutlineGradient.topRight);
						graph.m_outlineGradient.bottomLeft = graph.m_pColorTransition->snapshot(timestamp, graph.m_startOutlineGradient.bottomLeft, graph.m_endOutlineGradient.bottomLeft);
						graph.m_outlineGradient.bottomRight = graph.m_pColorTransition->snapshot(timestamp, graph.m_startOutlineGradient.bottomRight, graph.m_endOutlineGradient.bottomRight);
					}

					transitionsActive = true;
				}

				_requestFullRedraw();
			}
		}

		if (m_bTransitioning && !transitionsActive)
		{
			m_bTransitioning = false;
			_stopReceiveUpdates();
		}
	}

	//____ _didAddEntries() ___________________________________________________

	void AreaScrollChart::_didAddEntries(AreaScrollChartEntry* pEntry, int nb)
	{
		for (int i = 0; i < nb; i++)
			pEntry->m_pDisplay = this;

		_requestFullRedraw();
	}

	//____ _didMoveEntries() __________________________________________________

	void AreaScrollChart::_didMoveEntries(AreaScrollChartEntry* pFrom, AreaScrollChartEntry* pTo, int nb)
	{
		_requestFullRedraw();
	}

	//____ _willEraseEntries() ________________________________________________

	void AreaScrollChart::_willEraseEntries(AreaScrollChartEntry* pEntry, int nb)
	{
		_requestFullRedraw();
	}

	//____ _updateWaveformEdge() _________________________________________________

	void AreaScrollChart::_updateWaveformEdge(Waveform* pWaveform, int64_t beginUS, int pixelIncUS, bool bTopEdge, AreaScrollChartEntry::SampleSet* pSamples)
	{
		// TODO: Better interpolation, especially when shrinking.

		int sampleIdx = bTopEdge ? 0 : 1;

        int wfSamples = pWaveform->nbSamples();

		spx * pConverted = (spx*) Base::memStackAlloc(wfSamples * sizeof(spx));

		SizeSPX canvasSize = _canvasSize();

		float valueFactor = canvasSize.h / (m_displayFloor - m_displayCeiling);

		int64_t	pixelUS = beginUS;

		for (int i = 0; i < wfSamples; i++)
		{
			while( pSamples[1].timestamp < pixelUS )
				pSamples++;

			float frac2 = int(pixelUS - pSamples[0].timestamp) / float(pSamples[1].timestamp - pSamples[0].timestamp);
			float frac1 = 1.f - frac2;

			float val1 = (pSamples[0].samples[sampleIdx] - m_displayCeiling) * valueFactor;
			float val2 = (pSamples[1].samples[sampleIdx] - m_displayCeiling) * valueFactor;

			pConverted[i] = int(val1 * frac1 + val2 * frac2);

			pixelUS += pixelIncUS;
		}

		if (bTopEdge)
			pWaveform->setSamples(0, wfSamples, pConverted, nullptr);
		else
			pWaveform->setSamples(0, wfSamples, nullptr, pConverted);

		Base::memStackFree(wfSamples * sizeof(spx));
	}


	//____ _renderOnScrollSurface() ___________________________________________

	void AreaScrollChart::_renderOnScrollSurface( GfxDevice * pDevice, SizeSPX canvasSize, spx rightEdgeOfs, int64_t rightEdgeTimestamp, spx dirtLen )
	{
		int microsecPerPixel = m_displayTime / (canvasSize.w / 64);

		for( auto& entry : entries )
		{
			// Skip entry if it isn't visible.

			if( !entry.m_bVisible || (entry.m_fillColor.a == 0 && (entry.m_outlineColor.a == 0 || (entry.m_topOutlineThickness == 0 && entry.m_bottomOutlineThickness == 0) ) ) )
				continue;

			// Calculate needed margin left and right of our window.

			pts maxThickness = std::max(entry.m_topOutlineThickness,entry.m_bottomOutlineThickness);

			int pixelMargin = (((maxThickness * m_scale) / 2) + 63) / 64;

			int64_t firstEdgeTimestamp = rightEdgeTimestamp - (dirtLen/64+pixelMargin)*microsecPerPixel;
			int64_t lastEdgeTimestamp = rightEdgeTimestamp + pixelMargin*microsecPerPixel;

			// Fill in with default samples if missing up to our lastEdgeTimestamp.

			if( entry.m_samples.empty() )
				_initEntrySamples(&entry);

			if( entry.latestSampleTimestamp() < lastEdgeTimestamp )
			{
				if( entry.m_fetcher )
					entry.m_fetcher( entry.latestSampleTimestamp(), firstEdgeTimestamp, lastEdgeTimestamp, m_latestTimestamp);

				if( entry.latestSampleTimestamp() < lastEdgeTimestamp )
				{
					AreaScrollChartEntry::SampleSet spl;
					spl.timestamp = lastEdgeTimestamp;

					if( m_bPadWithLastSample && !entry.m_samples.empty() )
					{
						spl.samples[0] = entry.m_samples.back().samples[0];
						spl.samples[1] = entry.m_samples.back().samples[1];
					}
					else
					{
						spl.samples[0] = entry.m_defaultTopSample;
						spl.samples[1] = entry.m_defaultBottomSample;
					}

                    entry.m_samples.push_back(spl);
                }
            }


			// Create our waveform

			SizeI waveformSize = { dirtLen/64 + pixelMargin*2, canvasSize.h / 64 };

			int edgesNeeded = (dirtLen/64) + 1;

			auto pWaveform = Waveform::create(WGBP(Waveform,
				_.size = waveformSize,
				_.bottomOutlineThickness = entry.m_bottomOutlineThickness*m_scale,
				_.color = entry.m_fillColor,
				_.gradient = entry.m_fillGradient,
				_.origo = SampleOrigo::Top,
				_.outlineColor = entry.m_outlineColor,
				_.outlineGradient = entry.m_outlineGradient,
				_.topOutlineThickness = entry.m_topOutlineThickness*m_scale
			), m_pEdgemapFactory);


			// Find first sample to deal with

			auto pSample = entry.m_samples.begin();

			while( pSample->timestamp < firstEdgeTimestamp )
				pSample++;

			if( pSample != entry.m_samples.begin() )
				pSample--;

			// Update waveform and generate Edgemap

			_updateWaveformEdge(pWaveform, firstEdgeTimestamp, microsecPerPixel, true, &(*pSample));
			_updateWaveformEdge(pWaveform, firstEdgeTimestamp, microsecPerPixel, false, &(*pSample));

			auto pEdgemap = pWaveform->refresh();

			//

			if( rightEdgeOfs - dirtLen < 0 )
			{
				//TODO: fix clip rectangles so we don't overwrite what we shouldn't if sections are too close to each other.

				spx pos1 = rightEdgeOfs - dirtLen - pixelMargin*64;
				spx pos2 = canvasSize.w + pos1;

				pDevice->drawEdgemap({pos1,0}, pEdgemap);

				pDevice->drawEdgemap({pos2,0}, pEdgemap);
			}
			else
			{
				pDevice->drawEdgemap({rightEdgeOfs - dirtLen - pixelMargin*64, 0 }, pEdgemap);
			}
		}
	}

	//____ _startedOrEndedTransition() ________________________________________

	void AreaScrollChart::_startedOrEndedTransition()
	{
		bool bTransitioning = false;

		for (auto& entry : entries)
		{
			if( entry.m_pColorTransition)
			{
				bTransitioning = true;
				break;
			}
		}

		if (bTransitioning != m_bTransitioning)
		{
			if (bTransitioning)
				_startReceiveUpdates();
			else
				_stopReceiveUpdates();

			m_bTransitioning = bTransitioning;
		}

	}

	//____ _entryVisibilityChanged() __________________________________________

	void AreaScrollChart::_entryVisibilityChanged(AreaScrollChartEntry* pAreaChartEntry)
	{
		_requestFullRedraw();
	}

	//____ _removeOutdatedSamples() ______________________________________________

	void AreaScrollChart::_removeOutdatedSamples()
	{
		int64_t	timestampStillDisplayed = m_latestTimestamp - m_maxDisplayTime - m_latency;

		SizeSPX canvasSize = _canvasSize();

		int microsecPerPixel = canvasSize.w > 0 ? m_displayTime / (canvasSize.w / 64) : 0;

		for( auto& entry : entries )
		{
			if( entry.m_samples.size() < 2 )
				continue;

			pts maxThickness = std::max(entry.m_topOutlineThickness,entry.m_bottomOutlineThickness);

			int pixelMargin = (((maxThickness * m_scale) / 2) + 63) / 64;
			int64_t timestampStillNeeded = timestampStillDisplayed - pixelMargin*microsecPerPixel;

			auto beg = entry.m_samples.begin();
			auto end = entry.m_samples.end();
			auto it = beg;


			while( it != end && it->timestamp < timestampStillNeeded )
				it++;

			if( it != beg )
				it--;									// We should keep one sample of lower value.

			if( it != beg )
			{
				entry.m_samples.erase( beg, it );
			}
		}
	}

	//____ _initEntrySamples() ________________________________________________

	void AreaScrollChart::_initEntrySamples(AreaScrollChartEntry* pEntry)
	{
		AreaScrollChartEntry::SampleSet spl;
		spl.timestamp = m_latestTimestamp - 1000000000;		// We want a very low value but nothing that will overflow 32-bit calculations on time diff.
		spl.samples[0] = pEntry->m_defaultTopSample;
		spl.samples[1] = pEntry->m_defaultBottomSample;

		pEntry->m_samples.push_back(spl);

		spl.timestamp = m_latestTimestamp;
		pEntry->m_samples.push_back(spl);
	}


	//____ AreaScrollChartEntry::constructor ___________________________________

	AreaScrollChartEntry::AreaScrollChartEntry()
	{
	}

	AreaScrollChartEntry::AreaScrollChartEntry(const AreaScrollChartEntry::Blueprint& bp)
	{
		m_id						= bp.id;
		m_bottomOutlineThickness 	= bp.bottomOutlineThickness;
		m_fillColor					= bp.color;
		m_fillGradient				= bp.gradient;
		m_outlineColor				= bp.outlineColor;
		m_outlineGradient			= bp.outlineGradient;
		m_topOutlineThickness		= bp.topOutlineThickness;
		m_bVisible					= bp.visible;

		m_defaultBottomSample		= bp.defaultBottomSample;
		m_defaultTopSample			= bp.defaultTopSample;

		m_fetcher					= bp.fetcher;
	}

	//____ AreaScrollChartEntry::addNowSample() ______________________________

	bool AreaScrollChartEntry::addNowSample( float topSample, float bottomSample )
	{
		if( m_samples.empty() )
			m_pDisplay->_initEntrySamples(this);

		auto now = m_pDisplay->m_latestTimestamp;

		if( now <= m_samples.back().timestamp )
			return false;

		m_samples.push_back( {now, topSample, bottomSample} );

		return true;
	}

	//____ AreaScrollChartEntry::addSampleWithTimestamp() ___________________________

	bool AreaScrollChartEntry::addSampleWithTimestamp( int64_t timestamp, float topSample, float bottomSample )
	{
		if( m_samples.empty() )
			m_pDisplay->_initEntrySamples(this);

		if( timestamp <= m_samples.back().timestamp )
			return false;

		m_samples.push_back( {timestamp, topSample, bottomSample} );
		return true;
	}

	//____ AreaScrollChartEntry::addSamples() _________________________________

	void AreaScrollChartEntry::addSamples(int nbSamples, float sampleRate, const float* pTopSamples, const float * pBottomSamples, float rateTweak )
	{
        if(m_samples.empty())
            m_pDisplay->_initEntrySamples(this);

        float usPerSample = kMicrosecondsPerSecond / sampleRate;
        const int64_t timestamp = m_samples.back().timestamp; // Last sample currently.

        // -------------------------------------------------------------
        //                  --- PID REGULATION ---
        // -------------------------------------------------------------

        // Control signal
        const int64_t expectedEndTime = timestamp + (nbSamples * usPerSample);

        // Proportional Error
        const float timeError = static_cast<float>(expectedEndTime - m_pDisplay->m_latestTimestamp);

        // If we are more then 1 second ahead. Just stop adding new data.
        // This means we loose data, but this only happens during startup and preset changes while streaming.
        if(timeError < -kMicrosecondsPerSecond)
            return;

        // PID constants
        const float Kp = kPidProportionalScale / (nbSamples * usPerSample);
        const float Ki = kPidIntegralScale / (nbSamples * usPerSample);

        // Integral error
        m_errorIntegral = m_errorIntegral * kIntegralDecay + (1.0f-kIntegralDecay) * timeError;
        limit(m_errorIntegral, -1.0f/Kp, 1.0f/Kp);

        float adjustment = (Kp * timeError + Ki * m_errorIntegral);

        // PID Output
        usPerSample *= (1.0f - adjustment);
        limit(usPerSample, (kMicrosecondsPerSecond / (sampleRate * kMaxSpeedupFactor)), (kMicrosecondsPerSecond / (sampleRate * kMaxSlowdownFactor)));

        // Fill in the samples
		int offset = (int) m_samples.size();
		m_samples.resize(offset + nbSamples);

		for( int i = 0 ; i < nbSamples ; i++ )
			m_samples[offset+i].timestamp = timestamp + int64_t((i+1)*usPerSample);

		if( pTopSamples && pBottomSamples )
		{
			for( int i = 0 ; i < nbSamples ; i++ )
			{
				m_samples[offset+i].samples[0] = * pTopSamples++;
				m_samples[offset+i].samples[1] = * pBottomSamples++;
			}
		}
		else if( pTopSamples )
		{
			for( int i = 0 ; i < nbSamples ; i++ )
			{
				m_samples[offset+i].samples[0] = * pTopSamples++;
				m_samples[offset+i].samples[1] = m_defaultBottomSample;
			}
		}
		else if( pBottomSamples )
		{
			for( int i = 0 ; i < nbSamples ; i++ )
			{
				m_samples[offset+i].samples[0] = m_defaultTopSample;
				m_samples[offset+i].samples[1] = * pBottomSamples++;
			}
		}
		else
		{
			for( int i = 0 ; i < nbSamples ; i++ )
			{
				m_samples[offset+i].samples[0] = m_defaultTopSample;
				m_samples[offset+i].samples[1] = m_defaultBottomSample;
			}
		}
	}

	//____ AreaScrollChartEntry::setDefaultSample() _________________________________

	void AreaScrollChartEntry::setDefaultSample(float topSample, float bottomSample)
	{
		m_defaultTopSample = topSample;
		m_defaultBottomSample = bottomSample;
	}

	//____ AreaScrollChartEntry::clearSamples() ___________________________________

	void AreaScrollChartEntry::clearSamples()
	{
		m_samples.clear();

		if( m_bVisible )
			m_pDisplay->_requestFullRedraw();
	}


	//____ AreaScrollChartEntry::setColors() _________________________________

	bool AreaScrollChartEntry::setColors(HiColor fill, HiColor outline, ColorTransition* pTransition)
	{
		if (!fill.isValid() || !outline.isValid())
			return false;

		if (!m_fillGradient.isUndefined() || !m_outlineGradient.isUndefined())
		{
			m_fillGradient = Gradient::Undefined;
			m_outlineGradient = Gradient::Undefined;

			m_pDisplay->_requestFullRedraw();
		}
		else if( pTransition && pTransition == m_pColorTransition && fill == m_endFillColor && outline == m_endOutlineColor )
			return false;	// We ignore re-setting of same transition. Return false since result will differ form request.

		if (fill == m_fillColor && outline == m_outlineColor)
			return true;

		if (pTransition)
		{
			m_pColorTransition = pTransition;
			m_colorTransitionProgress = 0;

			m_endFillColor = fill;
			m_endOutlineColor = outline;

			m_startFillColor = m_fillColor;
			m_startOutlineColor = m_outlineColor;

			m_pDisplay->_startedOrEndedTransition();
		}
		else
		{
			if( m_pColorTransition )
				_endColorTransition();

			m_fillColor = fill;
			m_outlineColor = outline;

			m_pDisplay->_requestFullRedraw();
		}

		return true;
	}

	//____ AreaScrollChartEntry::setGradients() _________________________________

	bool AreaScrollChartEntry::setGradients(Gradient fill, Gradient outline, ColorTransition* pTransition)
	{
		// Gradients in scroll chart may only be vertical, not the least horizontal.

		if (!fill.isValid() || !outline.isValid() || fill.isHorizontal() || outline.isHorizontal() )
			return false;

		if (fill == m_fillGradient && outline == m_outlineGradient)
			return true;

		if (pTransition)
		{
			if( pTransition == m_pColorTransition && fill == m_endFillGradient && outline == m_endOutlineGradient )
				return false;	// We ignore re-setting of same transition. Return false since result will differ form request.

			m_pColorTransition = pTransition;
			m_colorTransitionProgress = 0;

			m_endFillGradient = fill;
			m_endOutlineGradient = outline;

			m_startFillGradient = m_fillGradient;
			m_startOutlineGradient = m_outlineGradient;

			m_pDisplay->_startedOrEndedTransition();
		}
		else
		{
			if( m_pColorTransition )
				_endColorTransition();

			m_fillGradient = fill;
			m_outlineGradient = outline;

			m_pDisplay->_requestFullRedraw();
		}

		return true;
	}

	//____ AreaScrollChartEntry::setOutlineThickness() ___________________________________

	bool AreaScrollChartEntry::setOutlineThickness( pts topOutline, pts bottomOutline )
	{
		if( topOutline == m_topOutlineThickness && bottomOutline == m_bottomOutlineThickness )
			return true;

		m_topOutlineThickness = topOutline;
		m_bottomOutlineThickness = bottomOutline;

		m_pDisplay->_requestFullRedraw();
		return true;
	}

	//____ AreaScrollChartEntry::latestSampleTimestamp() _________________________________

	int64_t AreaScrollChartEntry::latestSampleTimestamp() const
	{
		if( m_samples.empty() )
			return 0;

		return m_samples.back().timestamp;
	}

	//____ AreaScrollChartEntry::setVisible() _________________________________

	void AreaScrollChartEntry::setVisible(bool bVisible)
	{
		if (bVisible != m_bVisible)
		{
			m_bVisible = bVisible;
			m_pDisplay->_entryVisibilityChanged(this);
		}
	}

	//____ AreaScrollChartEntry::_endColorTransition() ______________________________________________

	void AreaScrollChartEntry::_endColorTransition()
	{
		m_pColorTransition = nullptr;

		m_startFillColor = m_fillColor;
		m_endFillColor = m_fillColor;

		m_startOutlineColor = m_outlineColor;
		m_endOutlineColor = m_outlineColor;

		m_pDisplay->_startedOrEndedTransition();
	}


} // namespace wg
