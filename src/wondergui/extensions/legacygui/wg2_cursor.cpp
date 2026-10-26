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
#include <wg2_cursor.h>


//____ WgCursor() _____________________________________________________________

WgCursor::WgCursor()
{
	for( int i = 0 ; i < N_MODES ; i++ )
	{
		m_pAnim[i]	= 0;
		m_advance[i] = 0;
		m_sizeRatio[i]	= 1.f;
	}
	m_blitMode = NORMAL;
}

WgCursor::WgCursor(	WgCursor& in )
{
	for( int i = 0 ; i < N_MODES ; i++ )
	{
		m_pAnim[i]		= in.m_pAnim[i];
		m_bearing[i]	= in.m_bearing[i];
		m_advance[i]	= in.m_advance[i];
		m_sizeRatio[i]	= in.m_sizeRatio[i];
	}

	m_blitMode = in.m_blitMode;
}

//____ SetBlitMode() __________________________________________________________

void WgCursor::SetBlitMode( BlitMode mode )
{
	m_blitMode = mode;
}


//____ SetSizeRatio() _________________________________________________________

void WgCursor::SetSizeRatio( Mode m, float ratio )
{
	if( m < 0 || m >= (Mode) N_MODES )
		return;

	m_sizeRatio[m] = ratio;
}


//____ SetMode() ______________________________________________________________

bool WgCursor::SetMode( Mode m, WgGfxAnim * pAnim, WgCoord bearing, int advance, float size_ratio )
{
	if( m < 0 || m >= (Mode) N_MODES )
		return false;

	m_pAnim[m]			= pAnim;
	m_bearing[m]		= bearing;
	m_advance[m]		= advance;
	m_sizeRatio[m]		= size_ratio;

	return true;
}

//____ SetBearing() ___________________________________________________________

void WgCursor::SetBearing( Mode m, WgCoord bearing )
{
	if( m < 0 || m >= (Mode) N_MODES )
		return;

	m_bearing[m] = bearing;
}


//____ SetAdvance() ___________________________________________________________

void WgCursor::SetAdvance( Mode m, int advance )
{
	if( m < 0 || m >= (Mode) N_MODES )
		return;

	m_advance[m] = advance;
}


//____ SetAnim() ______________________________________________________________

void WgCursor::SetAnim( Mode m, WgGfxAnim * pAnim )
{
	if( m < 0 || m >= (Mode) N_MODES )
		return;

	m_pAnim[m] = pAnim;
}

