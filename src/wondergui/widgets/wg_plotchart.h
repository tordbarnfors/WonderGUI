/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI PlotChartEntryics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI PlotChartEntryics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI PlotChartEntryics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef	WG_PLOTCHART_DOT_H
#define WG_PLOTCHART_DOT_H
#pragma once

#include <wg_chart.h>

namespace wg
{
	class PlotChart;

	class PlotChartEntry
	{
		friend class PlotChart;
	public:

		struct Blueprint
		{
			HiColor				color = HiColor::White;
			Tintmap_p			tintmap;

			HiColor				outlineColor = HiColor::Black;
			pts					outlineThickness = 1;

			pts					radius = 1;

			bool				visible = true;
		};

		PlotChartEntry() {}
		PlotChartEntry(const Blueprint& bp);

		bool	setTintmap(Tintmap * pTintmap, ColorTransition* pTransition = nullptr);

		bool	setColors(HiColor fill, HiColor outline, ColorTransition* pTransition = nullptr);

		bool	setSize(pts radius, pts outlineThickness, ValueTransition * pTransition = nullptr);

		void	setSamples(int nSamples, CoordF* pSamples);

		bool	transitionSamples(CoordTransition* pTransition, int nSamples, CoordF* pNewSamples);

		void	setVisible(bool bVisible);
		bool	isVisible() const { return m_bVisible; }

		bool	isTransitioningColors() const { return m_pColorTransition; }
		bool	isTransitioningSamples() const { return m_pSampleTransition; }
		bool	isTransitioningSize() const { return m_pSizeTransition; }
		bool	isTransitioningTintmap() const { return m_pSampleTransition; }

		HiColor	color() const { return m_fillColor; }
		HiColor	outlineColor() const { return m_outlineColor; }

		std::tuple<int, const CoordF*>	samples() const;

	protected:

		void				_endSampleTransition();
		void				_endColorTransition();
		void				_endSizeTransition();
		void				_endTintmapTransition();


		PlotChart*			m_pDisplay = nullptr;

		// Appearance

		bool				m_bVisible = true;

		HiColor				m_fillColor = Color::White;
		HiColor				m_outlineColor = Color::Black;

		Tintmap_p			m_pTintmap;

		pts					m_outlineThickness = 1;
		pts					m_radius = 1;

		Surface_p			m_sprite;

		// Size transitions

		ValueTransition_p	m_pSizeTransition;
		int					m_sizeTransitionProgress = 0;	// In microseconds

		pts					m_startRadius = 0;
		pts					m_endRadius = 0;

		pts					m_startOutlineThickness = 0;
		pts					m_endOutlineThickness = 0;

		// Color transitions

		ColorTransition_p	m_pColorTransition;
		int					m_colorTransitionProgress = 0;

		HiColor				m_startFillColor;
		HiColor				m_endFillColor;

		HiColor				m_startOutlineColor;
		HiColor				m_endOutlineColor;

		// Tint gradient transitions

		ColorTransition_p	m_pTintmapTransition;
		int					m_tintmapTransitionProgress = 0;

		Tintmap_p			m_pStartTintmap;
		Tintmap_p			m_pEndTintmap;

		// Sample transitions

		CoordTransition_p	m_pSampleTransition;
		int					m_sampleTransitionProgress = 0;	// In microseconds

		std::vector<CoordF>	m_startSamples;
		std::vector<CoordF>	m_endSamples;

		// PlotChartEntry samples

		std::vector<CoordF>	m_samples;				// Samples
	};



	class PlotChart;
	typedef	StrongPtr<PlotChart>	PlotChart_p;
	typedef	WeakPtr<PlotChart>		PlotChart_wp;

	class PlotChart : public Chart, private DynamicVector<PlotChartEntry>::Holder
	{
		friend class PlotChartEntry;
	public:

		struct Blueprint
		{
			Object_p		baggage;

			Placement		bottomLabelPlacement = Placement::South;
			pts				bottomLabelSpacing = 1;

			float			displayCeiling = 0.f;
			float			displayFloor = 1.f;
			Skin_p			displaySkin;

			bool			dropTarget = false;
			bool			disabled = false;
			Finalizer_p		finalizer = nullptr;

			Glow::Blueprint	glow;

			HiColor			gridColor = Color::DarkGray;
			pts				gridThickness = 1;

			int				id = 0;
			Skin_p			labelSkin;

			Placement		leftLabelPlacement = Placement::West;
			pts				leftLabelSpacing = 4;

			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;

			Placement		rightLabelPlacement = Placement::East;
			pts				rightLabelSpacing = 4;

			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;

			TextLayout_p	textLayout;
			TextStyle_p		textStyle;

			String			tooltip;

			Placement		topLabelPlacement = Placement::North;
			pts				topLabelSpacing = 1;
		};

		//.____ Creation __________________________________________

		static PlotChart_p	create( const Blueprint& blueprint ) { return PlotChart_p(new PlotChart(blueprint)); }


		//.____ Components _______________________________________

		DynamicVector<PlotChartEntry>	entries;

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		PlotChart();
		
		template<class BP> PlotChart( const BP& bp ) : Chart(bp), entries(this)
		{
		}

		virtual ~PlotChart();

		void		_fullRefreshOfChart() override;
		void		_renderCharts(GfxDevice* pDevice, const RectSPX& canvas) override;

		void		_startedOrEndedTransition();

		void		_didAddEntries(PlotChartEntry* pEntry, int nb) override;
		void		_didMoveEntries(PlotChartEntry* pFrom, PlotChartEntry* pTo, int nb) override;
		void		_willEraseEntries(PlotChartEntry* pEntry, int nb) override;

		void		_update(int microPassed, int64_t microsecTimestamp) override;


		//

	private:
		bool			m_bTransitioning = false;
	};




} // namespace wg
#endif //WG_PLOTCHART_DOT_H
