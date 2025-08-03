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

#include <wg_columnheader.h>
#include <wg_msg.h>
#include <wg_inputhandler.h>

namespace wg
{
	const TypeInfo	ColumnHeader::TYPEINFO = { "ColumnHeader", &Component::TYPEINFO };

	//____ constructor ___________________________________________________________

	ColumnHeader::ColumnHeader(Widget * pWidget) : Component(pWidget), m_skin(this)
	{
		m_bPressed = false;
		m_sortOrder = SortOrder::None;

		_layout()->addText(this);
	}

	//____ destructor ______________________________________________________________

	ColumnHeader::~ColumnHeader()
	{
		_layout()->removeText(this);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ColumnHeader::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ _initFromBlueprint() ___________________________________________________

	void ColumnHeader::_initFromBlueprint(const Blueprint& bp)
	{
		m_skin.set( bp.skin );
		m_text = bp.text;
		m_pStyle = bp.style;

		if( bp.layout )
		{
			m_pLayout = bp.layout;
			_layout()->addText(this);
		}
	}

	//____ setSkin() _____________________________________________________________

	void ColumnHeader::setSkin( Skin * pSkin )
	{
		if( pSkin != m_skin.get() )
		{
			m_skin.set(pSkin);
			_requestResize();
			_requestRender();		// Resize is not guaranteed to result in a re-render.
		}
	}

	//____ setText() __________________________________________________________

	void ColumnHeader::setText(const CharSeq& seq)
	{
		int removed = m_text.length();
		m_text = seq;
		_layout()->onTextModified(this, 0, removed, m_text.length());
	}

	void ColumnHeader::setText(const String& str)
	{
		int removed = m_text.length();
		m_text = str;
		_layout()->onTextModified(this, 0, removed, m_text.length());
	}

	//____ setStyle() _________________________________________________________

	void ColumnHeader::setStyle(TextStyle* pStyle)
	{
		TextStyle_p pOld = m_pStyle;			// Keep ref count until onStyleChanged has been called.
		m_pStyle = pStyle;
		_layout()->onStyleChanged(this, pStyle, pOld);
	}

	//____ clearStyle() _______________________________________________________

	void ColumnHeader::clearStyle()
	{
		setStyle(nullptr);
	}

	//____ setLayout() ________________________________________________________

	void ColumnHeader::setLayout(TextLayout* pLayout)
	{
		if (pLayout == m_pLayout)
			return;

		_layout()->removeText(this);
		m_pLayout = pLayout;
		_layout()->addText(this);
	}

	//____ clearLayout() ______________________________________________________

	void ColumnHeader::clearLayout()
	{
		setLayout(nullptr);
	}


	//____ _setSortOrder() ________________________________________________________

	void ColumnHeader::_setSortOrder(SortOrder order)
	{
		if (order != m_sortOrder)
		{
			m_sortOrder = order;
			m_state.setChecked(order != SortOrder::None);
			_requestRender();
		}
	}

	//____ _setState() ____________________________________________________________

	void ColumnHeader::_setState( State state )
	{
		State oldState = m_state;
		m_state = state;
		m_state.setChecked(m_sortOrder != SortOrder::None);
		_layout()->onStateChanged(this, state, oldState);


		_requestRender();			//TODO: Only request render if state change requires it.
	}

	//____ _setSize() ____________________________________________________________

	void ColumnHeader::_setSize( SizeSPX size, int scale )
	{
		if( m_size != size || m_scale != scale )
		{
			SizeSPX oldSize = size;
			int		oldScale = scale;

			m_size = size;
			m_scale = scale;

			_layout()->onResized(this, size, oldSize, scale, oldScale);
			_requestRender();
		}
	}

	//____ _defaultSize() _______________________________________________________

	SizeSPX ColumnHeader::_defaultSize(int scale) const
	{

		SizeSPX size = _layout()->defaultSize(this, scale);
		size = m_skin.sizeForContent(size,scale);

		return size;
	}

	//____ _matchingWidth() ____________________________________________________

	spx ColumnHeader::_matchingWidth( spx height, int scale ) const
	{
		SizeSPX border = m_skin.contentBorderSize(scale);

		height -= border.h;
		spx width = _layout()->matchingWidth(this, height, scale);
		width += border.w;

		return width;
	}

	//____ _matchingHeight() ___________________________________________________

	spx ColumnHeader::_matchingHeight( spx width, int scale ) const
	{
		SizeSPX border = m_skin.contentBorderSize(scale);

		width -= border.w;
		spx height = _layout()->matchingHeight(this, width, scale);
		height += border.h;

		return height;
	}

	//____ _receive() _____________________________________________________________

	bool  ColumnHeader::_receive( Msg * _pMsg )
	{
		switch( _pMsg->type() )
		{
			case MsgType::MouseMove:
			{
				MouseMoveMsg_p pMsg = static_cast<MouseMoveMsg*>(_pMsg);
				CoordSPX ofs = _toLocal(pMsg->pointerSpxPos());
				RectSPX geo = {0,0,_size()};
				bool bHovered = geo.contains(ofs) && (!Base::inputHandler()->isAnyButtonPressed() ||
					(Base::inputHandler()->isButtonPressed(MouseButton::Left) && m_bPressed));
				if( bHovered != m_state.isHovered() )
				{
					m_state.setHovered(bHovered);
					_requestRender();
				}
				break;
			}

			case MsgType::MouseLeave:
			{
				MouseLeaveMsg_p pMsg = static_cast<MouseLeaveMsg*>(_pMsg);
				if( m_state.isHovered() )
				{
					m_state.setPressed(false);
					m_state.setHovered(false);
					_requestRender();
				}
				break;
			}

			case MsgType::MousePress:
			{
				MousePressMsg_p pMsg = static_cast<MousePressMsg*>(_pMsg);
				CoordSPX ofs = _toLocal(pMsg->pointerSpxPos());
				RectSPX geo = {0,0,_size()};
				if(pMsg->button() == MouseButton::Left && geo.contains(ofs))
				{
					m_bPressed = true;
					m_state.setPressed(true);
					_requestRender();
					pMsg->swallow();
					return true;
				}
				break;
			}

			case MsgType::MouseDrag:
			{
				MouseDragMsg_p pMsg = static_cast<MouseDragMsg*>(_pMsg);
				if( m_bPressed )
				{
					CoordSPX ofs = _toLocal(pMsg->pointerSpxPos());
					bool bHovered = RectSPX(_size()).contains(ofs);
					if( bHovered != m_state.isHovered() )
					{
						m_state.setHovered(bHovered);
						m_state.setPressed(bHovered);
						_requestRender();
					}
					pMsg->swallow();
					return true;
				}
				break;
			}

			case MsgType::MouseRelease:
			{
				MouseReleaseMsg_p pMsg = static_cast<MouseReleaseMsg*>(_pMsg);
				if(pMsg->button() == MouseButton::Left && m_bPressed )
				{
					m_bPressed = false;
					m_state.setPressed(false);
					_requestRender();

					CoordSPX ofs = _toLocal(pMsg->pointerSpxPos());
					if( RectSPX(_size()).contains(ofs) )
					{
						if( m_sortOrder == SortOrder::Ascending )
							m_sortOrder = SortOrder::Descending;
						else
							m_sortOrder = SortOrder::Ascending;

						m_state.setChecked(true);
						_notify( ComponentNotif::SortOrderChanged, 0, nullptr );
					}
					pMsg->swallow();
					return true;
				}
				break;
			}
			default:
				break;
		}
		return false;
	}

	//____ _render() ____________________________________________________________

	void ColumnHeader::_render( GfxDevice * pDevice, const RectSPX& _canvas )
	{
		RectSPX canvas( _canvas );

		m_skin.render( pDevice, canvas, m_scale, m_state );
		canvas = m_skin.contentRect( canvas, m_scale, m_state );

		_layout()->render(this, pDevice, canvas);
	}

	//____ _textSize() ________________________________________________________

	SizeSPX ColumnHeader::_textSize() const
	{
		return m_skin.contentRect(m_size, m_scale, m_state);
	}

	State ColumnHeader::_textState() const
	{
		return m_state;
	}

	TextStyle* ColumnHeader::_textStyle() const
	{
		return m_pStyle;
	}

	const Char* ColumnHeader::_textBegin() const
	{
		return m_text.chars();
	}

	int ColumnHeader::_textLength() const
	{
		return m_text.length();
	}

	bool ColumnHeader::_caretVisible() const
	{
		return false;
	}

	int ColumnHeader::_caretOffset() const
	{
		return 0;
	}

	std::tuple<int, int> ColumnHeader::_selectedText() const
	{
		return std::make_tuple(0, 0);
	}

	void ColumnHeader::_mapperRequestRender()
	{
		_requestRender( m_skin.contentRect(m_size, m_scale, m_state) );
	}

	void ColumnHeader::_mapperRequestRender(const RectSPX& rect)
	{
		_requestRender(rect + m_skin.contentOfs(m_scale, m_state));
	}

	void ColumnHeader::_mapperRequestResize()
	{
		_requestResize();
	}

	int ColumnHeader::_scale() const
	{
		return m_scale;
	}

	float ColumnHeader::_skinValue(const SkinSlot* pSlot) const
	{
		return m_sortOrder == SortOrder::Descending ? 1.f : 0.f;
	}

	float ColumnHeader::_skinValue2(const SkinSlot* pSlot) const
	{
		return -1;
	}

	State ColumnHeader::_skinState(const SkinSlot* pSlot) const
	{
		return m_state;
	}

	SizeSPX ColumnHeader::_skinSize(const SkinSlot* pSlot) const
	{
		return m_size;
	}

	void ColumnHeader::_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect)
	{
		_requestRender(rect);
	}



} // namespace wg
