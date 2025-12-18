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
#include <wg2_shadowlayer.h>
#include <wg2_eventhandler.h>
#include <wg2_event.h>
#include <wg2_gfxdevice.h>
#include <wg_surfacefactory.h>
#include <wg2_patches.h>
#include <wg2_base.h>
#include <wg2_util.h>

#include <algorithm>

using namespace WgUtil;

static const char    c_widgetType[] = { "ShadowLayer" };
static const char    c_hookType[] = { "ShadowHook" };

//_____________________________________________________________________________
WgShadowLayer* WgShadowHook::Parent() const
{
	return m_pParent;
}


//_____________________________________________________________________________
const char * WgShadowHook::Type(void) const
{
	return ClassType();
}

//_____________________________________________________________________________
const char * WgShadowHook::ClassType()
{
	return c_hookType;
}

//_____________________________________________________________________________
void WgShadowHook::_requestResize()
{
}

//_____________________________________________________________________________
WgHook * WgShadowHook::_prevHook() const
{
	if (m_pParent->m_baseHook.Widget())
		return &m_pParent->m_baseHook;
	else
		return 0;
}

//_____________________________________________________________________________
WgHook * WgShadowHook::_nextHook() const
{
	return 0;
}

//_____________________________________________________________________________
WgContainer * WgShadowHook::_parent() const
{
	return m_pParent;
}




//____ Constructor ____________________________________________________________

WgShadowLayer::WgShadowLayer() : m_frontHook(this)
{
	_startReceiveTicks();
}

//____ Destructor _____________________________________________________________

WgShadowLayer::~WgShadowLayer()
{
	_stopReceiveTicks();
}

//____ Type() _________________________________________________________________

const char *WgShadowLayer::Type(void) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgShadowLayer::GetClass()
{
	return c_widgetType;
}

//____ SetFront() _____________________________________________________________

WgHook * WgShadowLayer::SetFront(WgWidget * pWidget)
{
	// Replace Widget

	WgWidget * pOldWidget = m_frontHook._releaseWidget();
	if (pOldWidget)
		delete pOldWidget;
	m_frontHook._attachWidget(pWidget);
	_onFrontChanged();
	return &m_baseHook;
}

//____ Front() ________________________________________________________________

WgWidget * WgShadowLayer::Front()
{
	return m_frontHook.Widget();
}

//____ DeleteFront() __________________________________________________________

bool WgShadowLayer::DeleteFront()
{
	WgWidget * pWidget = m_frontHook._releaseWidget();
	if (pWidget)
	{
		_onFrontChanged();
		delete pWidget;
		return true;
	}

	return false;
}

//____ ReleaseFront() _________________________________________________________

WgWidget * WgShadowLayer::ReleaseFront()
{
	WgWidget * pWidget = m_frontHook._releaseWidget();
	if (pWidget)
		_onFrontChanged();

	return pWidget;
}

//____ DeleteChild() __________________________________________________________

bool WgShadowLayer::DeleteChild(WgWidget * pWidget)
{
	if (pWidget == m_baseHook.Widget())
		return DeleteBase();
	else if (pWidget == m_frontHook.Widget())
		return DeleteFront();

	return false;
}

//____ ReleaseChild() _________________________________________________________

WgWidget * WgShadowLayer::ReleaseChild(WgWidget * pWidget)
{
	if (pWidget == m_baseHook.Widget())
		return ReleaseBase();
	else if (pWidget == m_frontHook.Widget())
		return ReleaseFront();

	return nullptr;
}

//____ DeleteAllChildren() ____________________________________________________

bool WgShadowLayer::DeleteAllChildren()
{
	return DeleteBase() || DeleteFront();
}

//____ ReleaseAllChildren() ___________________________________________________

bool WgShadowLayer::ReleaseAllChildren()
{
	return ReleaseBase() || ReleaseFront();
}

//____ ClearShadows() _________________________________________________________

void WgShadowLayer::ClearShadows()
{
	_willRemoveShadows(0, (int) m_shadows.size());
	m_shadows.clear();
}

//____ AddShadow() ____________________________________________________________

bool WgShadowLayer::AddShadow(WgWidget * pWidget, wg::Skin * pShadow)
{
	if( !pWidget || !pShadow )
		return false;

	int scale = pWidget->Scale();

	WgCoord pos;
	_descendantPos(pWidget, pos);

	WgRect geo = { pos - _skinContentOfs( pShadow, WgStateEnum::Default, scale), _skinSizeForContent( pShadow, pWidget->PixelSize(), scale) };

	m_shadows.push_back( WgShadow(pWidget,pShadow,geo));

	WgPatches patches;
	patches.add(geo);

	// Remove portions of patches that are covered by opaque front widgets

	m_frontHook.Widget()->_onMaskPatches(patches, m_size, m_size, _getBlendMode());

	// Make request render calls

	for (const WgRect * pRect = patches.begin(); pRect < patches.end(); pRect++)
		_requestRender(*pRect);

	return true;
}

