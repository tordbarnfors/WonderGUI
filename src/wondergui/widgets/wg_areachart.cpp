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
#include	<wg_areachart.h>
#include	<wg_gfxdevice.h>
#include	<wg_base.h>

#include	<cstring>

namespace wg
{

	const TypeInfo AreaChart::TYPEINFO = { "AreaChart", &Widget::TYPEINFO };


	//____ constructor ____________________________________________________________

	AreaChart::AreaChart() : entries(this)
	{
	}

	//____ destructor _____________________________________________________________

	AreaChart::~AreaChart()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& AreaChart::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setFlip() _____________________________________________________________

	void AreaChart::setFlip( GfxFlip flip )
	{
		if( flip != m_flip )
		{
			m_flip = flip;
			m_bAxisSwapped = ( flip == GfxFlip::Rot90 || flip == GfxFlip::Rot90FlipX || flip == GfxFlip::Rot90FlipY ||
							  flip == GfxFlip::Rot270 || flip == GfxFlip::Rot270FlipX || flip == GfxFlip::Rot270FlipY );

			_fullRefreshOfChart();
		}
	}

	//____ _startedOrEndedTransition() __________________________________________

	void AreaChart::_startedOrEndedTransition()
	{
		bool bTransitioning = false;

		for(auto& graph : entries)
		{
			if (graph.m_pSampleTransition || graph.m_pColorTransition)
			{
				bTransitioning = true;
				break;
			}
		}

		if( bTransitioning != m_bTransitioning)
		{
			if (bTransitioning)
				_startReceiveUpdates();
			else
				_stopReceiveUpdates();

			m_bTransitioning = bTransitioning;
		}
	}

	//____ _entryVisibilityChanged() _________________________________________

	void AreaChart::_entryVisibilityChanged(AreaChartEntry* pAreaChartEntry)
	{
		// We need to render the area no matter if graph appeared or disappeared.

		_requestRenderAreaChartEntry(pAreaChartEntry, pAreaChartEntry->m_begin, pAreaChartEntry->m_end);

		// If we turned visible we might have a waveform that needs refresh.
		// (we don't perform refresh while hidden)

		if (pAreaChartEntry->m_bVisible && (pAreaChartEntry->m_bSamplesChanged || pAreaChartEntry->m_bColorsChanged))
			_waveformNeedsRefresh(pAreaChartEntry, false, pAreaChartEntry->m_bSamplesChanged, pAreaChartEntry->m_bColorsChanged);
	}


	//____ _setAreaChartEntryRange() __________________________________________________

	void AreaChart::_setAreaChartEntryRange(AreaChartEntry* pAreaChartEntry, float begin, float end)
	{
		float leftmost = std::min(begin, pAreaChartEntry->m_begin);
		float rightmost = std::max(end, pAreaChartEntry->m_end);

		pAreaChartEntry->m_begin = begin;
		pAreaChartEntry->m_end = end;

		_requestRenderAreaChartEntry( pAreaChartEntry, leftmost, rightmost );
		_waveformNeedsRefresh(pAreaChartEntry, true, false, false);
	}

	//____ _didAddEntries() ___________________________________________________

	void AreaChart::_didAddEntries(AreaChartEntry* pEntry, int nb)
	{
		for (int i = 0; i < nb; i++)
		{
			pEntry->m_pDisplay = this;
			_waveformNeedsRefresh(pEntry + i, true, true, true);
		}
	}

	//____ _didMoveEntries() ___________________________________________________

	void AreaChart::_didMoveEntries(AreaChartEntry* pFrom, AreaChartEntry* pTo, int nb)
	{
		for (int i = 0; i < nb; i++)
		{
			AreaChartEntry* p = pTo + i;
			_requestRenderAreaChartEntry(p, p->m_begin, p->m_end);
		}

	}

	//____ _willEraseEntries() ___________________________________________________

	void AreaChart::_willEraseEntries(AreaChartEntry* pEntry, int nb)
	{
		for (int i = 0; i < nb; i++)
		{
			AreaChartEntry* p = pEntry + i;
			_requestRenderAreaChartEntry(p, p->m_begin, p->m_end);
		}
	}

