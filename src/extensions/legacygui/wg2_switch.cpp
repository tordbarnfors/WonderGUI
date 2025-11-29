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
#include <wg2_switch.h>
#include <wg2_gfxdevice.h>

#include <algorithm>

static const char	c_widgetType[] = {"Switch"};

//____ Constructor ____________________________________________________________

WgSwitch::WgSwitch()
{
	m_bOpaque = false;
	m_iValue = 0;
	m_onColor = WgColor::White;
	m_offColor = WgColor(64, 64, 64, 255);
	m_preferredSize = WgSize(46,46);
}

//____ Destructor _____________________________________________________________

WgSwitch::~WgSwitch()
{
}

//____ Type() _________________________________________________________________

const char * WgSwitch::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgSwitch::GetClass()
{
	return c_widgetType;
}

//____ SetValue() _____________________________________________________________

void WgSwitch::SetValue( const float value )
{
	int iValue;

	if(value > 0.6667f)
		iValue = 0;
	else if(value > 0.3333f)
		iValue = 1;
	else
		iValue = 2;

	if(m_iValue == iValue)
		return;

	m_iValue = iValue;
	_requestRender();
}

//____ SetColor() __________________________________________________________

void WgSwitch::SetColor( WgColor color )
{
	if( color == m_onColor )
		return;

	m_onColor = color;
	const float tint = 0.25f;
	m_offColor = color;
//    m_offColor.r = m_onColor.r * tint;
//    m_offColor.g = m_onColor.g * tint;
//    m_offColor.b = m_onColor.b * tint;
	m_offColor.a = (Uint8)((float)m_onColor.a * tint);

	_requestRender();
}

//____ SetOffColor() __________________________________________________________

void WgSwitch::SetOffColor(WgColor color )
{
	m_offColor = color;
}

//____ PreferredPixelSize() __________________________________________________________

void WgSwitch::SetPreferredPixelSize(WgSize size)
{
	m_preferredSize = size;
}

WgSize WgSwitch::PreferredPixelSize() const
{
	return m_preferredSize;
}


//____ _onCloneContent() _______________________________________________________

void WgSwitch::_onCloneContent( const WgWidget * _pOrg )
{
	const WgSwitch * pOrg = static_cast<const WgSwitch*>(_pOrg);
}

//____ _onRender() _____________________________________________________________

void WgSwitch::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	int sz = std::min( _canvas.w,_canvas.h );

	WgColor color;
	if( sz > 12 )
	{
		int stepping = (int)floor((float)sz*(1.0f/8.0f));
		int itemSize = stepping*2;

		int y = _canvas.y;
		int x = _canvas.x + (sz - itemSize) / 2;
		for( int i = 0 ; i < 3 ; i++ )
		{
			if (i == m_iValue) {
				color = m_onColor;
			} else {
				color = m_offColor;
			}
//			pDevice->ClipDrawElipse( _clip, WgRect(x,y,itemSize,itemSize), color );
			y += itemSize + stepping;
		}
	}
}

//____ _onAlphaTest() ___________________________________________________________

bool WgSwitch::_onAlphaTest( const WgCoord& ofs )
{
	return false;
}


//____ _onEnable() _____________________________________________________________

void WgSwitch::_onEnable()
{
	_requestRender();
}

//____ _onDisable() ____________________________________________________________

void WgSwitch::_onDisable()
{
	_requestRender();
}




