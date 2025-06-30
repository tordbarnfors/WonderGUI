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
#ifndef WG2_CHART_DOT_H
#define WG2_CHART_DOT_H


#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_COLOR_DOT_H
#	include <wg2_color.h>
#endif

#include <wg_string.h>
#include <wg2_patches.h>


#include <functional>
#include <vector>

#include <wg_surfacefactory.h>
#include <wg_skin.h>
#include <wg_gradient.h>


//____ WgChart ____________________________________________________________

class WgChart : public WgWidget
{
public:
	WgChart();
	virtual ~WgChart();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgChart(); };

	void	SetPreferredChartSize(const WgSize& size);			// Preferred size of the wave area (widget minus all padding) set in POINTS, not pixels!
	WgSize	PreferredPixelSize() const;

	void		SetCanvasPadding(WgBorders);
	WgBorders	CanvasPadding() const;

	void	SetSkin(wg::Skin * pSkin);

	struct GridLine
	{
		float	pos;
		float	thickness;
		WgColor	color;
		wg::String label;
	};

	void	ClearWaves();
	int		AddWave();

    bool	SetWaveStyle(int waveId, WgColor fillColor, float topLineThickness = 0.f, WgColor topLineColor = WgColor::Black, float bottomLineThickness = 0.f, WgColor bottomLineColor = WgColor::Black, int transitionMs = 0);

    bool    SetWaveGradient(int waveId, wg::Gradient gradient);

	bool	SetWaveSamples(int waveId, int firstSample, int nSamples, float * pTopBorderSamples, float * pBottomBorderSamples);
	bool	SetWaveSamples(int waveId, int firstSample, int nSamples, const float * pSamples, float floor = 0.f);

	bool	IsWaveDisplayed(int waveId) const;
	bool	HideWave(int waveId);
	bool	UnhideWave(int waveId);
	void	HideAllWaves();
	void	UnhideAllWaves();


	bool	SetFixedValueRange(float topValue, float bottomValue );
	void	SetDynamicValueRange();
	bool	IsValueRangeDynamic() const { return m_bDynamicValueRange;  }
    void    SetDynamicStepSize(float step) { m_fDynamicStepSize = step; m_bForceSymmetricAndQuantized = true;}
    void    SetDynamicMinSize(float minsize) { m_fDynamicMinSize = minsize; }
    void    IgnoreDynamicScaling(int waveId, bool ignore = true);
    void    SetDefaultValue(float v) { m_defaultValue = v; }

	float	ValueRangeStart() { return m_topValue; }
	float	ValueRangeEnd() { return m_bottomValue; }

	bool    SetRenderSectionWidth( int nPoints );
	int     RenderSectionWidth() const { return m_renderSectionWidth; }

	bool	SetFixedSampleRange(float firstSample, float lastSample);
	void	SetDynamicSampleRange();
	bool	IsSampleRangeDynamic() const { return m_bDynamicSampleRange; }
	int     NativeSampleRange() const;

	float	FirstSample() { return m_firstSample;  }
	float	LastSample() { return m_lastSample;  }

//	bool	SetResampleMethod();			// Nearest, Interpolate etc   We have two dimensions here...  Average, Maintain peaks...

	void	SetSampleLabelStyle(WgOrigo alignment, WgCoord offset, wg::Skin * pSkin, wg::TextStyle * pStyle, bool bLabelsOnTop = false );
	void	SetSampleGridLines(int nLines, GridLine * pLines);

    void	SetValueLabelStyle(WgOrigo alignment, WgCoord offset, wg::Skin * pSkin, wg::TextStyle * pStyle, bool bLabelsOnRight = false );
	void	SetValueGridLines(int nLines, GridLine * pLines);
    void    SetShowGrid(bool show) { m_bShowGrid = show; }

	void	SetResizeResponder(std::function<void(WgChart * pWidget, WgSize newSize)> func);	// Called when widgets size has changed.
	void	SetSampleRangeResponder(std::function<void(WgChart * pWidget, float firstSample, float lastSample)> func);	// Called when widgets sample range has changed.
	void	SetValueRangeResponder(std::function<void(WgChart * pWidget, float topValue, float bottomValue)> func);		// Called when widgets value range has changed.

	bool    SetBitmapCaching( int beginWaveId, int endWaveId, wg::SurfaceFactory * pFactory );
	void    ClearBitmapCaching();


	void	SetScale(int scale) { _setScale(scale); }

    void    UseTemporalFiltering(bool useFilter) { m_useTemporalFiltering = useFilter; }
    void    SetAttackTime(float attackTime) { m_attackTime = attackTime; }
    void    SetReleaseTime(float releaseTime) { m_releaseTime = releaseTime; }


    void    SetSampleLabelColor(WgColor col);
    void    SetSampleGridLineColor(WgColor col);
    void    SetValueLabelColor(WgColor col);
    void    SetValueGridLineColor(WgColor col);

