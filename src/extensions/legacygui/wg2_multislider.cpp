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


#include <assert.h>
#include <algorithm>
#include <wg2_multislider.h>
#include <wg2_gfxdevice.h>
#include <wg2_eventhandler.h>
#include <wg2_base.h>

static const char	c_widgetType[] = {"MultiSlider"};

//____ Constructor ____________________________________________________________

WgMultiSlider::WgMultiSlider()
{
	for( int i = 0 ; i < WG_MAX_BUTTONS+1 ; i++)
		m_ignoredPresses[i] = false;

}

//____ Destructor _____________________________________________________________

WgMultiSlider::~WgMultiSlider()
{
}

//____ Type() _________________________________________________________________

const char * WgMultiSlider::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgMultiSlider::GetClass()
{
	return c_widgetType;
}

//____ PreferredPixelSize() ___________________________________________________

WgSize WgMultiSlider::PreferredPixelSize() const
{
	return WgSize(128, 128);
}

//____ SetDefaults() __________________________________________________________

void WgMultiSlider::SetDefaults(wg::Skin * pSliderBgSkin, wg::Skin * pHandleSkin, WgCoordF handleHotspot, WgBorders handleMarkExtension, WgBorders sliderMarkExtension)
{
	m_pDefaultBgSkin = pSliderBgSkin;
	m_pDefaultHandleSkin = pHandleSkin;
	m_defaultHandleHotspot = handleHotspot;
	m_defaultHandleMarkExtension = handleMarkExtension;
	m_defaultSliderMarkExtension = sliderMarkExtension;
}

//____ SetCallback() __________________________________________________________

void WgMultiSlider::SetCallback(const std::function<void(int sliderId, float value, float value2 )>& callback)
{
	m_callback = callback;
}

//____ SetStaticCallback() __________________________________________________________

void WgMultiSlider::SetStaticCallback(const std::function<void(int sliderId, float value, float value2 )>& callback)
{
    m_staticCallback = callback;
}

//____ SetSlidersPointAligned() _______________________________________________

void WgMultiSlider::SetSlidersPointAligned(bool bAlign)
{
    m_bPointAlignSliders = true;
}


//____ SetPassive() ___________________________________________________________

void WgMultiSlider::SetPassive(bool bPassive)
{
	m_bPassive = bPassive;
}

//____ SetDeltaDrag() ___________________________________________________________

void WgMultiSlider::SetDeltaDrag(bool bDeltaDrag)
{
	m_bDeltaDrag = bDeltaDrag;
}

//____ SetGhostHandle() _______________________________________________________

void WgMultiSlider::SetGhostHandle(bool bGhost)
{
	m_bGhostHandle = bGhost;
}

//____ AddSlider() ____________________________________________________________

int WgMultiSlider::AddSlider(	int id, WgDirection dir, SetGeoFunc pSetGeoFunc, float startValue, float minValue, float maxValue, int steps,
								SetValueFunc pSetValueFunc, wg::Skin * pBgSkin,
								wg::Skin * pHandleSkin, WgCoordF handleHotspot, WgBorders handleMarkExtension, WgBorders sliderMarkExtension)
{
	WgOrigo origo;

	switch (dir)
	{
	case WgDirection::Up:
		origo = WgOrigo::South;
		break;
	case WgDirection::Down:
		origo = WgOrigo::North;
		break;
	case WgDirection::Left:
		origo = WgOrigo::East;
		break;
	case WgDirection::Right:
		origo = WgOrigo::West;
		break;
	}

	m_sliders.emplace_back();
	Slider& s = m_sliders.back();

	s.id = id;
	s.is2D = false;


	s.value[0] = startValue;
	s.bounds[0].min = minValue;
	s.bounds[0].max = maxValue;
	s.bounds[0].steps = steps;

	s.origo = origo;

	s.handleMarkExtension = handleMarkExtension;
	s.sliderMarkExtension = sliderMarkExtension;
	s.geoState = 0;
	s.pBgSkin = pBgSkin;
	s.pHandleSkin = pHandleSkin;
	s.pSetValueFunc = pSetValueFunc;

	s.pSetValueFunc2D = nullptr;

	s.pSetGeoFunc = pSetGeoFunc;

	s.handleHotspot = handleHotspot;
	s.sliderState = WgStateEnum::Default;
	s.handleState = WgStateEnum::Default;

	_updateHandlePos(s);
	_updateGeo(s);

	return int(m_sliders.size() - 1);
}

//____ AddSlider2D() __________________________________________________________

int WgMultiSlider::AddSlider2D( int id, WgOrigo origo, SetGeoFunc pSetGeoFunc, float startValueX, float startValueY,
								float minValueX, float maxValueX, int stepsX, float minValueY, float maxValueY, int stepsY,
								SetValueFunc2D pSetValueFunc,
								wg::Skin * pBgSkin, wg::Skin * pHandleSkin, WgCoordF handleHotspot, WgBorders handleMarkExtension, WgBorders sliderMarkExtension)
{
	if (origo != WgOrigo::NorthWest && origo != WgOrigo::NorthEast && origo != WgOrigo::SouthEast && origo != WgOrigo::SouthWest)
		return -1;

	m_sliders.emplace_back();
	Slider& s = m_sliders.back();

	s.id = id;
	s.is2D = true;

	s.value[0] = startValueX;
	s.bounds[0].min = minValueX;
	s.bounds[0].max = maxValueX;
	s.bounds[0].steps = stepsX;

	s.value[1] = startValueY;
	s.bounds[1].min = minValueY;
	s.bounds[1].max = maxValueY;
	s.bounds[1].steps = stepsY;

	s.origo = origo;

	s.handleMarkExtension = handleMarkExtension;
	s.sliderMarkExtension = sliderMarkExtension;
	s.geoState = 0;
	s.pBgSkin = pBgSkin;
	s.pHandleSkin = pHandleSkin;

	s.pSetValueFunc = nullptr;

	s.pSetValueFunc2D = pSetValueFunc;

	s.pSetGeoFunc = pSetGeoFunc;

	s.handleHotspot = handleHotspot;
	s.sliderState = WgStateEnum::Default;
	s.handleState = WgStateEnum::Default;

	_updateHandlePos(s);
	_updateGeo(s);

	return int(m_sliders.size() - 1);
}

//____ RemoveAllSliders() _______________________________________________________

void WgMultiSlider::RemoveAllSliders()
{
	m_sliders.clear();
	m_selectedSliderHandle = -1;

	_requestRender();
}


//____ SetSliderValue() _______________________________________________________

float WgMultiSlider::SetSliderValue(int id, float value, float value2)
{
	Slider * p = _findSlider(id);
	if (!p || (p->is2D == isnan(value2)) )
		return NAN;

	_invokeSetValueCallback(*p, value, value2);

	return _setValue(*p, value, value2, false);
}

//____ SetSliderVisible() ______________________________________________________

void WgMultiSlider::SetSliderVisible( int id, bool bVisible )
{
	// Get slider

	Slider * pSlider = _findSlider(id);
	if( !pSlider || pSlider->bVisible == bVisible )
		return;

	// Set and request render

	pSlider->bVisible = bVisible;

	_requestRenderSlider(pSlider);

	// reset selected if it was when it was made invisible (is this the right thing to do?)

	if( !bVisible )
	{
		int idx = int(pSlider - &m_sliders[0]);

		if( idx == m_selectedSlider )
		{
			m_selectedSlider = -1;
			_updateSliderStates();
		}

		if( idx == m_selectedSliderHandle )
		{
			m_selectedSliderHandle = -1;
			_updateSliderStates();
		}
	}
}
//____ GetSliderVisible() ______________________________________________________

bool WgMultiSlider::GetSliderVisible(int id)
{
    Slider *pSlider = _findSlider(id);
    if (pSlider && pSlider->bVisible == true)
        return true;
    return false;
}
//____ HandlePointPos() ________________________________________________________

WgCoord WgMultiSlider::HandlePointPos( int sliderId )
{
	WgCoord pixelPos = HandlePixelPos(sliderId);

	if(pixelPos.x == -1 && pixelPos.y == -1 )
		return pixelPos;

	return pixelPos*WG_SCALE_BASE / m_scale;
}

//____ HandlePixelPos() ________________________________________________________

WgCoord WgMultiSlider::HandlePixelPos( int sliderId )
{
	WgCoord pos(-1,-1);

	Slider * p = _findSlider(sliderId);
	if( p )
	{
		WgRect sliderGeo = _sliderGeo(*p, PixelSize());

		pos.x = sliderGeo.x + (int)(sliderGeo.w * p->handlePos.x);
		pos.y = sliderGeo.y + (int)(sliderGeo.h * p->handlePos.y);
	}
	return pos;
}

