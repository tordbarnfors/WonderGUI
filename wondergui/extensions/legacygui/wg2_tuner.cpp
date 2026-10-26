
#include <wg2_tuner.h>
#include <wg2_gfxdevice.h>
#include <wg2_eventhandler.h>
#include <math.h>
#include <algorithm>
#include <numeric>

static const char	c_widgetType[] = {"VolumeMeter"};

#ifdef WIN32
#include <wg2_userdefines.h>
#endif

//____ Constructor ____________________________________________________________

WgTuner::WgTuner()
{
	m_LEDColors[0].second = WgColor::Black;
	m_LEDColors[1].second = WgColor::Black;
	m_LEDColors[2].second = WgColor::Black;
	m_LEDColors[0].first = WgColor::Green;
	m_LEDColors[1].first = WgColor::Yellow;
	m_LEDColors[2].first = WgColor::Red;

    m_bUseBackgroundColor = false;
    m_BackgroundColor = WgColor::Black;

    m_nSectionLEDs.resize(3);
    m_nSectionLEDs[0] = 8;
	m_nSectionLEDs[1] = 2;
	m_nSectionLEDs[2] = 1;

    m_nSectionLEDs.resize(3, 0);
	m_nLEDs = m_nSectionLEDs[0] + m_nSectionLEDs[1] + m_nSectionLEDs[2];
	m_LEDSpacing = 0.33f;
	m_direction = WgDirection::Up;

	m_iPeak = 0;
	m_iHold = 0;
	m_fPeak = 0.0f;

    m_iNotePeak = -1;
    m_iNoteHold = 0;
    m_fNotePeak = 0.0f;

    m_fSidePadding = 0.05f;
	m_iSidePadding = 1;

	m_bZeroInMiddle = false;
	m_d = 1.0f/(float)(m_nLEDs-1);
	m_d2 = 0.5f/(float)(m_nLEDs);

	m_LEDStates.resize(m_nLEDs);
	for(int i=0;i<m_nLEDs;i++)
		m_LEDStates[i] = 0.0f;

	m_bUseFades = false;

    m_kFilter.resize(m_iFilterSize, 0.f);
    m_kNoteFilter.resize(m_iNoteFilterSize, 0);

    _startReceiveTicks();
}

std::string GetNote()
{
	return "";
}

//____ Destructor _____________________________________________________________

WgTuner::~WgTuner()
{
}

//____ Type() _________________________________________________________________

const char * WgTuner::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgTuner::GetClass()
{
	return c_widgetType;
}

//____ SetDirection() ________________________________________________________

void WgTuner::SetDirection( WgDirection direction )
{
	if( direction == WgDirection::Undefined )
		return;

	if( direction != m_direction )
	{
		m_direction = direction;
		_requestResize();
		_requestRender();
	}
}


//____ SetLEDColors() ___________________________________________________________
/*
void WgTuner::SetLEDColors(	WgColor bottomOn, WgColor middleOn, WgColor topOn,
									WgColor bottomOff, WgColor middleOff, WgColor topOff )
{
	m_LEDColors[0].second = bottomOff;
	m_LEDColors[0].first = bottomOn;

	m_LEDColors[1].second = middleOff;
	m_LEDColors[1].first = middleOn;

	m_LEDColors[2].second = topOff;
	m_LEDColors[2].first = topOn;

	_requestRender();
}

//____ SetNbLEDs() _________________________________________________________

void WgTuner::SetNbLEDs( int bottomSection, int middleSection, int topSection )
{
	if( bottomSection < 0 )
		bottomSection = 0;
	if( middleSection < 0 )
		middleSection = 0;
	if( topSection < 0 )
		topSection = 0;

	if( bottomSection != m_nSectionLEDs[0] || middleSection != m_nSectionLEDs[1] || topSection != m_nSectionLEDs[2] )
	{
		m_nSectionLEDs[0] = bottomSection;
		m_nSectionLEDs[1] = middleSection;
		m_nSectionLEDs[2] = topSection;
		m_nLEDs = bottomSection + middleSection + topSection;
		_requestResize();
		_requestRender();
	}


	d = 1.0f/(float)(m_nLEDs-1);
	d2 = 0.5f/(float)(m_nLEDs);

	m_LEDStates.resize(m_nLEDs);
	for(int i=0;i<m_nLEDs;i++)
		m_LEDStates[i] = 0.0f;
}*/

