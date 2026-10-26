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
#include <wg2_mouseblocker.h>
#include <wg2_gfxdevice.h>

static const char	c_widgetType[] = {"MouseBlocker"};

//____ Constructor ____________________________________________________________

WgMouseBlocker::WgMouseBlocker()
{
	m_defaultSize = WgSize(1,1);
	m_bOpaque = false;
}

//____ Destructor _____________________________________________________________

WgMouseBlocker::~WgMouseBlocker()
{
}

//____ Type() _________________________________________________________________

const char * WgMouseBlocker::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgMouseBlocker::GetClass()
{
	return c_widgetType;
}

//____ SetPreferredPixelSize() _______________________________________________________

void WgMouseBlocker::SetPreferredPointSize( const WgSize& size )
{
	if( size != m_defaultSize )
	{
		m_defaultSize = size;
		_requestResize();
	}
}

//____ PreferredPixelSize() __________________________________________________________

WgSize WgMouseBlocker::PreferredPixelSize() const
{
	return m_defaultSize*m_scale/WG_SCALE_BASE;
}


//____ SetHoles() ____________________________________________________________

void WgMouseBlocker::SetHoles( int nbHoles, const WgRect * pHoles )
{
	m_holes.clear();
	for( int i = 0 ; i < nbHoles ; i++ )
		m_holes.push_back(pHoles[i]);
}


//____ _setScale() ____________________________________________________________

void WgMouseBlocker::_setScale(int scale)
{
	WgWidget::_setScale(scale);

	_requestResize();
}


//____ _onCloneContent() _______________________________________________________

void WgMouseBlocker::_onCloneContent( const WgWidget * _pOrg )
{
	WgMouseBlocker * pOrg = (WgMouseBlocker*) _pOrg;

	m_defaultSize = pOrg->m_defaultSize;
}


//____ _onAlphaTest() ___________________________________________________________

bool WgMouseBlocker::_onAlphaTest( const WgCoord& ofs )
{
	WgCoord pointOfs = ofs * m_scale / WG_SCALE_BASE;
	
	for( auto& hole : m_holes )
		if(hole.contains(ofs))
			return false;
	
	return true;
}





