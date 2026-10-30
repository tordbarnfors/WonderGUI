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
#include <wg_popupoverlay.h>
#include <wg_util.h>
#include <wg_patches.h>
#include <wg_msgrouter.h>
#include <wg_panel.h>
#include <wg_base.h>
#include <wg_inputhandler.h>
#include <wg_staticslotvector.impl.h>
#include <wg_gfxdevice.h>

#include <algorithm>
#include <cstdlib>

namespace wg
{
	using namespace Util;
	template class StaticSlotVector<PopupOverlay::Slot>;

	const TypeInfo PopupOverlay::TYPEINFO = { "PopupOverlay", &Overlay::TYPEINFO };
	const TypeInfo PopupOverlay::Slot::TYPEINFO = { "PopupOverlay::Slot", &Overlay::Slot::TYPEINFO };

	//____ isInTriangle() ________________________________________________________

	// Points on an edge count as inside.

	static bool isInTriangle( const CoordSPX& point, const CoordSPX& a, const CoordSPX& b, const CoordSPX& c )
	{
		// Which side of the line from-to that point is on, as the sign of their cross product.

		auto side = [&point]( const CoordSPX& from, const CoordSPX& to )
		{
			return int64_t(to.x - from.x) * (point.y - from.y) - int64_t(to.y - from.y) * (point.x - from.x);
		};

		int64_t side1 = side(a, b);
		int64_t side2 = side(b, c);
		int64_t side3 = side(c, a);

		bool bNegative = side1 < 0 || side2 < 0 || side3 < 0;
		bool bPositive = side1 > 0 || side2 > 0 || side3 > 0;

		return !(bNegative && bPositive);
	}


	//____ pushFront() ________________________________________________

	void PopupOverlay::MySlots::pushFront(const Widget_p& pPopup, Widget * pOpener, const Rect& launcherGeo, Placement attachPoint, bool bPeek, bool bCloseOnSelect, Size maxSize, Border overflow, bool bTakeFocus )
	{
		int scale = _holder()->_scale();

		_pushFront(pPopup, pOpener, align(ptsToSpx(launcherGeo, scale)), attachPoint, bPeek, bCloseOnSelect, align(ptsToSpx(maxSize,scale)), align(ptsToSpx(overflow,scale)), bTakeFocus);
	}

	//____ pop() ________________________________________________

	void PopupOverlay::MySlots::pop(int nb)
	{
		if( nb <= 0 )
			return;

		nb = std::min(nb, size());

		_holder()->_removeSlots(0,nb);
	}

	//____ pop() ________________________________________________

	void PopupOverlay::MySlots::pop(Widget * pPopup)
	{
		int i = index(pPopup);
		if (i > -1)
			_holder()->_removeSlots(0,i+1);
	}

	//____ clear() ________________________________________________

	void PopupOverlay::MySlots::clear()
	{
		if( isEmpty() )
			return;

		_holder()->_removeSlots(0,size());
	}

	//____ _pushFront() ________________________________________________

	void PopupOverlay::MySlots::_pushFront(const Widget_p& pPopup, Widget* pOpener, const RectSPX& launcherGeo, Placement attachPoint, bool bPeek, bool bCloseOnSelect, SizeSPX maxSize, BorderSPX overflow, bool bTakeFocus)
	{
		pPopup->releaseFromParent();
		_holder()->_addSlot(pPopup, pOpener, launcherGeo, attachPoint, bPeek, bCloseOnSelect, maxSize, overflow, bTakeFocus);
	}


	//____ constructor ____________________________________________________________

	PopupOverlay::PopupOverlay() : popupSlots(this)
	{
	}

	//____ Destructor _____________________________________________________________

