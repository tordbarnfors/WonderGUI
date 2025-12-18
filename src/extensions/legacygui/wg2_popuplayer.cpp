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
#include <wg2_popuplayer.h>
#include <wg2_util.h>
#include <wg2_patches.h>
#include <wg2_panel.h>
#include <wg2_base.h>
#include <wg2_gfxdevice.h>
#include <wg2_eventhandler.h>
#include <wg2_popupopener.h>

#include <algorithm>


static const char	c_widgetType[] = { "PopupLayer" };
static const char	c_hookType[] = { "PopupHook" };
static const char	c_basehookType[] = { "PopupLayerBasehook" };


//_____________________________________________________________________________
WgPopupLayer* WgPopupHook::Parent() const
{
	return m_pParent;
}


//_____________________________________________________________________________
const char * WgPopupHook::Type(void) const
{
	return ClassType();
}

//_____________________________________________________________________________
const char * WgPopupHook::ClassType()
{
	return c_hookType;
}


//_____________________________________________________________________________
void WgPopupHook::_requestRender()
{
	m_pParent->_onRequestRender(m_geo, this);
}

//_____________________________________________________________________________
void WgPopupHook::_requestRender(const WgRect& rect)
{
	m_pParent->_onRequestRender(rect + m_geo.pos(), this);
}

//_____________________________________________________________________________
void WgPopupHook::_requestResize()
{
	m_pParent->_childRequestResize(this);
}

//_____________________________________________________________________________
WgHook * WgPopupHook::_prevHook() const
{
	WgPopupHook * p = _prev();

	if (p)
		return p;
	else if (m_pParent->m_baseHook.Widget())
		return &m_pParent->m_baseHook;
	else
		return 0;
}

//_____________________________________________________________________________
WgHook * WgPopupHook::_nextHook() const
{
	WgPopupHook * p = _next();

	// We have multiple inheritance, so lets make the cast in a safe way, preserving NULL-pointer as NULL.

	if (p)
		return p;
	else
		return 0;
}

//_____________________________________________________________________________
WgContainer * WgPopupHook::_parent() const
{
	return m_pParent;
}


//____ Constructor ____________________________________________________________

WgPopupLayer::WgPopupLayer()
{
}

//____ Destructor _____________________________________________________________

WgPopupLayer::~WgPopupLayer()
{
	// In contrast to all other panels we only delete our base child on exit.
	// Menus don't belong to us, we just display them, so they are not ours to delete.

	WgPopupHook * pHook = m_popupHooks.first();
	while (pHook)
	{
		pHook->_releaseWidget();
		pHook = pHook->_next();
	}
	
	// Need to do this since children might be deleted recursively and make _requestRender calls back to us.
	
	_removeSlots(m_popupHooks.size());	
}

//____ Type() _________________________________________________________________

const char *WgPopupLayer::Type(void) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgPopupLayer::GetClass()
{
	return c_widgetType;
}

//____ NbPopups() _____________________________________________________________

int WgPopupLayer::NbPopups() const
{
	return m_popupHooks.size();
}

//____ Push() _________________________________________________________________

void WgPopupLayer::Push(WgWidget * pPopup, WgWidget * pOpener, const WgRect& launcherGeo, WgOrigo attachPoint, WgCoord attachOfs, bool bAutoClose, bool bDelay, WgSize maxSize)
{
	if (!m_popupHooks.isEmpty())
		_closeAllOpenUntil(pOpener, true);

	_addSlot(pPopup, pOpener, launcherGeo, attachPoint, attachOfs, bAutoClose, bDelay, maxSize);
}

//____ Pop() __________________________________________________________________

void WgPopupLayer::Pop(int nb)
{
	if (nb <= 0)
		return;

	nb = std::min(nb, m_popupHooks.size());

	_removeSlots(nb);
}

//____ Pop() __________________________________________________________________

void WgPopupLayer::Pop(WgWidget * pPopup)
{
	int index = 0;
	auto p = m_popupHooks.first();

	while (p && p->m_pWidget != pPopup)
	{
		p = p->Next();
		index++;
	}


	if (p)
		_removeSlots(index + 1);

}

//____ Clear() ________________________________________________________________

