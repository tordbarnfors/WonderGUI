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
#include <wg2_scaleimage.h>
#include <wg2_gfxdevice.h>
#include <wg2_util.h>


static const char	c_widgetType[] = {"ScaleImage"};

//____ Constructor ____________________________________________________________

WgScaleImage::WgScaleImage()
{
	m_pOrgSurface = 0;
	m_pGenSurface = 0;
	m_pSurfaceFactory = 0;
	m_bScale = false;
	m_maxImgSize = WgSize(4096,2048);//WgSize(1024,1024);
	m_imgOrigo = WgOrigo::NorthWest;
}

//____ Destructor _____________________________________________________________

WgScaleImage::~WgScaleImage()
{
}

//____ Type() _________________________________________________________________

const char * WgScaleImage::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgScaleImage::GetClass()
{
	return c_widgetType;
}

//____ SetSurfaceFactory() ____________________________________________________

void WgScaleImage::SetSurfaceFactory( wg::SurfaceFactory * pFactory )
{
	m_pSurfaceFactory = pFactory;
}

//____ SetSource() _____________________________________________________________

void WgScaleImage::SetSource( wg::Surface * pSurf )
{
	if( pSurf != m_pOrgSurface )
	{
		m_pOrgSurface = pSurf;
		_recalcImageRect( PixelSize(), false );
		_regenerateSurface();
	}
}

//____ SetImageMaxSize() ______________________________________________________

void WgScaleImage::SetImageMaxSize( WgSize max )
{
	bool	bRegenerate = false;
	if( max.w < m_imgRect.w || max.h < m_imgRect.h ||
		(m_imgRect.w == m_maxImgSize.w && max.w > m_maxImgSize.w) ||
		(m_imgRect.h == m_maxImgSize.h && max.h > m_maxImgSize.h) )
		bRegenerate = true;

	m_imgRect.setSize(max);

	if( bRegenerate )
	{
		_recalcImageRect( PixelSize(), false );
		_regenerateSurface();
	}
}


//____ _recalcImageRect() _____________________________________________________

void WgScaleImage::_recalcImageRect( WgSize widgetSize, bool bRegenerate )
{
	int w = m_pOrgSurface->pixelSize().w;
	int h = m_pOrgSurface->pixelSize().h;

	if( w > 0 && h > 0 )
	{
		int bgW = widgetSize.w;
		int bgH = widgetSize.h;

		if( bgW > m_maxImgSize.w )
			bgW = m_maxImgSize.w;

		if( bgH > m_maxImgSize.h )
			bgH = m_maxImgSize.h;

		if( m_bScale )
		{
			if( (w / (float) bgW) > (h / (float) bgH) )
			{
				h = (int) ((h * bgW) / w);
				w = (int) (bgW);
			}
			else
			{
				w = (int) ((w * bgH) / h);
				h = (int) (bgH);
			}
		}

		//

		WgRect imgRect = WgUtil::OrigoToRect( m_imgOrigo, widgetSize, WgSize(w,h) );

		if( imgRect.size() != m_imgRect.size() )
		{
			m_imgRect = imgRect;
			if( bRegenerate )
				_regenerateSurface();
			else
				m_pGenSurface = nullptr;

		}
		else if( imgRect.pos() != m_imgRect.pos() )
		{
			m_imgRect = imgRect;
			_requestRender();
		}
	}
	else
	{
		m_imgRect.clear();
		m_pGenSurface = 0;
	}
}


//____ _regenerateSurface() ___________________________________________________

void WgScaleImage::_regenerateSurface()
{
	// Check/set opacity

	if( m_pOrgSurface && m_pOrgSurface->isOpaque() && !m_bScale )
		m_bOpaque = true;
	else
		m_bOpaque = false;

	//

	if( m_pSurfaceFactory )
	{
		m_pGenSurface = m_pSurfaceFactory->createSurface( WGBP(Surface,
															_.size = m_imgRect.size(), 
												 			_.format = WgPixelType::BGRA_8 ));

		// Insert code here to stretch-copy content from m_pOrgSurface to m_pGenSurface
		resample(m_pOrgSurface, m_pGenSurface);

//		m_pGenSurface->Fill( WgColor::rosybrown );
	}
	else
	{
		m_pGenSurface = nullptr;
		return;
	}

	// Force redraw

	_requestRender();
}


//____ SetImageScale() _____________________________________________________________

void WgScaleImage::SetImageScale( bool bScale )
{
	if( bScale != m_bScale )
	{
		m_bScale = bScale;
		_recalcImageRect( PixelSize(), false );
		_regenerateSurface();
	}
}

//____ SetImageOrigo() _____________________________________________________________

void WgScaleImage::SetImageOrigo( WgOrigo origo )
{
	if( origo != m_imgOrigo )
	{
		m_imgOrigo = origo;
		if( m_bScale )
		{
			_recalcImageRect( PixelSize(), true );
			_requestRender();
		}
	}
}


//____ PreferredPixelSize() _____________________________________________________________

WgSize WgScaleImage::PreferredPixelSize() const
{
	if( m_pOrgSurface )
		return m_pOrgSurface->pixelSize();

	return WgSize(1,1);
}


//____ _onNewSize() ___________________________________________________________

void WgScaleImage::_onNewSize( const WgSize& size )
{
	_recalcImageRect( size, false );
	_requestPreRenderCall();
}


//____ _onCloneContent() _______________________________________________________

void WgScaleImage::_onCloneContent( const WgWidget * _pOrg )
{
	WgScaleImage * pOrg = (WgScaleImage*) _pOrg;

	m_pSurfaceFactory = pOrg->m_pSurfaceFactory;
	m_pOrgSurface = pOrg->m_pOrgSurface;
	m_bScale = pOrg->m_bScale;
	m_maxImgSize = pOrg->m_maxImgSize;
	m_imgOrigo = pOrg->m_imgOrigo;
}

//____ _preRender() ___________________________________________________________

void WgScaleImage::_preRender()
{
	if( !m_pGenSurface )
		_regenerateSurface();
}


//____ _onRender() _____________________________________________________________

void WgScaleImage::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window)
{
	if( !m_pGenSurface )
		return;

	pDevice->setBlitSource(m_pGenSurface);
	pDevice->blit( WgCoord( _canvas.x + m_imgRect.x, _canvas.y + m_imgRect.y )*64 );
}

//____ _onAlphaTest() ___________________________________________________________

bool WgScaleImage::_onAlphaTest( const WgCoord& ofs )
{
	if( !m_pGenSurface )
		return	false;												// No visible pixel, so don't accept the mark...


	if (m_imgRect.contains(ofs))
	{
		WgCoord ofs2 = ofs - m_imgRect.pos();
		Uint8 opacity = m_pGenSurface->alpha(ofs2*64);
		if (opacity > 0)
			return true;
	}
	return false;
}






