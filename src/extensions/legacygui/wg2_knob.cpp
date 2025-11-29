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
#include <wg2_knob.h>
#include <wg2_gfxdevice.h>
#include <wg2_base.h>
//#include <wg2_color.h>
//#include "Debug.h"
#include <wg2_event.h>
#include <wg2_eventhandler.h>
#include <wg2_patches.h>
#include <wg2_util.h>

static const char	c_widgetType[] = {"Knob"};

#include <cmath>
#include <algorithm>

#define PI 3.141592653f

//____ Constructor ____________________________________________________________

WgKnob::WgKnob()
{
	m_pSurf = nullptr;
	m_bOpaque = false;
	m_fValue = 0.0f;
	m_preferredSize = WgSize(45,45);

}

WgKnob::WgKnob(wg::SurfaceFactory * pFactory)
{
	WgKnob();

	m_pSurfaceFactory = pFactory;

	m_size = PixelSize();

	// Initialize arrays
	SetNumSteps(m_iNumSteps);

}


//____ Destructor _____________________________________________________________

WgKnob::~WgKnob()
{
}

//____ Type() _________________________________________________________________

const char * WgKnob::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgKnob::GetClass()
{
	return c_widgetType;
}

//____ SetOrigo() _____________________________________________________________

void WgKnob::SetOrigo(WgOrigo origo)
{
	m_origo = origo;
	_requestRender();
}



//____ SetValue() _____________________________________________________________

void WgKnob::SetValue( float fValue )
{
	if(m_fValue == fValue)
		return;

	_requestRenderBackBuffer(_calcUdateRect(m_fAngleStart, m_fAngleEnd, fValue));

	m_fValue = fValue;
}


//____ SetPreferredPixelSize() __________________________________________________________
/*
void WgKnob::SetPreferredPixelSize(WgSize size)
 {
 m_preferredSize = size;
 }
 */


 //____ PreferredPixelSize() __________________________________________________________

WgSize WgKnob::PreferredPixelSize() const
{
	return m_pSkin ? m_preferredSize + _skinContentPadding( m_pSkin, m_scale): m_preferredSize;
}

//____ MatchingPixelHeight() __________________________________________________

int WgKnob::MatchingPixelHeight(int pixelWidth) const
{
	if (m_pSkin)
	{
		WgSize padding = _skinContentPadding( m_pSkin, m_scale);
		pixelWidth = pixelWidth - padding.w + padding.h;
	}

	return pixelWidth;
}

//____ MatchingPixelWidth() __________________________________________________

int WgKnob::MatchingPixelWidth(int pixelHeight) const
{
	if (m_pSkin)
	{
		WgSize padding = _skinContentPadding( m_pSkin, m_scale);
		pixelHeight = pixelHeight - padding.h + padding.w;
	}

	return pixelHeight;
}


//____ SetMouseControl() __________________________________________________________

void WgKnob::SetMouseControl(bool bMouseControl)
{
	m_bMouseControl = bMouseControl;
}

//____ SetNumSteps() __________________________________________________________

void WgKnob::SetNumSteps(int steps)
{
	if(m_iNumSteps == steps)
		return;

	int maxNumSteps = 11;
	int minNumSteps = 2;
	m_iNumSteps = steps;
	if(steps < minNumSteps || steps > maxNumSteps)
		return;

	// Idea:
	// 2 steps: two large segments  ([0.3 0.45], [0.55 0.7])
	// 3 steps: [
	m_AngleStart.resize(steps);
	m_AngleEnd.resize(steps);

	float divs = (float)(m_iNumSteps * 2 - 1);
	//float length = (m_fAngleEnd - m_fAngleStart)*steps/maxNumSteps;

	float weight = float(steps - minNumSteps)/float(maxNumSteps - minNumSteps);
	float length = (m_fAngleEnd - m_fAngleStart)*weight + 0.25f*(1.0f-weight);
	float start = 0.5f - length/2.0f; // start the sections

	for(int i=0;i<steps;i++)
	{
		m_AngleStart[i] = start + 2*i*length/divs;
		m_AngleEnd[i]   = start + (2*i+1)*length/divs;
		m_AngleStart[i] *= 2*PI;
		m_AngleEnd[i] *= 2*PI;
	}
}