	PopupOverlay::~PopupOverlay()
	{
		Base::inputHandler()->clearStayEnteredList(intptr_t(this));
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PopupOverlay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _updateGeo() __________________________________________________________

	void PopupOverlay::_updateGeo(Slot* pSlot, bool bDoResize)
	{
		// Get size of parent and correct launcherGeo


		//

		RectSPX geo(0,0,SizeSPX::min(pSlot->_widget()->_defaultSize(m_scale),SizeSPX::min(pSlot->m_maxSize,m_size)));

		geo -= pSlot->m_popupOverflow;	// Remove overflow for correct placement of content, will be added back afterwards.

		switch( pSlot->m_attachPoint )
		{
			case Placement::NorthEast:					// Right side of launcherGeo, going down.
			{
				geo.x = pSlot->m_launcherGeo.right();
				geo.y = pSlot->m_launcherGeo.top();
				break;
			}

			case Placement::SouthEast:					// Right side of launcherGeo, going up.
			{
				geo.x = pSlot->m_launcherGeo.right();
				geo.y = pSlot->m_launcherGeo.bottom() - geo.h;
				break;
			}

			default:
			case Placement::NorthWest:					// Left-aligned above launcher.
			{
				geo.x = pSlot->m_launcherGeo.left();
				geo.y = pSlot->m_launcherGeo.top() - geo.h;
				break;
			}

			case Placement::SouthWest:					// Left-aligned below launcher.
			{
				geo.x = pSlot->m_launcherGeo.left();
				geo.y = pSlot->m_launcherGeo.bottom();
				break;
			}

			case Placement::West:						// Centered left of launcherGeo.
			{
				geo.x = pSlot->m_launcherGeo.left() - geo.w;
				geo.y = align( pSlot->m_launcherGeo.top() + pSlot->m_launcherGeo.h / 2 -geo.h / 2 );
				break;
			}

			case Placement::North:						// Centered above launcherGeo.
			{
				geo.x = align( pSlot->m_launcherGeo.left() + pSlot->m_launcherGeo.w/2 - geo.w/2 );
				geo.y = pSlot->m_launcherGeo.top() - geo.h;
				break;
			}

			case Placement::East:						// Centered right of launcherGeo.
			{
				geo.x = pSlot->m_launcherGeo.right();
				geo.y = align( pSlot->m_launcherGeo.top() + pSlot->m_launcherGeo.h/2 - geo.h/2 );
				break;
			}

			case Placement::South:						// Centered below launcherGeo.
			{
				geo.x = align( pSlot->m_launcherGeo.left() + pSlot->m_launcherGeo.w/2 - geo.w/2 );
				geo.y = pSlot->m_launcherGeo.bottom();
				break;
			}
		}

		// Adjust geometry to fit inside parent.

		if( geo.right() > m_size.w )
		{
			if( geo.left() == pSlot->m_launcherGeo.right() )
			{
				if( pSlot->m_launcherGeo.left() > m_size.w - pSlot->m_launcherGeo.right() )
				{
					geo.x = pSlot->m_launcherGeo.left() - geo.w;
					if( geo.x < 0 )
					{
						geo.x = 0;
						geo.w = pSlot->m_launcherGeo.left();
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
			if( geo.right() == pSlot->m_launcherGeo.left() )
			{
				if( pSlot->m_launcherGeo.left() < m_size.w - pSlot->m_launcherGeo.right() )
				{
					geo.x = pSlot->m_launcherGeo.right();
					if( geo.right() > m_size.w )
						geo.w = m_size.w - geo.x;
				}
				else
				{
					geo.x = 0;
					geo.w = pSlot->m_launcherGeo.left();
				}

			}
			else
				geo.x = 0;
		}

		if( geo.bottom() > m_size.h )
		{
			if( geo.top() == pSlot->m_launcherGeo.bottom() )
			{
				if( pSlot->m_launcherGeo.top() > m_size.h - pSlot->m_launcherGeo.bottom() )
				{
					geo.y = pSlot->m_launcherGeo.top() - geo.h;
					if( geo.y < 0 )
					{
						geo.y = 0;
						geo.h = pSlot->m_launcherGeo.top();
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
			if( geo.bottom() == pSlot->m_launcherGeo.top() )
			{
				if( pSlot->m_launcherGeo.top() < m_size.h - pSlot->m_launcherGeo.bottom() )
				{
					geo.y = pSlot->m_launcherGeo.bottom();
					if( geo.bottom() > m_size.h )
						geo.h = m_size.h - geo.y;
				}
				else
				{
					geo.y = 0;
					geo.h = pSlot->m_launcherGeo.bottom();
				}
			}
			else
				geo.y = 0;
		}

		// Adding back overflow. Doesn't matter if this goes outside parent.

		geo += pSlot->m_popupOverflow;

		// Update geometry if it has changed.

		if (pSlot->_geo().size() != geo.size() || pSlot->_widget()->scale() != m_scale )
			bDoResize = true;


		if (geo != pSlot->m_geo)
		{
			BorderSPX overflow = pSlot->_widget()->_overflow();

			if( !pSlot->m_geo.isEmpty() )
				_onRequestRender(pSlot->m_geo + overflow, pSlot);
			pSlot->m_geo = geo;
			_onRequestRender(pSlot->m_geo + overflow, pSlot);
		}

		if (bDoResize)
			pSlot->_setSize(geo, m_scale);

	}

	//____ _refreshStayEnteredList() _____________________________________________

	void PopupOverlay::_refreshStayEnteredList()
	{

		if( popupSlots.isEmpty() )
			Base::inputHandler()->clearStayEnteredList(intptr_t(this));
		else
		{
			std::vector<Widget*> stayEnteredWidgets;

			for( auto& slot : popupSlots )
			{
				if( slot.m_state != Slot::State::Closing  && slot.m_state != Slot::State::ClosingDelay )
				{
					Widget * pWidget = slot.m_pOpener;

					while( pWidget != this && pWidget != nullptr )
					{
						stayEnteredWidgets.push_back(pWidget);
						pWidget = pWidget->parent();
					}
				}
			}

			Base::inputHandler()->setStayEnteredList(intptr_t(this), stayEnteredWidgets.data(), stayEnteredWidgets.data() + stayEnteredWidgets.size() );
		}

 }


	//____ _slotTypeInfo() ________________________________________________________

	const TypeInfo&	PopupOverlay::_slotTypeInfo(const StaticSlot * pSlot) const
	{
		if (pSlot == &mainSlot)
			return DynamicSlot::TYPEINFO;

		return Slot::TYPEINFO;
	}


	//____ _findWidget() ____________________________________________________________

	Widget * PopupOverlay::_findWidget( const CoordSPX& ofs, SearchMode mode )
	{
		Widget * pResult = _findWidgetIgnoringAim(ofs, mode);

		// Safe triangle: a widget that the pointer only passes on its way to a submenu
		// shouldn't be marked, or it would light up, and an opener would open its own
		// submenu. The opener of the submenu stays marked instead, see _isAimingAt().

		if( mode == SearchMode::ActionTarget && pResult )
		{
			Slot * pTarget = _aimTarget(pResult);
			if( pTarget )
			{
				int64_t timestamp = Base::timestamp();
				int64_t hoveredSince = (pResult == m_pAimHovered.rawPtr()) ? m_aimHoveredTimestamp : timestamp;

				if( _isAimingAt(pTarget, ofs, timestamp, hoveredSince) )
				{
					m_bAimRedirected = true;
					return pTarget->m_pOpener.rawPtr();
				}
			}
		}

		return pResult;
	}

	//____ _findWidgetIgnoringAim() ___________________________________________________

	Widget *  PopupOverlay::_findWidgetIgnoringAim( const CoordSPX& ofs, SearchMode mode )
	{
		// PopupOverlay has its own _findWidget() method since we need special treatment of
		// searchmode ACTION_TARGET when a menu is open.

		if( mode == SearchMode::ActionTarget && !popupSlots.isEmpty() && m_bBlockLeftMouseButton == true )
		{
			// In search mode ACTION_TARGET we limit our target to us, our menu-branches and the menu-opener if a menu is open.

			Slot * pSlot = popupSlots._begin();
			Widget * pResult = 0;

			while( pSlot != popupSlots._end() && !pResult )
			{
				if( pSlot->m_geo.contains( ofs ) )
				{
					if( pSlot->_widget()->isContainer() )
						pResult = static_cast<Container*>(pSlot->_widget())->_findWidget( ofs - pSlot->m_geo.pos(), mode );
					else if( pSlot->_widget()->_markTest( ofs - pSlot->m_geo.pos() ) )
						pResult = pSlot->_widget();

				}

				// We only allow actions on popups below if our popup is in peek-state.

				if( !pSlot->m_bPeek )
					break;

				pSlot++;
			}

			if( pResult == 0 )
			{
				// Check the root opener

				Slot * pSlot = popupSlots._last();
				if( pSlot->m_pOpener && pSlot->m_launcherGeo.contains(ofs) )
				{
					Widget * pOpener = pSlot->m_pOpener.rawPtr();

					CoordSPX	openerLocalPos = pOpener->_toLocal(_toGlobal(ofs));
					RectSPX		openerGeo 	= RectSPX(0,0,pOpener->_size());

					if( openerGeo.contains(openerLocalPos) && pOpener->_markTest( openerLocalPos ) )
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

			return Container::_findWidget( ofs, mode );
		}
	}

	//____ _onRequestRender() _____________________________________________________

	void PopupOverlay::_onRequestRender( const RectSPX& rect, const Overlay::Slot * pSlot )
	{
		// Don not render anything if not visible

		if (pSlot && ((Slot*)pSlot)->m_state == Slot::State::OpeningDelay)
			return;

		// Clip our geometry and put it in a dirtyrect-list

		PatchesSPX patches;
		patches.add( RectSPX::overlap( rect, RectSPX(0,0,m_size)) );

		// Remove portions of dirty rect that are covered by opaque upper siblings,
		// possibly filling list with many small dirty rects instead.

		if( !popupSlots.isEmpty() )
		{
			Slot * pCover;

			if (pSlot)
				pCover = ((Slot*)pSlot) - 1;
			else
				pCover = popupSlots._last();

			while (pCover >= popupSlots._begin())
			{
				if ((pCover->_widget()->_renderBounds() + pCover->m_geo.pos()).isOverlapping(rect) && pCover->m_state != Slot::State::OpeningDelay && pCover->m_state != Slot::State::Opening && pCover->m_state != Slot::State::Closing)
					pCover->_widget()->_maskPatches(patches, pCover->m_geo, RectSPX(0, 0, spx_max, spx_max));

				pCover--;
			}
		}
		// Make request render calls

		for( const RectSPX * pRect = patches.begin() ; pRect < patches.end() ; pRect++ )
			_requestRender( * pRect );
	}

	//____ _render() ___________________________________________________


	void PopupOverlay::_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		class WidgetRenderContext
		{
		public:
			WidgetRenderContext() : pSlot(0) {}
			WidgetRenderContext(PopupOverlay::Slot * pSlot, const RectSPX& geo) : pSlot(pSlot), geo(geo) {}

			PopupOverlay::Slot *	pSlot;
			RectSPX		geo;
			ClipPopData clipPop;
		};


		// Render container itself

		m_skin.render(pDevice, _canvas, m_scale, m_state);

		// Render children

		RectSPX	dirtBounds = pDevice->clipBounds();

		// Create WidgetRenderContext's for popups that might get dirty patches

		std::vector<WidgetRenderContext> renderList;

		for( auto pSlot = popupSlots._begin() ; pSlot != popupSlots._end() ; pSlot++ )
		{
			RectSPX geo = pSlot->m_geo + _canvas.pos();
			RectSPX renderBounds = pSlot->_widget()->_renderBounds() + geo.pos();

			if (renderBounds.isOverlapping(dirtBounds) && pSlot->m_state != Slot::State::OpeningDelay)
				renderList.push_back(WidgetRenderContext(pSlot, geo));
		}

		// Collect dirty patches from gfxDevice

		int nClipRects = pDevice->clipListSize();
		auto pClipRects = pDevice->clipList();
		PatchesSPX patches( nClipRects );

		for( int i = 0 ; i < nClipRects ; i++ )
			patches.push(pClipRects[i]);

		// Go through WidgetRenderContexts, push and mask dirt

		for (unsigned int i = 0; i < renderList.size(); i++)
		{
			WidgetRenderContext * p = &renderList[i];

			p->clipPop = patchesToClipList(pDevice, p->pSlot->_widget()->_renderBounds() + p->geo.pos(), patches);
			if( p->pSlot->m_state != Slot::State::Opening && p->pSlot->m_state != Slot::State::Closing )
				p->pSlot->_widget()->_maskPatches(patches, p->geo, p->geo);		//TODO: Need some optimizations here, grandchildren can be called repeatedly! Expensive!

			if (patches.isEmpty())
				break;
		}

		// Any dirt left in patches is for base child, lets render that first

		if ( mainSlot._widget() && !patches.isEmpty())
		{
			ClipPopData popData = patchesToClipList(pDevice, _window, patches);
			mainSlot._widget()->_render(pDevice, _canvas, _window);
			popClipList(pDevice, popData);
		}

		// Go through WidgetRenderContexts and render the patches in reverse order (topmost popup rendered last).

		for (int i = int(renderList.size()) - 1; i >= 0; i--)
		{
			WidgetRenderContext * p = &renderList[i];

			HiColor tint = HiColor::White;

			if (p->pSlot->m_state == Slot::State::Opening && m_openingFadeMs > 0 )
				tint.a = 4096 * p->pSlot->m_stateCounter / m_openingFadeMs;

			if (p->pSlot->m_state == Slot::State::Closing && m_closingFadeMs > 0 )
				tint.a = 4096 - (4096 * p->pSlot->m_stateCounter / m_closingFadeMs);

			if (tint.a == 4096)
				p->pSlot->_widget()->_render(pDevice, p->geo, p->geo);
			else
			{
				HiColor oldTint = pDevice->tintColor();
				pDevice->setTintColor(oldTint*tint);
				p->pSlot->_widget()->_render(pDevice, p->geo, p->geo);
				pDevice->setTintColor(oldTint);
			}
			popClipList(pDevice,p->clipPop);
		}
	}


	//____ _maskPatches() _____________________________________________________

//	void PopupOverlay::_maskPatches(Patches& patches, const RectI& geo, const RectI& clip)
//	{
//		Need to except children that are in states OpeningDelay, Opening and Closing.
//	}


	//____ _resize() ___________________________________________________________

	void PopupOverlay::_resize( const SizeSPX& sz, int scale )
	{
		Overlay::_resize(sz, scale);
	}

	//____ _receive() ______________________________________________________________

	void PopupOverlay::_receive( Msg * _pMsg )
	{

		Overlay::_receive(_pMsg);

		switch( _pMsg->type() )
		{
			case MsgType::MouseEnter:
			case MsgType::MouseMove:
			{
				auto pMsg = static_cast<InputMsg*>(_pMsg);
				CoordSPX 	pointerPos = _toLocal(pMsg->pointerSpxPos());

				_addAimSample(pointerPos, pMsg->timestamp());

				if (popupSlots.isEmpty())
					break;

				_processPointer(pointerPos, pMsg->timestamp());
			}
			break;

			case MsgType::MouseLeave:
			{
				// No moves reach us while the pointer is outside, so aiming starts over when
				// it comes back. Popups are left open.

				m_nbAimSamples = 0;
				m_pAimHovered = nullptr;
			}
			break;

/*
			case MsgType::MouseLeave:
			{
				// Top popup can be in state PeekOpen, which should begin closing when
				// pointer has left.

				Slot * pSlot = m_popups.first();
				if (pSlot && pSlot->m_state == Slot::State::PeekOpen)
				{
					pSlot->m_state = Slot::State::ClosingDelay;
					pSlot->m_stateCounter = 0;
				}
			}
			break;
*/

			case MsgType::MouseRelease:
			{
				// Mouse release is used for selecting in popups.

				if (popupSlots.isEmpty())
					break;					// Popup was removed already on the press.

				// Allow us to release the mouse within opener without closing any popups

				Slot * pSlot = popupSlots._first();
				Widget * pOpener = pSlot->m_pOpener.rawPtr();

				if (pOpener)
				{
					Coord 	pos = pOpener->toLocal(static_cast<MouseReleaseMsg*>(_pMsg)->pointerPos());

					if (pOpener->markTest(pos))
						break;
				}

				//

//				auto pSource = static_cast<Widget*>(_pMsg->originalSource().rawPtr());

				// What is released on is what is under the pointer, even while it aims at a submenu.

				CoordSPX 	pointerPos = _toLocal(static_cast<MouseReleaseMsg*>(_pMsg)->pointerSpxPos());
				auto pSource = _findWidgetIgnoringAim(pointerPos, SearchMode::ActionTarget);

				if (!pSource || pSource == this)
				{
					// Release outside any popup.

					// Close top popup and any hierarchy of peek-popups below it.

					_removeTopSlotAndPeeks();
				}
				else
				{
					Widget * pTopPopup = pSlot->widget();
					if(pSource == pTopPopup || pSource->isDescendantOf(pTopPopup) )
					{
						// Relese on top popup or child thereof

						if (pSource->isSelectable())
						{
							// We send 2 messages on selected:
							// PopupSelectMsg from opener (like a PopupOpener if availabe) or PopupOverlay itself.
							// Normal SelectMsg from selected widget itself.

							MsgRouter* pRouter = Base::msgRouter().rawPtr();

							if (pRouter)
							{
								if( pOpener )
									pRouter->post(PopupSelectMsg::create(pOpener, pSource));
								else
									pRouter->post(PopupSelectMsg::create(this, pSource));


								pRouter->post( SelectMsg::create(pSource) );
							}

							if( pSlot->m_bCloseOnSelect )
							{
								int i = 1;
								while( i < popupSlots.size() && popupSlots[i].m_bCloseOnSelect == true )
									i++;

								_removeSlots(0, i);
							}
						}
					}
					else
					{
						// Release on lower popup or widget inside it or its opener.
						// Figure out which one and close all popups ontop of it.

						int ofs = 0;
						Widget * pPopup = popupSlots[ofs].m_pWidget;

						while( pSource != pPopup && ofs < popupSlots.size() && !pSource->isDescendantOf(pPopup) && pSource != popupSlots[ofs].m_pOpener )
							pPopup = popupSlots[++ofs].m_pWidget;

						_removeSlots(0, ofs);
					}

				}

				_pMsg->swallow();
			}
			break;

			case MsgType::MousePress:
			{
				// The only mouse press we are interested in is a press outside all popups,
				// in which case we will close all popups.

				if (popupSlots.isEmpty())
					break;

				auto pMsg = static_cast<MousePressMsg*>(_pMsg);

				auto pSource = static_cast<Widget*>(pMsg->originalSource().rawPtr());
				if (!pSource || pSource == this )
				
					_removeTopSlotAndPeeks();

				if( pSource == this && pMsg->button() != MouseButton::Left && m_bBlockLeftMouseButton )
				{
					CoordSPX 	pointerPos = _toLocal(pMsg->pointerSpxPos());

					auto pTarget = _findWidget(pointerPos, SearchMode::ActionTarget);
					if( pTarget && pTarget != this )
						_pMsg->setRepost(pTarget, pTarget);
				}
				else
					_pMsg->swallow();
			}
			break;

			case MsgType::KeyPress:
			case MsgType::KeyRepeat:
			{
				auto pMsg = static_cast<KeyMsg*>(_pMsg);

				if( pMsg->translatedKeyCode() == Key::Escape )
				{
					if( !popupSlots.isEmpty() )
					{
						_removeSlots(0,1);
						_pMsg->swallow();
					}
				}
			}
			break;

			default:
				break;
		}
	}

	//____ _processPointer() ______________________________________________________

	void PopupOverlay::_processPointer( const CoordSPX& pointerPos, int64_t timestamp )
	{
		m_bAimHeld = false;

		Widget * pMarked = _findWidgetIgnoringAim(pointerPos, SearchMode::ActionTarget);

		if( pMarked != m_pAimHovered.rawPtr() )
		{
			m_pAimHovered = pMarked;
			m_aimHoveredTimestamp = timestamp;
		}

		// Top popup can be in state PeekOpen or Opening, which needs special attention.
		// Neither starts closing while the pointer is on its way to it, see _isAimingAt().

		Slot * pSlot = popupSlots._first();
		if (pSlot->m_state == Slot::State::PeekOpen)
		{
			// Promote popup to state WeakOpen if pointer has entered its geo,
			// otherwise begin delayed closing if pointer has left launcherGeo.

			if (pSlot->m_geo.contains(pointerPos))
				pSlot->m_state = Slot::State::WeakOpen;
			else if (!pSlot->m_launcherGeo.contains(pointerPos))
			{
				if( _isAimingAt(pSlot, pointerPos, timestamp, m_aimHoveredTimestamp) )
					m_bAimHeld = true;
				else
				{
					pSlot->m_state = Slot::State::ClosingDelay;
					pSlot->m_stateCounter = 0;
				}
			}
		}
		else if(pSlot->m_state == Slot::State::Opening)
		{
			if (!pSlot->m_geo.contains(pointerPos) && !pSlot->m_launcherGeo.contains(pointerPos))
			{
				if( _isAimingAt(pSlot, pointerPos, timestamp, m_aimHoveredTimestamp) )
					m_bAimHeld = true;
				else
				{
					pSlot->m_state = Slot::State::Closing;

					if( m_openingFadeMs > 0 && m_closingFadeMs > 0 )
						pSlot->m_stateCounter = (m_openingFadeMs - pSlot->m_stateCounter) * m_closingFadeMs / m_openingFadeMs;
					else
						pSlot->m_stateCounter = 0;
				}
			}
		}
		else if(pSlot->m_state == Slot::State::OpeningDelay)
		{
			// Has not been shown yet, just close it immediately

			if (!pSlot->m_geo.contains(pointerPos) && !pSlot->m_launcherGeo.contains(pointerPos))
			{
				pSlot->m_state = Slot::State::Closing;
				pSlot->m_stateCounter = m_closingFadeMs;
			}
		}

		// Find the popup beneath the mouse pointer and revert any initiated closing of it.
		// Also revert closing of a popup if mouse pointer is within its launcherGeo.

		for (Slot * pSlot = popupSlots._begin() ; pSlot != popupSlots._end() ; pSlot++)
		{
			Slot& popup = *pSlot;

			if (popup.m_launcherGeo.contains(pointerPos))
			{
				if (popup.m_state == Slot::State::ClosingDelay)
				{
					// When pointer enters the launcherGeo of a popup in state ClosingDelay, the popup should be promoted to PeekOpen.

					popup.m_state = popup.m_bPeek ? Slot::State::PeekOpen : Slot::State::FixedOpen;
					popup.m_stateCounter = 0;
					_requestRender( popup._widget()->_renderBounds() + popup.m_geo.pos() );
				}
				else if (popup.m_state == Slot::State::Closing)
				{
					// When pointer enters the launcherGeo of a popup in state Closing, the popup should be promoted to Opening.

					popup.m_state = Slot::State::Opening;

					if( m_openingFadeMs > 0 && m_closingFadeMs > 0 )
						popup.m_stateCounter = (m_closingFadeMs - popup.m_stateCounter) * m_openingFadeMs / m_closingFadeMs;
					else
						popup.m_stateCounter = 0;
				}
			}
			else if (popup.m_geo.contains(pointerPos))
			{
				if (popup.m_state == Slot::State::ClosingDelay)
				{
					// When pointer enters a popup in state ClosingDelay, the popup and its ancestors should be promoted to state WeakOpen.

					Slot * p = &popup;
					while (p != popupSlots._end() && p->m_state == Slot::State::ClosingDelay)
					{
						p->m_state = Slot::State::WeakOpen;
						p->m_stateCounter = 0;
						p++;
					}
				}
				else if (popup.m_state == Slot::State::Closing)
				{
					// When pointer enters a popup in state Closing, the popup and its ancestors should be promoted to state WeakOpen.

					Slot * p = &popup;
					while (p != popupSlots._end() && p->m_state == Slot::State::Closing)
					{
						_requestRender(p->_widget()->_renderBounds() + p->m_geo.pos());
						p->m_state = Slot::State::WeakOpen;
						p->m_stateCounter = 0;
						p++;
					}
				}

				break;		// We have found and acted on our hovered popup, we should not act on any lower popup that might overlap geometrically.
			}
		}

		// If pointer has entered a selectable widget of a popup that isn't the top one
		// and all widgets between them have bPeek=true, they should all enter
		// state ClosingDelay (unless already in state Closing). Not while the pointer
		// is on its way to a submenu of that popup though.

		if (pMarked && pMarked != this && pMarked->isSelectable() && popupSlots._first()->m_bPeek)
		{
			Slot * pTarget = _aimTarget(pMarked);

			if( pTarget && _isAimingAt(pTarget, pointerPos, timestamp, m_aimHoveredTimestamp) )
				m_bAimHeld = true;
			else
			{
				// Trace hierarchy from marked to one of our children.

				while (pMarked->_parent() != this)
					pMarked = pMarked->_parent();

				//

				auto p = popupSlots._first();
				while (p->m_bPeek && p->_widget() != pMarked)
				{
					if (p->m_state != Slot::State::Closing && p->m_state != Slot::State::ClosingDelay)
					{
						p->m_state = Slot::State::ClosingDelay;
						p->m_stateCounter = 0;
					}
					p++;
				}
			}
		}
	}

	//____ _isAimingPast() ________________________________________________________

	// Checks if the pointer, at pointerPos in global coordinates, is passing pHovered
	// on its way to an open submenu of the popup that pHovered is in. A PopupOpener
	// that opens on hover should then wait with opening, see _isAimingAt().

	bool PopupOverlay::_isAimingPast( Widget * pHovered, const CoordSPX& pointerPos, int64_t timestamp, int64_t hoveredSince )
	{
		Slot * pTarget = _aimTarget(pHovered);
		if( !pTarget )
			return false;

		return _isAimingAt(pTarget, _toLocal(pointerPos), timestamp, hoveredSince);
	}

	//____ _isAimingAt() __________________________________________________________

	// Safe triangle: checks if the pointer is on its way to pTarget, a submenu opened from
	// a popup the pointer is still in. If so, pTarget should stay open and its siblings
	// wait with opening, even though the pointer crosses their openers on its way.
	//
	// The pointer is aiming at the submenu as long as it is inside the triangle between
	// where it was a short distance back and the near edge of the submenu, keeps moving
	// and doesn't stay on the same widget for too long.

	bool PopupOverlay::_isAimingAt( Slot * pTarget, const CoordSPX& pointerPos, int64_t timestamp, int64_t hoveredSince )
	{
		if( m_aimTimeoutMs <= 0 || m_nbAimSamples == 0 || !pTarget->m_bPeek )
			return false;

		if( pTarget->m_state != Slot::State::Opening && pTarget->m_state != Slot::State::PeekOpen && pTarget->m_state != Slot::State::WeakOpen )
			return false;

		Slot * pParent = _slotOf(pTarget->m_pOpener.rawPtr());
		if( !pParent )
			return false;

		// Give up once the pointer has stopped or has stayed on one widget too long.
		// A position that isn't sampled yet is a move happening right now.

		bool bSampled = (pointerPos == m_aimSamples[0]);
		int64_t moveTimestamp = bSampled ? m_aimMoveTimestamp : timestamp;

		if( timestamp - moveTimestamp > int64_t(m_aimStillTimeMs) * 1000 || timestamp - hoveredSince > int64_t(m_aimTimeoutMs) * 1000 )
			return false;

		// The triangle starts at the latest sample a few points away. The last few moves
		// alone are mostly jitter when the pointer moves slowly. It needs to be inside the
		// popup the submenu was opened from, since coming from elsewhere isn't aiming.
		// A position that isn't sampled yet skips the sample it will push out, so the
		// answer is the same before and after sampling.

		spx minDistance = ptsToSpx(pts(c_minAimDistance), m_scale);
		int nbSamples = bSampled ? m_nbAimSamples : std::min(m_nbAimSamples, c_maxAimSamples - 1);

		CoordSPX apex = m_aimSamples[nbSamples - 1];
		for( int i = 0 ; i < nbSamples ; i++ )
		{
			if( std::abs(pointerPos.x - m_aimSamples[i].x) + std::abs(pointerPos.y - m_aimSamples[i].y) >= minDistance )
			{
				apex = m_aimSamples[i];
				break;
			}
		}

		if( apex == pointerPos || !pParent->m_geo.contains(apex) )
			return false;

		// The triangle ends at the near edge of the submenu, which depends on what side
		// of its opener it was placed, extended by the tolerance at both ends. But it is
		// never wider than c_maxAimSlope times its height, or moving along the menu close
		// to a tall submenu would count as aiming at it.

		RectSPX content = pTarget->m_geo;
		content -= pTarget->m_popupOverflow;

		if( content.contains(pointerPos) )
			return false;

		const RectSPX& launcher = pTarget->m_launcherGeo;
		spx tolerance = ptsToSpx(m_aimTolerance, m_scale);

		CoordSPX corner1, corner2;

		if( content.left() >= launcher.right() || content.right() <= launcher.left() )
		{
			spx edge = content.left() >= launcher.right() ? content.left() : content.right();
			spx reach = std::abs(edge - apex.x) * c_maxAimSlope;

			corner1 = { edge, std::max(content.top() - tolerance, apex.y - reach) };
			corner2 = { edge, std::min(content.bottom() + tolerance, apex.y + reach) };

			if( corner1.y >= corner2.y )
				return false;
		}
		else if( content.top() >= launcher.bottom() || content.bottom() <= launcher.top() )
		{
			spx edge = content.top() >= launcher.bottom() ? content.top() : content.bottom();
			spx reach = std::abs(edge - apex.y) * c_maxAimSlope;

			corner1 = { std::max(content.left() - tolerance, apex.x - reach), edge };
			corner2 = { std::min(content.right() + tolerance, apex.x + reach), edge };

			if( corner1.x >= corner2.x )
				return false;
		}
		else
			return false;		// Submenu has been squeezed over its opener, so there is no side to aim at.

		return isInTriangle(pointerPos, apex, corner1, corner2);
	}

	//____ _aimTarget() ___________________________________________________________

	// Returns the open submenu of the popup that pHovered is in, unless pHovered is its
	// opener or part of it.

	PopupOverlay::Slot * PopupOverlay::_aimTarget( Widget * pHovered )
	{
		Slot * pParent = _slotOf(pHovered);
		if( !pParent )
			return nullptr;

		for( Slot * p = popupSlots._begin() ; p < pParent ; p++ )
		{
			if( p->m_state == Slot::State::Closing || p->m_state == Slot::State::ClosingDelay )
				continue;

			Widget * pOpener = p->m_pOpener.rawPtr();
			if( pOpener && _slotOf(pOpener) == pParent )
			{
				if( pHovered == pOpener || pHovered->isDescendantOf(pOpener) )
					return nullptr;

				return p;
			}
		}

		return nullptr;
	}

	//____ _slotOf() ______________________________________________________________

	// Returns the slot of the popup that pWidget is in, if any.

	PopupOverlay::Slot * PopupOverlay::_slotOf( Widget * pWidget )
	{
		// Trace hierarchy from widget to one of our children.

		while( pWidget && pWidget->_parent() != this )
			pWidget = pWidget->_parent();

		if( !pWidget )
			return nullptr;

		int index = popupSlots.index(pWidget);
		return index >= 0 ? popupSlots._slot(index) : nullptr;
	}

	//____ _addAimSample() ________________________________________________________

	void PopupOverlay::_addAimSample( const CoordSPX& pointerPos, int64_t timestamp )
	{
		if( m_nbAimSamples > 0 && m_aimSamples[0] == pointerPos )
			return;

		for( int i = c_maxAimSamples - 1 ; i > 0 ; i-- )
			m_aimSamples[i] = m_aimSamples[i-1];

		m_aimSamples[0] = pointerPos;

		if( m_nbAimSamples < c_maxAimSamples )
			m_nbAimSamples++;

		m_aimMoveTimestamp = timestamp;
	}

	//____ _update() _______________________________________________________________

	void PopupOverlay::_update(int microPassed, int64_t microsecTimestamp)
	{
		int ms = microPassed / 1000;

		// A popup kept open since the pointer aimed at it should start closing once the
		// pointer stops, which no message will tell us. So we check again until it does.
		// The same goes for an opener that was passed over and should now be marked,
		// which the input handler otherwise would wait with since the pointer is still.

		if( m_bAimHeld || m_bAimRedirected )
		{
			bool bRemark = m_bAimRedirected;

			m_bAimHeld = false;
			m_bAimRedirected = false;

			auto pInputHandler = Base::inputHandler();

			if( !popupSlots.isEmpty() && pInputHandler->isEntered(this) )
			{
				_processPointer(_toLocal(pInputHandler->pointerSpxPos()), microsecTimestamp);

				if( bRemark && _root() )
					pInputHandler->setPointer(_root(), pInputHandler->pointerPos());
			}
		}

		// Update state for all open popups

		for (Slot* pSlot = popupSlots._begin(); pSlot != popupSlots._end(); pSlot++)
		{
			Slot& popup = *pSlot;

			switch (popup.m_state)
			{
			case Slot::State::OpeningDelay:
				if (popup.m_stateCounter + ms < m_openingDelayMs)
				{
					popup.m_stateCounter += ms;
					break;
				}
				else
				{
					popup.m_state = Slot::State::Opening;
					popup.m_stateCounter -= m_openingDelayMs;
					// No break here, let's continue down to opening...
				}
			case Slot::State::Opening:
				popup.m_stateCounter += ms;
				_requestRender(popup._widget()->_renderBounds() + popup.m_geo.pos());
				if (popup.m_stateCounter >= m_openingFadeMs)
				{
					popup.m_stateCounter = 0;
					popup.m_state = popup.m_bPeek ? Slot::State::PeekOpen : Slot::State::FixedOpen;
				}
				break;

			case Slot::State::ClosingDelay:
				if (popup.m_stateCounter + ms < m_closingDelayMs)
				{
					popup.m_stateCounter += ms;
					break;
				}
				else
				{
					popup.m_state = Slot::State::Closing;
					popup.m_stateCounter -= m_closingDelayMs;
					// No break here, let's continue down to closing...
				}
			case Slot::State::Closing:
				popup.m_stateCounter += ms;
				_requestRender(popup._widget()->_renderBounds() + popup.m_geo.pos());
				// Removing any closed popups is done in next loop
				break;
			default:
				break;
			}

		}

		// Close any popup that is due for closing.

		for( auto p = popupSlots._begin() ; p < popupSlots._end() ; p++ )
		{
			if( p->m_state == Slot::State::Closing && p->m_stateCounter >= m_closingFadeMs )
			{
				_removeSlots( int(p - popupSlots._begin()), 1 );
				p = popupSlots._begin();
			}
		}
	}


	//____ _stealKeyboardFocus() _________________________________________________

	void PopupOverlay::_stealKeyboardFocus()
	{
		// Verify that we have a root

		if( !_root() )
			return;

		// Save old keyboard focus in the top menu, so we can give it back when it closes.

		Slot * pSlot = popupSlots._first();
		pSlot->m_pKeyFocus = _root()->_focusedChild();

		// Steal keyboard focus to top menu

		_childRequestFocus(pSlot, pSlot->_widget() );
	}

	//____ _restoreKeyboardFocus() _________________________________________________

	void PopupOverlay::_restoreKeyboardFocus( Widget * pFocus )
	{
		// Verify that we have a root

		if( !_root() )
			return;

		//

		if( pFocus )
			_holder()->_childRequestFocus( _slot(), pFocus );
		else if( popupSlots.isEmpty() )
		{
			if( _root()->_focusedChild() )
				_holder()->_childReleaseFocus( _slot(), _root()->_focusedChild() );
		}
		else
			_holder()->_childRequestFocus( _slot(), nullptr );
	}

	//____ _childRequestResize() _______________________________________________

	void PopupOverlay::_childRequestResize(StaticSlot * pSlot)
	{
		if( pSlot == &mainSlot )
			_requestResize();
		else
			_updateGeo( (Slot *) pSlot, true );
	}

	//____ _releaseChild() _____________________________________________________

	void PopupOverlay::_releaseChild( StaticSlot * pSlot )
	{
		if (pSlot == &mainSlot)
			Overlay::_releaseChild(pSlot);
		else
		{
			// PopupOverlay is stack-based, releasing a popup forces us to also close all ontop of it

			_removeSlots(0, popupSlots._index(static_cast<Slot*>(pSlot)) + 1);
		}
	}

	//____ _addSlot() ____________________________________________________________

	void PopupOverlay::_addSlot(Widget * _pPopup, Widget * _pOpener, const RectSPX& _launcherGeo, Placement _attachPoint, bool _bPeek, bool _bCloseOnSelect, SizeSPX _maxSize, BorderSPX overflow, bool _bTakeFocus)
	{
		// Close unrelated popups

		Widget * pWidget = _pOpener;

		// Trace hierarchy from marked to one of our children.

		while( pWidget != nullptr && pWidget->_parent() != this )
			pWidget = pWidget->_parent();

		//

		if( pWidget != nullptr && pWidget != mainSlot._widget() )
		{
			auto p = popupSlots._first();
			while (p->m_bPeek && p->_widget() != pWidget)
			{
				if (p->m_state != Slot::State::Closing && p->m_state != Slot::State::ClosingDelay)
				{
					p->m_state = Slot::State::ClosingDelay;
					p->m_stateCounter = 0;
				}
				p++;
			}
		}

		//

		Slot * pSlot = popupSlots._pushFrontEmpty();
		pSlot->m_pOpener = _pOpener;
		pSlot->m_launcherGeo = _launcherGeo;
		pSlot->m_attachPoint = _attachPoint;
		pSlot->m_bPeek = _bPeek;
		pSlot->m_bCloseOnSelect = _bCloseOnSelect;
		pSlot->m_bTakeFocus = _bTakeFocus;
		pSlot->m_state = Slot::State::OpeningDelay;
		pSlot->m_stateCounter = 0;
		pSlot->m_maxSize = _maxSize;
		pSlot->m_popupOverflow = overflow;

		pSlot->_setWidget(_pPopup);

		_updateGeo(pSlot);

		if( _bTakeFocus )
			_stealKeyboardFocus();

		if( m_receivingUpdateCounter == 0 )
			_startReceiveUpdates();

		_refreshStayEnteredList();


		MsgRouter * pEH = Base::msgRouter().rawPtr();
		if( pEH )
			pEH->post( new PopupOpenedMsg( _pPopup, _pOpener ) );
	}

	//____ _removeTopSlotAndPeeks() ______________________________________________

	void PopupOverlay::_removeTopSlotAndPeeks()
	{
		if( popupSlots.isEmpty() )
			return;

		int i = 1;
		while( i < popupSlots.size() && popupSlots[i].m_bPeek )
			i++;

		_removeSlots(0, i);
	}

	//____ _removeSlots() __________________________________________________

	void PopupOverlay::_removeSlots(int ofs, int nb)
	{
		MsgRouter * pEH = Base::msgRouter().rawPtr();

		Slot * pSlot = popupSlots._slot(ofs);

		nb = std::min(nb, popupSlots.size());

		// Focus is only given back when the top popup closes, and then to the widget
		// that held it before the first of the closing popups took it. Popups closing
		// beneath the top one, or that never took focus, leave it alone.

		Widget_p	pRestoreFocus;
		bool		bRestoreFocus = false;

		if( ofs == 0 )
		{
			for( int i = nb - 1 ; i >= 0 ; i-- )
			{
				if( pSlot[i].m_bTakeFocus )
				{
					pRestoreFocus = pSlot[i].m_pKeyFocus.rawPtr();
					bRestoreFocus = true;
					break;
				}
			}
		}

		for(int i = 0 ; i < nb ; i++ )
		{
			if( pEH )
				pEH->post( new PopupClosedMsg( pSlot[i]._widget(), pSlot[i].m_pOpener ) );
			_requestRender(pSlot[i]._widget()->_renderBounds() + pSlot[i].m_geo.pos());
			pSlot[i]._setWidget(nullptr);
		}

		popupSlots._erase(ofs, nb);

		if( bRestoreFocus )
			_restoreKeyboardFocus( pRestoreFocus.rawPtr() );

		if (popupSlots.isEmpty() && m_receivingUpdateCounter > 0)
			_stopReceiveUpdates();

		_refreshStayEnteredList();
	}

	//____ _beginOverlaySlots() __________________________________________________

	const Overlay::Slot * PopupOverlay::_beginOverlaySlots() const
	{
		return popupSlots._begin();
	}

	//____ _endOverlaySlots() __________________________________________________

	const Overlay::Slot * PopupOverlay::_endOverlaySlots() const
	{
		return popupSlots._end();
	}

	//____ _sizeOfOverlaySlot() __________________________________________________

	int PopupOverlay::_sizeOfOverlaySlot() const
	{
		return sizeof(Slot);
	}


} // namespace wg