	//____ _requestRenderAreaChartEntry() ______________________________________________

	void AreaChart::_requestRenderAreaChartEntry(AreaChartEntry* pAreaChartEntry, float leftmost, float rightmost)
	{
		//TODO: I suspect this gets wrong if Chart is flipped so it is mirrored and chart has non-standard leftmost/rightmost.

		RectSPX dirt;

		if( m_bAxisSwapped )
		{
			dirt.x = m_chartCanvas.x;
			dirt.y = m_chartCanvas.y + m_chartCanvas.h * leftmost;
			dirt.w = m_chartCanvas.w;
			dirt.h = m_chartCanvas.h * (rightmost - leftmost);
		}
		else
		{
			dirt.x = m_chartCanvas.x + m_chartCanvas.w * leftmost;
			dirt.y = m_chartCanvas.y;
			dirt.w = m_chartCanvas.w * (rightmost - leftmost);
			dirt.h = m_chartCanvas.h;
		}

		pts outlineThickness = std::max(pAreaChartEntry->m_topOutlineThickness, pAreaChartEntry->m_bottomOutlineThickness);

		spx outlineMargin = (Util::ptsToSpx(outlineThickness, m_scale) / 2);

		dirt += BorderSPX(outlineMargin+64);

		_requestRender(RectSPX::overlap(Util::alignUp(dirt), m_chartCanvas) );
	}

	//____ _waveformNeedsRefresh() ____________________________________________

	void AreaChart::_waveformNeedsRefresh(AreaChartEntry * pAreaChartEntry, bool bGeo, bool bSamples, bool bColors)
	{
		if (bGeo)
		{
			pAreaChartEntry->m_pWaveform = nullptr;
		}
		else
		{
			if (bSamples)
				pAreaChartEntry->m_bSamplesChanged = true;

			if (bColors)
				pAreaChartEntry->m_bColorsChanged = true;
		}

		if (pAreaChartEntry->m_bVisible && !m_bPreRenderRequested)
			m_bPreRenderRequested = _requestPreRenderCall();
	}

	//____ _fullRefreshOfChart() _______________________________________

	void AreaChart::_fullRefreshOfChart()
	{
		for (auto& graph : entries)
			graph.m_pWaveform = nullptr;

		if (!m_bPreRenderRequested)
			m_bPreRenderRequested = _requestPreRenderCall();
	}

	//____ _renderCharts() ____________________________________________________

	void AreaChart::_renderCharts(GfxDevice* pDevice, const RectSPX& canvas)
	{
		auto popData = Util::limitClipList(pDevice, canvas);

		auto p = entries.end();
		while( p > entries.begin() )
		{
			p--;
			if (p->m_bVisible && p->m_pEdgemap)
				pDevice->flipDrawEdgemap(canvas + p->m_waveformPos, p->m_pEdgemap, m_flip );
		}

		Util::popClipList(pDevice, popData);
	}

	//____ _update() ____________________________________________________________