//____ SetAngleOffset() _______________________________________________________

void WgKnob::SetAngleOffset(float offset)
{
	if (offset != m_fAngleOffset)
	{
		m_fAngleOffset = offset;
		_requestRenderBackBuffer();
	}
}

//____ _calcUpdateRect() ______________________________________________________

WgRect WgKnob::_calcUdateRect(float newAngleStart, float newAngleEnd, float newValue)
{
	WgRect contentRect = m_size;

	if (m_bOptimizeUpdateRect)
	{
		// Subtract 2π from the angles if the start angle is offsetted more than 2π.
		const float old_K = (m_fAngleStart + m_fAngleOffset > 1.0f ? 1.0f : 0.0f);

		float old_start = (m_fAngleStart + m_fAngleOffset - old_K) * 2 * PI;
		float old_end = (m_fAngleEnd + m_fAngleOffset - old_K) * 2 * PI;
		float old_value = (old_end - old_start) * m_fValue + old_start;

		const float new_K = (newAngleStart + m_fAngleOffset > 1.0f ? 1.0f : 0.0f);

		float new_start = (newAngleStart + m_fAngleOffset - new_K) * 2 * PI;
		float new_end = (newAngleEnd + m_fAngleOffset - new_K) * 2 * PI;
		float new_value = (new_end - new_start) * newValue + new_start;

		float changeBeg, changeEnd;

		if (new_start == old_start)
			changeBeg = std::min(old_value, new_value);
		else
			changeBeg = std::min(old_start, new_start);


		if (new_value == old_value)
			changeEnd = std::max(old_start, new_start);
		else
			changeEnd = std::max(old_value, new_value);

		// Apparently we need some margin because of the fade at the end...

		changeBeg -= 0.3f;
		changeEnd += 0.3f;

		//

		if (changeBeg < PI / 2)
		{
			if (changeEnd < PI / 2)
				return { contentRect.x, contentRect.y + contentRect.h / 2, contentRect.w / 2, contentRect.h / 2 };
			else if (changeEnd < PI)
				return { contentRect.x, contentRect.y, contentRect.w / 2, contentRect.h };
		}
		else if (changeBeg < PI)
		{
			if (changeEnd < PI)
				return { contentRect.x, contentRect.y, contentRect.w / 2, contentRect.h / 2 };
			else if (changeEnd < PI + PI / 2)
				return { contentRect.x, contentRect.y, contentRect.w, contentRect.h / 2 };
		}
		else if (changeBeg < PI + PI / 2)
		{
			if (changeEnd < PI + PI / 2)
				return { contentRect.x + contentRect.w / 2, contentRect.y, contentRect.w / 2, contentRect.h / 2 };
			else if (changeEnd < PI * 2)
				return { contentRect.x + contentRect.w / 2, contentRect.y, contentRect.w / 2, contentRect.h };
		}
		else if (changeBeg < PI * 2)
		{
			if (changeEnd < PI * 2)
				return { contentRect.x + contentRect.w / 2, contentRect.y + contentRect.h / 2, contentRect.w / 2, contentRect.h / 2 };
			else if (changeEnd < PI * 2 + PI / 2)
				return { contentRect.x, contentRect.y + contentRect.h / 2, contentRect.w, contentRect.h / 2 };
		}
	}
	return contentRect;
}


//____ SetAngles() __________________________________________________________
void WgKnob::SetAngles(float angleStart, float angleEnd)
{
	if((m_fAngleStart == angleStart) && (m_fAngleEnd == angleEnd))
		return;

	_requestRenderBackBuffer( _calcUdateRect( angleStart, angleEnd, m_fValue ));

	m_fAngleStart = angleStart;
	m_fAngleEnd = angleEnd;
}

void WgKnob::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	switch( pEvent->Type() )
	{
		case    WG_EVENT_MOUSE_ENTER:
			if (m_bMouseControl)
			{
				m_bPointerInside = true;
				_requestRenderBackBuffer();
			}
			break;

		case    WG_EVENT_MOUSE_LEAVE:
			if (m_bMouseControl)
			{
				m_bPointerInside = false;
				_requestRenderBackBuffer();
			}
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			if (m_bMouseControl)
			{

				int button = static_cast<const WgEvent::MouseButtonPress*>(pEvent)->Button();
				if (button == 1)
					m_bPressed = true;

				_requestRenderBackBuffer();
			}
			break;
		}
		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			if (m_bMouseControl)
			{
				int button = static_cast<const WgEvent::MouseButtonRelease*>(pEvent)->Button();
				if (button == 1)
					m_bPressed = false;

				_requestRenderBackBuffer();
			}
			break;
		}
		default:
			break;

	}
	WgWidget::_onEvent(pEvent,pHandler);

}

