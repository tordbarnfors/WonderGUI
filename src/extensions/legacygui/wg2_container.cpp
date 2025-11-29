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
#include <vector>

#include <wg2_container.h>
#include <wg2_panel.h>
#include <wg2_patches.h>
#include <wg2_base.h>

#ifndef WG2_GFXDEVICE_DOT_H
#	include <wg2_gfxdevice.h>
#endif

//____ Constructor _____________________________________________________________

WgContainer::WgContainer()
: m_bSiblingsOverlap(true),
  m_bEavesdropping(false)
{
}



//____ IsWidget() ______________________________________________________________

bool WgContainer::IsWidget() const
{
	return true;
}

//____ IsContainer() ______________________________________________________________

bool WgContainer::IsContainer() const
{
	return true;
}

//____ IsPanel() ______________________________________________________________

bool WgContainer::IsPanel() const
{
	return false;
}

//____ IsCapsule() ______________________________________________________________

bool WgContainer::IsCapsule() const
{
	return false;
}

//____ IsLayer() ______________________________________________________________

bool WgContainer::IsLayer() const
{
	return false;
}


//____ CastToWidget() _______________________________________________________

WgWidget * WgContainer::CastToWidget()
{
	return this;
}

const WgWidget * WgContainer::CastToWidget() const
{
	return this;
}


//____ CastToContainer() _______________________________________________________

WgContainer * WgContainer::CastToContainer()
{
	return this;
}

const WgContainer * WgContainer::CastToContainer() const
{
	return this;
}

//____ CastToPanel() _______________________________________________________

WgPanel * WgContainer::CastToPanel()
{
	return 0;
}

const WgPanel * WgContainer::CastToPanel() const
{
	return 0;
}

//____ CastToCapsule() _______________________________________________________

WgCapsule * WgContainer::CastToCapsule()
{
	return 0;
}

const WgCapsule * WgContainer::CastToCapsule() const
{
	return 0;
}

//____ CastToLayer() _______________________________________________________

WgLayer * WgContainer::CastToLayer()
{
	return 0;
}

const WgLayer * WgContainer::CastToLayer() const
{
	return 0;
}

//____ FindWidget() ____________________________________________________________

WgWidget * WgContainer::FindWidget( const WgCoord& ofs, WgSearchMode mode )
{
	WgRect childGeo;
	WgHook * pHook = _lastHookWithGeo( childGeo );
	WgWidget * pResult = 0;

	while( pHook && !pResult )
	{
		bool bVisibleHook = IsPanel()?static_cast<WgPanelHook*>(pHook)->IsVisible():true;

		if( bVisibleHook && childGeo.contains( ofs ) )
		{
			if( pHook->Widget()->IsContainer() )
			{
				pResult = pHook->Widget()->CastToContainer()->FindWidget( ofs - childGeo.pos(), mode );
			}
			else if( mode == WgSearchMode::Geometry || pHook->Widget()->MarkTest( ofs - childGeo.pos() ) )
			{
				pResult = pHook->Widget();
			}
		}
		pHook = _prevHookWithGeo( childGeo, pHook );
	}

	// Return us if search mode is GEOMETRY

	if( !pResult && (mode == WgSearchMode::Geometry || MarkTest(ofs)) )
		pResult = this;

	return pResult;
}


//____ SetScale() ______________________________________________________________

void WgContainer::SetScale( int scale )
{
	if( m_scale != scale )
		_setScale(scale);
}

//____ SetEavesdropping() ______________________________________________________________

void WgContainer::SetEavesdropping( bool bEavesdropping )
{
	m_bEavesdropping = bEavesdropping;
}


//____ _descendantPos() _____________________________________________________

bool WgContainer::_descendantPos(WgWidget * pDescendant, WgCoord& pos)
{
	pos.clear();

	WgWidget * p = pDescendant;
	while (p != this && p!= nullptr )
	{
		auto pHook = p->Hook();
		if( pHook == nullptr )
			return false;

		auto pPanelHook = dynamic_cast<WgPanelHook*>(pHook);
        if(pPanelHook && !pPanelHook->IsVisible())
        {
            return false;
        }

		pos += pHook->PixelPos();
		p = pHook->Parent();
	}

	return true;
}


//____ _onNewRoot() ___________________________________________________________

void WgContainer::_onNewRoot( WgRoot * pRoot )
{
	for( WgHook * p = FirstHook() ; p != 0 ; p = p->Next() )
		p->Widget()->_onNewRoot( pRoot );

	WgWidget::_onNewRoot(pRoot);
}

//____ _focusRequested() _______________________________________________________

bool WgContainer::_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting )
{
	WgHook * p = Hook();
	if( p )
		return p->Holder()->_focusRequested( p, pWidgetRequesting );
	else
		return false;
}

//____ _focusReleased() ________________________________________________________

bool WgContainer::_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing )
{
	WgHook * p = Hook();
	if( p )
		return p->Holder()->_focusReleased( p, pWidgetReleasing );
	else
		return false;
}

//____ _inViewRequested() ______________________________________________________

void WgContainer::_inViewRequested( WgHook * pChild )
{
	WgHook * p = Hook();
	if( p )
	{
		WgRect geo = pChild->PixelGeo();
		p->Holder()->_inViewRequested( p, geo, geo );
	}
}

void WgContainer::_inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea )
{
	WgHook * p = Hook();
	if( p )
	{
		WgCoord ofs = pChild->PixelPos();
		p->Holder()->_inViewRequested( p, mustHaveArea+ofs, niceToHaveArea+ofs );
	}
}



