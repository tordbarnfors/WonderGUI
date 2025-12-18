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
#include <wg2_flexpanel.h>
#include <wg2_patches.h>
#include <wg2_util.h>

static const char	c_widgetType[] = {"FlexPanel"};
static const char	c_hookType[] = {"FlexHook"};

WgFlexAnchor		WgFlexPanel::g_baseAnchors[wg::Placement_size] = {	WgFlexAnchor(0.f, 0.f, WgCoord(0,0)),
														WgFlexAnchor(0.f, 0.f, WgCoord(0,0)),
														WgFlexAnchor(0.5f, 0.f, WgCoord(0,0)),
														WgFlexAnchor(1.f, 0.f, WgCoord(0,0)),
														WgFlexAnchor(1.f, 0.5f, WgCoord(0,0)),
														WgFlexAnchor(1.f, 1.f, WgCoord(0,0)),
														WgFlexAnchor(0.5f, 1.f, WgCoord(0,0)),
														WgFlexAnchor(0.f, 1.f, WgCoord(0,0)),
														WgFlexAnchor(0.f, 0.5f, WgCoord(0,0)),
														WgFlexAnchor(0.5f, 0.5f, WgCoord(0,0)) };

//____ WgFlexHook::Constructor ________________________________________________

WgFlexHook::WgFlexHook( WgFlexPanel * pParent, const WgRect& placementGeo, WgBorders padding ) : m_pParent(pParent),
	m_bFloating(false), m_widthPolicy(WgSizePolicy::Equal), m_heightPolicy(WgSizePolicy::Equal),
	m_anchor(int(WgOrigo::NorthWest)), m_hotspot(WgOrigo::NorthWest),
	m_placementGeo(placementGeo), m_anchorTopLeft(int(WgOrigo::NorthWest)),
	m_anchorBottomRight(int(WgOrigo::SouthEast))
{
	m_padding = padding;
}

//____ WgFlexHook::Type() _____________________________________________________

const char * WgFlexHook::Type( void ) const
{
	return ClassType();
}

//____ WgFlexHook::ClassType() ________________________________________________

const char * WgFlexHook::ClassType()
{
	return c_hookType;
}

//____ WgFlexHook::SetAnchored() ______________________________________________

bool WgFlexHook::SetAnchored()
{
	// Return to old anchors.

	m_bFloating = false;
	_refreshRealGeo();
	return true;
}

bool  WgFlexHook::SetAnchored( int anchorTopLeft, int anchorBottomRight, WgBorders padding )
{
	int nbAnchors = m_pParent->NbAnchors();
	if( anchorTopLeft >= nbAnchors || anchorBottomRight >= nbAnchors )
		return false;

	m_bFloating			= false;
	m_anchorTopLeft		= anchorTopLeft;
	m_anchorBottomRight = anchorBottomRight;
	m_padding			= padding;

	_refreshRealGeo();
	return true;
}


//____ WgFlexHook::SetFloating() ______________________________________________

bool WgFlexHook::SetFloating()
{
	// Return to old floating position and size.

	m_bFloating = true;
	_refreshRealGeo();
	return true;
}

bool WgFlexHook::SetFloating( const WgCoord& pos, WgOrigo origo )
{
	return SetFloating( pos, int(origo), origo );
}

bool WgFlexHook::SetFloating( const WgCoord& pos, int anchor, WgOrigo hotspot )
{
	if( anchor >= m_pParent->NbAnchors() )
		return false;

	m_widthPolicy	= WgSizePolicy::None;
	m_heightPolicy	= WgSizePolicy::None;
	m_bFloating		= true;
	m_anchor		= anchor;
	m_hotspot		= hotspot;
	m_placementGeo.setPos(pos);

	_refreshRealGeo();
	return true;
}

bool WgFlexHook::SetFloating( const WgRect& geometry, WgOrigo origo )
{
	return SetFloating( geometry, int(origo), origo );
}

bool WgFlexHook::SetFloating( const WgRect& geometry, int anchor, WgOrigo hotspot )
{
	if( anchor >= m_pParent->NbAnchors() )
		return false;

	m_widthPolicy	= WgSizePolicy::Equal;
	m_heightPolicy	= WgSizePolicy::Equal;
	m_bFloating		= true;
	m_anchor		= anchor;
	m_hotspot		= hotspot;
	m_placementGeo	= geometry;

	_refreshRealGeo();
	return true;
}

//____ WgFlexHook::SetPadding() ______________________________________________

bool WgFlexHook::SetPadding( WgBorders padding )
{
	if( padding != m_padding )
	{
		m_padding = padding;
		_refreshRealGeo();
	}
	return true;
}

//____ WgFlexHook::Top() ______________________________________________________

void WgFlexHook::Top()
{
	MoveOver( _chain()->last() );
}

//____ WgFlexHook::Bottom() ___________________________________________________

void WgFlexHook::Bottom()
{
	MoveUnder( _chain()->first() );
}

//____ WgFlexHook::Up() _______________________________________________________

bool WgFlexHook::Up()
{
	WgFlexHook * pNext = _next();

	if( pNext == 0 )
		return false;

	// Get area potentially exposed

	WgRect cover = WgRect::overlap( m_realGeo, pNext->m_realGeo );

	// Move up

	_moveUp();

	// Request render on potentially exposed area.

	if( pNext->m_bVisible && cover.w > 0 && cover.h > 0 )
	{
		m_pParent->_onRequestRender( cover, this );
	}

	return true;
}

