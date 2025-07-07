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

#ifndef WG_AREASCROLLCHART_DOT_H
#define WG_AREASCROLLCHART_DOT_H
#pragma once

#include <wg_scrollchart.h>
#include <wg_waveform.h>

namespace wg
{
	class AreaScrollChart;

	class AreaScrollChartEntry
	{
		friend class AreaScrollChart;
	public:

		struct Blueprint
		{
			pts					bottomOutlineThickness = 1;
			HiColor				color = Color8::LightGrey;
			float				defaultBottomSample = 0.f;
			float				defaultTopSample = 0.f;
			std::function<void(int64_t latestTimestamp, int64_t firstNeededTimestamp, int64_t lastNeededTimestamp, int64_t currentTimestamp)> fetcher;
			Gradient			gradient;								// Overrides color when set.
			int					id;
			HiColor				outlineColor = Color8::DarkGrey;
			Gradient			outlineGradient;						// Overrides outlineColor when set.
			pts					topOutlineThickness = 1;
			bool				visible = true;
		};

        AreaScrollChartEntry();
		AreaScrollChartEntry(const Blueprint& bp);

		void	setId(int id) { m_id = id; }
		int		id() const { return m_id; }

		bool	addNowSample( float topSample, float bottomSample );
		bool	addSampleWithTimestamp( int64_t timestamp, float topSample, float bottomSample );

		void    addSamples(int nbSamples, float sampleRate, const float* pTopSamples, const float * pBottomSamples, float rateTweak = 0.f );
        void    setDefaultSample(float topSample, float bottomSample);

		void	clearSamples();

		bool	setColors(HiColor fill, HiColor outline, ColorTransition* pTransition = nullptr);
		bool	setGradients(Gradient fill, Gradient outline, ColorTransition* pTransition = nullptr);

		bool	setOutlineThickness( pts topOutline, pts bottomOutline );
		pts		topOutlineThickness() const { return m_topOutlineThickness; }
		pts		bottomOutlineThickness() const { return m_bottomOutlineThickness; }

		int64_t latestSampleTimestamp() const;

		void	setVisible(bool bVisible);
		bool	isVisible() const { return m_bVisible; }

		bool	isTransitioningColors() const { return m_pColorTransition; }

		HiColor	color() const { return m_fillColor; }
		HiColor	outlineColor() const { return m_outlineColor; }

	protected:

        struct SampleSet
        {
			SampleSet( int64_t ts, float top, float bottom )
			{
				timestamp = ts;
				samples[0] = top;
				samples[1] = bottom;
			}

			SampleSet() {};

            int64_t    timestamp;
            float       samples[2];			// top sample followed by bottom sample.
        };


		void			_endColorTransition();

		AreaScrollChart* m_pDisplay = nullptr;

		// Appearance

		int					m_id = 0;

		bool				m_bVisible = true;

		HiColor				m_fillColor = Color::LightGray;
		HiColor				m_outlineColor = Color::Black;
		Gradient			m_fillGradient;
		Gradient			m_outlineGradient;

		pts					m_topOutlineThickness = 1;
		pts					m_bottomOutlineThickness = 1;

		// Transitions

		ColorTransition_p	m_pColorTransition;
		int					m_colorTransitionProgress = 0;

		HiColor				m_startFillColor;
		HiColor				m_endFillColor;

		HiColor				m_startOutlineColor;
		HiColor				m_endOutlineColor;

		Gradient			m_startFillGradient;
		Gradient			m_endFillGradient;

		Gradient			m_startOutlineGradient;
		Gradient			m_endOutlineGradient;


        // PID Constants
        static constexpr float kMicrosecondsPerSecond = 1000000.0f;
        static constexpr float kIntegralDecay = 0.98f;
        static constexpr float kPidProportionalScale = 0.05f * 0.10f;
        static constexpr float kPidIntegralScale = 0.1f * 1.00f;
        static constexpr float kMaxSpeedupFactor = 2.0f;
        static constexpr float kMaxSlowdownFactor = 0.5f;

