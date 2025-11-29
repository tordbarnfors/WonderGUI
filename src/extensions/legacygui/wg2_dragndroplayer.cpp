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
#include <wg2_dragndroplayer.h>
#include <wg2_base.h>
#include <wg2_gfxdevice.h>
#include <wg_surfacefactory.h>
#include <wg2_patches.h>
#include <wg2_image.h>
#include <wg2_eventhandler.h>


static const char    c_widgetType[] = { "DragNDropLayer" };
static const char    c_hookType[] = { "DragNDropHook" };


//_____________________________________________________________________________
WgDragNDropLayer* WgDragNDropHook::Parent() const
{
	return m_pParent;
}


//_____________________________________________________________________________
const char * WgDragNDropHook::Type(void) const
{
	return ClassType();
}

//_____________________________________________________________________________
const char * WgDragNDropHook::ClassType()
{
	return c_hookType;
}

//_____________________________________________________________________________
void WgDragNDropHook::_requestResize()
{
	m_geo.setSize(m_pWidget->PreferredPixelSize());
	m_pWidget->_onNewSize(m_geo);

	//TODO: Should we request render on old and new geometry here, or is that already taken care of?
}

//_____________________________________________________________________________
WgHook * WgDragNDropHook::_prevHook() const
{
	if (m_pParent->m_baseHook.Widget())
		return &m_pParent->m_baseHook;
	else
		return 0;
}

//_____________________________________________________________________________
WgHook * WgDragNDropHook::_nextHook() const
{
	return 0;
}

//_____________________________________________________________________________
WgContainer * WgDragNDropHook::_parent() const
{
	return m_pParent;
}


//____ Constructor ____________________________________________________________

WgDragNDropLayer::WgDragNDropLayer() : m_dragHook(this)
{
	_startReceiveTicks();
}

//____ Destructor _____________________________________________________________

WgDragNDropLayer::~WgDragNDropLayer()
{
	_stopReceiveTicks();
}

//____ Type() _________________________________________________________________

const char *WgDragNDropLayer::Type(void) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgDragNDropLayer::GetClass()
{
	return c_widgetType;
}

//____ FindWidget() __________________________________________________________

WgWidget * WgDragNDropLayer::FindWidget( const WgCoord& ofs, WgSearchMode mode )
{
    auto pEvHandler = _eventHandler();
    if( pEvHandler && pEvHandler->isPointerLocked() )
    {
        //TODO: Should make sure widget is a descendant of us.
        
        return pEvHandler->LockedPointerWidget();
    }
    
    // Widget being dragged is totally transparent to the mouse, so we just
	// forward to our child.

	WgWidget * pWidget = m_baseHook.Widget();
	if( pWidget )
	{
		if (pWidget->IsContainer())
		{
			WgWidget * pRes = static_cast<WgContainer*>(pWidget)->FindWidget(ofs, mode);
			if (pRes)
				return pRes;
		}
		else if (mode == wg::SearchMode::Geometry || pWidget->MarkTest(ofs))
			return pWidget;
	}
	else if( mode == wg::SearchMode::Geometry )
		return this;

	return nullptr;
}

//____ Pick() _________________________________________________________________

bool WgDragNDropLayer::Pick( WgWidget * pWidget, WgCoord pickOfs, bool bHidePointer )
{
	// Pick is not allowed if a widget already is picked.

	if( m_dragState != DragState::Idle && m_dragState != DragState::Picking )
		return false;

	// Verify that widget is descendant of us

	WgWidget * p = pWidget;
	while( p != this )
	{
		if( p == nullptr )
			return false;                       // Not a descendant of ours.

		p = p->Parent();
	}

	// Convert pickOfs from points to pixels.

	pickOfs.x = pickOfs.x * m_scale / WG_SCALE_BASE;
	pickOfs.y = pickOfs.y * m_scale / WG_SCALE_BASE;

	// Pick the widget.

	m_pPicked = pWidget;
	m_pickCategory = pWidget->pickCategory();

    auto pEvent = new WgEvent::DropPick(pWidget, pickOfs, this);
    pEvent->setHidePointer(bHidePointer);
	_eventHandler()->QueueEvent( pEvent );
	m_dragState = DragState::Picked;
    m_pLockedDropHoverTarget = nullptr;

	return true;
}


