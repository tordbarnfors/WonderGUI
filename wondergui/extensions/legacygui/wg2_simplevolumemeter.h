
#ifndef WG2_SIMPLEVOLUMEMETER_DOT_H
#define WG2_SIMPLEVOLUMEMETER_DOT_H

#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_COLOR_DOT_H
#	include <wg2_color.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

//____ WgSimpleVolumeMeter ____________________________________________________________

class WgSimpleVolumeMeter : public WgWidget
{
public:
	WgSimpleVolumeMeter();
	virtual ~WgSimpleVolumeMeter();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgSimpleVolumeMeter(); };

	//____ Methods __________________________________________

	void			SetColors( WgColor bottom, WgColor middle, WgColor top );
	void			SetSections( float bottomFraction, float topFraction );
	void			SetHoldHeight( float fraction );

	void			SetPeakSkin(wg::Skin * pSkin);
	wg::Skin_p		PeakSkin() const { return m_pPeakSkin;  }

    void            SetGradientSkin(wg::Skin * pSkin);
    wg::Skin_p      GradientSkin() const { return m_pGradientSkin; }

	void			SetHoldSkin(wg::Skin * pSkin);
	wg::Skin_p		HoldSkin() const { return m_pHoldSkin; }

    void            SetBackgroundColor(WgColor background);
    void            EnableBackgroundColor(bool enable) { m_bEnableBackgroundColor = enable; }

	void			SetValue( float peak, float hold );
	void			SetValue( float leftPeak, float leftHold, float rightPeak, float rightHold );
	void            SetGaps(float fSides, float fGap) {m_fSidePadding = fSides; m_fGap = fGap; }

	void			SetDirection(WgDirection direction);


	WgSize			PreferredPixelSize() const;

protected:

	void			_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	void			_onCloneContent( const WgWidget * _pOrg );
	bool			_onAlphaTest( const WgCoord& ofs );
	void			_onNewSize( const WgSize& size );

	void			_renderBar( wg::GfxDevice * pDevice, int nb, const WgRect& _rect );
	void 			_requestRenderPartial( WgSize sz, int newLeftPeak, int newLeftHold, int newRightPeak, int newRightHold );
	void			_updateIValues( WgSize sz );
	int 			_calcIHold( float holdValue, WgSize canvas );



	WgDirection		m_direction;
	wg::Skin_p		m_pPeakSkin;
	wg::Skin_p		m_pHoldSkin;
    wg::Skin_p      m_pGradientSkin;
	WgColor			m_sectionColors[3];
	float			m_fSectionHeight[3];
	float			m_fHoldHeight;
	bool			m_bStereo;
	float 			m_fGap;
	float 			m_fSidePadding;
	float			m_fPeak[2];
	float			m_fHold[2];

    bool            m_bEnableBackgroundColor = false;
    WgColor         m_backgroundColor;

	// Pixel values calculated from relative values above

	int 			m_iGap;
	int 			m_iSidePadding;
	int				m_iHoldHeight;
	int				m_iPeak[2];
	int 			m_iHold[2];
	int				m_iSectionHeight[3];
};



#endif //WG2_SIMPLEVOLUMEMETER_DOT_H