//____ RemoveShadow() _________________________________________________________

void WgShadowLayer::RemoveShadow(int index)
{
	_willRemoveShadows(index, 1);
	m_shadows.erase(m_shadows.begin() + index);
}

void WgShadowLayer::RemoveShadow(WgWidget * pWidget)
{
	for (auto it = m_shadows.begin() ; it != m_shadows.end(); it++)
	{
		if (it->widget() == pWidget)
		{
			_willRemoveShadows(int(it - m_shadows.begin()), 1);
			m_shadows.erase(it);
			break;
		}
	}
}

//____ SetShadowTint() __________________________________________________________

void WgShadowLayer::SetShadowTint( uint8_t alpha )
{
	if( m_shadowTint != alpha )
	{
		m_shadowTint = alpha;
		_requestRenderShadows( 0, (int) m_shadows.size() );
	}
}

//____ MatchingPixelHeight() _________________________________________________________________

int WgShadowLayer::MatchingPixelHeight( int width ) const
{
	int matchFront = 0;
	int matchBase = 0;

	WgSize padding = m_pSkin ? _skinContentPadding( m_pSkin, m_scale) : WgSize(0,0);
	width -= padding.w;

	if (m_baseHook.Widget() )
		matchBase = m_baseHook.Widget()->MatchingPixelHeight(width);
	if (m_frontHook.Widget() )
		matchFront = m_frontHook.Widget()->MatchingPixelHeight(width);

	return std::max(matchFront, matchBase) + padding.h;
}

//____ MatchingPixelWidth() _________________________________________________________________

int WgShadowLayer::MatchingPixelWidth( int height ) const
{
	int matchFront = 0;
	int matchBase = 0;

	WgSize padding = m_pSkin ? _skinContentPadding( m_pSkin, m_scale) : WgSize(0,0);
	height -= padding.h;

	if (m_baseHook.Widget() )
		matchBase = m_baseHook.Widget()->MatchingPixelWidth(height);
	if (m_frontHook.Widget() )
		matchFront = m_frontHook.Widget()->MatchingPixelWidth(height);

	return std::max(matchFront, matchBase) + padding.w;
}

//____ PreferredPixelSize() _________________________________________________________________

WgSize WgShadowLayer::PreferredPixelSize() const
{
	WgSize prefFront;
	WgSize prefBase;
	WgSize padding = m_pSkin ? _skinContentPadding( m_pSkin, m_scale) : WgSize(0,0);

	if (m_baseHook.Widget() )
		prefBase = m_baseHook.Widget()->PreferredPixelSize();
	if (m_frontHook.Widget() )
		prefFront = m_frontHook.Widget()->PreferredPixelSize();

	return WgSize::max(prefFront, prefBase) + padding;
}

//____ _willRemoveShadows() ________________________________________________________________

void WgShadowLayer::_willRemoveShadows(int ofs, int nb)
{
	_requestRenderShadows(ofs,nb);
}

//____ _requestRenderShadows() ________________________________________________________

void WgShadowLayer::_requestRenderShadows(int ofs, int nb)
{
	for (auto p = &m_shadows[ofs]; p != &m_shadows[ofs + nb]; p++)
	{
		WgPatches patches;
		patches.add(p->m_geo );

		// Remove portions of patches that are covered by opaque front widgets

		m_frontHook.Widget()->_onMaskPatches(patches, m_size, m_size, _getBlendMode());

		// Make request render calls

		for (const WgRect * pRect = patches.begin(); pRect < patches.end(); pRect++)
			_requestRender(*pRect);
	}
}

//____ _firstLayerHook() _______________________________________________________________

WgLayerHook * WgShadowLayer::_firstLayerHook() const
{
	if (m_frontHook.Widget() )
		return (WgLayerHook*) &m_frontHook;

	return nullptr;
}

//____ _firstHook() ___________________________________________________________________

WgHook* WgShadowLayer::_firstHook() const
{
	if (m_baseHook.Widget())
		return (WgHook*)&m_baseHook;
	else if (m_frontHook.Widget())
		return (WgHook*)&m_frontHook;

	return nullptr;
}

//____ _lastHook() ____________________________________________________________________