//____ MarkTest() _____________________________________________________________

bool WgMultiSlider::MarkTest(const WgCoord& ofs)
{
	// First do the normal transparency based test. This will call _onAlphaTest() and
	// take markOpacity into account.

	if (WgWidget::MarkTest(ofs))
		return true;

	// Check against our sliders (backgrounds)

	if (m_pressMode != WgMultiSlider::PressMode::NoMovement && _markedSlider(ofs))
		return true;

	// Check against our slider handles

	if (_markedSliderHandle(ofs))
		return true;

	return false;
}

//____ SliderHandleGeo() __________________________________________________________

WgRect WgMultiSlider::SliderHandleGeo(int sliderId) const
{
   auto pSlider = _findSlider(sliderId);

    if( !pSlider )
        return WgRect();

    WgRect geo = _sliderHandleGeo(*pSlider, _sliderGeo(*pSlider, PixelSize()));
    return geo * WG_SCALE_BASE / m_scale;
}


//____ SetPressMode() _________________________________________________________

void WgMultiSlider::SetPressMode(PressMode mode)
{
	m_pressMode = mode;
}

//____ SetFinetune() __________________________________________________________

void WgMultiSlider::SetFinetune(int stepSize, float stepIncrement)
{
	m_finetuneStepSize		= stepSize;
	m_finetuneStepIncrement = stepIncrement;
}

//____ SetModifierKeys() ______________________________________________________

void WgMultiSlider::SetModifierKeys(WgModifierKeys finetune, WgModifierKeys axisLock, WgModifierKeys overrideMode, WgModifierKeys staticMode )
{
	m_finetuneModifier = finetune;
	m_axisLockModifier = axisLock;
	m_overrideModifier = overrideMode;
	m_staticModifier = staticMode;
}


//____ _markedSlider() ________________________________________________________

WgMultiSlider::Slider * WgMultiSlider::_markedSlider(WgCoord ofs, WgCoord * pOfsOutput)
{
	Slider *	pMarked = nullptr;
	WgCoord		markedOfs = { 0,0 };

	if (ofs.x == 0 && ofs.y == 0)
		return nullptr;

	for (auto& slider : m_sliders)
	{
		if( !slider.bVisible )
			continue;

		WgRect sliderGeo = _sliderSkinGeo(slider,_sliderGeo(slider, PixelSize()));

		WgRect sliderMarkGeo = sliderGeo + (slider.sliderMarkExtension.isEmpty() ? m_defaultSliderMarkExtension : slider.sliderMarkExtension);

		if (sliderMarkGeo.contains(ofs) )
		{
			markedOfs = ofs - sliderGeo.pos();
			pMarked = &slider;
		}
	}

	if (pOfsOutput)
		*pOfsOutput = markedOfs;

	return pMarked;
}


//____ _markedSliderHandle() ________________________________________________________

WgMultiSlider::Slider * WgMultiSlider::_markedSliderHandle(WgCoord ofs, WgCoord * pOfsOutput )
{
	//TODO: Check distance for slightly marked and keep the closest one.

	Slider *	pSlightlyMarked = nullptr;
	WgCoord		slightlyMarkedOfs;
	WgCoord     slightlyMarkedDistance;

	Slider *	pFullyMarked = nullptr;
	WgCoord		fullyMarkedOfs;

	if (m_bGhostHandle || (ofs.x == 0 && ofs.y == 0) )
		return nullptr;

	for (auto& slider : m_sliders)
	{
		if( !slider.bVisible )
			continue;

		wg::Skin_p pHandleSkin = slider.pHandleSkin ? slider.pHandleSkin : m_pDefaultHandleSkin;
		if (pHandleSkin )
		{
			WgRect sliderGeo = _sliderGeo(slider, PixelSize());
			WgRect handleGeo = _sliderHandleGeo(slider, sliderGeo);

			// We are using WgStateEnum::Default on purpose here, so that hover hightlights are not included. Not perfect, but the lesser of two evils...

			if (handleGeo.contains(ofs) && _markTestSkin( pHandleSkin, ofs - handleGeo.pos(), handleGeo.size(), WgStateEnum::Default, m_markOpacity, m_scale))
			{
				fullyMarkedOfs = ofs - handleGeo.pos();
				pFullyMarked = &slider;
			}

			WgBorders markExtension = slider.handleMarkExtension.isEmpty() ? m_defaultHandleMarkExtension : slider.handleMarkExtension;

			if (!markExtension.isEmpty())
			{
				WgRect extendedGeo = handleGeo + markExtension.scale(m_scale);

				if (extendedGeo.contains(ofs))
				{
					WgCoord distance = ofs - handleGeo.center();

					// This is our new slightly marked if ofs is closer to handleGeo than any previous slightly marked.

					if( !pSlightlyMarked || (distance.x*distance.x + distance.y*distance.y) < (slightlyMarkedDistance.x*slightlyMarkedDistance.x + slightlyMarkedDistance.y*slightlyMarkedDistance.y) )
					{
						slightlyMarkedOfs = ofs - handleGeo.pos();
						pSlightlyMarked = &slider;
						slightlyMarkedDistance = distance;
					}

				}
			}
		}
	}

	if (pFullyMarked)
	{
		if (pOfsOutput)
			*pOfsOutput = fullyMarkedOfs;
		return pFullyMarked;
	}
	else
	{
		if (pOfsOutput)
			*pOfsOutput = slightlyMarkedOfs;
		return pSlightlyMarked;
	}
}

//____ _markSliderHandle() __________________________________________________________

void WgMultiSlider::_markSliderHandle(Slider * pSlider)
{
	if (pSlider)
		m_hoveredSliderHandle = int(pSlider - &m_sliders.front());
	else
		m_hoveredSliderHandle = -1;

	_updateSliderStates();
}

//____ _selectSliderHandle() ________________________________________________________

void WgMultiSlider::_selectSliderHandle(Slider * pSlider)
{
	if (pSlider && m_selectedSliderHandle == pSlider - &m_sliders.front())
	    return;											// Already selected, nothing to do.

	// Select this slider

	if (pSlider)
	{
		// Clear drag related information

		m_dragFraction = { 0.f,0.f };
		m_dragStartFraction = pSlider->handlePos;
		m_finetuneRemainder = { 0,0 };
		m_selectedSliderHandle = int(pSlider - &m_sliders.front());
	}
	else
		m_selectedSliderHandle = -1;

	_updateSliderStates();
}

//____ _setSliderStates() ___________________________________________________

void WgMultiSlider::_setSliderStates(Slider& slider, WgState newSliderState, WgState newHandleState)
{
	// Request render where needed

	wg::Skin * pSliderSkin = slider.pBgSkin ? slider.pBgSkin.rawPtr() : m_pDefaultBgSkin.rawPtr();
	wg::Skin * pHandleSkin = slider.pHandleSkin ? slider.pHandleSkin.rawPtr() : m_pDefaultHandleSkin.rawPtr();

	if (pSliderSkin && !_skinDirtyRect(pSliderSkin, {0,0,100,100}, m_scale, slider.sliderState, newSliderState).isEmpty())
		_requestRenderSlider(&slider);
	else if (pHandleSkin  && !_skinDirtyRect(pHandleSkin, {0,0,100,100}, m_scale, slider.handleState, newHandleState).isEmpty())
		_requestRenderHandle(&slider);

	// Possibly send messages

	WgEventHandler * pHandler = _eventHandler();
	if (pHandler)
	{
		if( !slider.sliderState.isHovered() && newSliderState.isHovered() )
			pHandler->QueueEvent(new WgEvent::SliderEnter(this, slider.id));

		if( slider.sliderState.isHovered() && !newSliderState.isHovered() )
			pHandler->QueueEvent(new WgEvent::SliderLeave(this, slider.id));

		if( !slider.handleState.isHovered() && newHandleState.isHovered() )
			pHandler->QueueEvent(new WgEvent::SliderHandleEnter(this, slider.id));

		if( slider.handleState.isHovered() && !newHandleState.isHovered() )
			pHandler->QueueEvent(new WgEvent::SliderHandleLeave(this, slider.id));
	}

	// Set the states

	slider.sliderState = newSliderState;
	slider.handleState = newHandleState;
}


//____ _requestRenderSlider() _________________________________________________