//____ _onRequestRender() _______________________________________________

void WgDragNDropLayer::_onRequestRender(const WgRect& rect, const WgLayerHook * pHook)
{
	// We don't mask against drag widget, it is assumed to be too small/transparent/irregular
	// for that to make sense.

	// Clip our geometry and pass it on

	_requestRender( WgRect::overlap( rect, WgRect(0,0,m_size) ));
}

//____ _firstLayerHook() ____________________________________________________________

WgLayerHook * WgDragNDropLayer::_firstLayerHook() const
{
	if( m_dragHook.Widget() )
		return const_cast<WgDragNDropHook*>(&m_dragHook);
	return nullptr;
}

//____ _firstHook() ____________________________________________________________

WgHook * WgDragNDropLayer::_firstHook() const
{
	if (m_baseHook.Widget())
		return const_cast<BaseHook*>(&m_baseHook);
	else if( m_dragHook.Widget() )
			return const_cast<WgDragNDropHook*>(&m_dragHook);
	return nullptr;
}

//____ _lastHook() ____________________________________________________________

WgHook * WgDragNDropLayer::_lastHook() const
{
	if( m_dragHook.Widget() )
		return const_cast<WgDragNDropHook*>(&m_dragHook);
	else if (m_baseHook.Widget())
		return const_cast<BaseHook*>(&m_baseHook);
	return nullptr;
}

//____ _firstHookWithGeo() ____________________________________________________________

WgHook * WgDragNDropLayer::_firstHookWithGeo(WgRect& geo) const
{
	if (m_baseHook.Widget())
	{
		geo = WgRect(0, 0, m_size);
		return const_cast<BaseHook*>(&m_baseHook);
	}
	else if( m_dragHook.Widget() )
	{
		geo = m_dragHook.m_geo;
		return const_cast<WgDragNDropHook*>(&m_dragHook);
	}
	else
		return nullptr;
}

//____ _nextHookWithGeo() ____________________________________________________________

WgHook * WgDragNDropLayer::_nextHookWithGeo(WgRect& geo, WgHook * pHook) const
{
	if( pHook == &m_baseHook && m_dragHook.Widget() )
	{
		geo = m_dragHook.m_geo;
		return const_cast<WgDragNDropHook*>(&m_dragHook);
	}
	else
		return nullptr;
}

//____ _lastHookWithGeo() ____________________________________________________________

WgHook * WgDragNDropLayer::_lastHookWithGeo(WgRect& geo) const
{
	if( m_dragHook.Widget() )
	{
		geo = m_dragHook.m_geo;
		return const_cast<WgDragNDropHook*>(&m_dragHook);
	}
	else if (m_baseHook.Widget())
	{
		geo = WgRect(0, 0, m_size);
		return const_cast<BaseHook*>(&m_baseHook);
	}
	else
		return nullptr;

}

//____ _prevHookWithGeo() ____________________________________________________________

WgHook * WgDragNDropLayer::_prevHookWithGeo(WgRect& geo, WgHook * pHook) const
{
	if( pHook == &m_dragHook && m_baseHook.Widget() )
	{
		geo = WgRect(0,0, m_size);
		return const_cast<BaseHook*>(&m_baseHook);
	}
	else
		return nullptr;

}

//____ _onCloneContent() ____________________________________________________

void WgDragNDropLayer::_onCloneContent( const WgWidget * _pOrg )
{
	WgLayer::_onCloneContent( (WgContainer*) _pOrg );
}

//____ _onNewSize() ___________________________________________________________

void WgDragNDropLayer::_onNewSize( const WgSize& sz )
{
	WgLayer::_onNewSize(sz);
}

//____ _onEvent() ____________________________________________________________

