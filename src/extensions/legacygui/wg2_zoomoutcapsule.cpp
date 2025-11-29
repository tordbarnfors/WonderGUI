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
#include <wg2_zoomoutcapsule.h>
#include <wg2_gfxdevice.h>
#include <wg2_util.h>
#include <wg2_eventhandler.h>
#include <wg2_patches.h>
#include <wg_surfacefactory.h>
#include <wg2_image.h>
#include <wg2_base.h>
#include <wg2_sizecapsule.h>
#include <wg2_shadercapsule.h>
#include <wg2_util.h>
#include <wg2_dragndroplayer.h>


static const char	c_widgetType[] = {"ZoomOutCapsule"};

//____ Constructor ____________________________________________________________

WgZoomOutCapsule::WgZoomOutCapsule()
{
}

//____ Destructor _____________________________________________________________

WgZoomOutCapsule::~WgZoomOutCapsule()
{
}

//____ Type() _________________________________________________________________

const char * WgZoomOutCapsule::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgZoomOutCapsule::GetClass()
{
	return c_widgetType;
}

//____ SetOutlineSkin() ________________________________________________________

void WgZoomOutCapsule::SetOutlineSkin(wg::Skin * pSkin)
{
	if (pSkin != m_pOutlineSkin)
	{
		m_pOutlineSkin = pSkin;
		if( m_bOutlineMode )
			_requestRender();
	}
}


//____ SetButtonSkin() ____________________________________________________________

void WgZoomOutCapsule::SetButtonSkin(wg::Skin * pSkin)
{
		if( pSkin != m_pButtonSkin )
		{
			m_pButtonSkin = pSkin;
			_requestRender();
		}
}

//____ SetMinPointSize() __________________________________________________________

void WgZoomOutCapsule::SetMinPointSize( WgSize min )
{
	m_minPointSize = min;

	WgSize size = PixelSize();
	if( size.w < min.w*m_scale/WG_SCALE_BASE || size.h < min.h *m_scale/WG_SCALE_BASE )
		_requestResize();

}

//____ MinPixelSize() ___________________________________________________________

WgSize WgZoomOutCapsule::MinPixelSize() const
{
	return m_minPointSize * m_scale / WG_SCALE_BASE;
}


//____ PreferredPixelSize() ______________________________________________________

WgSize WgZoomOutCapsule::PreferredPixelSize() const
{
	WgSize preferred = WgCapsule::PreferredPixelSize() * m_outerZoom;

	WgSize minSize = m_minPointSize * m_scale / WG_SCALE_BASE;

	if( preferred.w < minSize.w)
		preferred.w = minSize.w;

	if(preferred.h < minSize.h)
		preferred.h = minSize.h;

	return preferred;
}

//____ UnzoomedPreferredPixelSize() ______________________________________________________

WgSize WgZoomOutCapsule::UnzoomedPreferredPixelSize() const
{
	WgSize preferred = WgCapsule::PreferredPixelSize();

	WgSize minSize = m_minPointSize * m_scale / WG_SCALE_BASE;

	if( preferred.w < minSize.w)
		preferred.w = minSize.w;

	if(preferred.h < minSize.h)
		preferred.h = minSize.h;

	return preferred;
}

//____ MatchingPixelHeight() _____________________________________________________________

int WgZoomOutCapsule::MatchingPixelHeight(int pixelWidth) const
{
	if (pixelWidth == 0)
		return 0;

	int preferredWidth = WgCapsule::PreferredPixelSize().w;
	int preferredScaled = int(preferredWidth * m_outerZoom);

	float fraction = pixelWidth / (float) preferredScaled;

	int matching = int(WgCapsule::MatchingPixelHeight(int(preferredWidth*fraction)) * m_outerZoom);

	int min = m_minPointSize.h * m_scale / WG_SCALE_BASE;

	if( matching < min )
		matching = min;

	return matching;
}

//____ MatchingPixelWidth() _____________________________________________________________