void WgMultiSlider::_requestRenderSlider(Slider * pSlider)
{
	WgRect sliderGeo = _sliderGeo(*pSlider, PixelSize());
	WgRect handleGeo = _sliderHandleGeo(*pSlider, sliderGeo);

	wg::Skin_p	pBgSkin = pSlider->pBgSkin ? pSlider->pBgSkin : m_pDefaultBgSkin;

	if (pBgSkin)
	{
		WgRect sliderSkinGeo = _sliderSkinGeo(*pSlider, sliderGeo);
		handleGeo.growToContain(sliderSkinGeo);
	}

	_requestRender(handleGeo);
}

//____ _requestRenderHandle() _________________________________________________

void WgMultiSlider::_requestRenderHandle(Slider * pSlider)
{
	WgRect sliderGeo = _sliderGeo(*pSlider, PixelSize());
	WgRect handleGeo = _sliderHandleGeo(*pSlider, sliderGeo);

	_requestRender(handleGeo);
}


//____ _ updatePointerStyle() _________________________________________________

void WgMultiSlider::_updatePointerStyle(WgCoord pointerOfs)
{
	Slider * pMarked = nullptr;

	if (m_selectedSliderHandle != -1)
		pMarked = &m_sliders[m_selectedSliderHandle];
	else if (m_selectedSlider != -1)
		pMarked = &m_sliders[m_selectedSlider];
	else
		pMarked = _markedSliderHandle(pointerOfs);

	if (!pMarked && (m_pressMode == PressMode::SetValue || m_pressMode == PressMode::MultiSetValue))
		pMarked = _markedSlider(pointerOfs);

	if( pMarked )
	{
		switch (pMarked->origo)
		{
		case WgOrigo::North:
		case WgOrigo::South:
			m_pointerStyle = WgPointerStyle::ResizeNS;
			break;
		case WgOrigo::West:
		case WgOrigo::East:
			m_pointerStyle = WgPointerStyle::ResizeWE;
			break;
		default:
			m_pointerStyle = WgPointerStyle::ResizeAll;
			break;
		}
	}
	else
		m_pointerStyle = WgPointerStyle::Arrow;
}

bool WgMultiSlider::GetSliderEnabled(int iSliderID)
{
    for (auto &slider : m_sliders)
    {
        if (slider.id == iSliderID)
        {
            return !slider.sliderState.isDisabled();
        }
    }
    return false;
}

void WgMultiSlider::SetSliderState(int iSliderID, bool bEnabled)
{
    for (auto &slider : m_sliders)
    {
        if (slider.id == iSliderID)
        {
            if(!bEnabled)
                _setSliderStates(slider, WgStateEnum::Disabled, WgStateEnum::Disabled);
            else
                _setSliderStates(slider, WgStateEnum::Default, WgStateEnum::Default);
            break;
        }
    }
}

//____ _updateSliderStates() __________________________________________________

void WgMultiSlider::_updateSliderStates()
{

	if (m_state.isDisabled())
	{
		for (auto& slider : m_sliders)
		{
			if (!slider.sliderState.isDisabled())
				_setSliderStates(slider, WgStateEnum::Disabled, WgStateEnum::Disabled);
		}
		return;
	}

	// Update sliderStates (states for background)

	if (m_selectedSlider >= 0 || m_selectedSliderHandle >= 0)
	{
		// Fullfills the following rules:
		// 1. If some HANDLE is SELECTED -> That sliders background is PRESSED, no other background is HOVERED.
		// 2. If some SLIDER is PRESSED -> That sliders background is PRESSED, no other background is HOVERED.

		Slider& selected = m_sliders[m_selectedSliderHandle >= 0 ? m_selectedSliderHandle : m_selectedSlider];
		for ( auto& slider : m_sliders )
		{
			if (&slider == &selected)
			{
				if(!slider.sliderState.isPressed() )
					_setSliderStates(slider, slider.sliderState + WgStateEnum::Pressed, slider.handleState );
			}
			else
			{
				if( slider.sliderState.isHovered())
					_setSliderStates(slider, slider.sliderState - WgStateEnum::Hovered, slider.handleState);
			}
		}

	}
	else if (m_pressMode == PressMode::MultiSetValue && m_state.isPressed())
	{
		// Fullfills the following rules:
		// 3. If DRAW MODE and mouse pressed over background or handle -> Background is PRESSED.

		if (m_hoveredSlider >= 0 || m_hoveredSliderHandle >= 0)
		{
			Slider& hovered = m_sliders[m_hoveredSliderHandle >= 0 ? m_hoveredSliderHandle : m_hoveredSlider];
			for (auto& slider : m_sliders)
			{
				if (&slider == &hovered)
				{
					if (!slider.sliderState.isPressed())
						_setSliderStates(slider, slider.sliderState + WgStateEnum::Pressed, slider.handleState);
				}
				else
				{
					if (slider.sliderState.isHovered())
						_setSliderStates(slider, slider.sliderState - WgStateEnum::Hovered, slider.handleState);
				}
			}
		}
		else
		{
			for (auto& slider : m_sliders)
			{
				if (slider.sliderState.isHovered())
					_setSliderStates(slider, slider.sliderState - WgStateEnum::Hovered, slider.handleState);
			}
		}
	}
	else
	{
		// Fullfills the following rules:
		// 4. If mouse over selection area OR handle -> That sliders background is HOVERED.

		if( m_hoveredSlider >= 0 || m_hoveredSliderHandle >= 0 )
		{
			Slider& hovered = m_sliders[m_hoveredSliderHandle >= 0 ? m_hoveredSliderHandle : m_hoveredSlider];
			for (auto& slider : m_sliders)
			{
				if (&slider == &hovered)
				{
					if( slider.sliderState.isPressed() )
						_setSliderStates(slider, slider.sliderState - WgStateEnum::Pressed, slider.handleState);
					else if (!slider.sliderState.isHovered())
						_setSliderStates(slider, slider.sliderState + WgStateEnum::Hovered, slider.handleState);
				}
				else
				{
					if (slider.sliderState.isHovered())
						_setSliderStates(slider, slider.sliderState - WgStateEnum::Hovered, slider.handleState);
				}
			}
		}
		else
		{
			for (auto& slider : m_sliders)
			{
				if (slider.sliderState.isHovered())
					_setSliderStates(slider, slider.sliderState - WgStateEnum::Hovered, slider.handleState);
			}
		}
	}

	// Update handle states

	if (m_bGhostHandle)
	{
		// Fullfills the following rules:
		// 1. If GHOST mode -> All handles follows their backgrounds states.

		for (auto& slider : m_sliders)
		{
			if( slider.handleState != slider.sliderState )
				_setSliderStates(slider, slider.sliderState, slider.sliderState);
		}
	}
	else if (m_selectedSliderHandle >= 0)
	{
		// Fullfills the following rules:
		// 2. If a handle is SELECTED -> That handle is PRESSED, no other handle is HOVERED.

		Slider& selected = m_sliders[m_selectedSliderHandle];
		for (auto& slider : m_sliders)
		{
			if (&slider == &selected)
			{
				if (!slider.handleState.isPressed())
					_setSliderStates(slider, slider.sliderState, slider.handleState + WgStateEnum::Pressed );
			}
			else
			{
				if (slider.handleState.isHovered())
					_setSliderStates(slider, slider.sliderState, slider.handleState - WgStateEnum::Hovered);
			}
		}

	}
	else if (m_pressMode == PressMode::MultiSetValue && m_state.isPressed())
	{
		// Fullfills the following rules:
		// 3. If DRAW MODE and mouse pressed over background or handle -> Handle is PRESSED.

		if (m_hoveredSliderHandle >= 0)
		{
			Slider& hovered = m_sliders[m_hoveredSliderHandle];
			for (auto& slider : m_sliders)
			{
				if (&slider == &hovered)
				{
					if (!slider.handleState.isPressed())
						_setSliderStates(slider, slider.sliderState, slider.handleState + WgStateEnum::Pressed );
				}
				else
				{
					if (slider.handleState.isHovered())
						_setSliderStates(slider, slider.sliderState, slider.handleState - WgStateEnum::Hovered);
				}
			}
		}
		else
		{
			for (auto& slider : m_sliders)
			{
				if (slider.handleState.isHovered())
					_setSliderStates(slider, slider.sliderState, slider.handleState - WgStateEnum::Hovered);
			}
		}
	}

	else if (m_hoveredSliderHandle >= 0)
	{
		// Fullfills the following rules:
		// 4. If a handle is HOVERED -> That handle is HOVERED.

		Slider& hovered = m_sliders[m_hoveredSliderHandle];
		for (auto& slider : m_sliders)
		{
			if (&slider == &hovered)
			{
				if (slider.handleState.isPressed())
					_setSliderStates(slider, slider.sliderState, slider.handleState - WgStateEnum::Pressed);
				if (!slider.handleState.isHovered())
					_setSliderStates(slider, slider.sliderState, slider.handleState + WgStateEnum::Hovered);
			}
			else
			{
				if (slider.handleState.isHovered())
					_setSliderStates(slider, slider.sliderState, slider.handleState - WgStateEnum::Hovered);
			}
		}
	}
	else
	{
		for (auto& slider : m_sliders)
		{
			if (slider.handleState.isHovered())
				_setSliderStates(slider, slider.sliderState, slider.handleState - WgStateEnum::Hovered);
		}
	}
}

