/*=========================================================================

                         >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI AreaChartEntryics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

                            -----------

  The WonderGUI AreaChartEntryics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                            -----------

  The WonderGUI AreaChartEntryics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG_SCROLLCHART_DOT_H
#define WG_SCROLLCHART_DOT_H
#pragma once

#include <wg_chart.h>
#include <wg_waveform.h>

namespace wg
{
 
	class ScrollChart;
	typedef StrongPtr<ScrollChart> ScrollChart_p;
	typedef WeakPtr<ScrollChart> ScrollChart_wp;

	class ScrollChart: public Chart
	{
		friend class ScrollChartEntry;

	public:
		struct Blueprint
		{
			Object_p		baggage;

			Placement		bottomLabelPlacement = Placement::South;
			pts				bottomLabelSpacing = 1;

			float			displayCeiling = 1.f;
			float			displayFloor = 0.f;
			Skin_p			displaySkin;

			bool			dropTarget = false;
			bool			disabled = false;
			Finalizer_p		finalizer = nullptr;

			GfxFlip			flip = GfxFlip::None;

			Glow::Blueprint glow;

			HiColor			gridColor = Color::DarkGray;
			pts				gridThickness = 1;

			int				id = 0;
			Skin_p			labelSkin;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			int				maxDisplayTime = 0;    // microsec of samples saved for changes in displayTime. 0 = st to same as displayTime.
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;

			Placement		sideLabelPlacement = Placement::West;
			pts				sideLabelSpacing = 4;

			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;

			TextLayout_p	textLayout;
			TextStyle_p		textStyle;

			String			tooltip;

			int				displayTime = 4000000;    // microsec of samples displayed.
			int				latency = 10000;
		};

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo TYPEINFO;

		//.____ Appearance ____________________________________________________

		void	setFlip( GfxFlip flip );
		GfxFlip flip() const { return m_flip; }

		//.____ Control _______________________________________________________

		void	start();
		void	stop();

		void	setDisplayTime(int microsec);
		int		displayTime() const { return m_displayTime;  }


	protected:
		ScrollChart();

		template<class BP>
		ScrollChart(const BP& bp): Chart(bp)
		{
			m_displayTime = bp.displayTime;
			m_maxDisplayTime = bp.maxDisplayTime > 0 ? bp.maxDisplayTime : bp.displayTime;

			m_latency = bp.latency;

			m_flip = bp.flip;
			m_bAxisSwapped = ( bp.flip == GfxFlip::Rot90 || bp.flip == GfxFlip::Rot90FlipX || bp.flip == GfxFlip::Rot90FlipY ||
							  bp.flip == GfxFlip::Rot270 || bp.flip == GfxFlip::Rot270FlipX || bp.flip == GfxFlip::Rot270FlipY );
		}

		virtual ~ScrollChart();

		void		_fullRefreshOfChart() override;

		void		_update(int microPassed, int64_t microsecTimestamp) override;
		void		_preRender() override;

		void		_renderCharts(GfxDevice* pDevice, const RectSPX& canvas) override;

		virtual void _renderOnScrollSurface( GfxDevice * pDevice, SizeSPX canvasSize, spx rightEdgeOfs, int64_t rightEdgeTimestamp, spx dirtLen ) = 0;
		virtual void	_removeOutdatedSamples() = 0;

		void		_requestFullRedraw();

		// Use instead of m_chartCanvas.size(). Takes rotation into account.

		SizeSPX		_canvasSize() const { return m_bAxisSwapped ? SizeSPX( m_chartCanvas.h, m_chartCanvas.w ) : SizeSPX( m_chartCanvas.w, m_chartCanvas.h ); }

		//

	protected:
		int			m_displayTime = 4000000;
		int			m_maxDisplayTime = 4000000;
		int64_t		m_latestTimestamp = 0;
		int			m_latency = 10000;

	private:
		Surface_p	m_pScrollSurface;

		int64_t		m_rightEdgeTimestamp = 0;
		spx			m_rightEdgeOfs = 0;
		spx			m_dirtLen = 0;

		PixelFormat	m_scrollSurfaceFormat = PixelFormat::BGRA_8;
		HiColor		m_scrollSurfaceBgColor = HiColor::Transparent;

		GfxFlip		m_flip = GfxFlip::None;
		bool		m_bAxisSwapped = false;				// Set if flip results in X and Y being swapped.

		bool		m_bScrolling = false;

		bool		m_bFullRedrawRequested = false;
		bool		m_bPreRenderRequested = false;
	};

} // namespace wg
#endif // WG_SCROLLCHART_DOT_H
