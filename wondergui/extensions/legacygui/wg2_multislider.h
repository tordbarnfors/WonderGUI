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
#ifndef WG2_MULTISLIDERS_DOT_H
#define WG2_MULTISLIDERS_DOT_H


#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif


#include <vector>



//____ WgMultiSlider ____________________________________________________________

class WgMultiSlider : public WgWidget
{
protected:
	struct Slider;

public:
	WgMultiSlider();
	virtual ~WgMultiSlider();

	virtual const char *Type( void ) const override;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const override{ return new WgMultiSlider(); };

	WgSize	PreferredPixelSize() const override;

	enum PressMode
	{
		NoMovement,
		SetValue,
//		StepValue,
		MultiSetValue
//		MultiStepValue
	};

	struct Bounds
	{
		float	min;
		float	max;
		int		steps;
	};

	class Visitor
	{
	public:
		Visitor(WgMultiSlider * pWidget, Slider * pSlider) : m_pWidget(pWidget), m_pSlider(pSlider) {}
	protected:
		WgMultiSlider * m_pWidget;
		Slider *		m_pSlider;
	};

	class SetValueVisitorBase : public Visitor
	{
	public:
		SetValueVisitorBase(WgMultiSlider * pWidget, Slider * pSlider);

		float		value(int sliderId);
		float		setValue(int sliderId, float value);

		WgCoordF	value2D(int sliderId);
		WgCoordF	setValue2D(int sliderId, WgCoordF value);
	};

	class SetValueVisitor : public SetValueVisitorBase
	{
	public:
		using SetValueVisitorBase::value;
		SetValueVisitor(WgMultiSlider * pWidget, Slider * pSlider, float value);

		float		value();
		Bounds		valueBounds();

		float		m_value;
	};

	class SetValueVisitor2D : public SetValueVisitorBase
	{
	public:
		using SetValueVisitorBase::value2D;
		SetValueVisitor2D(WgMultiSlider * pWidget, Slider * pSlider, WgCoordF value );

		WgCoordF	value2D();
		Bounds		valueBoundsX();
		Bounds		valueBoundsY();

		WgCoordF	m_value;
	};




	class SetGeoVisitor : public Visitor
	{
	public:
		SetGeoVisitor(WgMultiSlider * pWidget, Slider * pSlider);

		WgCoordF	handlePos(int sliderId);
		WgRectF		geo(int sliderId);
	};

	typedef std::function<float(SetValueVisitor& visitor)>			SetValueFunc;
	typedef std::function<WgCoordF(SetValueVisitor2D& visitor)>		SetValueFunc2D;

	typedef std::function<WgRectF(SetGeoVisitor& visitor)>			SetGeoFunc;


	void	SetDefaults(wg::Skin * pBgSkin, wg::Skin * pHandleSkin, WgCoordF handleHotspot = { 0.5f,0.5f },
						WgBorders handleMarkExtension = WgBorders(0), WgBorders sliderMarkExtension = WgBorders(0) );

	void	SetCallback(const std::function<void(int sliderId, float value, float value2)>& callback);
    void    SetStaticCallback(const std::function<void(int sliderId, float value, float value2)>& callback);

	void	SetPassive(bool bPassive);							// No slider repositioning on its own, relies callback/event-listener to call SetSliderValue().
	bool	IsPassive() const { return m_bPassive; }

	void	SetDeltaDrag(bool bDelta);							// Use relative movements, instead of the new position, for slider placement.
	bool	IsDeltaDrag() const { return m_bDeltaDrag;  }

    void    SetSlidersPointAligned(bool bAlign);
    bool    IsSlidersPointAligned() const { return m_bPointAlignSliders; }

	void	SetGhostHandle(bool bGhost);						// Handle is not pressable, only background. Can be useful in some press-modes.
																// In ghost-mode, handle will also get hover and press states when slider is hovered/pressed.
	bool	IsGhostHandle() const { return m_bGhostHandle; }

	void	SetPressMode(WgMultiSlider::PressMode mode);
	PressMode GetPressMode() const { return m_pressMode; }

	void	SetFinetune( int stepSize, float stepIncrement = 0.f);

	void	SetModifierKeys( WgModifierKeys finetune, WgModifierKeys axisLock, WgModifierKeys overrideMode = WG_MODKEY_NONE, WgModifierKeys staticMode = WG_MODKEY_NONE );

	int		AddSlider(	int id, WgDirection dir, SetGeoFunc pSetGeoFunc, float startValue = 0.f, float minValue = 0.f, float maxValue = 1.f, int steps = 0,
						SetValueFunc pSetValueFunc = nullptr, wg::Skin * pBgSkin = nullptr,
						wg::Skin * pHandleSkin = nullptr, WgCoordF handleHotspot = { -1.f,-1.f },
						WgBorders handleMarkExtension = WgBorders(0), WgBorders sliderMarkExtension = WgBorders(0) );