void WgMultiSlider::SetSliderLast(int iSliderID)
{
    auto kSliderIter = std::find_if(std::begin(m_sliders), std::end(m_sliders), [=](Slider kSlider)
                                 { return kSlider.id == iSliderID; });

    std::iter_swap(kSliderIter, m_sliders.end()-1);

}

//____ _onEvent() _____________________________________________________________

void WgMultiSlider::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	//TODO: Handle swallowing of events.
	//TODO: Send SliderPressed events for all mouse buttons, not just button 1.
	//TODO: Detect release of modifier keys for finetune and axislock and handle it in a better way.

	bool	bSwallow = false;

	switch (pEvent->Type())
	{
		case WG_EVENT_KEY_RELEASE:
		{
			// Check if a modifier key has been released and act accordingly.


			WgModifierKeys modKeys = pEvent->ModKeys();

			if( (modKeys & m_finetuneModifier) != m_finetuneModifier )
				m_finetuneRemainder = { 0,0 };

			if ((modKeys & m_axisLockModifier) != m_axisLockModifier)
				m_axisLockState = AxisLockState::Unlocked;

			WgWidget::_onEvent(pEvent, pHandler);
			break;
		}


		case WG_EVENT_MOUSE_LEAVE:
			m_state.setHovered(false);

			if (m_selectedSliderHandle == -1)
			{
				m_hoveredSlider = -1;
				m_hoveredSliderHandle = -1;
				_updateSliderStates();
			}
			break;

		case WG_EVENT_MOUSE_ENTER:
			m_state.setHovered(true);
												// No break on purpose!
		case WG_EVENT_MOUSE_MOVE:
			if (m_selectedSliderHandle == -1)
			{
				Slider * p = _markedSliderHandle(pEvent->PointerPixelPos() );

				if(p)
					_markSliderHandle(p);
				else
				{
					m_hoveredSlider = -1;
					m_hoveredSliderHandle = -1;

					p = _markedSlider(pEvent->PointerPixelPos());
					if (p)
						m_hoveredSlider = int(p - &m_sliders.front());
					else
						int dbg = 1;

					_updateSliderStates();
				}
			}
			_updatePointerStyle(pEvent->PointerPixelPos());
			WgWidget::_onEvent(pEvent, pHandler);
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			if (m_state.isDisabled())
				break;

			const WgEvent::MouseButtonPress * pEv = static_cast<const WgEvent::MouseButtonPress*>(pEvent);
			WgCoord	pointerPos = pEvent->PointerPixelPos();


			// Find pressed slider and whether handle was pressed

			WgCoord markOfs;
			Slider * pMarked = _markedSliderHandle(pointerPos, nullptr);
			bool		bHandlePressed = pMarked;		// If press really was on handle, needed for message.

			if(!bHandlePressed && !m_bOnlyHandlesPressable)
				pMarked = _markedSlider(pointerPos, &markOfs);

			// Queue the event. Need to do this before we update any handle position.

			if (pMarked)
			{
				m_ignoredPresses[pEv->Button()] = false;

				// Set pressOfs for the event

				WgOrigo pressOfs = WgOrigo::Center;

				if (!bHandlePressed)
				{
					WgSize widgetContentSize = m_pSkin ? PixelSize() - _skinContentPadding(m_pSkin, m_scale) : PixelSize();
					WgCoordF relPos = { markOfs.x / (pMarked->geo.w*widgetContentSize.w), markOfs.y / (pMarked->geo.h*widgetContentSize.h) };

					if (pMarked->origo == WgOrigo::West || pMarked->origo == WgOrigo::East)		// Horizontal slider
					{
						if (relPos.x < pMarked->handlePos.x)
							pressOfs = WgOrigo::West;
						else
							pressOfs = WgOrigo::East;
					}
					else if (pMarked->origo == WgOrigo::North || pMarked->origo == WgOrigo::South)		// Vertical slider
					{
						if (relPos.y < pMarked->handlePos.y)
							pressOfs = WgOrigo::North;
						else
							pressOfs = WgOrigo::South;
					}
					else																// 2D slider
					{
						if (relPos.y < pMarked->handlePos.y)
						{
							if (relPos.x < pMarked->handlePos.x)
								pressOfs = WgOrigo::NorthWest;
							else
								pressOfs = WgOrigo::NorthEast;
						}
						else
						{
							if (relPos.x < pMarked->handlePos.x)
								pressOfs = WgOrigo::SouthWest;
							else
								pressOfs = WgOrigo::SouthEast;
						}
					}
				}

				pHandler->QueueEvent(new WgEvent::SliderPressed(this, pMarked->id, pEv->Button(), pressOfs));
			}
			else
			{
				m_ignoredPresses[pEv->Button()] = true;
				WgWidget::_onEvent(pEvent, pHandler);
				break;
			}


			// Do everyting else

            if (pEv->Button() != 1)
                break;

			m_state.setPressed(true);

            if(m_overrideModifier != WG_MODKEY_NONE && (pEv->ModKeys() == m_overrideModifier))
                break;

            GrabFocus();

            bool bStaticMode = (m_staticModifier != WG_MODKEY_NONE && (pEv->ModKeys() == m_staticModifier));


			if (bHandlePressed)
            {
                _selectSliderHandle(pMarked);
            }
            else if (pMarked)
            {
                WgSize widgetContentSize = m_pSkin ? PixelSize() - _skinContentPadding( m_pSkin, m_scale) : PixelSize();

                m_selectedSlider = int(pMarked - &m_sliders.front());
                m_selectedSliderHandle = -1;

                // Convert the press offset to fraction.

                WgCoordF relPos = { markOfs.x / (pMarked->geo.w*widgetContentSize.w), markOfs.y / (pMarked->geo.h*widgetContentSize.h) };

                // In PressMode SetValue and MultiSetValue we set the value directly.

                if (!bStaticMode && (m_pressMode == PressMode::SetValue || m_pressMode == PressMode::MultiSetValue))
                {
                    if (m_bPassive)
                        _calcSendValue(*pMarked, relPos);
                    else
                        _setHandlePosition(*pMarked, relPos);

                    m_hoveredSliderHandle = int(pMarked - &m_sliders.front());
                    m_hoveredSlider = -1;
                }

                if (!bStaticMode && m_pressMode == PressMode::SetValue)
                {
                    _selectSliderHandle(pMarked);				// In SetValue mode we actually select the handle

                    // HACK:
                    // In passive mode we need to set dragStartFraction directly since
                    // slider.handlePos is out of date (waiting to be set externally).

                    if( m_bPassive )
                    {
                        relPos = _alignPosToStep(*pMarked, relPos);

                        wg::limit(relPos.x, 0.f, 1.f);
                        wg::limit(relPos.y, 0.f, 1.f);

                        m_dragStartFraction = relPos;
                    }
                }
                else
                {
                    m_selectedSlider = -1;
                    _updateSliderStates();
                }
			}

			break;
		}

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{

			const WgEvent::MouseButtonRelease * p = static_cast<const WgEvent::MouseButtonRelease*>(pEvent);

			if( m_ignoredPresses[p->Button()] == true )
			{
//				m_ignoredPresses[p->Button()] = false;		// Can not clear flag here, CLICK & DOUBLE_CLICK comes after this call.
				WgWidget::_onEvent(pEvent, pHandler);
				break;
			}


			if (!m_state.isDisabled() && p->Button() == 1)
			{
				m_state.setPressed(false);

				m_selectedSlider = -1;
				m_selectedSliderHandle = -1;

				m_hoveredSliderHandle = -1;
				m_hoveredSlider = -1;

				Slider * p = _markedSliderHandle(pEvent->PointerPixelPos());
				if (p)
					m_hoveredSliderHandle = int(p - &m_sliders.front());
				else
				{
					p = _markedSlider(pEvent->PointerPixelPos());
					if (p)
						m_hoveredSlider = int(p - &m_sliders.front());
				}

				_updateSliderStates();
			}
			break;
		}

		case WG_EVENT_MOUSEBUTTON_DRAG:
		{
			const WgEvent::MouseButtonDrag * pEv = static_cast<const WgEvent::MouseButtonDrag*>(pEvent);

			if( m_ignoredPresses[pEv->Button()] == true )
			{
				WgWidget::_onEvent(pEvent, pHandler);
				break;
			}

            if( pEv->Button() != 1 || m_state.isDisabled() || (m_overrideModifier != WG_MODKEY_NONE && (pEv->ModKeys() == m_overrideModifier) ) )
                break;

			// If a modKey that we don't support is pressed, we ignore all modkeys.

			int modKeys = pEv->ModKeys();
			if (modKeys != (modKeys & (m_finetuneModifier | m_axisLockModifier)))
				modKeys = 0;

            // StaticMode is handled separately. Don't update anything, but call the callback if present.
            // Currently no support for axis-lock, finetune or sending message.

            if (m_staticModifier != WG_MODKEY_NONE && (pEv->ModKeys() == m_staticModifier) )
            {
                if (m_selectedSliderHandle >= 0)
                {
                    Slider& slider = m_sliders[m_selectedSliderHandle];

                    //

                    WgRect sliderGeo = _sliderGeo(slider, PixelSize());

                    WgCoord dragNow = pEv->DraggedNowPixels();

                    WgCoordF movement;

                    movement.x = sliderGeo.w == 0 ? 0 : ((float) dragNow.x) / (sliderGeo.w-0.0001);
                    movement.y = sliderGeo.h == 0 ? 0 : ((float) dragNow.y) / (sliderGeo.h-0.0001);

                    if (m_staticCallback)
                        m_staticCallback(slider.id, movement.x, movement.y);

                }

            }
			else
			{
				WgCoordF	fraction;

				if (m_selectedSliderHandle >= 0)
				{
					Slider& slider = m_sliders[m_selectedSliderHandle];

					//

					WgRect sliderGeo = _sliderGeo(slider, PixelSize());

					WgCoord dragNow = pEv->DraggedNowPixels();

					WgCoordF movement;


					if (modKeys & m_finetuneModifier)
					{
						int pixelStepSize = m_finetuneStepSize == 0 ? m_scale / WG_SCALE_BASE : m_finetuneStepSize * m_scale / WG_SCALE_BASE;
						WgCoordF stepIncrement;

						if (m_finetuneStepIncrement == 0.f)
						{
							stepIncrement.x = 1 / (sliderGeo.w - 0.0001f);
							stepIncrement.y = 1 / (sliderGeo.h - 0.0001f);
						}
						else
						{
							stepIncrement.x = m_finetuneStepIncrement;
							stepIncrement.y = m_finetuneStepIncrement;
						}

						m_finetuneRemainder += dragNow;

						movement.x = (m_finetuneRemainder.x / pixelStepSize) * stepIncrement.x;
						movement.y = (m_finetuneRemainder.y / pixelStepSize) * stepIncrement.y;

						m_finetuneRemainder.x %= pixelStepSize;
						m_finetuneRemainder.y %= pixelStepSize;
					}
					else
					{
						movement.x = sliderGeo.w == 0 ? 0 : ((float) dragNow.x) / (sliderGeo.w-0.0001f);
						movement.y = sliderGeo.h == 0 ? 0 : ((float) dragNow.y) / (sliderGeo.h-0.0001f);

						m_finetuneRemainder = { 0,0 };
					}

					if (modKeys & m_axisLockModifier && slider.is2D)
					{
						switch (m_axisLockState)
						{
						case AxisLockState::Unlocked:
						{
							m_axisLockPosition = pEv->PrevPixelPos();
							m_axisLockFraction = m_dragFraction;
							m_axisLockState = AxisLockState::Locking;
							break;
						}

						case AxisLockState::Locking:
						{
							WgCoord drag = pEv->CurrPixelPos() - m_axisLockPosition;
							drag.x = abs(drag.x);
							drag.y = abs(drag.y);

							if (drag.x >= 3 && drag.x > drag.y * 2)
								m_axisLockState = AxisLockState::Horizontal;
							else if (drag.y >= 3 && drag.y > drag.x * 2)
								m_axisLockState = AxisLockState::Vertical;
							break;
						}
						case AxisLockState::Horizontal:
							m_dragFraction.x += movement.x;
							break;
						case AxisLockState::Vertical:
							m_dragFraction.y += movement.y;
							break;
						default:
							assert(false);							// Should never get here...
						}
					}
					else
					{
						m_dragFraction += movement;
					}

					fraction = m_dragStartFraction + m_dragFraction;

					wg::limit(fraction.x, 0.f, 1.f);
					wg::limit(fraction.y, 0.f, 1.f);

					if (m_bDeltaDrag)
						m_dragFraction = fraction - m_dragStartFraction;

					if (m_bPassive)
						_calcSendValue(slider, fraction);
					else
						_setHandlePosition(slider, fraction);
				}
				else
				{
					if (m_pressMode == PressMode::MultiSetValue)
					{

						WgCoord prevPos = pEv->PrevPixelPos();
						WgCoord currPos = pEv->PointerPixelPos();

						for (auto& slider : m_sliders)
						{
							WgRect sliderGeo = _sliderSkinGeo(slider, _sliderGeo(slider, PixelSize()));
							WgRect sliderMarkGeo = sliderGeo + (slider.sliderMarkExtension.isEmpty() ? m_defaultSliderMarkExtension : slider.sliderMarkExtension);

							if (prevPos.x < sliderMarkGeo.x && currPos.x > sliderMarkGeo.x)
								int a = 0;

							if (sliderMarkGeo.intersectsWithOrContains(prevPos, currPos) )
							{
								WgCoord clippedPrevPos = prevPos;
								WgCoord clippedCurrPos = currPos;
								sliderMarkGeo.clipLine(&clippedPrevPos, &clippedCurrPos);

								WgCoord	handlePos = clippedCurrPos - sliderGeo.pos();

								fraction.x = sliderGeo.w == 0 ? 0 : handlePos.x / (float)sliderGeo.w;
								fraction.y = sliderGeo.h == 0 ? 0 : handlePos.y / (float)sliderGeo.h;

								wg::limit(fraction.x, 0.f, 1.f);
								wg::limit(fraction.y, 0.f, 1.f);

								if (m_bPassive)
									_calcSendValue(slider, fraction);
								else
									_setHandlePosition(slider, fraction);

								if (modKeys & m_finetuneModifier)
								{
									_selectSliderHandle(&slider);
									break;										// Only one can be selected to finetune.
								}
							}
						}
/*
						WgCoord		handlePos;
						pAffectedSlider = _markedSlider(pEv->CurrPixelPos(), &handlePos);
						if (pAffectedSlider)
						{
							WgRect sliderGeo = _sliderGeo(*pAffectedSlider, PixelSize());

							fraction.x = sliderGeo.w == 0 ? 0 : handlePos.x / (float)sliderGeo.w;
							fraction.y = sliderGeo.h == 0 ? 0 : handlePos.y / (float)sliderGeo.h;

							if (pEv->ModKeys() == m_finetuneModifier)
								_selectSliderHandle(pAffectedSlider);
						}
*/
					}
				}


			}

			break;
		}

		case WG_EVENT_MOUSEBUTTON_CLICK:
		case WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK:
		case WG_EVENT_MOUSEBUTTON_REPEAT:
		{
			const WgEvent::MouseButtonDrag * pEv = static_cast<const WgEvent::MouseButtonDrag*>(pEvent);
			if( m_ignoredPresses[pEv->Button()] == true )
				WgWidget::_onEvent(pEvent, pHandler);

			break;
		}

		default:
			WgWidget::_onEvent(pEvent, pHandler);
			break;
	}
}

