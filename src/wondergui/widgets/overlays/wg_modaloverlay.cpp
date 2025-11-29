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
#include <wg_modaloverlay.h>
#include <wg_util.h>
#include <wg_patches.h>
#include <wg_msgrouter.h>
#include <wg_base.h>
#include <wg_inputhandler.h>

#include <wg_dynamicslotvector.impl.h>


namespace wg
{
	using namespace Util;

	template class DynamicSlotVector<ModalOverlay::Slot>;

	const TypeInfo ModalOverlay::TYPEINFO = { "ModalOverlay", &Overlay::TYPEINFO };
	const TypeInfo ModalOverlay::Slot::TYPEINFO = { "ModalOverlay::Slot", &Overlay::Slot::TYPEINFO };

	//TODO: Improve Modal geometry handling, should be able to run on DefaultSize by default, answering to resize-requests.

	//____ Slot::_setBlueprint() _________________________________________________

	bool ModalOverlay::Slot::_setBlueprint( const Blueprint& bp )
	{
		m_origo = bp.origo;
		m_placementGeo = Rect( bp.pos, bp.size );
		return true;
	}


	//____ Slot::setOrigo() ____________________________________________________

	void ModalOverlay::Slot::setOrigo(const Placement origo)
	{
		m_origo = origo;
		_holder()->_refreshRealGeo(this);
	}

	//____ Slot::setGeo() ____________________________________________________

	void ModalOverlay::Slot::setGeo(const Rect& geometry)
	{
		m_placementGeo = geometry;
		_holder()->_refreshRealGeo(this);
	}

	//____ Slot::setPos() ____________________________________________________

	void ModalOverlay::Slot::setPos(const Coord& ofs)
	{
		m_placementGeo.setPos(ofs);
		_holder()->_refreshRealGeo(this);
	}

	//____ Slot::setSize() ____________________________________________________

	void ModalOverlay::Slot::setSize(const Size& size)
	{
		m_placementGeo.setSize(size);
		_holder()->_refreshRealGeo(this);
	}

	//____ Slot::move() ____________________________________________________

	void ModalOverlay::Slot::move(const Coord& ofs)
	{
		m_placementGeo += ofs;
		_holder()->_refreshRealGeo(this);
	}

	//____ _refreshRealGeo() __________________________________________________

	void ModalOverlay::_refreshRealGeo( Slot * pSlot, bool bForceResize )	// Return false if we couldn't get exactly the requested (floating) geometry.
	{
		RectSPX placementGeo = align(ptsToSpx(pSlot->m_placementGeo,m_scale));

		SizeSPX sz = placementGeo.size();

		if( sz.w == 0 && sz.h == 0 )
			sz = pSlot->_widget()->_defaultSize(m_scale);
		else if( sz.w == 0 )
			sz.w = pSlot->_widget()->_matchingWidth(sz.h,m_scale);
		else if( sz.h == 0 )
			sz.h = pSlot->_widget()->_matchingHeight(sz.w,m_scale);

		if( sz.w <= 0 )
			sz.w = 1;
		if( sz.h <= 0 )
			sz.h = 1;

		CoordSPX ofs = Util::placementToOfs( pSlot->m_origo, m_size ) - Util::placementToOfs( pSlot->m_origo, sz );
		ofs += placementGeo.pos();

		RectSPX geo( ofs, sz );

		if (geo != pSlot->m_geo)
		{
			_onRequestRender(pSlot->m_geo + pSlot->_widget()->_overflow(), pSlot);
			pSlot->m_geo = geo;
			_onRequestRender(pSlot->m_geo + pSlot->_widget()->_overflow(), pSlot);
		}

		if (bForceResize || pSlot->_widget()->_size() != geo.size())
			pSlot->_setSize(geo, m_scale);
	}

	//____ _childRequestResize() ______________________________________________

	void ModalOverlay::_childRequestResize( StaticSlot * pSlot )
	{
		if( pSlot == &mainSlot )
			_requestResize();
		else
		{
			auto p = static_cast<Slot*>(pSlot);
			_refreshRealGeo( p, true );
		}
	}

	//____ _releaseChild() ____________________________________________________

	void ModalOverlay::_releaseChild(StaticSlot * pSlot)
	{
		if (pSlot == &mainSlot)
			Overlay::_releaseChild(pSlot);
		else
		{
			_willEraseSlots(pSlot, 1);
			modalSlots.erase((Slot*)pSlot);
		}
	}

	//____ constructor ____________________________________________________________

	ModalOverlay::ModalOverlay() : modalSlots(this)
	{
	}


	//____ Destructor _____________________________________________________________