//____ WgFlexHook::Down() _____________________________________________________

bool WgFlexHook::Down()
{
	WgFlexHook * pPrev = _prev();

	if( pPrev == 0 )
		return false;

	// Get area of sibling potentially exposed

	WgRect cover = WgRect::overlap( m_realGeo, pPrev->m_realGeo );

	// Move down

	_moveDown();

	// Request render on potentially exposed area.

	if( m_bVisible && cover.w > 0 && cover.h > 0 )
	{
		m_pParent->_onRequestRender( cover, pPrev );
	}

	return true;
}

//____ WgFlexHook::MoveOver() _________________________________________________

bool WgFlexHook::MoveOver( WgFlexHook * pSibling )
{
	if( !pSibling || pSibling->m_pParent != m_pParent || pSibling == this )
		return false;

	WgFlexHook * pFirst = _chain()->first();
	while( pFirst != this && pFirst != pSibling )
		pFirst = pFirst->_next();

	WgFlexHook * pLast = pFirst->_next();
	while( pLast != this && pLast != pSibling )
		pLast = pLast->_next();

	if( pFirst == this )			// Move us forward
	{
		WgFlexHook * p = pFirst->_next();

		_moveAfter( pSibling );

		// Request render on all areas covered by siblings we have skipped in front of.

		if( m_bVisible )
		{
			while( p != this )
			{
				WgRect cover = WgRect::overlap( m_realGeo, p->m_realGeo );

				if( p->m_bVisible && cover.w > 0 && cover.h > 0 )
					m_pParent->_onRequestRender( cover, this );
				p = p->_next();
			}
		}
	}
	else							// Move us backward
	{
		WgFlexHook * p = pLast->_prev();

        if(p == pSibling)
        {
            // We are already in place - nothing more to do.
            return true;
        }

		_moveAfter( pSibling );

		// Request render on our siblings for the area we previously have covered.

		if( m_bVisible )
		{
			while( p != this )
			{
				WgRect cover = WgRect::overlap( m_realGeo, p->m_realGeo );

				if( p->m_bVisible && cover.w > 0 && cover.h > 0 )
					m_pParent->_onRequestRender( cover, p );
				p = p->_prev();
			}
		}
	}

	return true;
}

//____ WgFlexHook::MoveUnder() ________________________________________________

bool WgFlexHook::MoveUnder( WgFlexHook * pSibling )
{
	if( !pSibling || pSibling->m_pParent != m_pParent || pSibling == this )
		return false;

	WgFlexHook * pFirst = _chain()->first();
	while( pFirst != this && pFirst != pSibling )
		pFirst = pFirst->_next();

	WgFlexHook * pLast = pFirst->_next();
	while( pLast != this && pLast != pSibling )
		pLast = pLast->_next();

	if( pFirst == this )			// Move us forward
	{
		WgFlexHook * p = _next();

        if(p == pSibling)
        {
            // We are already in place - nothing more to do.
            return true;
        }

		_moveBefore( pSibling );

		// Request render on all areas covered by siblings we have skipped in front of.

		if( m_bVisible )
		{
			while( p != this )
			{
				WgRect cover = WgRect::overlap( m_realGeo, p->m_realGeo );

				if( p->m_bVisible && cover.w > 0 && cover.h > 0 )
					m_pParent->_onRequestRender( cover, this );

				p = p->_next();
			}
		}
	}
	else							// Move us backward
	{
		WgFlexHook * p = _prev();

		_moveBefore( pSibling );

		// Request render on our siblings for the area we previously have covered.

		if( m_bVisible )
		{
			while( p != this )
			{
				WgRect cover = WgRect::overlap( m_realGeo, p->m_realGeo );

				if( p->m_bVisible && cover.w > 0 && cover.h > 0 )
					m_pParent->_onRequestRender( cover, p );

				p = p->_prev();
			}
		}
	}

	return true;
}

//____ WgFlexHook::SetAnchor() _________________________________________________

bool WgFlexHook::SetAnchor( int anchor )
{
	if( anchor >= m_pParent->NbAnchors() )
		return false;

	if( anchor != m_anchor )
	{
		m_anchor = anchor;
		_refreshRealGeo();
	}

	return true;
}

//____ WgFlexHook::SetHotspot() _______________________________________________

bool WgFlexHook::SetHotspot( WgOrigo hotspot )
{
	if( hotspot != m_hotspot )
	{
		m_hotspot = hotspot;
		_refreshRealGeo();
	}

	return true;
}

//____ WgFlexHook::SetSizePolicy() ____________________________________________

bool WgFlexHook::SetSizePolicy( WgSizePolicy width, WgSizePolicy height )
{
	if( width != m_widthPolicy || height != m_heightPolicy )
	{
		m_widthPolicy = width;
		m_heightPolicy = height;
		if( m_bFloating )
			_refreshRealGeo();
	}
	return true;
}

//____ WgFlexHook::SetPointGeo() ___________________________________________________

bool WgFlexHook::SetPointGeo( const WgRect& geometry )
{
	m_placementGeo = geometry;
	if( m_bFloating )
		_refreshRealGeo();
	return true;
}

