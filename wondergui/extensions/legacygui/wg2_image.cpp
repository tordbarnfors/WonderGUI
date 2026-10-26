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

//____ SetImageTint() _________________________________________________________

void WgImage::SetImageTint( wg::Color color)
{
	m_imageTint = color;
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

//____ _imageRect() ___________________________________________________________

WgRect WgImage::_imageRect(const WgRect& _canvas)
{

	WgRect canvas = m_pSkin ? _skinContentRect(m_pSkin, _canvas, m_state, m_scale) : _canvas;

	if (m_bKeepAspectRatio)
	{
		WgSize imageSize = m_pImage->pixelSize();

		float imageRatio = imageSize.w / (float) imageSize.h;
		float canvasRatio = _canvas.w / (float)_canvas.h;

		WgSize outSize;

		if (imageRatio > canvasRatio)
		{
			outSize.w = canvas.w;
			outSize.h = (int) canvas.w / imageRatio;
		}
		else
		{
			outSize.w = (int) canvas.h * imageRatio;
			outSize.h = canvas.h;
		}

		return {canvas.x + (canvas.w - outSize.w) / 2 , canvas.y + (canvas.h - outSize.h) / 2, outSize.w, outSize.h };
	}
	else
		return canvas;
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

//	WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, _canvas, WgStateEnum::Default, m_scale) : _canvas;

	if( m_pImage )
	{
		if( m_imageTint.a != 0 )
		{
			if( m_imageTint != wg::Color::White )
				pDevice->setTintColor(m_imageTint);
				
			pDevice->setBlitSource(m_pImage);
			pDevice->stretchBlit(_imageRect(_canvas)*64);
			
			if( m_imageTint != wg::Color::White )
				pDevice->setTintColor(wg::Color::White);
		}
	}
}

//____ _onAlphaTest() ___________________________________________________________

bool WgImage::_onAlphaTest( const WgCoord& ofs )
{
	WgSize sz = PixelSize();

	if (m_pSkin && _markTestSkin(m_pSkin, ofs, sz, m_state, m_markOpacity, m_scale))
		return true;

	if( m_pImage )
	{
		WgRect canvas = m_pSkin ? _imageRect(sz) : WgRect(sz);
		if( canvas.contains(ofs) )
		{
			WgSize imgSize = m_pImage->pixelSize();

			WgCoord surfOfs = { (ofs.x - canvas.x)*imgSize.w/canvas.w, (ofs.y - canvas.y)*imgSize.h/canvas.h };
			return m_pImage->alpha(surfOfs*64) * m_imageTint.a / 255 * 255/4096;
		}
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



