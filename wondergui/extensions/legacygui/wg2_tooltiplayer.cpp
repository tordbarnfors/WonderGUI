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
#include <wg2_tooltiplayer.h>
#include <wg2_eventhandler.h>
#include <wg2_base.h>
#include <wg_boxskin.h>
#include <wg2_textdisplay.h>


static const char    c_widgetType[] = { "TooltipLayer" };
static const char    c_hookType[] = { "TooltipHook" };
static const char    c_basehookType[] = { "TooltipLayerBasehook" };

using namespace wg;

//_____________________________________________________________________________
WgTooltipLayer * WgTooltipLayer::TooltipHook::Parent() const
{
    return m_pParent;
}


//_____________________________________________________________________________
const char * WgTooltipLayer::TooltipHook::Type(void) const
{
    return ClassType();
}

//_____________________________________________________________________________
const char * WgTooltipLayer::TooltipHook::ClassType()
{
    return c_hookType;
}


//_____________________________________________________________________________
void WgTooltipLayer::TooltipHook::_requestRender()
{
    m_pParent->_onRequestRender(m_geo, this);
}

//_____________________________________________________________________________
void WgTooltipLayer::TooltipHook::_requestRender(const WgRect& rect)
{
    m_pParent->_onRequestRender(rect + m_geo.pos(), this);
}

//_____________________________________________________________________________
void WgTooltipLayer::TooltipHook::_requestResize()
{
    m_pParent->_childRequestResize(this);
}


//____ constructor ____________________________________________________________

WgTooltipLayer::WgTooltipLayer() : m_tooltipHook(this)
{
}

//____ Destructor _____________________________________________________________

WgTooltipLayer::~WgTooltipLayer()
{
}

//____ Type() _________________________________________________________________