//____ WgFlexHook::SetPointOfs() ___________________________________________________

bool WgFlexHook::SetPointOfs( const WgCoord& ofs )
{
	m_placementGeo.x = ofs.x;
	m_placementGeo.y = ofs.y;
	if( m_bFloating )
		_refreshRealGeo();
	return true;
}

//____ WgFlexHook::SetPointOfsX() __________________________________________________

bool WgFlexHook::SetPointOfsX( int x )
{
	m_placementGeo.x = x;
	if( m_bFloating )
		_refreshRealGeo();
	return true;
}

//____ WgFlexHook::SetPointOfsY() __________________________________________________

bool WgFlexHook::SetPointOfsY( int y )
{
	m_placementGeo.y = y;
	if( m_bFloating )
		_refreshRealGeo();
	return true;
}

//____ WgFlexHook::SetPointSize() __________________________________________________

bool WgFlexHook::SetPointSize( const WgSize& size )
{
	m_placementGeo.w = size.w;
	m_placementGeo.h = size.h;
	if( m_bFloating )
		_refreshRealGeo();
	return true;
}

//____ WgFlexHook::SetPointWidth() _________________________________________________

bool WgFlexHook::SetPointWidth( int width )
{
	m_placementGeo.w = width;
	if( m_bFloating )
		_refreshRealGeo();
	return true;
}

//____ WgFlexHook::SetPointHeight() ________________________________________________

bool WgFlexHook::SetPointHeight( int height )
{
	m_placementGeo.h = height;
	if( m_bFloating )
		_refreshRealGeo();
	return true;
}

//____ WgFlexHook::MovePoints() _____________________________________________________

bool WgFlexHook::MovePoints( const WgCoord& ofs )
{
	if( !m_bFloating )
		return false;

//	m_placementGeo = m_realGeo;
//	m_placementGeo -= m_pParent->Anchor(m_anchor)->Position(m_pParent->Size());
	m_placementGeo += ofs;

	_refreshRealGeo();
	return true;
}

//____ WgFlexHook::MovePointsX() ____________________________________________________

bool WgFlexHook::MovePointsX( int x )
{
	if( !m_bFloating )
		return false;

//	m_placementGeo = m_realGeo;
//	m_placementGeo -= m_pParent->Anchor(m_anchor)->Position(m_pParent->Size());
	m_placementGeo.x += x;

	_refreshRealGeo();
	return true;
}

//____ WgFlexHook::MovePointsY() ____________________________________________________

bool WgFlexHook::MovePointsY( int y )
{
	if( !m_bFloating )
		return false;

//	m_placementGeo = m_realGeo;
//	m_placementGeo -= m_pParent->Anchor(m_anchor)->Position(m_pParent->Size());
	m_placementGeo.y += y;

	_refreshRealGeo();
	return true;
}

//____ WgFlexHook::ScreenPixelPos() ________________________________________________

WgCoord WgFlexHook::ScreenPixelPos() const
{
	return m_realGeo.pos() + m_pParent->ScreenPixelPos();
}

//____ WgFlexHook::ScreenPixelGeo() ________________________________________________

WgRect WgFlexHook::ScreenPixelGeo() const
{
	return m_realGeo + m_pParent->ScreenPixelPos();
}

//____ WgFlexHook::_prevHook() ________________________________________________

WgHook * WgFlexHook::_prevHook() const
{
	return _prev();
}

//____ WgFlexHook::_nextHook() ________________________________________________

WgHook * WgFlexHook::_nextHook() const
{
	return _next();
}

//____ WgFlexHook::_parent() __________________________________________________

WgContainer * WgFlexHook::_parent() const
{
	return m_pParent;
}

//____ WgFlexHook::_refreshRealGeo() ___________________________________________

void WgFlexHook::_refreshRealGeo()
{

	WgRect	newGeo;
	WgSize	parentSize = m_pParent->PixelSize();

	int scale = m_pParent->m_scale;

	WgBorders scaledPadding( (m_padding.top * scale) >> WG_SCALE_BINALS,
							 (m_padding.right * scale) >> WG_SCALE_BINALS,
							 (m_padding.bottom * scale) >> WG_SCALE_BINALS,
							 (m_padding.left * scale) >> WG_SCALE_BINALS );

	if( m_bFloating )
	{
		// Scale placementGeo

		WgRect scaledGeo( (m_placementGeo.x * scale) >> WG_SCALE_BINALS,
						  (m_placementGeo.y * scale) >> WG_SCALE_BINALS,
						  (m_placementGeo.w * scale) >> WG_SCALE_BINALS,
						  (m_placementGeo.h * scale) >> WG_SCALE_BINALS );

		// Calculate size

		WgSize sz = _sizeFromPolicy( scaledGeo.size() + scaledPadding, m_widthPolicy, m_heightPolicy, scale );

		// Calculate position

		WgCoord pos = m_pParent->Anchor(m_anchor)->Position( parentSize, scale );	// Anchor,
		pos -= WgUtil::OrigoToOfs( m_hotspot, sz );						// hotspot
		pos += scaledGeo.pos();											// and Offset.

		// Limit size/pos according to parent

		if( m_pParent->IsConfiningChildren() )
		{
			if( sz.w > parentSize.w )
				sz.w = parentSize.w;
			if( sz.h > parentSize.h )

			if( pos.x + sz.w > parentSize.w )
				pos.x = parentSize.w - sz.w;

			if( pos.y + sz.h > parentSize.h )
				pos.y = parentSize.h - sz.h;
		}

		newGeo = WgRect( pos, sz );
	}
	else
	{
		WgCoord topLeft = m_pParent->Anchor(m_anchorTopLeft)->Position( parentSize, scale );
		WgCoord bottomRight = m_pParent->Anchor(m_anchorBottomRight)->Position( parentSize, scale );

		newGeo = WgRect(topLeft,bottomRight);
	}
	newGeo.shrink( scaledPadding );


	// Request render and update positions.

	if( newGeo != m_realGeo )
	{
		_requestRender();
		m_realGeo = newGeo;
		m_pWidget->_onNewSize(newGeo);
		_requestRender();
	}
}