void WgPopupLayer::Clear()
{
	if (m_popupHooks.isEmpty())
		return;

	_removeSlots(m_popupHooks.size());
}


//____ _updateGeo() __________________________________________________________

bool WgPopupLayer::_updateGeo(WgPopupHook* pSlot, bool bInitialUpdate )
{
	// Get size of parent and correct launcherGeo


	//

	WgRect geo(0,0,WgSize::min(pSlot->m_pWidget->PreferredPixelSize(),WgSize::min(pSlot->maxSize,m_size)));

	switch( pSlot->attachPoint )
	{
		case WgOrigo::NorthEast:					// Right side of launcherGeo, going down.
		{
			geo.x = pSlot->launcherGeo.right();
			geo.y = pSlot->launcherGeo.top();
			break;
		}

		case WgOrigo::SouthEast:					// Right side of launcherGeo, going up.
		{
			geo.x = pSlot->launcherGeo.right();
			geo.y = pSlot->launcherGeo.bottom() - geo.h;
			break;
		}

		case WgOrigo::NorthWest:					// Left-aligned above launcher.
		{
			geo.x = pSlot->launcherGeo.left();
			geo.y = pSlot->launcherGeo.top() - geo.h;
			break;
		}

		case WgOrigo::SouthWest:					// Left-aligned below launcher.
		{
			geo.x = pSlot->launcherGeo.left();
			geo.y = pSlot->launcherGeo.bottom();
			break;
		}

		case WgOrigo::West:						// Centered left of launcherGeo.
		{
			geo.x = pSlot->launcherGeo.left() - geo.w;
			geo.y = pSlot->launcherGeo.top() + pSlot->launcherGeo.h/2 - geo.h/2;
			break;
		}

		case WgOrigo::North:						// Centered above launcherGeo.
		{
			geo.x = pSlot->launcherGeo.left() + pSlot->launcherGeo.w/2 - geo.w/2;
			geo.y = pSlot->launcherGeo.top() - geo.h;
			break;
		}

		case WgOrigo::East:						// Centered right of launcherGeo.
		{
			geo.x = pSlot->launcherGeo.right();
			geo.y = pSlot->launcherGeo.top() + pSlot->launcherGeo.h/2 - geo.h/2;
			break;
		}

		default:
		{
			assert(false);
		}

		case WgOrigo::South:						// Centered below launcherGeo.
		{
			geo.x = pSlot->launcherGeo.left() + pSlot->launcherGeo.w/2 - geo.w/2;
			geo.y = pSlot->launcherGeo.bottom();
			break;
		}

	}

	// Adjust with offset.

	geo.x += pSlot->attachOfs.x * m_scale / WG_SCALE_BASE;
	geo.y += pSlot->attachOfs.y * m_scale / WG_SCALE_BASE;

	// Adjust geometry to fit inside parent.

	if( geo.right() > m_size.w )
	{
		if( geo.left() == pSlot->launcherGeo.right() )
		{
			if( pSlot->launcherGeo.left() > m_size.w - pSlot->launcherGeo.right() )
			{
				geo.x = pSlot->launcherGeo.left() - geo.w;
				if( geo.x < 0 )
				{
					geo.x = 0;
					geo.w = pSlot->launcherGeo.left();
				}
			}
			else
				geo.w = m_size.w - geo.x;
		}
		else
			geo.x = m_size.w - geo.w;
	}

	if( geo.left() < 0 )
	{
		if( geo.right() == pSlot->launcherGeo.left() )
		{
			if( pSlot->launcherGeo.left() < m_size.w - pSlot->launcherGeo.right() )
			{
				geo.x = pSlot->launcherGeo.right();
				if( geo.right() > m_size.w )
					geo.w = m_size.w - geo.x;
			}
			else
			{
				geo.x = 0;
				geo.w = pSlot->launcherGeo.left();
			}

		}
		else
			geo.x = 0;
	}

	if( geo.bottom() > m_size.h )
	{
		if( geo.top() == pSlot->launcherGeo.bottom() )
		{
			if( pSlot->launcherGeo.top() > m_size.h - pSlot->launcherGeo.bottom() )
			{
				geo.y = pSlot->launcherGeo.top() - geo.h;
				if( geo.y < 0 )
				{
					geo.y = 0;
					geo.h = pSlot->launcherGeo.top();
				}
			}
			else
				geo.h = m_size.h - geo.y;
		}
		else
			geo.y = m_size.h - geo.h;
	}

	if( geo.top() < 0 )
	{
		if( geo.bottom() == pSlot->launcherGeo.top() )
		{
			if( pSlot->launcherGeo.top() < m_size.h - pSlot->launcherGeo.bottom() )
			{
				geo.y = pSlot->launcherGeo.bottom();
				if( geo.bottom() > m_size.h )
					geo.h = m_size.h - geo.y;
			}
			else
			{
				geo.y = 0;
				geo.h = pSlot->launcherGeo.bottom();
			}
		}
		else
			geo.y = 0;
	}

	// Update geometry if it has changed.

	if( bInitialUpdate || geo != pSlot->m_geo )
	{
		if( !bInitialUpdate )
			_onRequestRender(pSlot->m_geo,pSlot);
		pSlot->m_geo = geo;
		_onRequestRender(pSlot->m_geo,pSlot);

//		if( pSlot->m_pWidget->PixelSize() != geo.Size() ) This check doesn't work in WG2, since it asks the Hook for the size...
			pSlot->m_pWidget->_onNewSize(geo.size());

		return true;
	}
	else
		return false;
}


