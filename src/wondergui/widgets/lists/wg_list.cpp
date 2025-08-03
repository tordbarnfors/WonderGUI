/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_list.h>
#include <wg_msgrouter.h>
#include <wg_icon.h>
#include <wg_dynamictext.h>
#include <wg_gfxdevice.h>

namespace wg
{


	const TypeInfo List::TYPEINFO = { "List", &Container::TYPEINFO };
	const TypeInfo List::Slot::TYPEINFO = { "List::Slot", &DynamicSlot::TYPEINFO };



	//____ constructor ____________________________________________________________

	List::List() : m_lassoSkin(this), m_selectMode(SelectMode::SingleEntry)
	{
	}

	//____ Destructor _____________________________________________________________

	List::~List()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& List::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setLassoSkin() _____________________________________________________

	void List::setLassoSkin(Skin* pSkin)
	{
		if (m_lassoSkin.get() != pSkin)
		{
			m_lassoSkin.set(pSkin);

			RectSPX dirtyRect(m_lassoBegin, m_lassoEnd);
			_requestRender(dirtyRect);
		}
	}


	//____ setEntrySkin() _________________________________________________________

	bool List::setEntrySkin( Skin * pSkin )
	{
		if (pSkin && pSkin->_isContentShifting())
			return false;

		BorderSPX oldPadding = m_pEntrySkin[0] ? m_pEntrySkin[0]->_contentBorder(m_scale, State::Default) : BorderSPX();

		m_pEntrySkin[0] = pSkin;
		m_pEntrySkin[1] = pSkin;

		m_entryPadding = pSkin ? pSkin->_contentBorder(m_scale, State::Default) : BorderSPX();
		_onEntryPaddingChanged( oldPadding, m_entryPadding );
		_refreshOverflow();
		return true;
	}

	bool List::setEntrySkin( Skin * pOddEntrySkin, Skin * pEvenEntrySkin )
	{
		if (!pOddEntrySkin || !pEvenEntrySkin)
			return false;

		if (pOddEntrySkin->_isContentShifting())
			return false;

		if (pOddEntrySkin->_contentBorder(m_scale,State::Default) != pEvenEntrySkin->_contentBorder(m_scale,State::Default) ||
			pOddEntrySkin->_geoOverflow(m_scale) != pEvenEntrySkin->_geoOverflow(m_scale))
			return false;

		BorderSPX oldPadding = m_pEntrySkin[0] ? m_pEntrySkin[0]->_contentBorder(m_scale, State::Default) : BorderSPX();

		m_pEntrySkin[0] = pOddEntrySkin;
		m_pEntrySkin[1] = pEvenEntrySkin;

		m_entryPadding = pOddEntrySkin->_contentBorder(m_scale, State::Default);
		_onEntryPaddingChanged( oldPadding, m_entryPadding );
		_refreshOverflow();
		return true;
	}

	//____ setSelectMode() ________________________________________________________

	bool List::setSelectMode( SelectMode mode )
	{
		if( mode != m_selectMode )
		{
			m_selectMode = mode;

			//TODO: Unselect all.
		}
		return true;
	}

	//____ _receive() _____________________________________________________________