//____ WgFlexHook::_sizeNeededForGeo() ________________________________________
/*
WgSize WgFlexHook::_sizeNeededForGeo()
{
	WgSize sz;

	if( m_bFloating )
	{
		WgRect geo = m_placementGeo + m_padding;

		const WgFlexAnchor * pa = m_pParent->Anchor(m_anchor);

		WgCoord hotspot = WgUtil::OrigoToOfs(m_hotspot,geo.Size());
		WgCoord offset = pa->Offset() + geo.Pos() - hotspot;

		int leftOfAnchor = 0 - offset.x;
		int rightOfAnchor = offset.x + geo.w;
		int aboveAnchor = 0 - offset.y;
		int belowAnchor = offset.y + geo.h;

		if( leftOfAnchor > 0 )
			sz.w = (int) (leftOfAnchor / pa->RelativeX());

		if( rightOfAnchor > 0 )
		{
			int w = (int) (rightOfAnchor / (1.f - pa->RelativeX()) );
			if( sz.w < w )
				sz.w = w;
		}

		if( aboveAnchor > 0 )
			sz.h = (int) (aboveAnchor / pa->RelativeY());

		if( belowAnchor > 0 )
		{
			int h = (int) (belowAnchor / (1.f - pa->RelativeY()) );
			if( sz.h < h )
				sz.h = h;
		}
	}
	else
	{
		sz = m_pWidget->PreferredPixelSize() + m_padding;

		const WgFlexAnchor * pa1 = m_pParent->Anchor(m_anchorTopLeft);
		const WgFlexAnchor * pa2 = m_pParent->Anchor(m_anchorBottomRight);

		sz += WgSize(pa1->OffsetX(),pa1->OffsetY());
		sz -= WgSize(pa2->OffsetX(),pa2->OffsetY());

		sz.w = (int) (sz.w / (float) (pa2->RelativeX() - pa1->RelativeX()));
		sz.h = (int) (sz.w / (float) (pa2->RelativeY() - pa1->RelativeY()));
	}

	return sz;
}
*/

WgSize WgFlexHook::_sizeNeededForGeo()
{
	WgSize sz;

	int scale = m_pParent->m_scale;

	WgBorders scaledPadding((m_padding.top * scale) >> WG_SCALE_BINALS,
							(m_padding.right * scale) >> WG_SCALE_BINALS,
							(m_padding.bottom * scale) >> WG_SCALE_BINALS,
							(m_padding.left * scale) >> WG_SCALE_BINALS );

	if( m_bFloating )
	{
		WgRect scaledGeo( (m_placementGeo.x * scale) >> WG_SCALE_BINALS,
						 (m_placementGeo.y * scale) >> WG_SCALE_BINALS,
						 (m_placementGeo.w * scale) >> WG_SCALE_BINALS,
						 (m_placementGeo.h * scale) >> WG_SCALE_BINALS );

		WgRect geo = scaledGeo + scaledPadding;

		const WgFlexAnchor * pa = m_pParent->Anchor(m_anchor);

		WgCoord hotspot = WgUtil::OrigoToOfs(m_hotspot,geo.size());
		WgCoord offset = (pa->Offset()*scale)/WG_SCALE_BASE + geo.pos() - hotspot;

		int leftOfAnchor = 0 - offset.x;
		int rightOfAnchor = offset.x + geo.w;
		int aboveAnchor = 0 - offset.y;
		int belowAnchor = offset.y + geo.h;

		if( leftOfAnchor > 0 )
        {
            if( pa->RelativeX() == 0.f )
                sz.w = 0;
            else
                sz.w = (int) (leftOfAnchor / pa->RelativeX());
        }

		if( rightOfAnchor > 0 )
		{
            int w;
            if( pa->RelativeX() == 1.f )
                w = 0;
            else
                w = (int) (rightOfAnchor / (1.f - pa->RelativeX()) );
			if( sz.w < w )
				sz.w = w;
		}

		if( aboveAnchor > 0 && pa->RelativeY() > 0.f )
        {
            if( pa->RelativeY() == 0.f )
                sz.h = 0;
            else
                sz.h = (int) (aboveAnchor / pa->RelativeY());
        }
            
		if( belowAnchor > 0 && pa->RelativeY() < 1.f )
		{
            int h;
            if( pa->RelativeY() == 1.f )
                h = 0;
            else
                h = (int) (belowAnchor / (1.f - pa->RelativeY()) );
			if( sz.h < h )
				sz.h = h;
		}
	}
	else
	{
		sz = m_pWidget->PreferredPixelSize() + scaledPadding;

		const WgFlexAnchor * pa1 = m_pParent->Anchor(m_anchorTopLeft);
		const WgFlexAnchor * pa2 = m_pParent->Anchor(m_anchorBottomRight);

		sz += WgSize((pa1->OffsetX()*scale)>>WG_SCALE_BINALS,(pa1->OffsetY()*scale)>>WG_SCALE_BINALS);
		sz -= WgSize((pa2->OffsetX()*scale)>>WG_SCALE_BINALS,(pa2->OffsetY()*scale)>>WG_SCALE_BINALS);

		float wc = (float) (pa2->RelativeX() - pa1->RelativeX());
		float wh = (float) (pa2->RelativeY() - pa1->RelativeY());

		// Init to zero
		sz.w = 0;
		sz.h = 0;

		if (std::abs(wc) > 0.0f)
			sz.w = (int) (sz.w / wc);

		if(std::abs(wh) > 0.0f)
			sz.h = (int) (sz.w / wh);
	}

	return sz;
}