	void AreaChart::_update(int microPassed, int64_t microsecTimestamp)
	{
		Chart::_update(microPassed, microsecTimestamp);

		if (!m_bTransitioning)
			return;

		bool	transitionsActive = false;

		// Update all transitions

		for (auto& graph : entries)
		{
			if (graph.m_pSampleTransition)
			{
				int timestamp = graph.m_sampleTransitionProgress + microPassed;

				if( timestamp >= graph.m_pSampleTransition->duration() )
				{
					graph.m_sampleTransitionProgress = 0;
					graph.m_pSampleTransition = nullptr;
					graph.m_topSamples.swap( graph.m_endTopSamples );
					graph.m_bottomSamples.swap( graph.m_endBottomSamples );
				}
				else
				{
					graph.m_sampleTransitionProgress = timestamp;

					graph.m_pSampleTransition->snapshot(timestamp, (int) graph.m_topSamples.size(),
														graph.m_startTopSamples.data(),
														graph.m_endTopSamples.data(),
														graph.m_topSamples.data());

					graph.m_pSampleTransition->snapshot(timestamp, (int) graph.m_bottomSamples.size(),
														graph.m_startBottomSamples.data(),
														graph.m_endBottomSamples.data(),
														graph.m_bottomSamples.data());

					transitionsActive = true;
				}

				_waveformNeedsRefresh(&graph, false, true, false);
			}

			if (graph.m_pColorTransition)
			{
				int timestamp = graph.m_colorTransitionProgress + microPassed;

				if (timestamp >= graph.m_pColorTransition->duration())
				{
					graph.m_colorTransitionProgress = 0;
					graph.m_pColorTransition = nullptr;

					graph.m_fillColor = graph.m_endFillColor;
					graph.m_outlineColor = graph.m_endOutlineColor;

					if (!graph.m_fillGradient.isUndefined())
					{
						graph.m_fillGradient = graph.m_endFillGradient;
						graph.m_outlineGradient = graph.m_endOutlineGradient;
					}
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

				_waveformNeedsRefresh(&graph, false, false, true);
			}
		}

		if (!transitionsActive)
		{
			m_bTransitioning = false;
			_stopReceiveUpdates();
		}
	}

	//____ _updateAreaChartEntrys() ______________________________________________________

	bool AreaChart::_updateAreaChartEntrys()
	{
		bool bSamplesChanged = false;

		for( int entry = 0 ; entry < entries.size() ; entry++ )
		{
			auto&  graph = entries[entry];

			if (graph.m_bVisible)
			{
				bool bNeedsFullRendering = false;
				spx displayHeight = m_bAxisSwapped ? m_chartCanvas.w : m_chartCanvas.h;

				if (!graph.m_pWaveform)
				{
					// Generate waveform

					RectSPX rect = _entryRangeToRect(graph.m_begin, graph.m_end, m_bAxisSwapped);

					if( !rect.isEmpty() )
					{
						graph.m_waveformPos = rect.pos();

						graph.m_pWaveform = Waveform::create(WGBP(Waveform,
							_.size = rect.size()/64,
							_.bottomOutlineThickness = graph.m_bottomOutlineThickness*m_scale,
							_.color = graph.m_fillColor,
							_.gradient = graph.m_fillGradient,
							_.origo = SampleOrigo::Top,
							_.outlineColor = graph.m_outlineColor,
							_.outlineGradient = graph.m_outlineGradient,
							_.topOutlineThickness = graph.m_topOutlineThickness*m_scale
						), m_pEdgemapFactory);

						// Interpolate and set samples

						_updateWaveformEdge(entry, graph.m_pWaveform, true, (int) graph.m_topSamples.size(), graph.m_topSamples.data(), m_bAxisSwapped );
						_updateWaveformEdge(entry, graph.m_pWaveform, false, (int) graph.m_bottomSamples.size(), graph.m_bottomSamples.data(), m_bAxisSwapped );

						graph.m_pEdgemap = graph.m_pWaveform->refresh();
						graph.m_bSamplesChanged = true;						// So _preRender() will understand.

						bSamplesChanged = true;
						bNeedsFullRendering = true;
					}
				}
				else
				{
					if (graph.m_bColorsChanged)
					{
						auto pWF = graph.m_pWaveform;

						pWF->setColor(graph.m_fillColor);
						pWF->setOutlineColor(graph.m_outlineColor);

						if (graph.m_fillGradient.isUndefined())
							pWF->clearGradient();
						else
							pWF->setGradient(graph.m_fillGradient);

						if (graph.m_outlineGradient.isUndefined())
							pWF->clearOutlineGradient();
						else
							pWF->setOutlineGradient(graph.m_outlineGradient);

						graph.m_bColorsChanged = false;

						bNeedsFullRendering = true;
					}

					if (graph.m_bSamplesChanged)
					{
						_updateWaveformEdge(entry, graph.m_pWaveform, true, (int) graph.m_topSamples.size(), graph.m_topSamples.data(), m_bAxisSwapped );
						_updateWaveformEdge(entry, graph.m_pWaveform, false, (int) graph.m_bottomSamples.size(), graph.m_bottomSamples.data(), m_bAxisSwapped );

						bSamplesChanged = true;
					}
				}
				if (bNeedsFullRendering )
					_requestRenderAreaChartEntry(&graph, graph.m_begin, graph.m_end);
			}
		}
		return bSamplesChanged;
	}

	//____ _updateWaveformEdge() _________________________________________________

	void AreaChart::_updateWaveformEdge(int entry, Waveform* pWaveform, bool bTopEdge, int nSamples, float* pSamples, bool bAxisSwapped )
	{
		// TODO: Better interpolation, especially when shrinking.

		int wfSamples = pWaveform->nbSamples();

		int height = bAxisSwapped ? m_chartCanvas.w : m_chartCanvas.h;

		if (nSamples <= 1)
		{
			float sample = nSamples == 0 ? 0.f : pSamples[0];

			spx spxSample = (sample - m_displayCeiling) / (m_displayFloor - m_displayCeiling) * height;

			if (bTopEdge)
				pWaveform->setFlatTopLine(0, wfSamples, spxSample);
			else
				pWaveform->setFlatBottomLine(0, wfSamples, spxSample);
		}
		else
		{
			spx * pConverted = (spx*) Base::memStackAlloc(wfSamples * sizeof(spx));

			float valueFactor = height / (m_displayFloor - m_displayCeiling);

			bool bResampled = false;

			// If we have a resampler we try with that first

			if( m_pResampler )
			{
				float * pIntermediate = (float*) Base::memStackAlloc(wfSamples * sizeof(float));

				bResampled = m_pResampler( entry, bTopEdge, wfSamples, pIntermediate, nSamples, pSamples );

				if( bResampled )
				{
					// Convert intermediate samples to spx in correct range.

					for (int i = 0; i < wfSamples; i++)
						pConverted[i] = (pIntermediate[i] - m_displayCeiling) * valueFactor;
				}

				Base::memStackFree(wfSamples * sizeof(float));
			}

			// Fall back to default resampler

			if( !bResampled )
			{
//				if( nSamples <= wfSamples )
				{
					float stepFactor = (nSamples - 1) / (float) wfSamples;

					for (int i = 0; i < wfSamples; i++)
					{
						float sample = stepFactor * i;
						int ofs = (int)sample;
						float frac2 = sample - ofs;
						float frac1 = 1.f - frac2;

						float interpolated = pSamples[ofs] * frac1 + pSamples[ofs+1] * frac2;

						pConverted[i] = int((interpolated - m_displayCeiling) * valueFactor);
					}

					if( m_bPreservePeaks )
					{
						for( int i = 1 ; i < nSamples-1 ; i++ )
						{
							float sample = pSamples[i];
							float neighbour1 = pSamples[i-1];
							float neighbour2 = pSamples[i+1];
							if( neighbour1 > neighbour2 )
								std::swap( neighbour1, neighbour2 );

							if( sample < neighbour1 || sample > neighbour2 )
							{
								int x = int((wfSamples*i/(float)(nSamples-1))+0.5f);
								pConverted[x] = int((sample - m_displayCeiling) * valueFactor);
							}
						}
					}
				}
//				else
//				{
//
//				}

			}

			//

			if (bTopEdge)
				pWaveform->setSamples(0, wfSamples, pConverted, nullptr);
			else
				pWaveform->setSamples(0, wfSamples, nullptr, pConverted);

			Base::memStackFree(wfSamples * sizeof(spx));
		}
	}

	//____ _entryRangeToRect() __________________________________

	RectSPX AreaChart::_entryRangeToRect(float begin, float end, bool axisSwapped) const
	{
		RectSPX rect;

		if( axisSwapped )
			rect = RectSPX( m_chartCanvas.h * begin, 0, m_chartCanvas.h * (end - begin), m_chartCanvas.w );
		else
			rect = RectSPX( m_chartCanvas.w * begin, 0, m_chartCanvas.w * (end - begin), m_chartCanvas.h );

		return Util::align(rect);
	}

	//____ _preRender() _______________________________________________________

	void AreaChart::_preRender()
	{
		bool bSamplesChanged = _updateAreaChartEntrys();

		if( bSamplesChanged )
		{

			//TODO: Limited range not tested.

			spx dirtySectionWidth = m_dirtySectionWidth * m_scale;

			int boundsPitch = sizeof(SectionBounds) / sizeof(spx);

			SizeSPX	canvasSize = m_chartCanvas.size();
			if( m_bAxisSwapped )
				std::swap(canvasSize.w,canvasSize.h);

			int nSections = (canvasSize.w + (dirtySectionWidth-1)) / dirtySectionWidth;

			// Prepare dirt spans for sections

			int sectionDirtAlloc = nSections * sizeof( Spans<spx,4,4*64> );
			auto pSectionDirt = (Spans<spx,4,4*64>*) Base::memStackAlloc(sectionDirtAlloc);

			for( int i = 0 ; i < nSections ; i++ )
				pSectionDirt[i].size = 0;

			//

			for (auto& entry : entries)
			{
				if (entry.m_bSamplesChanged && entry.m_bVisible && entry.m_pWaveform )
				{
					auto pEdgemap = entry.m_pWaveform->refresh();
					entry.m_pEdgemap = pEdgemap;

					if( entry.m_sectionBounds.size() == nSections )
					{
						SectionBounds * pSection = &entry.m_sectionBounds.front();
						for( int section = 0 ; section < nSections ; section++ )
						{
							pSectionDirt[section].add( pSection->topBeg & ~63, (pSection->topEnd + 63) & ~63 );
							pSectionDirt[section].add( pSection->bottomBeg & ~63, (pSection->bottomEnd + 63) & ~63 );
							pSection++;
						}

						auto pBuffer = &entry.m_sectionBounds.front();

						pEdgemap->exportBounds(&pBuffer->topBeg, nSections, dirtySectionWidth/64, 0, 1, entry.m_waveformPos.x/64, boundsPitch );
						pEdgemap->exportBounds(&pBuffer->bottomBeg, nSections, dirtySectionWidth/64, 2, 3, entry.m_waveformPos.x/64, boundsPitch );

						pSection = &entry.m_sectionBounds.front();
						for( int section = 0 ; section < nSections ; section++ )
						{
							assert( pSection->topBeg <= pSection->topEnd );
							assert( pSection->bottomBeg <= pSection->bottomEnd );

							pSectionDirt[section].add( pSection->topBeg & ~63, (pSection->topEnd + 63) & ~63 );
							pSectionDirt[section].add( pSection->bottomBeg & ~63, (pSection->bottomEnd + 63) & ~63 );
							pSection++;
						}
					}
					else
					{
						entry.m_sectionBounds.resize(nSections);

						auto pBounds = &entry.m_sectionBounds.front();
						pEdgemap->exportBounds(&pBounds->topBeg, nSections, dirtySectionWidth/64, 0, 1, entry.m_waveformPos.x/64, boundsPitch );
						pEdgemap->exportBounds(&pBounds->bottomBeg, nSections, dirtySectionWidth/64, 2, 3, entry.m_waveformPos.x/64, boundsPitch );

//						for( int section = 0 ; section < nSections ; section++ )
//							_requestRenderEntrySection(&entry, section, pBounds[section].topBeg, pBounds[section].bottomEnd);
					}

					entry.m_bSamplesChanged = false;
				}
			}

			// Request render dirty sections

			int sectionNb = 0;
			for( int sec = 0 ; sec < nSections ; sec++ )
			{
				auto& section = pSectionDirt[sec];
				for( int i = 0 ; i < section.size ; i++ )
				{
						RectSPX dirt;
						dirt.x = sectionNb * dirtySectionWidth;
						dirt.w = dirtySectionWidth;
						dirt.y = section.array[i].begin;
						dirt.h = section.array[i].end - section.array[i].begin;

						if( dirt.x + dirt.w > canvasSize.w )
							dirt.w = canvasSize.w - dirt.x;

						RectSPX r = Util::flipRect(dirt, m_flip, canvasSize) + m_chartCanvas.pos();
						_requestRender(r);
				}

//					_requestRenderSectionSpan(sectionNb, section.array[i].begin, section.array[i].end );

				sectionNb++;
			}

			Base::memStackFree(sectionDirtAlloc);
		}

		m_bPreRenderRequested = false;
	}

	//____ _requestRenderSectionSpan() ___________________________________________________

	void AreaChart::_requestRenderSectionSpan(int section, spx beginY, spx endY)
	{
		int dirtySectionWidth = m_dirtySectionWidth * m_scale;

		RectSPX dirt;
		dirt.x = section * dirtySectionWidth;
		dirt.w = dirtySectionWidth;
		dirt.y = beginY;
		dirt.h = endY - beginY;

		SizeSPX	canvasSize = m_chartCanvas.size();
		if( m_bAxisSwapped )
			std::swap(canvasSize.w,canvasSize.h);

		if( dirt.x + dirt.w > canvasSize.w )
			dirt.w = canvasSize.w - dirt.x;

//		RectSPX r = dirt + m_chartCanvas.pos()
		RectSPX r = Util::flipRect(dirt, m_flip, canvasSize) + m_chartCanvas.pos();
		_requestRender(r);
	}


	//____ AreaChartEntry::AreaChartEntry() _______________________________________________________

	AreaChartEntry::AreaChartEntry(const Blueprint& bp)
	{
		m_id = bp.id;
		m_bottomOutlineThickness = bp.bottomOutlineThickness;
		m_fillColor = bp.color;
		m_fillGradient = bp.gradient;
		m_outlineColor = bp.outlineColor;
		m_outlineGradient = bp.outlineGradient;
		m_begin = bp.rangeBegin;
		m_end = bp.rangeEnd;
		m_topOutlineThickness = bp.topOutlineThickness;
		m_bVisible = bp.visible;
		m_topSamples.push_back(0.f);
		m_bottomSamples.push_back(0.f);
	}

	//____ setColors() ________________________________________________________

	bool AreaChartEntry::setColors(HiColor fill, HiColor outline, ColorTransition* pTransition)
	{
		if( !fill.isValid() || !outline.isValid() )
			return false;

		if (!m_fillGradient.isUndefined() || !m_outlineGradient.isUndefined())
		{
			m_fillGradient = Gradient::Undefined;
			m_outlineGradient = Gradient::Undefined;

			m_pDisplay->_waveformNeedsRefresh(this, false, false, true);
		}
		else if( pTransition && pTransition == m_pColorTransition && fill == m_endFillColor && outline == m_endOutlineColor )
			return false;	// We ignore re-setting of same transition. Return false since result will differ form request.

		if( fill == m_fillColor && outline == m_outlineColor )
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

			m_pDisplay->_waveformNeedsRefresh(this, false, false, true);
		}

		return true;
	}