//____ FindWidget() ____________________________________________________________

WgWidget *  WgPopupLayer::FindWidget( const WgCoord& ofs, WgSearchMode mode )
{
	// MenuPanel has its own _findWidget() method since we need special treatment of
	// searchmode ACTION_TARGET when a menu is open.

	if( mode == WgSearchMode::ActionTarget && !m_popupHooks.isEmpty() )
	{
		// In search mode ACTION_TARGET we limit our target to us, our menu-branches and the menu-opener if a menu is open.

		WgPopupHook * pSlot = m_popupHooks.first();
		WgWidget * pResult = 0;

		while( pSlot && !pResult )
		{
			if( pSlot->m_geo.contains( ofs ) )
			{
				if( pSlot->m_pWidget->IsContainer() )
					pResult = static_cast<WgContainer*>(pSlot->m_pWidget)->FindWidget( ofs - pSlot->m_geo.pos(), mode );
				else if( pSlot->m_pWidget->MarkTest( ofs - pSlot->m_geo.pos() ) )
					pResult = pSlot->m_pWidget;
			}

			if( pSlot->state == WgPopupHook::State::Opening || pSlot->state == WgPopupHook::State::PeekOpen || pSlot->state == WgPopupHook::State::ClosingDelay )
				pSlot = nullptr;
			else
				pSlot = pSlot->Next();
		}

		if( pResult == 0 )
		{
			// Check the root opener

			WgPopupHook * pSlot = m_popupHooks.last();
			if( pSlot->pOpener )
			{
				WgWidget * pOpener = pSlot->pOpener.GetRealPtr();

				WgCoord 	absPos 		= ofs + ScreenPixelPos();
				WgRect	openerGeo 	= pOpener->ScreenPixelGeo();

				if( openerGeo.contains(absPos) && pOpener->MarkTest( absPos - openerGeo.pos() ) )
					pResult = pOpener;
			}

			// Fall back to us.

			if( pResult == 0 )
				pResult = this;
		}
		return pResult;
	}
	else
	{
		// For the rest of the modes we can rely on the default method.

		return WgContainer::FindWidget( ofs, mode );
	}
}


//____ _SetDelays() ___________________________________________________________

void WgPopupLayer::SetDelays(int popupOpenDelay, int popupFadeInDelay, int popupCloseDelay, int popupFadeOutDelay)
{
	wg::limit(popupOpenDelay, 0, 2000);
	wg::limit(popupFadeInDelay, 0, 2000);
	wg::limit(popupCloseDelay, 0, 2000);
	wg::limit(popupFadeOutDelay, 0, 2000);

	m_openingDelayMs = popupOpenDelay;
	m_openingFadeMs = popupFadeInDelay;
	m_closingFadeMs = popupCloseDelay;
	m_closingDelayMs = popupFadeOutDelay;
}


//____ _onRequestRender() _____________________________________________________