	void List::_receive( Msg * _pMsg )
	{
		Container::_receive(_pMsg);

		State state = m_state;

		switch( _pMsg->type() )
		{
			case MsgType::MouseMove:
			{
				MouseMoveMsg * pMsg = static_cast<MouseMoveMsg*>(_pMsg);
				Slot * pEntry = _findEntry(_toLocal(pMsg->pointerSpxPos()));
				if( pEntry && pEntry->_widget() != m_pHoveredChild.rawPtr() )
				{
					RectSPX geo;
					if( m_pHoveredChild )
					{
						_getEntryGeo( geo, (Slot*) m_pHoveredChild->_slot() );
						_requestRender(geo + m_maxEntryOverflow);
					}

					_getEntryGeo( geo, pEntry );
					_requestRender(geo + m_maxEntryOverflow);
					m_pHoveredChild = pEntry->_widget();
				}
				break;
			}
			case MsgType::MouseLeave:
			{
				MouseLeaveMsg * pMsg = static_cast<MouseLeaveMsg*>(_pMsg);
				Slot * pEntry = _findEntry(_toLocal(pMsg->pointerSpxPos()));
				if( m_pHoveredChild && !pEntry )
				{
					RectSPX geo;
					_getEntryGeo( geo, (Slot*) m_pHoveredChild->_slot() );
					_requestRender(geo + m_maxEntryOverflow);
					m_pHoveredChild = nullptr;
				}
				break;
			}
			case MsgType::MousePress:
			{
				grabFocus(true);

				MouseButtonMsg * pMsg = static_cast<MouseButtonMsg*>(_pMsg);
				if( m_selectMode != SelectMode::Unselectable && pMsg->button() == MouseButton::Left )
				{
					CoordSPX ofs = _toLocal(pMsg->pointerSpxPos());
					if( !_listWindow().contains(ofs) )
						break;								// Click on header or somewhere else outside the real list.

					RectSPX listArea = _listArea();
					Slot * pEntry = _findEntry(ofs);

					ofs = listArea.limit(ofs);
					m_lassoBegin = ofs;
					m_lassoEnd = ofs;

					if( pEntry )
					{
						switch( m_selectMode )
						{
							case SelectMode::Unselectable:
								break;
							case SelectMode::SingleEntry:
								if( !pEntry->_widget()->state().isSelected() )
								{
									_unselectSlots( _beginSlots(), _endSlots(), true );
									_selectSlot( pEntry, true );
									m_pFocusedChild = pEntry->_widget();
								}
								break;
							case SelectMode::FlipOnSelect:
								_setSlotSelection( pEntry, _nextSlot(pEntry), !pEntry->m_bSelected, true );
								m_pFocusedChild = pEntry->_widget();
								break;
							case SelectMode::MultiEntries:
								if( pMsg->modKeys() & ModKeys::Shift && m_pFocusedChild )
								{
									// Select range from focused to clicked entry.

									Slot * pFocused = (Slot*) m_pFocusedChild->_slot();
									Slot * pBeginSel = std::min( pEntry, pFocused );
									Slot * pEndSel = _nextSlot(std::max( pEntry, pFocused ));
									_selectSlots( pBeginSel, pEndSel, true );

									// Unselect the rest if not CTRL-click.

									if( !(pMsg->modKeys() & ModKeys::StdCtrl) )
									{
										Slot * pBegin = _beginSlots();
										Slot * pEnd = _endSlots();
										if( pBegin < pBeginSel )
											_unselectSlots( pBegin, pBeginSel, true );
										if( pEnd > pEndSel )
											_unselectSlots( pEndSel, pEnd, true );
									}
								}
								else
								{
									if( pMsg->modKeys() & ModKeys::StdCtrl)
									{
										// CTRL-click: We just flip the entry.
										_setSlotSelection( pEntry, _nextSlot(pEntry), !pEntry->m_bSelected, true );
									}
									else
									{
										//TODO: Not post unselected/selected for already selected pressed entry.
										_unselectSlots(_beginSlots(), _endSlots(), true);
										_selectSlot( pEntry, true );
									}
									m_pFocusedChild = pEntry->_widget();
								}
								break;
						}
					}
					else if( m_selectMode == SelectMode::SingleEntry || m_selectMode == SelectMode::MultiEntries )
					{
						_unselectSlots(_beginSlots(), _endSlots(), true);
						m_pFocusedChild = nullptr;
					}

					_pMsg->swallow();
				}
				break;
			}
			case MsgType::MouseRelease:
				if( m_selectMode != SelectMode::Unselectable &&  static_cast<MouseReleaseMsg*>(_pMsg)->button() == MouseButton::Left )
				{
					RectSPX dirtyRect( m_lassoBegin, m_lassoEnd );
					_requestRender(dirtyRect);

					m_lassoBegin = m_lassoEnd;
					_pMsg->swallow();
				}
				break;
			case MsgType::MouseClick:
				if( m_selectMode != SelectMode::Unselectable &&  static_cast<MouseClickMsg*>(_pMsg)->button() == MouseButton::Left )
					_pMsg->swallow();
				break;
			case MsgType::MouseDoubleClick:
			case MsgType::MouseRepeat:
				if( m_selectMode != SelectMode::Unselectable &&  static_cast<MouseButtonMsg*>(_pMsg)->button() == MouseButton::Left )
					_pMsg->swallow();
				break;
			case MsgType::MouseDrag:
			{
				auto pMsg = static_cast<MouseDragMsg*>(_pMsg);
				if( (m_selectMode == SelectMode::FlipOnSelect || m_selectMode == SelectMode::MultiEntries) && pMsg->button() == MouseButton::Left )
				{
					CoordSPX ofs = _listArea().limit(_toLocal(pMsg->pointerSpxPos()));
					ofs = _listWindow().limit(ofs);

					RectSPX oldLasso( m_lassoBegin, m_lassoEnd );
					RectSPX newLasso( m_lassoBegin, ofs );

					_onLassoUpdated( oldLasso, newLasso );

					RectSPX dirtyRect = oldLasso;
					dirtyRect.growToContain( ofs );
					_requestRender( dirtyRect );
					m_lassoEnd = ofs;
					_pMsg->swallow();
				}
				break;
			}

			default:
				break;

		}

		if( state != m_state )
			_setState(state);
	}