const char *WgTooltipLayer::Type(void) const
{
    return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgTooltipLayer::GetClass()
{
    return c_widgetType;
}

//____ setTooltipGenerator() ______________________________________________

void WgTooltipLayer::SetTooltipGenerator(const std::function<WgWidget*(Placement& direction, const WgWidget*, const WgBorders&)> func)
{
    m_tooltipGenerator = func;
}

//____ setTooltipPlacement() ______________________________________________

void WgTooltipLayer::SetTooltipPlacement(WgOrigo direction, WgBorders spacing, bool bTooltipAroundPointer)
{
    m_defaultPlacement.direction = direction;
    m_defaultPlacement.spacing = spacing;
    m_defaultPlacement.bTooltipAroundPointer = bTooltipAroundPointer;
}

//____ SetDisplayTooltips() _______________________________________________

void WgTooltipLayer::SetDisplayTooltips(bool bDisplay)
{
    m_bDisplayTooltips = bDisplay;
    if (!m_bDisplayTooltips)
        _closeTooltip();
}

//____ FindWidget() ______________________________________________________

WgWidget* WgTooltipLayer::FindWidget(const WgCoord& ofs, WgSearchMode mode)
{
    if (!WgRect(PixelSize()).contains(ofs))
        return nullptr;

    if (mode == WgSearchMode::ActionTarget)
    {
        if (m_baseHook.Widget())
        {
            //TODO: Take padding into account

            WgWidget* pChild = m_baseHook.Widget();
            if (pChild->IsContainer())
                return static_cast<WgContainer*>(pChild)->FindWidget(ofs, mode);
            else if (pChild->MarkTest(ofs))
                return pChild;
        }
        return nullptr;
    }
    else
        return WgLayer::FindWidget(ofs, mode);
}

//____ _updateTooltipGeo() ________________________________________________

void WgTooltipLayer::_updateTooltipGeo()
{
    //TODO: Implement!
}

//____ _openTooltip() _____________________________________________________

void WgTooltipLayer::_openTooltip()
{
    if (m_tooltipHook.Widget() || !m_bDisplayTooltips )
        return;

    WgWidget* pHovered = FindWidget(m_hoverPos, WgSearchMode::ActionTarget);
    if (pHovered == nullptr || pHovered->GetTooltipString().isEmpty() )
        return;

    WgRect hoveredGeo = pHovered->ScreenPixelGeo() - ScreenPixelPos();
    WgBorders widgetMargin{ hoveredGeo.y, m_size.w - hoveredGeo.right(), m_size.h - hoveredGeo.bottom(), hoveredGeo.x };

    m_activePlacement = m_defaultPlacement;
    WgWidget * pWidget = m_tooltipGenerator(m_activePlacement, pHovered, widgetMargin);

    if (pWidget)
    {
        pWidget->_setScale(m_scale);
        WgSize tooltipSize = pWidget->PreferredPixelSize();

		if( tooltipSize.w > m_size.w )
		{
			tooltipSize.w = m_size.w;
			tooltipSize.h = pWidget->MatchingPixelHeight(m_size.w);
		}
		
        Placement& placement = m_activePlacement;

        WgRect center = placement.bTooltipAroundPointer ? WgRect(m_hoverPos) : hoveredGeo;
        center += (placement.spacing * m_scale) / WG_SCALE_BASE;

        WgCoord tooltipPos;

        // Take care of vertical placement

        if( placement.direction == WgOrigo::NorthWest || placement.direction == WgOrigo::North || placement.direction == WgOrigo::NorthEast )
        {
            if (center.y < tooltipSize.h)
                tooltipPos.y = center.y + center.h;
            else
                tooltipPos.y = center.y - tooltipSize.h;
        }
        else if (placement.direction == WgOrigo::SouthWest || placement.direction == WgOrigo::South || placement.direction == WgOrigo::SouthEast)
        {
            if (m_size.h - center.bottom() < tooltipSize.h)
                tooltipPos.y = center.y - tooltipSize.h;
            else
                tooltipPos.y = center.y + center.h;
        }
        else
        {
            tooltipPos.y = (center.y + center.h / 2) - tooltipSize.h / 2;
        }

        // Take care of horizontal placement

        if (placement.direction == WgOrigo::NorthWest || placement.direction == WgOrigo::West || placement.direction == WgOrigo::SouthWest)
        {
            if (placement.direction == WgOrigo::West && center.x < tooltipSize.w)
                tooltipPos.x = center.x + center.w;
            else
                tooltipPos.x = center.x - tooltipSize.w;
        }
        else if (placement.direction == WgOrigo::NorthEast || placement.direction == WgOrigo::East || placement.direction == WgOrigo::SouthEast)
        {
            if (placement.direction == WgOrigo::East && m_size.w - center.right() < tooltipSize.w)
                tooltipPos.x = center.x - tooltipSize.w;
            else
                tooltipPos.x = center.x + center.w;
        }
        else
        {
            tooltipPos.x = (center.x + center.w / 2) - tooltipSize.w / 2;
        }

        // Make sure to stay inside screen (if possible).

        limit(tooltipPos.x, 0, m_size.w - tooltipSize.w);
        limit(tooltipPos.y, 0, m_size.h - tooltipSize.h);

        //

        m_tooltipHook._attachWidget(pWidget);
        m_tooltipHook.m_geo = { tooltipPos, tooltipSize };
        m_tooltipHook.m_pWidget->_onNewSize(m_tooltipHook.m_geo);
        _requestRender(m_tooltipHook.m_geo);
    }
}

//____ _closeTooltip() ____________________________________________________

void WgTooltipLayer::_closeTooltip()
{
    if (m_tooltipHook.Widget())
    {
        if (!m_tooltipHook.m_geo.isEmpty())
        {
            _requestRender(m_tooltipHook.m_geo);
            m_tooltipHook.m_geo.clear();
        }
        
        delete m_tooltipHook._releaseWidget();
        m_hoverCountdown = m_hoverMillisec;
    }
}


//____ _onEvent() _________________________________________________________

void WgTooltipLayer::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
    WgLayer::_onEvent(pEvent, pHandler);

    switch (pEvent->Type())
    {
        case WG_EVENT_TICK:
		{
			{
				WgCoord mousePos = pEvent->PointerScreenPixelPos() - ScreenPixelPos();
				WgWidget* pHovered = FindWidget(mousePos, WgSearchMode::ActionTarget);

				if (pHovered && pHovered->GetTooltipString().isEmpty())
					pHovered = nullptr;
				
				if(pHovered)
				{
					// Check that there isn't another TooltipLayer between us (closest one should get the tooltip).
					
					WgWidget * pWidget = pHovered;
					while( pWidget != this )
					{
						if( pWidget->Type() == WgTooltipLayer::GetClass() )
						{
							pHovered = nullptr;
							break;
						}
						pWidget = pWidget->Parent();
					}
				}
				
				
				if (m_tooltipHook.Widget())
				{
					if (pHovered != m_pHoverWidget)
					{
						_closeTooltip();

						m_pHoverWidget = pHovered;
						m_hoverPos = mousePos;
						m_hoverCountdown = m_hoverMillisec;
					}
				}
				else
				{
					if (pHovered != m_pHoverWidget)
					{
						m_bBlockReopen = false;
						m_pHoverWidget = pHovered;
					}
				}
			}


			if (pHandler->IsMouseButtonPressed(1) || pHandler->IsMouseButtonPressed(2) )
                return;

            if (m_hoverCountdown > 0 && m_pHoverWidget && !m_bBlockReopen)
            {
                int ms = static_cast<const WgEvent::Tick*>(pEvent)->Millisec();
                m_hoverCountdown -= ms;
                if (m_hoverCountdown <= 0)
                {
                    m_hoverCountdown = 0;
                    _openTooltip();
                }
            }
            break;
		}
        case WG_EVENT_MOUSE_ENTER:
        _startReceiveTicks();
        // No break, fall through to next case on purpose.
    case WG_EVENT_MOUSE_MOVE:
    {
        WgCoord mousePos = pEvent->PointerScreenPixelPos() - ScreenPixelPos();
        WgWidget* pHovered = FindWidget(mousePos, WgSearchMode::ActionTarget);

        if (pHovered && pHovered->GetTooltipString().isEmpty())
            pHovered = nullptr;
        
        if (m_tooltipHook.Widget())
        {
            if (pHovered != m_pHoverWidget)
            {
                _closeTooltip();

                m_pHoverWidget = pHovered;
                m_hoverPos = mousePos;
                m_hoverCountdown = m_hoverMillisec;
            }
            else if (abs(mousePos.x - m_hoverPos.x) > m_openTooltipMaxMovement || abs(mousePos.y - m_hoverPos.y) > m_openTooltipMaxMovement)
            {
                _closeTooltip();
                m_bBlockReopen = true;
            }
        }
        else
        {
            if (pHovered != m_pHoverWidget)
            {
                m_bBlockReopen = false;
                m_pHoverWidget = pHovered;
            }

            m_hoverPos = mousePos;
            m_hoverCountdown = m_hoverMillisec;
        }
        break;
    }
    case WG_EVENT_MOUSE_LEAVE:
        _stopReceiveTicks();
        m_hoverPos = { -1,-1 };

        if (m_tooltipHook.Widget())
            _closeTooltip();

        break;

    default:
        break;
    }
}