WgHook* WgShadowLayer::_lastHook() const
{
	if (m_frontHook.Widget())
		return (WgHook*)&m_frontHook;
	else if (m_baseHook.Widget())
		return (WgHook*)&m_baseHook;

	return nullptr;
}

//____ _firstHookWithGeo() ____________________________________________________

WgHook * WgShadowLayer::_firstHookWithGeo(WgRect& geo) const
{
	if (m_baseHook.Widget())
	{
		geo = m_frontHook.m_geo;
		return (WgHook*)&m_baseHook;
	}
	else if (m_frontHook.Widget())
	{
		geo = m_frontHook.m_geo;
		return (WgHook*)&m_frontHook;
	}
	return nullptr;
}

//____ _nextHookWithGeo() _____________________________________________________

WgHook * WgShadowLayer::_nextHookWithGeo(WgRect& geo, WgHook * pHook) const
{
	if (pHook == &m_baseHook && m_frontHook.Widget())
	{
		geo = m_frontHook.m_geo;
		return (WgHook*)&m_frontHook;
	}

	return nullptr;
}

//____ _lastHookWithGeo() _____________________________________________________

WgHook * WgShadowLayer::_lastHookWithGeo(WgRect& geo) const
{
	if (m_frontHook.Widget())
	{
		geo = m_frontHook.m_geo;
		return (WgHook*)&m_frontHook;
	}
	else if (m_baseHook.Widget())
	{
		geo = m_frontHook.m_geo;
		return (WgHook*)&m_baseHook;
	}

	return nullptr;
}

//____ _prevHookWithGeo() _____________________________________________________

WgHook * WgShadowLayer::_prevHookWithGeo(WgRect& geo, WgHook * pHook) const
{
	if (pHook == &m_frontHook && m_baseHook.Widget())
	{
		geo = m_frontHook.m_geo;
		return (WgHook*)&m_baseHook;
	}

	return nullptr;
}

//____ _onCloneContent() ______________________________________________________

void WgShadowLayer::_onCloneContent(const WgWidget * _pOrg)
{

}

//____ _onEvent() _____________________________________________________________

void WgShadowLayer::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	switch (pEvent->Type())
	{
		case WgEventType::WG_EVENT_TICK:
		{
			if( !m_bPreRenderRequested )
			{
				_requestPreRenderCall();
				m_bPreRenderRequested = true;
			}
			break;
		}

		default:
			WgLayer::_onEvent(pEvent, pHandler);
			break;
	}
}

//____ _preRender() ____________________________________________________________

void WgShadowLayer::_preRender()
{
	// Check for removed children and changes to geo that will affect shadows.

	WgPatches patches;

	for (auto it = m_shadows.begin(); it < m_shadows.end();)
	{
		WgWidget * pWidget = it->widget();
		WgShadow * pShadow = &(*it);

		// Remove shadow for deleted widget.

		if (pWidget == nullptr)
		{
			patches.add(pShadow->m_geo);
			it = m_shadows.erase(it);
			continue;
		}

		//

		WgCoord pos;
		WgRect oldGeo = pShadow->m_geo;
		WgSize widgetSize = pWidget->PixelSize();

		if( widgetSize.w > 0 && widgetSize.h > 0 && _descendantPos(pWidget, pos))
		{
			// Widget is still our descendant and visible, check
			// so its geo has not changed.

			int scale = pWidget->Scale();

			wg::Skin * pSkin = pShadow->shadow();
			WgRect geo = { pos - _skinContentOfs( pSkin, WgStateEnum::Default,scale), _skinSizeForContent( pSkin, widgetSize, scale) };

			if (geo != oldGeo)
			{
				patches.add(oldGeo);
				patches.add(geo);
				pShadow->m_geo = geo;
			}
		}
		else
		{
			// Widget is currently hidden or not a descendant of us,
			// hide the shadow.

			if (!oldGeo.isEmpty())
			{
				patches.add(oldGeo);
				pShadow->m_geo = {0,0,0,0};
			}
		}

		it++;       // Only increase iterator if widget wasn't deleted.
	}

	// Early out if there is nothing to update in shadow layer.

	if (patches.isEmpty())
		return;

	// Mask foreground from shadow updates and request render on the remains.

	if (m_frontHook.Widget())
		m_frontHook.Widget()->_onMaskPatches(patches, m_size, m_size, WgBlendMode::Blend);

	for (const WgRect * pRect = patches.begin(); pRect < patches.end(); pRect++)
		_requestRender(*pRect);
}

//____ _onNewSize() ___________________________________________________________