void WgDragNDropLayer::_onEvent(const WgEvent::Event * _pEvent, WgEventHandler * pHandler)
{
	switch (_pEvent->Type())
	{
		case WG_EVENT_TICK:
		{
			switch( m_dragState )
			{
				case DragState::Dragging:
				{
					// Check if we entered/left a (possible) target.

					WgCoord ofs = _pEvent->PointerPixelPos();

					WgWidget * pProbed = FindWidget(ofs, wg::SearchMode::ActionTarget );

					while (pProbed && pProbed != this && !pProbed->isDropTarget())
						pProbed = pProbed->Parent();

					if( pProbed && pProbed != this && pProbed != m_pProbed.GetRealPtr() )
					{
						m_pProbed = pProbed;
						pHandler->QueueEvent(new WgEvent::DropProbe(pProbed, m_pickCategory, m_pPayload, m_pPicked.GetRealPtr(), this ));
					}
					break;
				}
				case DragState::Targeting:
				{
					// Check if our target has changed

					WgCoord ofs = _pEvent->PointerPixelPos();

					WgWidget * pHovered = FindWidget(ofs, wg::SearchMode::ActionTarget );

					while (pHovered && pHovered != this && !pHovered->isDropTarget())
						pHovered = pHovered->Parent();

					if( pHovered != m_pTargeted.GetRealPtr() )
					{
						// Untarget previous target. Probing possibly new target we leave for next round.

						if( m_pTargeted )
							pHandler->QueueEvent(new WgEvent::DropLeave(m_pTargeted.GetRealPtr(), m_pickCategory, m_pPayload, m_pPicked.GetRealPtr() ));

						m_pTargeted = nullptr;
						m_dragState = DragState::Dragging;
					}

					// Send move messages to targeted widget

					if( m_pTargeted )                                   // Check our weak pointer just in case it has been deleted...
					{
						WgCoord targetOfs = _pEvent->PointerScreenPixelPos() - m_pTargeted->ScreenPixelPos();
						if( targetOfs != m_targetOfs )
						{
							m_targetOfs = targetOfs;
							pHandler->QueueEvent(new WgEvent::DropMove(m_pTargeted.GetRealPtr(), m_pickCategory, m_pPayload, m_pPicked.GetRealPtr(), m_dragHook.m_pWidget, m_dragWidgetOfs, m_bDeleteDraggedWhenDone, this ));
						}
					}
					break;
				}
				default:
					break;

			}


			break;
		}

		case WG_EVENT_MOUSEBUTTON_DRAG:
		{
			auto pEvent = static_cast<const WgEvent::MouseButtonDrag*>(_pEvent);
			if (pEvent->Button() != 1)
			{
				WgLayer::_onEvent( _pEvent, pHandler );
				break;
			}

			switch (m_dragState)
			{
				case DragState::Picking:
				{
					WgCoord total = pEvent->DraggedTotalPixels();
					if (abs(total.x) + abs(total.y) > m_dragStartTreshold)
					{
						WgCoord pickOfs = pEvent->StartPixelPos();
						pHandler->QueueEvent(new WgEvent::DropPick(m_pPicked.GetRealPtr(), pickOfs, this));
						m_dragState = DragState::Picked;
                        m_pLockedDropHoverTarget = nullptr;
					}
					break;
				}

				case DragState::Dragging:
				{
					_updateDropHovered( pEvent->PointerPixelPos() );

					// Move the drag-widget onscreen.

					_requestRender(m_dragHook.m_geo);
					m_dragHook.m_geo.setPos( pEvent->PointerPixelPos() + m_dragWidgetOfs );
					_requestRender(m_dragHook.m_geo);

					break;
				}

				case DragState::Targeting:
				{
					_updateDropHovered( pEvent->PointerPixelPos() );

					// Move the drag-widget onscreen.

					_requestRender(m_dragHook.m_geo);
					m_dragHook.m_geo.setPos( pEvent->PointerPixelPos() + m_dragWidgetOfs );
					_requestRender(m_dragHook.m_geo);

					break;
				}

				default:
				{
					WgLayer::_onEvent( _pEvent, pHandler );
					break;
				}
			}

			break;
		}

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			auto pEvent = static_cast<const WgEvent::MouseButtonPress*>(_pEvent);
			if (pEvent->Button() != 1 || m_dragState != DragState::Idle || pEvent->Widget() == nullptr )
			{
				WgLayer::_onEvent( _pEvent, pHandler );
				break;
			}
			WgCoord ofs = pEvent->PointerPixelPos();
			WgWidget * pSource = FindWidget(ofs, wg::SearchMode::ActionTarget);

			while (pSource && pSource != this && !pSource->isPickable())
				pSource = pSource->Parent();

			if (pSource && pSource != this && pSource->isPickable())
			{
				m_pPicked = pSource;
				m_pickCategory = pSource->pickCategory();
				m_dragState = DragState::Picking;
			}
			else
				WgLayer::_onEvent( _pEvent, pHandler );

			break;
		}

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			auto pEvent = static_cast<const WgEvent::MouseButtonRelease*>(_pEvent);
			if( pEvent->Button() != 1)
			{
				WgLayer::_onEvent( _pEvent, pHandler );
				break;
			}
			switch (m_dragState)
			{
				case Picking:
				{
					m_pPicked = nullptr;
					m_dragState = DragState::Idle;
					break;
				}
				case Picked:
				{
					assert(false);		// There should be no mouse release msg before our DropPickMsg is returned.
				}
				case Dragging:
				{
					_cancel();
					break;
				}
				case Targeting:
				{
					if( m_pTargeted )
					{
						pHandler->QueueEvent(new WgEvent::DropDeliver(m_pTargeted.GetRealPtr(), m_pickCategory, m_pPayload, m_pPicked.GetRealPtr(), this));
						m_dragState = DragState::Delivering;
						pHandler->QueueEvent(new WgEvent::DropLeave(m_pTargeted.GetRealPtr(), m_pickCategory, m_pPayload, m_pPicked.GetRealPtr()));
						m_pTargeted = nullptr;
					}
					else
						_cancel();
					break;
				}
				default:
					WgLayer::_onEvent( _pEvent, pHandler );
					break;
			}
			break;
		}

		case WG_EVENT_DROP_PICK:
		{
			auto pEvent = static_cast<const WgEvent::DropPick*>(_pEvent);

			assert(m_dragState == DragState::Picked);

			if (pEvent->hasPayload() && m_dragState == DragState::Picked )
			{
				// Set payload

				m_pPayload = pEvent->payload();

                // Possibly hide pointer
                
                if( pEvent->isHidingPointer() )
                {
                    m_bPointerHidden = true;
                    _eventHandler()->HidePointer();
                }
                
                
				// Set/generate drag widget (widget actually dragged across the screen)

				auto pDragWidget = pEvent->dragWidget();
				WgSize    dragWidgetSize;

				if (pDragWidget)
				{
					_replaceWidgetInHook(pDragWidget);

					// Ugly hack needed for AmpRoom to force a refresh of DragWidget geometry.
					// Apparently the background image has f-cked  up geometry and I haven't investigated why. /Tord

					if( m_scale == 4096 )
					{
						pDragWidget->_setScale(8192);
						pDragWidget->_setScale(4096);
					}

					dragWidgetSize = pDragWidget->PreferredPixelSize();
					m_dragWidgetOfs = pEvent->dragWidgetPointerOfs();
					m_bDeleteDraggedWhenDone = pEvent->deleteDragWidgetWhenDone();
				}
				else
				{
					m_dragWidgetOfs = WgCoord(0,0) - pEvent->pickOfs();
					dragWidgetSize = m_pPicked->PixelSize();                             //TODO: Possible source of error, if picked widget changes size before the render call.
					m_bDeleteDraggedWhenDone = true;
				}

				WgCoord mousePos = Abs2localPixel(pEvent->PointerScreenPixelPos());
				m_dragHook.m_geo = { mousePos + m_dragWidgetOfs, dragWidgetSize };

				if (pDragWidget)
				{
//					pDragWidget->_onNewSize(dragWidgetSize);
				}

				_requestRender(m_dragHook.m_geo);
				m_dragState = DragState::Dragging;
			}
			else
				_cancel();

			break;
		}

		case WG_EVENT_DROP_PROBE:
		{
			auto pEvent= static_cast<const WgEvent::DropProbe*>(_pEvent);

			// Check if our probe was acccepted and in that case start targeting.

			if( pEvent->isAccepted() )
			{
				WgWidget * pTargeted = pEvent->Widget();

				pHandler->QueueEvent(new WgEvent::DropEnter(pTargeted, m_pickCategory, m_pPayload, m_pPicked.GetRealPtr(), m_dragHook.m_pWidget, m_dragWidgetOfs, m_bDeleteDraggedWhenDone, this));

				m_pProbed = nullptr;
				m_pTargeted = pTargeted;
				m_targetOfs = _pEvent->PointerScreenPixelPos() - m_pTargeted->ScreenPixelPos();
				m_dragState = DragState::Targeting;
			}

			break;
		}

		case WG_EVENT_DROP_MOVE:
		{
			auto pEvent = static_cast<const WgEvent::DropMove*>(_pEvent);

			// Check if we need to change drag widget

			if( pEvent->dragWidget() != m_dragHook.m_pWidget )
				_replaceDragWidget(pEvent->dragWidget(),pEvent->deleteDragWidgetWhenDone());
			break;
		}

		case WG_EVENT_DROP_ENTER:
		{
			auto pEvent = static_cast<const WgEvent::DropEnter*>(_pEvent);

			// Check if we need to change drag widget

			if( pEvent->dragWidget() != m_dragHook.m_pWidget )
				_replaceDragWidget(pEvent->dragWidget(),pEvent->deleteDragWidgetWhenDone());
			break;
		}

		case WG_EVENT_DROP_DELIVER:
		{
			auto pEvent = static_cast<const WgEvent::DropDeliver*>(_pEvent);

			// Check if our delivery was accepted and in that case complete, otherwise cancel.

			if( pEvent->isAccepted() )
				_complete( pEvent->deliveredTo() );
			else
				_cancel();

			break;
		}

		default:
			WgLayer::_onEvent( _pEvent, pHandler );
			break;
	}

}