void WgPopupLayer::_onRequestRender( const WgRect& rect, const WgPopupHook * pSlot )
{
	// Don not render anything if not visible

	if (pSlot && pSlot->state == WgPopupHook::State::OpeningDelay)
		return;

	// Clip our geometry and put it in a dirtyrect-list

	WgPatches patches;
	patches.add( WgRect::overlap( rect, WgRect(0,0,m_size)) );

	// Remove portions of dirty rect that are covered by opaque upper siblings,
	// possibly filling list with many small dirty rects instead.

	if( !m_popupHooks.isEmpty() )
	{
		WgPopupHook * pCover;

		if (pSlot)
			pCover = pSlot->Prev();
		else
			pCover = m_popupHooks.last();

		while (pCover)
		{
			if (pCover->m_geo.isOverlapping(rect) && pCover->state != WgPopupHook::State::OpeningDelay && pCover->state != WgPopupHook::State::Opening && pCover->state != WgPopupHook::State::Closing)
				pCover->m_pWidget->_onMaskPatches(patches, pCover->m_geo, WgRect(0, 0, INT_MAX, INT_MAX), _getBlendMode());

			pCover = pCover->Prev();
		}
	}
	// Make request render calls

	for( const WgRect * pRect = patches.begin() ; pRect < patches.end() ; pRect++ )
		_requestRender( * pRect );
}

//____ _renderPatches() ___________________________________________________

class WidgetRenderContext
{
public:
	WidgetRenderContext() : pSlot(0) {}
	WidgetRenderContext(WgPopupHook * pSlot, const WgRect& geo) : pSlot(pSlot), geo(geo) {}

	WgPopupHook *	pSlot;
	WgRect			geo;
	WgPatches		patches;
};

void WgPopupLayer::_renderPatches(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches)
{

	// We start by eliminating dirt outside our geometry

	WgPatches 	patches(_pPatches->size());								// TODO: Optimize by pre-allocating?

	for (const WgRect * pRect = _pPatches->begin(); pRect != _pPatches->end(); pRect++)
	{
		if (_canvas.isOverlapping(*pRect))
			patches.push(WgRect::overlap(*pRect, _canvas));
	}

	if( patches.isEmpty() )
		return;
	
	// Render container itself

	int bytesToRelease = _convertAndPushClipList( pDevice, patches.size(), patches.begin() );
	_onRender(pDevice, _canvas, _window);


	// Render children

	WgRect	dirtBounds = patches.bounds();

	// Create WidgetRenderContext's for popups that might get dirty patches

	std::vector<WidgetRenderContext> renderList;

	auto pSlot = m_popupHooks.first();

	while (pSlot != nullptr )
	{
		WgRect geo = pSlot->m_geo + _canvas.pos();

		if (geo.isOverlapping(dirtBounds) && pSlot->state != WgPopupHook::State::OpeningDelay)
			renderList.push_back(WidgetRenderContext(pSlot, geo));

		pSlot = pSlot->Next();
	}

	// Go through WidgetRenderContexts, push and mask dirt

	for (unsigned int i = 0; i < renderList.size(); i++)
	{
		WidgetRenderContext * p = &renderList[i];

		p->patches.push(&patches);
		if( p->pSlot->state != WgPopupHook::State::Opening && p->pSlot->state != WgPopupHook::State::Closing )
			p->pSlot->m_pWidget->_onMaskPatches(patches, p->geo, p->geo, pDevice->blendMode());		//TODO: Need some optimizations here, grandchildren can be called repeatedly! Expensive!

		if (patches.isEmpty())
			break;
	}

	// Any dirt left in patches is for base child, lets render that first


	if (!patches.isEmpty())
		m_baseHook.Widget()->_renderPatches(pDevice, _canvas, _window, &patches);


	// Go through WidgetRenderContexts and render the patches in reverse order (topmost popup rendered last).

	for (int i = int(renderList.size()) - 1; i >= 0; i--)
	{
		WidgetRenderContext * p = &renderList[i];

		WgColor tint = WgColor::White;

		if (p->pSlot->state == WgPopupHook::State::Opening)
			tint.a = 255 * p->pSlot->stateCounter / m_openingFadeMs;

		if (p->pSlot->state == WgPopupHook::State::Closing)
			tint.a = 255 - (255 * p->pSlot->stateCounter / m_closingFadeMs);

		if (tint.a == 255)
			p->pSlot->m_pWidget->_renderPatches(pDevice, p->geo, p->geo, &p->patches);
		else if( !p->patches.isEmpty() )
		{
			WgColor oldTint = pDevice->tintColor();
			pDevice->setTintColor(oldTint*tint);
			p->pSlot->m_pWidget->_renderPatches(pDevice, p->geo, p->geo, &p->patches);
			pDevice->setTintColor(oldTint);
		}
	}
	_popAndReleaseClipList( pDevice, bytesToRelease );
}


