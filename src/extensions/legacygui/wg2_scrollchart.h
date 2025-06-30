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
#ifndef WG2_SCROLLCHART_DOT_H
#define WG2_SCROLLCHART_DOT_H


#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_COLOR_DOT_H
#	include <wg2_color.h>
#endif

#include <wg_string.h>
#include <wg_skin.h>


#include <functional>
#include <vector>
#include <deque>

#include <wg_surfacefactory.h>
#include <wg_gradient.h>

//____ WgScrollChart ____________________________________________________________

class WgScrollChart : public WgWidget
{
public:
	WgScrollChart();
	virtual ~WgScrollChart();

	virtual const char *Type(void) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgScrollChart(); };

	void	SetPreferredChartSize(const WgSize& size);			// Preferred size of the wave area (widget minus all padding) set in POINTS, not pixels!
	WgSize	PreferredPixelSize() const;

	void		SetSurfaceFactory(wg::SurfaceFactory * pFactory);
	wg::SurfaceFactory_p SurfaceFactory() const {return m_pFactory;}

	void		SetCanvasPadding(WgBorders);
	WgBorders	CanvasPadding() const { return m_pointPadding; }

	void		SetSkin(wg::Skin * pSkin);

	void		SetChartColor(WgColor color);
	WgColor		ChartColor() const { return m_chartColor;  }

	void		SetScrollFromStart(bool bScrollOnStart);
	bool		ScrollFromStart() const { return m_bScrollFromStart; }

	struct GridLine
	{
		float	pos;
		float	thickness;
		WgColor	color;
		wg::String label;
	};

	struct SamplePair {
		float top;
		float bottom;
	};

	//

	bool	Start( int sampleTTL );
	void	Stop();
	bool	IsStarted() const { return m_bStarted; }

	void	Pause();
	void	Continue();
	bool	IsPaused() const { return m_bPaused;  }

	bool	SetValueRange(float topValue, float bottomValue);
	void	SetDynamicValueRange(bool bDynamic);
	bool	IsValueRangeDynamic() const { return m_bDynamicValueRange; }

	float	ValueRangeStart() { return m_topValue; }
	float	ValueRangeEnd() { return m_bottomValue; }

	int		StartLineWave( float startSample, float thickness = 1.f, WgColor color = WgColor::Black, std::function<float(uint64_t timeCode)> sampleFeeder = nullptr );

	int		StartSimpleWave( float startSample, float floor, float topLineThickness, WgColor topLineColor,
							float floorLineThickness, WgColor floorLineColor, WgColor fillColor, std::function<float(uint64_t timeCode)> sampleFeeder = nullptr );

	int		StartComplexWave( SamplePair startSample, float topLineThickness, WgColor topLineColor,
							float bottomLineThickness, WgColor bottomLineColor, WgColor frontFill, std::function<SamplePair(uint64_t timeCode)> sampleFeeder = nullptr );

	bool	StopWave(int waveId );
	void	StopAllWaves();

	bool	SetWaveColors( int waveId, WgColor topLineColor, WgColor bottomLineColor, WgColor fillColor );

	bool	IsWaveDisplayed(int waveId) const;
	bool	HideWave(int waveId);
	bool	UnhideWave(int waveId);
	void	HideAllWaves();
	void	UnhideAllWaves();

	bool	FeedSample(int waveId, float sample);
	bool	FeedSample(int waveId, SamplePair sample);

	void	SetValueLabelStyle(WgOrigo alignment, WgCoord offset, wg::Skin * pSkin, wg::TextStyle * pStyle, bool bLabelsOnRight = false);
	void	SetValueGridLines(int nLines, GridLine * pLines);

	void	SetSampleLabelStyle(WgOrigo alignment, WgCoord offset, wg::Skin * pSkin, wg::TextStyle * pStyle, bool bLabelsOnTop = false );
	void	SetSampleGridLines(int nLines, GridLine * pLines);

	void	SetResizeResponder(std::function<void(WgScrollChart * pWidget, WgSize newSize)> func);	// Called when widgets size has changed.
	void	SetValueRangeResponder(std::function<void(WgScrollChart * pWidget, float topValue, float bottomValue)> func);		// Called when widgets sample range has changed.

    void    SetPixelType(WgPixelType pixelType) { m_pixelType = pixelType; _regenCanvas(); }
	void	SetGridToForeground( bool bForeground ) { m_bForegroundGrid = bForeground; }

	void	SetCombinedWavesBlend(WgBlendMode blendMode) { m_combinedWavesBlendMode = blendMode; _requestRender(); };
	void	SetCombinedWavesTint(WgColor tintColor) { m_combinedWavesTintColor = tintColor; _requestRender(); };
	void	SetCombinedWavesGradient(wg::Gradient gradient) { m_combinedWavesTintGradient = gradient; _requestRender(); };

	void	SetStaticMode( bool bStatic );
	
	void	SetStaticModeFadeOutLength( int length );
	void	SetStaticModeGradient( const wg::Gradient& gradient );
	
	bool	ClearWave(int waveId, float topSample, float bottomSample);