//____ _cancel() ________________________________________________________________

void WgDragNDropLayer::_cancel()
{
	_clearDropHovered();

	if( m_dragHook.Widget() )
	{
		_requestRender(m_dragHook.m_geo);
		_replaceWidgetInHook(nullptr);
	}

	if( m_pTargeted )
	{
		_eventHandler()->QueueEvent(new WgEvent::DropLeave(m_pTargeted.GetRealPtr(), m_pickCategory, m_pPayload, m_pPicked.GetRealPtr()));
		m_pTargeted = nullptr;
	}

    _eventHandler()->QueueEvent(new WgEvent::DropCancel(m_pPicked.GetRealPtr(), m_pickCategory, nullptr, m_pPicked));
    m_pPicked = nullptr;
	m_pPayload = nullptr;
	m_dragState = DragState::Idle;
    
    if( m_bPointerHidden )
        _eventHandler()->ShowPointer();
}

//____ _complete() _______________________________________________________________

void WgDragNDropLayer::_complete( WgWidget * pDeliveredTo )
{
	assert( !m_pTargeted );

	_clearDropHovered();

	if( m_dragHook.m_pWidget )
	{
		_requestRender(m_dragHook.m_geo);
		_replaceWidgetInHook(nullptr);
	}

	_eventHandler()->QueueEvent(new WgEvent::DropComplete(m_pPicked.GetRealPtr(), pDeliveredTo, m_pickCategory, m_pPayload));
	m_pPicked = nullptr;
	m_pPayload = nullptr;
	m_dragState = DragState::Idle;

    if( m_bPointerHidden )
        _eventHandler()->ShowPointer();
}