void WgTuner::SetLeds(int sections, std::vector<int> ledPerSection, std::map<int, std::pair<WgColor, WgColor> > sectionColor)
{
    m_nLEDs = std::accumulate(std::begin(ledPerSection), std::end(ledPerSection), 0);
    m_nSectionLEDs = ledPerSection;
    m_LEDColors = sectionColor;

	m_d = 1.0f/(float)(m_nLEDs-1);
	m_d2 = 0.5f/(float)(m_nLEDs);

    _requestResize();
	_requestRender();

    m_LEDStates.resize(m_nLEDs);
	for(int i=0;i<m_nLEDs;i++)
		m_LEDStates[i] = 0.0f;

}

//____ SetLEDSpacing() ___________________________________________________________

void WgTuner::SetLEDSpacing( float spacing )
{
	if( spacing < 0.f )
		spacing = 0.f;

	if( spacing != m_LEDSpacing )
	{
		m_LEDSpacing = spacing;
		_requestResize();
		_requestRender();
	}
}
//____ SetNoteValue() ______________________________________________________________

void WgTuner::SetNoteValue(float fPeak, float fHold)
{
    wg::limit(fPeak, 0.f, 1.f);
    wg::limit(fHold, 0.f, 1.f);
    m_kNoteFilter[m_iNoteFilterPos] = fPeak;
    m_iNoteFilterPos++;
    m_iNoteFilterPos = m_iNoteFilterPos > m_iNoteFilterSize - 1 ? 0 : m_iNoteFilterPos;

    auto kTmpVec = m_kNoteFilter;
    std::sort(kTmpVec.begin(), kTmpVec.end());
    fPeak = kTmpVec[m_iNoteFilterSize / 2];
    int iNote = static_cast<int>(fPeak * 70);

    m_kNote = g_sTone[iNote];
    if (m_iDelay == -1 || iNote != m_iNoteHold)/*
    {
        m_iDelay = m_iTickMillisec + 200;
        m_iNoteHold = iNote;
    }

    if ((m_iDelay - m_iTickMillisec) < 0 && m_iNotePeak != m_iNoteHold)
    {
        m_iNoteHold = m_iNotePeak;

    }*/

    m_fNotePeak = fPeak;
}

//____ SetPitchValue() ______________________________________________________________

void WgTuner::SetPitchValue(float fPeak, float fHold)
{
	wg::limit( fPeak, 0.f, 1.f );
	wg::limit( fHold, 0.f, 1.f );

    // median filter values
    m_kFilter[m_iFilterPos] = fPeak;
    m_iFilterPos++;
    m_iFilterPos = m_iFilterPos > m_iFilterSize - 1 ? 0 : m_iFilterPos;

    auto kTmpVec = m_kFilter;
    std::sort(kTmpVec.begin(), kTmpVec.end());
    fPeak = kTmpVec[m_iFilterSize / 2];

    if(fPeak < 0.5 - m_fPitchtolerance )
    {
        m_ePitch = pitch::flat;
    }
    else if(fPeak > 0.5 + m_fPitchtolerance )
    {
        m_ePitch = pitch::sharp;
    }
    else
    {
        m_ePitch = pitch::onNote;
    }

    int iPeak = (int)wg_round(fPeak * m_nLEDs);
    int iHold = (int)wg_round(fHold * m_nLEDs);

	if(iPeak != m_iPeak || iHold != m_iHold)
	{
		m_iPeak = iPeak;
		m_iHold = iHold;
 	}

	m_fPeak = fPeak;

}