//____ SetKnobScale() ______________________________________________________________

void WgKnob::SetKnobScale(float scale)
{
	wg::limit(scale, 0.f, 1.f);
	m_knobScale = scale;
	_onNewSize(PixelSize());					// Force a refresh of size and backbuffer.
	_requestRender();
}


//____ _onCloneContent() _______________________________________________________

void WgKnob::_onCloneContent( const WgWidget * _pOrg )
{
	const WgKnob * pOrg = static_cast<const WgKnob*>(_pOrg);
}

//____ _onNewSize() ___________________________________________________________

void WgKnob::_onNewSize(const WgSize& size)
{
	WgSize sz = m_pSkin ? size - _skinContentPadding( m_pSkin, m_scale) : size;

	const int w = std::max(int(std::min(sz.w, sz.h)*m_knobScale), 0);
	const int h = w;

	WgSize newSize(w, h);

	if (m_size != newSize )
	{
		m_pSurf = nullptr;
		m_size = newSize;
	}
	_requestRenderBackBuffer();
}

//____ _renderPatches() _______________________________________________________

void WgKnob::_renderPatches(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches)
{
	if( _pPatches->isEmpty() )
		return;
	
	if (!m_pSurf)
	{
		if( !m_pSurfaceFactory )
			return;
		m_pSurf = m_pSurfaceFactory->createSurface( WGBP(Surface,
													_.size = m_size*m_iOversampleX, 
													_.format = WgPixelType::BGRA_8) );
	}

	if( !m_backBufferDirtyRect.isEmpty() )
	{
		_redrawBackBuffer(WgRect::overlap({ 0, 0, m_size.w, m_size.h}, m_backBufferDirtyRect));
		m_backBufferDirtyRect.clear();
	}

	WgColor orgTintColor = pDevice->tintColor();

	pDevice->setTintColor(m_kColor * orgTintColor);

	int bytesToRelease = _convertAndPushClipList( pDevice, _pPatches->size(), _pPatches->begin() );
	_onRender(pDevice, _canvas, _window);
	_popAndReleaseClipList( pDevice, bytesToRelease );

	pDevice->setTintColor(orgTintColor);

}

//____ _onRender() _____________________________________________________________
void WgKnob::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, _canvas, m_state, m_scale) : _canvas;

	WgRect surfRect = m_size;
	WgRect contentRect = WgUtil::OrigoToRect(m_origo, canvas.size(), surfRect) + canvas.pos();

	pDevice->setBlitSource(m_pSurf);
	pDevice->blit(WgCoord(contentRect.x,contentRect.y)*64, {0,0,m_size*64});
}

//____ _redrawBackBuffer() ____________________________________________________