	//____ _firstChild() __________________________________________________________

	Widget * List::_firstChild() const
	{
		Slot * p = _beginSlots();
		if (p == _endSlots())
			return nullptr;

		return p->_widget();
	}

	//____ _lastChild() __________________________________________________________

	Widget * List::_lastChild() const
	{
		Slot * p = _endSlots();
		if (p == _beginSlots())
			return nullptr;

		return _prevSlot(p)->_widget();
	}

	//____ _setSlotSelection() _________________________________________________________

	int List::_setSlotSelection(Slot * pBegin, Slot * pEnd, bool bSelected, bool bPostMsg)
	{
		// Count slots to be selected (if we need to post a message)

		int nbChanges = 0;
		for (Slot * p = pBegin; p < pEnd; p = _nextSlot(p))
		{
			if (p->m_bVisible && bSelected != p->m_bSelected && (p->_widget()->isSelectable() || bSelected == false) )
				nbChanges++;
		}

		if (nbChanges == 0)
			return 0;

		ItemInfo * pItemInfo = 0;
		if( bPostMsg )
			pItemInfo = new ItemInfo[nbChanges];

		//

		int nbItems = 0;
		for (Slot * p = pBegin; p < pEnd; p = _nextSlot(p))
		{
			if( p->m_bVisible && bSelected != p->m_bSelected && (p->_widget()->isSelectable() || bSelected == false))
			{
				RectSPX entryGeo;
				_getEntryGeo(entryGeo, p);
				_requestRender( entryGeo + m_maxEntryOverflow);

				p->m_bSelected = bSelected;

				State	state = p->_widget()->state();
				state.setSelected(bSelected);
				p->_widget()->_setState( state );

				if( bPostMsg )
				{
					pItemInfo[nbItems].pObject	= p->_widget();
					pItemInfo[nbItems].id		= p->_widget()->id();
					nbItems++;
				}
			}
		}

		//

		if (bPostMsg)
		{
			if (bSelected)
				Base::msgRouter()->post(ItemsSelectMsg::create(this, nbItems, pItemInfo));
			else
				Base::msgRouter()->post(ItemsUnselectMsg::create(this, nbItems, pItemInfo));
		}

		return nbChanges;
	}





	//____ _flipSelection() _________________________________________________________