//____ PreferredPixelSize() ________________________________________________________________

WgSize WgTuner::PreferredPixelSize() const
{
	if( m_direction == WgDirection::Up || m_direction == WgDirection::Down )
		return WgSize((10*m_scale)>>WG_SCALE_BINALS,(5*m_nLEDs*m_scale)>>WG_SCALE_BINALS);
	else
		return WgSize((5*m_nLEDs*m_scale)>>WG_SCALE_BINALS,(10*m_scale)>>WG_SCALE_BINALS);
}

//____ _onNewSize() ____________________________________________________________________

void WgTuner::_onNewSize( const WgSize& size )
{
	switch (m_direction)
	{
		case WgDirection::Left:
		case WgDirection::Right:
			m_iSidePadding = (int) ((float)size.h * m_fSidePadding);
			break;
		case WgDirection::Up:
		case WgDirection::Down:
		default:
			m_iSidePadding = (int) ((float)size.w * m_fSidePadding);
			break;
	}

	if(m_iSidePadding <= 0)
		m_iSidePadding = 0;

	_requestRender();
}

//____ _onEvent() ______________________________________________________________

void WgTuner::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	switch( pEvent->Type() )
	{
		case WG_EVENT_TICK:
		{
			const WgEvent::Tick * pTick = static_cast<const WgEvent::Tick*>(pEvent);
            m_iTickMillisec = pTick->Millisec();

            int firstRenderLED = -1;
            int lastRenderLED = -1;

			for( int i = 0 ; i < m_nLEDs ; i++ )
			{
				bool 	on = false;


				// Tuner Mode: single dot is moving
				float id = m_d*(float)i;
				if(abs(m_fPeak - id) < m_d2)
					on = true;

				if(m_bTunerCorrect)
				{
					if((i == 0) || i == (m_nLEDs-1))
						on = true;
				}
				/*else if(i == 0 && m_fPeak < 0.5f)
				{
					on = true;
				}
				else if(i == (m_nLEDs-1) && m_fPeak > 0.5f)
				{
					on = true;
				}*/

				if(!m_bActive)
					on = false;

                float LEDState;

                if (on)
                    LEDState = 1.f;
                else
                {
                    if (m_bUseFades)
                    {
                        LEDState = m_LEDStates[i] * 0.95f; // Fade out

                        if (LEDState < 0.1f)
                            LEDState = 0.f;
                    }
                    else
                        LEDState = 0.f;
                }

                if (LEDState != m_LEDStates[i])
                {
                    m_LEDStates[i] = LEDState;
                    if (firstRenderLED == -1)
                        firstRenderLED = i;
                    lastRenderLED = i;
                }
            }

			// Possibly calculate a dirty rectangle

			if( firstRenderLED != -1 )
			{
				WgSize sz = PixelSize();
				WgRect rect;

				int meterLen = (m_direction == WgDirection::Up || m_direction == WgDirection::Down) ? sz.h : sz.w;

				int dirtBeg = firstRenderLED*meterLen/m_nLEDs;
				int dirtLen = (lastRenderLED+1)*meterLen/m_nLEDs + 1 - dirtBeg;	// One pixels margin, due to subpixel precision one LED can end on same that the next start

				switch( m_direction )
				{
					case WgDirection::Undefined:
						break;
					case WgDirection::Up:

						rect.x = 0;
						rect.y = sz.h - (dirtBeg+dirtLen);
						rect.w = sz.w;
						rect.h = dirtLen;
					break;

					case WgDirection::Down:
						rect.x = 0;
						rect.y = dirtBeg;
						rect.w = sz.w;
						rect.h = dirtLen;
					break;

					case WgDirection::Left:
						rect.x = sz.w - (dirtBeg+dirtLen);
						rect.y = 0;
						rect.w = dirtLen;
						rect.h = sz.h;
					break;

					case WgDirection::Right:
						rect.x = dirtBeg;
						rect.y = 0;
						rect.w = dirtLen;
						rect.h = sz.h;
					break;
				}

				_requestRender(rect);
			}
		}
		break;

		default:
			WgWidget::_onEvent(pEvent,pHandler);
		break;
	}
}