//____ _onCloneContent() _______________________________________________________

void WgMultiSlider::_onCloneContent( const WgWidget * _pOrg )
{
	WgMultiSlider * pOrg = (WgMultiSlider*) _pOrg;

}

//____ _onRender() _____________________________________________________________

void WgMultiSlider::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	WgWidget::_onRender(pDevice, _canvas, _window);

	for (auto& slider : m_sliders)
	{
		if( !slider.bVisible )
			continue;

		WgRect sliderGeo = _sliderGeo(slider, _canvas);

		wg::Skin_p pBgSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;
		if (pBgSkin)
		{
			WgRect bgGeo = _sliderSkinGeo(slider, sliderGeo);

			if( bgGeo.isOverlapping(pDevice->clipBounds()/64) )
			{
				WgState	emptyPartState = slider.sliderState;
				WgState filledPartState = slider.sliderState;
				filledPartState.setSelekted(true);

                if( _skinDirtyRect(pBgSkin, {0,0,100,100}, m_scale, emptyPartState, filledPartState).isEmpty() )
                    _renderSkin( pBgSkin, pDevice, emptyPartState, bgGeo, m_scale);
				else
				{
					// Different parts of background should be rendered in different states.
					// We need to split the background into one "filled" part and one or two "empty" parts.

					WgCoord divider = { sliderGeo.x + (int)(slider.handlePos.x*sliderGeo.w), sliderGeo.y + (int)(slider.handlePos.y*sliderGeo.h) };

					WgRect filledPart;
					WgRect emptyPart1;
					WgRect emptyPart2;

					switch (slider.origo)
					{
					case WgOrigo::NorthWest:
						filledPart = { bgGeo.x,bgGeo.y,divider.x - bgGeo.x,divider.y - bgGeo.y };
						emptyPart1 = { divider.x, bgGeo.y, bgGeo.x + bgGeo.w - divider.x, divider.y - bgGeo.y };
						emptyPart2 = { bgGeo.x,divider.y,bgGeo.w, bgGeo.y + bgGeo.h - divider.y };
						break;
					case WgOrigo::North:
						filledPart = { bgGeo.x,bgGeo.y,bgGeo.w,divider.y - bgGeo.y };
						emptyPart1 = { bgGeo.x, divider.y, bgGeo.w, bgGeo.y + bgGeo.h - divider.y };
						break;
					case WgOrigo::NorthEast:
						emptyPart1 = { bgGeo.x,bgGeo.y,divider.x - bgGeo.x,divider.y - bgGeo.y };
						filledPart = { divider.x, bgGeo.y, bgGeo.x + bgGeo.w - divider.x, divider.y - bgGeo.y };
						emptyPart2 = { bgGeo.x,divider.y,bgGeo.w, bgGeo.y + bgGeo.h - divider.y };
						break;
					case WgOrigo::East:
						emptyPart1 = { bgGeo.x,bgGeo.y,divider.x - bgGeo.x,bgGeo.h };
						filledPart = { divider.x, bgGeo.y, bgGeo.x + bgGeo.w - divider.x, bgGeo.h };
						break;
					case WgOrigo::SouthEast:
						emptyPart1 = { bgGeo.x,bgGeo.y,bgGeo.w, divider.y - bgGeo.y };
						emptyPart2 = { bgGeo.x,divider.y,divider.x - bgGeo.x,bgGeo.y + bgGeo.h - divider.y };
						filledPart = { divider.x, divider.y, bgGeo.x + bgGeo.w - divider.x, bgGeo.y + bgGeo.h - divider.y };
						break;
					case WgOrigo::South:
						emptyPart1 = { bgGeo.x,bgGeo.y,bgGeo.w,divider.y - bgGeo.y };
						filledPart = { bgGeo.x, divider.y, bgGeo.w, bgGeo.y + bgGeo.h - divider.y };
						break;
					case WgOrigo::SouthWest:
						emptyPart1 = { bgGeo.x,bgGeo.y,bgGeo.w, divider.y - bgGeo.y };
						filledPart = { bgGeo.x,divider.y,divider.x - bgGeo.x,bgGeo.y + bgGeo.h - divider.y };
						emptyPart2 = { divider.x, divider.y, bgGeo.x + bgGeo.w - divider.x, bgGeo.y + bgGeo.h - divider.y };
						break;
					case WgOrigo::West:
						filledPart = { bgGeo.x,bgGeo.y,divider.x - bgGeo.x,bgGeo.h };
						emptyPart1 = { divider.x, bgGeo.y, bgGeo.x + bgGeo.w - divider.x, bgGeo.h };
						break;

					default:
						assert(0);
					}

					// Prepare for custom cliplists

					const WgRect * pOldClipList = pDevice->clipList();
					int     oldClipListSize = pDevice->clipListSize();

					int allocSize = oldClipListSize*sizeof(WgRect)*2;
					WgRect * pRects = (WgRect*) wg::GfxBase::memStackAlloc( allocSize );

					// Generate cliplist for filledPart

					int nRects = 0;
					for( int i = 0 ; i < oldClipListSize ; i++ )
					{
						pRects[nRects] = WgRect::overlap(pOldClipList[i], filledPart*64 );
						if( !pRects[nRects].isEmpty() )
							nRects++;
					}

					// Render filled part

					if( nRects > 0 )
					{
						pDevice->setClipList(nRects, pRects);
						_renderSkin( pBgSkin, pDevice, filledPartState, bgGeo, m_scale);
					}

					// Generate cliplist for emptyPart1

					nRects = 0;
					for( int i = 0 ; i < oldClipListSize ; i++ )
					{
						pRects[nRects] = WgRect::overlap(pOldClipList[i], emptyPart1*64 );
						if( !pRects[nRects].isEmpty() )
							nRects++;
					}

					// Add cliplist for emptyPart2

					for( int i = 0 ; i < oldClipListSize ; i++ )
					{
						pRects[nRects] = WgRect::overlap(pOldClipList[i], emptyPart2*64 );
						if( !pRects[nRects].isEmpty() )
							nRects++;
					}

					// Render empty part

					if( nRects > 0 )
					{
						pDevice->setClipList(nRects, pRects);
						_renderSkin( pBgSkin, pDevice, emptyPartState, bgGeo, m_scale);
					}

					// Clean up

					pDevice->setClipList(oldClipListSize, pOldClipList);
					wg::GfxBase::memStackFree(allocSize);

				}

			}
		}

		wg::Skin_p pHandleSkin = slider.pHandleSkin ? slider.pHandleSkin : m_pDefaultHandleSkin;
		if (pHandleSkin)
		{
			WgRect sliderHandleGeo = _sliderHandleGeo(slider, sliderGeo);
			_renderSkin( pHandleSkin, pDevice, slider.handleState, sliderHandleGeo, m_scale);
		}
	}
}