//____ _maskPatches() _____________________________________________________

//	void WgPopupLayer::_maskPatches(Patches& patches, const Rect& geo, const Rect& clip, BlendMode blendMode)
//	{
//		Need to except children that are in states OpeningDelay, Opening and Closing.
//	}


//____ _onNewSize() ___________________________________________________________

void WgPopupLayer::_onNewSize( const WgSize& sz )
{
	WgLayer::_onNewSize(sz);
}

//____ _onCloneContent() ______________________________________________________

void WgPopupLayer::_onCloneContent( const WgWidget * _pOrg )
{
	WgLayer::_onCloneContent( (WgContainer*) _pOrg );
}

//____ _onEvent() ______________________________________________________________

void WgPopupLayer::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	switch( pEvent->Type() )
	{
		case WG_EVENT_TICK:
		{
			// Update state for all open popups

			int ms = static_cast<const WgEvent::Tick*>(pEvent)->Millisec();

			WgPopupHook * pHook = m_popupHooks.first();
			while (pHook)
			{
				WgPopupHook * pNext = pHook->_next();

				switch (pHook->state)
				{
				case WgPopupHook::State::OpeningDelay:
					if (pHook->stateCounter + ms < m_openingDelayMs)
					{
						pHook->stateCounter += ms;
						break;
					}
					else
					{
						pHook->state = WgPopupHook::State::Opening;
						pHook->stateCounter -= m_openingDelayMs;
						pHook->bOpened = true;
						// No break here, let's continue down to opening...
					}
				case WgPopupHook::State::Opening:
					pHook->stateCounter += ms;
					if (pHook->stateCounter >= m_openingFadeMs)
					{
						pHook->stateCounter = 0;
						pHook->state = pHook->bAutoClose ? WgPopupHook::State::PeekOpen : WgPopupHook::State::FixedOpen;
					}
					_onRequestRender(pHook->m_geo, pHook);
					break;

				case WgPopupHook::State::ClosingDelay:
					if (pHook->stateCounter + ms < m_closingDelayMs)
					{
						pHook->stateCounter += ms;
						break;
					}
					else
					{
						pHook->state = WgPopupHook::State::Closing;
						pHook->stateCounter -= m_closingDelayMs;
						// No break here, let's continue down to closing...
					}
				case WgPopupHook::State::Closing:
					pHook->stateCounter += ms;

					if (pHook->stateCounter >= m_closingFadeMs)
						_removeSlot(pHook);
					else
						_onRequestRender(pHook->m_geo, pHook);

					break;
				default:
					break;
				}

				pHook = pNext;
			}

			// Close any popup that is due for closing.

//			while (!m_popupHooks.IsEmpty() && m_popupHooks.First()->state == WgPopupHook::State::Closing && m_popupHooks.First()->stateCounter >= m_closingFadeMs)
//				_removeSlots(1);
		}
		break;

		case WG_EVENT_MOUSE_ENTER:
		case WG_EVENT_MOUSE_MOVE:
		{
			if (m_popupHooks.isEmpty())
				break;

			WgCoord 	pointerPos = pEvent->PointerPixelPos() - ScreenPixelPos();

			// Popups that are of type 'auto closing' needs special attention

			WgPopupHook * pHook = m_popupHooks.first();
			if (pHook && pHook->bAutoClose )
			{
				// Promote popup in state PeekOpen to state WeakOpen if pointer has entered its geo.

				if( pHook->state == WgPopupHook::State::PeekOpen)
				{
					if (pHook->m_geo.contains(pointerPos))
						pHook->state = WgPopupHook::State::WeakOpen;
				}

				// Close PeekOpen popups (and popups that are about to become PeekOpen) if pointer has left launcherGeo.

				if ( pHook->state == WgPopupHook::State::PeekOpen || pHook->state == WgPopupHook::State::Opening || pHook->state == WgPopupHook::State::OpeningDelay )
				{
					if (!pHook->launcherGeo.contains(pointerPos))
						_beginClosing(pHook, false);
				}
			}



			// A popup in state ClosingDelay should be promoted to
			// state PeekOpen if pointer has entered its launcherGeo and
			// to state WeakOpen if pointer has entered its geo.
			// Promoting to WeakOpen Should also promote any ancestor also in state ClosingDelay.


			pHook = m_popupHooks.first();
			while (pHook)
			{
				if (pHook->state == WgPopupHook::State::ClosingDelay)
				{
					if (pHook->launcherGeo.contains(pointerPos))
					{
						pHook->state = WgPopupHook::State::PeekOpen;
						pHook->stateCounter = 0;
					}
					else if (pHook->m_geo.contains(pointerPos))
					{
						WgPopupHook * p = pHook;
						while (p && p->state == WgPopupHook::State::ClosingDelay)
						{
							p->state = WgPopupHook::State::WeakOpen;
							p->stateCounter = 0;
							p = p->_next();
						}
						break;		// Nothing more to do further down.
					}
				}
				pHook = pHook->_next();
			}

			// If pointer has entered a selectable widget
			// and all widgets between them have bAutoClose=true, they should all enter
			// state ClosingDelay (unless already in state Closing).

			WgWidget * pMarked = FindWidget(pointerPos, WgSearchMode::ActionTarget);

			if (pMarked != this && (pMarked->IsSelectable() && m_popupHooks.first()->bAutoClose))
				_closeAutoOpenedUntil(pMarked, false);

		}
		break;


		case WG_EVENT_MOUSE_LEAVE:
		{
			if( !pEvent->ForwardedFrom() )
				_closeAutoOpenedUntil(this, false);
/*
			// Top popup can be in state PeekOpen, which should begin closing when
			// pointer has left.

			PopupSlot * pSlot = m_popups.first();
			if (pSlot && pSlot->state == PopupSlot::State::PeekOpen)
			{
				pSlot->state = PopupSlot::State::ClosingDelay;
				pSlot->stateCounter = 0;
			}
 */
		}
		break;


		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			if (m_popupHooks.isEmpty())
				break;					// Popup was removed already on the press.

			// Allow us to release the mouse within opener without closing any popups

			WgPopupHook * pSlot = m_popupHooks.first();
			if (pSlot->pOpener)
			{
				WgWidget * pOpener = pSlot->pOpener.GetRealPtr();

				WgCoord absPos = static_cast<const WgEvent::MouseButtonRelease*>(pEvent)->PointerPixelPos();
				WgRect	openerGeo = pOpener->ScreenPixelGeo();

				if (pOpener->MarkTest(absPos - openerGeo.pos()))
					break;
			}

			// DON'T BREAK! Continuing down to case MousePress on purpose.
		}
		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			if (m_popupHooks.isEmpty())
				break;

			auto pEv = static_cast<const WgEvent::MouseButtonPress*>(pEvent);

			auto pSource = static_cast<WgWidget*>(pEvent->ForwardedFrom() );
			if (!pSource || pSource == this )
				_removeSlots(m_popupHooks.size());
			else if (pSource->IsSelectable())
			{
				pHandler->QueueEvent(new WgEvent::Selected(pSource));

				_removeSlots(m_popupHooks.size());
			}
			return;
		}
		break;

		case WG_EVENT_KEY_PRESS:
		case WG_EVENT_KEY_REPEAT:
		{
			auto pEv = static_cast<const WgEvent::KeyEvent*>(pEvent);

			if( pEv->TranslatedKeyCode() == WgKey::WG_KEY_ESCAPE )
			{
				if( !m_popupHooks.isEmpty() )
				{
					_removeSlots(1);
					return;
				}
			}
		}
		break;

		default:
			break;
	}

	WgLayer::_onEvent(pEvent, pHandler);
}