//____ WgFlexHook::_requestRender() ____________________________________________

void WgFlexHook::_requestRender()
{
	m_pParent->_onRequestRender( m_realGeo, this );
}

//____ WgFlexHook::_requestRender() ____________________________________________

void WgFlexHook::_requestRender( const WgRect& rect )
{
	m_pParent->_onRequestRender( rect + m_realGeo.pos(), this );
}

//____ WgFlexHook::_requestResize() ____________________________________________

void WgFlexHook::_requestResize()
{
	if( m_widthPolicy != WgSizePolicy::Equal || m_heightPolicy != WgSizePolicy::Equal )
		_refreshRealGeo();
}


//____ Constructor ____________________________________________________________

WgFlexPanel::WgFlexPanel() : m_bConfineChildren(false)
{
	m_bSiblingsOverlap = true;
}

//____ Destructor _____________________________________________________________

WgFlexPanel::~WgFlexPanel()
{
// This is done automatically now, calls like these in destructor are forbidden.

//	DeleteAllWidgets();
//	DeleteAllAnchors();
}

//____ Type() _________________________________________________________________

const char * WgFlexPanel::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgFlexPanel::GetClass()
{
	return c_widgetType;
}

//____ SetConfineChildren() ___________________________________________________

void WgFlexPanel::SetConfineChildren( bool bConfineChildren )
{
	if( bConfineChildren != m_bConfineChildren )
	{
		m_bConfineChildren = bConfineChildren;

		WgFlexHook * pHook = m_hooks.first();
		while( pHook )
		{
			pHook->_refreshRealGeo();
			pHook = pHook->Next();
		}
	}
}



//____ AddChild() _____________________________________________________________

WgFlexHook * WgFlexPanel::AddChild( WgWidget * pWidget )
{
	if( !pWidget )
		return 0;

	WgFlexHook * p = new WgFlexHook( this, WgRect(0,0,_scaledPreferredPixelSize(pWidget)), WgBorders(0) );
	p->_attachWidget( pWidget );

	m_hooks.pushBack(p);

	p->SetFloating();
	return p;
}

//____ AddChild() _____________________________________________________________

WgFlexHook * WgFlexPanel::AddChild( WgWidget * pWidget, int anchorTopLeft, int anchorBottomRight, WgBorders padding )
{
	if( !pWidget )
		return 0;

	WgFlexHook * p = new WgFlexHook( this, WgRect(0,0,_scaledPreferredPixelSize(pWidget)), padding );
	p->_attachWidget( pWidget );

	m_hooks.pushBack(p);

	p->SetAnchored( anchorTopLeft, anchorBottomRight, padding );
	return p;
}

WgFlexHook * WgFlexPanel::AddChild( WgWidget * pWidget, const WgCoord& pos, WgOrigo origo, WgBorders padding )
{
	return AddChild( pWidget, pos, int(origo), origo, padding );
}

WgFlexHook * WgFlexPanel::AddChild( WgWidget * pWidget, const WgCoord& pos, int anchor, WgOrigo hotspot, WgBorders padding )
{
	if( !pWidget )
		return 0;

	WgSize bestSize = _scaledPreferredPixelSize(pWidget);

	WgFlexHook * p = new WgFlexHook( this, WgRect(0,0,bestSize), padding );
	p->_attachWidget( pWidget );
	m_hooks.pushBack(p);
	p->SetFloating( pos, anchor, hotspot );
	return p;
}

WgFlexHook * WgFlexPanel::AddChild( WgWidget * pWidget, const WgRect& geometry, WgOrigo origo, WgBorders padding )
{
	return AddChild( pWidget, geometry, int(origo), origo, padding );
}

WgFlexHook * WgFlexPanel::AddChild( WgWidget * pWidget, const WgRect& geometry, int anchor, WgOrigo hotspot, WgBorders padding )
{
	if( !pWidget )
		return 0;

	WgFlexHook * p = new WgFlexHook( this, WgRect(0,0,_scaledPreferredPixelSize(pWidget)), padding );
	p->_attachWidget( pWidget );

	m_hooks.pushBack(p);

	p->SetFloating( geometry, anchor, hotspot );
	return p;
}