//____ _onAlphaTest() ___________________________________________________________

bool WgMultiSlider::_onAlphaTest( const WgCoord& ofs )
{
	if (WgWidget::_onAlphaTest(ofs))
		return true;

	for (auto& slider : m_sliders)
	{
		WgRect sliderGeo = _sliderGeo(slider, PixelSize() );

		wg::Skin_p pBgSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;
		if (pBgSkin)
		{
			WgRect bgGeo = _sliderSkinGeo(slider, sliderGeo);

			if( bgGeo.contains(ofs) && _markTestSkin( pBgSkin, ofs-bgGeo.pos(), bgGeo.size(), slider.sliderState, m_markOpacity, m_scale) )
				return true;
		}

		if (!m_bGhostHandle)
		{
			wg::Skin_p pHandleSkin = slider.pHandleSkin ? slider.pHandleSkin : m_pDefaultHandleSkin;
			if (pHandleSkin)
			{
				WgRect handleGeo = _sliderHandleGeo(slider, sliderGeo);

				if (handleGeo.contains(ofs) && _markTestSkin( pHandleSkin, ofs - handleGeo.pos(), handleGeo.size(), slider.handleState, m_markOpacity, m_scale))
					return true;
			}
		}
	}

	return false;
}

//____ _setScale() ____________________________________________________________

void WgMultiSlider::_setScale(int scale)
{
	WgWidget::_setScale(scale);
}

//____ _updateHandlePos() ______________________________________________________

