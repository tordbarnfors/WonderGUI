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

#include <wg_selectbox.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>
#include <wg_msgrouter.h>
#include <wg_base.h>
#include <wg_popupoverlay.h>
#include <wg_inputhandler.h>

#include <algorithm>

namespace wg
{

	using namespace Util;

	const TypeInfo SelectBox::TYPEINFO = { "SelectBox", &Widget::TYPEINFO };

	//____ constructor ____________________________________________________________

	SelectBox::SelectBox() : text(this), entries(this)
	{
		m_pListCanvas = new MySideCanvas(this);
	}

	//____ Destructor _____________________________________________________________

	SelectBox::~SelectBox()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SelectBox::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setEntrySkin() _____________________________________________________

	void SelectBox::setEntrySkin(Skin * pSkin)
	{
		SizeSPX oldPadding = m_entryContentPaddingSize;
		SizeSPX newPadding = pSkin ? pSkin->_contentBorderSize(m_scale) : SizeSPX();

		m_pEntrySkin = pSkin;
		if (oldPadding != newPadding)
		{
			for (auto& entry : entries)
				entry.m_height += newPadding.h - oldPadding.h;

			m_listCanvasDefaultSize.w += newPadding.w - oldPadding.w;
			m_listCanvasDefaultSize.h += (newPadding.h - oldPadding.h) * entries.size();

			m_entryContentPaddingSize = newPadding;
			m_pListCanvas->_requestResize();
		}

		m_pListCanvas->_requestRender();
	}

	//____ setEntryStyle() _____________________________________________________

	void SelectBox::setEntryStyle(TextStyle * pStyle)
	{
		if (m_pEntryStyle != pStyle)
		{
			m_pEntryStyle = pStyle;

			m_pListCanvas->_requestResize();
			m_pListCanvas->_requestRender();
		}
	}

	//____ setEntryTextLayout() ______________________________________________

	void SelectBox::setEntryTextLayout(TextLayout * pTextLayout)
	{
		m_pEntryTextLayout = pTextLayout;
		m_pListCanvas->_requestResize();
		m_pListCanvas->_requestRender();
	}

	//____ setListSkin() _____________________________________________________

	void SelectBox::setListSkin(Skin * pSkin)
	{
		Skin* pOldSkin = m_pListCanvas->m_skin.get();

		SizeSPX oldPadding = pOldSkin ? pOldSkin->_contentBorderSize(m_scale) : SizeSPX();
		SizeSPX newPadding = pSkin ? pSkin->_contentBorderSize(m_scale) : SizeSPX();

		if (oldPadding != newPadding)
			m_listCanvasDefaultSize += newPadding - oldPadding;

		m_pListCanvas->m_skin.set(pSkin);
	}

	//____ selectEntry() _____________________________________________________

	void SelectBox::selectEntry(const DynamicVector<SelectBoxEntry>::iterator& it)
	{
		if (it < entries.begin() || it >= entries.end())
		{
			auto pObject = dynamic_cast<Object*>(m_pHolder);
			const TypeInfo* pTypeInfo = pObject ? &pObject->typeInfo() : nullptr;
			Base::throwError(ErrorLevel::SilentError, ErrorCode::InvalidIterator, "Invalid iterator", pObject, pTypeInfo, __func__, __FILE__, __LINE__);
			return;
		}

		_selectEntry( int(it - entries.begin()) );
	}

	//____ selectEntryById() __________________________________________________

	bool SelectBox::selectEntryById(int id)
	{
		for (auto& entry : entries)
		{
			if (entry.m_id == id)
			{
				int idx = int(&entry - &entries[0]);
				_selectEntry(idx);
				return true;
			}
		}
		return false;
	}

	//____ selectEntryByIndex() _______________________________________________

	void SelectBox::selectEntryByIndex(int index)
	{
		if (index < 0 || index >= entries.size())
		{
			auto pObject = dynamic_cast<Object*>(m_pHolder);
			const TypeInfo* pTypeInfo = pObject ? &pObject->typeInfo() : nullptr;
			Base::throwError(ErrorLevel::SilentError, ErrorCode::OutOfRange, "index out of range", pObject, pTypeInfo, __func__, __FILE__, __LINE__);
			return;
		}

		_selectEntry(index);
	}