//____ InsertChild() _________________________________________________

WgFlexHook * WgFlexPanel::InsertChild( WgWidget * pWidget, WgWidget * pSibling )
{
	if( !pWidget || !pSibling || !pSibling->Hook() || pSibling->Hook()->Parent() != this )
		return 0;

	WgFlexHook * p = new WgFlexHook( this, WgRect(0,0,_scaledPreferredPixelSize(pWidget)), WgBorders(0) );
	p->_attachWidget( pWidget );
	p->_moveBefore( (WgFlexHook*)pSibling->Hook() );
	p->SetFloating();
	return p;
}


WgFlexHook * WgFlexPanel::InsertChild( WgWidget * pWidget, WgWidget * pSibling, int anchorTopLeft, int anchorBottomRight, WgBorders padding )
{
	if( !pWidget || !pSibling || !pSibling->Hook() || pSibling->Hook()->Parent() != this )
		return 0;

	WgFlexHook * p = new WgFlexHook( this, WgRect(0,0,_scaledPreferredPixelSize(pWidget)), WgBorders(0) );
	p->_attachWidget( pWidget );
	p->_moveBefore( (WgFlexHook*)pSibling->Hook() );
	p->SetAnchored( anchorTopLeft, anchorBottomRight, padding );
	return p;
}


WgFlexHook * WgFlexPanel::InsertChild( WgWidget * pWidget, WgWidget * pSibling, const WgRect& geometry, WgOrigo origo, WgBorders padding )
{
	return InsertChild( pWidget, pSibling, geometry, int(origo), origo, padding );
}

WgFlexHook * WgFlexPanel::InsertChild( WgWidget * pWidget, WgWidget * pSibling, const WgRect& geometry, int anchor, WgOrigo hotspot, WgBorders padding )
{
	if( !pWidget || !pSibling || !pSibling->Hook() || pSibling->Hook()->Parent() != this )
		return 0;

	WgFlexHook * p = new WgFlexHook( this, WgRect(0,0,_scaledPreferredPixelSize(pWidget)), padding );
	p->_attachWidget( pWidget );
	p->_moveBefore( (WgFlexHook*)pSibling->Hook() );
	p->SetFloating( geometry, anchor, hotspot );
	return p;
}

WgFlexHook * WgFlexPanel::InsertChild( WgWidget * pWidget, WgWidget * pSibling, const WgCoord& pos, WgOrigo origo, WgBorders padding )
{
	return InsertChild( pWidget, pSibling, pos, int(origo), origo, padding );
}

WgFlexHook * WgFlexPanel::InsertChild( WgWidget * pWidget, WgWidget * pSibling, const WgCoord& pos, int anchor, WgOrigo hotspot, WgBorders padding )
{
	if( !pWidget || !pSibling || !pSibling->Hook() || pSibling->Hook()->Parent() != this )
		return 0;

	WgFlexHook * p = new WgFlexHook( this, WgRect(0,0,_scaledPreferredPixelSize(pWidget)), padding );
	p->_attachWidget( pWidget );
	p->_moveBefore( (WgFlexHook*)pSibling->Hook() );
	p->SetFloating( pos, anchor, hotspot );
	return p;
}




//____ DeleteChild() _________________________________________________

bool WgFlexPanel::DeleteChild( WgWidget * pWidget )
{
	if( !pWidget || !pWidget->Hook() || pWidget->Hook()->Parent() != this )
		return false;

	// Force rendering of the area the widget was covering

	WgFlexHook * pHook = (WgFlexHook *) pWidget->Hook();
	_onRequestRender( pHook->m_realGeo, pHook );

	// Delete the widget and return

	delete pHook;
	return true;
}

//____ ReleaseChild() _________________________________________________________

WgWidget * WgFlexPanel::ReleaseChild( WgWidget * pWidget )
{
	if( !pWidget || !pWidget->Hook() || pWidget->Hook()->Parent() != this )
		return 0;

	// Force rendering of the area the widget was covering

	WgFlexHook * pHook = (WgFlexHook *) pWidget->Hook();
	_onRequestRender( pHook->m_realGeo, pHook );

	// Delete the widget and return

	pHook->_releaseWidget();
	delete pHook;
	return pWidget;
}

//____ DeleteAllChildren() ______________________________________________________

bool WgFlexPanel::DeleteAllChildren()
{
	WgPatches	dirt;

	// Collect dirty areas and delete hooks, taking any connected widgets with them.

	WgFlexHook * pHook = m_hooks.first();
	while( pHook )
	{
		if( pHook->m_bVisible )
			dirt.add( pHook->m_realGeo );
		WgFlexHook * pDelete = pHook;
		pHook = pHook->Next();
		delete pDelete;
	}

	// RequestRender on all dirty patches

	for( const WgRect * pRect = dirt.begin() ; pRect != dirt.end() ; pRect++ )
		_requestRender( * pRect );

	return true;
}

//____ ReleaseAllChildren() ______________________________________________________