void WgMultiSlider::_updateHandlePos(Slider& slider)
{
	WgCoordF	handlePos;

	if (slider.is2D)
	{
		WgCoordF values;

		values.x = (slider.value[0] - slider.bounds[0].min) / (slider.bounds[0].max - slider.bounds[0].min);
		values.y = (slider.value[1] - slider.bounds[1].min) / (slider.bounds[1].max - slider.bounds[1].min);

		switch (slider.origo)
		{
		case WgOrigo::NorthWest:
			handlePos = values;
			break;
		case WgOrigo::NorthEast:
			handlePos.x = 1.f - values.x;
			handlePos.y = values.y;
			break;
		case WgOrigo::SouthEast:
			handlePos.x = 1.f - values.x;
			handlePos.y = 1.f - values.y;
			break;
		case WgOrigo::SouthWest:
			handlePos.x = values.x;
			handlePos.y = 1.f - values.y;
			break;
		default:
			assert(false);   // Should never get here!
		}
	}
	else
	{
		float value;

		value = (slider.value[0] - slider.bounds[0].min) / (slider.bounds[0].max - slider.bounds[0].min);

		switch (slider.origo)
		{
		case WgOrigo::West:
			handlePos.x = value;
			break;
		case WgOrigo::East:
			handlePos.x = 1.f -value;
			break;
		case WgOrigo::North:
			handlePos.y = value;
			break;
		case WgOrigo::South:
			handlePos.y = 1.f - value;
			break;
		default:
			assert(false);   // Should never get here!
		}
	}

	// Align position to even step sizes.

	handlePos = _alignPosToStep(slider, handlePos);

	// Limit range

	wg::limit(handlePos.x, 0.f, 1.f);
	wg::limit(handlePos.y, 0.f, 1.f);

	// Early out if nothing has changed

	if (handlePos == slider.handlePos)
		return;


	// Render what needs to be rendered

	WgRect sliderGeo = _sliderGeo(slider, PixelSize());

	WgRect oldHandleGeo = _sliderHandleGeo(slider, sliderGeo);

	slider.handlePos = handlePos;

	WgRect newHandleGeo = _sliderHandleGeo(slider, sliderGeo);

	if (newHandleGeo != oldHandleGeo)
	{
		_requestRender(oldHandleGeo);
		_requestRender(newHandleGeo);
	}

	wg::Skin_p pBgSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;
	if (pBgSkin)
	{
        wg::RectI dirty = _skinDirtyRect(pBgSkin, _sliderSkinGeo(slider, sliderGeo), m_scale, slider.handleState, slider.handleState + WgStateEnum::Selekted);
        if( !dirty.isEmpty() )
            _requestRender( dirty );
	}


	return;
}



//____ _updateGeo() ______________________________________________________

void WgMultiSlider::_updateGeo(Slider& slider)
{
	slider.geoState = 1;		// Update in progress

	SetGeoVisitor visitor(this, &slider);


	WgRectF newGeo = slider.pSetGeoFunc(visitor);

	if (newGeo != slider.geo)
	{
		WgRect	widgetGeo = PixelSize();

		WgRect oldSliderGeo = _sliderGeo(slider, widgetGeo);

		slider.geo = newGeo;

		WgRect newSliderGeo = _sliderGeo(slider, widgetGeo);

		// Check on pixel-geo as well, might be it has not been affected...

		if (newSliderGeo != oldSliderGeo)
		{
			WgRect oldGeo = _sliderHandleGeo(slider, oldSliderGeo);
			WgRect newGeo = _sliderHandleGeo(slider, newSliderGeo);

			// If we have background we just merge that into the geometries

			if (slider.pBgSkin || m_pDefaultBgSkin)
			{
				oldGeo.growToContain(_sliderSkinGeo(slider, oldSliderGeo));
				newGeo.growToContain(_sliderSkinGeo(slider, newSliderGeo));
			}

			_requestRender(oldGeo);
			_requestRender(newGeo);
		}

	}

	slider.geoState = 2;		// Update completed
}

//____ _sliderGeo() ___________________________________________________________

WgRect  WgMultiSlider::_sliderGeo(const Slider& slider, const WgRect& _canvas ) const
{
	WgRect canvas = m_pSkin ? _skinContentRect(m_pSkin, _canvas, m_state, m_scale ) : _canvas;

	WgRect r = { canvas.x + (int)(slider.geo.x * canvas.w + 0.5f), canvas.y + (int)(slider.geo.y * canvas.h + 0.5f), (int)(canvas.w * slider.geo.w), (int)(canvas.h * slider.geo.h) };

    if( m_bPointAlignSliders )
    {
        r = (r * WG_SCALE_BASE ) / m_scale;
        r = (r * m_scale) / WG_SCALE_BASE;
    }

    return r;
}

//____ _sliderSkinGeo() _______________________________________________________

WgRect  WgMultiSlider::_sliderSkinGeo(const Slider& slider, const WgRect& sliderGeo) const
{
	wg::Skin_p pSkin = slider.pBgSkin ? slider.pBgSkin : m_pDefaultBgSkin;

	if(pSkin)
	{
		WgRect bgGeo = sliderGeo;
		bgGeo -= _skinContentOfs( pSkin, m_state, m_scale);
		bgGeo += _skinContentPadding( pSkin, m_scale);

		WgSize min = _skinPreferredSize( pSkin, m_scale);

		return { bgGeo.pos(),std::max(bgGeo.w,min.w), std::max(bgGeo.h,min.h) };
	}
	else
		return sliderGeo;

}

//____ _sliderHandleGeo() _____________________________________________________

WgRect  WgMultiSlider::_sliderHandleGeo(const Slider& slider, const WgRect& sliderGeo) const
{
	wg::Skin_p pSkin = slider.pHandleSkin ? slider.pHandleSkin : m_pDefaultHandleSkin;

	WgSize sz = pSkin ? _skinPreferredSize( pSkin, m_scale) : WgSize(0,0);
	WgCoordF handleHotspot = slider.handleHotspot.x == -1.f ? m_defaultHandleHotspot : slider.handleHotspot;

	WgCoord pos = WgCoord( std::round(slider.handlePos.x * sliderGeo.w), std::round(slider.handlePos.y * sliderGeo.h) );
	pos += sliderGeo.pos();
	pos -= WgCoord( std::round(handleHotspot.x * sz.w), std::round(handleHotspot.y * sz.h) );

	return { pos,sz };
}


//____ invokeSetValueCallback() _____________________________________________

void WgMultiSlider::_invokeSetValueCallback(Slider& slider, float& value, float& value2)
{
	if (slider.pSetValueFunc)
	{
		SetValueVisitor v(this, &slider, value);
		value = slider.pSetValueFunc(v);
		value2 = NAN;
	}
	else if (slider.pSetValueFunc2D)
	{
		SetValueVisitor2D v(this, &slider, { value, value2 });
		WgCoordF values = slider.pSetValueFunc2D(v);
		value = values.x;
		value2 = values.y;
	}
}



//____ _setValue() ____________________________________________________________

float WgMultiSlider::_setValue(Slider& slider, float value, float value2, bool bSendOnUpdate )
{
	wg::limit(value, slider.bounds[0].min, slider.bounds[0].max);

	bool bUpdate = false;

	if(value != slider.value[0] )
	{
		slider.value[0] = value;
		bUpdate = true;
	}

	if ( !isnan(value2) )
	{
		wg::limit(value2, slider.bounds[1].min, slider.bounds[1].max);

		if(value2 != slider.value[1] )
		{
			slider.value[1] = value2;
			bUpdate = true;
		}
	}

	if( bUpdate )
	{
		// Update handle position

		_updateHandlePos(slider);

		// Update geo in case some sliders geo is affected

		_refreshSliderGeo();

		// Send values

		if( bSendOnUpdate )
			_sendValue(slider, value, value2);

	}
	return value;
}

//____ _alignPosToStep() ______________________________________________________

WgCoordF WgMultiSlider::_alignPosToStep(Slider& slider, WgCoordF pos) const
{
	// Get parameters controlled by X and Y axies.

	int x = -1;
	int y = -1;

	switch (slider.origo)
	{
	case WgOrigo::West:
	case WgOrigo::East:
		x = 0;
		break;

	case WgOrigo::North:
	case WgOrigo::South:
		y = 0;
		break;

	case WgOrigo::NorthWest:
	case WgOrigo::NorthEast:
	case WgOrigo::SouthEast:
	case WgOrigo::SouthWest:
		x = 0;
		y = 1;
		break;
	default:
		assert(false);   // Should never get here!
	}

	if (x >= 0)
	{
		if (slider.bounds[x].steps != 0)
		{
//			float stepPos = 1.f / (slider.bounds[x].steps - 1);
//			float stepSize = 1.f / (slider.bounds[x].steps);
//			int step = pos.x / stepSize;
//			pos.x = stepPos * step;

			float stepSize = 1.f / (slider.bounds[x].steps - 1);
			pos.x += stepSize / 2;
			pos.x -= fmod(pos.x, stepSize);
		}
	}

	if (y >= 0)
	{
		if (slider.bounds[y].steps != 0)
		{
			float stepSize = 1.f / (slider.bounds[y].steps / 1);
			pos.y += stepSize / 2;
			pos.y -= fmod(pos.y, stepSize);
		}
	}

	return pos;
}

//____ _calcSendValue() _______________________________________________________