        // PID states
        float m_lastError = 0.0f;
        float m_errorIntegral = 0.0f;

		// AreaChartEntry samples

		std::vector<SampleSet>	m_samples;

		float				m_defaultTopSample = 0.f;
		float				m_defaultBottomSample = 0.f;


		// Output

		bool				m_bColorsChanged = false;

		Waveform_p			m_pWaveform;

		std::function<void(int64_t, int64_t, int64_t, int64_t)>	m_fetcher;
	};



    class AreaScrollChart;
    typedef StrongPtr<AreaScrollChart> AreaScrollChart_p;
    typedef WeakPtr<AreaScrollChart> AreaScrollChart_wp;


    class AreaScrollChart: public ScrollChart, private DynamicVector<AreaScrollChartEntry>::Holder
    {
        friend class AreaScrollChartEntry;

    public:
		struct Blueprint
		{
			Object_p		baggage;

			Placement		bottomLabelPlacement = Placement::South;
			pts				bottomLabelSpacing = 1;

			float			displayCeiling = 1.f;
			float			displayFloor = 0.f;
			Skin_p			displaySkin;
			int				displayTime = 4000000;    // microsec of samples displayed.

			bool			dropTarget = false;
			bool			disabled = false;
			Finalizer_p		finalizer = nullptr;

			GfxFlip			flip = GfxFlip::None;

			bool			padWithLastSample = false;		// Pad with last sample when we run out of samples. Otherwise default sample is used.

			Glow::Blueprint glow;

			HiColor			gridColor = Color::DarkGray;
			pts				gridThickness = 1;

			int				id = 0;
			Skin_p			labelSkin;
			int				latency = 10000;

			Placement		leftLabelPlacement = Placement::West;
			pts				leftLabelSpacing = 4;

			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			int				maxDisplayTime = 0;
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

		static AreaScrollChart_p create() { return AreaScrollChart_p(new AreaScrollChart()); }
		static AreaScrollChart_p create(const Blueprint& blueprint) { return AreaScrollChart_p(new AreaScrollChart(blueprint)); }

		//.____ Components _______________________________________

		DynamicVector<AreaScrollChartEntry>	entries;

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo TYPEINFO;

		//.____ Behavior ____________________________________________________________

		void	setPadWithLastSample(bool bPadWithLast);
		bool	padWithLastSample() const { return m_bPadWithLastSample; }

	protected:
		AreaScrollChart();

		template<class BP>
		AreaScrollChart(const BP& bp): ScrollChart(bp), entries(this)
		{
			m_bPadWithLastSample 	= bp.padWithLastSample;
		}

		virtual ~AreaScrollChart();

		void	_update(int microPassed, int64_t microsecTimestamp) override;

		void	_updateWaveformEdge(Waveform* pWaveform, int64_t beginUS, int pixelIncUS, bool bTopEdge, AreaScrollChartEntry::SampleSet* pSamples);

		void	_initEntrySamples(AreaScrollChartEntry* pEntry);


		//

   		void	_didAddEntries(AreaScrollChartEntry* pEntry, int nb) override;
		void	_didMoveEntries(AreaScrollChartEntry* pFrom, AreaScrollChartEntry* pTo, int nb) override;
		void	_willEraseEntries(AreaScrollChartEntry* pEntry, int nb) override;

		void	_renderOnScrollSurface( GfxDevice * pDevice, SizeSPX canvasSize, spx leftEdgeOfs, int64_t leftEdgeTimestamp, spx dirtLen ) override;
		void	_removeOutdatedSamples() override;

		void	_startedOrEndedTransition();
		void	_entryVisibilityChanged(AreaScrollChartEntry* pAreaChartEntry);


	private:

		bool m_bTransitioning = false;
		bool m_bPadWithLastSample = false;
	};

} // namespace wg
#endif // WG_AREASCROLLCHART_DOT_H