//____ _closeAutoOpenedUntil() _________________________________________________

void WgPopupLayer::_closeAutoOpenedUntil(WgWidget * pStayOpen, bool bCloseImmediately)
{
	// Follow pStayOpen up the hierarchy to one of our popups or null.

	while (pStayOpen != nullptr && pStayOpen->Parent() != this)
		pStayOpen = pStayOpen->Parent();

	// Remove all children ontop of pStayOpen, which is now either a child of ours or null, in which case
	// all will be removed.

	auto p = m_popupHooks.first();
	while (p && p->bAutoClose && p->m_pWidget != pStayOpen)
	{
		_beginClosing(p, bCloseImmediately);
		p = p->_next();
	}
}

//____ _closeAllOpenUntil() _________________________________________________

void WgPopupLayer::_closeAllOpenUntil(WgWidget * pStayOpen, bool bCloseImmediately)
{
	// Follow pStayOpen up the hierarchy to one of our popups or null.

	while (pStayOpen != nullptr && pStayOpen->Parent() != this)
		pStayOpen = pStayOpen->Parent();

	// Remove all children ontop of pStayOpen, which is now either a child of ours or null, in which case
	// all will be removed.

	auto p = m_popupHooks.first();
	while (p && p->m_pWidget != pStayOpen)
	{
		_beginClosing(p, bCloseImmediately);
		p = p->_next();
	}
}