WgCoordF WgMultiSlider::_calcSendValue(Slider& slider, WgCoordF pos)
{
	// Align position to even step sizes.

	pos = _alignPosToStep(slider, pos);

	// Limit range

	wg::limit(pos.x, 0.f, 1.f);
	wg::limit(pos.y, 0.f, 1.f);

	// Early out if nothing has changed

	if (pos == slider.handlePos)
		return pos;

	// Set handle position and handle re-rendering.

//	_requestRenderSlider(&slider);
//	slider.handlePos = pos;
//	_requestRenderSlider(&slider);

	// Calculate values from handle position

	float	value;
	float	value2;


	WgCoordF	handleFactor = _convertFactorPos(pos, slider.origo);

	if (slider.is2D)
	{
		value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.x;
		value2 = slider.bounds[1].min + (slider.bounds[1].max - slider.bounds[1].min)*handleFactor.y;
	}
	else
	{
		if (slider.origo == WgOrigo::West || slider.origo == WgOrigo::East )
			value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.x;
		else
			value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.y;
		value2 = NAN;
	}

	// Set values and possibly update any other affected sliders

	_sendValue(slider, value, value2);
	return pos;
}




//____ _sendValue() ____________________________________________________________

void WgMultiSlider::_sendValue(Slider& slider, float value, float value2)
{
    if(m_CheckStateForMove && !GetSliderEnabled(slider.id))
    {
        return;
    }

    // Callback
	if (m_callback)
		m_callback(slider.id, value, value2);

	// Send event

	WgEventHandler * pHandler = _eventHandler();
	if (pHandler)
		pHandler->QueueEvent(new WgEvent::SliderMoved(this, slider.id, value, value2));

}


//____ _setHandlePosition() ______________________________________________

WgCoordF WgMultiSlider::_setHandlePosition(Slider& slider, WgCoordF pos)
{
	// Align position to even step sizes.

	pos = _alignPosToStep(slider, pos);

	// Limit range

	wg::limit(pos.x, 0.f, 1.f);
	wg::limit(pos.y, 0.f, 1.f);

	// Early out if nothing has changed

	if (pos == slider.handlePos)
		return pos;

	// Set handle position and handle re-rendering.

	_requestRenderSlider(&slider);
	slider.handlePos = pos;
	_requestRenderSlider(&slider);

	// Calculate values from handle position

	float	value;
	float	value2;

	WgCoordF	handleFactor = _convertFactorPos(slider.handlePos, slider.origo);

	if (slider.is2D)
	{
		value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.x;
		value2 = slider.bounds[1].min + (slider.bounds[1].max - slider.bounds[1].min)*handleFactor.y;
	}
	else
	{
		if (slider.origo == WgOrigo::West || slider.origo == WgOrigo::East)
			value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.x;
		else
			value = slider.bounds[0].min + (slider.bounds[0].max - slider.bounds[0].min)*handleFactor.y;
		value2 = NAN;
	}

	// Set values and possibly update any other affected sliders

	_invokeSetValueCallback(slider, value, value2);
	_setValue(slider, value, value2, true);
	return pos;
}


//____ _convertFactorPos() ________________________________________________________

WgCoordF WgMultiSlider::_convertFactorPos(WgCoordF in, WgOrigo origo)
{
	WgCoordF	pos = in;

	switch (origo)
	{
	case WgOrigo::West:
	case WgOrigo::North:
	case WgOrigo::NorthWest:
		break;

	case WgOrigo::NorthEast:
	case WgOrigo::East:
		pos.x = 1.f - pos.x;
		break;

	case WgOrigo::SouthEast:
		pos.x = 1.f - pos.x;
		pos.y = 1.f - pos.y;
		break;

	case WgOrigo::South:
	case WgOrigo::SouthWest:
		pos.y = 1.f - pos.y;
		break;
	default:
		assert(false);   // Should never get here!
	}

	return pos;
}




//____ _refreshSliders() ___________________________________________________

void WgMultiSlider::_refreshSliders()
{
	for (auto& slider : m_sliders)
		slider.geoState = 0;

	for (auto& slider : m_sliders)
	{
		_updateHandlePos(slider);
		if(slider.geoState == 0 )
			_updateGeo(slider);
	}
}


//____ _refreshSliderGeo() ___________________________________________________

void WgMultiSlider::_refreshSliderGeo()
{
	for (auto& slider : m_sliders)
		slider.geoState = 0;

	for (auto& slider : m_sliders)
	{
		if (slider.geoState == 0)
			_updateGeo(slider);
	}
}

//____ _findSlider() __________________________________________________________

WgMultiSlider::Slider * WgMultiSlider::_findSlider(int sliderId)
{
	for (auto& slider : m_sliders)
		if (slider.id == sliderId)
			return &slider;

	return nullptr;
}

const WgMultiSlider::Slider * WgMultiSlider::_findSlider(int sliderId) const
{
    for (auto& slider : m_sliders)
        if (slider.id == sliderId)
            return &slider;

    return nullptr;
}


//____ SetValueVisitorBase::Constructor _______________________________________

WgMultiSlider::SetValueVisitorBase::SetValueVisitorBase(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider) : Visitor(pWidget,pSlider)
{
}

//____ SetValueVisitorBase::value() _______________________________________

float WgMultiSlider::SetValueVisitorBase::value(int sliderId)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || p->is2D)
		return NAN;

	return p->value[0];
}

//____ SetValueVisitorBase::setValue() _______________________________________

float WgMultiSlider::SetValueVisitorBase::setValue(int sliderId, float value)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || p->is2D)
		return NAN;

	bool bSendOnUpdate = (p == m_pSlider) ? false : true;
	return m_pWidget->_setValue(*p, value, NAN, bSendOnUpdate);
}

//____ SetValueVisitorBase::value2D() _______________________________________

WgCoordF WgMultiSlider::SetValueVisitorBase::value2D(int sliderId)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || !p->is2D)
		return WgCoordF(NAN,NAN);

	return { p->value[0], p->value[0] };
}

//____ SetValueVisitorBase::setValue2D() _______________________________________

WgCoordF WgMultiSlider::SetValueVisitorBase::setValue2D(int sliderId, WgCoordF value)
{
	Slider * p = m_pWidget->_findSlider(sliderId);
	if (!p || !p->is2D)
		return WgCoordF(NAN, NAN);

	bool bSendOnUpdate = (p == m_pSlider) ? false : true;
	m_pWidget->_setValue(*p, value.x, value.y, bSendOnUpdate);

	return { p->value[0], p->value[1] };
}


//____ SetValueVisitor::Constructor ___________________________________________

WgMultiSlider::SetValueVisitor::SetValueVisitor(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider, float value) : SetValueVisitorBase(pWidget,pSlider)
{
	m_value = value;
}

//____ SetValueVisitor::value() ___________________________________________

float WgMultiSlider::SetValueVisitor::value()
{
	return m_value;
}

//____ SetValueVisitor::valueBounds() ___________________________________________

WgMultiSlider::Bounds WgMultiSlider::SetValueVisitor::valueBounds()
{
	return m_pSlider->bounds[0];
}


//____ SetValueVisitor2D::Constructor ___________________________________________

WgMultiSlider::SetValueVisitor2D::SetValueVisitor2D(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider, WgCoordF value) : SetValueVisitorBase(pWidget, pSlider)
{
	m_value = value;
}

//____ SetValueVisitor2D::value2D() _______________________________________

WgCoordF WgMultiSlider::SetValueVisitor2D::value2D()
{
	return m_value;
}

//____ SetValueVisitor2D::valueBoundsX() ___________________________________________

WgMultiSlider::Bounds WgMultiSlider::SetValueVisitor2D::valueBoundsX()
{
	return m_pSlider->bounds[0];
}

//____ SetValueVisitor2D::valueBoundsY() ___________________________________________

WgMultiSlider::Bounds WgMultiSlider::SetValueVisitor2D::valueBoundsY()
{
	return m_pSlider->bounds[1];
}

//____ SetGeoVisitor::Constructor ___________________________________________________

WgMultiSlider::SetGeoVisitor::SetGeoVisitor(WgMultiSlider * pWidget, WgMultiSlider::Slider * pSlider) : Visitor(pWidget, pSlider)
{
}

//____ SetGeoVisitor::handlePos() ___________________________________________________

WgCoordF WgMultiSlider::SetGeoVisitor::handlePos(int sliderId)
{
	Slider * p = m_pWidget->_findSlider(sliderId);

	if (p)
		return p->handlePos;

	return WgCoordF(NAN, NAN);
}

//____ SetGeoVisitor::geo() ___________________________________________________

WgRectF WgMultiSlider::SetGeoVisitor::geo(int sliderId)
{
	Slider * pSlider = m_pWidget->_findSlider(sliderId);
	assert(pSlider->geoState != 1);

	if (pSlider->geoState == 0)
		m_pWidget->_updateGeo(*pSlider);

	return pSlider->geo;
}