	int List::_flipSelection( Slot * pBegin, Slot * pEnd, bool bPostMsg )
	{
		int nSelected = 0;
		int nDeselected = 0;
		Slot * pLast = _prevSlot(pEnd);

		// Request render for the range

		//TODO: This will only work for linear lists. Not FlowLists.

		RectSPX geoFirst;
		RectSPX geoLast;
		_getEntryGeo( geoFirst, pBegin );
		_getEntryGeo( geoLast, pLast );
		_requestRender( RectSPX::bounds(geoFirst + pBegin->_widget()->_overflow(), geoLast + pEnd->_widget()->_overflow()));

		// Reserve ItemInfo array of right size if we are going to post message

		ItemInfo * pSelectedItemsInfo = 0;
		ItemInfo * pDeselectedItemsInfo = 0;
		if( bPostMsg )
		{
			int nToSelect = 0;
			int nToDeselect = 0;

			for( Slot * pSlot = pBegin ; pSlot != pEnd ; pSlot = _nextSlot(pSlot) )
			{
				if (pSlot->m_bVisible)
				{
					if( pSlot->m_bSelected )
						nToDeselect++;
					else if( pSlot->_widget()->isSelectable() )
						nToSelect++;
				}
			}

			if( nToSelect > 0 )
				pSelectedItemsInfo = new ItemInfo[nToSelect];
			if( nToDeselect > 0 )
				pDeselectedItemsInfo = new ItemInfo[nToDeselect];
		}

		// Loop through entries

		for( Slot * pSlot = pBegin ; pSlot != pEnd ; pSlot = _nextSlot(pSlot) )
		{

			if (pSlot->m_bVisible && (pSlot->_widget()->isSelectable() || pSlot->m_bSelected))
			{
				pSlot->m_bSelected = !pSlot->m_bSelected;
				State	state = pSlot->_widget()->state();
				state.setSelected(pSlot->m_bSelected);
				pSlot->_widget()->_setState(state);

				if (bPostMsg)
				{
					ItemInfo * p;
					if (pSlot->m_bSelected)
						p = &pSelectedItemsInfo[nSelected++];
					else
						p = &pDeselectedItemsInfo[nDeselected++];

					p->pObject = pSlot->_widget();
					p->id = pSlot->_widget()->id();
				}
			}
		}

		// Post message

		if( bPostMsg )
		{
			if( nSelected > 0 )
				Base::msgRouter()->post( ItemsSelectMsg::create(this, nSelected, pSelectedItemsInfo) );

			if( nDeselected > 0 )
				Base::msgRouter()->post( ItemsUnselectMsg::create(this, nDeselected, pDeselectedItemsInfo) );
		}

		return nSelected + nDeselected;
	}

	//____ _skinState() __________________________________________________

	State List::_skinState(const SkinSlot* pSlot) const
	{
		if (pSlot == &m_lassoSkin)
			return State::Default;
		else
			return m_state;
	}

	//____ _skinSize() ___________________________________________________

	SizeSPX List::_skinSize(const SkinSlot* pSlot) const
	{
		if (pSlot == &m_lassoSkin)
			return SizeSPX(m_lassoEnd - m_lassoBegin);
		else
			return m_size;

	}

	//____ _skinRequestRender() __________________________________________

	void List::_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect)
	{
		if (pSlot == &m_lassoSkin)
			return _requestRender( rect + m_lassoBegin );
		else
			return _requestRender(rect);
	}

	//____ _calcOverflow() _________________________________________________

	BorderSPX List::_calcOverflow()
	{
		// Calculate and set m_maxEntryOverflow

		BorderSPX	maxChildOverflow;

		auto pEnd = _endSlots();
		for (Slot* pSlot = _beginSlots(); pSlot != pEnd; pSlot = _nextSlot(pSlot))
		{
			if (pSlot->_widget()->_hasOverflow())
				maxChildOverflow.growToContain(pSlot->_widget()->_overflow());
		}

		if (m_pEntrySkin[0])
			m_maxEntryOverflow = BorderSPX::max(m_pEntrySkin[0]->_geoOverflow(m_scale), maxChildOverflow - m_entryPadding);
		else
			m_maxEntryOverflow = maxChildOverflow;

		//

		BorderSPX 	overflow = m_skin.overflow(m_scale);
		overflow.growToContain(m_maxEntryOverflow - m_skin.contentBorder(m_scale, State::Default));
		return overflow;
	}

	//____ _addEntryOverflow() _____________________________________________

	void List::_addEntryOverflow(const BorderSPX& entryOverflow)
	{
		if (m_maxEntryOverflow.contains(entryOverflow))
			return;

		m_maxEntryOverflow = BorderSPX::max(m_maxEntryOverflow, entryOverflow);
		_addChildOverflow(_contentRect(), entryOverflow);
	}


} // namespace wg