//____ _onRender() _____________________________________________________________________

void WgTuner::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	int p = m_iSidePadding;

	float ledSize = ((m_direction == WgDirection::Up || m_direction == WgDirection::Down)?_canvas.h:_canvas.w) / (float)(m_nLEDs + (m_nLEDs-1)*m_LEDSpacing);
	float stepSize = ledSize * (1.f+m_LEDSpacing);

	WgRectF ledRect;
	float	stepX;
	float	stepY;

	switch( m_direction )
	{
		case WgDirection::Undefined:
			break;
		case WgDirection::Up:
			ledRect = WgRectF( (float) (_canvas.x + p), _canvas.y + _canvas.h - ledSize, (float) (_canvas.w - 2*p), ledSize );
			stepX = 0.f;
			stepY = -stepSize;
			break;
		case WgDirection::Down:
			ledRect = WgRectF( (float) (_canvas.x+p), (float) _canvas.y, (float) (_canvas.w - 2*p), ledSize );
			stepX = 0.f;
			stepY = stepSize;
			break;
		case WgDirection::Left:
			ledRect = WgRectF( _canvas.x + _canvas.w - ledSize, (float) (_canvas.y + p), ledSize, (float) (_canvas.h - 2*p));
			stepX = -stepSize;
			stepY = 0.f;
			break;
		case WgDirection::Right:
			ledRect = WgRectF( (float) _canvas.x, (float) (_canvas.y + p), ledSize, (float) (_canvas.h - 2*p));
			stepX = stepSize;
			stepY = 0.f;
			break;
	}

	if( ledRect.w <= 0.f || ledRect.h <= 0.f )
		return;

    if(m_bUseBackgroundColor)
    {
        pDevice->fill(_canvas*64, m_BackgroundColor);
    }

	for( int i = 0 ; i < m_nLEDs ; i++ )
	{
		int section;
        int sum = 0;
        for(int n = 0; n < m_nSectionLEDs.size(); ++n)
        {
            sum += m_nSectionLEDs[n];
            if (i < sum)
            {
                section = n;
                break;
            }
        }
		/*if( i < m_nSectionLEDs[0] )
			section = 0;
		else if( i < m_nSectionLEDs[0] + m_nSectionLEDs[1] )
			section = 1;
		else
			section = 2;*/

		WgColor color;
		//if(m_bTunerMode && ((i==0) || (i==(m_nLEDs-1))))
		//	color = m_TunerIndicatorCol*m_LEDStates[i];
		//else
		color = m_LEDColors[section].first*(1.0f-m_LEDStates[i]) + m_LEDColors[section].second*m_LEDStates[i];

        pDevice->fill(wg::RectSPX(ledRect * 64), color);

        ledRect.x += stepX;
		ledRect.y += stepY;
	}

}

//____ _onCloneContent() _________________________________________________________________

void WgTuner::_onCloneContent( const WgWidget * _pOrg )
{
	/*const WgTuner * pOrg = static_cast<const WgTuner*>(_pOrg);

	for( int i = 0 ; i < 3 ; i++ )
	{
		m_LEDColors[i].first 	= pOrg->m_LEDColors[i].first;
		m_LEDColors[i].second 	= pOrg->m_LEDColors[i].second;
		m_nSectionLEDs[i] = pOrg->m_nSectionLEDs[i];
	}

	m_direction = pOrg->m_direction;
	m_nLEDs = pOrg->m_nLEDs;
	m_LEDSpacing = pOrg->m_LEDSpacing;
	m_iPeak = pOrg->m_iPeak;
	m_iHold = pOrg->m_iHold;*/
}

//____ _onAlphaTest() ____________________________________________________________________

bool WgTuner::_onAlphaTest( const WgCoord& ofs )
{
	return false;
}