bool WgFlexPanel::ReleaseAllChildren()
{
	WgFlexHook * pHook = m_hooks.first();
	while( pHook )
	{
		pHook->_releaseWidget();
		pHook = pHook->Next();
	}

	DeleteAllChildren();	// Will only delete the hooks and request render on dirty areas since
							// we already have disconnected the children.
	return true;
}

//____ AddAnchor() ____________________________________________________________

int WgFlexPanel::AddAnchor( float relativeX, float relativeY, const WgCoord& pixelOfs )
{
	m_anchors.push_back( WgFlexAnchor( relativeX, relativeY, pixelOfs ) );
	return int(m_anchors.size())+wg::Placement_size-1;
}

//____ ReplaceAnchor() ________________________________________________________

bool WgFlexPanel::ReplaceAnchor( int index, float relativeX, float relativeY, const WgCoord& pixelOfs )
{
	if( index < wg::Placement_size || index >= NbAnchors() )
		return false;

	m_anchors[index-wg::Placement_size] = WgFlexAnchor( relativeX, relativeY, pixelOfs );

	// Update geometry for all widgets using this anchor.

	WgFlexHook * pHook = m_hooks.first();
	while( pHook )
	{
		if( (pHook->m_bFloating && pHook->m_anchor == index) ||
			(!pHook->m_bFloating && (pHook->m_anchorBottomRight == index || pHook->m_anchorTopLeft == index)) )
			pHook->_refreshRealGeo();

		pHook = pHook->Next();
	}

	return true;
}

//____ DeleteAnchor() _________________________________________________________

bool WgFlexPanel::DeleteAnchor( int index )
{
	if( index < wg::Placement_size || index >= NbAnchors() )
		return false;


	m_anchors.erase( m_anchors.begin()+(index-wg::Placement_size) );

	// Update hooks with affected anchors. Save list of affected hooks for later
	// update of geometry since we need to update all anchors first.

	std::vector<WgFlexHook*>	vNeedsUpdate;
	WgFlexHook * pHook = m_hooks.first();
	while( pHook )
	{
		// Check if this widget will have its geometry altered.

		if( (pHook->m_bFloating && pHook->m_anchor == index) ||
			(!pHook->m_bFloating && (pHook->m_anchorTopLeft == index || pHook->m_anchorBottomRight == index)) )
			vNeedsUpdate.push_back(pHook);

		// Update hooks anchor references.

		if( pHook->m_anchor == index )
			pHook->m_anchor = int(WgOrigo::NorthWest);
		else if( pHook->m_anchor > index )
			pHook->m_anchor--;

		if( pHook->m_anchorTopLeft == index )
			pHook->m_anchorTopLeft = int(WgOrigo::NorthWest);
		else if( pHook->m_anchorTopLeft > index )
			pHook->m_anchorTopLeft--;

		if( pHook->m_anchorBottomRight == index )
			pHook->m_anchorBottomRight = int(WgOrigo::SouthEast);
		else if( pHook->m_anchorBottomRight > index )
			pHook->m_anchorBottomRight--;

		pHook = pHook->Next();
	}

	// Update geometry for all affected hooks.

	for( unsigned int i = 0 ; i < vNeedsUpdate.size() ; i++ )
		vNeedsUpdate[i]->_refreshRealGeo();

	return true;
}

//____ DeleteAllAnchors() _____________________________________________________

void WgFlexPanel::DeleteAllAnchors()
{
	if( m_anchors.empty() )
		return;


	m_anchors.clear();

	// Update hooks with affected anchors. Save list of affected hooks for later
	// update of geometry since we need to update all anchors first.

	std::vector<WgFlexHook*>	vNeedsUpdate;
	WgFlexHook * pHook = m_hooks.first();
	while( pHook )
	{
		// Check if this widget will have its geometry altered.

		if( (pHook->m_bFloating && pHook->m_anchor >= wg::Placement_size) ||
			(!pHook->m_bFloating && (pHook->m_anchorTopLeft >= wg::Placement_size || pHook->m_anchorBottomRight >= wg::Placement_size)) )
			vNeedsUpdate.push_back(pHook);

		// Update hooks anchor references.

		if( pHook->m_anchor >= wg::Placement_size )
			pHook->m_anchor = int(WgOrigo::NorthWest);

		if( pHook->m_anchorTopLeft >= wg::Placement_size )
			pHook->m_anchorTopLeft = int(WgOrigo::NorthWest);

		if( pHook->m_anchorBottomRight >= wg::Placement_size )
			pHook->m_anchorBottomRight = int(WgOrigo::SouthEast);

		pHook = pHook->Next();
	}

	// Update geometry for all affected hooks.

	for( unsigned int i = 0 ; i < vNeedsUpdate.size() ; i++ )
		vNeedsUpdate[i]->_refreshRealGeo();
}




//____ Anchor() _______________________________________________________________

const WgFlexAnchor * WgFlexPanel::Anchor( int index )
{
	if( index < wg::Placement_size )
		return &g_baseAnchors[index];
	else
	{
		index -= wg::Placement_size;
		if( index < (int) m_anchors.size() )
			return &m_anchors[index];
	}
	return 0;
}

//____ SetPreferredPointSize() ________________________________________________________

void WgFlexPanel::SetPreferredPointSize( WgSize size )
{
	if( size != m_preferredSize )
	{
		m_preferredSize = size;
		_requestResize();
	}
}


