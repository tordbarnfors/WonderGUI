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
#ifndef WG2_KNOB_DOT_H
#define WG2_KNOB_DOT_H

#include <wg_surface.h>

#include <wg_surfacefactory.h>

#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#include <wg2_color.h>

#include <cmath>
#include <algorithm>
#include <vector>

//____ WgKnob ____________________________________________________________

#define WG2_KNOB_PIXEL_BUFFER_SIZE 1000

class WgKnob : public WgWidget
{
public:
	WgKnob();
	WgKnob(wg::SurfaceFactory * pFactory);
	virtual ~WgKnob();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgKnob(); };

	void	SetOrigo(WgOrigo origo);

	void    SetOversample(int oversample) { m_iOversampleX = oversample; }
	void    SetValue( float value );
	float   GetValue() const { return m_fValue; }
	int     GetValueIndex() const
	{
		int i = (int)(m_fValue * m_iNumSteps);
		i = i>=m_iNumSteps?m_iNumSteps-1:i;
		return i;
	}
	void    SetColor( WgColor color )
	{
		if( m_kColor != color )
		{
			m_kColor = color;
			_requestRender();
		}
	};

	void    SetForegroundColor(WgColor color)
	{
		SetColor(color);
	};

	void    SetBackgroundAlpha(float alpha)			// Set alpha of background relative foreground (0.0 -> 1.0). Forces regeneration of graphics.
	{
		if(m_fBackgroundAlpha != alpha)
		{
			m_fBackgroundAlpha = alpha;
            m_kBackground.a = uint8_t(255 * m_fBackgroundAlpha);
            _requestRenderBackBuffer();
        }
	};

	void	SetKnobScale(float scale);

	// Speeds up things significantly by only updating needed quadrants, but generates artifacts if gradiants are present.
	void	SetOptimizeUpdateRect(bool bOptimize)
	{
		m_bOptimizeUpdateRect = bOptimize;
	}

	void	SetMouseControl(bool bMouseControl);

	void    SetNumSteps(int steps);
	void    SetWidth(float width) { m_iWidth = width; }
	void    SetAngles(float angleStart, float angleEnd);
	void    SetAngleOffset(float offset);
	void    SetDrawFromMiddle(bool bDrawFromMiddle) { m_bDrawFromMiddle = bDrawFromMiddle; }
	bool    DrawFromMiddle() { return m_bDrawFromMiddle; }
	float   GetAngleStart() const { return m_fAngleStart; }
	float   GetAngleEnd() const { return m_fAngleEnd; }

	WgSize    PreferredPixelSize() const;
	//    void    SetPreferredPixelSize(WgSize size);
	//    WgSize  PreferredPointSize() { PreferredPixelSize()*m_scale/WG_SCALE_BASE;}
	//    void    SetPreferredPointSize(WgSize size) { SetPreferredPixelSize(size*WG_SCALE_BASE/m_scale); }

	int		MatchingPixelHeight(int pixelWidth) const;
	int		MatchingPixelWidth(int pixelHeight) const;

    void SetForgroundolor(WgColor kColor)
    {
        m_kForeground = kColor;
        _requestRenderBackBuffer();
    }
    void SetBackgroundColor(WgColor kColor)
    {
        m_kBackground = kColor;
        m_kBackground.a = uint8_t(255 * m_fBackgroundAlpha);
        _requestRenderBackBuffer();
    }
    void SetBackTransparancyColor(WgColor kColor)
    {
        m_kBackTransp = kColor;
        _requestRenderBackBuffer();
    }

protected:
	void    _onCloneContent( const WgWidget * _pOrg );
	void	_renderPatches(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches);
	void    _onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window);
	bool    _onAlphaTest( const WgCoord& ofs );
	void    _onEnable();
	void    _onDisable();
	void    _onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	void	_onNewSize(const WgSize& size);


	void	_requestRenderBackBuffer();
	void	_requestRenderBackBuffer(const WgRect& rect);



private:

	enum DrawState
	{
		eDrawNothing,
		eDrawForegroundAABegin,
		eDrawForeground,
		eDrawForegroundAAEnd,
		eDrawBackgroundAABegin,
		eDrawBackground,
		eDrawBackgroundAAEnd,
		eDrawForegroundAALeftEdge,
		eDrawForegroundAARightEdge
	};

	WgRect	_calcUdateRect(float newAngleStart, float newAngleEnd, float newValue);
	void	_redrawBackBuffer(WgRect region);
	void	_downsample(wg::Surface* pSurf, int oversample);
	static inline WgColor Blend( const WgColor& start, const WgColor& dest, float grade );

	WgSize m_preferredSize;
	bool	m_bOptimizeUpdateRect = false;
	float	m_knobScale = 1.f;					// Scale the knob inside its area (0.f-1.f)

	wg::Surface_p m_pSurf;
	wg::SurfaceFactory_p m_pSurfaceFactory;
	float m_fValue = 0.0f;

	// For new knob mode.
	int m_iOversampleX = 1;
	bool m_bPressed = false;
	bool m_bPointerInside = false;
	bool m_bMouseControl = true;

	float	m_fBackgroundAlpha = 0.25f;							// Alpha of background, relative alpha of foreground.
	WgColor m_kColor = WgColor( 38,  169, 224, 255 );

    WgColor m_kForeground = {255, 255, 255, 255};
    WgColor m_kBackground = {255, 255, 255, uint8_t(255 * m_fBackgroundAlpha)};
    WgColor m_kBackTransp = {255, 255, 255, 0};

    float m_fAngleStart = 0.1f;
	float m_fAngleEnd = 0.9f;
	std::vector<float> m_AngleStart;
	std::vector<float> m_AngleEnd;
	int m_iNumSteps = 0;
	float m_iWidth = 0.40f;
	float m_fAngleOffset = 0.0f;
	bool m_bDrawFromMiddle = false;

	WgSize m_size = WgSize(0,0);

	WgRect	m_backBufferDirtyRect;
	WgOrigo	m_origo = WgOrigo::NorthWest;
};


#endif //WG2_KNOB_DOT_H
