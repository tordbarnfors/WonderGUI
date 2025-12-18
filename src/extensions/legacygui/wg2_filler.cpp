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
#include <wg2_filler.h>
#include <wg2_gfxdevice.h>

static const char	c_widgetType[] = {"Filler"};

//____ Constructor ____________________________________________________________

WgFiller::WgFiller()
{
	m_defaultSize = WgSize(1,1);
	m_bOpaque = false;
	m_mode = WG_MODE_NORMAL;
}

//____ Destructor _____________________________________________________________

WgFiller::~WgFiller()
{
}

//____ Type() _________________________________________________________________

const char * WgFiller::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgFiller::GetClass()
{
	return c_widgetType;
}

//____ SetPreferredPixelSize() _______________________________________________________

void WgFiller::SetPreferredPointSize( const WgSize& size )
{
	if( size != m_defaultSize )
	{
		m_defaultSize = size;
		_requestResize();
	}
}

//____ SetColors() _____________________________________________________________

void WgFiller::SetColors( const WgColorsetPtr& pColors )
{
	WgColor oldC = m_pColors?m_pColors->Color(m_mode):WgColor(0,0,0,0);
	WgColor newC = pColors?pColors->Color(m_mode):WgColor(0,0,0,0);

	m_pColors = pColors;

	if( oldC != newC )
	{
		m_bOpaque = (newC.a == 255);
		_requestRender();
	}
}

//____ PreferredPixelSize() __________________________________________________________

WgSize WgFiller::PreferredPixelSize() const
{
	return m_defaultSize*m_scale/WG_SCALE_BASE;
}

//____ SetSkinValue() _____________________________________________________________

void WgFiller::SetSkinValue( float value )
{
	if( value != m_skinValue )
	{
		if( m_pSkin )
		{
			wg::RectSPX rect = m_pSkin->_dirtyRect(PixelSize()*64, m_scale, m_state, m_state, m_skinValue, value );
			m_skinValue = value;
			_requestRender( rect/64 );
		}
		else
		{
			m_skinValue = value;
		}

	}
}

//____ _setScale() ____________________________________________________________

void WgFiller::_setScale(int scale)
{
	WgWidget::_setScale(scale);

	_requestResize();
}


//____ _onCloneContent() _______________________________________________________

void WgFiller::_onCloneContent( const WgWidget * _pOrg )
{
	WgFiller * pOrg = (WgFiller*) _pOrg;

	m_defaultSize = pOrg->m_defaultSize;
	m_pColors = pOrg->m_pColors;
}

//____ _onRender() _____________________________________________________________

void WgFiller::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	if( m_pColors )
		pDevice->fill( _canvas*64, m_pColors->Color(m_mode) );
    else if( m_pSkin )
        _renderSkin(m_pSkin, pDevice, m_state, _canvas, m_scale, m_skinValue );
}

//____ _onAlphaTest() ___________________________________________________________

bool WgFiller::_onAlphaTest( const WgCoord& ofs )
{
	if( m_pColors && m_pColors->Color(m_mode).a != 0 )
		return true;

	if(m_pSkin)
		return _markTestSkin(m_pSkin, ofs, PixelSize(), wg::State::Default, m_markOpacity, m_scale, m_skinValue);

	return false;
}

//____ _isColorChanged() ______________________________________________________

bool WgFiller::_isColorChanged( WgMode newMode ) const
{
	if( newMode != m_mode && m_pColors && m_pColors->Color(m_mode) != m_pColors->Color(newMode) )
		return true;

	return false;
}

//____ _onEnable() _____________________________________________________________

void WgFiller::_onEnable()
{
	if( _isColorChanged(WG_MODE_NORMAL) )
		_requestRender();

	if( m_pColors && m_pColors->Color(WG_MODE_NORMAL).a == 255 )
		m_bOpaque = true;
	else
		m_bOpaque = false;

	m_mode = WG_MODE_NORMAL;
}

//____ _onDisable() ____________________________________________________________

void WgFiller::_onDisable()
{
	if( _isColorChanged(WG_MODE_DISABLED) )
		_requestRender();

	if( m_pColors && m_pColors->Color(WG_MODE_DISABLED).a == 255 )
		m_bOpaque = true;
	else
		m_bOpaque = false;

	m_mode = WG_MODE_DISABLED;
}