	int		AddSlider2D( int id, WgOrigo origo, SetGeoFunc pSetGeoFunc, float startValueX = 0.f, float startValueY = 0.f,
						float minValueX = 0.f,  float maxValueX = 1.f, int stepsX = 0, float minValueY = 0.f, float maxValueY = 1.f, int stepsY = 0,
						SetValueFunc2D pSetValueFunc = nullptr,
						wg::Skin * pBgSkin = nullptr, wg::Skin * pHandleSkin = nullptr, WgCoordF handleHotspot = { -1.f, -1.f },
						WgBorders handleMarkExtension = WgBorders(0), WgBorders sliderMarkExtension = WgBorders(0) );

	void    RemoveAllSliders();

	float	SetSliderValue(int id, float value, float value2 = NAN);

	void    SetSliderVisible( int id, bool bVisible );
    bool    GetSliderVisible(int id);

    WgCoord HandlePointPos( int sliderId );
	WgCoord HandlePixelPos( int sliderId );

    int     NbSliders() const { return (int) m_sliders.size(); }
    int     SliderId(int index) { return m_sliders[index].id; }

	bool	MarkTest(const WgCoord& ofs) override;

	void	SetOnlyHandlesPressable(bool bOnlyHandles) { m_bOnlyHandlesPressable = bOnlyHandles; }

	
	wg::Skin_p GetHandleSkin(int sliderId)
	{
		Slider * s  = _findSlider(sliderId);

		if(s)
			return s->pHandleSkin;
		else
			return nullptr;

	}

	void SetHandleSkin(int sliderId, wg::Skin * pkHandleSkin)
	{
		Slider * s  = _findSlider(sliderId);
		if( !s )
			return;

		WgRect sliderGeo = _sliderGeo(*s, PixelSize());

		WgRect handleGeo1 = _sliderHandleGeo(*s, sliderGeo);
		s->pHandleSkin = pkHandleSkin;
		WgRect handleGeo2 = _sliderHandleGeo(*s, sliderGeo);

		_requestRender(WgRect::bounds(handleGeo1,handleGeo2));
	}

    //NOTE: Would be better if this just returned the sliderId, instead of exposing internal structure.
    Slider* MarkedSlider(WgCoord kPointOfs) { return _markedSlider(kPointOfs * m_scale / WG_SCALE_BASE); }

    WgRect  SliderHandleGeo(int sliderId) const;

	//Used in WeissEQ1 & WeissEQMP when using lasso feature, so set the lock state of all selected bands (sliders)
	//to the same direction.
	int GetAxisLockState() { return m_axisLockState; }
	void SetAxisLockState(int iLockState) { m_axisLockState = (AxisLockState)iLockState; }

    void SetSliderLast(int iSliderID);

    void SetSliderState(int iSliderID, bool bEnabled);
    bool GetSliderEnabled(int iSliderID);
    void SetSliderCheckState(bool bState) { m_CheckStateForMove = bState; }

    int HoveredSliderHandleIndex() { return m_hoveredSliderHandle; }
    void RefreshSliderGeo() { _refreshSliderGeo(); }

protected:

	enum AxisLockState
	{
		Unlocked,
		Locking,							// Locking started, but axis not determined yet.
		Horizontal,							// Only horizontal movement.
		Vertical,							// Only vertical movement.
	};

	struct Slider
	{
		int				id;

		bool			is2D;
		bool            bVisible = true;

		float			value[2];
		Bounds			bounds[2];

		WgOrigo			origo;
		WgRectF			geo;				// 0.f -> 1.f, within widgets content rect
		WgCoordF		handlePos;			// 0.f -> 1.f, within geo
		WgCoordF		handleHotspot;		// 0.f -> 1.f, within its own size
		WgState			sliderState;
		WgState			handleState;
		int				geoState;			// 0 = needs refresh, 1 = refresh in progress, 2 = refreshed.

		wg::Skin_p		pBgSkin;
		wg::Skin_p		pHandleSkin;
		WgBorders		handleMarkExtension;		// Frame surrounding handle that also marks the handle. Measured in points, not pixels.
		WgBorders		sliderMarkExtension;		// Frame surrounding slider that also marks the slider. Measured in points, not pixels.

		SetValueFunc	pSetValueFunc;
		SetValueFunc2D	pSetValueFunc2D;

		SetGeoFunc		pSetGeoFunc;
	};