int WgZoomOutCapsule::MatchingPixelWidth(int pixelHeight) const
{
	if (pixelHeight == 0)
		return 0;

	int preferredHeight = WgCapsule::PreferredPixelSize().h;
	int preferredScaled = int(preferredHeight * m_outerZoom);

	float fraction = pixelHeight / (float) preferredScaled;

	int matching = int(WgCapsule::MatchingPixelWidth(int(preferredHeight*fraction)) * m_outerZoom);

	int min = m_minPointSize.w * m_scale / WG_SCALE_BASE;

	if( matching < min )
		matching = min;

	return matching;
}

//____ SetPickableContent() ______________________________________________________________

void WgZoomOutCapsule::SetPickableContent(bool bPickable)
{
    m_bPickableContent = bPickable;
}



//____ SetInOutTransition() ___________________________________________________________

void WgZoomOutCapsule::SetInOutTransition(float factor )
{
	wg::limit(factor, 0.f, 1.f);

	if( factor != m_transitionFactor )
	{
		m_transitionFactor = factor;

		m_outerZoom = WgUtil::ParametricBlendInOut(factor);
		_requestResize();
	}
}

//____ SetOutlineMode() ___________________________________________________________

void WgZoomOutCapsule::SetOutlineMode(bool bOutline)
{
	if( bOutline != m_bOutlineMode )
	{
		m_bOutlineMode = bOutline;
		_requestRender();
	}
}

//____ SetTint() ________________________________________________________________

void WgZoomOutCapsule::SetTint( WgColor tint )
{
	if( tint != m_tint )
	{
		m_tint = tint;
		_requestRender();
	}
}

//____ ContentAreaPixels() ____________________________________________________________

WgRect WgZoomOutCapsule::ContentAreaPixels() const
{
	return PixelSize();
}

//____ ContentAreaPoints() ____________________________________________________________

WgRect WgZoomOutCapsule::ContentAreaPoints() const
{
	return PointSize();
}


//____ FindWidget() ____________________________________________________________

WgWidget * WgZoomOutCapsule::FindWidget( const WgCoord& ofs, WgSearchMode mode )
{
	if( m_bPickableContent == false )
		return WgCapsule::FindWidget(ofs,mode);
	else
	{
		if( mode == WgSearchMode::Geometry || _onAlphaTest(ofs) )
			return this;

		return nullptr;
	}
}

//____ SetStaticScreenshot() ___________________________________________________

void WgZoomOutCapsule::SetStaticScreenshot( wg::Surface *  pSurface )
{
	m_pScreenshot = pSurface;
	m_bStaticScreenshot = true;
}


//____ _onCloneContent() _______________________________________________________

void WgZoomOutCapsule::_onCloneContent( const WgWidget * _pOrg )
{
//	WgZoomOutCapsule * pOrg = (WgZoomOutCapsule*) _pOrg;

}

//____ _childPos() _____________________________________________________________

WgCoord WgZoomOutCapsule::_childPos()
{
	return WgCapsule::_childPos();
}

//____ _childSize() ____________________________________________________________

WgSize WgZoomOutCapsule::_childSize()
{
	if( m_bTakingScreenshot )
		return m_hook.Widget()->PreferredPixelSize();

	return WgCapsule::_childSize();
}

//____ _childGeo() _____________________________________________________________

WgRect WgZoomOutCapsule::_childGeo()
{
	if( m_bTakingScreenshot )
	{
		if( m_pSkin )
			return { _skinContentOfs( m_pSkin, m_state, m_scale ), m_hook.Widget()->PreferredPixelSize() };
		else
			return { 0,0, m_hook.Widget()->PreferredPixelSize() };
	}

	return WgCapsule::_childGeo();
}

//____ _onEvent() ______________________________________________________________