//____ _replaceDragWidget() ______________________________________________________

void WgDragNDropLayer::_replaceDragWidget( WgWidget * pNewWidget, bool bDeleteWhenDone )
{
	WgSize newSize = pNewWidget->PreferredPixelSize();
	WgSize maxSize = WgSize::max(m_dragHook.m_geo.size(),newSize);

    WgWidget * pOld = m_dragHook.m_pWidget;
	_replaceWidgetInHook( pNewWidget );
	m_dragHook.m_geo.setSize(newSize);
    _requestRender(maxSize);

    if( pOld && m_bDeleteDraggedWhenDone )
        delete pOld;
    
    m_bDeleteDraggedWhenDone = bDeleteWhenDone;
    
}


//____ _renderPatches() __________________________________________________________

void WgDragNDropLayer::_renderPatches(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches)
{
	// Generate drag widget as an image of picked widget if missing and needed.

	if( m_dragState == DragState::Dragging && !m_dragHook.m_pWidget )
	{
		WgSize sz = m_pPicked->PixelSize();

		auto pCanvas = m_pPicked->Screenshot({});
		auto pImage = new WgImage();
		pImage->SetImage( pCanvas );
		pImage->_onNewSize(sz);

		_replaceWidgetInHook(pImage);
	}

	WgLayer::_renderPatches(pDevice,_canvas,_window, _pPatches);
}