//____ PreferredPixelSize() _____________________________________________________________

WgSize WgFlexPanel::PreferredPixelSize() const
{
	if( m_preferredSize.w > 0 && m_preferredSize.h > 0 )
	{
		return WgSize( m_preferredSize.w * m_scale >> WG_SCALE_BINALS, m_preferredSize.h * m_scale >> WG_SCALE_BINALS );
	}
	else
	{
		WgSize minSize;

		WgFlexHook * pHook = m_hooks.first();
		while( pHook )
		{
			WgSize sz = pHook->_sizeNeededForGeo();
			minSize = WgSize::max(minSize,sz);
			pHook = pHook->Next();
		}
		return minSize;
	}
}

//____ _onEvent() ______________________________________________________

void WgFlexPanel::_onEvent( const WgEvent::Event * _pEvent, WgEventHandler * pHandler )
{
	WgState state = m_state;

	switch( _pEvent->Type() )
	{
		case WG_EVENT_MOUSE_ENTER:
			state.setHovered(true);
			break;

		case WG_EVENT_MOUSE_LEAVE:
			state.setHovered(false);
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			int button = static_cast<const WgEvent::MouseButtonPress*>(_pEvent)->Button();
			if( button == 1 )
				state.setPressed(true);
			break;
		}

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			int button = static_cast<const WgEvent::MouseButtonRelease*>(_pEvent)->Button();
			if( button == 1 )
				state.setPressed(false);
			break;
		}

		default:
			break;
	}

	if( state != m_state )
	{
        if( m_pSkin )
        {
            wg::RectI dirt = _skinDirtyRect( m_pSkin, PixelSize(), m_scale, state, m_state );
            if( !dirt.isEmpty() )
                _requestRender(dirt);
        }
        
		m_state = state;
	}

	WgPanel::_onEvent( _pEvent, pHandler );
}

//____ _onRequestRender() ______________________________________________________

void WgFlexPanel::_onRequestRender( const WgRect& rect, const WgFlexHook * pHook )
{
	if( !pHook->m_bVisible )
		return;

	// Clip our geometry and put it in a dirtyrect-list

	WgPatches patches;
	patches.add( WgRect::overlap( rect, WgRect(0,0,PixelSize())) );

	// Remove portions of patches that are covered by opaque upper siblings

	WgFlexHook * pCover = pHook->Next();
	while( pCover )
	{
		if( pCover->m_bVisible && pCover->m_realGeo.isOverlapping( rect ) )
			pCover->Widget()->_onMaskPatches( patches, pCover->m_realGeo, WgRect(0,0,65536,65536 ), _getBlendMode() );

		pCover = pCover->Next();
	}

	// Make request render calls

	for( const WgRect * pRect = patches.begin() ; pRect < patches.end() ; pRect++ )
		_requestRender( * pRect );
}

//____ _onCloneContent() _______________________________________________________

void WgFlexPanel::_onCloneContent( const WgWidget * _pOrg )
{
	//TODO: Implement
}

//____ _onNewSize() ____________________________________________________________

void WgFlexPanel::_onNewSize( const WgSize& size )
{
	WgFlexHook * pHook = m_hooks.last();

	while( pHook )
	{
		pHook->_refreshRealGeo();
		pHook = pHook->Prev();
	}
}


//____ _setScale() ____________________________________________________________

void WgFlexPanel::_setScale( int scale )
{
	WgPanel::_setScale( scale );

	WgFlexHook * pHook = m_hooks.last();

	while( pHook )
	{
		pHook->_refreshRealGeo();
		pHook = pHook->Prev();
	}

		_requestResize();
}

//____ _scaledPreferredPixelSize() __________________________________________________

WgSize WgFlexPanel::_scaledPreferredPixelSize( WgWidget * pWidget )
{
	WgSize sz = pWidget->PreferredPointSize();

//    sz.w = (sz.w << WG_SCALE_BINALS) / m_scale;
//    sz.h = (sz.h << WG_SCALE_BINALS) / m_scale;

	return sz;
}



//____ _firstHookWithGeo() _____________________________________________________

WgHook * WgFlexPanel::_firstHookWithGeo( WgRect& writeGeo ) const
{
	WgFlexHook * p = m_hooks.first();
	if( p )
		writeGeo = p->m_realGeo;

	return p;
}

//____ _nextHookWithGeo() ______________________________________________________

WgHook * WgFlexPanel::_nextHookWithGeo( WgRect& writeGeo, WgHook * pHook ) const
{
	WgFlexHook * p = ((WgFlexHook*)pHook)->_next();
	if( p )
		writeGeo = p->m_realGeo;

	return p;
}

//____ _lastHookWithGeo() _____________________________________________________

WgHook * WgFlexPanel::_lastHookWithGeo( WgRect& writeGeo ) const
{
	WgFlexHook * p = m_hooks.last();
	if( p )
		writeGeo = p->m_realGeo;

	return p;
}

//____ _prevHookWithGeo() ______________________________________________________

WgHook * WgFlexPanel::_prevHookWithGeo( WgRect& writeGeo, WgHook * pHook ) const
{
	WgFlexHook * p = ((WgFlexHook*)pHook)->_prev();
	if( p )
		writeGeo = p->m_realGeo;

	return p;
}