protected:


	struct Wave
	{
		Wave() {}

		int		id;

		WgColor	fillColor = WgColor::DarkGrey;
		float	topLineThickness = 1.f;
		WgColor topLineColor = WgColor::Black;
		float	bottomLineThickness = 0.f;
		WgColor	bottomLineColor = WgColor::Black;

        // Transitions
        WgColor fillStart = WgColor::DarkGrey;
        WgColor fillEnd = WgColor::DarkGrey;
        WgColor topLineColorStart = WgColor::Black;
        WgColor topLineColorEnd = WgColor::Black;
        WgColor bottomLineColorStart = WgColor::Black;
        WgColor bottomLineColorEnd = WgColor::Black;
        int     fadeTimeCounter = 0;
        int     fadeTime = 0;

		int		firstSample = 0;
		int		nSamples = 0;
		float	defaultSample = 0.f;

		float	minSample = 0.f;
		float	maxSample = 0.f;

		bool		bHidden = false;

		std::vector<float>	orgTopSamples;
		std::vector<float>	orgBottomSamples;

		std::vector<int>	resampledTop;
		std::vector<int>	resampledBottom;
		int					resampledDefault;
		int					resampledFirst;

        bool                ignoreDynamicScaling = false;

        wg::Gradient        m_waveGradient;
	};

	struct LabelStyle
	{
		WgOrigo				alignment;
		WgCoord				offset;
		wg::Skin_p			pSkin;
		wg::TextStyle_p		pTextStyle;
        bool                bLabelAtEnd;
	};

	void    _updateBitmapCache( wg::GfxDevice * pDevice );
	void    _renderWave( Wave& wave, wg::GfxDevice * pDevice, const WgRect& waveCanvas );

	void    _renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches );
	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onNewSize( const WgSize& size );
	void	_setScale( int scale );
    void    _onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler);


	bool	_setWaveSamples(int waveId, int firstSample, int nSamples, const float * pTopBorderSamples, float * pBottomBorderSamples, float defaultSample);
	void	_resampleAllWaves();
	void	_resampleWave( Wave * pWave, bool bRequestRenderOnChanges = false );
    void    _filterWaveSamples(const float* in, Wave * p, int ms);


	void	_requestRenderOnNewSamples(	int begOrgSamples, int nbOrgTopSamples, int * pOrgTopSamples, int nbOrgBottomSamples, int * pOrgBottomSamples,
										int begNewSamples, int nbNewTopSamples, int * pNewTopSamples, int nbNewBottomSamples, int * pNewBottomSamples,
										int orgDefaultSample, int newDefaultSample, float maxLineThickness, bool bInCache );
	void    _requestRenderInCache();
	void    _requestRenderInCache( const WgRect& rect );

	bool	_updateDynamics();
	WgCoord	_placeLabel(WgCoord startPoint, WgOrigo alignment, WgCoord labelOffset, WgSize labelSize ) const;

	Wave *	_getWave(int waveId);
	const Wave *	_getWave(int waveId) const;

	bool		_lineFragmentMinMax(int begin, int length, int nbSamples, int * pSamples, int defaultSample, int * wpMin, int * wpMax);


//	void	_onEnable();
//	void	_onDisable();

    void SetLabelHorizontalOffset(int iOffset) { m_iLabelHorizontalOffset = iOffset; }

private:

	WgSize			m_defaultSize;
	WgMode			m_mode;

	int             m_renderSectionWidth;      // Number of render sections chart is split in horizontally.

	WgBorders		m_pointPadding;			// Padding for the canvas in points. To allow thick lines to fully stay inside widget. Grid is allowed outside.
	WgBorders		m_pixelPadding;			// Same, but in pixels.

	int				m_waveIdCounter;

	std::vector<Wave>		m_waves;
	std::vector<GridLine>	m_sampleGridLines;
	std::vector<GridLine>	m_valueGridLines;
    bool m_bShowGrid = true;

	bool		m_bDynamicValueRange;
	bool		m_bDynamicSampleRange;
    bool        m_bForceSymmetricAndQuantized = false;
    float       m_fDynamicStepSize = 1.0f;
    float       m_fDynamicMinSize = 1.0f;


	float		m_topValue;
	float		m_bottomValue;
    float       m_defaultValue = 0.0f;

	float		m_firstSample;
	float		m_lastSample;

	LabelStyle	m_sampleLabelStyle;
	LabelStyle	m_valueLabelStyle;

	std::function<void(WgChart * pWidget, WgSize newSize)>	m_resizeResponder;
	std::function<void(WgChart * pWidget, float firstSample, float lastSample)> m_sampleRangeResponder;
	std::function<void(WgChart * pWidget, float topValue, float bottomValue)> m_valueRangeResponder;

    wg::SurfaceFactory_p  m_pSurfaceFactory;
    wg::Surface_p         m_pCacheBitmap;
    WgPatches           m_cacheDirt;

    int                 m_cacheFirst = 0;
    int                 m_cacheLast = 0;

    float               m_attackTime = 100.0f;
    float               m_releaseTime = 10.0f;
    int                 m_msTimeState = 0;
    int                 m_msTime = 0;
    bool                m_useTemporalFiltering = false;

    int                 m_iLabelHorizontalOffset = 0;
};


#endif //WG2_CHART_DOT_H
