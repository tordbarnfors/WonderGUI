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



#include <wg2_image.h>
#include <wg2_gfxdevice.h>
#include <wg2_util.h>


static const char	c_widgetType[] = {"Image"};

//____ Constructor ____________________________________________________________

WgImage::WgImage()
{
}

//____ Destructor _____________________________________________________________

WgImage::~WgImage()
{
}

//____ Type() _________________________________________________________________

const char * WgImage::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgImage::GetClass()
{
	return c_widgetType;
}

//____ SetSkin() ______________________________________________________________

void WgImage::SetSkin(wg::Skin * pSkin)
{
	if (pSkin != m_pSkin)
	{
		if( pSkin && pSkin->isOpaque() )
			m_bOpaque = true;
		else
			m_bOpaque = false;

		m_pSkin = pSkin;
		_requestResize();
		_requestRender();
	}
}

//____ SetImage() ______________________________________________________________

void WgImage::SetImage( wg::Surface * pSurface )
{
	m_pImage = pSurface;

	_requestResize();
	_requestRender();
}


//____ PreferredPixelSize() _____________________________________________________________

WgSize WgImage::PreferredPixelSize() const
{
	if( m_pImage )
	{
		WgSize sz = (m_pImage->pixelSize() * m_scale) / (m_pImage->scale()*4096/64);

		if( m_pSkin )
			sz += _skinContentPadding( m_pSkin, m_scale);

		return sz;
	}
	else if( m_pSkin )
		return _skinPreferredSize( m_pSkin, m_scale);

	return WgSize(1,1);
}

//____ MatchingPixelHeight() _____________________________________________________________

int  WgImage::MatchingPixelHeight(int pixelWidth) const
{
	WgSize imageSize;
	WgSize paddingSize;

	if( m_pImage )
	{
		imageSize = (m_pImage->pixelSize() * m_scale) / (m_pImage->scale()*4096/64);
		if( m_pSkin )
			paddingSize = _skinContentPadding( m_pSkin, m_scale);
	}
	else if(m_pSkin)
		imageSize = _skinPreferredSize( m_pSkin, WG_SCALE_BASE);
	else
		return 1;

	if( imageSize.w == 0 )
	   return 0;

	return ((pixelWidth-paddingSize.w) * imageSize.h / imageSize.w) + paddingSize.h;
}

//____ MatchingPixelWeight() _____________________________________________________________

int  WgImage::MatchingPixelWidth(int pixelHeight) const
{
	WgSize imageSize;
	WgSize paddingSize;

	if( m_pImage )
	{
		imageSize = (m_pImage->pixelSize() * m_scale) / (m_pImage->scale()*4096/64);
		if( m_pSkin )
			paddingSize = _skinContentPadding( m_pSkin, m_scale);
	}
	else if(m_pSkin)
		imageSize = _skinPreferredSize( m_pSkin, WG_SCALE_BASE);
	else
		return 1;

	if( imageSize.h == 0 )
		return 0;

	return ((pixelHeight-paddingSize.h) * imageSize.w / imageSize.h) + paddingSize.w;
}




//____ _onCloneContent() _______________________________________________________

void WgImage::_onCloneContent( const WgWidget * _pOrg )
{
	WgImage * pOrg = (WgImage*) _pOrg;

}

//____ _onRender() _____________________________________________________________

void WgImage::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	WgWidget::_onRender(pDevice, _canvas, _window);

	WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, _canvas, WgStateEnum::Normal, m_scale) : _canvas;

	if( m_pImage )
	{
		pDevice->setBlitSource(m_pImage);
		pDevice->stretchBlit(canvas*64);
	}
}

//____ _onAlphaTest() ___________________________________________________________

bool WgImage::_onAlphaTest( const WgCoord& ofs )
{
	WgSize sz = PixelSize();

	if( m_pImage )
	{
		if( m_pSkin && _markTestSkin( m_pSkin, ofs, sz, m_state, m_markOpacity, m_scale) )
			return true;

		WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, sz, m_state, m_scale) : WgRect(sz);
		if( canvas.contains(ofs) )
		{
			WgSize imgSize = m_pImage->pixelSize();

			WgCoord surfOfs = { (ofs.x - canvas.x)*imgSize.w/canvas.w, (ofs.y - canvas.y)*imgSize.h/canvas.h };
			m_pImage->alpha(surfOfs*64);
		}
	}
	else if( m_pSkin )
	{
		return _markTestSkin(m_pSkin, ofs, sz, m_state, m_markOpacity, m_scale);
	}

	return false;
}

//____ _onEnable() _____________________________________________________________

void WgImage::_onEnable()
{
	_requestRender();
}

//____ _onDisable() ____________________________________________________________

void WgImage::_onDisable()
{
	_requestRender();
}

//____ _setScale() ____________________________________________________________

void WgImage::_setScale( int scale )
{
	WgWidget::_setScale(scale);

	if( m_pSkin || m_pImage )
		_requestResize();
}