void WgZoomOutCapsule::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	if( m_bPickableContent == false )
	{
		WgCapsule::_onEvent(pEvent, pHandler);
		return;
	}

	switch( pEvent->Type() )
	{
		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
		   auto * pEv = static_cast<const WgEvent::MouseButtonPress*>(pEvent);

			if( pEv->Button() == 1 && m_buttonState.isHovered() )
			{
				m_bButtonPressed = true;
				_updateButtonState(PixelSize(), pEvent->PointerPixelPos() );
			}
		}
		break;

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			auto * pEv = static_cast<const WgEvent::MouseButtonRelease*>(pEvent);

			if( pEv->Button() == 1 )
			{
				m_bButtonPressed = true;
				if( m_buttonState.isPressed() )
				{
					_updateButtonState(PixelSize(), pEvent->PointerPixelPos() );
					pHandler->QueueEvent( new WgEvent::Selected(this) );
				}
			}
		}
		break;

		case WG_EVENT_MOUSE_ENTER:
		case WG_EVENT_MOUSE_MOVE:
		case WG_EVENT_MOUSE_LEAVE:
		{
			_updateButtonState(PixelSize(), pEvent->PointerPixelPos() );
		}
		break;

		case WG_EVENT_DROP_PICK:
		{
			auto * pEv = const_cast<WgEvent::DropPick*>(static_cast<const WgEvent::DropPick*>(pEvent));

            pEv->setHidePointer(true);
            
			WgCoord pickOfs = pEv->pickOfs();

			WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, PixelSize(), m_state, m_scale) : WgRect(PixelSize());

//			if( screenshotArea.contains(pickOfs) || m_innerZoom > 0.95f )
			{
				pEv->setPayload(wg::Dataset<WgWidgetWeakPtr>::create(m_hook.Widget()));

				// Generate a screenshot

				if( !m_pScreenshot )
					_regenScreenshot();

				// We need a separate surface that will belong to our image.

				auto pSurface = wg::GfxBase::defaultSurfaceFactory()->createSurface( WGBP(Surface,
																						  _.size = m_pScreenshot->pixelSize(),
																						  _.format = WgPixelType::BGRA_8,
																						  _.mipmap = true));
				pSurface->copy( {0,0}, m_pScreenshot );

				auto pImage = new WgImage();
				pImage->SetImage(pSurface);

				// We need to know the scale factor of our drag-n-drop layer since that likely is different.

				WgContainer * p = Parent();
				while( p && p->Type() != WgDragNDropLayer::GetClass() )
					p = p->Parent();

				int destScaleFactor = p ? p->Scale() : WG_SCALE_BASE;

				//

				WgSize sz = m_pScreenshot->pixelSize();


				auto pDragWidget = new WgSizeCapsule();
				pDragWidget->SetPreferredSize(sz*WG_SCALE_BASE/destScaleFactor);

				//

				auto pShader = new WgShaderCapsule();
				pShader->SetColor( {255,255,255,128} );
				pShader->SetChild(pImage);

				pDragWidget->SetChild(pShader);
                pEv->setDragWidget( pDragWidget, pickOfs );
			}
//			else
//				return;             // Prevent WgWidget from setting default payload.
		}
		break;

//        case WG_EVENT_DROP_CANCEL:
//            SetOutlineMode(false);
//            break;

		default:
			break;

	}

	//TODO: Should swallow mouse button events

	WgCapsule::_onEvent(pEvent, pHandler);

}

//____ _updateButtonState() __________________________________________________

void WgZoomOutCapsule::_updateButtonState( WgSize gizmoCanvas, WgCoord pointerPos )
{
	WgRect r = _buttonArea(gizmoCanvas);
	WgState buttonState = WgStateEnum::Default;

	if( r.contains(pointerPos) )
	{
		if( m_bButtonPressed )
			buttonState = WgStateEnum::Pressed;
		else
			buttonState = WgStateEnum::Hovered;
	}

	if( buttonState != m_buttonState )
	{
		m_buttonState = buttonState;
		_requestRender();
	}
}

//____ _regenScreenshot() _____________________________________________________

void WgZoomOutCapsule::_regenScreenshot()
{
	if( m_bStaticScreenshot )
		return;

	WgSize realSize = m_hook.PixelSize();
	WgSize prefSize = m_hook.Widget()->PreferredPixelSize();

	m_bTakingScreenshot = true;

	if( realSize != prefSize )
		m_hook.Widget()->_onNewSize(prefSize);

	m_pScreenshot = m_hook.Widget()->Screenshot( WGBP(Surface, _.mipmap = true) );

	if( realSize != prefSize )
		m_hook.Widget()->_onNewSize(realSize);

	m_bTakingScreenshot = false;
}

//____ _onCollectPatches() ______________________________________________________

void WgZoomOutCapsule::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
	// Do the simple thing here

	container.add( WgRect::overlap( geo, clip ) );
}

