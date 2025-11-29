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
#include	<stdio.h>
#include	<wg2_fpsdisplay.h>
#include	<wg2_gfxdevice.h>
#include	<wg2_color.h>
#include	<wg_char.h>
#include    <wg_texttool.h>
#include	<wg2_pen.h>
#include	<wg2_base.h>

static const char	c_widgetType[] = {"FpsDisplay"};


#define		TICK_BUFFER			64



//____ WgFpsDisplay() _________________________________________________________________

WgFpsDisplay::WgFpsDisplay( void )
{
	m_pTickBuffer		= new Uint32[TICK_BUFFER];
	for( int i = 0 ; i < TICK_BUFFER ; i++ )
		m_pTickBuffer[i] = 1;

	m_tickBufferOfs		= 0;
	m_bOpaque			= false;
}

//____ ~WgFpsDisplay() __________________________________________________________

WgFpsDisplay::~WgFpsDisplay( void )
{
	if( m_pTickBuffer )
	{
		delete [] m_pTickBuffer;
		m_pTickBuffer = 0;
	}
}


//____ Type() _________________________________________________________________

const char * WgFpsDisplay::Type( void ) const
{
	return GetClass();
}

const char * WgFpsDisplay::GetClass( void )
{
	return c_widgetType;
}


//____ SetTextStyle() ____________________________________________________

void WgFpsDisplay::SetTextStyle( wg::TextStyle * pStyle )
{
	m_pStyle = pStyle;
	_requestRender();
}

//____ PreferredPixelSize() __________________________________________________________

WgSize WgFpsDisplay::PreferredPixelSize() const
{
	//TODO: Implement!

	return WgSize(1,1);
}


//____ _onRender() ________________________________________________________

void WgFpsDisplay::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{

	const int	cCurrentFrames = 10;
	int currOfs = ((int)m_tickBufferOfs) - cCurrentFrames;
	if( currOfs < 0 )
		currOfs += TICK_BUFFER;

	Uint32	currTotal = 0;
	for( int i = 0 ; i < cCurrentFrames ; i++ )
	{
		currTotal += m_pTickBuffer[currOfs++];
		currOfs %= TICK_BUFFER;
	}
	float	fpsCurrent = 1000.f / (currTotal / (float) cCurrentFrames);

	//____

	Uint32	avg = 0;
	for( int i = 0 ; i < TICK_BUFFER ; i++ )
		avg += m_pTickBuffer[i];
	float fpsAvg = 1000.f / (((float)avg)/TICK_BUFFER);

	//____

	Uint32	min = 1000000000;
	for( int i = 0 ; i < TICK_BUFFER ; i++ )
		if( min > m_pTickBuffer[i] )
			min = m_pTickBuffer[i];
	float fpsMax = 1000.f / min;

	//____

	Uint32	max = 0;
	for( int i = 0 ; i < TICK_BUFFER ; i++ )
		if( max < m_pTickBuffer[i] )
			max = m_pTickBuffer[i];
	float fpsMin = 1000.f / max;

	//____

	char	temp[40];
	wg::Char	temp2[40];
	const char *	pTemp;

	WgPen	pen( pDevice, _canvas);
	wg::TextAttr attr;

	if( m_pStyle )
		m_pStyle->exportAttr( wg::State::Default, &attr, m_scale  );

	pen.SetAttributes( attr );
	pen.SetPos( WgCoord(_canvas.x, _canvas.y + pen.GetBaseline()) );

	int height = pen.GetLineSpacing();

	pTemp = temp;
	snprintf( temp, 40, "Now: %.2f", fpsCurrent );
	wg::TextTool::readString( pTemp, temp2, 39 );

	WgGfxDevice::PrintLine( pDevice, pen, attr, temp2 );

	pTemp = temp;
	snprintf( temp, 40, "Min: %.2f", fpsMin );
	wg::TextTool::readString( pTemp, temp2, 39 );
	pen.SetPosX( _canvas.x );
	pen.MoveY( height );
	WgGfxDevice::PrintLine( pDevice, pen, attr, temp2 );

	pTemp = temp;
	snprintf( temp, 40, "Avg: %.2f", fpsAvg );
	wg::TextTool::readString( pTemp, temp2, 39 );
	pen.SetPosX( _canvas.x );
	pen.MoveY( height );
	WgGfxDevice::PrintLine( pDevice, pen, attr, temp2 );

	pTemp = temp;
	snprintf( temp, 40, "Max: %.2f", fpsMax );
	wg::TextTool::readString( pTemp, temp2, 39 );
	pen.SetPosX( _canvas.x );
	pen.MoveY( height );
	WgGfxDevice::PrintLine( pDevice, pen, attr, temp2 );
}


//____ _onEvent() _____________________________________________________________

void WgFpsDisplay::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	switch( pEvent->Type() )
	{
		case WG_EVENT_TICK:
		{
			m_tickBufferOfs = (++m_tickBufferOfs) % TICK_BUFFER;

			int msDiff = ((WgEvent::Tick*)pEvent)->Millisec();
			if( msDiff > 0 )
				m_pTickBuffer[m_tickBufferOfs] = msDiff;
			else
				m_pTickBuffer[m_tickBufferOfs] = 1;
			_requestRender();
		}
		default:
			WgWidget::_onEvent(pEvent,pHandler);
			break;
	}
}

//____ DoMyOwnCloning() _______________________________________________________

void WgFpsDisplay::_onCloneContent( const WgWidget * _pOrg )
{
	WgFpsDisplay * pOrg		= (WgFpsDisplay *) _pOrg;

	m_pStyle			= pOrg->m_pStyle;
	m_tickBufferOfs	= pOrg->m_tickBufferOfs;

	for( int i = 0 ; i < TICK_BUFFER ; i++ )
		m_pTickBuffer[i] = pOrg->m_pTickBuffer[i];
}


