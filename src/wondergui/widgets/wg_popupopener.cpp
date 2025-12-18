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
#include <wg_popupopener.h>
#include <wg_container.h>
#include <wg_popupoverlay.h>
#include <wg_msg.h>
#include <wg_inputhandler.h>

namespace wg
{
	using namespace Util;

	const TypeInfo PopupOpener::TYPEINFO = { "PopupOpener", &Widget::TYPEINFO };

	//____ constructor ____________________________________________________________

	PopupOpener::PopupOpener() : label(this), icon(this)
	{
		m_bSelectable = false;
	}

	//____ Destructor _____________________________________________________________

	PopupOpener::~PopupOpener()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PopupOpener::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setPopup() _________________________________________________________

	void PopupOpener::setPopup(Widget * pPopup)
	{
		m_pPopup = pPopup;
	}

	//____ setOpenOnHover() ___________________________________________________

	void PopupOpener::setOpenOnHover(bool bOpen)
	{
		m_bOpenOnHover = bOpen;
	}

	//____ setMouseButton() ______________________________________________________
	
	void PopupOpener::setMouseButton(MouseButton button)
	{
		m_mouseButton = button;
	}


	//____ setAttachPoint() ___________________________________________________

	void PopupOpener::setAttachPoint(Placement attachPoint)
	{
		m_attachPoint = attachPoint;
	}

	//____ _matchingHeight() _______________________________________________________

	spx PopupOpener::_matchingHeight(spx width, int scale) const
	{
		spx height = m_skin.defaultSize(scale).h;

		if (!label.isEmpty())
		{
			SizeSPX padding = m_skin.contentBorderSize(scale);

			spx heightForText = label._matchingHeight(width - padding.w, scale) + padding.h;
			if (heightForText > height)
				height = heightForText;
		}

		//TODO: Take icon into account.

		return height;
	}


	//____ _defaultSize() _____________________________________________________________

	SizeSPX PopupOpener::_defaultSize(int scale) const
	{
		SizeSPX defaultSize;

		if (!label.isEmpty())
			defaultSize = label._defaultSize(scale);

		defaultSize = m_skin.sizeForContent(defaultSize,scale);

		//TODO: Take icon into account.

		return defaultSize;
	}

	//____ _render() __________________________________________________________

	void PopupOpener::_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		Widget::_render(pDevice, _canvas, _window);

		RectSPX	contentRect = m_skin.contentRect(_canvas, m_scale, m_state);

		// Get icon and text rect from content rect

		RectSPX iconRect = _icon()._getIconRect(contentRect,m_scale);
		RectSPX textRect = _icon()._getTextRect(contentRect, iconRect,m_scale);

		// Render icon

		if (_icon().skin())
			_icon().skin()->_render(pDevice, iconRect, m_scale, m_state);

		// Print text

		if (!label.isEmpty())
			label._render(pDevice, textRect);
	}


	//____ _resize() ____________________________________________________________

	void PopupOpener::_resize(const SizeSPX& _size, int scale)
	{
		Widget::_resize(_size,scale);

		RectSPX	contentRect(0, 0, _size);

		contentRect -= m_skin.contentBorderSize(m_scale);

		RectSPX textRect = _icon()._getTextRect(contentRect, _icon()._getIconRect(contentRect,m_scale),m_scale);

		label._setSize(textRect,m_scale);
	}

	//____ _receive() _________________________________________________________

	void PopupOpener::_receive(Msg * pMsg)
	{
		Widget::_receive(pMsg);

		switch (pMsg->type())
		{
			case MsgType::PopupClosed:
			{
				m_bOpen = false;
				break;
			}

			case MsgType::MouseEnter:
			{
				if (m_bOpenOnHover && !m_bOpen)
					_open();
				break;
			}

			case MsgType::MousePress:
			{
				if (!m_bOpenOnHover )
				{
					if (m_bOpen)
					{
						_close();
					}
					else if(static_cast<MouseButtonMsg*>(pMsg)->button() == m_mouseButton)
					{
						_open();
						Base::inputHandler()->_yieldButtonEvents(m_mouseButton, this, m_pPopup);
						m_bPressed = false;		// We have yielded our press...
					}
				}
				break;
			}
/*
			case MsgType::MouseRelease:
			{
				auto pButtonMsg = MouseReleaseMsg::cast(pMsg);
				if (!m_bOpenOnHover && pButtonMsg->button() == m_mouseButton && !pButtonMsg->releaseInside() )
					_close();
				break;
			}
*/

			default:
				break;
		}

		if (pMsg->isMouseButtonMsg() && static_cast<MouseButtonMsg*>(pMsg)->button() == m_mouseButton)
			pMsg->swallow();

	}

	//____ _setState() ________________________________________________________

	void PopupOpener::_setState(State state)
	{
		Widget::_setState(state);
		label._setState(state);
		_requestRender(); //TODO: Only requestRender if text appearance has changed (let text.setState() return if rendering is needed)
	}

	//____ _open() ____________________________________________________________

	void PopupOpener::_open()
	{
		auto pLayer = _parent()->_getPopupOverlay();
		if (pLayer && m_pPopup)
		{
			pLayer->popupSlots.pushFront(m_pPopup, this, pLayer->toLocal(globalGeo()), m_attachPoint, m_bOpenOnHover, m_bCloseOnSelect, Size(1000000, 1000000), m_popupOverflow );
			m_bOpen = true;
		}
	}

	//____ _close() ___________________________________________________________

	void PopupOpener::_close()
	{
		auto pLayer = _parent()->_getPopupOverlay();
		if (pLayer && m_pPopup)
		{
			pLayer->popupSlots.pop(m_pPopup.rawPtr());
		}
	}

	//____ _componentPos() ______________________________________________________________

	CoordSPX PopupOpener::_componentPos(const Component * pComponent) const
	{
		RectSPX contentRect = m_skin.contentRect(m_size, m_scale, m_state);

		// Get icon and text rect from content rect

		RectSPX iconRect = _icon()._getIconRect(contentRect, m_scale);

		if (pComponent == &icon)
			return iconRect.pos();

		RectSPX textRect = _icon()._getTextRect(contentRect, iconRect, m_scale);
		return textRect.pos();
	}

	//____ _componentSize() ______________________________________________________________

	SizeSPX PopupOpener::_componentSize(const Component * pComponent) const
	{
		SizeSPX sz = m_size - m_skin.contentBorderSize(m_scale);

		RectSPX iconRect = _icon()._getIconRect(sz,m_scale);

		if (pComponent == &icon)
			return iconRect.size();

		RectSPX textRect = _icon()._getTextRect(sz, iconRect, m_scale);
		return textRect.size();

	}

	//____ _componentGeo() ______________________________________________________________

	RectSPX PopupOpener::_componentGeo(const Component * pComponent) const
	{
		RectSPX	contentRect = m_skin.contentRect(m_size, m_scale, m_state);

		// Get icon and text rect from content rect

		RectSPX iconRect = _icon()._getIconRect(contentRect, m_scale);

		if (pComponent == &icon)
			return iconRect;

		RectSPX textRect = _icon()._getTextRect(contentRect, iconRect, m_scale);
		return textRect;
	}

}	// namespace wg
