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

#include <wg2_rangeslider.h>
#include <wg2_gfxdevice.h>
#include <wg2_event.h>
#include <wg2_eventhandler.h>

using namespace wg::Util;

static const char	c_widgetType[] = { "RangeSlider" };

//____ constructor ____________________________________________________________

WgRangeSlider::WgRangeSlider()
{
}

//____ Destructor _____________________________________________________________

WgRangeSlider::~WgRangeSlider()
{
}

//____ Type() _________________________________________________________________

const char* WgRangeSlider::Type(void) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char* WgRangeSlider::GetClass()
{
	return c_widgetType;
}

//____ PreferredPixelSize() __________________________________________________________

WgSize WgRangeSlider::PreferredPixelSize() const
{
	if( m_preferredSize.w >= 0 && m_preferredSize.h >= 0 )
		return m_preferredSize;
	else
		return WgSize(4,4);
}

//____ SetPreferredSlideLength() _________________________________________

void WgRangeSlider::SetPreferredSlideLength(int pointLength)
{
	if (pointLength < 0)
	{
		//TODO: Error handling!
		return;
	}

	m_preferredSlideLength = pointLength;
	_updatePreferredSize();
}

//____ SetSkin() _________________________________________________________

void WgRangeSlider::SetSkin(wg::Skin * pSkin)
{
	WgWidget::SetSkin(pSkin);
}

//____ SetBeginHandleSkin() ___________________________________________________

void WgRangeSlider::SetBeginHandleSkin(wg::Skin * pSkin)
{
	if (pSkin != m_pBeginHandleSkin)
	{
		m_pBeginHandleSkin = pSkin;
		_requestRender();
		_updatePreferredSize();
	}
}

//____ SetEndHandleSkin() ___________________________________________________

void WgRangeSlider::SetEndHandleSkin(wg::Skin * pSkin)
{
	if (pSkin != m_pEndHandleSkin)
	{
		m_pEndHandleSkin = pSkin;
		_requestRender();
		_updatePreferredSize();
	}
}


//____ SetAxis() _________________________________________________________

void WgRangeSlider::SetAxis(wg::Axis axis)
{
	if (axis != m_axis)
	{
		m_axis = axis;
		_requestRender();
		_updatePreferredSize();
	}
}

//____ SetSteps() ________________________________________________________

void WgRangeSlider::SetSteps(int nbSteps)
{
	if (nbSteps < 0)
	{
		//TODO: Error handling!
		return;
	}

	if (nbSteps != m_nbSteps)
	{
		m_nbSteps = nbSteps;
		if( nbSteps > 0 )
			SetRange(m_rangeBegin, m_rangeEnd);		// Have the range aligned to the steps.
	}
}

//____ SetRange() _________________________________________________________

void WgRangeSlider::SetRange(float begin, float end)
{
	wg::limit(begin, 0.f, 1.f - m_minRange);
	wg::limit(end, begin + m_minRange, 1.f);

	if (begin != m_rangeBegin || end != m_rangeEnd)
		_setRange(begin,end);
}

//____ _onEvent() __________________________________________________________