	//____ selectedEntryId() __________________________________________________

	int SelectBox::selectedEntryId() const
	{
		if (m_selectedEntryIndex >= 0)
			return entries[m_selectedEntryIndex].m_id;
		else
			return -1;
	}

	//____ _defaultSize() _____________________________________________________________

	SizeSPX SelectBox::_defaultSize(int scale) const
	{
		//TODO: Handle defaultSize in other scale than our current.

		return m_defaultSize;
	}

	//____ _matchingHeight() ____________________________________________________________

	spx SelectBox::_matchingHeight(spx width, int scale) const
	{
		if (width == m_size.w && scale == m_scale)
			return m_matchingHeight;
		else
		{
			SizeSPX contentBorder = m_skin.contentBorderSize(scale);

			spx contentWidth = width - contentBorder.w;
			spx contentHeight = 0;

			auto pMapper = _entryTextLayout();
			for (auto& entry : entries)
			{
				contentHeight = std::max(contentHeight, pMapper->matchingHeight(&entry, contentWidth, scale) );
			}
			return contentHeight + contentBorder.h;
		}
	}

	//____ _receive() _________________________________________________________________

	void SelectBox::_receive(Msg* pMsg)
	{
		Widget::_receive(pMsg);

		switch (pMsg->type())
		{
		case MsgType::PopupClosed:
		{
			m_bOpen = false;
			_setState(m_closeState);
			break;
		}

		case MsgType::MousePress:
		{
			if (static_cast<MouseButtonMsg*>(pMsg)->button() == MouseButton::Left)
			{
				if (m_bOpen)
				{
					_close();
				}
				else
				{
					_open();
					Base::inputHandler()->_yieldButtonEvents(MouseButton::Left, this, m_pListCanvas);
					m_bPressed = false;		// We have yielded our press...
				}
			}
			break;
		}

		default:
			break;
		}

		if (pMsg->isMouseButtonMsg() && static_cast<MouseButtonMsg*>(pMsg)->button() == MouseButton::Left)
			pMsg->swallow();

	}

	//____ _render() _____________________________________________________________

	void SelectBox::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		Widget::_render(pDevice,_canvas,_window);

		RectSPX canvas = m_skin.contentRect(_canvas, m_scale, m_state);