void WgShadowLayer::_onNewSize(const WgSize& size)
{
	m_pShadowSurface = nullptr;

	if( m_pSkin )
		m_frontHook.m_geo = _skinContentRect( m_pSkin, size, m_state, m_scale);
	else
		m_frontHook.m_geo = { 0,0,size };

	if (m_frontHook.Widget())
		m_frontHook.Widget()->_onNewSize(m_frontHook.m_geo);

	WgLayer::_onNewSize(size);
}

//____ _renderPatches() _______________________________________________________

void WgShadowLayer::_renderPatches(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches)
{
	WgRect contentGeo = m_frontHook.m_geo + _canvas.pos();

	WgPatches patches;
	patches.push(_pPatches);

	// Generate masked patches for our skin, baseSlot widget, and shadow.

	if (m_frontHook.Widget() )
		m_frontHook.Widget()->_onMaskPatches(patches, contentGeo, contentGeo, pDevice->blendMode());		//TODO: Need some optimizations here, grandchildren can be called repeatedly! Expensive!

	patches.clip(_canvas);
	if (!patches.isEmpty())
	{
		// If we have a skin, render it

		if (m_pSkin)
		{
			int bytesToRelease = _convertAndPushClipList( pDevice, patches.size(), patches.begin() );
			_renderSkin( m_pSkin, pDevice, m_state, _canvas, m_scale );
			_popAndReleaseClipList( pDevice, bytesToRelease );
		}

		// Render base slot widgets

		if (m_baseHook.Widget() )
			m_baseHook.Widget()->_renderPatches(pDevice, contentGeo, contentGeo, &patches);

		// Update shadow layer

		bool bFullSurfaceUpdate = !m_pShadowSurface;

		if (!m_pShadowSurface)
		{
			auto pSurfaceFactory = wg::GfxBase::defaultSurfaceFactory();

			if (pSurfaceFactory)
			{
				m_pShadowSurface = pSurfaceFactory->createSurface( WGBP(Surface,
																		_.size = _canvas.size(),
																		_.format = WgPixelType::BGRA_8,
																		_.canvas = true ));
				m_pShadowSurface->fill(WgColor::Transparent);
			}
		}

		if (m_pShadowSurface)
		{
			WgPatches   shadowPatches;

			if (bFullSurfaceUpdate)
				shadowPatches.push({ 0,0, _canvas.size()*64 });
			else
			{
				const WgRect * pPatches = patches.begin();
				for (int i = 0; i < patches.size(); i++)
					shadowPatches.push((pPatches[i] - _canvas.pos())*64);
			}

			auto oldBlendMode = pDevice->blendMode();
			auto oldTint = pDevice->tintColor();

			if (!shadowPatches.isEmpty())
			{
				pDevice->beginCanvasUpdate(m_pShadowSurface,shadowPatches.size(), shadowPatches.begin());

				pDevice->setTintColor(WgColor::White);
				pDevice->setBlendMode(WgBlendMode::Replace);
				pDevice->fill( wg::Color8(0,0,0,0) );

				pDevice->setBlendMode(WgBlendMode::Max);

				for (auto& shadow : m_shadows)
					if( !shadow.m_geo.isEmpty() && shadow.widget() )
						_renderSkin( shadow.shadow(), pDevice, WgStateEnum::Default, shadow.m_geo, shadow.widget()->Scale());


				pDevice->endCanvasUpdate();
			}

        
			// Render shadows

			int bytesToRelease = _convertAndPushClipList( pDevice, patches.size(), patches.begin() );

			pDevice->setTintColor( wg::Color8(255,255,255,m_shadowTint) );
			pDevice->setBlendMode(WgBlendMode::Blend);
			pDevice->setBlitSource( m_pShadowSurface );
			pDevice->blit( _canvas.pos()*64 );
 
			pDevice->setTintColor(oldTint);
			pDevice->setBlendMode(oldBlendMode);

			_popAndReleaseClipList( pDevice, bytesToRelease );
		}
	}

	//Render front slot widgets

	if (m_frontHook.Widget() )
		m_frontHook.Widget()->_renderPatches(pDevice, contentGeo, contentGeo, _pPatches);

    // Request next PreRenderCall.
  
	m_bPreRenderRequested = false;
//    _requestPreRenderCall();

}


//____ _onFrontChanged() ______________________________________________________

void WgShadowLayer::_onFrontChanged()
{
	_onRequestRender(WgRect(0, 0, m_size), 0);
	_requestResize();

	if (m_frontHook.Widget())
		m_frontHook.Widget()->_onNewSize(m_size);
}