void WgRangeSlider::_onEvent(const WgEvent::Event* _pEvent, WgEventHandler* pHandler)
{
	switch (_pEvent->Type())
	{
		case WG_EVENT_MOUSE_ENTER:
		case WG_EVENT_MOUSE_MOVE:
		{
			WgCoord pos = _pEvent->PointerPixelPos();

			if( _handleGeo(PixelSize(),true).contains(pos) != m_beginHandleState.isHovered() )
				_setHandleState(m_beginHandleState + wg::State::Hovered, true);

			if (_handleGeo(PixelSize(), false).contains(pos) != m_endHandleState.isHovered())
				_setHandleState(m_endHandleState + wg::State::Hovered, false);
			break;
		}

		case WG_EVENT_MOUSE_LEAVE:
			if (m_beginHandleState.isHovered() && !m_beginHandleState.isPressed() )
				_setHandleState(m_beginHandleState - wg::State::Hovered, true);

			if (m_endHandleState.isHovered() && !m_endHandleState.isPressed())
				_setHandleState(m_endHandleState - wg::State::Hovered, false);

			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			auto pMsg = static_cast<const WgEvent::MouseButtonEvent*>(_pEvent);

			if (pMsg->Button() != 1)
				break;

			WgRect	geo = PixelSize();
			WgCoord pos = pMsg->PointerPixelPos();
			WgRect  beginHandle = _handleGeo(geo,true);
			WgRect  endHandle = _handleGeo(geo, false);
			if (beginHandle.contains(pos))
			{
				_setHandleState(m_beginHandleState + wg::State::Pressed,true);
				m_valueAtPress = m_rangeBegin;
			}
			else if (endHandle.contains(pos))
			{
				_setHandleState(m_endHandleState + wg::State::Pressed, false);
				m_valueAtPress = m_rangeEnd;
			}
			else
			{
				//TODO: Handle click on background.
			}
			break;
		}

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			auto pMsg = static_cast<const WgEvent::MouseButtonEvent*>(_pEvent);

			if (pMsg->Button() != 1)
				break;

//				Coord pos = toLocal(pMsg->pointerPos());
//				Rect  handle = _handleGeo(m_size);
			if (m_beginHandleState.isPressed())
			{
				_setHandleState(m_beginHandleState - wg::State::Pressed, true);
			}
			else if (m_endHandleState.isPressed())
			{
				_setHandleState(m_endHandleState - wg::State::Pressed, false);
			}
			else
			{
				//TODO: Handle click on background.
			}
			break;

		}

		case WG_EVENT_MOUSEBUTTON_DRAG:
		{
			if (m_beginHandleState.isPressed() || m_endHandleState.isPressed())
			{
				auto pMsg = static_cast<const WgEvent::MouseButtonDrag*>(_pEvent);

				bool isBeginHandle = m_beginHandleState.isPressed();

				WgSize widgetSize = PixelSize();
				WgSize contentSize = m_pSkin ? widgetSize - _skinContentPadding(m_pSkin, m_scale) : widgetSize;
				WgSize handleSize = _handleGeo(widgetSize, isBeginHandle).size();
				WgCoord totalDrag = pMsg->DraggedTotalPixels();

				int slideLen;
				int slided;

				if (m_axis == wg::Axis::X)
				{
					slideLen = contentSize.w - handleSize.w;
					slided = totalDrag.x;
				}
				else
				{
					slideLen = contentSize.h - handleSize.h;
					slided = -totalDrag.y;
				}

				float newValue = m_valueAtPress + slided / (float) slideLen;
				wg::limit(newValue, 0.f, 1.f);

				if (isBeginHandle)
				{
					if (newValue > m_rangeEnd - m_minRange)
						newValue = m_rangeEnd - m_minRange;
					_setRange(newValue, m_rangeEnd);
				}
				else
				{
					if (newValue < m_rangeBegin + m_minRange)
						newValue = m_rangeBegin + m_minRange;
					_setRange(m_rangeBegin, newValue);
				}

				pHandler->QueueEvent(new WgEvent::RangeChanged(this,m_rangeBegin,m_rangeEnd));

			}
		}
        default:
            break;
	}
}


//____ _onCloneContent() _______________________________________________________

void WgRangeSlider::_onCloneContent( const WgWidget * _pOrg )
{
//	WgWidget::_onCloneContent( _pOrg );

	WgRangeSlider * pOrg = (WgRangeSlider*) _pOrg;

	m_preferredSize = pOrg->m_preferredSize;
}

//____ _onRender() __________________________________________________________

void WgRangeSlider::_onRender(wg::GfxDevice * pDevice, const WgRect& canvas, const WgRect& window)
{
	if (m_pSkin)
		_renderSkin(m_pSkin, pDevice, m_state, canvas, m_scale, m_rangeBegin, m_rangeEnd);

	if (m_pBeginHandleSkin)
		_renderSkin(m_pBeginHandleSkin, pDevice, m_beginHandleState, _handleGeo(canvas,true), m_scale, m_rangeBegin);

	if (m_pEndHandleSkin)
		_renderSkin(m_pEndHandleSkin, pDevice, m_endHandleState, _handleGeo(canvas, false), m_scale, m_rangeEnd);
}

//____ _onAlphaTest() ________________________________________________________

bool WgRangeSlider::_onAlphaTest(const WgCoord& ofs)
{
	bool bMarked = false;

	if (m_pSkin)
		bMarked = _markTestSkin(m_pSkin, ofs, WgRect(PixelSize()), m_state, m_markOpacity, m_scale, m_rangeBegin, m_rangeEnd);

	if (!bMarked && m_pBeginHandleSkin)
		bMarked = _markTestSkin(m_pBeginHandleSkin, ofs, _handleGeo(PixelSize(),true), m_beginHandleState, m_markOpacity, m_scale, m_rangeBegin);

	if (!bMarked && m_pEndHandleSkin)
		bMarked = _markTestSkin(m_pEndHandleSkin, ofs, _handleGeo(PixelSize(), false), m_endHandleState, m_markOpacity, m_scale, m_rangeEnd);

	return bMarked;
}


//____ _updatePreferredSize() ________________________________________________

