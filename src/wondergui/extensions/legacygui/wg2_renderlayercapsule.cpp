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
#include <wg2_renderlayercapsule.h>
#include <wg2_gfxdevice.h>
#include <wg2_patches.h>

static const char	c_widgetType[] = {"RenderLayerCapsule"};

//____ Constructor ____________________________________________________________

WgRenderLayerCapsule::WgRenderLayerCapsule()
{
}

WgRenderLayerCapsule::WgRenderLayerCapsule(WgWidget * pChild, int layer)
{
    if( pChild )
    {
        m_hook._attachWidget(pChild);
        pChild->_onNewSize(PixelSize());
    }

    m_renderLayer = layer;
}

//____ Destructor _____________________________________________________________

WgRenderLayerCapsule::~WgRenderLayerCapsule()
{
}

//____ Type() _________________________________________________________________

const char * WgRenderLayerCapsule::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgRenderLayerCapsule::GetClass()
{
	return c_widgetType;
}

//____ SetRenderLayer() ______________________________________________________________

void WgRenderLayerCapsule::SetRenderLayer( int renderLayer)
{
	if( renderLayer != m_renderLayer )
	{
		m_renderLayer = renderLayer;
		_requestRender();
	}
}

//____ _renderPatches() ________________________________________________________

void WgRenderLayerCapsule::_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{
	pDevice->setClipList(_pPatches->size(), _pPatches->begin());

	if( m_pSkin )
		_renderSkin( m_pSkin, pDevice, m_state, _canvas, m_scale );

    // Set RenderLayer
    
    int oldRenderLayer = pDevice->renderLayer();
    
    if( m_renderLayer >= 0 )
        pDevice->setRenderLayer(m_renderLayer);

	// Render children recursively

	if( m_hook.Widget() )
	{
		WgRect childCanvas = _canvas;
		if( m_pSkin )
			childCanvas = _skinContentRect( m_pSkin, _canvas, m_state, m_scale );

		m_hook.Widget()->_renderPatches( pDevice, childCanvas, childCanvas, _pPatches );
	}

	// Reset old RenderLayer

    if( m_renderLayer >= 0 )
        pDevice->setRenderLayer(oldRenderLayer);
}


//____ _onCloneContent() _______________________________________________________

void WgRenderLayerCapsule::_onCloneContent( const WgWidget * _pOrg )
{
	WgRenderLayerCapsule * pOrg = (WgRenderLayerCapsule*) _pOrg;

	m_renderLayer	= pOrg->m_renderLayer;
}
