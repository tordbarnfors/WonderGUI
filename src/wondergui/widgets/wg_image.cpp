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
#include <wg_image.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>
#include <wg_surface.h>

namespace wg
{
	using namespace Util;

	const TypeInfo Image::TYPEINFO = { "Image", &Widget::TYPEINFO };


	//____ typeInfo() _________________________________________________________

	const TypeInfo& Image::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setImage() _____________________________________________________________

	void Image::setImage( Surface * pSurface, const Rect& rect )
	{
		if( pSurface != m_pSurface || rect != m_rect )
		{
			bool bResize = false;
			if( rect.w != m_rect.w || rect.h != m_rect.h )
				bResize = true;

			m_pSurface = pSurface;

			if( pSurface )
				m_rect = Rect::overlap( rect, Rect(pSurface->pointSize()) );
			else
				m_rect.clear();

			if( bResize )
				_requestResize();
			_requestRender();
		}
	}

	void Image::setImage( Surface * pSurface )
	{
		if( pSurface != m_pSurface )
		{
			bool bResize = false;
			if( !pSurface || !m_pSurface || pSurface->pointSize() != m_pSurface->pointSize() )
				bResize = true;

			m_pSurface = pSurface;

			if( pSurface )
				m_rect = pSurface->pointSize();
			else
				m_rect.clear();

			if( bResize )
				_requestResize();
			_requestRender();
		}
	}

	//____ setImageTint() ______________________________________________________

	void Image::setImageTint(HiColor tint)
	{
		if (tint != m_imageTint)
		{
			m_imageTint = tint;
			_requestRender();
		}
	}

	//____ _matchingHeight() _____________________________________________________

	spx Image::_matchingHeight(spx width, int scale) const
	{
		SizeSPX borderSize = m_skin.contentBorderSize(scale);

		spx contentHeight = borderSize.h;

		if( m_pSurface )
		{
			if( m_sizePolicy == SizePolicy2D::Scale )
			{
				spx contentWidth = width - borderSize.w;
				if( contentWidth > 0 )
					contentHeight += align(int(int64_t(contentWidth * m_rect.h) / m_rect.w));
			}
			else if( m_sizePolicy == SizePolicy2D::ScaleDown )
			{
				spx contentWidth = width - borderSize.w;

				if( m_pSurface && m_pSurface->pixelWidth()*64 > contentWidth )
					contentHeight += align(int(int64_t(contentWidth * m_rect.h) / m_rect.w));
				else
					contentHeight += align(ptsToSpx(m_rect.h,scale));
			}
			else
				contentHeight += align(ptsToSpx(m_rect.h,scale));
		}

		return contentHeight;
	}

	//____ _matchingWidth() ______________________________________________________

	spx	Image::_matchingWidth(spx height, int scale) const
	{
		SizeSPX borderSize = m_skin.contentBorderSize(scale);

		spx contentWidth = borderSize.w;
		
		if( m_pSurface )
		{
			if( m_sizePolicy == SizePolicy2D::Scale )
			{
				int contentHeight = height - borderSize.h;
				if( contentHeight > 0 )
					contentWidth += align(int(int64_t(contentHeight * m_rect.w) / m_rect.h));
			}
			else if( m_sizePolicy == SizePolicy2D::ScaleDown )
			{
				int contentHeight = height - borderSize.h;

				if( m_pSurface && m_pSurface->pixelHeight()*64 > contentHeight )
					contentWidth += align(int(int64_t(contentHeight * m_rect.w) / m_rect.h));
				else
					contentWidth += align(ptsToSpx(m_rect.w,scale));
			}
			else
				contentWidth += align(ptsToSpx(m_rect.w,scale));
		}

		return contentWidth;
	}

	//____ _defaultSize() _____________________________________________________________

	SizeSPX Image::_defaultSize(int scale) const
	{
		SizeSPX size = m_skin.contentBorderSize(scale);
		
		if (m_pSurface)
			size += align(ptsToSpx(m_rect.size(),scale));

		return size;
	}

	//____ _render() _____________________________________________________________

	void Image::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		Widget::_render(pDevice,_canvas,_window);

		if( m_pSurface && !m_rect.isEmpty() )
		{
			RectSPX content = m_skin.contentRect( _canvas, m_scale, state() );

			RectSPX imgRect = ptsToSpx(m_rect, m_pSurface->scale());
			
			RectSPX dest = rectFromPolicy(m_sizePolicy, m_placement, content, imgRect.size() );

			bool bTint = (!m_imageTint.isUndefined() && m_imageTint != HiColor::White);

			if( bTint )
				pDevice->setTintColor(m_imageTint);

			pDevice->setBlitSource(m_pSurface);
			pDevice->stretchBlit( dest, imgRect );

			if (bTint)
				pDevice->setTintColor(HiColor::White);
		}
	}

	//____ _alphaTest() ___________________________________________________________

	bool Image::_alphaTest( const CoordSPX& ofs )
	{
		if( m_pSurface && !m_rect.isEmpty() )
		{
			RectSPX content = m_skin.contentRect( m_size, m_scale, state() );

			RectSPX imgRect = ptsToSpx(m_rect, m_pSurface->scale());
			
			RectSPX dest = rectFromPolicy(m_sizePolicy, m_placement, content, imgRect.size() );
		
			int imageAlpha = m_imageTint.isUndefined() ? 4096 : m_imageTint.a;

			if (imageAlpha == 0)
			{
				// Avoid division by zero

				if( m_imageMarkAlpha == 0 )
					return true;
			}
			else
			{
				int opacityTreshold = m_imageMarkAlpha * 4096 / imageAlpha;

				if (Util::markTestStretchRect(ofs, m_pSurface, imgRect, dest, opacityTreshold))
					return true;
			}
		}

		return Widget::_alphaTest(ofs);
	}

} // namespace wg