void WgRangeSlider::_updatePreferredSize()
{
	WgSize sz	=	m_pBeginHandleSkin ? _skinPreferredSize(m_pBeginHandleSkin, m_scale) : WgSize( 4, 4 );
	WgSize sz2	=	m_pEndHandleSkin ? _skinPreferredSize(m_pEndHandleSkin, m_scale) : WgSize(4, 4);

	if (m_axis == wg::Axis::X)
		sz.w += m_preferredSlideLength*m_scale/WG_SCALE_BASE + sz2.w;
	else
		sz.h += m_preferredSlideLength * m_scale / WG_SCALE_BASE + sz2.h;

	if (m_pSkin)
	{
		sz += m_pSkin->_contentBorderSize(m_scale >> 6);
		sz = WgSize::max(sz, _skinPreferredSize(m_pSkin, m_scale));
	}

	if (sz != m_preferredSize)
	{
		m_preferredSize = sz;
		_requestResize();
	}
}

//____ _setRange() __________________________________________________________

void WgRangeSlider::_setRange(float begin, float end)
{
	if (m_nbSteps > 0)
	{
		int val = int(begin * m_nbSteps + 0.5f);
		begin = 1.f / val;

		val = int(end * m_nbSteps + 0.5f);
		end = 1.f / val;
	}

	WgSize size = PixelSize();

	if (begin != m_rangeBegin || end != m_rangeEnd)
	{
		WgRect changeRect;

		if (m_pSkin && !m_pSkin->_ignoresValue())
		{
			changeRect = PixelSize();
			m_rangeBegin = begin;
			m_rangeEnd = end;
		}
		else
		{
			if (begin != m_rangeBegin)
			{
				WgRect oldGeo = _handleGeo(size,true);

				m_rangeBegin = begin;
				WgRect newGeo = _handleGeo(size,true);

				changeRect = WgRect::bounds(oldGeo, newGeo);
			}

			if (end != m_rangeEnd)
			{
				WgRect oldGeo = _handleGeo(size, false);

				m_rangeEnd = end;
				WgRect newGeo = _handleGeo(size, false);

				changeRect = WgRect::bounds(changeRect, WgRect::bounds(oldGeo, newGeo));
			}
		}

		_requestRender( changeRect );
	}
}

//____ _setHandleState() __________________________________________________

void WgRangeSlider::_setHandleState(wg::State state, bool isBeginHandle)
{
	if (isBeginHandle )
	{
		if (m_pBeginHandleSkin )
        {
            wg::RectI dirty = _skinDirtyRect(m_pBeginHandleSkin, _handleGeo(PixelSize(), isBeginHandle), m_scale, state, m_beginHandleState, m_rangeBegin, m_rangeBegin);
            if( !dirty.isEmpty() )
                _requestRender( dirty );
        }
		m_beginHandleState = state;
	}
	else
	{
        if (m_pEndHandleSkin )
        {
            wg::RectI dirty = _skinDirtyRect(m_pEndHandleSkin, _handleGeo(PixelSize(), isBeginHandle), m_scale, state, m_endHandleState, m_rangeEnd, m_rangeEnd);
            if( !dirty.isEmpty() )
                _requestRender( dirty );
        }
        
		m_endHandleState = state;
	}
}

//____ _handleGeo() _______________________________________________________

WgRect WgRangeSlider::_handleGeo(const WgRect& widgetGeo, bool isBeginHandle)
{
	WgRect contentGeo = m_pSkin ? _skinContentRect(m_pSkin, widgetGeo, m_state, m_scale) : widgetGeo;

	WgRect handleGeo;

	WgSize handlePrefSize = isBeginHandle ? _skinPreferredSize(m_pBeginHandleSkin, m_scale) : 
											_skinPreferredSize(m_pEndHandleSkin, m_scale);

	float value = isBeginHandle ? m_rangeBegin : m_rangeEnd;

	if (m_axis == wg::Axis::X)
	{
		handleGeo.w = handlePrefSize.w * (contentGeo.h / float(handlePrefSize.h));
		handleGeo.h = contentGeo.h;

		handleGeo.x = contentGeo.x + (contentGeo.w - handleGeo.w) * value;
		handleGeo.y = contentGeo.y;
	}
	else
	{
		handleGeo.w = contentGeo.w;
		handleGeo.h = handlePrefSize.h * (contentGeo.w / float(handlePrefSize.w));

		handleGeo.x = contentGeo.x;
		handleGeo.y = contentGeo.y + contentGeo.h - handleGeo.h - (contentGeo.h - handleGeo.h) * value;
	}

	return handleGeo;
}