void WgKnob::_redrawBackBuffer(WgRect region)
{
	int w = m_size.w;
	int h = m_size.h;

	// TODO:
	// Better calculation of angles, without atan and div for every pixel (polygon?)

	const float PI_HALF = PI / 2.0f;

	const int oversampling = m_iOversampleX;

	const float xd = 2.0f / (float)(w * oversampling);
	const float yd = 2.0f / (float)(h * oversampling);
	const float rd = 3.0f * 2.0f / (float)(w * oversampling); // should be sqrt(w+h);
	const float rd_inv = 1.0f / rd;

	const float r_outer = 1.0f - rd;
	const float r_inner = 1.0f - rd - m_iWidth/(1.f+rd);

	// Subtract 2π from the angles if the start angle is offsetted more than 2π.
	const float K = (m_fAngleStart + m_fAngleOffset > 1.0f ? 1.0f : 0.0f);

	const float a_start_const = (m_fAngleStart + m_fAngleOffset - K) * 2 * PI;
	const float a_end_const = (m_fAngleEnd + m_fAngleOffset - K) * 2 * PI;
	const float a_value_const = (a_end_const - a_start_const) * m_fValue + a_start_const;

	const int value_index = GetValueIndex();

	// Set up colors
	//WgColor kForeground = { 255,255,255,255 };
	//WgColor kBackground = { 255,255,255, uint8_t(255 * m_fBackgroundAlpha) };

	/*    if(m_bPressed)
	kForeground = Blend(m_kForeground, WgColor::white, 0.1f);
	else if(m_bPointerInside)
	kForeground = Blend(m_kForeground, WgColor::white, 0.5f);
	*/
	//WgColor kBackTransp = { 255,255,255,0 };

    int background = (m_kBackground.b) | ((m_kBackground.g) << 8) | ((m_kBackground.r) << 16) | ((m_kBackground.a) << 24);
    int foreground = (m_kForeground.b) | ((m_kForeground.g) << 8) | ((m_kForeground.r) << 16) | ((m_kForeground.a) << 24);
    int backtransp = (m_kBackTransp.b) | ((m_kBackTransp.g) << 8) | ((m_kBackTransp.r) << 16) | ((m_kBackTransp.a) << 24);

    WgColor col;

    auto pixbuf = m_pSurf->allocPixelBuffer(region);
    unsigned char* dest = (unsigned char*) pixbuf.pixels;

	float x = 0.0f, y = 0.0f;
	float y_inv = 0.0f;
	int color = 0;
	float r, in, out, weight, R, a;
	const bool bDrawFromMiddle = DrawFromMiddle();
	for (int yc = region.y; yc< region.y + region.h*oversampling; yc++)
	{
		// [-1, 1] coordinates
		y = (float)yc*yd - 1.0f;
		y_inv = 1.0f / std::max(std::abs(y), 0.0001f);

        unsigned int *ddest = (unsigned int *)(dest + pixbuf.pitch * (yc-region.y));    // Don't forget about pitch
		for (int xc = region.x; xc<region.x + region.w*oversampling; xc++)
		{
			// [-1, 1] coordinates
			x = (float)xc*xd - 1.0f;

			// Radius
			r = x * x + y * y;

			// Reset color
			color = 0;

			// Calc boundaries for AA
			in = r - r_inner; // in > 0
			out = r_outer - r; // out > 0
			weight = 0.0f;

			R = 0.0f;
			a = 0.0f;

			const bool USE_ANGLE_AA = true;
			const float ANGLE_AA_WIDTH = 3.0f / (float)(w * oversampling); //0.05f;

			if ((in > -rd) && (out > -rd))
			{
				// Calculate anti-aliasing on the inner and outer circle diameters ("CIRCLE AA")
				if (in < 0 && in > -rd)
					weight = 1.0f + in * rd_inv;
				else if (out < 0 && out > -rd)
					weight = 1.0f + out * rd_inv;
  				else
					weight = 1.0f;

				// Calculate angle.
				// We compare atan(-x/y) > a_start. Find equivalent -x/y > tan(a_start)?
				R = -x * y_inv;
				a = std::atan(R);

				if (y < 0.0f)
					a = PI - a;

				if (a < 0.0f)
					a += 2 * PI;

				if (a > 2 * PI)
					a -= 2 * PI;

				if (m_iNumSteps < 2 || m_iNumSteps > 11)
				{
					// Continuous version.
					// ========================================================================

					float a_start = -1.0f;
					float a_end = -1.0f;
					float a_value = -1.0f;

					// 1. Determine current state(s).
					DrawState CurrentDrawState;

					// const float mid_line_width = 0.1f;
					const float mid_line_width = 0.07f;

					if(bDrawFromMiddle)
					{
						// Regular case.
						a_start = a_start_const;
						a_end = a_end_const;
						a_value = a_value_const;



						// a: current angle we're drawing
						// a_value: parameter value (angle)
						// a_start/a_end: circle segment start/end (angle)

						float a_mid = (a_end-a_start) * 0.5f + a_start;



						if (a_value < a_mid) //Before midpoint
						{
							// DBGM(DBG_GUI, ("fabs(a_value - PI) = %f, before mid", fabs(a_value - PI)));
							if(a < a_start)
							{
								CurrentDrawState = DrawState::eDrawNothing;
							}

							else if(a >= a_start && a < a_start + ANGLE_AA_WIDTH && a_value > a_start + ANGLE_AA_WIDTH) //OK. left edge of background
							{
								CurrentDrawState = DrawState::eDrawBackgroundAABegin;
							}
							else if(a >= a_start && a < a_value)
							{
								CurrentDrawState = DrawState::eDrawBackground;
							}





							else if(a >= a_value && a < a_value + ANGLE_AA_WIDTH) //om a_mid - a_value < mid_line_width, rita foreground
							{											  // a_mid - mid_line_width < a_value
								if(a_value < a_mid - ANGLE_AA_WIDTH)
									CurrentDrawState = DrawState::eDrawForegroundAALeftEdge;
								else
									CurrentDrawState = DrawState::eDrawForeground;
							}
							else if(a >= a_value + ANGLE_AA_WIDTH && a < a_mid)
							{
								CurrentDrawState = DrawState::eDrawForeground;
							}
							else if(a >= a_mid  && a < a_mid + mid_line_width )
							{
								//Right edge of light blue
								CurrentDrawState = DrawState::eDrawForeground;
							}


							else if(a >= a_end - ANGLE_AA_WIDTH && a < a_end) //OK. Right edge of background
							{
								CurrentDrawState = DrawState::eDrawBackgroundAAEnd;
							}
							else if(a >= a_mid + mid_line_width  && a < a_end - ANGLE_AA_WIDTH) //After midpoint
							{
								CurrentDrawState = DrawState::eDrawBackground;
							}


							else if(a > a_end)
								CurrentDrawState = DrawState::eDrawNothing;



								//To draw line in middle when value close to 0.5f
							if(a >= a_mid - mid_line_width && a < a_mid + mid_line_width && (fabs(a_value - a_mid) < mid_line_width))
							{
//								DBGM(DBG_GUI, ("Close to 0.5"));
								// if(a < a_mid - mid_line_width + ANGLE_AA_WIDTH)// && a_value < a_mid - mid_line_width + ANGLE_AA_WIDTH)
								// 	CurrentDrawState = DrawState::eDrawForegroundAALeftEdge;
								// else
									CurrentDrawState = DrawState::eDrawForeground;
							}


						}
						else //After midpoint
						{
//							DBGM(DBG_GUI, ("fabs(a_value - PI) = %f, after mid", fabs(a_value - PI)));
							if(a < a_start)
								CurrentDrawState = DrawState::eDrawNothing;
							else if(a >= a_start && a < a_start + ANGLE_AA_WIDTH)
							{
								CurrentDrawState = DrawState::eDrawBackgroundAABegin;
							}
							else if(a >= a_start && a < a_mid )
							{
								CurrentDrawState = DrawState::eDrawBackground;
							}


							else if(a >= a_mid - mid_line_width && a < a_mid)
							{
								CurrentDrawState = DrawState::eDrawForeground;
							}
							else if(a >= a_mid  && a < a_value - ANGLE_AA_WIDTH)
							{
								CurrentDrawState = DrawState::eDrawForeground;
							}
							else if(a >= a_value - ANGLE_AA_WIDTH && a < a_value)
							{
								//Right edge of light blue
								if(a_value >= a_mid + ANGLE_AA_WIDTH)
									CurrentDrawState = DrawState::eDrawForegroundAAEnd;
								else
									CurrentDrawState = DrawState::eDrawForeground;
							}


							if(a >= a_end - ANGLE_AA_WIDTH && a < a_end && a_value < a_end - ANGLE_AA_WIDTH)
							{
								CurrentDrawState = DrawState::eDrawBackgroundAAEnd;
							}
							else if(a >= a_value && a < a_end - ANGLE_AA_WIDTH)
							{
								CurrentDrawState = DrawState::eDrawBackground;
							}

							else if(a > a_end)
								CurrentDrawState = DrawState::eDrawNothing;

							if(a > a_mid - mid_line_width && a <= a_mid + mid_line_width && (fabs(a_value - a_mid) < mid_line_width))
							{
//								DBGM(DBG_GUI, ("Close to 0.5"));

								// if(a > a_mid + mid_line_width - ANGLE_AA_WIDTH && a_value > a_mid + mid_line_width - ANGLE_AA_WIDTH)
								// 	CurrentDrawState = DrawState::eDrawForegroundAARightEdge;
								// else
									CurrentDrawState = DrawState::eDrawForeground;
							}

						}
					} //end case draw from middle
					else if (a_end_const > 2 * PI)
					{
						// Special case: end angle larger than 2π.
						a_start = a_start_const;
						a_end = a_end_const - 2 * PI;
						a_value = a_value_const;

						if (a_value > 2 * PI)
						{
							a_value -= 2 * PI;

							if (a < a_value)
							{
								CurrentDrawState = DrawState::eDrawForeground;

								if (USE_ANGLE_AA && (a >= a_value - ANGLE_AA_WIDTH))
								{
									CurrentDrawState = DrawState::eDrawForegroundAAEnd;
								}
								else
								{
									// Compensate for gradient calculation.
									a += 2 * PI;
									a_value += 2 * PI;
								}
							}
							else if ((a >= a_value) && (a < a_end))
							{
								CurrentDrawState = DrawState::eDrawBackground;

								if (USE_ANGLE_AA && (a >(a_end - ANGLE_AA_WIDTH)))
								{
									CurrentDrawState = DrawState::eDrawBackgroundAAEnd;
								}
							}
							else if ((a >= a_end) && (a < a_start))
							{
								CurrentDrawState = CurrentDrawState = DrawState::eDrawNothing;
							}
							else
							{
								CurrentDrawState = DrawState::eDrawForeground;

								if (USE_ANGLE_AA && (a < (a_start + ANGLE_AA_WIDTH)))
								{
									CurrentDrawState = DrawState::eDrawForegroundAABegin;
								}

								// Compensate for gradient calculation.
								a_value += 2 * PI;
							}
						}
						else
						{
							if (a < a_end)
							{
								CurrentDrawState = DrawState::eDrawBackground;

								if (USE_ANGLE_AA && (a >(a_end - ANGLE_AA_WIDTH)))
								{
									CurrentDrawState = DrawState::eDrawBackgroundAAEnd;
								}
							}
							else if (a < a_start)
							{
								CurrentDrawState = CurrentDrawState = DrawState::eDrawNothing;
							}
							else if (USE_ANGLE_AA && (a >= a_start) && (a < a_start + ANGLE_AA_WIDTH) && (a_value <= a_start + ANGLE_AA_WIDTH))
							{
								CurrentDrawState = DrawState::eDrawBackgroundAABegin;
							}
							else if ((a >= a_start) && (a < a_value - (USE_ANGLE_AA ? ANGLE_AA_WIDTH : 0.0f)))
							{
								CurrentDrawState = DrawState::eDrawForeground;

								if (USE_ANGLE_AA && (a < (a_start + ANGLE_AA_WIDTH)))
								{
									CurrentDrawState = DrawState::eDrawForegroundAABegin;
								}
							}
							else if (USE_ANGLE_AA && (a >= a_value - ANGLE_AA_WIDTH) && (a < a_value))
							{
								CurrentDrawState = DrawState::eDrawForegroundAAEnd;
							}
							else
							{
								CurrentDrawState = DrawState::eDrawBackground;
							}
						}
					}
					else
					{
						// Regular case.
						a_start = a_start_const;
						a_end = a_end_const;
						a_value = a_value_const;

						if (((a > a_start) && (a < a_value - (USE_ANGLE_AA ? ANGLE_AA_WIDTH : 0.0f))))
						{
							CurrentDrawState = DrawState::eDrawForeground;

							if (USE_ANGLE_AA && (a < (a_start + ANGLE_AA_WIDTH)))
							{
								CurrentDrawState = DrawState::eDrawForegroundAABegin;
							}
						}
						else if (USE_ANGLE_AA && (a >= a_start) && (a < a_start + ANGLE_AA_WIDTH) && (a_value <= a_start + ANGLE_AA_WIDTH))
						{
							CurrentDrawState = DrawState::eDrawBackgroundAABegin;
						}
						else if (USE_ANGLE_AA && (a >= a_value - ANGLE_AA_WIDTH) && (a < a_value) && (a_value > a_start + ANGLE_AA_WIDTH))
						{
							CurrentDrawState = DrawState::eDrawForegroundAAEnd;
						}
						else if ((a >= a_value) && (a < a_end))
						{
							CurrentDrawState = DrawState::eDrawBackground;

							if (USE_ANGLE_AA && (a >(a_end - ANGLE_AA_WIDTH)))
							{
								CurrentDrawState = DrawState::eDrawBackgroundAAEnd;
							}
						}
						else
						{
							CurrentDrawState = DrawState::eDrawNothing;
						}
					}

					// 2. Draw current state.
					switch (CurrentDrawState)
					{
					case DrawState::eDrawNothing:
					{
						// Draw nothing.
						// -------------------------------------------------
						col = m_kBackTransp;
						break;
					}
					case DrawState::eDrawForegroundAALeftEdge:
					{
						//AA for left side of light blue area when start from middle

						const float ww = (a - a_value)*(1.0f / ANGLE_AA_WIDTH);

						if (m_bPressed | m_bPointerInside)
							col = WgColor(255, 255, 255, 255);
						else
							col = m_kForeground;

						col = Blend(col, m_kBackground, ww);

						col = Blend(col, m_kBackTransp, weight); // CIRCLE AA
						break;
					}

					case DrawState::eDrawForegroundAABegin:
					{
						// Draw anti-alias of beginning of foreground.
						// -------------------------------------------------
						if (m_bPressed | m_bPointerInside)
							col = Blend(WgColor(255, 255, 255, 255), m_kForeground, (a - a_start) / (a_value - a_start));
						else
							col = m_kForeground;

						const float ww = (a - a_start) * (1.0f / ANGLE_AA_WIDTH);
						col = Blend(col, m_kBackTransp, ww);

						col = Blend(col, m_kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawForeground:
					{
						// Draw foreground.
						// -------------------------------------------------
						if (m_bPressed | m_bPointerInside)
							col = Blend(WgColor(255, 255, 255, 255), m_kForeground, (a - a_start) / (a_value - a_start));
						else
							col = m_kForeground;

						col = Blend(col, m_kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawForegroundAAEnd:
					{
						// Draw anti-alias of end of foreground.
						// -------------------------------------------------

						// This is an example of a quick and dirty anti-aliasing
						// that works pretty good.

						// 20 is 1/0.05, which is the angle that's being AAd.
						const float ww = (a - (a_value - ANGLE_AA_WIDTH)) * (1.0f / ANGLE_AA_WIDTH);

						if (m_bPressed | m_bPointerInside)
							col = WgColor(255, 255, 255, 255);
						else
							col = m_kForeground;

						col = Blend(m_kBackground, col, ww);
						col = Blend(col, m_kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawForegroundAARightEdge:
					{
						float a_mid = (a_end - a_start) * 0.5f + a_start;

						//Draw anti-alias of right edge of foreground, by the middle of the circle.

						const float ww = ((a_mid + mid_line_width) - a)*(1.0f / ANGLE_AA_WIDTH);

						if (m_bPressed | m_bPointerInside)
							col = WgColor(255, 255, 255, 255);
						else
							col = m_kForeground;

						col = Blend(m_kBackground, col, ww);
						col = Blend(col, m_kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawBackgroundAABegin:
					{
						// Draw anti-alias of beginning of background.
						// -------------------------------------------------
						const float ww = (a - a_start) * (1.0f / ANGLE_AA_WIDTH);
						col = Blend(m_kBackground, m_kBackTransp, ww);

						col = Blend(col, m_kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawBackground:
					{
						// Draw background.
						// -------------------------------------------------
						col = Blend(m_kBackground, m_kBackTransp, weight); // CIRCLE AA
						break;
					}
					case DrawState::eDrawBackgroundAAEnd:
					{
						// Draw anti-alias of end of background.
						// -------------------------------------------------
						col = Blend(m_kBackground, m_kBackTransp, weight); // CIRCLE AA

						const float ww = (a_end - a) * (1.0f / ANGLE_AA_WIDTH);
						col = Blend(col, m_kBackTransp, ww);
						break;
					}
					}

				}
				else
				{
					// Discrete version.
					// ========================================================================

					bool colorize = false;
					WgColor set_col = m_kBackground;
					for (int i = 0; i<m_iNumSteps; i++)
					{
						if ((a > m_AngleStart[i] && (a < m_AngleEnd[i])))
						{
							colorize = true;
							if (value_index == i)
								set_col = m_kForeground;
						}
					}
					if (colorize)
						col = Blend(set_col, m_kBackTransp, weight);
					else
						col = m_kBackTransp;
				}

				color = (col.b) | ((col.g) << 8) | ((col.r) << 16) | ((col.a) << 24);
			}

			*ddest++ = color;
		}


	}

    m_pSurf->pullPixels(pixbuf);
    m_pSurf->freePixelBuffer(pixbuf);

	// Downsample. Oversampling is not used.
	//    _downsample(m_pSurf, m_iOversampleX);

}



WgColor WgKnob::Blend( const WgColor& start, const WgColor& dest, float grade )
{
	WgColor col;

	const unsigned int g = (unsigned int)(grade * 255.0f);
	const unsigned int ig = 255 - g;

	// Yes, it's backwards. Sue me.
	/*
	col.r = (Uint8)( (float)start.r * grade + (1.0f-grade) * (float)dest.r );
	col.g = (Uint8)( (float)start.g * grade + (1.0f-grade) * (float)dest.g );
	col.b = (Uint8)( (float)start.b * grade + (1.0f-grade) * (float)dest.b );
	col.a = (Uint8)( (float)start.a * grade + (1.0f-grade) * (float)dest.a );
	*/

	col.r = (start.r * g + ig * dest.r) >> 8;
	col.g = (start.g * g + ig * dest.g) >> 8;
	col.b = (start.b * g + ig * dest.b) >> 8;
	col.a = (start.a * g + ig * dest.a) >> 8;

	return col;
}

void WgKnob::_downsample(wg::Surface* pSurf, const int oversample)
{
	if(oversample == 1)
		return;

	int w = pSurf->pixelSize().w; //Width();
	int h = pSurf->pixelSize().h; //Height();
	unsigned int col = (255) | (2<<8) | (1<<16) | (192<<24);

    auto pixbuf = pSurf->allocPixelBuffer();
    pSurf->pushPixels(pixbuf);
    
    unsigned int* data = (unsigned int*) pixbuf.pixels;
	int i=0, j=0;

	// Loop over small size
	for(int y=0; y<h/oversample; y++)
	{
		for(int x=0; x<w/oversample; x++)
		{
			i = y*w+x; // every pixel
			j = y*oversample*w + x*oversample; // every other pixel

			data[i] = 0; // clear pixel. Wrong for (0,0).
			uint8_t* qd = (uint8_t*)(&(data[i]));

			for(int ys=0; ys<oversample;ys++)
			{
				for(int xs=0; xs<oversample;xs++)
				{
					uint8_t* qs = (uint8_t*)(&(data[j + xs + ys*w]));
					qd[0] += qs[0]>>oversample;
					qd[1] += qs[1]>>oversample;
					qd[2] += qs[2]>>oversample;
					qd[3] += qs[3]>>oversample;
				}
			}
		}
	}

    pSurf->pullPixels(pixbuf);
    pSurf->freePixelBuffer(pixbuf);
}

//____ _onAlphaTest() ___________________________________________________________

bool WgKnob::_onAlphaTest( const WgCoord& ofs )
{
	return true;
}


//____ _onEnable() _____________________________________________________________

void WgKnob::_onEnable()
{
	_requestRenderBackBuffer();
}

//____ _onDisable() ____________________________________________________________

void WgKnob::_onDisable()
{
	_requestRenderBackBuffer();
}

//____ _requestRenderBackBuffer() _____________________________________________________

void  WgKnob::_requestRenderBackBuffer()
{
	m_backBufferDirtyRect = { 0,0, 1000000, 1000000 };
	_requestRender();
}

void  WgKnob::_requestRenderBackBuffer(const WgRect& rect)
{
	if (m_backBufferDirtyRect.isEmpty())
		m_backBufferDirtyRect = rect;
	else
		m_backBufferDirtyRect.growToContain( rect );

	WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, PixelSize(), m_state, m_scale) : WgRect(PixelSize());

	WgRect surfRect = m_size;
	WgRect contentRect = WgUtil::OrigoToRect(m_origo, canvas.size(), surfRect) + canvas.pos();

	WgRect dirtyCanvasRect = WgRect::overlap(contentRect, rect + contentRect.pos());
	_requestRender(dirtyCanvasRect);

	//	_requestRender();
}
