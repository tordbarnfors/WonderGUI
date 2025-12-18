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
#include <wg2_iconholder.h>
#include <wg2_util.h>
#include <wg2_base.h>
#include <wg_util.h>
#include <wg_gfxbase.h>

#include <algorithm>

//____ Constructor ____________________________________________________________

WgIconHolder::WgIconHolder()
{
	m_iconOrigo	= WgOrigo::NorthWest;
	m_iconScale			= 0.f;
	m_bIconPushText		= true;
}

//____ SetIconScale() _________________________________________________________

bool WgIconHolder::SetIconScale( float scaleFactor )
{
	if( scaleFactor > 1.f || scaleFactor < 0.f )
		return false;

	if( scaleFactor != m_iconScale )
	{
		m_iconScale = scaleFactor;
		_iconModified();
	}

	return true;
}

//____ SetIconOrigo() ___________________________________________________

void WgIconHolder::SetIconOrigo( WgOrigo origo )
{
	if( origo != m_iconOrigo )
	{
		m_iconOrigo = origo;
		_iconModified();
	}
}

//____ SetIconBorders() _______________________________________________________

void WgIconHolder::SetIconBorders( WgBorders borders )
{
	if( borders != m_iconBorders )
	{
		m_iconBorders = borders;
		_iconModified();
	}
}

//____ SetIconPushingText() _________________________________________________________

void WgIconHolder::SetIconPushingText( bool bPush )
{
	if( bPush != m_bIconPushText )
	{
		m_bIconPushText = bPush;
		_iconModified();
	}
}


//____ _getIconRect() _________________________________________________________

/*
	Gets an icon-rect for the icon excluding borders, relative to upper left corner of content area.

*/

WgRect WgIconHolder::_getIconRect( const WgRect& contentRect, wg::Skin * pSkin, int scale ) const
{
	if( pSkin )
		return _getIconRect(contentRect, _skinPrefSize( pSkin, scale ), scale);
	else
		return WgRect();
}

WgRect WgIconHolder::_getIconRect( const WgRect& contentRect, const WgSize& iconSize, int scale ) const
{
	WgRect rect;

	WgBorders iconBorders = m_iconBorders.scale(scale);

	int w = iconSize.w;
	int h = iconSize.h;

	if( w > 0 && h > 0 )
	{
		int bgW = contentRect.w - iconBorders.width();
		int bgH = contentRect.h - iconBorders.height();

		if( m_iconScale != 0.f )
		{
			if( (w / (float) bgW) > (h / (float) bgH) )
			{
				h = (int) ((h * bgW * m_iconScale) / h);
				w = (int) (bgW * m_iconScale);
			}
			else
			{
				w = (int) ((w * bgH * m_iconScale) / h);
				h = (int) (bgH * m_iconScale);
			}
		}

		//

		w += iconBorders.width();
		h += iconBorders.height();

		rect = WgUtil::OrigoToRect( m_iconOrigo, contentRect.size(), WgSize(w,h) );
		rect += contentRect.pos();
		rect -= iconBorders;
	}

	return rect;
}



//____ _getTextRect() _____________________________________________________

WgRect WgIconHolder::_getTextRect( const WgRect& contentRect, const WgRect& iconRect ) const
{
	WgRect textRect = contentRect;

	if( m_bIconPushText && iconRect.w > 0 && iconRect.h > 0 )
	{
		switch( m_iconOrigo )
		{
			default:
			case WgOrigo::NorthWest:
			case WgOrigo::SouthWest:
			case WgOrigo::West:
			{
				int diff = iconRect.x - contentRect.x + iconRect.w + m_iconBorders.right;
				textRect.x += diff;
				textRect.w -= diff;
				if( textRect.w < 0 )
					textRect.w = 0;
				break;
			}
			case WgOrigo::NorthEast:
			case WgOrigo::East:
			case WgOrigo::SouthEast:
			{
				textRect.w = iconRect.x - contentRect.x - m_iconBorders.left;
				if( textRect.w < 0 )
					textRect.w = 0;
				break;
			}

			case WgOrigo::North:
			case WgOrigo::Center:
			{
				int diff = iconRect.y - contentRect.y + iconRect.h + m_iconBorders.bottom;
				textRect.y += diff;
				textRect.h -= diff;
				if( textRect.h < 0 )
					textRect.h = 0;
				break;
			}
			case WgOrigo::South:
			{
				textRect.h = iconRect.y - contentRect.y - m_iconBorders.top;
				if( textRect.h < 0 )
					textRect.h = 0;
				break;
			}
		}
	}

	return textRect;
}

//____ _expandTextRect() ______________________________________________________

WgSize WgIconHolder::_expandTextRect(WgSize textRectSize, wg::Skin * pIconSkin, int scale) const
{
	if (!pIconSkin)
		return textRectSize;

	WgSize iconSize = _skinPrefSize(pIconSkin, scale) + m_iconBorders.scale(scale);

	if (m_bIconPushText)
	{
		if (m_iconOrigo == WgOrigo::North || m_iconOrigo == WgOrigo::Center || m_iconOrigo == WgOrigo::South )
			return { std::max(textRectSize.w, iconSize.w), textRectSize.h + iconSize.h };
		else
			return { textRectSize.w + iconSize.w , std::max(textRectSize.h, iconSize.h) };
	}
	else
		return { std::max(textRectSize.w, iconSize.w), std::max(textRectSize.h, iconSize.h) };
}


//____ _onCloneContent() ________________________________________________________

void WgIconHolder::_onCloneContent( const WgIconHolder * _pOrg )
{
	m_iconOrigo	= _pOrg->m_iconOrigo;
	m_iconScale			= _pOrg->m_iconScale;
	m_bIconPushText		= _pOrg->m_bIconPushText;
	m_iconBorders		= _pOrg->m_iconBorders;
}

//____ _skinPreferredSize() __________________________________________________________

wg::SizeI WgIconHolder::_skinPrefSize( wg::Skin * pSkin, int scale ) const
{
	return pSkin->_defaultSize(scale >> 6) / 64;
}