//____ _beginClosing() _____________________________________________________

void WgPopupLayer::_beginClosing( WgPopupHook * p, bool bCloseImmediately )
{
	switch (p->state)
	{
		case WgPopupHook::State::OpeningDelay:
			p->state = WgPopupHook::State::Closing;
			p->stateCounter = m_closingFadeMs;
			break;
		case WgPopupHook::State::Opening:
			p->state = WgPopupHook::State::Closing;
			p->stateCounter = p->stateCounter * m_closingFadeMs / m_openingFadeMs;
			break;
		case WgPopupHook::State::Closing:
			break;
		case WgPopupHook::State::ClosingDelay:
			if( bCloseImmediately )
			{
				p->state = WgPopupHook::State::Closing;
				p->stateCounter = 0;
			}
			break;
		default:
			p->state = bCloseImmediately ? WgPopupHook::State::Closing : WgPopupHook::State::ClosingDelay;
			p->stateCounter = 0;
	}
}



//____ _stealKeyboardFocus() _________________________________________________

void WgPopupLayer::_stealKeyboardFocus()
{

	// Verify that we have a root

	if( !Hook() || !Hook()->Root() )
		return;

	// Save old keyboard focus, which we assume belonged to previous menu in hierarchy.

	if( m_popupHooks.size() < 2 )
		m_pKeyFocus = _eventHandler()->KeyboardFocus();
	else
		m_popupHooks.get(1)->pKeyFocus = _eventHandler()->KeyboardFocus();

	// Steal keyboard focus to top menu

	WgWidget * pWidget = m_popupHooks.first()->m_pWidget;

	pWidget->GrabFocus();

}

//____ _restoreKeyboardFocus() _________________________________________________

void WgPopupLayer::_restoreKeyboardFocus()
{

	// Verify that we have a root

	if (!Hook() || !Hook()->Root())
		return;

	//

	if (m_popupHooks.isEmpty())
	{
		if( m_pKeyFocus )
		m_pKeyFocus->GrabFocus();
	}
	else
		 m_popupHooks.first()->m_pWidget->GrabFocus();

}

//____ _childRequestResize() _______________________________________________

void WgPopupLayer::_childRequestResize(WgPopupHook * pHook)
{
	if( ((WgHook*)pHook) == &m_baseHook )
		_requestResize();
	else
		_updateGeo( pHook );
}

//____ _addSlot() ____________________________________________________________