//____ _replaceWidgetInHook() _________________________________________________

void WgDragNDropLayer::_replaceWidgetInHook(WgWidget * pNewWidget)
{
	WgWidget * pOldWidget = m_dragHook._releaseWidget();
	if( pOldWidget && m_bDeleteDraggedWhenDone )
		delete pOldWidget;
	if( pNewWidget )
		m_dragHook._attachWidget(pNewWidget);
//    _requestRender( m_dragHook.m_geo );
}

//____ _widgetPosInList() ________________________________________________________

int WgDragNDropLayer::_widgetPosInList( const WgWidget * pWidget, const std::vector<WgWidgetWeakPtr>& list )
{
	for( size_t i = 0 ; i < list.size() ; i++ )
		if( list[i].GetRealPtr() == pWidget )
			return (int) i;

	return -1;
}

//____ _updateDropHovered() _____________________________________________________

void WgDragNDropLayer::_updateDropHovered( WgCoord hoverPos )
{

	WgWidget * pMarkedWidget = m_pLockedDropHoverTarget ? m_pLockedDropHoverTarget.GetRealPtr() : FindWidget(hoverPos, wg::SearchMode::ActionTarget);

	// Loop through our new widgets and check if they already
	// were entered. Send DropHoverEnter to all new widgets and notice the first
	// common ancestor .

	for( WgWidget * pWidget = pMarkedWidget ; pWidget != 0 ; pWidget = pWidget->Parent() )
	{
		int ofs = _widgetPosInList( pWidget, m_vHoveredInside );
		if( ofs >= 0 )
		{
			_queueEvent( new WgEvent::DropHoverMove( m_vHoveredInside[ofs].GetRealPtr(), m_pickCategory, m_pPayload, m_pPicked.GetRealPtr() ) );
			m_vHoveredInside[ofs] = 0;
		}
		else
			_queueEvent( new WgEvent::DropHoverEnter( pWidget, m_pickCategory, m_pPayload, m_pPicked.GetRealPtr() ) );
	}

	// Send DropHoverLeave to those that were left.

	for( size_t i = 0 ; i < m_vHoveredInside.size() ; i++ )
		if(m_vHoveredInside[i] )
			_queueEvent( new WgEvent::DropHoverLeave( m_vHoveredInside[i].GetRealPtr(), m_pickCategory, m_pPayload, m_pPicked.GetRealPtr() ) );

	// Replace the old list with a new one.

	m_vHoveredInside.clear();
	for( WgWidget * pWidget = pMarkedWidget ; pWidget != 0 ; pWidget = pWidget->Parent() )
		m_vHoveredInside.push_back( pWidget );
}

//____ _clearDropHovered() _____________________________________________________

void WgDragNDropLayer::_clearDropHovered()
{
	for( size_t i = 0 ; i < m_vHoveredInside.size() ; i++ )
		_queueEvent( new WgEvent::DropHoverLeave( m_vHoveredInside[i].GetRealPtr(), m_pickCategory, m_pPayload, m_pPicked.GetRealPtr() ) );

	m_vHoveredInside.clear();
}