	ModalOverlay::~ModalOverlay()
	{
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& ModalOverlay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _matchingHeight() _______________________________________________________

	spx ModalOverlay::_matchingHeight( spx width, int scale ) const
	{
		if( mainSlot._widget() )
			return mainSlot._widget()->_matchingHeight( width, scale );
		else
			return Widget::_matchingHeight(width, scale);
	}

	//____ _matchingWidth() _______________________________________________________

	spx ModalOverlay::_matchingWidth( spx height, int scale ) const
	{
		if( mainSlot._widget() )
			return mainSlot._widget()->_matchingWidth( height, scale );
		else
			return Widget::_matchingWidth(height, scale);
	}

	//____ _defaultSize() _____________________________________________________________

	SizeSPX ModalOverlay::_defaultSize(int scale) const
	{
		if( mainSlot._widget() )
			return mainSlot._widget()->_defaultSize(scale);
		else
			return SizeSPX(64,64);
	}

	//____ _slotTypeInfo() ________________________________________________________

	const TypeInfo&	ModalOverlay::_slotTypeInfo(const StaticSlot * pSlot) const
	{
		if (pSlot == &mainSlot)
			return DynamicSlot::TYPEINFO;

		return Slot::TYPEINFO;
	}

	//____ _findWidget() ____________________________________________________________

	Widget *  ModalOverlay::_findWidget( const CoordSPX& ofs, SearchMode mode )
	{
		// In search mode ACTION_TARGET we always return either the topmost non-hidden modal Widget (or its children),
		// or us.

		if( mode == SearchMode::ActionTarget )
		{
			if( !modalSlots.isEmpty() )
			{
				Slot * pSlot = &modalSlots.back();

				if( pSlot->_widget()->isContainer() )
				{
					Widget * pResult = static_cast<Container*>(pSlot->_widget())->_findWidget( ofs - pSlot->m_geo.pos(), mode );
					if( pResult )
						return pResult;
				}
				else
				{
					if( pSlot->_widget()->_markTest(ofs - pSlot->m_geo.pos()) )
						return pSlot->_widget();
					else
						return this;
				}
			}
			else if( mainSlot._widget() )
			{
				Widget * pWidget = mainSlot._widget();

				if( pWidget->isContainer() )
				{
					Widget * pResult = static_cast<Container*>(pWidget)->_findWidget( ofs, mode );
					if( pResult )
						return pResult;
				}
				else
					return pWidget;
			}

			return nullptr;
		}

		// For the rest of the modes we can rely on the default method.

		return Container::_findWidget( ofs, mode );
	}

	//____ _updateKeyboardFocus() _______________________________________________________

	void ModalOverlay::_updateKeyboardFocus()
	{
		// Get message handler, verify that we have a root

		if( !_holder() || !_holder()->_root() )
			return;

		// Retrieve focused Widget and verify it being a descendant to us.

		Widget * pFocused = Base::inputHandler()->focusedWidget().rawPtr();

		Widget * p = pFocused;
		while( p && p->parent() && p->parent().rawPtr() != this )
			p = p->_parent();

		if( p && p->_parent() != this )
			return;								// Focus belongs to a Widget that is not a descendant to us,
												// so we can't save and shouldn't steal focus.

		// Save old focus so we can return it properly in the future.
		if( p )
		{
			if( p == mainSlot._widget() )
				m_pBaseKeyFocus = pFocused;
			else
			{
				Slot * pSlot = static_cast<Slot*>(p->_slot());
				pSlot->m_pKeyFocus = pFocused;
			}
		}

		// Find which child-branch to focus and switch to our previously saved focus

		Slot * pSlot = &modalSlots.back();

		Widget * 	pSavedFocus = nullptr;
		StaticSlot *		pBranch	= nullptr;

		while( pSlot >= modalSlots.begin() && pSlot->m_geo.isEmpty() )
			pSlot--;

		if( pSlot >= modalSlots.begin() )
		{
			pSavedFocus = pSlot->m_pKeyFocus.rawPtr();
			pSlot->m_pKeyFocus = nullptr;								// Needs to be cleared for the future.
			pBranch = pSlot;
		}
		else if( mainSlot._widget() )
		{
			pSavedFocus = m_pBaseKeyFocus.rawPtr();
			m_pBaseKeyFocus = nullptr;								// Needs to be cleared for the future.
			pBranch = &mainSlot;
		}

		// Verify that saved focus still is within branch and is not hidden

		Widget * pW = pSavedFocus;
		while( pW && pW->_parent() != this )
			pW = pW->_parent();

		if( pW )
		{
			StaticSlot * pSlot = (StaticSlot*) pW->_slot();
			if( pSlot != pBranch )
				pSavedFocus = 0;				// Previously focused Widget is no longer a child of focused branch.
		}

		// Switch to previously saved focus, or null if not applicable

		_holder()->_childRequestFocus( m_pSlot, pSavedFocus );
	}

	//____ _didAddSlots() __________________________________________________

	void ModalOverlay::_didAddSlots(StaticSlot * _pSlot, int nb)
	{
		Slot * pSlot = (Slot*) _pSlot;
		for( int i = 0 ; i < nb ; i++ )
			_refreshRealGeo(&pSlot[i]);

		_updateKeyboardFocus();
	}

	//____ _didMoveSlots() ___________________________________________________________

	void ModalOverlay::_didMoveSlots(StaticSlot * _pFrom, StaticSlot * _pTo, int nb)
	{
		if (nb > 1)
		{
			_requestRender();	//TODO: Optimize! Correctly calculate what is dirty even if more than one is moved.
			return;
		}

		auto pFrom = static_cast<Slot*>(_pFrom);
		auto pTo = static_cast<Slot*>(_pTo);


		// Now we have switched places, pTo contains the widget that was moved

		if (pTo < pFrom)			// We were moved forward
		{
			// Request render on all areas covered by siblings we have skipped in front of.

			Slot * p = pTo+1;
			while (p <= pFrom)
			{
				RectSPX cover = RectSPX::overlap(pTo->_widget()->_renderBounds() + pTo->m_geo.pos(), p->_widget()->_renderBounds() + p->m_geo.pos());

				if (!cover.isEmpty())
					_onRequestRender(cover, pTo);
				p++;
			}
		}
		else							// Move us backward
		{
			// Request render on our siblings for the area we previously have covered.

			Slot * p = pFrom;
			while (p < pTo)
			{
				RectSPX cover = RectSPX::overlap(pTo->_widget()->_renderBounds() + pTo->m_geo.pos(), p->_widget()->_renderBounds() + p->m_geo.pos());

				if (!cover.isEmpty())
					_onRequestRender(cover, p);
				p++;
			}
		}
	}


	//____ _willEraseSlots() __________________________________________________

	void ModalOverlay::_willEraseSlots(StaticSlot * _pSlot, int nb)
	{
		Slot * pSlot = (Slot*) _pSlot;
		for( int i = 0 ; i < nb ; i++ )
		{
			_childRequestRender(pSlot+i, pSlot->_widget()->_renderBounds() );
			pSlot[i].m_geo.clear();
		}

		_updateKeyboardFocus();
	}

	//____ _hideSlots() _____________________________________________________________

	void ModalOverlay::_hideSlots(StaticSlot * _pSlot, int nb)
	{
		// TODO: Implement!!!
	}

	//____ _unhideSlots() _____________________________________________________________

	void ModalOverlay::_unhideSlots(StaticSlot * _pSlot, int nb)
	{
		// TODO: Implement!!!
	}


	//____ _beginOverlaySlots() __________________________________________________

	const Overlay::Overlay::Slot * ModalOverlay::_beginOverlaySlots() const
	{
		return modalSlots.begin();
	}

	//____ _endOverlaySlots() ____________________________________________________

	const Overlay::Overlay::Slot *  ModalOverlay::_endOverlaySlots() const
	{
		return modalSlots.end();
	}

	//____ _sizeOfOverlaySlot() __________________________________________________

	int ModalOverlay::_sizeOfOverlaySlot() const
	{
		return sizeof(Slot);
	}


	//____ _resize() ___________________________________________________________

	void ModalOverlay::_resize( const SizeSPX& sz, int scale )
	{
		Overlay::_resize(sz,scale);

		// Refresh modal widgets geometry, their positions might have changed.

		for( auto pSlot = modalSlots.begin() ; pSlot != modalSlots.end() ; pSlot++ )
			_refreshRealGeo( pSlot );
	}

	//____ _receive() ______________________________________________________________

	void ModalOverlay::_receive( Msg * _pMsg )
	{
		Overlay::_receive(_pMsg);

		if( _pMsg->isInstanceOf( InputMsg::TYPEINFO ) )
		{
			InputMsg * pMsg = static_cast<InputMsg*>(_pMsg);

			if( !modalSlots.isEmpty() && _findWidget( pMsg->pointerSpxPos(), SearchMode::ActionTarget ) == this )
			{
				switch( pMsg->type() )
				{
					case MsgType::MousePress:
					{
						MouseButtonMsg_p pMsg = static_cast<MouseButtonMsg*>(_pMsg);
						Base::msgRouter()->post( new ModalBlockedPressMsg( pMsg->inputId(), pMsg->button(), this, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()) );
					}
					break;

					case MsgType::MouseRelease:
					{
						MouseButtonMsg_p pMsg = static_cast<MouseButtonMsg*>(_pMsg);
						Base::msgRouter()->post( new ModalBlockedPressMsg( pMsg->inputId(), pMsg->button(), this, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()) );
					}
					break;

					case MsgType::MouseMove:
					{
						Base::msgRouter()->post( new ModalMoveOutsideMsg( pMsg->inputId(), this, pMsg->modKeys(), pMsg->pointerPos(), pMsg->pointerSpxPos(), pMsg->timestamp()) );
					}
					break;

					default:
						break;
				}
			}
		}
	}


} // namespace wg