//____ _onMaskPatches() _________________________________________________________

void WgZoomOutCapsule::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
    if( m_bOutlineMode == false )
    {
        WgCapsule::_onMaskPatches( patches, geo, clip, blendMode );
    }
}


//____ _renderPatches() __________________________________________________________

void WgZoomOutCapsule::_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{
	if( _pPatches->isEmpty() )
		return;
	
	if( m_bOutlineMode == false ) //&& m_outerZoom == 1.f )
	{
		WgColor oldTint;

		if( m_tint != WgColor::White )
		{
			oldTint = pDevice->tintColor();
			pDevice->setTintColor( oldTint * m_tint );
		}

		WgCapsule::_renderPatches( pDevice, _canvas, _window, _pPatches );

		if( m_tint != WgColor::White )
			pDevice->setTintColor( oldTint );
	}
	else
	{
		WgColor oldTint;

		if( m_tint != WgColor::White )
		{
			oldTint = pDevice->tintColor();
			pDevice->setTintColor( oldTint * m_tint );
		}

		int bytesToRelease = _convertAndPushClipList( pDevice, _pPatches->size(), _pPatches->begin() );
		_onRender( pDevice, _canvas, _window );
		_popAndReleaseClipList( pDevice, bytesToRelease);

		if( m_tint != WgColor::White )
			pDevice->setTintColor( oldTint );
	}
}


//____ _onRender() _____________________________________________________________

void WgZoomOutCapsule::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{

//	WgCapsule::_onRender(pDevice, _canvas, _window);

	if( m_bOutlineMode == true || m_outerZoom < 1.f )
	{
		WgColor oldTint = pDevice->tintColor();

	    uint8_t newTint = uint8_t(oldTint.a*WgUtil::ParametricBlendInOut(1.f /*-m_innerTransitionFactor*/));

		pDevice->setTintColor( wg::Color8(oldTint.r, oldTint.g, oldTint.b, newTint) );

		if (m_pSkin)
			_renderSkin( m_pSkin, pDevice, m_state, _canvas, m_scale);

		WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, _canvas, m_state, m_scale) : _canvas;

		if( m_pButtonSkin )
		{
			pDevice->setTintColor( wg::Color8(oldTint.r, oldTint.g, oldTint.b, uint8_t(newTint*m_outerZoom)) );
			_renderSkin( m_pButtonSkin, pDevice, m_buttonState, _buttonArea(canvas), m_scale);
		}
		pDevice->setTintColor(oldTint);

		if( m_bOutlineMode && m_pOutlineSkin )
			_renderSkin( m_pOutlineSkin, pDevice, m_state, _canvas, m_scale);
	}
}

//____ _onAlphaTest() ___________________________________________________________

bool WgZoomOutCapsule::_onAlphaTest( const WgCoord& ofs )
{
	WgSize sz = PixelSize();

	WgRect canvas = m_pSkin ? _skinContentRect( m_pSkin, sz, m_state, m_scale) : WgRect(sz);

	if( m_pButtonSkin )
	{
		WgRect r = _buttonArea(canvas);

		if( r.contains(ofs) )
		{
			int alpha = _markTestSkin( m_pButtonSkin, ofs - r.pos(), r.size(), m_state, m_markOpacity, m_scale);
			if( alpha >= m_markOpacity )
				return true;
		}
	}

	if( m_pSkin )
	{
		return _markTestSkin( m_pSkin, ofs, sz, m_state, m_markOpacity, m_scale);
	}

	return false;
}

//____ _setScale() ____________________________________________________________

void WgZoomOutCapsule::_setScale( int scale )
{
	WgCapsule::_setScale(scale);

	if( m_pScreenshot || m_pSkin )
		_requestResize();

	if( !m_bStaticScreenshot )
	{
		m_pScreenshot = nullptr;
	}
}

//____ _buttonArea() _____________________________________________________

WgRect WgZoomOutCapsule::_buttonArea( WgRect canvas ) const
{
	if( !m_pButtonSkin )
		return { 0, 0, 0, 0 };

	WgSize sz = _skinPreferredSize( m_pButtonSkin, m_scale);

	return { canvas.pos(), sz };
}