	void	_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler) override;
	void	_onCloneContent( const WgWidget * _pOrg ) override;
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window ) override;
	bool	_onAlphaTest( const WgCoord& ofs ) override;
	void	_setScale(int scale) override;

	void	_setSliderStates(Slider& slider, WgState newSliderState, WgState newHandleState);
	void	_updateHandlePos(Slider& slider);									// Updates handlePos from its parameter values.
	void	_updateGeo(Slider& slider);

	WgRect	_sliderGeo(const Slider& slider, const WgRect& canvas) const;
	WgRect	_sliderSkinGeo(const Slider& slider, const WgRect& sliderGeo ) const;
	WgRect	_sliderHandleGeo(const Slider& slider, const WgRect& sliderGeo ) const;

	Slider * _markedSliderHandle(WgCoord ofs, WgCoord * pOfsOutput = nullptr );
	void	_markSliderHandle(Slider * pSlider);
	void	_selectSliderHandle(Slider * pSlider);
	void	_requestRenderSlider(Slider * pSlider);								// Render whole slider (background and handle)
	void	_requestRenderHandle(Slider * pSlider);								// Render only handle part of slider

	Slider * _markedSlider(WgCoord ofs, WgCoord * pOfsOutput = nullptr);

	WgCoordF	_alignPosToStep(Slider& slider, WgCoordF pos) const;

	WgCoordF	_setHandlePosition(Slider& slider, WgCoordF pos);					// Set handlePos and update value(s).
	WgCoordF	_calcSendValue(Slider& slider, WgCoordF pos);						// Like _setHandlePosition(), but only calculates and sends the value (no internal update).

	void		_invokeSetValueCallback(Slider& slider, float& value, float& value2);

	float	_setValue(Slider& slider, float valueX, float valueY, bool bSendOnUpdate );			// Set value(s) and update handlePos.
	void		_sendValue(Slider& slider, float value, float value2);

	WgCoordF	_convertFactorPos(WgCoordF in, WgOrigo origo);						// Convert between sliderFactor and sliderPosition.


	void		_refreshSliders();
	void		_refreshSliderGeo();
	void		_updatePointerStyle(WgCoord pointerPos);
	void		_updateSliderStates();

	Slider *	_findSlider(int sliderId);
	const Slider *	_findSlider(int sliderId) const;

    int SelectedSliderHandleIndex() { return m_selectedSliderHandle; }

private:
    wg::Skin_p			m_pDefaultBgSkin;
	wg::Skin_p			m_pDefaultHandleSkin;
	WgCoordF			m_defaultHandleHotspot = { 0.5f, 0.5f };
	WgBorders			m_defaultHandleMarkExtension;
	WgBorders			m_defaultSliderMarkExtension;

	std::vector<Slider>	m_sliders;

	// Settings parameters

	PressMode			m_pressMode = PressMode::NoMovement;

    bool                   m_bPassive = false;
    bool                   m_bGhostHandle = false;
    bool                   m_bDeltaDrag = false;
    bool                   m_bPointAlignSliders = false;
    bool                   m_CheckStateForMove = false;

	bool				m_bOnlyHandlesPressable = false;
	bool				m_ignoredPresses[WG_MAX_BUTTONS+1];

	
	
    WgModifierKeys         m_axisLockModifier = WG_MODKEY_ALT;
    WgModifierKeys         m_finetuneModifier = WG_MODKEY_STD_CTRL;
    WgModifierKeys         m_overrideModifier = WG_MODKEY_NONE;             // If pressed, we only do callback and send event, we don't grab or move slider.
    WgModifierKeys         m_staticModifier = WG_MODKEY_NONE;

	int					m_finetuneStepSize = 5;							// 0 = increment on every pixel, otherwise points to move pointer before value incremented.
	float				m_finetuneStepIncrement = 0.f;					// 0 = increment with value of one pixels movement, otherwise by specified value.

	// Dynamically changing parameters

	int					m_selectedSliderHandle = -1;					// Handle selected
	int					m_selectedSlider = -1;							// Selected slider, when slider but not handle is selected (used in certain modes).

	int					m_hoveredSliderHandle = -1;						// Handle hovered
	int					m_hoveredSlider = -1;							// Hovered slider, when slider but not handle is hovered.

	WgCoordF			m_dragFraction;									// Accumulated drag movement from dragStartFraction.
	WgCoordF			m_dragStartFraction;							// Slider handle position in fractions when selected for drag.

	WgCoord				m_finetuneRemainder;							// pixels dragged not a full step yet.

	AxisLockState		m_axisLockState = AxisLockState::Unlocked;
	WgCoord				m_axisLockPosition;
	WgCoordF			m_axisLockFraction;

	std::function<void(int sliderId, float value, float value2 )>	m_callback = nullptr;
	std::function<void(int sliderId, float value, float value2 )>	m_staticCallback = nullptr;

};

#endif //WG2_MULTISLIDERS_DOT_H