		text._render(pDevice, canvas);
	}

	//____ _resize() _______________________________________________________________

	void SelectBox::_resize(const SizeSPX& size, int scale)
	{
		Widget::_resize(size, scale);

		m_matchingHeight = _matchingHeight(size.w,m_scale);
	}

	//____ _setState() ________________________________________________________

	void SelectBox::_setState(State state)
	{
		if (m_bOpen)
		{
			m_closeState = state;
			state.setPressed(true);			// Force pressed state when popup is open.
		}
		Widget::_setState(state);
		text._setState(state);
		_requestRender(); //TODO: Only requestRender if text appearance has changed (let text._setState() return if rendering is needed)
	}

	//____ _open() ____________________________________________________________

	void SelectBox::_open()
	{
		auto pLayer = _parent()->_getPopupOverlay();
		if (pLayer && m_pListCanvas)
		{
			pLayer->popupSlots.pushFront(m_pListCanvas, this, pLayer->toLocal(globalGeo()), Placement::SouthWest, false);
			m_bOpen = true;
			m_closeState = m_state;
		}
	}

	//____ _close() ___________________________________________________________

	void SelectBox::_close()
	{
		auto pLayer = _parent()->_getPopupOverlay();
		if (pLayer && m_pListCanvas)
		{
			pLayer->popupSlots.pop(m_pListCanvas.rawPtr());
		}
	}

	//____ _requestRenderEntry() __________________________________________________

	void SelectBox::_requestRenderEntry(SelectBoxEntry* pEntry)
	{
		if (!m_pListCanvas->_parent())
			return;

		spx ofs = 0;
		for (auto p = &entries[0]; p != pEntry; p++)
			ofs += p->m_height;

		RectSPX rect = m_pListCanvas->_contentRect();
		rect.y += ofs;
		rect.h = pEntry->m_height;

		m_pListCanvas->_requestRender(rect);
	}

	//____ _markEntry() ________________________________________________________

	void SelectBox::_markEntry(int index)
	{
		if (index == m_markedEntryIndex)
			return;

		if (m_markedEntryIndex >= 0)
		{
			State oldState = entries[m_markedEntryIndex].m_state;
			entries[m_markedEntryIndex].m_state.setHovered(false);
			_entryTextLayout()->onStateChanged(&entries[m_markedEntryIndex], entries[m_markedEntryIndex].m_state, oldState);
			_requestRenderEntry(&entries[m_markedEntryIndex]);
		}

		if (index >= 0)
		{
			State oldState = entries[index].m_state;
			entries[index].m_state.setHovered(true);
			_entryTextLayout()->onStateChanged(&entries[index], entries[index].m_state, oldState);
			_requestRenderEntry(&entries[index]);
		}

		m_markedEntryIndex = index;
	}

	//____ _selectEntry() ________________________________________________________

	void SelectBox::_selectEntry(int index)
	{
		if (index == m_selectedEntryIndex)
			return;

		if (m_selectedEntryIndex >= 0)
		{
			State oldState = entries[m_selectedEntryIndex].m_state;
			entries[m_selectedEntryIndex].m_state.setSelected(false);
			_entryTextLayout()->onStateChanged(&entries[m_selectedEntryIndex], entries[m_selectedEntryIndex].m_state, oldState);
			_requestRenderEntry(&entries[m_selectedEntryIndex]);
		}

		State oldState = entries[index].m_state;
		entries[index].m_state.setSelected(true);
		_entryTextLayout()->onStateChanged(&entries[index], entries[index].m_state, oldState);
		_requestRenderEntry(&entries[index]);
		m_selectedEntryIndex = index;

		text.setText(entries[index].m_string);
		_requestRender();

		Base::msgRouter()->post(SelectMsg::create(this));
	}

	//____ _findEntry() _____________________________________________________

	int SelectBox::_findEntry(const CoordSPX& ofsInListPanel, CoordSPX * pOfsOut)
	{
		RectSPX contentRect = m_pListCanvas->_contentRect();

		if (contentRect.contains(ofsInListPanel))
		{
			CoordSPX ofsInEntry = ofsInListPanel - contentRect.pos();
			for (int i = 0; i < entries.size(); i++)
			{
				auto& entry = entries[i];

				if (ofsInEntry.y < entry.m_height)
				{
					if (pOfsOut)
						* pOfsOut = ofsInEntry;

					return i;
				}

				ofsInEntry.y -= entry.m_height;
			}
		}

		return -1;
	}

	//____ _didAddEntries() ______________________________________________________

	void SelectBox::_didAddEntries(SelectBoxEntry * pEntry, int nb)
	{
		auto pMapper = _entryTextLayout();

		SizeSPX entryPadding = m_pEntrySkin ? m_pEntrySkin->_contentBorderSize(m_scale) : SizeSPX();
		SizeSPX boxPadding = m_skin.contentBorderSize(m_scale);
		SizeSPX listPadding = m_pListCanvas->m_skin.contentBorderSize(m_scale);

		SizeSPX	oldDefaultSize		= m_defaultSize;
//		int		oldMatchingHeight = m_matchingHeight;

		for (int i = 0; i < nb; i++)
		{
			// Set entry parent and mapper.

			pEntry->m_pParent = this;
			pMapper->addText(pEntry);

			// Update entry height and listCanvas matchingHeight

			spx entryHeight = pMapper->matchingHeight(pEntry, m_entryContentWidth, m_scale) + entryPadding.h;

			pEntry->m_height = entryHeight;
			m_listCanvasMatchingHeight += entryHeight;

			// Update m_matchingHeight

			spx boxHeight = pMapper->matchingHeight(pEntry, (m_size.w - boxPadding.w), m_scale ) + boxPadding.h;

			if (boxHeight > m_matchingHeight)
				m_matchingHeight = boxHeight;

			// Update m_defaultSize and m_listCanvasDefaultSize

			SizeSPX contentDefault = pMapper->defaultSize(pEntry, m_scale);
			SizeSPX entryDefault = contentDefault + entryPadding;
			SizeSPX boxDefault = contentDefault + boxPadding;

			if (entryDefault.w + listPadding.w > m_listCanvasDefaultSize.w)
				m_listCanvasDefaultSize.w = entryDefault.w + listPadding.w;
			m_listCanvasDefaultSize.h += entryDefault.h;

			if (m_defaultSize.w < boxDefault.w)
				m_defaultSize.w = boxDefault.w;
			if (m_defaultSize.h < boxDefault.h)
				m_defaultSize.h = boxDefault.h;

			pEntry++;
		}

		// Check if we need to adjust defaultSize for the SelectBox.

		if (m_defaultSize != oldDefaultSize)
			_requestResize();

		// No use to requestRender here, added content is outside canvas content rect.

		m_pListCanvas->_requestResize();

	}

	//____ _didMoveEntries() __________________________________________________

	void SelectBox::_didMoveEntries(SelectBoxEntry * pFrom, SelectBoxEntry * pTo, int nb)
	{
		//TODO: Implement!!!
	}

	//____ _willEraseEntries() ________________________________________________

	void SelectBox::_willEraseEntries(SelectBoxEntry * pEntry, int nb)
	{
		//TODO: Implement!!!
	}

	//____ _sideCanvasMatchingHeight() ________________________________________

	spx SelectBox::_sideCanvasMatchingHeight(const SideCanvas * pCanvas, spx width, int scale) const
	{
		if (width == m_pListCanvas->m_size.w)
			return m_listCanvasMatchingHeight;
		else
		{
			SizeSPX canvasPadding = m_pListCanvas->_contentBorderSize(scale);
			SizeSPX entryPadding = m_pEntrySkin ? m_pEntrySkin->_contentBorderSize(m_scale) : SizeSPX();

			spx contentWidth = width - canvasPadding.w - entryPadding.w;

			spx height = canvasPadding.h + entryPadding.h * entries.size();

			auto pMapper = _entryTextLayout();
			for (auto& entry : entries)
			{
				height += pMapper->matchingHeight(&entry, contentWidth, scale);
			}
			return height;
		}
	}

	//____ _sideCanvasMatchingWidth() _________________________________________

	spx SelectBox::_sideCanvasMatchingWidth(const SideCanvas * pCanvas, spx height, int scale) const
	{
		return std::max(m_listCanvasDefaultSize.w, m_size.w);
	}

	//____ _sideCanvasDefaultSize() _________________________________________

	SizeSPX SelectBox::_sideCanvasDefaultSize(const SideCanvas * pCanvas, int scale) const
	{
		return { std::max(m_listCanvasDefaultSize.w, m_size.w), m_listCanvasDefaultSize.h };
	}

	//____ _sideCanvasRender() ________________________________________________
	
	void SelectBox::_sideCanvasRender(SideCanvas * _pWidget, GfxDevice * pDevice, const RectSPX& canvas, const RectSPX& window)
	{
		auto pWidget = static_cast<MySideCanvas*>(_pWidget);
		pWidget->Widget::_render(pDevice, canvas, window);

		RectSPX contentRect = pWidget->_contentRect(canvas);

		TextLayout* pTextLayout = _entryTextLayout();

		CoordSPX pos = contentRect.pos();
		for (auto& entry : entries )
		{
			RectSPX entryRect = { pos, contentRect.w, entry.m_height };
			if (m_pEntrySkin)
			{
				m_pEntrySkin->_render(pDevice, entryRect, m_scale, entry.m_state);
				entryRect = m_pEntrySkin->_contentRect(entryRect, m_scale, entry.m_state);
			}
			pTextLayout->render(&entry, pDevice, entryRect);
			pos.y += entry.m_height;
		}
	}

	//____ _sideCanvasResize() _________________________________________________

	void SelectBox::_sideCanvasResize(SideCanvas * pCanvas, const SizeSPX& size, int scale)
	{
		TextLayout* pMapper = _entryTextLayout();

		SizeSPX listCanvasPaddingSize = m_pListCanvas->m_skin.contentBorderSize(scale);

		spx newContentWidth = size.w - listCanvasPaddingSize.w;
		spx matchingHeight = listCanvasPaddingSize.h;

		for (auto& entry : entries)
		{
			spx newEntryHeight = pMapper->matchingHeight(&entry, newContentWidth, scale);
			pMapper->onResized(&entry, { newContentWidth, newEntryHeight }, { m_entryContentWidth,entry.m_height }, scale, pCanvas->scale() );
			newEntryHeight += m_entryContentPaddingSize.h;
			entry.m_height = newEntryHeight;
			matchingHeight += newEntryHeight;
		}

		m_entryContentWidth = newContentWidth;
		m_listCanvasMatchingHeight = matchingHeight;
		m_pListCanvas->Widget::_resize(size, scale);
	}

	//____ _sideCanvasReceive() ________________________________________________

	void SelectBox::_sideCanvasReceive(SideCanvas * pCanvas, Msg * pMsg)
	{
		switch (pMsg->type())
		{
		case MsgType::MouseEnter:
		case MsgType::MouseMove:
		{
			CoordSPX pos = pCanvas->_toLocal(static_cast<InputMsg*>(pMsg)->pointerSpxPos());
			_markEntry(_findEntry(pos));
			break;
		}

		case MsgType::MouseLeave:
			_markEntry(-1);
			break;

		case MsgType::MouseRelease:
		{
			if (m_markedEntryIndex != -1)
				_selectEntry(m_markedEntryIndex);
			break;
		}


		default:
			break;
		}
	}

	//____ _sideCanvasAlphaTest() _____________________________________________

	bool SelectBox::_sideCanvasAlphaTest(SideCanvas * pCanvas, const CoordSPX& ofs)
	{
		// Test against SideCanvas widget itself

		if (m_pListCanvas->Widget::_alphaTest(ofs))
			return true;

		// Test against entries

		if (m_pEntrySkin)
		{
			CoordSPX ofsInEntry;
			int entryIdx = _findEntry(ofs, &ofsInEntry);

			if (entryIdx >= 0)

			{
				RectSPX contentRect = m_pListCanvas->_contentRect();
				RectSPX entryGeo = { 0,0,contentRect.w,entries[entryIdx].m_height };

				return m_pEntrySkin->_markTest(ofsInEntry, entryGeo, m_scale, entries[entryIdx].m_state);
			}
		}

		return false;
	}

	//____ _sideCanvasSetSkin() _______________________________________________

	void SelectBox::_sideCanvasSetSkin(SideCanvas * pCanvas, Skin * pSkin)
	{
		m_pListCanvas->Widget::setSkin(pSkin);
	}

	//____ _sideCanvasSetState() ______________________________________________

	void SelectBox::_sideCanvasSetState(SideCanvas * pCanvas, State state)
	{
		//TODO: Implement!
	}



	SizeSPX SelectBoxEntry::_textSize() const
	{
		return SizeSPX( m_pParent->m_entryContentWidth, m_height - m_pParent->m_entryContentPaddingSize.h );
	}

	State SelectBoxEntry::_textState() const
	{
		return m_state;
	}

	TextStyle * SelectBoxEntry::_textStyle() const
	{
		return m_pParent->m_pEntryStyle ? m_pParent->m_pEntryStyle : Base::defaultStyle();
	}

	const Char * SelectBoxEntry::_textBegin() const
	{
		return m_string.chars();
	}

	int SelectBoxEntry::_textLength() const
	{
		return m_string.length();
	}

	bool SelectBoxEntry::_caretVisible() const
	{
		return false;
	}

	int SelectBoxEntry::_caretOffset() const
	{
		return 0;
	}

	std::tuple<int, int> SelectBoxEntry::_selectedText() const
	{
		return std::make_tuple(0, 0);
	}

	void SelectBoxEntry::_mapperRequestRender()
	{
		m_pParent->_requestRenderEntry(this);
	}

	void SelectBoxEntry::_mapperRequestRender(const RectSPX& rect)
	{
		m_pParent->_requestRenderEntry(this);			// Make it easy, entry is small anyway.
	}

	void SelectBoxEntry::_mapperRequestResize()
	{
		return;			// We ignore this, make sure to check size whenever we update text,style etc.
	}

	int SelectBoxEntry::_scale() const
	{
		return m_pParent->_scale();
	}


} // namespace wg