void WgPopupLayer::_addSlot(WgWidget * _pPopup, WgWidget * _pOpener, const WgRect& _launcherGeo, WgOrigo _attachPoint, WgCoord _attachOfs, bool _bAutoClose, bool bDelay, WgSize _maxSize)
{
	WgPopupHook * pHook = new WgPopupHook(this);
	pHook->_attachWidget(_pPopup);
	m_popupHooks.pushFront(pHook);

	pHook->pOpener = _pOpener;
	pHook->launcherGeo = _launcherGeo;
	pHook->attachPoint = _attachPoint;
	pHook->attachOfs = _attachOfs;
	pHook->bAutoClose = _bAutoClose;
	pHook->state = bDelay ? WgPopupHook::State::OpeningDelay : WgPopupHook::State::Opening;
	pHook->bOpened = !bDelay;
	pHook->stateCounter = 0;
	pHook->maxSize = _maxSize;

	_updateGeo(pHook, true);
	_stealKeyboardFocus();

	if( m_popupHooks.size() == 1 )
		_startReceiveTicks();
}


//____ _removeSlots() __________________________________________________

void WgPopupLayer::_removeSlots(int nb)
{
	WgEventHandler * pEH = _eventHandler();

	nb = std::min(nb, m_popupHooks.size());

	WgPopupHook * pHook = (WgPopupHook *) m_popupHooks.first();


	for (int i = 0; i < nb; i++)
	{
		WgPopupHook * p = pHook;
		pHook = pHook->Next();

		if (pEH)
			pEH->QueueEvent(new WgEvent::PopupClosed(p->Widget(), p->pOpener));

		if( p->bOpened )
			p->_requestRender();

		p->_releaseWidget();
		delete p;
	}
	_restoreKeyboardFocus();

	if (m_popupHooks.isEmpty() )
		_stopReceiveTicks();
}


//____ _removeSlot() __________________________________________________

void WgPopupLayer::_removeSlot(WgPopupHook * p)
{
	WgEventHandler * pEH = _eventHandler();

	if (pEH)
		pEH->QueueEvent(new WgEvent::PopupClosed(p->Widget(), p->pOpener));

	if( p->bOpened )
		p->_requestRender();
	p->_releaseWidget();
	delete p;

	_restoreKeyboardFocus();

	if (m_popupHooks.isEmpty())
		_stopReceiveTicks();
}



//____ _firstLayerHook() ______________________________________________________

WgLayerHook * WgPopupLayer::_firstLayerHook() const
{
	return m_popupHooks.first();
}

//____ _firstHook() ___________________________________________________________

WgHook* WgPopupLayer::_firstHook() const
{
	if (m_baseHook.Widget())
		return const_cast<BaseHook*>(&m_baseHook);
	else
		return m_popupHooks.first();
}

//____ _lastHook() ____________________________________________________________

WgHook* WgPopupLayer::_lastHook() const
{
	auto p =m_popupHooks.last();
	if( p )
		return p;
	if (m_baseHook.Widget())
		return const_cast<BaseHook*>(&m_baseHook);
	return nullptr;
}

//____ _firstHookWithGeo() _____________________________________________________

WgHook * WgPopupLayer::_firstHookWithGeo(WgRect& geo) const
{
	if (m_baseHook.Widget())
	{
		geo = WgRect(0, 0, m_size);
		return const_cast<BaseHook*>(&m_baseHook);
	}
	else
	{
		WgPopupHook * p = m_popupHooks.first();
		if (p)
			geo = p->m_geo;

		return p;
	}
}

//____ _nextHookWithGeo() _______________________________________________________

WgHook * WgPopupLayer::_nextHookWithGeo(WgRect& geo, WgHook * pHook) const
{
	WgHook * p = pHook->Next();
	if (p)
		geo = ((WgPopupHook*)p)->m_geo;

	return p;
}

//____ _lastHookWithGeo() _____________________________________________________

WgHook * WgPopupLayer::_lastHookWithGeo(WgRect& geo) const
{
	WgPopupHook * p = m_popupHooks.last();
	if (p)
	{
		geo = p->m_geo;
		return p;
	}
	else if (m_baseHook.Widget())
	{
		geo = WgRect(0, 0, m_size);
		return const_cast<BaseHook*>(&m_baseHook);
	}
	else
		return 0;
}

//____ _prevHookWithGeo() _______________________________________________________

WgHook * WgPopupLayer::_prevHookWithGeo(WgRect& geo, WgHook * pHook) const
{
	WgHook * p = pHook->Prev();
	if (p)
		geo = p->PixelGeo();

	return p;
}