	//____ setGradients() _____________________________________________________

	bool AreaChartEntry::setGradients(Gradient fill, Gradient outline, ColorTransition* pTransition)
	{
		if( !fill.isValid() || !outline.isValid() )
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

			m_pDisplay->_waveformNeedsRefresh(this, false, false, true);
		}

		return true;
	}

	//____ AreaChartEntry::setOutlineThickness() ___________________________________

	bool AreaChartEntry::setOutlineThickness( pts topOutline, pts bottomOutline )
	{
		if( topOutline == m_topOutlineThickness && bottomOutline == m_bottomOutlineThickness )
			return true;

		m_topOutlineThickness = topOutline;
		m_bottomOutlineThickness = bottomOutline;

		m_pDisplay->_waveformNeedsRefresh(this, true, false, false );
		return true;
	}

	//____ setRange() _________________________________________________________

	void AreaChartEntry::setRange(float begin, float end)
	{
		limit(begin, 0.f, 1.f);
		limit(end, begin, 1.f);

		if( begin == m_begin && end == m_end )
			return;

		m_pDisplay->_setAreaChartEntryRange(this, begin, end);
	}

	//____ setTopSamples() ____________________________________________________

	void AreaChartEntry::setTopSamples(int nSamples, const float* pSamples)
	{
		if (nSamples == 0)
		{
			m_topSamples.resize(1);
			m_topSamples[0] = 0.f;
		}
		else
		{
			m_topSamples.resize(nSamples);
			memcpy(m_topSamples.data(), pSamples, nSamples * sizeof(float));
		}

		_endSampleTransition();

		m_pDisplay->_waveformNeedsRefresh(this, false, true, false);
	}

	//____ setBottomSamples() _________________________________________________

	void AreaChartEntry::setBottomSamples(int nSamples, const float* pSamples)
	{
		if (nSamples == 0)
		{
			m_bottomSamples.resize(1);
			m_bottomSamples[0] = 0.f;
		}
		else
		{
			m_bottomSamples.resize(nSamples);
			memcpy(m_bottomSamples.data(), pSamples, nSamples * sizeof(float));
		}

		_endSampleTransition();
		m_pDisplay->_waveformNeedsRefresh(this, false, true, false);
	}

	//____ transitionSamples() ________________________________________________

	bool AreaChartEntry::transitionSamples(ValueTransition* pTransition, int nTopSamples, const float* pNewTopSamples, int nBottomSamples, const float* pNewBottomSamples)
	{
		if( nTopSamples != m_topSamples.size() || nBottomSamples != m_bottomSamples.size() )
			return false;

		m_pSampleTransition = pTransition;
		m_sampleTransitionProgress = 0;

		m_startBottomSamples.resize(nBottomSamples);
		memcpy(m_startBottomSamples.data(), m_bottomSamples.data(), nBottomSamples * sizeof(float));

		m_endBottomSamples.resize(nBottomSamples);
		memcpy(m_endBottomSamples.data(), pNewBottomSamples, nBottomSamples * sizeof(float));

		m_startTopSamples.resize(nTopSamples);
		memcpy(m_startTopSamples.data(), m_topSamples.data(), nTopSamples * sizeof(float));

		m_endTopSamples.resize(nTopSamples);
		memcpy(m_endTopSamples.data(), pNewTopSamples, nTopSamples * sizeof(float));

		m_pDisplay->_startedOrEndedTransition();

		return true;
	}

	//____ setVisible() _______________________________________________________

	void AreaChartEntry::setVisible(bool bVisible)
	{
		if( bVisible != m_bVisible )
		{
			m_bVisible = bVisible;
			m_pDisplay->_entryVisibilityChanged(this);
		}
	}

	//____ topSamples() _______________________________________________________

	std::tuple<int, const float*>  AreaChartEntry::topSamples() const
	{
		return std::make_tuple((int)m_topSamples.size(), m_topSamples.data());
	}

	//____ bottomSamples() ____________________________________________________

	std::tuple<int, const float*>  AreaChartEntry::bottomSamples() const
	{
		return std::make_tuple((int)m_bottomSamples.size(), m_bottomSamples.data());
	}

	//____ _endSampleTransition() _____________________________________________

	void AreaChartEntry::_endSampleTransition()
	{
		m_pSampleTransition = nullptr;

		m_startBottomSamples.resize(0);
		m_endBottomSamples.resize(0);

		m_startTopSamples.resize(0);
		m_endTopSamples.resize(0);

		m_pDisplay->_startedOrEndedTransition();
	}

	//____ _endColorTransition() ______________________________________________

	void AreaChartEntry::_endColorTransition()
	{
		m_pColorTransition = nullptr;

		m_startFillColor = m_fillColor;
		m_endFillColor = m_fillColor;

		m_startOutlineColor = m_outlineColor;
		m_endOutlineColor = m_outlineColor;

		m_pDisplay->_startedOrEndedTransition();
	}

}