//____ _getModalLayer() _______________________________________________________

WgModalLayer *  WgContainer::_getModalLayer() const
{
	const WgContainer * p = Parent();

	if( p )
		return p->_getModalLayer();
	else
		return 0;
}

//____ _getPopupLayer() _______________________________________________________

WgPopupLayer *  WgContainer::_getPopupLayer() const
{
	const WgContainer * p = Parent();

	if (p)
		return p->_getPopupLayer();
	else
		return 0;
}



//____ _onEnable() _____________________________________________________________

void WgContainer::_onEnable()
{
	WgWidget * p = FirstWidget();
	while( p )
	{
		p->SetEnabled(true);
		p = p->NextSibling();
	}
}

//____ _onDisable() ____________________________________________________________

void WgContainer::_onDisable()
{
	WgWidget * p = FirstWidget();
	while( p )
	{
		p->SetEnabled(false);
		p = p->NextSibling();
	}
}

//____ _setScale() _________________________________________________________

void WgContainer::_setScale( int scale )
{
	WgWidget::_setScale( scale );

	WgWidget * p = FirstWidget();
	while( p )
	{
		p->_setScale( scale );
		p = p->NextSibling();
	}
}


//____ _renderPatches() _____________________________________________________
// Default implementation for panel rendering patches.
class WidgetRenderContext
{
public:
	WidgetRenderContext() : pWidget(0) {}
	WidgetRenderContext( WgWidget * pWidget, const WgRect& geo ) : pWidget(pWidget), geo(geo) {}

	WgWidget *	pWidget;
	WgRect		geo;
	WgPatches	patches;
};

void WgContainer::_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{

	// We start by eliminating dirt outside our geometry

	WgPatches 	patches( _pPatches->size() );								// TODO: Optimize by pre-allocating?
	
	for( const WgRect * pRect = _pPatches->begin() ; pRect != _pPatches->end() ; pRect++ )
	{
		if( _canvas.isOverlapping( *pRect ) )
			patches.push( WgRect::overlap(*pRect,_canvas) );
	}

	if( patches.isEmpty() )
		return;

	// Render container itself

	const WgRect * pRects = patches.begin();
	int nRects = patches.size();

	int bytesToRelease = _convertAndPushClipList( pDevice, patches.size(), patches.begin() );
	
	_onRender(pDevice, _canvas, _window );


	// Render children

	WgRect	dirtBounds = patches.bounds();

	if( m_bSiblingsOverlap )
	{

		// Create WidgetRenderContext's for siblings that might get dirty patches

		std::vector<WidgetRenderContext> renderList;

		WgRect childGeo;
		WgHook * p = _firstHookWithGeo( childGeo );
		while(p)
		{
			WgRect geo = childGeo + _canvas.pos();

			bool bVisibleHook = IsPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;

			if( bVisibleHook && geo.isOverlapping( dirtBounds ) )
				renderList.push_back( WidgetRenderContext(p->Widget(), geo ) );

			p = _nextHookWithGeo( childGeo, p );
		}

		// Go through WidgetRenderContexts in reverse order (topmost first), push and mask dirt

		for( int i = int(renderList.size()-1) ; i >= 0 ; i-- )
		{
			WidgetRenderContext * p = &renderList[i];

			p->patches.push( &patches );

			p->pWidget->_onMaskPatches( patches, p->geo, p->geo, pDevice->blendMode() );		//TODO: Need some optimizations here, grandchildren can be called repeatedly! Expensive!

			if( patches.isEmpty() )
				break;
		}

		// Go through WidgetRenderContexts and render the patches

		for( int i = 0 ; i < (int) renderList.size() ; i++ )
		{
			WidgetRenderContext * p = &renderList[i];
            if( !p->patches.isEmpty() )
                p->pWidget->_renderPatches( pDevice, p->geo, p->geo, &p->patches );
		}

	}
	else
	{
		WgRect childGeo;
		WgHook * p = _firstHookWithGeo( childGeo );

		while(p)
		{
			WgRect canvas = childGeo + _canvas.pos();
			bool bVisibleHook = IsPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;
			if( bVisibleHook && canvas.isOverlapping( dirtBounds ) )
				p->Widget()->_renderPatches( pDevice, canvas, canvas, &patches );
			p = _nextHookWithGeo( childGeo, p );
		}

	}
	
	_popAndReleaseClipList( pDevice, bytesToRelease );
}

//____ _onCloneContent() _______________________________________________________

void WgContainer::_onCloneContent( const WgContainer * _pOrg )
{
	m_bSiblingsOverlap 	= _pOrg->m_bSiblingsOverlap;
}

//____ _onCollectPatches() _______________________________________________________

void WgContainer::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
	WgRect childGeo;
	WgHook * p = _firstHookWithGeo( childGeo );

	while(p)
	{
		bool bVisibleHook = IsPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;
		if( bVisibleHook )
			p->Widget()->_onCollectPatches( container, childGeo + geo.pos(), clip );
		p = _nextHookWithGeo( childGeo, p );
	}
}

//____ _onMaskPatches() __________________________________________________________

void WgContainer::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	// Default implementation, should probably be made redundant over time...

	WgRect childGeo;
	WgHook * p = _firstHookWithGeo( childGeo );

	while(p)
	{
		bool bVisibleHook = IsPanel()?static_cast<WgPanelHook*>(p)->IsVisible():true;
		if( bVisibleHook )
			p->Widget()->_onMaskPatches( patches, childGeo + geo.pos(), clip, blendMode );
		p = _nextHookWithGeo( childGeo, p );
	}
}