//____ _onCloneContent() _________________________________________________

void WgTooltipLayer::_onCloneContent(const WgWidget * _pOrg)
{
    
}


//____ _childRequestResize() ______________________________________________

void WgTooltipLayer::_childRequestResize(TooltipHook* pHook)
{
    if( ((WgHook*)pHook) == &m_baseHook )
        _requestResize();
    else
        _updateTooltipGeo();
}

//____ _firstLayerHook() ___________________________________________________

WgLayerHook * WgTooltipLayer::_firstLayerHook() const
{
    if( m_tooltipHook.Widget() )
        return const_cast<TooltipHook*>(&m_tooltipHook);
    else
        return nullptr;
}

//____ _firstHook() ________________________________________________________

WgHook * WgTooltipLayer::_firstHook() const
{
    if (m_baseHook.Widget())
        return const_cast<BaseHook*>(&m_baseHook);
    else if( m_tooltipHook.Widget() )
        return const_cast<TooltipHook*>(&m_tooltipHook);
    else
        return nullptr;
}

//____ _lastHook() _________________________________________________________

WgHook * WgTooltipLayer::_lastHook() const
{
    if( m_tooltipHook.Widget() )
        return const_cast<TooltipHook*>(&m_tooltipHook);
    else if (m_baseHook.Widget())
        return const_cast<BaseHook*>(&m_baseHook);
    else
        return nullptr;
}

//____ _firstHookWithGeo() ____________________________________________________

WgHook * WgTooltipLayer::_firstHookWithGeo(WgRect& geo) const
{
    if (m_baseHook.Widget())
    {
        geo = WgRect(0, 0, m_size);
        return const_cast<BaseHook*>(&m_baseHook);
    }
    else if( m_tooltipHook.Widget() )
    {
        geo = m_tooltipHook.m_geo;
        return const_cast<TooltipHook*>(&m_tooltipHook);
    }
    
    return nullptr;
}

//____ _nextHookWithGeo() _______________________________________________________

WgHook * WgTooltipLayer::_nextHookWithGeo(WgRect& geo, WgHook * pHook) const
{
    WgHook * p = pHook->Next();
    if (p)
        geo = ((TooltipHook*)p)->m_geo;

    return p;
}

//____ _lastHookWithGeo() ________________________________________________________

WgHook * WgTooltipLayer::_lastHookWithGeo(WgRect& geo) const
{
    if( m_tooltipHook.Widget() )
    {
        geo = m_tooltipHook.m_geo;
        return const_cast<TooltipHook*>(&m_tooltipHook);
    }
    else if (m_baseHook.Widget())
    {
        geo = WgRect(0, 0, m_size);
        return const_cast<BaseHook*>(&m_baseHook);
    }
    
    return nullptr;
}

//____ _prevHookWithGeo() ________________________________________________________

WgHook * WgTooltipLayer::_prevHookWithGeo(WgRect& geo, WgHook * pHook) const
{
    WgHook * p = pHook->Prev();
    if (p)
        geo = ((TooltipHook*)p)->m_geo;

    return p;
}


//____ _defaultTooltipGenerator() ____________________________________________

WgWidget * WgTooltipLayer::_defaultTooltipGenerator(Placement& placement, const WgWidget* pHovered, const WgBorders& margin)
{
	BoxSkin::Blueprint bp;
	bp.color = Color::LightYellow;
	bp.outlineColor = Color::Black;
	bp.outlineThickness = 1;
	bp.padding = 2;
	
	BoxSkin_p pSkin = BoxSkin::create( bp );
    auto pWidget = new WgTextDisplay();
    pWidget->SetSkin(pSkin);

    pWidget->SetText(pHovered->GetTooltipString());
    return pWidget;
}