protected:

	enum WaveType
	{
		Line,
		Simple,
		Complex
	};

	struct Sample
	{
		int			length;							// Milliseconds this sample remains until next sample.
		float		top;
		float		bottom;							// Only used for complex waves, for others we read floorValue.
	};

	struct Wave
	{
		Wave() {}

		WgColor	fillColor = WgColor::DarkGrey;
		float	topLineThickness = 1.f;
		WgColor topLineColor = WgColor::Black;
		float	bottomLineThickness = 0.f;
		WgColor	bottomLineColor = WgColor::Black;

		int			id;
		WaveType	type;
		bool		bStopped = false;
		bool		bHidden = false;

		uint64_t	startTimestamp = 0;

		float		startTopSample = 0;						// Saved for any future restart
		float		startBottomSample = 0;					// Saved for any future restart

		float		nextTopSample = 0;
		float		nextBottomSample = 0;

		float		floorValue = 0.f;						// Only used for simple waves

		float		minSample = 0.f;
		float		maxSample = 0.f;
		int			minSampleCount = 0;						// Ref count for minSample
		int			maxSampleCount = 0;						// Ref count for maxSample

		std::deque<Sample>	samples;

		std::function<float(uint64_t timeCode)> simpleSampleFeeder;
		std::function<SamplePair(uint64_t timeCode)> complexSampleFeeder;
	};

	struct LabelStyle
	{
		WgOrigo				alignment;
		WgCoord				offset;
		wg::Skin_p			pSkin;
		wg::TextStyle_p		pTextStyle;
        bool                bLabelAtEnd;
	};

	void	_renderPatches(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches);

	void	_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler);
	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onNewSize( const WgSize& size );
	void	_setScale( int scale );


	void	_regenCanvas();
	bool	_updateDynamics();
	WgCoord	_placeLabel(WgCoord startPoint, WgOrigo alignment, WgCoord labelOffset, WgSize labelSize ) const;
	Wave *	_getWave(int waveId);
	const Wave *	_getWave(int waveId) const;

	void	_renderWaveSegment(wg::GfxDevice * pDevice, const WgRect& _canvas, double startTimeStamp, double endTimeStamp, float timestampInc );

	void		_resampleWavePortion(int& ofs, int& nSamples, int * pOutTop, int * pOutBottom, const WgScrollChart::Wave& wave, double startTimeStamp, double endTimeStamp, float timestampInc);
	void		_renderValueGridLines(wg::GfxDevice * pDevice, const WgRect& _canvas);
	void		_renderSampleGridLines(wg::GfxDevice* pDevice, const WgRect& _canvas);

	float			_thickestLine() const;

	WgRect	_getScrollWindow() const;

//	void	_onEnable();
//	void	_onDisable();

private:

	WgSize			m_defaultSize;
	WgMode			m_mode;
	WgColor			m_chartColor;

	WgColor			m_combinedWavesTintColor = WgColor::White;
	WgBlendMode		m_combinedWavesBlendMode = WgBlendMode::Blend;
	wg::Gradient	m_combinedWavesTintGradient;
	
	WgBorders		m_pointPadding;			// Padding for the canvas in points. To allow thick lines to fully stay inside widget. Grid is allowed outside.
	WgBorders		m_pixelPadding;			// Same, but in pixels.

	bool			m_bScrollFromStart;		// If set, first data is scrolled in from right. If not set, data is filled from left to right before the chart starts to scroll.
	bool			m_bStarted;
	bool			m_bPaused;

	bool			m_bStaticMode = false;
	int				m_fadeTailLength = 16;		// Pixels of "fade out tail" if m_bScrolling == false.
	
	wg::Gradient 	m_fadeTailGradient = wg::Gradient( wg::HiColor(4096, 4096, 4096, 0), wg::HiColor::White, wg::HiColor::White, wg::HiColor(4096, 4096, 4096, 0) );

	
	std::vector<Wave>		m_waves;
	std::vector<GridLine>	m_valueGridLines;
	std::vector<GridLine>	m_sampleGridLines;

	bool		m_bDynamicValueRange;

	float		m_setTopValue;
	float		m_setBottomValue;

	float		m_topValue;
	float		m_bottomValue;

	int			m_sampleTTL;				// Samples Time To Live in microsec, before it has scrolled out.
	int			m_waveIdCounter;

	double		m_windowBegin;				// In microseconds, oldest sample displayed
	double		m_windowEnd;				// In microseconds, latest sample displayed

	float		m_scrollFraction;			// Fractions of a pixel to scroll, measured in microseconds
	int			m_scrollAmount;				// Number of full pixels to scroll

	uint64_t	m_sampleBeginTimestamp;		// Timestamp of first samples in store
	uint64_t	m_sampleEndTimestamp;		// Timestamp of last samples in store

	wg::SurfaceFactory_p	m_pFactory;
	wg::Surface_p		m_pCanvas;
	int					m_canvasOfs;		// offset in pixels for left edge of widget.
	bool				m_bRefreshCanvas;	// Totally redraw everything on our canvas.

	LabelStyle	m_sampleLabelStyle;
	LabelStyle	m_valueLabelStyle;

	std::function<void(WgScrollChart * pWidget, WgSize newSize)>	m_resizeResponder;
	std::function<void(WgScrollChart * pWidget, float topValue, float bottomValue)> m_valueRangeResponder;
    
    WgPixelType m_pixelType = WgPixelType::BGRX_8;
	bool		m_bForegroundGrid = false;
};


#endif //WG2_SCROLLCHART_DOT_H
